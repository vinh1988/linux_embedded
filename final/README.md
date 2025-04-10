# Sensor Gateway System

This project implements a sensor gateway system that collects data from sensor nodes, processes it, and stores it in an SQLite database. The system consists of a main process with three threads and a separate log process.

## Architecture

![Sensor Gateway Architecture](https://github.com/user/repo/raw/main/architecture.png)

The system consists of the following components:

1. **Main Process**: Contains three threads:
   - Connection Manager Thread
   - Data Manager Thread
   - Storage Manager Thread

2. **Log Process**: Handles logging of events from the main process

3. **Shared Data Structure**: Thread-safe data structure for communication between threads

4. **FIFO**: Named pipe for communication between the main process and log process

## Features

- TCP socket communication with sensor nodes
- Thread-safe shared data structure
- Running average calculation for temperature data
- SQLite database storage
- Logging system with sequence numbers and timestamps
- Graceful error handling and shutdown

## Requirements

- C compiler (gcc recommended)
- SQLite3 development libraries
- POSIX threads library
- POSIX real-time extensions library

## Building

To build the project, run:

```bash
make
```

This will create two executables:
- `server`: The main sensor gateway process
- `log_process`: The log process (started automatically by the server)

## Running

To run the sensor gateway, use:

```bash
./server <port>
```

For example:

```bash
./server 1234
```

The server will listen for sensor connections on the specified port, process the data, store it in an SQLite database, and log events to a log file.

## Sensor Node Protocol

Sensor nodes should send data packets in the following format:

```c
typedef struct {
    uint16_t node_id;
    uint16_t temperature;
    uint16_t humidity;
    uint16_t light;
} sensor_packet_t;
```

- `node_id`: Unique identifier for the sensor node (1-100)
- `temperature`: Temperature in 0.1°C units (e.g., 250 = 25.0°C)
- `humidity`: Humidity in 0.1% units (e.g., 500 = 50.0%)
- `light`: Light level (arbitrary units)

## Log File Format

The log file (`gateway.log`) contains entries in the following format:

```
<sequence number> <timestamp> <log message>
```

For example:

```
1 2023-05-01 12:34:56 A sensor node with 1 has opened a new connection
2 2023-05-01 12:35:01 The sensor node with 1 reports it's too hot (running avg temperature = 32.5)
```

## Database Schema

The sensor data is stored in an SQLite database (`sensor_data.db`) with the following schema:

```sql
CREATE TABLE sensor_data (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    node_id INTEGER NOT NULL,
    temperature REAL NOT NULL,
    humidity REAL NOT NULL,
    light REAL NOT NULL,
    timestamp INTEGER NOT NULL
);
```

## Cleaning Up

To clean up build artifacts and generated files, run:

```bash
make clean
```

## Implementation Details

### Connection Manager Thread
- Listens on TCP socket for incoming connections
- Accepts connections from sensor nodes
- Reads sensor data packets
- Writes data to shared data structure
- Logs connection events

### Data Manager Thread
- Reads sensor data from shared data structure
- Calculates running average of temperature
- Determines if temperature is too hot/cold
- Logs temperature events

### Storage Manager Thread
- Reads sensor data from shared data structure
- Connects to SQLite database
- Inserts data into database
- Handles database connection failures
- Logs database events

### Log Process
- Opens FIFO for reading
- Reads log events from FIFO
- Writes log events to log file with sequence number and timestamp

## License

This project is licensed under the MIT License - see the LICENSE file for details.
