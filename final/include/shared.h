#ifndef SHARED_H
#define SHARED_H

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <time.h>
#include <netinet/in.h>
#include <stdbool.h>

/* Constants */
#define MAX_SENSOR_NODES 1000  /* Increased from 100 to support more connections */
#define MAX_BUFFER_SIZE 1024
#define FIFO_NAME "logFifo"
#define LOG_FILE "gateway.log"

/* Connection management constants */
#define MAX_CONNECTIONS 1000
#define MAX_CONNECTIONS_PER_IP 10
#define CONNECTION_TIMEOUT 300  /* seconds */
#define MAX_RETRY_ATTEMPTS 3
#define RETRY_DELAY 5  /* seconds */

/* Security constants */
#define MAX_USERNAME_LENGTH 32
#define MAX_PASSWORD_LENGTH 64
#define AUTH_TIMEOUT 60  /* seconds */

/* System status constants */
#define STATUS_UPDATE_INTERVAL 5  /* seconds */

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

/* Connection state */
typedef enum {
    CONN_STATE_NEW,
    CONN_STATE_AUTHENTICATING,
    CONN_STATE_AUTHENTICATED,
    CONN_STATE_ACTIVE,
    CONN_STATE_INACTIVE,
    CONN_STATE_ERROR,
    CONN_STATE_CLOSING
} connection_state_t;

/* Authentication state */
typedef enum {
    AUTH_STATE_NONE,
    AUTH_STATE_PENDING,
    AUTH_STATE_SUCCESS,
    AUTH_STATE_FAILED
} auth_state_t;

/* Connection session structure */
typedef struct {
    int socket_fd;
    uint32_t connection_id;
    struct sockaddr_in addr;
    char ip_str[INET_ADDRSTRLEN];
    int port;
    time_t connect_time;
    time_t last_activity;
    connection_state_t state;
    auth_state_t auth_state;
    char username[MAX_USERNAME_LENGTH];
    uint32_t msg_received;
    uint32_t msg_sent;
    uint32_t bytes_received;
    uint32_t bytes_sent;
    uint32_t errors;
    pthread_mutex_t mutex;
} connection_session_t;

/* Connection list structure */
typedef struct {
    connection_session_t *connections[MAX_CONNECTIONS];
    int count;
    pthread_mutex_t mutex;
} connection_list_t;

/* System status structure */
typedef struct {
    time_t start_time;
    uint32_t total_connections;
    uint32_t active_connections;
    uint32_t total_messages_received;
    uint32_t total_messages_sent;
    uint32_t total_bytes_received;
    uint32_t total_bytes_sent;
    uint32_t total_errors;
    float cpu_usage;
    uint64_t memory_usage;
    pthread_mutex_t mutex;
} system_status_t;

/* Log event types */
typedef enum {
    LOG_CONNECTION_OPENED,
    LOG_CONNECTION_CLOSED,
    LOG_CONNECTION_TIMEOUT,
    LOG_CONNECTION_ERROR,
    LOG_AUTH_SUCCESS,
    LOG_AUTH_FAILED,
    LOG_TOO_COLD,
    LOG_TOO_HOT,
    LOG_INVALID_SENSOR_ID,
    LOG_SQL_CONNECTED,
    LOG_SQL_TABLE_CREATED,
    LOG_SQL_CONNECTION_LOST,
    LOG_SQL_CONNECTION_FAILED,
    LOG_SYSTEM_STATUS,
    LOG_SECURITY_VIOLATION
} log_event_type_t;

/* Log event structure */
typedef struct {
    log_event_type_t type;
    uint16_t node_id;
    float value;
    char message[MAX_BUFFER_SIZE];
} log_event_t;

/* Function prototypes */

/* Logging functions */
void write_log_event(int fifo_fd, log_event_type_t type, uint16_t node_id, float value, const char *message);

/* Connection management functions */
connection_session_t* create_connection_session(int socket_fd, struct sockaddr_in *addr);
void free_connection_session(connection_session_t *session);
int add_connection(connection_list_t *list, connection_session_t *session);
int remove_connection(connection_list_t *list, uint32_t connection_id);
connection_session_t* find_connection_by_id(connection_list_t *list, uint32_t connection_id);
connection_session_t* find_connection_by_socket(connection_list_t *list, int socket_fd);
void update_connection_activity(connection_session_t *session);
void check_connection_timeouts(connection_list_t *list);

/* Authentication functions */
bool authenticate_connection(connection_session_t *session, const char *username, const char *password);
void set_connection_authenticated(connection_session_t *session, bool authenticated);

/* System status functions */
void init_system_status(system_status_t *status);
void update_system_status(system_status_t *status, connection_list_t *connections);
void print_system_status(system_status_t *status);
void print_connection_stats(connection_list_t *list);

/* Security functions */
bool check_connection_limit(const char *ip_address, connection_list_t *list);
bool is_ip_blacklisted(const char *ip_address);
void add_ip_to_blacklist(const char *ip_address);

#endif /* SHARED_H */
