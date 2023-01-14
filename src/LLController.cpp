//
// Created by Markus on 2019-10-15.
//

#include "common.h"
#include <csignal>
#include <thread>

#include "utility/Config.h"
#include "utility/utils.h"
#include "SequenceManager.h"
#include "LLInterface.h"
#include "EcuiSocket.h"
#include "EventManager.h"
#include "driver/PythonController.h"

#include "LLController.h"

void LLController::PrintLogo()
{
    std::ifstream f("img/stAscii.txt");

    if (f.is_open())
        std::cout << f.rdbuf();

    std::cout << std::endl << std::endl;
    f.close();
}

void LLController::Init(std::string &configPath)
{
    try
    {
        Config::Init(configPath);
        config = Config();

        Debug::Init();

        //LLInterface needs to be initialized first to ensure proper initialization before receiving
        //aynchronous commands from the web server
        PrintLogo();
        std::string version = config["/version"];

        Debug::printNoTime("Version: " + version);
        Debug::printNoTime("\n----------------------");

        Debug::print("Initializing LLInterface...");
        llInterface = LLInterface::Instance();
        llInterface->Init(config);
        Debug::print("Initializing LLInterface done\n");

        Debug::print("Initializing ECUISocket...");
        EcuiSocket::Init(std::bind(&LLController::OnECUISocketRecv, this, std::placeholders::_1),
                std::bind(&LLController::OnECUISocketClose, this));
        //TODO: new thread with periodic keep alive messages
        Debug::print("Initializing ECUISocket done\n");

        //TODO: MP maybe move to llInterface
        Debug::print("Initializing Sequence Manager...");
        seqManager = SequenceManager::Instance();
        seqManager->Init();
        Debug::print("Initializing Sequence Manager done\n");

        Debug::printNoTime("----------------------");
        Debug::print("Low-Level Server started!\n");

        initialized = true;
    }
    catch (std::exception &e)
    {
        Debug::error("LLController - Init: initialization failed, %s", e.what());
        raise(SIGTERM);
    }



}

bool LLController::IsInitialized()
{
    return initialized;
}

LLController::~LLController()
{
    //    SequenceManager::AbortSequence();
    Debug::print("Shutting down PythonController...");
    PythonController::Destroy();
    Debug::print("Shutting down ECUISocket...");
    EcuiSocket::Destroy();
    Debug::print("Shutting down LLInterface...");
    LLInterface::Destroy();
    Debug::print("Shutting down Debug...");
    Debug::close();
}

/**
 * used to abort on ecui socket from hardware
 */
void LLController::Abort(std::string &abortMsg)
{
    EcuiSocket::SendJson("abort", abortMsg);
}

