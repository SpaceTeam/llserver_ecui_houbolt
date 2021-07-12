#ifndef INFLUXDBLOGGER_H
#define INFLUXDBLOGGER_H

#include "logging/InfluxDbWriter.h"
#include "logging/MessageLogger.h"
#include "logging/DataLogger.h"

class InfluxDbLogger
    : public MessageLogger, public DataLogger{
        public:
            InfluxDbLogger();
            InfluxDbLogger(const InfluxDbLogger&) = delete;
            ~InfluxDbLogger();
            void Init(std::string db_hostname, unsigned db_port, std::string db_name, std::string measurement, timestamp_precision_t precision);

            void log(std::string source, std::string msg, std::size_t timestamp, Severity severity);
            void log(std::string key, std::size_t value, std::size_t timestamp);
            void log(std::string key, double value, std::size_t timestamp);
            void log(std::string key, bool value, std::size_t timestamp);
            //template <typename T> void log(std::string key, T value, std::size_t timestamp);

            void flush();
        private:
            InfluxDbWriter *dbWriter;
};

#endif