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
    static int32 sensorTransmissionInterval;
    static int32 sensorSampleRate;
    static int32 timerSyncInterval;
    //----

    static json jsonSequence;
    static json jsonAbortSequence;
    static std::string comments;
    static std::string currentDirPath;
    static std::string logFileName;
    static std::string lastDir;

    static std::map<std::string, Interpolation> interpolationMap;
    static std::map<int64, std::map<std::string, double[2]>> sensorsNominalRangeTimeMap;
    static std::map<std::string, std::map<int64, double[2]>> sensorsNominalRangeMap;
    static std::map<std::string, std::map<int64, double>> deviceMap;

    static void SetupLogging();

    static void LoadInterpolationMap();
    static bool LoadSequence(json jsonSeq);

    static void LogSensors(int64 microTime, std::vector<double > sensors);
    static void StopGetSensors();
    static void GetSensors(int64 microTime);

    static double GetTimestamp(json obj);
    static void Tick(int64 microTime);

    static void StopAbortSequence();
    static void StartAbortSequence();


    static void plotMaps(uint8 option);

    SequenceManager();

    ~SequenceManager();

public:

    static void init();

    static void AbortSequence(std::string abortMsg="abort");
    static void StopSequence();
    static void StartSequence(json jsonSeq, json jsonAbortSeq, std::string comments);
    static void WritePostSeqComment(std::string msg);

};


#endif //TXV_ECUI_LLSERVER_SEQUENCEMANAGER_H
