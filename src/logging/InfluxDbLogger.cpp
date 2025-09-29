#include "logging/InfluxDbLogger.h"
#include <iostream>

InfluxDbLogger::InfluxDbLogger() {
}

InfluxDbLogger::~InfluxDbLogger() {
}

void InfluxDbLogger::Init(std::string db_hostname, unsigned db_port, std::string db_name, std::string measurement,
                            timestamp_precision_t precision, std::size_t buffer_size) {
    try {
        dbWriter = std::make_shared<InfluxDbWriter>(db_hostname, db_port, db_name, buffer_size);
        dbWriter->setMeasurement(measurement);
        dbWriter->setTimestampPrecision(precision);
    }
    catch (const std::runtime_error &e) {
        throw std::runtime_error("Couldn't initialize InfluxDbLogger: " + std::string(e.what()));   
    }
}

void InfluxDbLogger::log(std::string source, std::string msg, std::size_t timestamp, Severity severity) {
    dbWriter->startDataPoint();
    dbWriter->addTag("source", source);
    dbWriter->addTag("severity", severityToString(severity));
    dbWriter->tagsDone();
    dbWriter->addField("msg", msg);
    dbWriter->endDataPoint(timestamp);
}

void InfluxDbLogger::log(const std::string &key, const std::string &msg, const std::size_t timestamp) const {
    dbWriter->startDataPoint();
    dbWriter->addTag("key", key);
    dbWriter->tagsDone();
    dbWriter->addField("msg", msg);
    dbWriter->endDataPoint(timestamp);
}

void InfluxDbLogger::log(std::string key, std::size_t value, std::size_t timestamp){
    dbWriter->startDataPoint();
    dbWriter->addTag("key", key);
    dbWriter->tagsDone();
    dbWriter->addField("value", value);
    dbWriter->endDataPoint(timestamp);
}

void InfluxDbLogger::log(std::string key, double value, std::size_t timestamp){
    dbWriter->startDataPoint();
    dbWriter->addTag("key", key);
    dbWriter->tagsDone();
    dbWriter->addField("value", value);
    dbWriter->endDataPoint(timestamp);
}

void InfluxDbLogger::log(std::string key, bool value, std::size_t timestamp){
    dbWriter->startDataPoint();
    dbWriter->addTag("key", key);
    dbWriter->tagsDone();
    dbWriter->addField("value", value);
    dbWriter->endDataPoint(timestamp);
}
/*template <typename T> void InfluxDbLogger::log(std::string key, T value, std::size_t timestamp){
    dbWriter->startDataPoint();
    dbWriter->addTag("key", key);
    dbWriter->tagsDone();
    dbWriter->addField("value", value);
    dbWriter->endDataPoint(timestamp);
}*/

void InfluxDbLogger::flush() {
    dbWriter->flush();
}