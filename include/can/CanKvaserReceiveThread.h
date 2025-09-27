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
#include <memory>
#include "CANDriver.h"

// Forward declarations for tracing components
namespace llserver { namespace trace {
    class HeartbeatEmitter;
    class BacklogDetector;
}}

class CanKvaserReceiveThread {
public:
    explicit CanKvaserReceiveThread( canRecvCallback_t onRecvCallback);

    ~CanKvaserReceiveThread();

    void stop();
    void pushMessage(std::unique_ptr<RawKvaserMessage> msg);
    [[nodiscard]] bool isRunning() const;

private:
    std::atomic_bool running{false};
    std::atomic_int32_t messagesInQueue{0};
    std::mutex enqueueMutex;
    std::shared_ptr<moodycamel::BlockingReaderWriterQueue<std::unique_ptr<RawKvaserMessage>> > queue;
    canRecvCallback_t onRecvCallback;

    // Tracing components
    std::unique_ptr<llserver::trace::HeartbeatEmitter> heartbeat_emitter;
    std::unique_ptr<llserver::trace::BacklogDetector> backlog_detector;
    std::atomic<uint64_t> sequence_number{0};

    void receiveLoop();
};


#endif

#endif //CANRECEIVETHREAD_H
