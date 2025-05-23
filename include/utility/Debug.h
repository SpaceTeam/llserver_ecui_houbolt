//
// Created by Markus on 2019-09-27.
//

#ifndef TXV_ECUI_LLSERVER_DEBUG_H
#define TXV_ECUI_LLSERVER_DEBUG_H

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>

#include "common.h"
#include "utility/Config.h"

#include "logging/InfluxDbLogger.h"

class Debug {

private:
    static std::recursive_mutex _outMutex;
    static std::mutex outFileMutex;
    static std::stringstream logStream;
    static std::ofstream logFile;
    static bool isLogFileOpen;

    static bool printWarnings;
    static bool printInfos;

    //NOTE: NOT THREAD SAFE
    static void writeToFile();

    static std::string getTimeString();
    static std::size_t getTime();

	static bool initialized;

    static std::unique_ptr<InfluxDbLogger> logger;

public:
    static void Init(Config &config);

    static void close();
    static void flush();
    static void changeOutputFile(const std::string &outFilePath);
    static void log(const std::string &msg);

    static int32_t print(const std::string &fmt, ...);
    static int32_t printNoTime(std::string fmt, ...);
    static int32_t error(const std::string &fmt, ...);
    static int32_t info(const std::string &fmt, ...);
    static int32_t warning(const std::string& fmt, ...);

};

#endif //TXV_ECUI_LLSERVER_DEBUG_H
