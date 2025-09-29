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
        std::string msg;
        if (queue->wait_dequeue_timed(msg, std::chrono::milliseconds(100))) {
            --messagesInQueue;
            sendData(context.get(), msg.data(), msg.size());
            writer.returnBuffer(std::move(msg));
        }
        if (messagesInQueue>40) {
            Debug::error("Queue size above 40, size: %i", messagesInQueue.load());
        }
    }
}
