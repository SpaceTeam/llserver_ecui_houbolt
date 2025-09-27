# LTTng-UST Tracing for llserver

This document provides usage instructions for the LTTng-UST tracing implementation in llserver, designed to diagnose intermittent backlog spikes and identify root causes of performance issues.

## Quick Start

### Prerequisites

Install LTTng-UST development packages:
```bash
# Ubuntu/Debian
sudo apt-get install liblttng-ust-dev lttng-tools

# RHEL/CentOS/Fedora
sudo dnf install lttng-ust-devel lttng-tools
```

### Building with Tracing Support

```bash
# Enable tracing (default)
cmake -DENABLE_LTTNG=ON ..
make

# Disable tracing
cmake -DENABLE_LTTNG=OFF ..
make
```

### Basic Trace Collection

```bash
# Start a tracing session
lttng create llserver_session --output=./traces
lttng enable-event -u 'llserver:*'
lttng add-context -u -t vtid -t procname
lttng start

# Run your workload until backlog occurs
./llserver_ecui_houbolt

# Stop and view traces
lttng stop
lttng view | head -100
lttng destroy
```

## Event Reference

### Core Events

| Event | Purpose | Key Fields |
|-------|---------|------------|
| `llserver:packet_received` | Producer enqueue | seqno, queue_size, t_produced_ns |
| `llserver:process_start` | Consumer dequeue | seqno, queue_delay_ns, queue_size |
| `llserver:process_end` | Processing complete | seqno, processing_ns, success |
| `llserver:lock_wait` | Mutex contention | mutex_name, wait_ns |
| `llserver:lock_hold` | Critical section | mutex_name, hold_ns |
| `llserver:logging_io` | Blocking I/O | bytes, io_ns, op |
| `llserver:queue_snapshot` | Periodic status | queue_size, produced_total, consumed_total |
| `llserver:backlog_trigger` | Backlog detection | queue_size, growth_rate_per_sec, reason_code |
| `llserver:item_dropped` | Item loss | seqno, queue_size, cause |
| `llserver:thread_heartbeat` | Thread liveness | thread_role, t_beat_ns |

### Field Enumerations

**logging_io.op:**
- 0 = file_write
- 1 = net_send  
- 2 = flush
- 3 = rotate

**backlog_trigger.reason_code:**
- 0 = size_threshold
- 1 = growth_rate
- 2 = manual

**thread_heartbeat.thread_role:**
- 0 = producer
- 1 = consumer
- 2 = logging
- 3 = monitor

## Advanced Usage

### Selective Event Filtering

```bash
# Only lock waits > 100µs
lttng enable-event -u llserver:lock_wait --filter 'wait_ns > 100000'

# Processing latency tail events only  
lttng enable-event -u llserver:process_end --filter 'processing_ns > 500000'

# Large queue sizes only
lttng enable-event -u llserver:queue_snapshot --filter 'queue_size > 1000'

# I/O operations taking > 1ms
lttng enable-event -u llserver:logging_io --filter 'io_ns > 1000000'
```

### Focused Backlog Investigation

```bash
# Create session for backlog analysis
lttng create backlog_session --output=./backlog_traces

# Enable core events with contexts
lttng enable-event -u 'llserver:packet_received'
lttng enable-event -u 'llserver:process_start'  
lttng enable-event -u 'llserver:process_end'
lttng enable-event -u 'llserver:backlog_trigger'
lttng enable-event -u 'llserver:queue_snapshot'

# Add timing contexts
lttng add-context -u -t vtid -t procname -t cpu_id

lttng start

# Run until backlog occurs, then stop immediately
# ./llserver_ecui_houbolt
# ... wait for backlog trigger ...

lttng stop
lttng view
lttng destroy
```

### Lock Contention Analysis

```bash
lttng create lock_session --output=./lock_traces

# Focus on lock events with thresholds
lttng enable-event -u llserver:lock_wait --filter 'wait_ns > 50000'
lttng enable-event -u llserver:lock_hold --filter 'hold_ns > 100000'

# Add logging I/O to correlate  
lttng enable-event -u llserver:logging_io

lttng add-context -u -t vtid -t procname
lttng start

# Run workload
lttng stop && lttng view && lttng destroy
```

