#!/usr/bin/env python3
"""
LTTng Trace Analysis for llserver Backlog Diagnosis

This script parses LTTng traces and generates plots to analyze:
- Queue size over time
- Processing latencies and queue delays
- Lock contention patterns
- I/O operation durations
- Producer vs consumer rates

Usage:
    python3 analyze_traces.py /path/to/trace/directory
"""

import sys
import os
import json
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from datetime import datetime
from collections import defaultdict
import bt2
import pickle
import argparse

class LLServerTraceAnalyzer:
    def __init__(self, trace_path, debug=False, cache_events=False, legends_outside=True, legend_mode='below'):
        self.trace_path = trace_path
        self.debug = debug
        self.cache_events = cache_events
        self.events = []
        self.queue_snapshots = []
        self.processing_events = defaultdict(dict)
        self.lock_events = []
        self.io_events = []
        self.backlog_triggers = []
        self.heartbeat_events = []  # Store thread_heartbeat events
        self.dropped_events = []    # Store item_dropped events
        self.field_names = set()  # Collect all unique field names
        self.legends_outside = legends_outside
        self.legend_mode = legend_mode  # 'right', 'below', 'inside'

    def debug_log(self, msg):
        if self.debug:
            print(f"[DEBUG] {msg}")

    def parse_traces(self):
        """Parse LTTng traces using babeltrace2"""
        print(f"Parsing traces from: {self.trace_path}")

        # Create a trace collection and add the trace
        tc = bt2.TraceCollectionMessageIterator([self.trace_path])


        event_count = 0
        for msg in tc:
            if isinstance(msg, bt2._EventMessageConst):
                self._process_event(msg.event,msg.default_clock_snapshot.ns_from_origin)
                event_count += 1
                if event_count % 10000 == 0:
                    print(f"Processed {event_count} events...")
                if event_count > 500000 and False:
                    print("Reached 500,000 events, stopping for demo purposes")
                    break
            else:
                pass
                #print(f"Skipping non-event message: {msg} with type {type(msg)}")

        print(f"Finished parsing {event_count} total events")
        print(f"  thread_heartbeat events: {len(self.heartbeat_events)}")
        print(f"  item_dropped events: {len(self.dropped_events)}")

    def _to_primitive(self, v):
        """Best-effort convert babeltrace field objects (SwigPyObject) to plain Python primitives."""
        if isinstance(v, (int, float, str, bool)) or v is None:
            return v
        # Try common attribute 'value'
        for attr in ('value', 'py', 'as_integer'):  # heuristic
            if hasattr(v, attr):
                try:
                    candidate = getattr(v, attr)
                    if callable(candidate):
                        candidate = candidate()
                    if isinstance(candidate, (int, float, str, bool)):
                        return candidate
                except Exception:
                    pass
        # Try int conversion
        try:
            return int(v)
        except Exception:
            pass
        # Try float conversion
        try:
            return float(v)
        except Exception:
            pass
        # Fallback to string (last resort)
        try:
            s = str(v)
            # Attempt parse numeric from string
            if s.isdigit():
                return int(s)
            return s
        except Exception:
            return None

    def _sanitize_structures(self):
        """Convert all stored data structures to primitives to allow pickling and numeric ops."""
        def sanitize_list(lst):
            for d in lst:
                for k, val in list(d.items()):
                    d[k] = self._to_primitive(val)
        sanitize_list(self.queue_snapshots)
        sanitize_list(self.lock_events)
        sanitize_list(self.io_events)
        sanitize_list(self.backlog_triggers)
        sanitize_list(self.heartbeat_events)
        sanitize_list(self.dropped_events)
        # processing_events dict of seqno -> dict
        for seq, d in list(self.processing_events.items()):
            for k, val in list(d.items()):
                d[k] = self._to_primitive(val)
        if self.debug:
            self.debug_log("Sanitization complete for all data structures")

    def _process_event(self, event, timestamp):
        """Process individual LTTng events"""
        event_name = event.name

        # Extract common fields
        fields = {}
        for field_name, field_value in event.payload_field.items():
            fields[field_name] = self._to_primitive(field_value)
        # Collect all field names
        self.field_names.update(fields.keys())

        # Store all events for general analysis
        self.events.append({
            'timestamp': timestamp,
            'name': event_name,
            'fields': fields
        })

        # Process specific event types
        if event_name == 'llserver:packet_received':
            self._process_packet_received(timestamp, fields)
        elif event_name == 'llserver:process_start':
            self._process_process_start(timestamp, fields)
        elif event_name == 'llserver:process_end':
            self._process_process_end(timestamp, fields)
        elif event_name == 'llserver:queue_snapshot':
            self._process_queue_snapshot(timestamp, fields)
        elif event_name == 'llserver:lock_wait':
            self._process_lock_wait(timestamp, fields)
        elif event_name == 'llserver:lock_hold':
            self._process_lock_hold(timestamp, fields)
        elif event_name == 'llserver:logging_io':
            self._process_logging_io(timestamp, fields)
        elif event_name == 'llserver:backlog_trigger':
            self._process_backlog_trigger(timestamp, fields)
        elif event_name == 'llserver:thread_heartbeat':
            self._process_thread_heartbeat(timestamp, fields)
        elif event_name == 'llserver:item_dropped':
            self._process_item_dropped(timestamp, fields)
        else:
            print(f"Unknown event type: {event_name}")

    def _process_packet_received(self, timestamp, fields):
        """Process packet_received events"""
        seqno = fields.get('seqno', 0)
        self.processing_events[seqno]['received_time'] = timestamp
        self.processing_events[seqno]['queue_size_at_receive'] = fields.get('queue_size', 0)
        self.processing_events[seqno]['size_bytes'] = fields.get('size_bytes', 0)

    def _process_process_start(self, timestamp, fields):
        """Process process_start events"""
        seqno = fields.get('seqno', 0)
        self.processing_events[seqno]['start_time'] = timestamp
        self.processing_events[seqno]['queue_delay_ns'] = fields.get('queue_delay_ns', 0)
        self.processing_events[seqno]['queue_size_at_start'] = fields.get('queue_size', 0)

    def _process_process_end(self, timestamp, fields):
        """Process process_end events"""
        seqno = fields.get('seqno', 0)
        self.processing_events[seqno]['end_time'] = timestamp
        self.processing_events[seqno]['processing_ns'] = fields.get('processing_ns', 0)
        self.processing_events[seqno]['success'] = fields.get('success', 1)

    def _process_queue_snapshot(self, timestamp, fields):
        """Process queue_snapshot events"""
        self.queue_snapshots.append({
            'timestamp': timestamp,
            'queue_size': fields.get('queue_size', 0),
            'produced_total': fields.get('produced_total', 0),
            'consumed_total': fields.get('consumed_total', 0),
            'dropped_total': fields.get('dropped_total', 0)
        })

    def _process_lock_wait(self, timestamp, fields):
        """Process lock_wait events"""
        self.lock_events.append({
            'timestamp': timestamp,
            'type': 'wait',
            'mutex_name': fields.get('mutex_name', 'unknown'),
            'duration_ns': fields.get('wait_ns', 0)
        })

    def _process_lock_hold(self, timestamp, fields):
        """Process lock_hold events"""
        self.lock_events.append({
            'timestamp': timestamp,
            'type': 'hold',
            'mutex_name': fields.get('mutex_name', 'unknown'),
            'duration_ns': fields.get('hold_ns', 0)
        })

    def _process_logging_io(self, timestamp, fields):
        """Process logging_io events"""
        op_types = {0: 'file_write', 1: 'net_send', 2: 'flush', 3: 'rotate'}
        self.io_events.append({
            'timestamp': timestamp,
            'bytes': fields.get('bytes', 0),
            'duration_ns': fields.get('io_ns', 0),
            'operation': op_types.get(fields.get('op', 0), 'unknown')
        })

    def _process_backlog_trigger(self, timestamp, fields):
        """Process backlog_trigger events"""
        reason_codes = {0: 'size_threshold', 1: 'growth_rate', 2: 'manual'}
        self.backlog_triggers.append({
            'timestamp': timestamp,
            'queue_size': fields.get('queue_size', 0),
            'growth_rate': fields.get('growth_rate_per_sec', 0),
            'reason': reason_codes.get(fields.get('reason_code', 0), 'unknown')
        })

    def _process_thread_heartbeat(self, timestamp, fields):
        """Process thread_heartbeat events"""
        self.heartbeat_events.append({
            'timestamp': timestamp,
            'thread_role': fields.get('thread_role', 0),
            't_beat_ns': fields.get('t_beat_ns', 0)
        })

    def _process_item_dropped(self, timestamp, fields):
        """Process item_dropped events"""
        self.dropped_events.append({
            'timestamp': timestamp,
            'seqno': fields.get('seqno', 0),
            'queue_size': fields.get('queue_size', 0),
            'cause': fields.get('cause', 0)
        })

    def save_cache(self, cache_file):
        """Serialize all arrays to a cache file."""
        self._sanitize_structures()
        data = {
            # Optionally include raw events (sanitized) if requested; otherwise skip to reduce size & avoid non-primitive data.
            'events': self.events if self.cache_events else None,
            'queue_snapshots': self.queue_snapshots,
            'processing_events': dict(self.processing_events),
            'lock_events': self.lock_events,
            'io_events': self.io_events,
            'backlog_triggers': self.backlog_triggers,
            'heartbeat_events': self.heartbeat_events,
            'dropped_events': self.dropped_events,
            'field_names': list(self.field_names),
            'version': 2,
            'cache_events': self.cache_events,
        }
        with open(cache_file, 'wb') as f:
            pickle.dump(data, f, protocol=pickle.HIGHEST_PROTOCOL)
        print(f"Serialized parsed data to {cache_file} (events included={self.cache_events})")

    def load_cache(self, cache_file):
        """Load all arrays from a cache file."""
        with open(cache_file, 'rb') as f:
            data = pickle.load(f)
        # Backward compatibility handling
        self.queue_snapshots = data.get('queue_snapshots', [])
        self.processing_events = defaultdict(dict, data.get('processing_events', {}))
        self.lock_events = data.get('lock_events', [])
        self.io_events = data.get('io_events', [])
        self.backlog_triggers = data.get('backlog_triggers', [])
        self.heartbeat_events = data.get('heartbeat_events', [])
        self.dropped_events = data.get('dropped_events', [])
        self.field_names = set(data.get('field_names', []))
        if data.get('events') and self.cache_events:
            self.events = data['events']
        print(f"Loaded parsed data from {cache_file} (cached events present={data.get('events') is not None})")

    def generate_plots(self, output_dir='plots'):
        """Generate all analysis plots"""
        os.makedirs(output_dir, exist_ok=True)
        print(f"Generating plots in {output_dir}/")

        # Set up matplotlib for better plots
        plt.style.use('seaborn-v0_8' if 'seaborn-v0_8' in plt.style.available else 'default')

        self.plot_queue_size_over_time(output_dir)
        self.plot_processing_latencies(output_dir)
        self.plot_queue_delays(output_dir)
        self.plot_lock_contention(output_dir)
        self.plot_io_performance(output_dir)
        self.plot_producer_consumer_rates(output_dir)
        self.plot_backlog_analysis(output_dir)

        print("All plots generated successfully!")

    def _coerce_numeric(self, df, columns):
        """Ensure listed columns are numeric, coercing errors to NaN. Provides debug details on coercion losses."""
        for c in columns:
            if c in df.columns:
                orig = df[c]
                # Keep copy only if debug
                if self.debug:
                    orig_non_null = orig[~orig.isna()].copy()
                df[c] = pd.to_numeric(df[c], errors='coerce')
                if self.debug:
                    coerced = df[c]
                    # Values that became NaN during coercion (were not NaN before)
                    loss_mask = (~orig_non_null.index.isin(orig[orig.isna()].index)) & (coerced.isna().reindex(orig_non_null.index, fill_value=True))
                    # Simpler: identify indices where orig not NaN and coerced is NaN
                    loss_mask = (~orig.isna()) & (coerced.isna())
                    loss_count = int(loss_mask.sum())
                    if loss_count > 0:
                        samples = orig[loss_mask].head(5).tolist()
                        self.debug_log(f"Column '{c}': {loss_count} values could not be coerced to numeric. Sample problematic values: {samples}")
        return df

    def _drop_non_finite(self, df, columns):
        """Replace inf with NaN then drop rows where any of columns are non-finite. Debug logs rows removed and columns causing removal."""
        before = len(df)
        df = df.replace([np.inf, -np.inf], np.nan)
        if self.debug:
            inf_counts = {c: int((~np.isfinite(df[c])).sum()) for c in columns if c in df.columns}
            problematic = {k: v for k, v in inf_counts.items() if v > 0}
            if problematic:
                self.debug_log(f"Non-finite counts before drop: {problematic}")
        # Build combined mask
        combined_mask = np.ones(len(df), dtype=bool)
        for c in columns:
            if c in df.columns:
                colmask = np.isfinite(df[c])
                combined_mask &= colmask
        df = df[combined_mask]
        after = len(df)
        removed = before - after
        if self.debug:
            self.debug_log(f"Filtered non-finite rows: removed {removed} / {before} ({(removed / before * 100) if before else 0:.2f}%) based on columns {columns}")
        return df

    def plot_queue_size_over_time(self, output_dir):
        """Plot queue size over time to identify backlog patterns"""
        if not self.queue_snapshots:
            print("No queue snapshot data available")
            return

        df = pd.DataFrame(self.queue_snapshots)
        df['time_seconds'] = (df['timestamp'] - df['timestamp'].min()) / 1e9

        df = self._coerce_numeric(df, ['queue_size','produced_total','consumed_total','dropped_total','time_seconds'])
        df = self._drop_non_finite(df, ['queue_size'])

        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(15, 10))

        # Queue size over time
        ax1.plot(df['time_seconds'], df['queue_size'], 'b-', linewidth=1)
        ax1.set_ylabel('Queue Size')
        ax1.set_title('Queue Size Over Time')
        ax1.grid(True, alpha=0.3)

        # Mark backlog trigger points
        if self.backlog_triggers:
            trigger_df = pd.DataFrame(self.backlog_triggers)
            trigger_df['time_seconds'] = (trigger_df['timestamp'] - df['timestamp'].min()) / 1e9
            ax1.scatter(trigger_df['time_seconds'], trigger_df['queue_size'],
                       color='red', s=50, marker='x', label='Backlog Triggers')
            ax1.legend(loc="upper right")

        # Producer vs Consumer rate
        ax2.plot(df['time_seconds'], df['produced_total'], 'g-', label='Produced Total', linewidth=2)
        ax2.plot(df['time_seconds'], df['consumed_total'], 'b-', label='Consumed Total', linewidth=2)
        if df['dropped_total'].max() > 0:
            ax2.plot(df['time_seconds'], df['dropped_total'], 'r-', label='Dropped Total', linewidth=2)
        ax2.set_ylabel('Total Count')
        ax2.set_xlabel('Time (seconds)')
        ax2.set_title('Producer vs Consumer Totals')
        ax2.legend(loc="upper right")
        ax2.grid(True, alpha=0.3)

        plt.tight_layout()
        plt.savefig(f'{output_dir}/queue_analysis.png', dpi=300, bbox_inches='tight')
        plt.close()

    def plot_processing_latencies(self, output_dir):
        """Plot processing latency distributions and trends"""
        # Convert processing events to dataframe
        processing_data = []
        for seqno, data in self.processing_events.items():
            if 'start_time' in data and 'end_time' in data and 'processing_ns' in data:
                processing_data.append({
                    'seqno': seqno,
                    'start_time': data['start_time'],
                    'processing_ns': data['processing_ns'],
                    'queue_delay_ns': data.get('queue_delay_ns', 0),
                    'success': data.get('success', 1)
                })

        if not processing_data:
            print("No processing latency data available")
            return

        df = pd.DataFrame(processing_data)
        df['time_seconds'] = (df['start_time'] - df['start_time'].min()) / 1e9
        df['processing_ms'] = df['processing_ns'] / 1e6
        df['queue_delay_ms'] = df['queue_delay_ns'] / 1e6

        df = self._coerce_numeric(df, ['processing_ms','queue_delay_ms','time_seconds'])
        df = self._drop_non_finite(df, ['processing_ms','queue_delay_ms'])
        if df.empty:
            print('Processing latency dataframe empty after filtering non-finite values; skipping plot.')
            return

        fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(18, 12))

        # Processing latency over time
        ax1.scatter(df['time_seconds'], df['processing_ms'], alpha=0.6, s=1)
        ax1.set_xlabel('Time (seconds)')
        ax1.set_ylabel('Processing Latency (ms)')
        ax1.set_title('Processing Latency Over Time')
        ax1.grid(True, alpha=0.3)

        # Processing latency histogram
        ax2.hist(df['processing_ms'], bins=50, alpha=0.7, edgecolor='black')
        ax2.set_xlabel('Processing Latency (ms)')
        ax2.set_ylabel('Count')
        ax2.set_title('Processing Latency Distribution')
        ax2.axvline(df['processing_ms'].mean(), color='red', linestyle='--',
                   label=f'Mean: {df["processing_ms"].mean():.2f}ms')
        ax2.axvline(df['processing_ms'].quantile(0.95), color='orange', linestyle='--',
                   label=f'95th percentile: {df["processing_ms"].quantile(0.95):.2f}ms')
        ax2.legend(loc="upper right")
        ax2.grid(True, alpha=0.3)

        # Queue delay over time
        ax3.scatter(df['time_seconds'], df['queue_delay_ms'], alpha=0.6, s=1, color='orange')
        ax3.set_xlabel('Time (seconds)')
        ax3.set_ylabel('Queue Delay (ms)')
        ax3.set_title('Queue Delay Over Time')
        ax3.grid(True, alpha=0.3)

        # Queue delay histogram
        ax4.hist(df['queue_delay_ms'], bins=50, alpha=0.7, edgecolor='black', color='orange')
        ax4.set_xlabel('Queue Delay (ms)')
        ax4.set_ylabel('Count')
        ax4.set_title('Queue Delay Distribution')
        ax4.axvline(df['queue_delay_ms'].mean(), color='red', linestyle='--',
                   label=f'Mean: {df["queue_delay_ms"].mean():.2f}ms')
        ax4.axvline(df['queue_delay_ms'].quantile(0.95), color='darkred', linestyle='--',
                   label=f'95th percentile: {df["queue_delay_ms"].quantile(0.95):.2f}ms')
        ax4.legend(loc="upper right")
        ax4.grid(True, alpha=0.3)

        # Place legends for axes with legend content
        self._place_legend(ax2)
        self._place_legend(ax4)

        plt.tight_layout()
        plt.savefig(f'{output_dir}/processing_latencies.png', dpi=300, bbox_inches='tight')
        plt.close()

    def plot_queue_delays(self, output_dir):
        """Plot detailed queue delay analysis"""
        processing_data = []
        for seqno, data in self.processing_events.items():
            if 'queue_delay_ns' in data and 'start_time' in data:
                processing_data.append({
                    'start_time': data['start_time'],
                    'queue_delay_ns': data['queue_delay_ns']
                })

        if not processing_data:
            print("No queue delay data available")
            return

        df = pd.DataFrame(processing_data)
        df['time_seconds'] = (df['start_time'] - df['start_time'].min()) / 1e9
        df['queue_delay_ms'] = df['queue_delay_ns'] / 1e6

        df = self._coerce_numeric(df, ['queue_delay_ms','time_seconds'])
        df = self._drop_non_finite(df, ['queue_delay_ms'])
        if df.empty:
            print('Queue delay dataframe empty after filtering non-finite values; skipping plot.')
            return

        # Calculate moving averages
        window_size = max(len(df) // 100, 10)  # 1% of data points or minimum 10
        df = df.sort_values('time_seconds')
        df['queue_delay_ma'] = df['queue_delay_ms'].rolling(window=window_size, center=True).mean()

        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(15, 10))

        # Queue delay over time with moving average
        ax1.scatter(df['time_seconds'], df['queue_delay_ms'], alpha=0.3, s=1, label='Individual delays')
        ax1.plot(df['time_seconds'], df['queue_delay_ma'], 'r-', linewidth=2,
                label=f'Moving Average (window={window_size})')
        ax1.set_xlabel('Time (seconds)')
        ax1.set_ylabel('Queue Delay (ms)')
        ax1.set_title('Queue Delay Analysis Over Time')
        ax1.legend(loc="upper right")
        ax1.grid(True, alpha=0.3)

        # Percentile analysis over time
        time_bins = np.linspace(df['time_seconds'].min(), df['time_seconds'].max(), 50)
        percentiles = []

        for i in range(len(time_bins) - 1):
            mask = (df['time_seconds'] >= time_bins[i]) & (df['time_seconds'] < time_bins[i + 1])
            subset = df[mask]['queue_delay_ms']
            if len(subset) > 5:  # Only calculate if we have enough samples
                percentiles.append({
                    'time': (time_bins[i] + time_bins[i + 1]) / 2,
                    'p50': subset.quantile(0.5),
                    'p95': subset.quantile(0.95),
                    'p99': subset.quantile(0.99)
                })

        if percentiles:
            perc_df = pd.DataFrame(percentiles)
            ax2.plot(perc_df['time'], perc_df['p50'], 'g-', label='50th percentile', linewidth=2)
            ax2.plot(perc_df['time'], perc_df['p95'], 'orange', label='95th percentile', linewidth=2)
            ax2.plot(perc_df['time'], perc_df['p99'], 'r-', label='99th percentile', linewidth=2)
            ax2.set_xlabel('Time (seconds)')
            ax2.set_ylabel('Queue Delay (ms)')
            ax2.set_title('Queue Delay Percentiles Over Time')
            ax2.legend(loc="upper right")
            ax2.grid(True, alpha=0.3)

        # Place legends last
        self._place_legend(ax1)
        self._place_legend(ax2)

        plt.tight_layout()
        plt.savefig(f'{output_dir}/queue_delays_detailed.png', dpi=300, bbox_inches='tight')
        plt.close()

    def plot_lock_contention(self, output_dir):
        """Plot lock contention analysis"""
        if not self.lock_events:
            print("No lock event data available")
            return

        df = pd.DataFrame(self.lock_events)
        df['time_seconds'] = (df['timestamp'] - df['timestamp'].min()) / 1e9
        df['duration_ms'] = df['duration_ns'] / 1e6

        df = self._coerce_numeric(df, ['duration_ms','time_seconds'])
        df = self._drop_non_finite(df, ['duration_ms'])
        if df.empty:
            print('Lock events dataframe empty after filtering non-finite values; skipping plot.')
            return

        fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(18, 12))

        # Lock wait times by mutex
        wait_events = df[df['type'] == 'wait']
        if not wait_events.empty:
            mutex_names = wait_events['mutex_name'].unique()
            for mutex in mutex_names:
                mutex_data = wait_events[wait_events['mutex_name'] == mutex]
                ax1.scatter(mutex_data['time_seconds'], mutex_data['duration_ms'],
                           alpha=0.6, s=10, label=mutex)
            ax1.set_xlabel('Time (seconds)')
            ax1.set_ylabel('Lock Wait Time (ms)')
            ax1.set_title('Lock Wait Times Over Time')
            ax1.set_yscale('log')
            #ax1.legend(loc="upper right")
            #ax1.grid(True, alpha=0.3)

            # Lock wait time distribution by mutex
            for mutex in mutex_names:
                mutex_data = wait_events[wait_events['mutex_name'] == mutex]
                ax2.hist(mutex_data['duration_ms'], bins=30, alpha=0.7,
                        label=f'{mutex} (mean: {mutex_data["duration_ms"].mean():.2f}ms)')
            ax2.set_xlabel('Lock Wait Time (ms)')
            ax2.set_ylabel('Count')
            ax2.set_title('Lock Wait Time Distribution by Mutex')
            ax2.set_xscale('log')
            #ax2.legend(loc="upper right")
            #ax2.grid(True, alpha=0.3)

        # Lock hold times by mutex
        hold_events = df[df['type'] == 'hold']
        if not hold_events.empty:
            mutex_names = hold_events['mutex_name'].unique()
            for mutex in mutex_names:
                mutex_data = hold_events[hold_events['mutex_name'] == mutex]
                ax3.scatter(mutex_data['time_seconds'], mutex_data['duration_ms'],
                           alpha=0.6, s=10, label=mutex)
            ax3.set_xlabel('Time (seconds)')
            ax3.set_ylabel('Lock Hold Time (ms)')
            ax3.set_title('Lock Hold Times Over Time')
            ax3.set_yscale('log')
            #ax3.legend(loc="upper right")
            #ax3.grid(True, alpha=0.3)

            # Average lock metrics per mutex
            mutex_stats = []
            for mutex in mutex_names:
                wait_data = wait_events[wait_events['mutex_name'] == mutex]['duration_ms']
                hold_data = hold_events[hold_events['mutex_name'] == mutex]['duration_ms']
                mutex_stats.append({
                    'mutex': mutex,
                    'avg_wait_ms': wait_data.mean() if len(wait_data) > 0 else 0,
                    'avg_hold_ms': hold_data.mean() if len(hold_data) > 0 else 0,
                    'wait_count': len(wait_data),
                    'hold_count': len(hold_data)
                })

            stats_df = pd.DataFrame(mutex_stats)
            x = np.arange(len(stats_df))
            width = 0.35

            ax4.bar(x - width/2, stats_df['avg_wait_ms'], width, label='Avg Wait Time', alpha=0.7)
            ax4.bar(x + width/2, stats_df['avg_hold_ms'], width, label='Avg Hold Time', alpha=0.7)
            ax4.set_xlabel('Mutex')
            ax4.set_ylabel('Average Time (ms)')
            ax4.set_title('Average Lock Wait vs Hold Times by Mutex')
            ax4.set_xticks(x)
            ax4.set_xticklabels(stats_df['mutex'], rotation=45)
            ax4.legend(loc="upper right")
            ax4.grid(True, alpha=0.3)

        # Place legends last
        if 'ax1' in locals(): self._place_legend(ax1)
        if 'ax2' in locals(): self._place_legend(ax2)
        if 'ax3' in locals(): self._place_legend(ax3)
        if 'ax4' in locals(): self._place_legend(ax4)

        plt.tight_layout()
        plt.savefig(f'{output_dir}/lock_contention.png', dpi=300, bbox_inches='tight')
        plt.close()

    def plot_io_performance(self, output_dir):
        """Plot I/O performance analysis"""
        if not self.io_events:
            print("No I/O event data available")
            return

        df = pd.DataFrame(self.io_events)
        df['time_seconds'] = (df['timestamp'] - df['timestamp'].min()) / 1e9
        df['duration_ms'] = df['duration_ns'] / 1e6

        df = self._coerce_numeric(df, ['bytes','duration_ns','duration_ms'])
        df['throughput_mbps'] = (df['bytes'] * 8) / (df['duration_ns'] / 1e9) / 1e6
        df = self._coerce_numeric(df, ['throughput_mbps','time_seconds'])
        df = self._drop_non_finite(df, ['duration_ms'])

        fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(18, 12))

        # I/O duration over time by operation type
        operations = df['operation'].unique()
        for op in operations:
            op_data = df[df['operation'] == op]
            ax1.scatter(op_data['time_seconds'], op_data['duration_ms'],
                       alpha=0.6, s=10, label=op)
        ax1.set_xlabel('Time (seconds)')
        ax1.set_ylabel('I/O Duration (ms)')
        ax1.set_title('I/O Operation Duration Over Time')
        ax1.set_yscale('log')
        #ax1.legend(loc="upper right")
        #ax1.grid(True, alpha=0.3)

        # I/O duration distribution by operation
        for op in operations:
            op_data = df[df['operation'] == op]
            ax2.hist(op_data['duration_ms'], bins=30, alpha=0.7,
                    label=f'{op} (mean: {op_data["duration_ms"].mean():.2f}ms)')
        ax2.set_xlabel('I/O Duration (ms)')
        ax2.set_ylabel('Count')
        ax2.set_title('I/O Duration Distribution by Operation')
        ax2.set_xscale('log')
        #ax2.legend(loc="upper right")
        #ax2.grid(True, alpha=0.3)

        # Bytes vs duration scatter
        ax3.scatter(df['bytes'], df['duration_ms'], alpha=0.6, s=10)
        ax3.set_xlabel('Bytes')
        ax3.set_ylabel('I/O Duration (ms)')
        ax3.set_title('I/O Duration vs Bytes Transferred')
        ax3.set_xscale('log')
        ax3.set_yscale('log')
        ax3.grid(True, alpha=0.3)

        # I/O throughput over time
        df['throughput_mbps'] = (df['bytes'] * 8) / (df['duration_ns'] / 1e9) / 1e6  # Mbps
        df['throughput_mbps'] = pd.to_numeric(df['throughput_mbps'], errors='coerce')
        df = df[np.isfinite(df['throughput_mbps'])]

        if not df.empty:
            for op in operations:
                op_data = df[df['operation'] == op]
                if not op_data.empty:
                    ax4.scatter(op_data['time_seconds'], op_data['throughput_mbps'],
                               alpha=0.6, s=10, label=op)
            ax4.set_xlabel('Time (seconds)')
            ax4.set_ylabel('Throughput (Mbps)')
            ax4.set_title('I/O Throughput Over Time')
            ax4.set_yscale('log')
            #ax4.legend(loc="upper right")
            #ax4.grid(True, alpha=0.3)

        # Place legends for axes with legend content
        self._place_legend(ax1)
        self._place_legend(ax2)
        if 'ax4' in locals(): self._place_legend(ax4)

        plt.tight_layout()
        plt.savefig(f'{output_dir}/io_performance.png', dpi=300, bbox_inches='tight')
        plt.close()

    def plot_producer_consumer_rates(self, output_dir):
        """Plot producer vs consumer rate analysis"""
        if not self.queue_snapshots:
            print("No queue snapshot data for rate analysis")
            return

        df = pd.DataFrame(self.queue_snapshots)
        df['time_seconds'] = (df['timestamp'] - df['timestamp'].min()) / 1e9

        # Calculate rates (derivatives)
        df = df.sort_values('time_seconds')
        df['producer_rate'] = df['produced_total'].diff() / df['time_seconds'].diff()
        df['consumer_rate'] = df['consumed_total'].diff() / df['time_seconds'].diff()
        df['drop_rate'] = df['dropped_total'].diff() / df['time_seconds'].diff()

        # Remove infinite/NaN values
        df = df.dropna()
        # Ensure numeric types before np.isfinite
        df['producer_rate'] = pd.to_numeric(df['producer_rate'], errors='coerce')
        df['consumer_rate'] = pd.to_numeric(df['consumer_rate'], errors='coerce')
        df['drop_rate'] = pd.to_numeric(df['drop_rate'], errors='coerce')
        df['queue_size'] = pd.to_numeric(df['queue_size'], errors='coerce')
        mask = np.isfinite(df['producer_rate']) & np.isfinite(df['consumer_rate'])
        df = df[mask]

        df = self._coerce_numeric(df, ['producer_rate','consumer_rate','drop_rate','queue_size','time_seconds'])
        df = self._drop_non_finite(df, ['producer_rate','consumer_rate'])
        if df.empty:
            print('Producer/consumer rate dataframe empty after filtering; skipping plot.')
            return

        fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(18, 12))

        # Producer vs Consumer rates over time
        ax1.plot(df['time_seconds'], df['producer_rate'], 'g-', label='Producer Rate', linewidth=2)
        ax1.plot(df['time_seconds'], df['consumer_rate'], 'b-', label='Consumer Rate', linewidth=2)
        if df['drop_rate'].max() > 0:
            ax1.plot(df['time_seconds'], df['drop_rate'], 'r-', label='Drop Rate', linewidth=2)
        ax1.set_xlabel('Time (seconds)')
        ax1.set_ylabel('Rate (items/second)')
        ax1.set_title('Producer vs Consumer Rates Over Time')
        ax1.grid(True, alpha=0.3)

        # Rate difference (producer - consumer)
        df['rate_diff'] = df['producer_rate'] - df['consumer_rate']
        ax2.plot(df['time_seconds'], df['rate_diff'], 'purple', linewidth=2)
        ax2.axhline(y=0, color='black', linestyle='--', alpha=0.5)
        ax2.set_xlabel('Time (seconds)')
        ax2.set_ylabel('Rate Difference (producer - consumer)')
        ax2.set_title('Producer - Consumer Rate Difference')
        ax2.grid(True, alpha=0.3)

        # Queue size vs rate difference
        ax3.scatter(df['rate_diff'], df['queue_size'], alpha=0.6, s=20)
        ax3.set_xlabel('Rate Difference (producer - consumer)')
        ax3.set_ylabel('Queue Size')
        ax3.set_title('Queue Size vs Rate Difference')
        ax3.grid(True, alpha=0.3)

        # Moving average comparison
        window_size = max(len(df) // 50, 5)
        df['producer_rate_ma'] = df['producer_rate'].rolling(window=window_size, center=True).mean()
        df['consumer_rate_ma'] = df['consumer_rate'].rolling(window=window_size, center=True).mean()

        ax4.plot(df['time_seconds'], df['producer_rate_ma'], 'g-',
                label=f'Producer Rate (MA-{window_size})', linewidth=3)
        ax4.plot(df['time_seconds'], df['consumer_rate_ma'], 'b-',
                label=f'Consumer Rate (MA-{window_size})', linewidth=3)
        ax4.set_xlabel('Time (seconds)')
        ax4.set_ylabel('Rate (items/second)')
        ax4.set_title('Smoothed Producer vs Consumer Rates')
        ax4.grid(True, alpha=0.3)

        # Place legends for axes with legend content
        self._place_legend(ax1)
        self._place_legend(ax4)

        plt.tight_layout()
        plt.savefig(f'{output_dir}/producer_consumer_rates.png', dpi=300, bbox_inches='tight')
        plt.close()

    def plot_backlog_analysis(self, output_dir):
        """Plot backlog episode analysis"""
        if not self.backlog_triggers:
            print("No backlog trigger data available")
            return

        fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(18, 12))

        trigger_df = pd.DataFrame(self.backlog_triggers)
        trigger_df['time_seconds'] = (trigger_df['timestamp'] - trigger_df['timestamp'].min()) / 1e9

        trigger_df = self._coerce_numeric(trigger_df, ['queue_size','growth_rate','time_seconds'])
        trigger_df = self._drop_non_finite(trigger_df, ['queue_size','growth_rate'])
        if trigger_df.empty:
            print('Backlog trigger dataframe empty after filtering non-finite values; skipping backlog plots.')
            return

        # Backlog episodes over time
        ax1.scatter(trigger_df['time_seconds'], trigger_df['queue_size'],
                   c=trigger_df['growth_rate'], s=60, alpha=0.7, cmap='viridis')
        cbar = plt.colorbar(ax1.collections[0], ax=ax1)
        cbar.set_label('Growth Rate (items/sec)')
        ax1.set_xlabel('Time (seconds)')
        ax1.set_ylabel('Queue Size at Trigger')
        ax1.set_title('Backlog Episodes: Queue Size vs Growth Rate')
        ax1.grid(True, alpha=0.3)

        # Growth rate histogram
        ax2.hist(trigger_df['growth_rate'], bins=20, alpha=0.7, edgecolor='black')
        ax2.set_xlabel('Growth Rate (items/sec)')
        ax2.set_ylabel('Count')
        ax2.set_title('Backlog Growth Rate Distribution')
        ax2.axvline(trigger_df['growth_rate'].mean(), color='red', linestyle='--',
                   label=f'Mean: {trigger_df["growth_rate"].mean():.0f}')
        ax2.legend(loc="upper right")
        ax2.grid(True, alpha=0.3)

        # Queue size at trigger histogram
        ax3.hist(trigger_df['queue_size'], bins=20, alpha=0.7, edgecolor='black', color='orange')
        ax3.set_xlabel('Queue Size at Trigger')
        ax3.set_ylabel('Count')
        ax3.set_title('Queue Size at Backlog Trigger Distribution')
        ax3.axvline(trigger_df['queue_size'].mean(), color='red', linestyle='--',
                   label=f'Mean: {trigger_df["queue_size"].mean():.0f}')
        ax3.legend(loc="upper right")
        ax3.grid(True, alpha=0.3)

        # Trigger reasons
        reason_counts = trigger_df['reason'].value_counts()
        ax4.pie(reason_counts.values, labels=reason_counts.index, autopct='%1.1f%%')
        ax4.set_title('Backlog Trigger Reasons')

        # Place legends for axes with legend content
        self._place_legend(ax2)
        self._place_legend(ax3)

        plt.tight_layout()
        plt.savefig(f'{output_dir}/backlog_analysis.png', dpi=300, bbox_inches='tight')
        plt.close()

    def generate_summary_report(self, output_dir='plots'):
        """Generate a summary report with key statistics"""
        report_file = f'{output_dir}/analysis_summary.txt'

        with open(report_file, 'w') as f:
            f.write("LLServer Trace Analysis Summary Report\n")
            f.write("=" * 50 + "\n\n")

            f.write(f"Total events processed: {len(self.events)}\n")
            f.write(f"Analysis duration: {(max(e['timestamp'] for e in self.events) - min(e['timestamp'] for e in self.events)) / 1e9:.2f} seconds\n\n")

            # Processing statistics
            processing_data = [data for data in self.processing_events.values()
                             if 'processing_ns' in data and 'queue_delay_ns' in data]
            if processing_data:
                processing_times = [d['processing_ns'] / 1e6 for d in processing_data]  # ms
                queue_delays = [d['queue_delay_ns'] / 1e6 for d in processing_data]  # ms

                f.write("Processing Latency Statistics:\n")
                f.write(f"  Mean processing time: {np.mean(processing_times):.2f} ms\n")
                f.write(f"  95th percentile: {np.percentile(processing_times, 95):.2f} ms\n")
                f.write(f"  99th percentile: {np.percentile(processing_times, 99):.2f} ms\n")
                f.write(f"  Max processing time: {np.max(processing_times):.2f} ms\n\n")

                f.write("Queue Delay Statistics:\n")
                f.write(f"  Mean queue delay: {np.mean(queue_delays):.2f} ms\n")
                f.write(f"  95th percentile: {np.percentile(queue_delays, 95):.2f} ms\n")
                f.write(f"  99th percentile: {np.percentile(queue_delays, 99):.2f} ms\n")
                f.write(f"  Max queue delay: {np.max(queue_delays):.2f} ms\n\n")

            # Queue statistics
            if self.queue_snapshots:
                queue_sizes = [s['queue_size'] for s in self.queue_snapshots]
                f.write("Queue Size Statistics:\n")
                f.write(f"  Mean queue size: {np.mean(queue_sizes):.1f}\n")
                f.write(f"  Max queue size: {np.max(queue_sizes)}\n")
                f.write(f"  95th percentile: {np.percentile(queue_sizes, 95):.1f}\n\n")

            # Lock statistics
            if self.lock_events:
                wait_times = [e['duration_ns'] / 1e6 for e in self.lock_events if e['type'] == 'wait']
                hold_times = [e['duration_ns'] / 1e6 for e in self.lock_events if e['type'] == 'hold']

                if wait_times:
                    f.write("Lock Wait Time Statistics:\n")
                    f.write(f"  Mean wait time: {np.mean(wait_times):.2f} ms\n")
                    f.write(f"  95th percentile: {np.percentile(wait_times, 95):.2f} ms\n")
                    f.write(f"  Max wait time: {np.max(wait_times):.2f} ms\n\n")

                if hold_times:
                    f.write("Lock Hold Time Statistics:\n")
                    f.write(f"  Mean hold time: {np.mean(hold_times):.2f} ms\n")
                    f.write(f"  95th percentile: {np.percentile(hold_times, 95):.2f} ms\n")
                    f.write(f"  Max hold time: {np.max(hold_times):.2f} ms\n\n")

            # I/O statistics
            if self.io_events:
                io_times = [e['duration_ns'] / 1e6 for e in self.io_events]
                f.write("I/O Operation Statistics:\n")
                f.write(f"  Mean I/O time: {np.mean(io_times):.2f} ms\n")
                f.write(f"  95th percentile: {np.percentile(io_times, 95):.2f} ms\n")
                f.write(f"  Max I/O time: {np.max(io_times):.2f} ms\n\n")

            # Backlog statistics
            if self.backlog_triggers:
                f.write("Backlog Episode Statistics:\n")
                f.write(f"  Total backlog episodes: {len(self.backlog_triggers)}\n")
                growth_rates = [t['growth_rate'] for t in self.backlog_triggers]
                f.write(f"  Mean growth rate: {np.mean(growth_rates):.0f} items/sec\n")
                f.write(f"  Max growth rate: {np.max(growth_rates):.0f} items/sec\n\n")

        print(f"Summary report written to {report_file}")

    def _place_legend(self, ax, loc="upper right"):
        # Updated flexible legend handling
        leg_handles, leg_labels = ax.get_legend_handles_labels()
        if not leg_handles:
            return
        mode = self.legend_mode
        if mode == 'right':
            ax.legend(leg_handles, leg_labels, loc='center left', bbox_to_anchor=(1.02, 0.5),
                      borderaxespad=0., frameon=False, fontsize='small')
        elif mode == 'below':
            ax.legend(leg_handles, leg_labels, loc='upper center', bbox_to_anchor=(0.5, -0.18),
                      ncol=min(4, len(leg_labels)), frameon=False, fontsize='small')
        else:  # inside
            ax.legend(loc=loc, fontsize='small', frameon=False)

