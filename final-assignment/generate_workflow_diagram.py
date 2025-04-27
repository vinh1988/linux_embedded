#!/usr/bin/env python3
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import numpy as np

# Create figure and axis
fig, ax = plt.subplots(figsize=(12, 10))
ax.set_xlim(0, 100)
ax.set_ylim(0, 100)
ax.axis('off')

# Define colors
colors = {
    'sensor': '#AED6F1',  # Light blue
    'main_process': '#D5F5E3',  # Light green
    'thread': '#FADBD8',  # Light red
    'database': '#E8DAEF',  # Light purple
    'log': '#FCF3CF',  # Light yellow
    'arrow': '#5D6D7E',  # Dark gray
    'text': '#17202A',  # Almost black
    'visualization': '#FDEBD0',  # Light orange
    'security': '#D6EAF8'  # Very light blue
}

# Helper function to draw a box
def draw_box(x, y, width, height, label, color, fontsize=10):
    rect = patches.Rectangle((x, y), width, height, linewidth=1, edgecolor='black', facecolor=color)
    ax.add_patch(rect)
    ax.text(x + width/2, y + height/2, label, ha='center', va='center', fontsize=fontsize, wrap=True)
    return rect

# Helper function to draw an arrow
def draw_arrow(start_x, start_y, end_x, end_y, label=None, fontsize=8):
    ax.arrow(start_x, start_y, end_x - start_x, end_y - start_y, 
             head_width=1, head_length=1, fc=colors['arrow'], ec=colors['arrow'],
             length_includes_head=True)
    if label:
        mid_x = (start_x + end_x) / 2
        mid_y = (start_y + end_y) / 2
        ax.text(mid_x, mid_y, label, ha='center', va='center', fontsize=fontsize,
                bbox=dict(facecolor='white', alpha=0.7, edgecolor='none'))

# Title
ax.text(50, 95, 'Sensor Gateway System Workflow', ha='center', fontsize=16, weight='bold')

# Draw sensor nodes
sensor_width = 15
sensor_height = 5
for i in range(3):
    y_pos = 80 - i * 8
    draw_box(5, y_pos, sensor_width, sensor_height, f'Sensor Node {i+1}', colors['sensor'])
    draw_arrow(20, y_pos + sensor_height/2, 30, y_pos + sensor_height/2, 'TCP')

# Draw main process container
main_process = draw_box(30, 40, 40, 50, '', colors['main_process'])
ax.text(50, 85, 'Main Process', ha='center', fontsize=12, weight='bold')

# Draw threads inside main process
thread_width = 30
thread_height = 8
thread_x = 35

# Connection Manager Thread
conn_thread = draw_box(thread_x, 75, thread_width, thread_height, 'Connection Manager\nThread', colors['thread'])
draw_arrow(50, 75, 50, 70, 'Shared Data')

# Data Manager Thread
data_thread = draw_box(thread_x, 60, thread_width, thread_height, 'Data Manager\nThread', colors['thread'])
draw_arrow(50, 60, 50, 55, 'Shared Data')

# Storage Manager Thread
storage_thread = draw_box(thread_x, 45, thread_width, thread_height, 'Storage Manager\nThread', colors['thread'])

# Command Input Thread
command_thread = draw_box(thread_x, 30, thread_width, thread_height, 'Command Input\nThread', colors['thread'])

# Draw FIFO arrow
draw_arrow(50, 45, 50, 35, 'FIFO')

# Draw Log Process
log_process = draw_box(30, 20, 40, 10, 'Log Process', colors['log'])

# Draw Database
database = draw_box(5, 20, 15, 10, 'SQLite\nDatabase', colors['database'])
draw_arrow(30, 25, 20, 25, 'Writes')

# Draw Log File
log_file = draw_box(30, 5, 40, 8, 'Log File (gateway.log)', colors['log'])
draw_arrow(50, 20, 50, 13, 'Writes')

# Draw Visualization Components
ax.text(80, 40, 'Visualization Components', ha='center', fontsize=12, weight='bold')

# Temperature Animation
temp_anim = draw_box(75, 30, 20, 8, 'Temperature\nAnimation', colors['visualization'])
draw_arrow(20, 20, 75, 34, 'Reads')

# Simulate Sensors
sim_sensors = draw_box(75, 20, 20, 8, 'Simulate\nSensors Script', colors['visualization'])
draw_arrow(85, 20, 85, 30, 'Simulates')

# Visualization Output
vis_output = draw_box(75, 5, 20, 8, 'Visualization\nOutput', colors['visualization'])
draw_arrow(85, 30, 85, 13, 'Displays')

# Security Components
ax.text(80, 70, 'Security Components', ha='center', fontsize=12, weight='bold')

# Authentication
auth = draw_box(75, 60, 20, 8, 'Authentication', colors['security'])

# IP Blacklist
blacklist = draw_box(75, 50, 20, 8, 'IP Blacklist', colors['security'])
draw_arrow(85, 60, 85, 58, 'Check')

# Connect security to main process
draw_arrow(75, 64, 65, 64, 'Secures')

# Add legend
legend_x = 5
legend_y = 95
legend_items = [
    ('Sensor Nodes', colors['sensor']),
    ('Main Process', colors['main_process']),
    ('Threads', colors['thread']),
    ('Database', colors['database']),
    ('Logging', colors['log']),
    ('Visualization', colors['visualization']),
    ('Security', colors['security'])
]

for i, (label, color) in enumerate(legend_items):
    x = legend_x + (i % 4) * 20
    y = legend_y - (i // 4) * 5
    draw_box(x, y, 4, 3, '', color)
    ax.text(x + 5, y + 1.5, label, ha='left', va='center', fontsize=8)

# Save the diagram
plt.tight_layout()
plt.savefig('workflow_diagram.png', dpi=150, bbox_inches='tight')
print("Workflow diagram saved as 'workflow_diagram.png'")

# Uncomment to display the diagram
# plt.show()
