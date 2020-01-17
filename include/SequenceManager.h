//
// Created by Markus on 2019-09-27.
//

#ifndef TXV_ECUI_LLSERVER_SEQUENCEMANAGER_H
#define TXV_ECUI_LLSERVER_SEQUENCEMANAGER_H

#include "common.h"
#include "json.hpp"

#include "Logging.h"

//#include "spdlog/async.h"
//#include "spdlog/sinks/basic_file_sink.h"

#include "Timer.h"
#include "I2C.h"

using json = nlohmann::json;

typedef struct point_s
{
    int64 x;
    int64 y;
} Point;

class SequenceManager
{

private:

    static bool isRunning;
    static bool isAutoAbort;
    static bool isAbort;
    static bool isAbortRunning;
    static Timer* timer;
    static Timer* sensorTimer;
    static std::mutex syncMtx;

    //config variables
    static int32 sensorTransmissionInterval;
    static int32 sensorSampleRate;
    static int32 timerSyncInterval;
    //----

    static json jsonSequence;
    static json jsonAbortSequence;

    static std::map<std::string, Point[2]> sequenceIntervalMap;
    static std::map<std::string, double[2]> sensorsNominalRangeMap;

    static void ChangeLogFile();

//    static std::shared_ptr<spdlog::logger> async_file;

    static void LoadIntervalMap();
    static void UpdateIntervalMap(std::string name, int64 microTime, uint8 newValue);


    static void LogSensors(int64 microTime, std::vector<double > sensors);
    static void StopGetSensors();
    static void GetSensors(int64 microTime);
    static void Tick(int64 microTime);

    static void StopAbortSequence();
    static void StartAbortSequence();

    SequenceManager();

    ~SequenceManager();

public:

    static void init();

    static void AbortSequence(std::string abortMsg="abort");
    static void StopSequence();
    static void StartSequence(json jsonSeq, json jsonAbortSeq);




};


#endif //TXV_ECUI_LLSERVER_SEQUENCEMANAGER_H
