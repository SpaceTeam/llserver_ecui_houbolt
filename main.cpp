
#include "common.h"
#include "utils.h"

#include "Socket.h"
#include "SequenceManager.h"


// for convenience

using namespace std;

void testTick(uint64 time)
{
    Debug::print("Hello from tick");
}

void testStop()
{
    Debug::print("Test from Stop");
}

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
            cout << "it works!" << endl;
            SequenceManager::StartSequence(msg["content"][0], msg["content"][1]);
        }
        else if (type.compare("abort") == 0)
        {
            SequenceManager::AbortSequence();
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

    while (true){ sleep(1); }

    Socket::destroy();

    return 0;
}

