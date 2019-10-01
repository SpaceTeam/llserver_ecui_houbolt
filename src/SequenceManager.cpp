//
// Created by Markus on 2019-09-27.
//

#include "SequenceManager.h"
#include "json.hpp"
#include "Socket.h"

#include "utils.h"
#include "HcpManager.h"

using json = nlohmann::json;

using namespace std;

bool SequenceManager::isRunning = false;
bool SequenceManager::isAbort = false;
bool SequenceManager::isAbortRunning = false;
Timer* SequenceManager::timer;

json SequenceManager::jsonSequence = json::object();
json SequenceManager::jsonAbortSequence = json::object();

void SequenceManager::init()
{
    timer = new Timer();
}

void SequenceManager::StopSequence()
{
    //TODO: implement
    Debug::info("sequence done");
    isRunning = false;
    if (!isAbort)
    {
        Socket::sendJson("timer-done");
    }
}

void SequenceManager::AbortSequence()
{
    if (isRunning)
    {
        isAbort = true;
        timer->stop();
        Socket::sendJson("abort");
        Debug::print("aborting...");

        isRunning = false;
        StartAbortSequence();
        isAbort = false;
    }
    else
    {
        Debug::error("cannot abort sequence: no running sequence");
    }

}

void SequenceManager::StartSequence(json jsonSeq, json jsonAbortSeq)
{
    if (!isRunning && !isAbortRunning)
    {
        isRunning = true;
        jsonSequence = jsonSeq;
        jsonAbortSequence = jsonAbortSeq;


        int64 startTime = utils::toMicros(jsonSeq["globals"]["startTime"]);
        int64 endTime = utils::toMicros(jsonSeq["globals"]["endTime"]);
        int64 interval = utils::toMicros(jsonSeq["globals"]["interval"]);
        Debug::info("%d %d %d", startTime, endTime, interval);

        Socket::sendJson("timer-start");

        HcpManager::EnableAllServos();
        timer->start(startTime, endTime, interval, Tick, StopSequence);
        //TODO: discuss if we need that feature
//        if (HcpManager::EnableAllServos())
//        {
//            timer->start(startTime, endTime, interval, Tick, StopSequence);
//        }
//        else
//        {
//            Debug::error("couldn't enable all servos, aborting...");
//            AbortSequence();
//        }

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
                Debug::print("timestamp | %d", time);
                for (auto it = actionItem.begin(); it != actionItem.end(); ++it)
                {
                    if (it.key().compare("timestamp") != 0)
                    {
                        Debug::print(it.key() + " | %d", (uint8)it.value());
                        HcpManager::ExecCommand(it.key(), it.value());
                    }
                }
                Debug::print("--------------");
            }
            else if (timestampMicros > microTime)
            {
                break;
            }
        }
    }

}

void SequenceManager::StopAbortSequence()
{
    if (isAbortRunning)
    {
        Debug::info("abort sequence done");
        isAbortRunning = false;
        HcpManager::DisableAllServos();
    }
}

void SequenceManager::StartAbortSequence()
{
    if (!isRunning && !isAbortRunning)
    {
        isAbortRunning = true;

        for (auto it = jsonAbortSequence["actions"].begin(); it != jsonAbortSequence["actions"].end(); ++it)
        {
            if (it.key().compare("timestamp") != 0)
            {
                Debug::print(it.key() + " | %d", (uint8)it.value());
                HcpManager::ExecCommand(it.key(), it.value());
            }
        }
        Debug::print("--------------");
        StopAbortSequence();
    }

}
