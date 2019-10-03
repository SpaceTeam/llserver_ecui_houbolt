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

#include "config.h"
#include "common.h"

class Debug {

private:
    static std::recursive_mutex _outMutex;
    static int32 _lockCount;

public:
    //TODO: implement, only log, no output
    void log(std::string fmt, ...);

    static int32 print(std::string fmt, ...);
    static int32 error(std::string fmt, ...);
    static int32 info(std::string fmt, ...);
    static int32 warning(std::string fmt, ...);

};

#endif //TXV_ECUI_LLSERVER_DEBUG_H