// Callback on recieving message from ECUI Socket
void LLController::OnECUISocketRecv(nlohmann::json msg)
{
    try
    {
        if (utils::keyExists(msg, "type"))
        {
            std::string type = msg["type"];

            if (type.compare("sequence-start") == 0)
            {
                //TODO: stop Transmission first (from old version, still needed?)
                // llInterface->StopStateTransmission();

                //send(sock, strmsg.c_str(), strmsg.size(), 0);
                nlohmann::json seq = msg["content"][0];
                nlohmann::json abortSeq = msg["content"][1];
                seqManager->StartSequence(msg["content"][0], msg["content"][1], msg["content"][2]);
            }
            else if (type.compare("send-postseq-comment") == 0)
            {
                seqManager->WritePostSeqComment(msg["content"][0]);
            }
            //TODO: MP Move this logic to state and event manager
            else if (type.compare("abort") == 0)
            {
                seqManager->AbortSequence("manual abort");
            }
            else if (type.compare("auto-abort-change") == 0)
            {
                bool isAutoAbortActive = msg["content"];
                seqManager->SetAutoAbort(isAutoAbortActive);
                //send it to server as acknowledgement
                isAutoAbortActive = seqManager->GetAutoAbort();
                EcuiSocket::SendJson("auto-abort-change", isAutoAbortActive);
            }
            //TODO: MP probably not even needed
            else if (type.compare("states-load") == 0)
            {
                nlohmann::json stateLabels = llInterface->GetAllStateLabels();
                while (stateLabels.size() > 500)
                {
                    nlohmann::json stateLabelsChunk(stateLabels.begin(), stateLabels.begin() + 500);
                    
                    EcuiSocket::SendJson("states-load", stateLabelsChunk);
                    stateLabels.erase(stateLabels.begin(), stateLabels.begin() + 500);
                }
                
                EcuiSocket::SendJson("states-load", stateLabels);

                //send all states to initialize correctly
                nlohmann::json states = llInterface->GetAllStates();

                while (states.size() > 500)
                {
                    nlohmann::json statesChunk(states.begin(), states.begin() + 500);
                    
                    EcuiSocket::SendJson("states-init", statesChunk);
                    states.erase(states.begin(), states.begin() + 500);
                }
                
                EcuiSocket::SendJson("states-init", states);

                bool isAutoAbortActive = seqManager->GetAutoAbort();
                EcuiSocket::SendJson("auto-abort-change", isAutoAbortActive);
            }
            else if (type.compare("states-get") == 0)
            {
                nlohmann::json states = llInterface->GetStates(msg["content"]);
                EcuiSocket::SendJson("states", states);
            }
            else if (type.compare("states-set") == 0)
            {
                if (seqManager->IsSequenceRunning())
                {
                    //TODO: send error, states-set not allowed when sequence is running
                    throw std::runtime_error("states-set not allowed when sequence is running");
                }
                std::string stateName;
                double value;
                uint64_t timestamp;
                for (auto state : msg["content"])
                {
                    stateName = state["name"];
                    value = state["value"];
                    timestamp = state["timestamp"];
                    Debug::print("%s, %f, %d", stateName.c_str(), value, timestamp);
                    llInterface->SetState(stateName, value, timestamp);
                }
            }
            else if (type.compare("states-start") == 0)
            {
                Debug::print("state transmission started");
                llInterface->StartStateTransmission(config);
            }
            else if (type.compare("states-stop") == 0)
            {
                llInterface->StopStateTransmission();
            }
            else if (type.compare("gui-mapping-load") == 0)
            {
                nlohmann::json mapping = llInterface->GetGUIMapping();
                EcuiSocket::SendJson("gui-mapping-load", mapping);
            }
            else if (type.compare("commands-load") == 0)
            {
                auto commandMap = llInterface->GetCommands();
                nlohmann::json commandsJson = nlohmann::json::array();
                nlohmann::json commandJson;
                for (auto &command : commandMap)
                {
                    commandJson = nlohmann::json::object();
                    commandJson["commandName"] = command.first;
                    std::vector<std::string> parameterNames = std::get<1>(command.second);
                    commandJson["parameterNames"] = nlohmann::json::array();
                    for (auto &paramName : parameterNames)
                    {
                        commandJson["parameterNames"].push_back(paramName);
                    }
                    commandsJson.push_back(commandJson);
                    if (commandsJson.size() >= 500)
                    {
                        EcuiSocket::SendJson("commands-load", commandsJson);
                        commandsJson.erase(commandsJson.begin(), commandsJson.begin() + 500);
                    }
                }
                
                EcuiSocket::SendJson("commands-load", commandsJson);
            }
            else if (type.compare("commands-set") == 0)
            {
                nlohmann::json commandsErrorJson = nlohmann::json::array();
                for (auto &command : msg["content"])
                {
                    try
                    {
                        std::string commandName = command["commandName"];
                        std::vector<double> params = command["params"];
                        bool testOnly = command["testOnly"];
                        llInterface->ExecuteCommand(commandName, params, testOnly);
                    }
                    catch (std::exception &e)
                    {
                        nlohmann::json errorObj = nlohmann::json::object();
                        errorObj["commandName"] = command["commandName"];
                        errorObj["command"] = command;
                        errorObj["errorMessage"] = e.what();

                        commandsErrorJson.push_back(errorObj);
                    }

                }
                if (!commandsErrorJson.empty())
                {
                    EcuiSocket::SendJson("commands-error", commandsErrorJson);
                }

            }
            else if (type.compare("pythonScript-start") == 0) {
#ifdef NO_PYTHON
                Debug::warning("Python features not enabled in build settings");
#else
                std::string scriptPath = msg["content"];
                PythonController *pyController = PythonController::Instance();
                pyController->StartPythonScript(scriptPath);
#endif
            }
            else if (type.compare("pythonScript-runChecklistItem") == 0) {
#ifdef NO_PYTHON
                Debug::warning("Python features not enabled in build settings");
#else
                std::string scriptPath = msg["content"]["scriptPath"];
                std::string item = msg["content"]["checklistItem"];
                std::vector<std::string> args = {"script.py", "run_checklist_item", item};
                PythonController *pyController = PythonController::Instance();
                pyController->StartPythonScript(scriptPath, args);
#endif
            }
            else
            {
                throw std::runtime_error("ECUISocket: message not supported");
            }
        }
    }
    catch (std::exception &e)
    {
        Debug::error("LLController - OnECUISocketRecv: Message processing failed ignoring msg for now, %s", e.what());
        Debug::print("Json msg dump: \n%s", ((std::string) msg.dump(4)).c_str());
    }
}

void LLController::OnECUISocketClose()
{

}
