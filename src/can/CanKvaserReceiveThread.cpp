//
// Created by raffael on 24.08.25.
//
#ifndef NO_CANLIB
#include "can/CanKvaserReceiveThread.h"

#include <utility>

CanKvaserReceiveThread::CanKvaserReceiveThread(canRecvCallback_t onRecvCallback): queue(std::make_shared<moodycamel::BlockingReaderWriterQueue<std::unique_ptr<RawKvaserMessage>>>(100)),
    onRecvCallback(onRecvCallback) {
    running = true;

    thread = std::thread([this]() { this->receiveLoop(); });
}

CanKvaserReceiveThread::~CanKvaserReceiveThread() {
    stop();
    join();
}

void CanKvaserReceiveThread::stop() {
    running = false;
}
void CanKvaserReceiveThread::join() {
    if (thread.joinable()) {
        thread.join();
    }
}

void CanKvaserReceiveThread::pushMessage(std::unique_ptr<RawKvaserMessage> msg) {
    std::lock_guard guard(enqueueMutex);
    queue->enqueue(std::move(msg));
    ++messagesInQueue;
}

bool CanKvaserReceiveThread::isRunning() const {
    return running.load();
}

void CanKvaserReceiveThread::receiveLoop(){
    while (running.load()) {
        std::unique_ptr<RawKvaserMessage> msg;
        if (queue->wait_dequeue_timed(msg,std::chrono::milliseconds(1))) {
            --messagesInQueue;
            onRecvCallback(msg->busChannelID, msg->messageID, msg->data, msg->dlc, msg->timestamp, msg->driver);
        }
        if (messagesInQueue>40) {
            Debug::warning("CanKvaserReceiveThread::receiveLoop: High message load, messages in queue: %d", messagesInQueue.load());
        }
    }
}


#endif