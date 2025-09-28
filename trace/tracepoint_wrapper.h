#pragma once

#include <chrono>
#include <cstdint>
#include <atomic>


// Simple tracepoint macro approach to avoid LTTng include conflicts
#if defined(HAVE_LTTNG) && HAVE_LTTNG

// Forward declare the actual tracepoint functions (defined in tracepoint_impl.cpp)
extern "C" {
    void __tracepoint_llserver___packet_received(uint64_t seqno, uint32_t size_bytes, uint32_t queue_size, uint64_t t_produced_ns);
    void __tracepoint_llserver___process_start(uint64_t seqno, uint64_t queue_delay_ns, uint32_t queue_size, uint64_t t_start_ns);
    void __tracepoint_llserver___process_end(uint64_t seqno, uint64_t processing_ns, uint64_t t_end_ns, uint8_t success);
    void __tracepoint_llserver___lock_wait(const char* mutex_name, uint64_t wait_ns);
    void __tracepoint_llserver___lock_hold(const char* mutex_name, uint64_t hold_ns);
    void __tracepoint_llserver___logging_io(uint32_t bytes, uint64_t io_ns, uint8_t op);
    void __tracepoint_llserver___logging_io_error(uint32_t attempted_bytes, uint64_t io_ns, uint8_t op, int32_t err_no); // new error wrapper
    void __tracepoint_llserver___queue_snapshot(uint32_t queue_size, uint64_t produced_total, uint64_t consumed_total, uint64_t dropped_total, uint64_t t_snapshot_ns);
    void __tracepoint_llserver___backlog_trigger(uint32_t queue_size, int64_t growth_rate_per_sec, uint8_t reason_code, uint64_t t_trigger_ns);
    void __tracepoint_llserver___item_dropped(uint64_t seqno, uint32_t queue_size, uint8_t cause);
    void __tracepoint_llserver___thread_heartbeat(uint8_t thread_role, uint64_t t_beat_ns);
    // New unique network + writer join events
    void __tracepoint_llserver___net_send(uint32_t bytes, uint64_t io_ns);
    void __tracepoint_llserver___net_send_error(uint32_t attempted_bytes, uint64_t io_ns, int32_t err_no);
    void __tracepoint_llserver___net_recv(uint32_t bytes, uint64_t io_ns);
    void __tracepoint_llserver___net_recv_error(uint32_t attempted_bytes, uint64_t io_ns, int32_t err_no);
    void __tracepoint_llserver___writer_join_threads(uint32_t thread_count);
    void __tracepoint_llserver___writer_thread_join(uint64_t join_ns);
}

// Simplified tracepoint macro
#define tracepoint(provider, name, ...) __tracepoint_##provider##___##name(__VA_ARGS__)

#else
#define tracepoint(provider, name, ...) ((void)0)
#endif

