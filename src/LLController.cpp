//
// Created by Markus on 2019-10-15.
//

#include "Config.h"
#include "utils.h"
#include "drivers/SocketOld.h"
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

void LLController::Init()
{
    //LLInterface needs to be initialized first to ensure proper initialization before receiving
    //aynchronous commands from the web server
    PrintLogo();
    std::string version = std::get<std::string>(Config::getData("version"));

    Debug::printNoTime("Version: " + version);
    Debug::printNoTime("\n----------------------");

    Debug::print("Initializing LLInterface...");
    LLInterface::Init();
    Debug::print("Initializing LLInterface done\n");

    Debug::print("Initializing ECUISocket...");
    EcuiSocket::Init(OnECUISocketRecv, Abort);
    Debug::print("Initializing ECUISocket done\n");

//    Debug::print("Initializing Sequence Manager...");
//    SequenceManager::init();
//    Debug::print("Initializing Sequence Manager done\n");

    Debug::printNoTime("----------------------");
    Debug::print("Low-Level Server started!\n");
}

void LLController::Destroy()
{
//    SequenceManager::AbortSequence();
    EcuiSocket::Destroy();
    Debug::close();
    LLInterface::Destroy();
}

void LLController::Abort()
{
    Destroy();
}

void LLController::OnECUISocketRecv(nlohmann::json msg)
{
    if (utils::keyExists(msg, "type"))
    {
        std::string type = msg["type"];

        if (type.compare("sequence-start") == 0)
        {
            //stop Transmission first
            LLInterface::StopStateTransmission();

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
            nlohmann::json states = LLInterface::GetAllStates();
            EcuiSocket::SendJson("states-load", states);
        }
        else if (type.compare("states-set") == 0)
        {
            std::string name;
            double value;
            uint64_t timestamp;
            for (auto servo : msg["content"])
            {
                name = servo["state"];
                value = servo["value"];
                timestamp = servo["timestamp"];
                LLInterface::SetState(name, value, timestamp);
            }
        }
        else if (type.compare("states-start") == 0)
        {
            LLInterface::StartStateTransmission();
        }
        else if (type.compare("states-stop") == 0)
        {
            LLInterface::StopStateTransmission();
        }
        else
        {
            Debug::error("ECUISocket: message not supported");
        }
    }
}
