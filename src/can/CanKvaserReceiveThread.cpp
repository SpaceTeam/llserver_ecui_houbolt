//
// Created by raffael on 24.08.25.
//
#include "utility/utils.h"
#ifndef NO_CANLIB
#include "can/CanKvaserReceiveThread.h"

#include <utility>

CanKvaserReceiveThread::CanKvaserReceiveThread(canRecvCallback_t onRecvCallback): queue(std::make_shared<moodycamel::BlockingReaderWriterQueue<std::unique_ptr<RawKvaserMessage>>>(100)),
    onRecvCallback(onRecvCallback) {
    running = true;

    std::thread([this]() { this->receiveLoop(); }).detach();
}

CanKvaserReceiveThread::~CanKvaserReceiveThread() {
    stop();
}

void CanKvaserReceiveThread::stop() {
    running = false;
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
            Debug::error("CanKvaserReceiveThread::receiveLoop: High message load, messages in queue: %d", messagesInQueue.load());
        }
        Debug::error("Time offset between messages is %i",utils::getCurrentTimestamp()-msg->timestamp);
    }
}


#endif