namespace llserver {
namespace trace {

// Enumeration values per specification
enum class LoggingOp : uint8_t {
    FILE_WRITE = 0,
    NET_SEND = 1,
    FLUSH = 2,
    ROTATE = 3,
    NET_RECV = 4 // Added receive operation for network reads
};

enum class BacklogReason : uint8_t {
    SIZE_THRESHOLD = 0,
    GROWTH_RATE = 1,
    MANUAL = 2
};

enum class ThreadRole : uint8_t {
    PRODUCER = 0,
    CONSUMER = 1,
    LOGGING = 2,
    MONITOR = 3
};

enum class DropCause : uint8_t {
    QUEUE_FULL = 0,
    PROCESSING_ERROR = 1,
    SHUTDOWN = 2
};

// Static mutex name strings to avoid dynamic allocation
static constexpr const char* MUTEX_LOGGING = "logging";
static constexpr const char* MUTEX_QUEUE = "queue";
static constexpr const char* MUTEX_STATE = "state";
static constexpr const char* MUTEX_CONFIG = "config";
static constexpr const char* MUTEX_SENSOR_BUFFER = "sensor_buffer";
static constexpr const char* MUTEX_FAST_LOGGING = "fast_logging";
static constexpr const char* MUTEX_INFLUX_LOGGER = "influx_logger";
static constexpr const char* MUTEX_DEBUG_OUTPUT = "debug_output";

// Function-specific mutex names for better tracing granularity
static constexpr const char* MUTEX_DEBUG_LOG_OUTFILE = "debug::log:outfile";
static constexpr const char* MUTEX_DEBUG_CLOSE_OUTFILE = "debug::close:outfile";
static constexpr const char* MUTEX_DEBUG_FLUSH_OUTFILE = "debug::flush:outfile";
static constexpr const char* MUTEX_DEBUG_CHANGEFILE_OUTFILE = "debug::changeOutputFile:outfile";
static constexpr const char* MUTEX_NODE_SENSOR_BUFFER_WRITE = "node::ProcessSensorData:sensor_buffer";
static constexpr const char* MUTEX_NODE_SENSOR_BUFFER_READ = "node::GetLatestSensorData:sensor_buffer";
static constexpr const char* MUTEX_NODE_FAST_LOGGING_WRITE = "node::ProcessSensorData:fast_logging";
static constexpr const char* MUTEX_STATE_ADD_UNINIT = "stateController::AddUninitializedStates:state";
static constexpr const char* MUTEX_STATE_ADD_STATES = "stateController::AddStates:state";
static constexpr const char* MUTEX_STATE_GET_STATE = "stateController::GetState:state";
static constexpr const char* MUTEX_STATE_SET_STATE = "stateController::SetState:state";
static constexpr const char* MUTEX_STATE_GET_VALUE = "stateController::GetStateValue:state";
static constexpr const char* MUTEX_STATE_GET_DIRTY = "stateController::GetDirtyStates:state";
static constexpr const char* MUTEX_STATE_GET_ALL = "stateController::GetAllStates:state";

// High-resolution timing helper
inline uint64_t steady_time_ns() {
    return std::chrono::steady_clock::now().time_since_epoch().count();
}

// Lock timing helper class for RAII-style lock instrumentation
template<typename MutexType>
class InstrumentedLock {
private:
    MutexType& mutex_;
    const char* name_;
    uint64_t lock_start_;
    uint64_t hold_start_;

public:
    InstrumentedLock(MutexType& mutex, const char* name)
        : mutex_(mutex), name_(name) {
        lock_start_ = steady_time_ns();
        mutex_.lock();
        hold_start_ = steady_time_ns();

        uint64_t wait_ns = hold_start_ - lock_start_;
        // Emit wait event (always for logging mutex, threshold for others)
        if (name == MUTEX_LOGGING || wait_ns > 50000) { // 50µs threshold
            tracepoint(llserver, lock_wait, name_, wait_ns);
        }
    }

    ~InstrumentedLock() {
        uint64_t hold_end = steady_time_ns();
        uint64_t hold_ns = hold_end - hold_start_;
        tracepoint(llserver, lock_hold, name_, hold_ns);
        mutex_.unlock();
    }

    // Non-copyable, non-movable
    InstrumentedLock(const InstrumentedLock&) = delete;
    InstrumentedLock& operator=(const InstrumentedLock&) = delete;
};

// Helper for conditional emission based on thresholds
class ConditionalTracer {
private:
    static std::atomic<uint32_t> sample_counter_;

public:
    // Sample 1/N events for high-frequency tracepoints
    static bool should_sample(uint32_t rate = 32) {
        return (sample_counter_.fetch_add(1) % rate) == 0;
    }

    // Always sample for critical events
    static bool should_sample_critical() {
        return true;
    }
};

// Processing latency tracker for RAII-style measurement
class ProcessingTracker {
private:
    uint64_t seqno_;
    uint64_t start_time_;
    uint32_t queue_size_;

public:
    ProcessingTracker(uint64_t seqno, uint64_t queue_delay_ns, uint32_t queue_size)
        : seqno_(seqno), queue_size_(queue_size) {
        start_time_ = steady_time_ns();
        tracepoint(llserver, process_start, seqno_, queue_delay_ns, queue_size_, start_time_);
    }

