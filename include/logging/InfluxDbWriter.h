#ifndef INFLUXDBWRITER_H
#define INFLUXDBWRITER_H
#define BUFFER_SIZE 1024 * 2

#include <string>
#include <thread>
#include <vector>

extern "C" {
    #include "logging/influxDb.h"
}

class InfluxDbWriter {
    public:
        // Might want to make the buffer size and concurrency configurable (DB)
        InfluxDbWriter(std::string hostname, unsigned port, std::string dbName);
        InfluxDbWriter(const InfluxDbWriter&) = delete;
        ~InfluxDbWriter();
        void Init();
        void setCredentials(std::string user, std::string password);
        void setTimestampPrecision(timestamp_precision_t precision);

        void setMeasurement(std::string measurement);
        void startDataPoint();
        void addTag(std::string key, std::string value);
        void tagsDone();
        void addField(std::string key, std::string value);
        void addField(std::string key, std::size_t value);
        void addField(std::string key, double value);
        void addField(std::string key, bool value);
        void endDataPoint(std::size_t timestamp);

        void flush();
    private:
        char buffer[2][BUFFER_SIZE];
        influxDbContext cntxt;
        uint8_t buffer_sel = 0;
        std::size_t buffer_pos = 0;
        std::size_t last_measurement = 0;
        std::string host, portStr, db, measurement;
        void pushBuffer();
        std::vector<std::thread> threads;
        void joinThreads();
        void push();
        void transferPartialWrite();
};

#endif