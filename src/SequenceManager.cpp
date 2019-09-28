//
// Created by Markus on 2019-09-27.
//

#include "SequenceManager.h"
#include "json.hpp"
#include "Socket.h"

#include "utils.h"

using json = nlohmann::json;

bool SequenceManager::isRunning = false;
Timer* SequenceManager::timer;

json SequenceManager::jsonSequence = json::object();
json SequenceManager::jsonAbortSequence = json::object();

void SequenceManager::StopSequence()
{
    //TODO: implement
    Debug::print("we're done");
    isRunning = false;
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
        timer->start(startTime, endTime, interval, Tick, StopSequence);
    }
}

void SequenceManager::Tick(int64 microTime)
{
    if (microTime % 500000 == 0)
    {
        Debug::print("Micro Seconds: %d", microTime);
    }
    if (microTime % 100000 == 0)
    {
        Socket::sendJson("sequence-sync", ((microTime/1000) / 1000.0));
    }

}