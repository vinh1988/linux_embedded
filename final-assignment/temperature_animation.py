#!/usr/bin/env python3
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import time
import random
import argparse
import sqlite3
from matplotlib.patches import Rectangle

# Constants
TEMP_TOO_HOT = 30.0
TEMP_TOO_COLD = 10.0
MAX_POINTS = 100
UPDATE_INTERVAL = 1000  # ms

# Command line arguments
parser = argparse.ArgumentParser(description='Animate temperature measurements')
parser.add_argument('--simulate', action='store_true', help='Simulate data instead of reading from database')
parser.add_argument('--nodes', type=int, default=3, help='Number of sensor nodes to simulate')
args = parser.parse_args()

# Setup figure and subplots
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))
fig.suptitle('Sensor Gateway Temperature Monitoring', fontsize=16)

# Setup temperature plot
ax1.set_ylim(0, 40)
ax1.set_xlim(0, MAX_POINTS)
ax1.set_title('Temperature Readings')
ax1.set_ylabel('Temperature (°C)')
ax1.set_xlabel('Time (s)')
ax1.grid(True)

# Add temperature threshold lines
ax1.axhline(y=TEMP_TOO_HOT, color='r', linestyle='--', alpha=0.7, label='Too Hot')
ax1.axhline(y=TEMP_TOO_COLD, color='b', linestyle='--', alpha=0.7, label='Too Cold')
ax1.legend(loc='upper right')

# Setup status indicators
ax2.set_title('Sensor Status')
ax2.set_xlim(0, 1)
ax2.set_ylim(0, args.nodes)
ax2.set_yticks(np.arange(args.nodes) + 0.5)
ax2.set_yticklabels([f'Sensor {i+1}' for i in range(args.nodes)])
ax2.set_xticks([])
ax2.set_frame_on(False)

# Initialize data
x_data = np.arange(0, MAX_POINTS)
y_data = [[] for _ in range(args.nodes)]
lines = []
status_indicators = []
colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b']

# Create line objects for each sensor
for i in range(args.nodes):
    line, = ax1.plot([], [], lw=2, label=f'Sensor {i+1}', color=colors[i % len(colors)])
    lines.append(line)
    
    # Create status indicator rectangles
    rect = Rectangle((0.1, i + 0.1), 0.8, 0.8, facecolor='gray', alpha=0.5)
    status_indicators.append(rect)
    ax2.add_patch(rect)

# Add legend to temperature plot
ax1.legend(loc='upper left')

# Function to get data from database
def get_db_data():
    try:
        conn = sqlite3.connect('sensor_data.db')
        cursor = conn.cursor()
        cursor.execute('''
            SELECT node_id, temperature 
            FROM sensor_data 
            ORDER BY timestamp DESC 
            LIMIT ?
        ''', (args.nodes,))
        results = cursor.fetchall()
        conn.close()
        
        # Convert to dictionary with node_id as key
        data = {}
        for node_id, temp in results:
            data[node_id] = temp / 100.0  # Convert from integer to float
        
        return data
    except Exception as e:
        print(f"Database error: {e}")
        return {}

# Function to simulate data
def simulate_data():
    data = {}
    for i in range(1, args.nodes + 1):
        # Base temperature with some randomness
        base_temp = 20.0 + 5.0 * np.sin(time.time() / 10.0 + i)
        # Add noise
        noise = random.uniform(-2.0, 2.0)
        data[i] = base_temp + noise
    return data

# Animation update function
def update(frame):
    # Get data
    if args.simulate:
        data = simulate_data()
    else:
        data = get_db_data()
    
    # Update each sensor's data
    for i in range(args.nodes):
        node_id = i + 1
        if node_id in data:
            temp = data[node_id]
            
            # Add new data point
            if len(y_data[i]) < MAX_POINTS:
                y_data[i].append(temp)
            else:
                y_data[i] = y_data[i][1:] + [temp]
            
            # Update line
            lines[i].set_data(x_data[:len(y_data[i])], y_data[i])
            
            # Update status indicator
            if temp > TEMP_TOO_HOT:
                status_indicators[i].set_facecolor('red')
                status_indicators[i].set_alpha(0.7)
            elif temp < TEMP_TOO_COLD:
                status_indicators[i].set_facecolor('blue')
                status_indicators[i].set_alpha(0.7)
            else:
                status_indicators[i].set_facecolor('green')
                status_indicators[i].set_alpha(0.7)
        else:
            # No data for this sensor
            status_indicators[i].set_facecolor('gray')
            status_indicators[i].set_alpha(0.5)
    
    return lines + status_indicators

# Create animation
ani = animation.FuncAnimation(
    fig, update, interval=UPDATE_INTERVAL, blit=True, save_count=50)

# Adjust layout and display
plt.tight_layout()
plt.subplots_adjust(top=0.9)
plt.show()
