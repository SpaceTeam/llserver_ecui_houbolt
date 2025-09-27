//
// Created by raffael on 24.08.25.
//
#ifndef NO_CANLIB
#include "can/CanKvaserReceiveThread.h"
#include "tracepoint_wrapper.h"

#include <utility>

CanKvaserReceiveThread::CanKvaserReceiveThread(canRecvCallback_t onRecvCallback): queue(std::make_shared<moodycamel::BlockingReaderWriterQueue<std::unique_ptr<RawKvaserMessage>>>(100)),
    onRecvCallback(onRecvCallback) {
    running = true;

    // Initialize tracing components
    heartbeat_emitter = std::make_unique<llserver::trace::HeartbeatEmitter>(llserver::trace::ThreadRole::CONSUMER, 5);
    backlog_detector = std::make_unique<llserver::trace::BacklogDetector>(50); // 50 message threshold
    sequence_number = 0;

    std::thread([this]() { this->receiveLoop(); }).detach();
}

CanKvaserReceiveThread::~CanKvaserReceiveThread() {
    stop();
}

void CanKvaserReceiveThread::stop() {
    running = false;
}

void CanKvaserReceiveThread::pushMessage(std::unique_ptr<RawKvaserMessage> msg) {
    // Instrument the producer side
    {
        LLSERVER_INSTRUMENTED_LOCK(enqueueMutex, llserver::trace::MUTEX_QUEUE);

        uint64_t seqno = ++sequence_number;
        uint32_t queue_size = messagesInQueue.load();
        uint32_t size_bytes = sizeof(RawKvaserMessage) + msg->dlc;

        // Store production timestamp in message for queue delay calculation
        msg->trace_produced_ns = llserver::trace::steady_time_ns();
        msg->trace_seqno = seqno;

        queue->enqueue(std::move(msg));
        ++messagesInQueue;

        // Emit packet_received event
        LLSERVER_TRACE_PACKET_RECEIVED(seqno, size_bytes, queue_size + 1);

        // Check for backlog condition
        backlog_detector->check_backlog(queue_size + 1);
    }
}

bool CanKvaserReceiveThread::isRunning() const {
    return running.load();
}

void CanKvaserReceiveThread::receiveLoop(){
    uint64_t produced_total = 0;
    uint64_t consumed_total = 0;
    uint64_t dropped_total = 0;
    auto last_snapshot = std::chrono::steady_clock::now();

    while (running.load()) {
        // Emit heartbeat periodically
        heartbeat_emitter->maybe_emit();

        std::unique_ptr<RawKvaserMessage> msg;
        if (queue->wait_dequeue_timed(msg,std::chrono::milliseconds(1))) {
            uint32_t current_queue_size = --messagesInQueue;
            consumed_total++;

            // Calculate queue delay
            uint64_t now = llserver::trace::steady_time_ns();
            uint64_t queue_delay_ns = now - msg->trace_produced_ns;

            // Start processing tracking
            llserver::trace::ProcessingTracker processing_tracker(
                msg->trace_seqno, queue_delay_ns, current_queue_size);

            try {
                // Call the actual message processing callback
                onRecvCallback(msg->busChannelID, msg->messageID, msg->data, msg->dlc, msg->timestamp, msg->driver);
                // ProcessingTracker destructor will emit process_end with success=1
            } catch (const std::exception& e) {
                Debug::error("CanKvaserReceiveThread::receiveLoop: Processing failed: %s", e.what());
                processing_tracker.mark_failure();
            }
        }

        if (messagesInQueue > 40) {
            Debug::warning("CanKvaserReceiveThread::receiveLoop: High message load, messages in queue: %d", messagesInQueue.load());
            // Emit backlog trigger if not already triggered by detector
            uint64_t now = llserver::trace::steady_time_ns();
            tracepoint(llserver, backlog_trigger,
                      messagesInQueue.load(),
                      0, // growth_rate unknown here
                      static_cast<uint8_t>(llserver::trace::BacklogReason::SIZE_THRESHOLD),
                      now);
        }

        // Periodic queue snapshot (every 1 second)
        auto current_time = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(current_time - last_snapshot).count() >= 1) {
            LLSERVER_TRACE_QUEUE_SNAPSHOT(messagesInQueue.load(), produced_total, consumed_total, dropped_total);
            last_snapshot = current_time;
        }
    }
}


#endif

