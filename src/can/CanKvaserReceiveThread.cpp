//
// Created by raffael on 24.08.25.
//

#include "can/CanKvaserReceiveThread.h"

#include <utility>

CanKvaserReceiveThread::CanKvaserReceiveThread(canRecvCallback_t onRecvCallback): queue(std::make_shared<moodycamel::ReaderWriterQueue<std::unique_ptr<RawKvaserMessage>>>(100)),
    onRecvCallback(std::move(onRecvCallback)) {

    std::thread([this]() { this->receiveLoop(); }).detach();
}

CanKvaserReceiveThread::~CanKvaserReceiveThread() {
    stop();
}

void CanKvaserReceiveThread::stop() {
    running.store(false);
}

void CanKvaserReceiveThread::pushMessage(std::unique_ptr<RawKvaserMessage> msg) {
    queue->enqueue(std::move(msg));
    ++messagesInQueue;
}

bool CanKvaserReceiveThread::isRunning() const {
    return running.load();
}

void CanKvaserReceiveThread::receiveLoop(){
    while (running.load()) {
        std::unique_ptr<RawKvaserMessage> msg;
        if (queue->try_dequeue(msg)) {
            onRecvCallback(msg->busChannelID, msg->messageID, msg->data, msg->dlc, msg->timestamp, msg->driver);
        }
        --messagesInQueue;
        if (messagesInQueue>40) {
            Debug::warning("CanKvaserReceiveThread::receiveLoop: High message load, messages in queue: %d", messagesInQueue.load());
        }
    }
}

