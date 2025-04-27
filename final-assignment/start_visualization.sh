#!/bin/bash

# Configuration
SERVER_PORT=1245
NUM_SENSORS=3
SENSOR_INTERVAL=30
LOG_PARSER_INTERVAL=5
HTTP_PORT=8000

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to clean up on exit
cleanup() {
    echo -e "${YELLOW}Shutting down...${NC}"
    
    # Kill background processes
    if [ -n "$SERVER_PID" ]; then
        echo "Stopping server (PID: $SERVER_PID)..."
        kill $SERVER_PID 2>/dev/null
    fi
    
    if [ -n "$SENSORS_PID" ]; then
        echo "Stopping sensor simulators..."
        kill $SENSORS_PID 2>/dev/null
    fi
    
    if [ -n "$PARSER_PID" ]; then
        echo "Stopping log parser..."
        kill $PARSER_PID 2>/dev/null
    fi
    
    if [ -n "$HTTP_PID" ]; then
        echo "Stopping HTTP server..."
        kill $HTTP_PID 2>/dev/null
    fi
    
    echo -e "${GREEN}Cleanup complete.${NC}"
    exit 0
}

# Set up trap for clean exit
trap cleanup SIGINT SIGTERM

# Print banner
echo -e "${BLUE}=======================================${NC}"
echo -e "${BLUE}  Sensor Gateway Visualization System  ${NC}"
echo -e "${BLUE}=======================================${NC}"
echo

# Check for required commands
if ! command_exists python3; then
    echo -e "${YELLOW}Warning: python3 not found. Log parsing may not work.${NC}"
fi

# Start the sensor gateway server
echo -e "${GREEN}Starting sensor gateway server on port $SERVER_PORT...${NC}"
./server $SERVER_PORT &
SERVER_PID=$!
echo "Server started with PID: $SERVER_PID"
echo

# Wait for server to initialize
sleep 2

# Start sensor simulators
echo -e "${GREEN}Starting $NUM_SENSORS sensor simulators...${NC}"
./run_sensors.sh --num $NUM_SENSORS --port $SERVER_PORT --interval $SENSOR_INTERVAL &
SENSORS_PID=$!
echo "Sensor simulators started with PID: $SENSORS_PID"
echo

# Start log parser in monitor mode
echo -e "${GREEN}Starting log parser with update interval of $LOG_PARSER_INTERVAL seconds...${NC}"
python3 generate_sensor_data.py &
PARSER_PID=$!
echo "Log parser started with PID: $PARSER_PID"
echo

# Start a simple HTTP server
echo -e "${GREEN}Starting HTTP server on port $HTTP_PORT...${NC}"
if command_exists python3; then
    python3 -m http.server $HTTP_PORT &
    HTTP_PID=$!
    echo "HTTP server started with PID: $HTTP_PID"
    echo -e "${BLUE}Visualization available at: http://localhost:$HTTP_PORT/sensor_history.html${NC}"
else
    echo -e "${YELLOW}Warning: python3 not found. HTTP server not started.${NC}"
    echo -e "${YELLOW}Please open sensor_history.html in your browser manually.${NC}"
fi
echo

echo -e "${GREEN}All components started successfully.${NC}"
echo -e "${YELLOW}Press Ctrl+C to stop all components.${NC}"
echo

# Keep script running until user presses Ctrl+C
wait
