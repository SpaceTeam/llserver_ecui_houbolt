#ifndef INFLUXDBWRITER_H
#define INFLUXDBWRITER_H

#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include "blockingconcurrentqueue.h"

extern "C" {
#include "logging/influxDb.h"
}

class InfluxDbSendThread; // forward declaration

class InfluxDbWriter {
    // Might want to make the buffer size and concurrency configurable (DB)
public:
    InfluxDbWriter(std::string hostname, unsigned port, std::string dbName, std::size_t buffer_size);
    InfluxDbWriter(const InfluxDbWriter &) = delete;
    ~InfluxDbWriter();

    void setCredentials(const std::string& user, const std::string& password);
    void setTimestampPrecision(timestamp_precision_t precision) const;
    void setMeasurement(std::string _measurement);

    void startDataPoint();
    void addTag(std::string_view key, std::string_view value);
    void tagsDone();
    void addField(std::string_view key, std::string_view value);
    void addField(std::string_view key, std::size_t value);
    void addField(std::string_view key, double value);
    void addField(std::string_view key, bool value);
    void endDataPoint(std::size_t timestamp);
    void flush();
    void returnBuffer(std::string buffer);

private:
    const std::size_t buffer_size_max = 1024;
    // allocations that can be reused as buffers
    std::vector<std::string> available_buffers;
    std::vector<std::shared_ptr<influxDbContext>> contexts;
    std::mutex buffer_mutex;
    // the current buffer being filled
    std::string current_buffer;
    std::string host, portStr, db, measurement;
    std::vector<std::unique_ptr<InfluxDbSendThread>> threads;
    std::shared_ptr<moodycamel::BlockingConcurrentQueue<std::string>> queue;

    void joinThreads() const;
};

#endif