## Trace Analysis

### Key Metrics to Calculate

1. **Producer vs Consumer Rate:**
   ```bash
   # Count events over time windows
   babeltrace traces/ | grep "packet_received" | wc -l
   babeltrace traces/ | grep "process_end" | wc -l
   ```

2. **Queue Growth Rate:**
   ```bash
   # Extract queue_size from queue_snapshot events
   babeltrace traces/ | grep "queue_snapshot" | \
     awk '{print $1, $NF}' | grep "queue_size"
   ```

3. **Processing Latency Distribution:**
   ```bash
   # Extract processing_ns values
   babeltrace traces/ | grep "process_end" | \
     grep -o "processing_ns = [0-9]*" | cut -d' ' -f3
   ```

4. **Lock Wait Times:**
   ```bash
   # Lock contention by mutex
   babeltrace traces/ | grep "lock_wait" | \
     grep -o "mutex_name = \"[^\"]*\".*wait_ns = [0-9]*"
   ```

### Correlation Analysis

Look for these patterns around backlog events:

1. **Consumer Stall:** `process_end` rate drops while `packet_received` continues
2. **Lock Contention:** `lock_wait` events spike before backlog  
3. **I/O Blocking:** `logging_io` events with high `io_ns` values
4. **Processing Slowdown:** `processing_ns` values increase over time

## Performance Impact

- **Tracing Disabled:** Zero overhead (macros expand to `((void)0)`)
- **Tracing Enabled:** ~50-200ns per event (depends on system)
- **Recommended:** Use filtering for high-frequency events in production

## Troubleshooting

### Build Issues

```bash
# Check if LTTng-UST is found
cmake .. 2>&1 | grep -i lttng

# Verify tracepoints are compiled in
nm ./llserver_ecui_houbolt | grep lttng_ust_tracepoint_provider_llserver
```

### Runtime Issues

```bash
# Check if tracepoints are registered
lttng list -u

# Verify events are being generated
lttng create test --output=/tmp/test_traces
lttng enable-event -u 'llserver:*'
lttng start
# Run briefly
lttng stop
lttng view | head -10
lttng destroy
```

### No Events Generated

1. Ensure `HAVE_LTTNG=1` is defined during compilation
2. Check that tracepoints library is linked
3. Verify LTTng session daemon is running: `lttng-sessiond --daemonize`

## Example Analysis Scripts

### Queue Growth Rate Calculator

```bash
#!/bin/bash
# calculate_growth_rate.sh

babeltrace "$1" | grep "queue_snapshot" | \
  awk '{
    match($0, /queue_size = ([0-9]+)/, qs);
    match($0, /t_snapshot_ns = ([0-9]+)/, ts);
    if (NR > 1) {
      dt = (ts[1] - prev_t) / 1e9;
      dq = qs[1] - prev_q;
      if (dt > 0) print ts[1], qs[1], dq/dt;
    }
    prev_q = qs[1]; prev_t = ts[1];
  }'
```

### Lock Contention Summary

```bash
#!/bin/bash
# lock_summary.sh

babeltrace "$1" | grep "lock_wait" | \
  awk '{
    match($0, /mutex_name = "([^"]*)"/, mutex);
    match($0, /wait_ns = ([0-9]+)/, wait);
    waits[mutex[1]] += wait[1];
    counts[mutex[1]]++;
  }
  END {
    for (m in waits) {
      printf "%s: %d events, %.2fms total, %.2fµs avg\n", 
        m, counts[m], waits[m]/1e6, waits[m]/counts[m]/1000;
    }
  }'
```

## Integration with Monitoring

Consider correlating LTTng events with:
- System metrics (CPU, memory, I/O)
- Application logs  
- InfluxDB metrics already collected by llserver
- Off-CPU analysis tools (eBPF)

For automated backlog detection, monitor for `backlog_trigger` events in your alerting system.
