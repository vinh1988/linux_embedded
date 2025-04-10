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
            sprintf(buffer, "A sensor node with %d has opened a new connection", event->node_id);
            break;
        case LOG_CONNECTION_CLOSED:
            sprintf(buffer, "The sensor node with %d has closed the connection", event->node_id);
            break;
        case LOG_TOO_COLD:
            sprintf(buffer, "The sensor node with %d reports it's too cold (running avg temperature = %.1f)", 
                   event->node_id, event->value);
            break;
        case LOG_TOO_HOT:
            sprintf(buffer, "The sensor node with %d reports it's too hot (running avg temperature = %.1f)", 
                   event->node_id, event->value);
            break;
        case LOG_INVALID_SENSOR_ID:
            sprintf(buffer, "Received sensor data with invalid sensor node ID %d", event->node_id);
            break;
        case LOG_SQL_CONNECTED:
            strcpy(buffer, "Connection to SQL server established.");
            break;
        case LOG_SQL_TABLE_CREATED:
            sprintf(buffer, "New table %s created.", event->message);
            break;
        case LOG_SQL_CONNECTION_LOST:
            strcpy(buffer, "Connection to SQL server lost.");
            break;
        case LOG_SQL_CONNECTION_FAILED:
            strcpy(buffer, "Unable to connect to SQL server.");
            break;
        default:
            if (event->message[0] != '\0') {
                strncpy(buffer, event->message, MAX_BUFFER_SIZE - 1);
                buffer[MAX_BUFFER_SIZE - 1] = '\0';
            } else {
                strcpy(buffer, "Unknown event");
            }
            break;
    }
    
    return buffer;
}

int main() {
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
