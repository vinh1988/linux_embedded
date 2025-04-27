# Temperature Measurement Animation

This directory contains scripts to visualize temperature data from the sensor gateway system.

## Files

- `temperature_animation.py`: Python script that creates an animated visualization of temperature data
- `simulate_sensors.py`: Python script that simulates sensor data and inserts it into the database
- `run_animation.sh`: Shell script to run the animation with either simulated or real data
- `generate_temperature_image.py`: Python script to generate a static image of temperature data
- `temperature_measurement.png`: Example static image of temperature visualization

## Requirements

- Python 3.6 or higher
- matplotlib
- numpy
- sqlite3 (included with Python)

Install the required Python packages:

```bash
# Install pip if not already installed
sudo apt install python3-pip

# Install required packages
pip3 install matplotlib numpy
```

## Running the Animation

### Option 1: Simulated Data (No Server Required)

Run the animation with simulated data:

```bash
./run_animation.sh --simulate
```

This will generate random temperature data and display it in the animation without starting the server.

### Option 2: With Sensor Gateway Server

Run the animation with the sensor gateway server:

```bash
./run_animation.sh
```

This will:
1. Start the sensor gateway server on port 1234 (if not already running)
2. Start the data simulation script to generate sensor data
3. Start the animation to visualize the data
4. Clean up processes when the animation is closed

## Animation Features

The animation displays:

- Temperature readings from multiple sensors over time
- Temperature threshold lines for "Too Hot" (30°C) and "Too Cold" (10°C)
- Status indicators for each sensor showing their current state:
  - Green: Normal temperature
  - Red: Too hot
  - Blue: Too cold
  - Gray: No data available

## Customizing the Simulation

You can customize the simulation by editing the parameters in `run_animation.sh` or by running the scripts directly:

```bash
# Simulate 5 sensors with data every 0.5 seconds for 2 minutes
./simulate_sensors.py --nodes 5 --interval 0.5 --duration 120

# Run animation with 5 simulated sensors
./temperature_animation.py --simulate --nodes 5
```

## Generating a Static Image

To generate a static image of temperature data (similar to `temperature_measurement.png`):

```bash
./generate_temperature_image.py --nodes 3 --output my_temperature_image.png
```

This will create a static visualization with simulated data for the specified number of sensor nodes.

## Troubleshooting

- If the animation doesn't show any data, make sure the database exists and contains data
- If using the server, ensure it's running and properly configured
- Check for error messages in the terminal output
