#!/bin/bash

# Default values
SERVER_IP="127.0.0.1"
SERVER_PORT=1245
NUM_SENSORS=3
INTERVAL=30

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -s|--server)
            SERVER_IP="$2"
            shift 2
            ;;
        -p|--port)
            SERVER_PORT="$2"
            shift 2
            ;;
        -n|--num)
            NUM_SENSORS="$2"
            shift 2
            ;;
        -t|--interval)
            INTERVAL="$2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  -s, --server IP     Server IP address (default: 127.0.0.1)"
            echo "  -p, --port PORT     Server port (default: 1245)"
            echo "  -n, --num COUNT     Number of sensors to simulate (default: 3)"
            echo "  -t, --interval SEC  Data transmission interval in seconds (default: 30)"
            echo "  -h, --help          Display this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

echo "Starting $NUM_SENSORS sensor simulators..."
echo "Server: $SERVER_IP:$SERVER_PORT"
echo "Interval: $INTERVAL seconds"

# Start sensor simulators
for ((i=1; i<=$NUM_SENSORS; i++)); do
    # Calculate a slightly different interval for each sensor to avoid synchronized transmissions
    SENSOR_INTERVAL=$((INTERVAL - 2 + i))
    
    # Start the sensor simulator in the background
    ./sensor_simulator --id $i --server $SERVER_IP --port $SERVER_PORT --interval $SENSOR_INTERVAL > sensor_${i}.log 2>&1 &
    
    # Store the process ID
    PID=$!
    echo "Started sensor $i (PID: $PID)"
done

echo "All sensors started. Press Ctrl+C to stop all sensors."
echo "Sensor logs are being written to sensor_N.log files."

# Wait for Ctrl+C
trap 'echo "Stopping all sensors..."; pkill -P $$; wait; echo "All sensors stopped."; exit 0' INT TERM

# Keep the script running
wait
