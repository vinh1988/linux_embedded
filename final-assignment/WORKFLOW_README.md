# Sensor Gateway System Workflow

This document explains the workflow and architecture of the Sensor Gateway System.

## System Overview

The Sensor Gateway System is designed to collect, process, store, and visualize data from multiple sensor nodes. The system consists of several components that work together to provide a robust and efficient solution.

## Components

### 1. Main Process

The main process is the core of the system and contains multiple threads:

#### 1.1 Connection Manager Thread
- Listens for incoming TCP connections from sensor nodes
- Accepts connections and reads sensor data packets
- Validates sensor data
- Stores data in the shared data structure
- Handles connection authentication and security
- Manages connection timeouts and cleanup
- Uses epoll for efficient I/O multiplexing

#### 1.2 Data Manager Thread
- Reads sensor data from the shared data structure
- Calculates running averages for temperature readings
- Detects temperature anomalies (too hot/too cold)
- Logs temperature events

#### 1.3 Storage Manager Thread
- Reads sensor data from the shared data structure
- Connects to the SQLite database
- Stores sensor data in the database
- Handles database connection failures and retries

#### 1.4 Command Input Thread
- Processes user commands from the terminal
- Provides system status information
- Handles graceful shutdown

### 2. Log Process

The Log Process is a separate process that:
- Reads log events from the FIFO
- Formats log messages with timestamps and sequence numbers
- Writes log messages to the log file (gateway.log)

### 3. Shared Data Structure

A thread-safe data structure that:
- Stores the latest data from each sensor node
- Is protected by a mutex to prevent race conditions
- Is accessed by all threads in the main process

### 4. FIFO (Named Pipe)

A named pipe that:
- Provides inter-process communication between the main process and log process
- Transfers log events from the main process to the log process

### 5. SQLite Database

A relational database that:
- Stores historical sensor data
- Provides persistent storage for later analysis
- Contains tables for sensor readings with timestamps

### 6. Security Components

Security features include:
- Connection authentication
- IP blacklisting
- Connection limits per IP address
- Protection against DoS attacks

### 7. Visualization Components

Tools for visualizing the sensor data:
- Temperature animation script for real-time visualization
- Sensor simulation script for testing
- Static image generator for reports

## Data Flow

1. **Sensor Data Collection**:
   - Sensor nodes connect to the Connection Manager via TCP
   - Connection Manager authenticates connections and reads data packets
   - Data is stored in the shared data structure

2. **Data Processing**:
   - Data Manager reads from the shared data structure
   - Calculates running averages and detects anomalies
   - Generates log events for temperature anomalies

3. **Data Storage**:
   - Storage Manager reads from the shared data structure
   - Stores data in the SQLite database

4. **Logging**:
   - Log events are sent through the FIFO to the Log Process
   - Log Process writes formatted log messages to the log file

5. **Visualization**:
   - Temperature Animation reads data from the database
   - Displays real-time visualization of temperature readings
   - Shows status indicators for each sensor

## Alternative Data Flow (Simulation)

For testing and demonstration purposes:
1. Simulate Sensors script generates random sensor data
2. Data is either:
   - Sent to the server via TCP connections, or
   - Directly inserted into the database
3. Temperature Animation visualizes the simulated data

## Command Flow

1. User enters commands in the terminal
2. Command Input Thread processes the commands
3. System responds with requested information or actions

## Generating the Workflow Diagram

To generate the workflow diagram:

```bash
./generate_workflow_diagram.py
```

This will create a visual representation of the system workflow as `workflow_diagram.png`.

## Conclusion

The Sensor Gateway System provides a comprehensive solution for collecting, processing, storing, and visualizing sensor data. Its modular design allows for easy maintenance and extension, while its multi-threaded architecture ensures efficient operation even with multiple concurrent connections.
