#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <sys/resource.h>
#include <pthread.h>
#include "../include/shared.h"

/* Global variables */
extern int fifo_fd;
extern volatile int running;
system_status_t system_status;

/* Local variables */
static pthread_t status_thread;

/* Forward declarations */
static void *status_manager_thread(void *arg);
static float get_cpu_usage();
static uint64_t get_memory_usage();

/* Initialize the system status manager */
int init_status_manager() {
    /* Initialize system status */
    memset(&system_status, 0, sizeof(system_status));
    system_status.start_time = time(NULL);

    if (pthread_mutex_init(&system_status.mutex, NULL) != 0) {
        perror("pthread_mutex_init (system_status)");
        return -1;
    }

    /* Create status manager thread */
    if (pthread_create(&status_thread, NULL, status_manager_thread, NULL) != 0) {
        perror("pthread_create (status_manager)");
        pthread_mutex_destroy(&system_status.mutex);
        return -1;
    }

    return 0;
}

/* Clean up the system status manager */
void cleanup_status_manager() {
    /* Stop the thread */
    if (status_thread) {
        pthread_cancel(status_thread);
        pthread_join(status_thread, NULL);
    }

    /* Destroy mutex */
    pthread_mutex_destroy(&system_status.mutex);
}

/* Status manager thread function */
static void *status_manager_thread(void *arg) {
    /* Unused parameter */
    (void)arg;

    printf("Status manager thread starting...\n");

    /* Set thread to be cancelable */
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    /* Main loop */
    while (running) {
        /* Update system status */
        pthread_mutex_lock(&system_status.mutex);

        /* Update CPU and memory usage */
        system_status.cpu_usage = get_cpu_usage();
        system_status.memory_usage = get_memory_usage();

        /* Log system status periodically */
        char msg[256];
        snprintf(msg, sizeof(msg),
                "System status: Connections=%u, CPU=%.1f%%, Memory=%lu KB",
                system_status.active_connections,
                system_status.cpu_usage,
                system_status.memory_usage / 1024);

        pthread_mutex_unlock(&system_status.mutex);

        write_log_event(fifo_fd, LOG_SYSTEM_STATUS, 0, 0.0, msg);

        /* Sleep for the update interval */
        sleep(STATUS_UPDATE_INTERVAL);
    }

    printf("Status manager thread exiting\n");
    return NULL;
}

/* Initialize system status */
void init_system_status(system_status_t *status) {
    if (!status) return;

    memset(status, 0, sizeof(system_status_t));
    status->start_time = time(NULL);
    pthread_mutex_init(&status->mutex, NULL);
}

/* Update system status with connection information */
void update_system_status(system_status_t *status, connection_list_t *connections) {
    if (!status || !connections) return;

    pthread_mutex_lock(&status->mutex);
    pthread_mutex_lock(&connections->mutex);

    status->active_connections = connections->count;

    /* Calculate total statistics */
    status->total_messages_received = 0;
    status->total_messages_sent = 0;
    status->total_bytes_received = 0;
    status->total_bytes_sent = 0;
    status->total_errors = 0;

    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections->connections[i]) {
            connection_session_t *session = connections->connections[i];

            pthread_mutex_lock(&session->mutex);
            status->total_messages_received += session->msg_received;
            status->total_messages_sent += session->msg_sent;
            status->total_bytes_received += session->bytes_received;
            status->total_bytes_sent += session->bytes_sent;
            status->total_errors += session->errors;
            pthread_mutex_unlock(&session->mutex);
        }
    }

    /* Update CPU and memory usage */
    status->cpu_usage = get_cpu_usage();
    status->memory_usage = get_memory_usage();

    pthread_mutex_unlock(&connections->mutex);
    pthread_mutex_unlock(&status->mutex);
}

/* Print system status */
void print_system_status(system_status_t *status) {
    if (!status) return;

    pthread_mutex_lock(&status->mutex);

    time_t now = time(NULL);
    time_t uptime = now - status->start_time;

    printf("\n=== System Status ===\n");
    printf("Uptime: %ld days, %ld hours, %ld minutes, %ld seconds\n",
           uptime / 86400, (uptime % 86400) / 3600, (uptime % 3600) / 60, uptime % 60);
    printf("Active connections: %u\n", status->active_connections);
    printf("Total connections: %u\n", status->total_connections);
    printf("Messages received: %u\n", status->total_messages_received);
    printf("Messages sent: %u\n", status->total_messages_sent);
    printf("Bytes received: %u\n", status->total_bytes_received);
    printf("Bytes sent: %u\n", status->total_bytes_sent);
    printf("Errors: %u\n", status->total_errors);
    printf("CPU usage: %.1f%%\n", status->cpu_usage);
    printf("Memory usage: %lu KB\n", status->memory_usage / 1024);
    printf("=====================\n\n");

    pthread_mutex_unlock(&status->mutex);
}

