//
// Created by raffael on 29.09.25.
//

#include "logging/InfluxDbSendThread.h"

#include <utility>

#include "utility/Debug.h"
#include "logging/InfluxDbWriter.hpp"

bool InfluxDbSendThread::isRunning() const {
    return running.load();
}

InfluxDbSendThread::InfluxDbSendThread(std::shared_ptr<influxDbContext> context,
                                       std::shared_ptr<moodycamel::BlockingConcurrentQueue<std::string> > queue,
                                       InfluxDbWriter& influx_db_writer)
    : queue(std::move(queue))
    , context(std::move(context))
    , writer(influx_db_writer) {
    running = true;

    thread = std::thread([this]() { this->messageSendingLoop(); });
}

InfluxDbSendThread::~InfluxDbSendThread() {
    stop();
    join();
}

void InfluxDbSendThread::stop() {
    running = false;
}

void InfluxDbSendThread::join() {
    if (thread.joinable()) thread.join();
}

void InfluxDbSendThread::pushBuffer(std::string msg) {
    queue->enqueue(std::move(msg));
    ++messagesInQueue;
}

void InfluxDbSendThread::messageSendingLoop() {
    while (running.load()) {
        std::string buffer;
        if (queue->wait_dequeue_timed(buffer, std::chrono::milliseconds(100))) {
            --messagesInQueue;
            sendData(context.get(), buffer.data(), buffer.size());
            writer.returnBuffer(std::move(buffer));
        }
        int size_approx = queue->size_approx();
        if (size_approx>50) {
            Debug::warning("Queue size above 50, size: %i",size_approx );
        }
    }
}
