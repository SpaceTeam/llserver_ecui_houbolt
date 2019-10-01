//
// Created by Markus on 2019-09-27.
//

#ifndef TXV_ECUI_LLSERVER_SEQUENCEMANAGER_H
#define TXV_ECUI_LLSERVER_SEQUENCEMANAGER_H

#include "common.h"
#include "json.hpp"

#include "Timer.h"

using json = nlohmann::json;

class SequenceManager
{

private:

    static bool isRunning;
    static bool isAbort;
    static bool isAbortRunning;
    static Timer* timer;

    static json jsonSequence;
    static json jsonAbortSequence;

    static void Tick(int64 microTime);

    static void StopAbortSequence();
    static void StartAbortSequence();

    SequenceManager();

    ~SequenceManager();

public:

    static void init();

    static void AbortSequence();
    static void StopSequence();
    static void StartSequence(json jsonSeq, json jsonAbortSeq);




};


#endif //TXV_ECUI_LLSERVER_SEQUENCEMANAGER_H
