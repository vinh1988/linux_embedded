#!/usr/bin/env python3
"""
Generate synthetic sensor data for visualization.
"""

import json
import time
import random
import math
from datetime import datetime, timedelta

def generate_sensor_data(num_sensors=3, num_points=100):
    """Generate synthetic sensor data for visualization."""
    data = []
    
    # Base values for each sensor
    base_temps = [22.0, 24.0, 20.0]
    base_humidity = [50.0, 45.0, 55.0]
    base_pressure = [1013.0, 1010.0, 1015.0]
    
    # Generate data points
    end_time = time.time()
    start_time = end_time - (num_points * 30)  # 30 seconds between points
    
    for i in range(num_points):
        timestamp = start_time + (i * 30)
        
        for sensor_id in range(1, num_sensors + 1):
            # Add some variation to the base values
            temp_variation = math.sin(i / 10) * 3 + random.uniform(-1, 1)
            humidity_variation = math.cos(i / 8) * 5 + random.uniform(-2, 2)
            pressure_variation = math.sin(i / 15) * 2 + random.uniform(-1, 1)
            
            idx = (sensor_id - 1) % len(base_temps)
            temperature = base_temps[idx] + temp_variation
            humidity = base_humidity[idx] + humidity_variation
            pressure = base_pressure[idx] + pressure_variation
            
            data.append({
                'node_id': sensor_id,
                'temperature': round(temperature, 2),
                'humidity': round(humidity, 2),
                'pressure': round(pressure, 2),
                'timestamp': timestamp
            })
    
    # Sort by timestamp
    data.sort(key=lambda x: x['timestamp'])
    
    return data

def save_json(data, output_file='sensor_history.json'):
    """Save data to JSON file."""
    with open(output_file, 'w') as f:
        json.dump(data, f, indent=2)
    print(f"Generated {len(data)} data points and saved to {output_file}")

def main():
    data = generate_sensor_data(num_sensors=3, num_points=100)
    save_json(data)

if __name__ == '__main__':
    main()
