//
// Created by Markus on 2019-09-27.
//

#ifndef TXV_ECUI_LLSERVER_DEBUG_H
#define TXV_ECUI_LLSERVER_DEBUG_H

#include <mutex>
#include <string>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <thread>
#include <fstream>

#include "config.h"
#include "common.h"

class Debug {

private:
    static std::recursive_mutex _outMutex;
    static std::mutex outFileMutex;
    static std::ofstream logFile;
    static bool isLogFileOpen;

public:
    //TODO: implement, only log, no output
    static void flush();
    static void changeOutputFile(std::string outFilePath);
    static void log(std::string msg);

    static int32 print(std::string fmt, ...);
    static int32 error(std::string fmt, ...);
    static int32 info(std::string fmt, ...);
    static int32 warning(std::string fmt, ...);

};

#endif //TXV_ECUI_LLSERVER_DEBUG_H
