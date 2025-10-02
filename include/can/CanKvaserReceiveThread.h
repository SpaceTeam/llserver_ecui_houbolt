//
// Created by raffael on 24.08.25.
//

#ifndef CANRECEIVETHREAD_H
#define CANRECEIVETHREAD_H

#ifndef NO_CANLIB
#include "CommonKvaser.h"
#include <readerwriterqueue.h>
#include <functional>
#include <atomic>
#include "CANDriver.h"


class CanKvaserReceiveThread {
public:
    explicit CanKvaserReceiveThread( canRecvCallback_t onRecvCallback);

    ~CanKvaserReceiveThread();

    void stop();
    void join();
    void pushMessage(std::unique_ptr<RawKvaserMessage> msg);
    [[nodiscard]] bool isRunning() const;

private:
    std::atomic_bool running{false};
    std::atomic_int32_t messagesInQueue{0};
    std::mutex enqueueMutex;
    std::shared_ptr<moodycamel::BlockingReaderWriterQueue<std::unique_ptr<RawKvaserMessage>> > queue;
    canRecvCallback_t onRecvCallback;
    std::thread thread;

    void receiveLoop();
};


#endif

#endif //CANRECEIVETHREAD_H
