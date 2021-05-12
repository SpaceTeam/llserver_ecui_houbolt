//
// Created by Markus on 2019-10-15.
//

#include "common.h"
#include <csignal>

#include "utility/Config.h"
#include "utility/utils.h"
#include "driver/SocketOld.h"
#include "hcp/HcpManager.h"
#include "SequenceManager.h"
#include "LLInterface.h"
#include "EcuiSocket.h"

#include "LLController.h"

void LLController::PrintLogo()
{
    std::ifstream f("img/txvLogoSquashed.txt");

    if (f.is_open())
        std::cout << f.rdbuf();

    std::cout << std::endl << std::endl;
    f.close();
}

void LLController::Init(ServerMode serverMode)
{
    try
    {
        std::string configPath = "";
        switch (serverMode)
        {
            case ServerMode::SMALL_TESTSTAND:
                configPath = "config_small_teststand.json";
                break;
            case ServerMode::SMALL_OXFILL:
                configPath = "config_small_oxfill.json";
                break;
            case ServerMode::LARGE_TESTSTAND:
                configPath = "config_Franz.json";
                break;
            default:
                exit(1);

        }
        Config::Init(configPath);

        Debug::Init();

        //LLInterface needs to be initialized first to ensure proper initialization before receiving
        //aynchronous commands from the web server
        PrintLogo();
        std::string version = std::get<std::string>(Config::getData("version"));

        Debug::printNoTime("Version: " + version);
        Debug::printNoTime("\n----------------------");

        Debug::print("Initializing LLInterface...");
        llInterface = LLInterface::Instance();
        llInterface->Init();
        Debug::print("Initializing LLInterface done\n");

        Debug::print("Initializing ECUISocket...");
        EcuiSocket::Init(std::bind(&LLController::OnECUISocketRecv, this, std::placeholders::_1),
                std::bind(&LLController::OnECUISocketClose, this));
        Debug::print("Initializing ECUISocket done\n");

    //    Debug::print("Initializing Sequence Manager...");
    //    SequenceManager::init();
    //    Debug::print("Initializing Sequence Manager done\n");

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

void LLController::OnECUISocketRecv(nlohmann::json msg)
{
    try
    {
        if (utils::keyExists(msg, "type"))
        {
            std::string type = msg["type"];

            if (type.compare("sequence-start") == 0)
            {
                //stop Transmission first
                llInterface->StopStateTransmission();

                //send(sock, strmsg.c_str(), strmsg.size(), 0);
                nlohmann::json seq = msg["content"][0];
                nlohmann::json abortSeq = msg["content"][1];
    //            SequenceManager::StartSequence(msg["content"][0], msg["content"][1], msg["content"][2]);
            }
            else if (type.compare("send-postseq-comment") == 0)
            {
    //            SequenceManager::WritePostSeqComment(msg["content"][0]);
            }
            //TODO: MP Move this logic to state and event manager
            else if (type.compare("abort") == 0)
            {
    //            SequenceManager::AbortSequence("manual abort");
            }
            //TODO: MP probably not even needed
            else if (type.compare("states-load") == 0)
            {
                nlohmann::json states = llInterface->GetAllStates();
                EcuiSocket::SendJson("states-load", states);
            }
            else if (type.compare("states-set") == 0)
            {
                std::string stateName;
                double value;
                uint64_t timestamp;
                for (auto servo : msg["content"])
                {
                    stateName = servo["state"];
                    value = servo["value"];
                    timestamp = servo["timestamp"];
                    llInterface->SetState(stateName, value, timestamp);
                }
            }
            else if (type.compare("states-start") == 0)
            {
                llInterface->StartStateTransmission();
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