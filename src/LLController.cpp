//
// Created by Markus on 2019-10-15.
//

#include "utils.h"
#include "Socket.h"
#include "HcpManager.h"
#include "SequenceManager.h"
#include "LLInterface.h"
#include "EcuiSocket.h"

#include "LLController.h"

using namespace std;

void LLController::Init()
{
    EcuiSocket::Init(OnECUISocketRecv);

    LLInterface::Init();
    SequenceManager::init();
}

void LLController::Destroy()
{
    EcuiSocket::Destroy();
}

void LLController::OnECUISocketRecv(json msg)
{
    if (utils::keyExists(msg, "type"))
    {
        string type = msg["type"];

        if (type.compare("sequence-start") == 0)
        {
            //send(sock, strmsg.c_str(), strmsg.size(), 0);
            json seq = msg["content"][0];
            json abortSeq = msg["content"][1];
            SequenceManager::StartSequence(msg["content"][0], msg["content"][1]);
        }
        else if (type.compare("abort") == 0)
        {
            SequenceManager::AbortSequence();
        }
        else if (type.compare("servos-load") == 0)
        {
            json servosData = HcpManager::GetAllServoData();
            EcuiSocket::SendJson("servos-load", servosData);
        }
        else if (type.compare("servos-enable") == 0)
        {
            HcpManager::EnableAllServos();
        }
        else if (type.compare("servos-disable") == 0)
        {
            HcpManager::DisableAllServos();
        }
        else if (type.compare("servos-set") == 0)
        {
            string name;
            uint8 value;
            for (auto servo : msg["content"])
            {
                name = servo["id"];
                value = servo["value"];
                HcpManager::SetServo(name, value);
            }
        }
        else if (type.compare("servos-set-raw") == 0)
        {
            string name;
            uint16 value;
            for (auto servo : msg["content"])
            {
                name = servo["id"];
                value = servo["value"];
                HcpManager::SetServoRaw(name, value);
            }
        }
        else if (type.compare("servos-calibrate") == 0)
        {
            for (auto servo : msg["content"])
            {
                if (utils::keyExists(servo, "min"))
                {
                    HcpManager::SetServoMin(servo["id"], servo["min"]);
                }
                else if (utils::keyExists(servo, "max"))
                {
                    HcpManager::SetServoMax(servo["id"], servo["max"]);
                }
                else
                {
                    Debug::error("no valid key in servos-calibrate found");
                }
            }
            //Could be more efficient if new min and max are saved from for loop above
            json servosData = HcpManager::GetAllServoData();
            EcuiSocket::SendJson("servos-load", servosData);

        }
        else if (type.compare("digital-outs-set") == 0)
        {
            string name;
            bool value;
            for (auto digitalOutputs : msg["content"])
            {
                name = digitalOutputs["id"];
                value = digitalOutputs["value"];
                HcpManager::SetDigitalOutputs(name, value);
            }
        }
        else
        {
            cerr << "message not supported" << endl;
        }
    }
}