    ~ProcessingTracker() {
        uint64_t end_time = steady_time_ns();
        uint64_t processing_ns = end_time - start_time_;
        tracepoint(llserver, process_end, seqno_, processing_ns, end_time, 1); // success=1
    }

    void mark_failure() {
        uint64_t end_time = steady_time_ns();
        uint64_t processing_ns = end_time - start_time_;
        tracepoint(llserver, process_end, seqno_, processing_ns, end_time, 0); // success=0
    }
};

// I/O timing helper
class IOTracker {
private:
    uint64_t start_time_;
    uint32_t bytes_;
    LoggingOp op_;

public:
    IOTracker(uint32_t bytes, LoggingOp op) : bytes_(bytes), op_(op) {
        start_time_ = steady_time_ns();
    }

    ~IOTracker() {
        uint64_t end_time = steady_time_ns();
        uint64_t io_ns = end_time - start_time_;
        tracepoint(llserver, logging_io, bytes_, io_ns, static_cast<uint8_t>(op_));
    }
};

// Backlog detection helper
class BacklogDetector {
private:
    uint32_t threshold_;
    uint32_t last_queue_size_;
    uint64_t last_check_time_;
    bool armed_;

public:
    BacklogDetector(uint32_t threshold = 1000)
        : threshold_(threshold), last_queue_size_(0), last_check_time_(0), armed_(true) {}

    void check_backlog(uint32_t current_queue_size) {
        uint64_t now = steady_time_ns();

        if (armed_ && current_queue_size > threshold_) {
            int64_t growth_rate = 0;
            if (last_check_time_ > 0) {
                uint64_t dt_ns = now - last_check_time_;
                if (dt_ns > 0) {
                    int64_t dq = static_cast<int64_t>(current_queue_size) - static_cast<int64_t>(last_queue_size_);
                    growth_rate = (dq * 1000000000LL) / static_cast<int64_t>(dt_ns); // items per second
                }
            }

            tracepoint(llserver, backlog_trigger, current_queue_size, growth_rate,
                      static_cast<uint8_t>(BacklogReason::SIZE_THRESHOLD), now);
            armed_ = false; // Disarm until queue drops below half threshold
        }

        // Re-arm if queue has dropped sufficiently
        if (!armed_ && current_queue_size < threshold_ / 2) {
            armed_ = true;
        }

        last_queue_size_ = current_queue_size;
        last_check_time_ = now;
    }
};

// Heartbeat helper
class HeartbeatEmitter {
private:
    ThreadRole role_;
    uint64_t last_beat_;
    uint64_t interval_ns_;

public:
    HeartbeatEmitter(ThreadRole role, uint64_t interval_seconds = 5)
        : role_(role), last_beat_(0), interval_ns_(interval_seconds * 1000000000ULL) {}

    void maybe_emit() {
        uint64_t now = steady_time_ns();
        if (now - last_beat_ >= interval_ns_) {
            tracepoint(llserver, thread_heartbeat, static_cast<uint8_t>(role_), now);
            last_beat_ = now;
        }
    }

    void force_emit() {
        uint64_t now = steady_time_ns();
        tracepoint(llserver, thread_heartbeat, static_cast<uint8_t>(role_), now);
        last_beat_ = now;
    }
};

} // namespace trace
} // namespace llserver

// Convenience macros for common patterns
#define LLSERVER_TRACE_PACKET_RECEIVED(seqno, size_bytes, queue_size) \
    tracepoint(llserver, packet_received, seqno, size_bytes, queue_size, llserver::trace::steady_time_ns())

#define LLSERVER_TRACE_ITEM_DROPPED(seqno, queue_size, cause) \
    tracepoint(llserver, item_dropped, seqno, queue_size, static_cast<uint8_t>(cause))

#define LLSERVER_TRACE_QUEUE_SNAPSHOT(queue_size, produced, consumed, dropped) \
    tracepoint(llserver, queue_snapshot, queue_size, produced, consumed, dropped, llserver::trace::steady_time_ns())

// RAII lock instrumentation macro
#define LLSERVER_INSTRUMENTED_LOCK(mutex, name) \
    llserver::trace::InstrumentedLock<decltype(mutex)> _trace_lock(mutex, name)