def main():
    parser = argparse.ArgumentParser(description='Analyze llserver LTTng traces')
    parser.add_argument('trace_path', help='Path to LTTng trace directory')
    parser.add_argument('--debug', action='store_true', help='Enable verbose debug output')
    parser.add_argument('--no-cache', action='store_true', help='Do not use existing cache; force reparse')
    parser.add_argument('--use-cache', action='store_true', help='Use cache automatically without prompt if present')
    parser.add_argument('--cache-events', action='store_true', help='Store per-event records in cache (larger file)')
    parser.add_argument('--legend-mode', choices=['right','below','inside'], default='right', help='Legend placement strategy to avoid overlap (default: right outside)')
    args = parser.parse_args()

    trace_path = args.trace_path
    if not os.path.exists(trace_path):
        print(f"Error: Trace path {trace_path} does not exist")
        sys.exit(1)

    analyzer = LLServerTraceAnalyzer(trace_path, debug=args.debug, cache_events=args.cache_events,
                                     legends_outside=(args.legend_mode!='inside'), legend_mode=args.legend_mode)
    cache_dir = 'plots'
    os.makedirs(cache_dir, exist_ok=True)
    cache_file = os.path.join(cache_dir, 'trace_cache.pkl')

    use_cache = False
    if not args.no_cache and os.path.exists(cache_file):
        if args.use_cache:
            use_cache = True
            if args.debug:
                print("[DEBUG] Using cache (auto mode)")
        else:
            resp = input(f"Cached parsed data found at {cache_file}. Use cached data? [y/N]: ").strip().lower()
            if resp == 'y':
                use_cache = True

    try:
        if use_cache:
            analyzer.load_cache(cache_file)
        else:
            analyzer.parse_traces()
            analyzer.save_cache(cache_file)
        # Print all unique field names after parsing
        print("\nUnique field names found in trace events:")
        for name in sorted(analyzer.field_names):
            print(f"  {name}")
        analyzer.generate_plots()
        analyzer.generate_summary_report()
        print("\nAnalysis complete! Check the 'plots' directory for results.")
    except Exception as e:
        print(f"Error during analysis: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

if __name__ == "__main__":
    main()
