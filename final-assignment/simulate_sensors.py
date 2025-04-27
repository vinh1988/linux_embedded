#!/usr/bin/env python3
import sqlite3
import time
import random
import argparse
import math
import os

# Command line arguments
parser = argparse.ArgumentParser(description='Simulate sensor data')
parser.add_argument('--nodes', type=int, default=3, help='Number of sensor nodes to simulate')
parser.add_argument('--interval', type=float, default=1.0, help='Interval between data points (seconds)')
parser.add_argument('--duration', type=float, default=60.0, help='Duration of simulation (seconds)')
parser.add_argument('--hot-node', type=int, default=0, help='Node ID to simulate as too hot (0 for none)')
parser.add_argument('--cold-node', type=int, default=0, help='Node ID to simulate as too cold (0 for none)')
args = parser.parse_args()

# Constants
DB_NAME = 'sensor_data.db'

# Create database if it doesn't exist
def init_database():
    if os.path.exists(DB_NAME):
        print(f"Using existing database: {DB_NAME}")
        return
    
    print(f"Creating new database: {DB_NAME}")
    conn = sqlite3.connect(DB_NAME)
    cursor = conn.cursor()
    
    # Create table
    cursor.execute('''
    CREATE TABLE IF NOT EXISTS sensor_data (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        node_id INTEGER NOT NULL,
        temperature INTEGER NOT NULL,
        humidity INTEGER NOT NULL,
        light INTEGER NOT NULL,
        timestamp INTEGER NOT NULL
    )
    ''')
    
    conn.commit()
    conn.close()

# Insert sensor data
def insert_data(node_id, temperature, humidity, light):
    conn = sqlite3.connect(DB_NAME)
    cursor = conn.cursor()
    
    # Convert to integers (as in the C code)
    temp_int = int(temperature * 100)
    humidity_int = int(humidity * 100)
    light_int = int(light)
    
    # Insert data
    cursor.execute('''
    INSERT INTO sensor_data (node_id, temperature, humidity, light, timestamp)
    VALUES (?, ?, ?, ?, ?)
    ''', (node_id, temp_int, humidity_int, light_int, int(time.time())))
    
    conn.commit()
    conn.close()

# Main simulation function
def run_simulation():
    init_database()
    
    print(f"Starting simulation with {args.nodes} nodes for {args.duration} seconds")
    print(f"Press Ctrl+C to stop")
    
    start_time = time.time()
    try:
        while time.time() - start_time < args.duration:
            for node_id in range(1, args.nodes + 1):
                # Base temperature with some randomness and sinusoidal variation
                base_temp = 20.0 + 5.0 * math.sin((time.time() - start_time) / 10.0 + node_id)
                
                # Add specific behavior for hot/cold nodes
                if node_id == args.hot_node:
                    base_temp = 32.0 + random.uniform(-1.0, 1.0)  # Too hot
                elif node_id == args.cold_node:
                    base_temp = 8.0 + random.uniform(-1.0, 1.0)   # Too cold
                else:
                    # Add noise to normal nodes
                    base_temp += random.uniform(-2.0, 2.0)
                
                # Generate humidity (inversely related to temperature)
                humidity = 100.0 - base_temp * 2 + random.uniform(-5.0, 5.0)
                humidity = max(10.0, min(90.0, humidity))  # Clamp between 10-90%
                
                # Generate light level (time of day dependent)
                hour_of_day = (time.time() % 86400) / 3600  # 0-24
                if 6 <= hour_of_day <= 18:  # Daytime
                    light = 800 + random.uniform(-200, 200)
                else:  # Nighttime
                    light = 200 + random.uniform(-100, 100)
                
                # Insert into database
                insert_data(node_id, base_temp, humidity, light)
                print(f"Node {node_id}: Temp={base_temp:.1f}°C, Humidity={humidity:.1f}%, Light={light:.0f}")
            
            # Wait for next interval
            time.sleep(args.interval)
            
    except KeyboardInterrupt:
        print("\nSimulation stopped by user")

if __name__ == "__main__":
    run_simulation()
