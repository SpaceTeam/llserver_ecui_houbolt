//
// Created by Markus on 2019-09-27.
//

#include "SequenceManager.h"
#include "json.hpp"
#include "Socket.h"

#include "utils.h"

using json = nlohmann::json;

using namespace std;

bool SequenceManager::isRunning = false;
Timer* SequenceManager::timer;

json SequenceManager::jsonSequence = json::object();
json SequenceManager::jsonAbortSequence = json::object();

void SequenceManager::StopSequence()
{
    //TODO: implement
    Debug::print("we're done");
    isRunning = false;
    Socket::sendJson("timer-done");
}

void SequenceManager::AbortSequence()
{
    if (isRunning)
    {
        timer->stop();
        Debug::print("aborted");

        //TODO: start abort sequence
    }
    else
    {
        Debug::error("cannot abort sequence: no running sequence");
    }

}

void SequenceManager::StartSequence(json jsonSeq, json jsonAbortSeq)
{
    if (!isRunning)
    {
        isRunning = true;
        jsonSequence = jsonSeq;
        jsonAbortSequence = jsonAbortSequence;

        timer = new Timer();
        int64 startTime = utils::toMicros(jsonSeq["globals"]["startTime"]);
        int64 endTime = utils::toMicros(jsonSeq["globals"]["endTime"]);
        int64 interval = utils::toMicros(jsonSeq["globals"]["interval"]);
        Debug::info("%d %d %d", startTime, endTime, interval);

        Socket::sendJson("timer-start");

        timer->start(startTime, endTime, interval, Tick, StopSequence);
    }
}

void SequenceManager::Tick(int64 microTime)
{
    if (microTime % 500000 == 0)
    {
        Debug::print("Micro Seconds: %d", microTime);
    }
    if (microTime % TIMER_SYNC_INTERVAL == 0)
    {
        Socket::sendJson("timer-sync", ((microTime/1000) / 1000.0));
    }

    //TODO: this is very inefficient right now; make it so actions can be accessed by timestamp as key
    for (auto dataItem : jsonSequence["data"])
    {
        for (auto actionItem : dataItem["actions"])
        {

            float time;
            if (actionItem["timestamp"].type() == json::value_t::string)
            {
                string timeStr = actionItem["timestamp"];
                if (timeStr.compare("START") == 0)
                {
                    time = utils::toMicros(jsonSequence["globals"]["startTime"]);
                }
                else if (timeStr.compare("END") == 0)
                {
                    time = utils::toMicros(jsonSequence["globals"]["endTime"]);
                }
            }
            else
            {
                time = actionItem["timestamp"];
            }

            int64 timestampMicros = utils::toMicros(time);
            if (timestampMicros == microTime)
            {
                cout << "timestamp | " << time << endl;
                for (auto it = actionItem.begin(); it != actionItem.end(); ++it)
                {
                    if (it.key().compare("timestamp") != 0)
                    {
                        std::cout << it.key() << " | " << it.value() << "\n";
                    }
                }
                cout << "--------------" << endl;
            }
            else if (timestampMicros > microTime)
            {
                break;
            }
        }
    }

}