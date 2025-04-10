#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <pthread.h>
#include "../include/shared.h"
#include "../include/sensor_data.h"

/* Thread function prototypes */
void *connection_manager_thread(void *arg);
void *data_manager_thread(void *arg);
void *storage_manager_thread(void *arg);

/* Global variables */
shared_data_t shared_data;
int fifo_fd;
int port;
volatile int running = 1;

/* Signal handler for graceful shutdown */
void signal_handler(int sig) {
    printf("Received signal %d, shutting down...\n", sig);
    running = 0;
}

int main(int argc, char *argv[]) {
    pthread_t conn_thread, data_thread, storage_thread;
    pid_t log_pid;
    int status;

    /* Check command line arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    /* Parse port number */
    port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number: %s\n", argv[1]);
        return 1;
    }

    /* Set up signal handler */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /* Initialize shared data */
    memset(&shared_data, 0, sizeof(shared_data));
    if (pthread_mutex_init(&shared_data.mutex, NULL) != 0) {
        perror("pthread_mutex_init");
        return 1;
    }

    /* Create FIFO for log events if it doesn't exist */
    if (access(FIFO_NAME, F_OK) == -1) {
        if (mkfifo(FIFO_NAME, 0666) != 0) {
            perror("mkfifo");
            return 1;
        }
    }

    /* Open FIFO for writing */
    fifo_fd = open(FIFO_NAME, O_WRONLY | O_NONBLOCK);
    if (fifo_fd == -1) {
        /* FIFO might not be opened for reading yet, we'll try again later */
        printf("FIFO not ready yet, will try again later\n");
    }

    /* Fork to create log process */
    log_pid = fork();
    if (log_pid < 0) {
        perror("fork");
        return 1;
    } else if (log_pid == 0) {
        /* Child process (log process) */
        execl("./log_process", "log_process", NULL);
        perror("execl");
        return 1;
    }

    /* Parent process continues here */
    printf("Log process started with PID: %d\n", log_pid);

    /* If FIFO wasn't ready before, try again now */
    if (fifo_fd == -1) {
        sleep(1); /* Give log process time to start */
        fifo_fd = open(FIFO_NAME, O_WRONLY);
        if (fifo_fd == -1) {
            perror("open FIFO");
            kill(log_pid, SIGTERM);
            return 1;
        }
    }

    /* Create threads */
    if (pthread_create(&conn_thread, NULL, connection_manager_thread, NULL) != 0) {
        perror("pthread_create (connection manager)");
        kill(log_pid, SIGTERM);
        return 1;
    }

    if (pthread_create(&data_thread, NULL, data_manager_thread, NULL) != 0) {
        perror("pthread_create (data manager)");
        running = 0;
        pthread_join(conn_thread, NULL);
        kill(log_pid, SIGTERM);
        return 1;
    }

    if (pthread_create(&storage_thread, NULL, storage_manager_thread, NULL) != 0) {
        perror("pthread_create (storage manager)");
        running = 0;
        pthread_join(conn_thread, NULL);
        pthread_join(data_thread, NULL);
        kill(log_pid, SIGTERM);
        return 1;
    }

    /* Wait for threads to complete */
    pthread_join(conn_thread, NULL);
    pthread_join(data_thread, NULL);
    pthread_join(storage_thread, NULL);

    /* Clean up */
    pthread_mutex_destroy(&shared_data.mutex);
    close(fifo_fd);

    /* Wait for log process to exit */
    waitpid(log_pid, &status, 0);

    printf("Sensor gateway shutdown complete\n");
    return 0;
}

/* Function to write log events to FIFO */
void write_log_event(int fifo_fd, log_event_type_t type, uint16_t node_id, float value, const char *message) {
    log_event_t event;
    
    if (fifo_fd == -1) {
        return; /* FIFO not available */
    }
    
    /* Prepare log event */
    event.type = type;
    event.node_id = node_id;
    event.value = value;
    if (message) {
        strncpy(event.message, message, MAX_BUFFER_SIZE - 1);
        event.message[MAX_BUFFER_SIZE - 1] = '\0';
    } else {
        event.message[0] = '\0';
    }
    
    /* Write to FIFO */
    pthread_mutex_lock(&shared_data.mutex); /* Use shared mutex for FIFO access */
    write(fifo_fd, &event, sizeof(event));
    pthread_mutex_unlock(&shared_data.mutex);
}
