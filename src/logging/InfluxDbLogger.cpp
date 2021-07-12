#include "logging/InfluxDbLogger.h"
#include <iostream>

InfluxDbLogger::InfluxDbLogger() {
}

InfluxDbLogger::~InfluxDbLogger() {
    delete dbWriter;
}

void InfluxDbLogger::Init(std::string db_hostname, unsigned db_port, std::string db_name, std::string measurement, timestamp_precision_t precision) {
    try {
        dbWriter = new InfluxDbWriter(db_hostname, db_port, db_name);
        dbWriter->Init();
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