/* Print connection statistics */
void print_connection_stats(connection_list_t *list) {
    if (!list) return;

    pthread_mutex_lock(&list->mutex);

    printf("\n=== Connection Statistics ===\n");
    printf("Total active connections: %d\n", list->count);

    /* Count connections by state */
    int state_counts[7] = {0}; /* One for each CONN_STATE_* value */
    int auth_counts[4] = {0};  /* One for each AUTH_STATE_* value */

    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (list->connections[i]) {
            connection_session_t *session = list->connections[i];
            pthread_mutex_lock(&session->mutex);

            /* Count by connection state */
            if (session->state >= CONN_STATE_NEW && session->state <= CONN_STATE_CLOSING) {
                state_counts[session->state]++;
            }

            /* Count by auth state */
            if (session->auth_state >= AUTH_STATE_NONE && session->auth_state <= AUTH_STATE_FAILED) {
                auth_counts[session->auth_state]++;
            }

            pthread_mutex_unlock(&session->mutex);
        }
    }

    /* Print summary of connection states */
    if (list->count > 0) {
        printf("\nConnection States:\n");
        if (state_counts[CONN_STATE_NEW] > 0)
            printf("  New: %d\n", state_counts[CONN_STATE_NEW]);
        if (state_counts[CONN_STATE_AUTHENTICATING] > 0)
            printf("  Authenticating: %d\n", state_counts[CONN_STATE_AUTHENTICATING]);
        if (state_counts[CONN_STATE_AUTHENTICATED] > 0)
            printf("  Authenticated: %d\n", state_counts[CONN_STATE_AUTHENTICATED]);
        if (state_counts[CONN_STATE_ACTIVE] > 0)
            printf("  Active: %d\n", state_counts[CONN_STATE_ACTIVE]);
        if (state_counts[CONN_STATE_INACTIVE] > 0)
            printf("  Inactive: %d\n", state_counts[CONN_STATE_INACTIVE]);
        if (state_counts[CONN_STATE_ERROR] > 0)
            printf("  Error: %d\n", state_counts[CONN_STATE_ERROR]);
        if (state_counts[CONN_STATE_CLOSING] > 0)
            printf("  Closing: %d\n", state_counts[CONN_STATE_CLOSING]);

        printf("\nAuthentication States:\n");
        if (auth_counts[AUTH_STATE_NONE] > 0)
            printf("  None: %d\n", auth_counts[AUTH_STATE_NONE]);
        if (auth_counts[AUTH_STATE_PENDING] > 0)
            printf("  Pending: %d\n", auth_counts[AUTH_STATE_PENDING]);
        if (auth_counts[AUTH_STATE_SUCCESS] > 0)
            printf("  Success: %d\n", auth_counts[AUTH_STATE_SUCCESS]);
        if (auth_counts[AUTH_STATE_FAILED] > 0)
            printf("  Failed: %d\n", auth_counts[AUTH_STATE_FAILED]);

        printf("\nConnection Details:\n");
    }

    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (list->connections[i]) {
            connection_session_t *session = list->connections[i];

            pthread_mutex_lock(&session->mutex);

            time_t now = time(NULL);
            time_t conn_time = now - session->connect_time;
            time_t idle_time = now - session->last_activity;

            printf("Connection ID: %u\n", session->connection_id);
            printf("  IP: %s:%d\n", session->ip_str, session->port);
            /* Convert state to descriptive text */
            const char *state_str = "Unknown";
            switch (session->state) {
                case CONN_STATE_NEW: state_str = "New"; break;
                case CONN_STATE_AUTHENTICATING: state_str = "Authenticating"; break;
                case CONN_STATE_AUTHENTICATED: state_str = "Authenticated"; break;
                case CONN_STATE_ACTIVE: state_str = "Active"; break;
                case CONN_STATE_INACTIVE: state_str = "Inactive"; break;
                case CONN_STATE_ERROR: state_str = "Error"; break;
                case CONN_STATE_CLOSING: state_str = "Closing"; break;
            }

            /* Convert auth state to descriptive text */
            const char *auth_str = "Unknown";
            switch (session->auth_state) {
                case AUTH_STATE_NONE: auth_str = "None"; break;
                case AUTH_STATE_PENDING: auth_str = "Pending"; break;
                case AUTH_STATE_SUCCESS: auth_str = "Success"; break;
                case AUTH_STATE_FAILED: auth_str = "Failed"; break;
            }

            printf("  State: %s\n", state_str);
            printf("  Auth state: %s\n", auth_str);

            /* Show username if authenticated */
            if (session->auth_state == AUTH_STATE_SUCCESS && session->username[0] != '\0') {
                printf("  Username: %s\n", session->username);
            }
            printf("  Connected for: %ld minutes, %ld seconds\n", conn_time / 60, conn_time % 60);
            printf("  Idle for: %ld seconds\n", idle_time);
            printf("  Messages received: %u\n", session->msg_received);
            printf("  Messages sent: %u\n", session->msg_sent);
            printf("  Bytes received: %u\n", session->bytes_received);
            printf("  Bytes sent: %u\n", session->bytes_sent);
            printf("  Errors: %u\n", session->errors);
            printf("\n");

            pthread_mutex_unlock(&session->mutex);
        }
    }

    printf("===========================\n\n");

    pthread_mutex_unlock(&list->mutex);
}

/* Get CPU usage */
static float get_cpu_usage() {
    /* In a real implementation, this would read /proc/stat and calculate CPU usage */
    /* For simplicity, we'll return a random value between 0 and 100 */
    return (float)(rand() % 100);
}

/* Get memory usage */
static uint64_t get_memory_usage() {
    /* Get process memory usage using getrusage */
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return (uint64_t)usage.ru_maxrss;
    }

    return 0;
}
