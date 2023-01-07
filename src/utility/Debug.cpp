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

#include "utility/Debug.h"

#include "utility/Config.h"

std::recursive_mutex Debug::_outMutex;
std::mutex Debug::outFileMutex;
std::stringstream Debug::logStream;
std::ofstream Debug::logFile;
bool Debug::isLogFileOpen = false;
bool Debug::printWarnings = false;
bool Debug::printInfos = false;
bool Debug::initialized = false;
std::unique_ptr<InfluxDbLogger> Debug::logger = nullptr;

void Debug::Init()  
{
    if (!initialized) {
        printWarnings = std::get<bool>(Config::getData("DEBUG/printWarnings"));
        printInfos = std::get<bool>(Config::getData("DEBUG/printInfos"));
        logger.reset(new InfluxDbLogger());
        logger->Init(std::get<std::string>(Config::getData("INFLUXDB/database_ip")),
                     std::get<int>(Config::getData("INFLUXDB/database_port")),
                     std::get<std::string>(Config::getData("INFLUXDB/database_name")),
                     std::get<std::string>(Config::getData("INFLUXDB/debug_measurement")), MILLISECONDS,
                     std::get<int>(Config::getData("INFLUXDB/buffer_size")));
        initialized = true;
    }
}

std::string Debug::getTimeString()
{
    tm * curr_tm;
    char time_string[100];

    struct timespec curr_time_struct;
    clock_gettime(CLOCK_REALTIME, &curr_time_struct);

    curr_tm = localtime(&curr_time_struct.tv_sec);

    strftime(time_string, 100, "%T.", curr_tm);

    return "<" + std::string(time_string) + std::to_string(curr_time_struct.tv_nsec/1000000) + ">\t";
}

std::size_t Debug::getTime() {
    struct timespec curr_time_struct;
    clock_gettime(CLOCK_REALTIME, &curr_time_struct);

    return curr_time_struct.tv_sec * 1000 + curr_time_struct.tv_nsec / 1000000;
}

int32_t Debug::printNoTime(std::string fmt, ...)
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

int32_t Debug::print(std::string fmt, ...)
{
    std::lock_guard<std::recursive_mutex> lock(_outMutex);

    int printed;
    va_list args;
    char msg[1024];
    std::size_t time_ms = getTime();
    std::string time_str = getTimeString();

    va_start(args, fmt);
    printed = vsnprintf(msg, 1024, fmt.c_str(), args);
    if(printed == 1024) msg[printed-1] = '\0';
    else msg[printed] = '\0';
    va_end(args);

    if(initialized) {
        //logger->log("Class:Debug", msg, time_ms, DEBUG);
    }

    printed = fprintf(stderr, "%s %s\n", time_str.c_str(), msg);

    return printed;
}

int32_t Debug::info(std::string fmt, ...)
{
    if (printInfos)
    {
        std::lock_guard<std::recursive_mutex> lock(_outMutex);
        int printed;
        va_list args;
        char msg[1024];
        std::size_t time_ms = getTime();
        std::string time_str = getTimeString();

        va_start(args, fmt);
        printed = vsnprintf(msg, 1024, fmt.c_str(), args);
        if(printed == 1024) msg[printed-1] = '\0';
        else msg[printed] = '\0';
        va_end(args);

        if(initialized) {
            //logger->log("Class:Debug", msg, time_ms, INFO);
        }

        printed = fprintf(stderr, "%sinfo: %s\n", time_str.c_str(), msg);

        return printed;
    }

	return 0;
}

int32_t Debug::warning(std::string fmt, ...)
{
    if (printWarnings)
    {
        std::lock_guard<std::recursive_mutex> lock(_outMutex);
        int printed;
        va_list args;
        char msg[1024];
        std::size_t time_ms = getTime();
        std::string time_str = getTimeString();

        va_start(args, fmt);
        printed = vsnprintf(msg, 1024, fmt.c_str(), args);
        if(printed == 1024) msg[printed-1] = '\0';
        else msg[printed] = '\0';
        va_end(args);

        if(initialized) {
            //logger->log("Class:Debug", msg, time_ms, WARNING);
        }

        printed = fprintf(stderr, "%swarning: %s\n", time_str.c_str(), msg);

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
        info("in Debug close: log output file is not open yet, try Debug::changeOutputFile");
    }
    if (initialized) {
        logger->flush();
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

    if (initialized) {
        logger->flush();
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
    if(initialized) {
        //TODO: MP this makes no sense here, since it is only used to write to files in csv format
        // logger->log("Class:Debug", msg, getTime(), DEBUG); //FIXME causes crash sometimes?
    }

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

int32_t Debug::error(std::string fmt, ...)
{
    std::lock_guard<std::recursive_mutex> lock(_outMutex);
    int printed;
    va_list args;
    char msg[1024];
    std::size_t time_ms = getTime();
    std::string time_str = getTimeString();

    va_start(args, fmt);
    printed = vsnprintf(msg, 1024, fmt.c_str(), args);
    if(printed == 1024) msg[printed-1] = '\0';
    else msg[printed] = '\0';
    va_end(args);

    if(initialized) {
        //logger->log("Class:Debug", msg, time_ms, ERROR);
    }

    printed = fprintf(stderr, "%serror: %s\n", time_str.c_str(), msg);
    
    return printed;
}
