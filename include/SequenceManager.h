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
    static bool isAbort;
    static bool isAbortRunning;
    static Timer* timer;
    static Timer* sensorTimer;

    static json jsonSequence;
    static json jsonAbortSequence;

    static std::map<std::string, Point[2]> sequenceIntervalMap;
    static std::map<std::string, int16[2]> sensorsNominalRangeMap;

    //TODO: move this to another class
    static I2C* i2cDevice;

//    static std::shared_ptr<spdlog::logger> async_file;

    static void LoadIntervalMap();
    static void UpdateIntervalMap(std::string name, int64 microTime, uint8 newValue);

    static void TransmitSensors(int64 microTime, std::map<std::string, uint16> sensors);
    static void LogSensors(int64 microTime, std::vector<uint16> sensors);
    static void StopGetSensors();
    static void GetSensors(int64 microTime);
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
