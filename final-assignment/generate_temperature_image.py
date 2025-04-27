#!/usr/bin/env python3
import matplotlib.pyplot as plt
import numpy as np
import random
import argparse
from matplotlib.patches import Rectangle

# Constants
TEMP_TOO_HOT = 30.0
TEMP_TOO_COLD = 10.0
MAX_POINTS = 100

# Command line arguments
parser = argparse.ArgumentParser(description='Generate temperature measurement image')
parser.add_argument('--nodes', type=int, default=3, help='Number of sensor nodes to simulate')
parser.add_argument('--output', type=str, default='temperature_measurement.png', help='Output file name')
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

# Generate simulated data
x_data = np.arange(0, MAX_POINTS)
y_data = []
colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b']
status_colors = []

for i in range(args.nodes):
    # Generate data with different patterns for each sensor
    if i == 0:  # Normal sensor
        base = 20.0 + np.sin(np.linspace(0, 4*np.pi, MAX_POINTS)) * 5
        noise = np.random.normal(0, 1, MAX_POINTS)
        data = base + noise
        status_color = 'green'
    elif i == 1:  # Hot sensor
        base = 28.0 + np.sin(np.linspace(0, 3*np.pi, MAX_POINTS)) * 4
        noise = np.random.normal(0, 1, MAX_POINTS)
        data = base + noise
        status_color = 'red'
    else:  # Cold sensor
        base = 12.0 + np.sin(np.linspace(0, 5*np.pi, MAX_POINTS)) * 3
        noise = np.random.normal(0, 1, MAX_POINTS)
        data = base + noise
        status_color = 'blue'
    
    y_data.append(data)
    status_colors.append(status_color)
    
    # Plot the data
    line, = ax1.plot(x_data, data, lw=2, label=f'Sensor {i+1}', color=colors[i % len(colors)])
    
    # Create status indicator
    rect = Rectangle((0.1, i + 0.1), 0.8, 0.8, facecolor=status_color, alpha=0.7)
    ax2.add_patch(rect)

# Add legend to temperature plot
ax1.legend(loc='upper left')

# Add annotations
for i in range(args.nodes):
    if status_colors[i] == 'red':
        ax2.text(0.5, i + 0.5, 'TOO HOT', ha='center', va='center', fontweight='bold')
    elif status_colors[i] == 'blue':
        ax2.text(0.5, i + 0.5, 'TOO COLD', ha='center', va='center', fontweight='bold')
    else:
        ax2.text(0.5, i + 0.5, 'NORMAL', ha='center', va='center', fontweight='bold')

# Adjust layout and save
plt.tight_layout()
plt.subplots_adjust(top=0.9)
plt.savefig(args.output, dpi=100)
print(f"Image saved to {args.output}")

# Optionally display the image
# plt.show()
