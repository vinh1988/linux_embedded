#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../include/shared.h"

/* Global variables */
volatile int running = 1;

/* FIFO and log file names with port */
char FIFO_NAME[64];
char LOG_FILE[64];

/* Signal handler for graceful shutdown */
void signal_handler(int sig) {
    printf("Log process received signal %d, shutting down...\n", sig);
    running = 0;
}

/* Function to get log message based on event type */
const char *get_log_message(log_event_t *event) {
    static char buffer[MAX_BUFFER_SIZE];

    switch (event->type) {
        case LOG_CONNECTION_OPENED:
            snprintf(buffer, MAX_BUFFER_SIZE, "A sensor node with %d has opened a new connection", event->node_id);
            break;
        case LOG_CONNECTION_CLOSED:
            snprintf(buffer, MAX_BUFFER_SIZE, "The sensor node with %d has closed the connection", event->node_id);
            break;
        case LOG_TOO_COLD:
            snprintf(buffer, MAX_BUFFER_SIZE, "The sensor node with %d reports it's too cold (running avg temperature = %.1f)",
                   event->node_id, event->value);
            break;
        case LOG_TOO_HOT:
            snprintf(buffer, MAX_BUFFER_SIZE, "The sensor node with %d reports it's too hot (running avg temperature = %.1f)",
                   event->node_id, event->value);
            break;
        case LOG_INVALID_SENSOR_ID:
            snprintf(buffer, MAX_BUFFER_SIZE, "Received sensor data with invalid sensor node ID %d", event->node_id);
            break;
        case LOG_SQL_CONNECTED:
            strncpy(buffer, "Connection to SQL server established.", MAX_BUFFER_SIZE - 1);
            buffer[MAX_BUFFER_SIZE - 1] = '\0';
            break;
        case LOG_SQL_TABLE_CREATED:
            /* Use snprintf to prevent buffer overflow */
            /* Truncate event->message if needed to prevent buffer overflow */
            {
                char truncated_msg[MAX_BUFFER_SIZE - 20]; /* Reserve space for "New table " and "created." */
                strncpy(truncated_msg, event->message, sizeof(truncated_msg) - 1);
                truncated_msg[sizeof(truncated_msg) - 1] = '\0';
                snprintf(buffer, MAX_BUFFER_SIZE, "New table %s created.", truncated_msg);
            }
            break;
        case LOG_SQL_CONNECTION_LOST:
            strncpy(buffer, "Connection to SQL server lost.", MAX_BUFFER_SIZE - 1);
            buffer[MAX_BUFFER_SIZE - 1] = '\0';
            break;
        case LOG_SQL_CONNECTION_FAILED:
            strncpy(buffer, "Unable to connect to SQL server.", MAX_BUFFER_SIZE - 1);
            buffer[MAX_BUFFER_SIZE - 1] = '\0';
            break;
        default:
            if (event->message[0] != '\0') {
                strncpy(buffer, event->message, MAX_BUFFER_SIZE - 1);
                buffer[MAX_BUFFER_SIZE - 1] = '\0';
            } else {
                strncpy(buffer, "Unknown event", MAX_BUFFER_SIZE - 1);
                buffer[MAX_BUFFER_SIZE - 1] = '\0';
            }
            break;
    }

    return buffer;
}

int main(int argc, char *argv[]) {
    int port = 1234; /* Default port */

    /* Parse command line arguments */
    if (argc > 1) {
        port = atoi(argv[1]);
    }

    /* Set FIFO and log file names based on port */
    snprintf(FIFO_NAME, sizeof(FIFO_NAME), "%s_%d", FIFO_BASE_NAME, port);
    snprintf(LOG_FILE, sizeof(LOG_FILE), "%s_%d", LOG_BASE_NAME, port);

    printf("Log process starting for port %d\n", port);
    printf("Using FIFO: %s\n", FIFO_NAME);
    printf("Using log file: %s\n", LOG_FILE);
    int fifo_fd;
    FILE *log_file;
    log_event_t event;
    int sequence_number = 1;

    /* Set up signal handler */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /* Create FIFO if it doesn't exist */
    if (access(FIFO_NAME, F_OK) == -1) {
        if (mkfifo(FIFO_NAME, 0666) != 0) {
            perror("mkfifo");
            return 1;
        }
    }

    /* Open FIFO for reading */
    fifo_fd = open(FIFO_NAME, O_RDONLY);
    if (fifo_fd == -1) {
        perror("open FIFO");
        return 1;
    }

    /* Open log file */
    log_file = fopen(LOG_FILE, "a");
    if (!log_file) {
        perror("fopen");
        close(fifo_fd);
        return 1;
    }

    printf("Log process started\n");

    /* Main loop */
    while (running) {
        ssize_t bytes_read;

        /* Read log event from FIFO */
        bytes_read = read(fifo_fd, &event, sizeof(event));
        if (bytes_read == -1) {
            perror("read");
            break;
        } else if (bytes_read == 0) {
            /* FIFO closed by writer */
            usleep(100000); /* 100ms */
            continue;
        } else if (bytes_read != sizeof(event)) {
            /* Partial read */
            fprintf(stderr, "Partial read from FIFO\n");
            continue;
        }

        /* Get current timestamp */
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

        /* Get log message */
        const char *message = get_log_message(&event);

        /* Write to log file */
        fprintf(log_file, "%d %s %s\n", sequence_number++, timestamp, message);
        fflush(log_file);

        /* Also print to stdout for debugging */
        printf("LOG: %d %s %s\n", sequence_number - 1, timestamp, message);
    }

    /* Clean up */
    fclose(log_file);
    close(fifo_fd);

    printf("Log process exiting\n");
    return 0;
}
