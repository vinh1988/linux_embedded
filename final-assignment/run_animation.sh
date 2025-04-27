#!/bin/bash

# Make scripts executable
chmod +x temperature_animation.py
chmod +x simulate_sensors.py

# Check if we should simulate data
if [ "$1" == "--simulate" ]; then
    echo "Running animation with simulated data..."
    ./temperature_animation.py --simulate
else
    # Start the sensor gateway server if not already running
    if ! pgrep -f "./server" > /dev/null; then
        echo "Starting sensor gateway server..."
        ./server --port 1234 &
        SERVER_PID=$!
        sleep 2  # Give the server time to start
    fi
    
    # Start the data simulation in the background
    echo "Starting sensor data simulation..."
    ./simulate_sensors.py --nodes 3 --interval 1.0 --duration 300 --hot-node 2 --cold-node 3 &
    SIM_PID=$!
    
    # Start the animation
    echo "Starting temperature animation..."
    ./temperature_animation.py
    
    # Clean up
    echo "Cleaning up..."
    if [ -n "$SIM_PID" ]; then
        kill $SIM_PID 2>/dev/null
    fi
    
    if [ -n "$SERVER_PID" ]; then
        kill $SERVER_PID 2>/dev/null
    fi
fi

echo "Done!"
