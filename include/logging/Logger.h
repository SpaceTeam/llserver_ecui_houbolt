//
// Created by chris on 9/3/22.
//

#ifndef LLSERVER_ECUI_HOUBOLT_LOGGER_H
#define LLSERVER_ECUI_HOUBOLT_LOGGER_H

#include <string>
#include "control_flag.h"

enum Severity {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARNING = 3,
    ERROR = 4,
    FATAL = 5
};

template<Severity severity>
void log(std::string source, std::string msg);

template<typename Type>
void logSensorValue(std::string key, Type value, std::size_t timestamp);


#endif //LLSERVER_ECUI_HOUBOLT_LOGGER_H
