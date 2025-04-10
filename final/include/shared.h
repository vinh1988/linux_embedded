#ifndef SHARED_H
#define SHARED_H

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <time.h>

/* Constants */
#define MAX_SENSOR_NODES 100
#define MAX_BUFFER_SIZE 1024
#define FIFO_NAME "logFifo"
#define LOG_FILE "gateway.log"

/* Temperature thresholds */
#define TEMP_TOO_HOT 30.0
#define TEMP_TOO_COLD 10.0

/* Sensor data structure */
typedef struct {
    uint16_t node_id;
    uint16_t temperature;
    uint16_t humidity;
    uint16_t light;
    time_t timestamp;
} sensor_data_t;

/* Shared data structure */
typedef struct {
    sensor_data_t data[MAX_SENSOR_NODES];
    int count;
    pthread_mutex_t mutex;
} shared_data_t;

/* Log event types */
typedef enum {
    LOG_CONNECTION_OPENED,
    LOG_CONNECTION_CLOSED,
    LOG_TOO_COLD,
    LOG_TOO_HOT,
    LOG_INVALID_SENSOR_ID,
    LOG_SQL_CONNECTED,
    LOG_SQL_TABLE_CREATED,
    LOG_SQL_CONNECTION_LOST,
    LOG_SQL_CONNECTION_FAILED
} log_event_type_t;

/* Log event structure */
typedef struct {
    log_event_type_t type;
    uint16_t node_id;
    float value;
    char message[MAX_BUFFER_SIZE];
} log_event_t;

/* Function prototypes */
void write_log_event(int fifo_fd, log_event_type_t type, uint16_t node_id, float value, const char *message);

#endif /* SHARED_H */
