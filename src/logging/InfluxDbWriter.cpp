#include <iostream>

#include "logging/InfluxDbWriter.hpp"

#include <format>
#include <iterator>
#include <string>
#include <utility>

#include "logging/influxDb.h"
#include "utility/Debug.h"
#include "blockingconcurrentqueue.h" // ensure complete type for queue
#include "logging/InfluxDbSendThread.h"

using namespace std::chrono_literals;

InfluxDbWriter::InfluxDbWriter(std::string hostname, unsigned port, std::string dbName,
                               std::size_t bufferSize) : buffer_size_max(bufferSize),cntxt(std::make_shared<influxDbContext>()) {
    host = std::move(hostname);
    db = std::move(dbName);
    portStr = std::to_string(port);

    if (initDbContext(cntxt.get(), host.c_str(), portStr.c_str(), db.c_str()) < 0) {
        throw std::runtime_error("Couldn't initialize influxDbWriter (bad context)");
    }

    queue = std::make_shared<moodycamel::BlockingConcurrentQueue<std::string>>();
    // spawn worker threads referencing *this
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back(std::make_unique<InfluxDbSendThread>(cntxt, queue, *this));
    }
}

InfluxDbWriter::~InfluxDbWriter() {
    flush();
    joinThreads();

    (void) deInitDbContext(cntxt.get());
}

void InfluxDbWriter::setCredentials(const std::string& user, const std::string& password) {
    std::string user_http = "&u=" + user;
    std::string pw_http = "&p=" + password;
    set_credentials(cntxt.get(), user_http.c_str(), pw_http.c_str());
}

void InfluxDbWriter::setTimestampPrecision(timestamp_precision_t precision) {
    set_timestamp_precision(cntxt.get(), precision);
}

void InfluxDbWriter::setMeasurement(std::string _measurement) {
    this->measurement = std::move(_measurement);
}

void InfluxDbWriter::startDataPoint() {
    std::format_to(std::back_inserter(current_buffer), "{}", measurement);
}

void InfluxDbWriter::addTag(const std::string_view key, const std::string_view value) {
    std::format_to(std::back_inserter(current_buffer), ",{}={}", key, value);
}

void InfluxDbWriter::tagsDone() {
    std::format_to(std::back_inserter(current_buffer), " ");
}

// Properly sanitize strings if needed, neglected so far because of the overhead (DB)
void InfluxDbWriter::addField(const std::string_view key, const std::string_view value) {
    std::format_to(std::back_inserter(current_buffer), "{}=\"{}\",", key, value);
}

void InfluxDbWriter::addField(const std::string_view key, const std::size_t value) {
    std::format_to(std::back_inserter(current_buffer), "{}={}i,", key, value);
}

// Might let user select the precision and scientific notation
void InfluxDbWriter::addField(const std::string_view key, const double value) {
    std::format_to(std::back_inserter(current_buffer), "{}={:.6f},", key, value);
}

void InfluxDbWriter::addField(const std::string_view key, const bool value) {
    std::format_to(std::back_inserter(current_buffer), "{}={},", key, value ? "t" : "f");
}

void InfluxDbWriter::endDataPoint(const std::size_t timestamp) {
    // replace the last comma with a space
    if (!current_buffer.empty() && current_buffer.back() == ',') {
        current_buffer.pop_back();
    }
    std::format_to(std::back_inserter(current_buffer), " {}", timestamp);
    if (current_buffer.size() >= buffer_size_max) {
        flush();
    }
}

void InfluxDbWriter::joinThreads() {
    // signal threads to stop and join them
    for (auto & t : threads) {
        if (t) t->stop();
    }
    for (auto & t : threads) {
        if (t) t->join();
    }
}

void InfluxDbWriter::flush() {
    std::lock_guard lock(buffer_mutex);
    if (current_buffer.empty()) {
        Debug::warning("InfluxDbWriter flush called with empty buffer, nothing to push.");
        return;
    }
    if (!queue) {
        Debug::warning("InfluxDbWriter flush called before queue initialization");
        return;
    }

    std::string buf_to_send = std::move(current_buffer);

    // get a new buffer
    if (!available_buffers.empty()) {
        current_buffer = std::move(available_buffers.back());
        available_buffers.pop_back();
    } else {
        current_buffer.clear();
    }

    // enqueue buffer for sending
    queue->enqueue(std::move(buf_to_send));
}

void InfluxDbWriter::returnBuffer(std::string buffer) {
    std::lock_guard thread_lock(buffer_mutex);
    available_buffers.emplace_back(std::move(buffer));
}
