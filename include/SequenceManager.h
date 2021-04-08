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
#include "drivers/I2C.h"

using json = nlohmann::json;

typedef struct point_s
{
    int64_t x;
    int64_t y;
} Point;

typedef enum class interpolation_e
{
    NONE,
    LINEAR
} Interpolation;

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
    static int32_t sensorTransmissionInterval;
    static int32_t sensorSampleRate;
    static int32_t timerSyncInterval;
    //----

    static nlohmann::json jsonSequence;
    static nlohmann::json jsonAbortSequence;
    static std::string comments;
    static std::string currentDirPath;
    static std::string logFileName;
    static std::string lastDir;

    static std::map<std::string, Interpolation> interpolationMap;
    static std::map<int64_t, std::map<std::string, double[2]>> sensorsNominalRangeTimeMap;
    static std::map<std::string, std::map<int64_t, double[2]>> sensorsNominalRangeMap;
    static std::map<std::string, std::map<int64_t, double>> deviceMap;

    static void SetupLogging();

    static void LoadInterpolationMap();
    static bool LoadSequence(nlohmann::json jsonSeq);

    static void LogSensors(int64_t microTime, std::vector<double > sensors);
    static void StopGetSensors();
    static void GetSensors(int64_t microTime);

    static double GetTimestamp(nlohmann::json obj);
    static void Tick(int64_t microTime);

    static void StopAbortSequence();
    static void StartAbortSequence();


    static void plotMaps(uint8_t option);

    SequenceManager();

    ~SequenceManager();

public:

    static void init();

    static void AbortSequence(std::string abortMsg="abort");
    static void StopSequence();
    static void StartSequence(nlohmann::json jsonSeq, nlohmann::json jsonAbortSeq, std::string comments);
    static void WritePostSeqComment(std::string msg);

};


#endif //TXV_ECUI_LLSERVER_SEQUENCEMANAGER_H
