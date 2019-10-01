//
// Created by Markus on 2019-09-27.
//

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <mutex>
#include <thread>

#include "Debug.h"

std::recursive_mutex Debug::_outMutex;

#ifdef LLSERVER_DEBUG


int32 Debug::print(std::string fmt, ...)
{
    std::lock_guard<std::recursive_mutex> lock(_outMutex);

    int printed;
    va_list args;

    fmt.append("\n");

    va_start(args, fmt);
    printed = vprintf(fmt.c_str(), args);
    va_end(args);

    return printed;
}

int32 Debug::error(std::string fmt, ...)
{
    int printed;
    va_list args;

    fmt.append("\n");

    va_start(args, fmt);
    printed = vfprintf(stderr, fmt.c_str(), args);
    va_end(args);

    return printed;
}

int32 Debug::info(std::string fmt, ...)
{
    int printed;
    va_list args;

    fmt = "info: " + fmt;
    fmt.append("\n");

    va_start(args, fmt);
    printed = vprintf(fmt.c_str(), args);
    va_end(args);

    return printed;
}

int32 Debug::warning(std::string fmt, ...)
{
    int printed;
    va_list args;

    fmt = "warning: " + fmt;
    fmt.append("\n");

    va_start(args, fmt);
    printed = vfprintf(stderr, fmt.c_str(), args);
    va_end(args);

    return printed;
}

#else

int32 Debug::print(std::string fmt, ...)
{
    return 0;
}

int32 Debug::error(std::string fmt, ...)
{
    return 0;
}

int32 Debug::info(std::string fmt, ...)
{
    return 0;
}

int32 Debug::warning(std::string fmt, ...)
{
    return 0;
}

#endif