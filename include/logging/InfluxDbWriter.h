#ifndef INFLUXDBWRITER_H
#define INFLUXDBWRITER_H

#include <string>
#include <thread>
#include <vector>

extern "C" {
#include "logging/influxDb.h"
}

class InfluxDbWriter {
public:
    // Might want to make the buffer size and concurrency configurable (DB)
    InfluxDbWriter(std::string hostname, unsigned port, std::string dbName, std::size_t buffer_size);

    InfluxDbWriter(const InfluxDbWriter &) = delete;

    ~InfluxDbWriter();

    void Init();

    void setCredentials(std::string user, std::string password);

    void setTimestampPrecision(timestamp_precision_t precision);

    void setMeasurement(std::string measurement);

    void startDataPoint();

    void addTag(std::string_view key, std::string_view value);

    void tagsDone();

    void addField(std::string_view key, std::string_view value);

    void addField(std::string_view key, std::size_t value);

    void addField(std::string_view key, double value);

    void addField(std::string_view key, bool value);

    void endDataPoint(std::size_t timestamp);

    void flush();

private:
    const std::size_t buffer_size_max = 1024;
    // allocations that can be reused as buffers
    std::vector<std::string> available_buffers;
    std::mutex buffer_mutex;
    // the current buffer being filled
    std::string current_buffer;
    influxDbContext cntxt;
    std::string host, portStr, db, measurement;
    std::vector<std::thread> threads;

    void joinThreads();
};

#endif
