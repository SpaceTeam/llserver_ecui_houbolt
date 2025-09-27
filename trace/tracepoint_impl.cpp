#define TRACEPOINT_CREATE_PROBES
#define TRACEPOINT_DEFINE
#include "tracepoint.tp"

// Implement the wrapper functions that forward to real LTTng tracepoints
extern "C" {
    void __tracepoint_llserver___packet_received(uint64_t seqno, uint32_t size_bytes, uint32_t queue_size, uint64_t t_produced_ns) {
        tracepoint(llserver, packet_received, seqno, size_bytes, queue_size, t_produced_ns);
    }

    void __tracepoint_llserver___process_start(uint64_t seqno, uint64_t queue_delay_ns, uint32_t queue_size, uint64_t t_start_ns) {
        tracepoint(llserver, process_start, seqno, queue_delay_ns, queue_size, t_start_ns);
    }

    void __tracepoint_llserver___process_end(uint64_t seqno, uint64_t processing_ns, uint64_t t_end_ns, uint8_t success) {
        tracepoint(llserver, process_end, seqno, processing_ns, t_end_ns, success);
    }

    void __tracepoint_llserver___lock_wait(const char* mutex_name, uint64_t wait_ns) {
        tracepoint(llserver, lock_wait, mutex_name, wait_ns);
    }

    void __tracepoint_llserver___lock_hold(const char* mutex_name, uint64_t hold_ns) {
        tracepoint(llserver, lock_hold, mutex_name, hold_ns);
    }

    void __tracepoint_llserver___logging_io(uint32_t bytes, uint64_t io_ns, uint8_t op) {
        tracepoint(llserver, logging_io, bytes, io_ns, op);
    }

    void __tracepoint_llserver___queue_snapshot(uint32_t queue_size, uint64_t produced_total, uint64_t consumed_total, uint64_t dropped_total, uint64_t t_snapshot_ns) {
        tracepoint(llserver, queue_snapshot, queue_size, produced_total, consumed_total, dropped_total, t_snapshot_ns);
    }

    void __tracepoint_llserver___backlog_trigger(uint32_t queue_size, int64_t growth_rate_per_sec, uint8_t reason_code, uint64_t t_trigger_ns) {
        tracepoint(llserver, backlog_trigger, queue_size, growth_rate_per_sec, reason_code, t_trigger_ns);
    }

    void __tracepoint_llserver___item_dropped(uint64_t seqno, uint32_t queue_size, uint8_t cause) {
        tracepoint(llserver, item_dropped, seqno, queue_size, cause);
    }

    void __tracepoint_llserver___thread_heartbeat(uint8_t thread_role, uint64_t t_beat_ns) {
        tracepoint(llserver, thread_heartbeat, thread_role, t_beat_ns);
    }
}
