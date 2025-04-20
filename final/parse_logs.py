#!/usr/bin/env python3
"""
Parse sensor gateway logs and extract sensor data for visualization.
"""

import os
import re
import json
import time
import argparse
from datetime import datetime

# Regular expressions for parsing log entries
SENSOR_DATA_PATTERN = re.compile(r'Received data from sensor node (\d+): temp=([\d\.]+), humidity=([\d\.]+), pressure=([\d\.]+)')
LOG_ENTRY_PATTERN = re.compile(r'LOG: (\d+) (\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}) (.+)')

def parse_log_file(log_file):
    """Parse the log file and extract sensor data."""
    sensor_data = []
    
    try:
        with open(log_file, 'r') as f:
            for line in f:
                # Check if line contains sensor data
                sensor_match = SENSOR_DATA_PATTERN.search(line)
                if sensor_match:
                    node_id = int(sensor_match.group(1))
                    temperature = float(sensor_match.group(2))
                    humidity = float(sensor_match.group(3))
                    pressure = float(sensor_match.group(4))
                    
                    # Extract timestamp from log entry if available
                    timestamp = None
                    log_match = LOG_ENTRY_PATTERN.search(line)
                    if log_match:
                        timestamp_str = log_match.group(2)
                        try:
                            timestamp = datetime.strptime(timestamp_str, '%Y-%m-%d %H:%M:%S').timestamp()
                        except ValueError:
                            timestamp = time.time()  # Use current time if parsing fails
                    else:
                        timestamp = time.time()  # Use current time if no timestamp found
                    
                    sensor_data.append({
                        'node_id': node_id,
                        'temperature': temperature,
                        'humidity': humidity,
                        'pressure': pressure,
                        'timestamp': timestamp
                    })
    except FileNotFoundError:
        print(f"Warning: Log file {log_file} not found.")
    
    return sensor_data

def find_log_files(directory='.'):
    """Find all gateway log files in the directory."""
    log_files = []
    for file in os.listdir(directory):
        if file.startswith('gateway.log_'):
            log_files.append(os.path.join(directory, file))
    return log_files

def parse_all_logs(directory='.'):
    """Parse all log files and combine the data."""
    log_files = find_log_files(directory)
    all_data = []
    
    for log_file in log_files:
        data = parse_log_file(log_file)
        all_data.extend(data)
    
    # Sort by timestamp
    all_data.sort(key=lambda x: x['timestamp'])
    
    return all_data

def generate_json(data, output_file='sensor_history.json'):
    """Generate JSON file from parsed data."""
    with open(output_file, 'w') as f:
        json.dump(data, f, indent=2)
    print(f"Sensor data saved to {output_file}")

def monitor_logs(directory='.', output_file='sensor_history.json', interval=5):
    """Monitor log files for changes and update JSON file."""
    print(f"Monitoring log files in {directory}...")
    print(f"Data will be updated every {interval} seconds.")
    print(f"Press Ctrl+C to stop.")
    
    try:
        while True:
            data = parse_all_logs(directory)
            generate_json(data, output_file)
            time.sleep(interval)
    except KeyboardInterrupt:
        print("\nMonitoring stopped.")

def main():
    parser = argparse.ArgumentParser(description='Parse sensor gateway logs for visualization.')
    parser.add_argument('-d', '--directory', default='.', help='Directory containing log files')
    parser.add_argument('-o', '--output', default='sensor_history.json', help='Output JSON file')
    parser.add_argument('-m', '--monitor', action='store_true', help='Monitor logs for changes')
    parser.add_argument('-i', '--interval', type=int, default=5, help='Update interval in seconds (for monitor mode)')
    
    args = parser.parse_args()
    
    if args.monitor:
        monitor_logs(args.directory, args.output, args.interval)
    else:
        data = parse_all_logs(args.directory)
        generate_json(data, args.output)

if __name__ == '__main__':
    main()
