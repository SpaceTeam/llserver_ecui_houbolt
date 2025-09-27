#!/usr/bin/env python3
"""
Simple LTTng Trace Analysis for llserver - Text-based parser

This script parses text output from LTTng traces (using 'babeltrace' command)
and generates plots for backlog diagnosis.

Usage:
    # First convert your trace to text format:
    babeltrace /path/to/trace > trace.txt
    
    # Then analyze:
    python3 simple_analyze.py trace.txt
"""

import sys
import re
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from datetime import datetime
from collections import defaultdict
import os

class SimpleTraceAnalyzer:
    def __init__(self, trace_file):
        self.trace_file = trace_file
        self.events = []
        self.processing_events = defaultdict(dict)
        
    def parse_trace_text(self):
        """Parse text output from babeltrace command"""
        print(f"Parsing trace file: {self.trace_file}")
        
        # Regex patterns for different event types
        patterns = {
            'timestamp': r'\[(\d{2}:\d{2}:\d{2}\.\d{9})\]',
            'llserver_event': r'llserver:(\w+):',
            'field': r'(\w+)\s*=\s*([^,\s\}]+)'
        }
        
        event_count = 0
        with open(self.trace_file, 'r') as f:
            for line in f:
                if 'llserver:' in line:
                    event = self.parse_line(line, patterns)
                    if event:
                        self.events.append(event)
                        self.process_event(event)
                        event_count += 1
                        
                        if event_count % 1000 == 0:
                            print(f"Processed {event_count} events...")
                            
        print(f"Finished parsing {event_count} total events")
        
    def parse_line(self, line, patterns):
        """Parse a single trace line"""
        # Extract timestamp
        ts_match = re.search(patterns['timestamp'], line)
        if not ts_match:
            return None
            
        timestamp = ts_match.group(1)
        
        # Extract event name
        event_match = re.search(patterns['llserver_event'], line)
        if not event_match:
            return None
            
        event_name = event_match.group(1)
        
        # Extract fields
        fields = {}
        for field_match in re.finditer(patterns['field'], line):
            key = field_match.group(1)
            value = field_match.group(2)
            # Try to convert to number
            try:
                if '.' in value:
                    value = float(value)
                else:
                    value = int(value)
            except ValueError:
                # Keep as string, remove quotes if present
                value = value.strip('"')
            fields[key] = value
            
        return {
            'timestamp_str': timestamp,
            'timestamp_ns': self.parse_timestamp_to_ns(timestamp),
            'event_name': event_name,
            'fields': fields
        }
        
    def parse_timestamp_to_ns(self, timestamp_str):
        """Convert timestamp string to nanoseconds"""
        # Format: HH:MM:SS.nnnnnnnnn
        try:
            parts = timestamp_str.split(':')
            hours = int(parts[0])
            minutes = int(parts[1])
            sec_parts = parts[2].split('.')
            seconds = int(sec_parts[0])
            nanoseconds = int(sec_parts[1])
            
            total_ns = (hours * 3600 + minutes * 60 + seconds) * 1_000_000_000 + nanoseconds
            return total_ns
        except:
            return 0
            
    def process_event(self, event):
        """Process individual events"""
        event_name = event['event_name']
        fields = event['fields']
        timestamp = event['timestamp_ns']
        
        if event_name == 'packet_received':
            seqno = fields.get('seqno', 0)
            self.processing_events[seqno]['received_time'] = timestamp
            self.processing_events[seqno]['queue_size_at_receive'] = fields.get('queue_size', 0)
            
        elif event_name == 'process_start':
            seqno = fields.get('seqno', 0)
            self.processing_events[seqno]['start_time'] = timestamp
            self.processing_events[seqno]['queue_delay_ns'] = fields.get('queue_delay_ns', 0)
            
        elif event_name == 'process_end':
            seqno = fields.get('seqno', 0)
            self.processing_events[seqno]['end_time'] = timestamp
            self.processing_events[seqno]['processing_ns'] = fields.get('processing_ns', 0)
            
    def generate_basic_plots(self, output_dir='simple_plots'):
        """Generate basic analysis plots"""
        os.makedirs(output_dir, exist_ok=True)
        print(f"Generating plots in {output_dir}/")
        
        # Filter events by type
        packet_events = [e for e in self.events if e['event_name'] == 'packet_received']
        snapshot_events = [e for e in self.events if e['event_name'] == 'queue_snapshot']
        backlog_events = [e for e in self.events if e['event_name'] == 'backlog_trigger']
        lock_wait_events = [e for e in self.events if e['event_name'] == 'lock_wait']
        io_events = [e for e in self.events if e['event_name'] == 'logging_io']
        
        self.plot_queue_size_simple(snapshot_events, backlog_events, output_dir)
        self.plot_processing_latencies_simple(output_dir)
        self.plot_lock_analysis_simple(lock_wait_events, output_dir)
        self.plot_io_analysis_simple(io_events, output_dir)
        self.generate_simple_summary(output_dir)
        
    def plot_queue_size_simple(self, snapshot_events, backlog_events, output_dir):
        """Plot queue size over time"""
        if not snapshot_events:
            print("No queue snapshot events found")
            return
            
        # Convert to relative timestamps (seconds from start)
        start_time = min(e['timestamp_ns'] for e in snapshot_events)
        times = [(e['timestamp_ns'] - start_time) / 1e9 for e in snapshot_events]
        queue_sizes = [e['fields'].get('queue_size', 0) for e in snapshot_events]
        
        fig, ax = plt.subplots(figsize=(15, 8))
        
        ax.plot(times, queue_sizes, 'b-', linewidth=2, label='Queue Size')
        
        # Mark backlog triggers
        if backlog_events:
            backlog_times = [(e['timestamp_ns'] - start_time) / 1e9 for e in backlog_events]
            backlog_sizes = [e['fields'].get('queue_size', 0) for e in backlog_events]
            ax.scatter(backlog_times, backlog_sizes, color='red', s=100, marker='x', 
                      label='Backlog Triggers', zorder=5)
        
        ax.set_xlabel('Time (seconds)')
        ax.set_ylabel('Queue Size')
        ax.set_title('Queue Size Over Time')
        ax.legend()
        ax.grid(True, alpha=0.3)
        
        plt.tight_layout()
        plt.savefig(f'{output_dir}/queue_size.png', dpi=300, bbox_inches='tight')
        plt.close()
        
    def plot_processing_latencies_simple(self, output_dir):
        """Plot processing latency analysis"""
        # Collect complete processing events
        complete_events = []
        for seqno, data in self.processing_events.items():
            if 'processing_ns' in data and 'queue_delay_ns' in data:
                complete_events.append({
                    'processing_ms': data['processing_ns'] / 1e6,
                    'queue_delay_ms': data['queue_delay_ns'] / 1e6,
                    'start_time': data.get('start_time', 0)
                })
                
        if not complete_events:
            print("No complete processing events found")
            return
            
        df = pd.DataFrame(complete_events)
        
        fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(16, 12))
        
        # Processing latency histogram
        ax1.hist(df['processing_ms'], bins=50, alpha=0.7, edgecolor='black')
        ax1.set_xlabel('Processing Latency (ms)')
        ax1.set_ylabel('Count')
        ax1.set_title('Processing Latency Distribution')
        ax1.axvline(df['processing_ms'].mean(), color='red', linestyle='--',
                   label=f'Mean: {df["processing_ms"].mean():.2f}ms')
        ax1.axvline(df['processing_ms'].quantile(0.95), color='orange', linestyle='--',
                   label=f'95th percentile: {df["processing_ms"].quantile(0.95):.2f}ms')
        ax1.legend()
        ax1.grid(True, alpha=0.3)
        
        # Queue delay histogram
        ax2.hist(df['queue_delay_ms'], bins=50, alpha=0.7, edgecolor='black', color='orange')
        ax2.set_xlabel('Queue Delay (ms)')
        ax2.set_ylabel('Count')
        ax2.set_title('Queue Delay Distribution')
        ax2.axvline(df['queue_delay_ms'].mean(), color='red', linestyle='--',
                   label=f'Mean: {df["queue_delay_ms"].mean():.2f}ms')
        ax2.axvline(df['queue_delay_ms'].quantile(0.95), color='darkred', linestyle='--',
                   label=f'95th percentile: {df["queue_delay_ms"].quantile(0.95):.2f}ms')
        ax2.legend()
        ax2.grid(True, alpha=0.3)
        
        # Processing vs queue delay scatter
        ax3.scatter(df['queue_delay_ms'], df['processing_ms'], alpha=0.6, s=10)
        ax3.set_xlabel('Queue Delay (ms)')
        ax3.set_ylabel('Processing Latency (ms)')
        ax3.set_title('Processing Latency vs Queue Delay')
        ax3.grid(True, alpha=0.3)
        
        # Time series if we have timestamp info
        if df['start_time'].max() > 0:
            start_time = df['start_time'].min()
            df['time_seconds'] = (df['start_time'] - start_time) / 1e9
            ax4.scatter(df['time_seconds'], df['processing_ms'], alpha=0.6, s=5, label='Processing')
            ax4.scatter(df['time_seconds'], df['queue_delay_ms'], alpha=0.6, s=5, label='Queue Delay')
            ax4.set_xlabel('Time (seconds)')
            ax4.set_ylabel('Latency (ms)')
            ax4.set_title('Latencies Over Time')
            ax4.legend()
            ax4.grid(True, alpha=0.3)
        else:
            ax4.text(0.5, 0.5, 'No timestamp data available', 
                    ha='center', va='center', transform=ax4.transAxes)
            ax4.set_title('Latencies Over Time (No Data)')
        
        plt.tight_layout()
        plt.savefig(f'{output_dir}/processing_latencies.png', dpi=300, bbox_inches='tight')
        plt.close()
        
    def plot_lock_analysis_simple(self, lock_events, output_dir):
        """Plot lock contention analysis"""
        if not lock_events:
            print("No lock events found")
            return
            
        # Extract lock wait times by mutex
        mutex_waits = defaultdict(list)
        for event in lock_events:
            mutex_name = event['fields'].get('mutex_name', 'unknown')
            wait_ns = event['fields'].get('wait_ns', 0)
            mutex_waits[mutex_name].append(wait_ns / 1e6)  # Convert to ms
            
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 6))
        
        # Lock wait time distribution by mutex
        for mutex, waits in mutex_waits.items():
            ax1.hist(waits, bins=30, alpha=0.7, label=f'{mutex} (mean: {np.mean(waits):.2f}ms)')
            
        ax1.set_xlabel('Lock Wait Time (ms)')
        ax1.set_ylabel('Count')
        ax1.set_title('Lock Wait Time Distribution by Mutex')
        ax1.legend()
        ax1.grid(True, alpha=0.3)
        ax1.set_xscale('log')
        
        # Average wait times by mutex
        mutex_names = list(mutex_waits.keys())
        avg_waits = [np.mean(mutex_waits[mutex]) for mutex in mutex_names]
        max_waits = [np.max(mutex_waits[mutex]) for mutex in mutex_names]
        
        x = np.arange(len(mutex_names))
        width = 0.35
        
        ax2.bar(x - width/2, avg_waits, width, label='Average Wait Time', alpha=0.7)
        ax2.bar(x + width/2, max_waits, width, label='Max Wait Time', alpha=0.7)
        ax2.set_xlabel('Mutex')
        ax2.set_ylabel('Wait Time (ms)')
        ax2.set_title('Lock Wait Times by Mutex')
        ax2.set_xticks(x)
        ax2.set_xticklabels(mutex_names, rotation=45)
        ax2.legend()
        ax2.grid(True, alpha=0.3)
        ax2.set_yscale('log')
        
        plt.tight_layout()
        plt.savefig(f'{output_dir}/lock_analysis.png', dpi=300, bbox_inches='tight')
        plt.close()
        
    def plot_io_analysis_simple(self, io_events, output_dir):
        """Plot I/O analysis"""
        if not io_events:
            print("No I/O events found")
            return
            
        # Extract I/O durations
        io_durations = [event['fields'].get('io_ns', 0) / 1e6 for event in io_events]  # ms
        io_bytes = [event['fields'].get('bytes', 0) for event in io_events]
        
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 6))
        
        # I/O duration histogram
        ax1.hist(io_durations, bins=50, alpha=0.7, edgecolor='black')
        ax1.set_xlabel('I/O Duration (ms)')
        ax1.set_ylabel('Count')
        ax1.set_title('I/O Duration Distribution')
        ax1.axvline(np.mean(io_durations), color='red', linestyle='--',
                   label=f'Mean: {np.mean(io_durations):.2f}ms')
        ax1.axvline(np.percentile(io_durations, 95), color='orange', linestyle='--',
                   label=f'95th percentile: {np.percentile(io_durations, 95):.2f}ms')
        ax1.legend()
        ax1.grid(True, alpha=0.3)
        ax1.set_xscale('log')
        
        # I/O duration vs bytes
        ax2.scatter(io_bytes, io_durations, alpha=0.6, s=10)
        ax2.set_xlabel('Bytes')
        ax2.set_ylabel('I/O Duration (ms)')
        ax2.set_title('I/O Duration vs Bytes')
        ax2.grid(True, alpha=0.3)
        ax2.set_xscale('log')
        ax2.set_yscale('log')
        
        plt.tight_layout()
        plt.savefig(f'{output_dir}/io_analysis.png', dpi=300, bbox_inches='tight')
        plt.close()
        
    def generate_simple_summary(self, output_dir):
        """Generate a simple text summary"""
        with open(f'{output_dir}/summary.txt', 'w') as f:
            f.write("Simple LLServer Trace Analysis Summary\n")
            f.write("=" * 40 + "\n\n")
            
            f.write(f"Total events processed: {len(self.events)}\n")
            
            # Event type breakdown
            event_types = defaultdict(int)
            for event in self.events:
                event_types[event['event_name']] += 1
                
            f.write("\nEvent type breakdown:\n")
            for event_type, count in sorted(event_types.items()):
                f.write(f"  {event_type}: {count}\n")
            
            # Processing statistics
            complete_events = [data for data in self.processing_events.values() 
                             if 'processing_ns' in data and 'queue_delay_ns' in data]
            
            if complete_events:
                processing_times = [d['processing_ns'] / 1e6 for d in complete_events]
                queue_delays = [d['queue_delay_ns'] / 1e6 for d in complete_events]
                
                f.write(f"\nProcessing Statistics ({len(complete_events)} samples):\n")
                f.write(f"  Mean processing time: {np.mean(processing_times):.2f} ms\n")
                f.write(f"  95th percentile: {np.percentile(processing_times, 95):.2f} ms\n")
                f.write(f"  Max processing time: {np.max(processing_times):.2f} ms\n")
                
                f.write(f"\nQueue Delay Statistics:\n")
                f.write(f"  Mean queue delay: {np.mean(queue_delays):.2f} ms\n")
                f.write(f"  95th percentile: {np.percentile(queue_delays, 95):.2f} ms\n")
                f.write(f"  Max queue delay: {np.max(queue_delays):.2f} ms\n")
            
            # Lock statistics
            lock_events = [e for e in self.events if e['event_name'] == 'lock_wait']
            if lock_events:
                wait_times = [e['fields'].get('wait_ns', 0) / 1e6 for e in lock_events]
                f.write(f"\nLock Wait Statistics ({len(lock_events)} samples):\n")
                f.write(f"  Mean wait time: {np.mean(wait_times):.2f} ms\n")
                f.write(f"  95th percentile: {np.percentile(wait_times, 95):.2f} ms\n")
                f.write(f"  Max wait time: {np.max(wait_times):.2f} ms\n")
            
            # I/O statistics
            io_events = [e for e in self.events if e['event_name'] == 'logging_io']
            if io_events:
                io_times = [e['fields'].get('io_ns', 0) / 1e6 for e in io_events]
                f.write(f"\nI/O Statistics ({len(io_events)} samples):\n")
                f.write(f"  Mean I/O time: {np.mean(io_times):.2f} ms\n")
                f.write(f"  95th percentile: {np.percentile(io_times, 95):.2f} ms\n")
                f.write(f"  Max I/O time: {np.max(io_times):.2f} ms\n")
        
        print(f"Summary written to {output_dir}/summary.txt")

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 simple_analyze.py trace.txt")
        print("\nTo generate trace.txt from LTTng trace:")
        print("  babeltrace /path/to/trace > trace.txt")
        sys.exit(1)
    
    trace_file = sys.argv[1]
    if not os.path.exists(trace_file):
        print(f"Error: Trace file {trace_file} does not exist")
        sys.exit(1)
    
    analyzer = SimpleTraceAnalyzer(trace_file)
    
    try:
        analyzer.parse_trace_text()
        analyzer.generate_basic_plots()
        print("\nAnalysis complete! Check the 'simple_plots' directory for results.")
    except Exception as e:
        print(f"Error during analysis: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

if __name__ == "__main__":
    main()
