
#include "common.h"
#include "utils.h"

#include "Socket.h"
#include "HcpManager.h"
#include "SequenceManager.h"


// for convenience

using namespace std;


void processMessage(int sock, json msg)
{
    json testmsg;
    testmsg["type"] = "success";
    testmsg["content"] = json::object();
    string strmsg = testmsg.dump() + "\n\0";
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
        else if (type.compare("servos-set") == 0)
        {
            string name;
            uint8 value;
            for (auto servo : msg)
            {
                name = servo["id"];
                value = servo["value"];
                HcpManager::SetServo(name, value);
            }
        }
        else if (type.compare("servos-set-raw") == 0)
        {
            string name;
            uint8 value;
            for (auto servo : msg)
            {
                name = servo["id"];
                value = servo["value"];
                HcpManager::SetServoRaw(name, value);
            }
        }
        else if (type.compare("servos-calibrate") == 0)
        {
            for (auto servo : msg)
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

        }
        else
        {
            cerr << "message not supported" << endl;
        }
    }

}

int main(int argc, char const *argv[])
{
    Socket::init(processMessage);

    //TODO: make another class which manages inits, so seq manager doesn't need initialized hcp manager
    HcpManager::init();
    SequenceManager::init();

    while (true){ sleep(1); }

    Socket::destroy();

    return 0;
}

