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

#include "Config.h"

std::recursive_mutex Debug::_outMutex;
std::mutex Debug::outFileMutex;
std::stringstream Debug::logStream;
std::ofstream Debug::logFile;
bool Debug::isLogFileOpen = false;
bool Debug::printWarnings = false;
bool Debug::printInfos = false;

void Debug::Init()
{
    printWarnings = std::get<bool>(Config::getData("DEBUG/printWarnings"));
    printInfos = std::get<bool>(Config::getData("DEBUG/printInfos"));
}

std::string Debug::getTimeString()
{
    tm * curr_tm;
    char time_string[100];

    struct timespec curr_time_struct;
    clock_gettime(CLOCK_MONOTONIC_RAW, &curr_time_struct);

    curr_tm = localtime(&curr_time_struct.tv_sec);

    strftime(time_string, 100, "%T.", curr_tm);

    return "<" + std::string(time_string) + std::to_string(curr_time_struct.tv_nsec/1000000) + ">\t";
}

int32 Debug::printNoTime(std::string fmt, ...)
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

int32 Debug::print(std::string fmt, ...)
{
    std::lock_guard<std::recursive_mutex> lock(_outMutex);

    int printed;
    va_list args;

    fmt = fmt.insert(0, getTimeString());
    fmt.append("\n");

    va_start(args, fmt);
    printed = vprintf(fmt.c_str(), args);
    va_end(args);

    return printed;
}

int32 Debug::info(std::string fmt, ...)
{
    if (printInfos)
    {
        int printed;
        va_list args;

        fmt = getTimeString() + "info: " + fmt;
        fmt.append("\n");

        va_start(args, fmt);
        printed = vprintf(fmt.c_str(), args);
        va_end(args);

        return printed;
    }

	return 0;
}

int32 Debug::warning(std::string fmt, ...)
{
    if (printWarnings)
    {
        int printed;
        va_list args;

        fmt = getTimeString() + "warning: " + fmt;
        fmt.append("\n");

        va_start(args, fmt);
        printed = vfprintf(stderr, fmt.c_str(), args);
        va_end(args);

        return printed;
    }

    return 0;
}

void Debug::close()
{
    std::lock_guard<std::mutex> lock(outFileMutex);
    if (logFile.is_open())
    {
        logFile << logStream.str();
        logFile.flush();
        logFile.close();
    }
    else
    {
        error("in Debug close: log output file is not open yet, try Debug::changeOutputFile");
    }
}

void Debug::flush()
{
    std::lock_guard<std::mutex> lock(outFileMutex);
    if (logFile.is_open())
    {
        writeToFile();
        logFile.flush();
    }
    else
    {
        error("in Debug flush: log output file is not open yet, try Debug::changeOutputFile");
    }
}

void Debug::changeOutputFile(std::string outFilePath)
{
    std::lock_guard<std::mutex> lock(outFileMutex);
    if (logFile.is_open())
    {
        writeToFile();
        logFile.flush();
        logFile.close();
        isLogFileOpen = false;
    }

    logFile.open(outFilePath);
    if (logFile.is_open())
    {
        isLogFileOpen = true;
    }
    else
    {
        error("log output file failed to open");
    }
}

void Debug::log(std::string msg)
{
    std::lock_guard<std::mutex> lock(outFileMutex);
    logStream << msg;

}

void Debug::writeToFile()
{
    if (logFile.is_open())
    {
        if (logFile.bad())
        {
            error("write before: bad!!!!!!");
        }
        if (logFile.fail())
        {
            error("write before: failed!!!!!!");
        }
        if (logFile.eof())
        {
            error("write before: eof!!!!!!");
        }
        logFile << logStream.str();
        logStream.str("");
        if (logFile.bad())
        {
            error("write bad!!!!!!");
        }
        if (logFile.fail())
        {
            error("write failed!!!!!!");
        }
        if (logFile.eof())
        {
            error("write eof!!!!!!");
        }
    }
    else
    {
        error("in log: log output file is not open yet, try Debug::changeOutputFile");
    }
}

int32 Debug::error(std::string fmt, ...)
{
    int printed;
    va_list args;

    fmt = getTimeString() + "error: " + fmt;
    fmt.append("\n");

    va_start(args, fmt);
    printed = vfprintf(stderr, fmt.c_str(), args);
    va_end(args);

    return printed;
}
