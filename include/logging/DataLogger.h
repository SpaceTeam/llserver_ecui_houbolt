#ifndef DATALOGGER_H
#define DATALOGGER_H

#include <string>

class DataLogger {
    public:
        virtual void log(std::string key, std::size_t value, std::size_t timestamp) = 0;
        virtual void log(std::string key, double value, std::size_t timestamp) = 0;
        virtual void log(std::string key, bool value, std::size_t timestamp) = 0;
};

#endif