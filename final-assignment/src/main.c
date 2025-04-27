#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <getopt.h>
#include "../include/shared.h"

/* Include sensor data header */
#include "../include/sensor_data.h"

/* Function prototypes */
void *connection_manager_thread(void *arg);
void *data_manager_thread(void *arg);
void *storage_manager_thread(void *arg);

/* New components */
int init_status_manager();
void cleanup_status_manager();
int init_security_manager();
void cleanup_security_manager();
void print_system_status(system_status_t *status);
void print_connection_stats(connection_list_t *list);
connection_list_t* get_connection_list(); /* Added function to get connection list */

/* Command handlers */
void handle_status_command();
void handle_stats_command();
void handle_exit_command();

/* Global variables */
shared_data_t shared_data;
int fifo_fd;
int port;
volatile int running = 1;
extern system_status_t system_status;

/* FIFO and log file names with port */
char FIFO_NAME[64];
char LOG_FILE[64];

/* Signal handler for graceful shutdown */
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
void signal_handler(int sig) {
    printf("Received signal %d, shutting down...\n", sig);
    running = 0;
}

/* Command line options */
static struct option long_options[] = {
    {"port", required_argument, 0, 'p'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
};

/* Print usage information */
void print_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS] [PORT]\n\n", program_name);
    printf("Options:\n");
    printf("  -p, --port PORT    Port to listen on (default: 1234)\n");
    printf("  -h, --help         Display this help message\n");
    printf("\n");
    printf("You can also specify the port as a positional argument:\n");
    printf("  %s 5000           Start server on port 5000\n", program_name);
    printf("\n");
    printf("Commands (during runtime):\n");
    printf("  status             Display system status\n");
    printf("  stats              Display connection statistics\n");
    printf("  exit               Exit the program\n");
}

/* Command input thread function */
void *command_input_thread(void *arg) {
    /* Unused parameter */
    (void)arg;

    char command[100];

    printf("\nEnter commands (status, stats, exit):\n");

    while (running) {
        printf("> ");
        fflush(stdout);

        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }

        /* Remove newline */
        size_t len = strlen(command);
        if (len > 0 && command[len - 1] == '\n') {
            command[len - 1] = '\0';
        }

        /* Process command */
        if (strcmp(command, "status") == 0) {
            handle_status_command();
        } else if (strcmp(command, "stats") == 0) {
            handle_stats_command();
        } else if (strcmp(command, "exit") == 0) {
            handle_exit_command();
            break;
        } else if (strlen(command) > 0) {
            printf("Unknown command: %s\n", command);
        }
    }

    return NULL;
}

/* Handle status command */
void handle_status_command() {
    print_system_status(&system_status);
}

/* Handle stats command */
void handle_stats_command() {
    /* Get connection list and print statistics */
    connection_list_t *conn_list = get_connection_list();
    if (conn_list) {
        print_connection_stats(conn_list);
    } else {
        printf("Failed to get connection list\n");
    }
}

/* Handle exit command */
void handle_exit_command() {
    printf("Shutting down...\n");
    running = 0;
}

int main(int argc, char *argv[]) {
    pthread_t conn_thread, data_thread, storage_thread, cmd_thread;
    pid_t log_pid;
    int status, opt, option_index = 0;

    /* Set default port */
    port = 1234;

    /* Parse command line options */
    while ((opt = getopt_long(argc, argv, "p:h", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                if (port <= 0 || port > 65535) {
                    fprintf(stderr, "Invalid port number: %s\n", optarg);
                    return 1;
                }
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    /* Check for positional arguments (port number without -p flag) */
    if (optind < argc) {
        /* Use the first non-option argument as port */
        port = atoi(argv[optind]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Invalid port number: %s\n", argv[optind]);
            return 1;
        }
    }

    /* Set FIFO and log file names based on port */
    snprintf(FIFO_NAME, sizeof(FIFO_NAME), "%s_%d", FIFO_BASE_NAME, port);
    snprintf(LOG_FILE, sizeof(LOG_FILE), "%s_%d", LOG_BASE_NAME, port);

    /* Set up signal handler */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    printf("Sensor Gateway starting on port %d...\n", port);

    /* Initialize shared data */
    memset(&shared_data, 0, sizeof(shared_data));
    if (pthread_mutex_init(&shared_data.mutex, NULL) != 0) {
        perror("pthread_mutex_init");
        return 1;
    }

    /* Initialize security manager */
    if (init_security_manager() != 0) {
        fprintf(stderr, "Failed to initialize security manager\n");
        return 1;
    }

    /* Initialize status manager */
    if (init_status_manager() != 0) {
        fprintf(stderr, "Failed to initialize status manager\n");
        cleanup_security_manager();
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
        char port_str[16];
        snprintf(port_str, sizeof(port_str), "%d", port);
        execl("./log_process", "log_process", port_str, NULL);
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
        cleanup_status_manager();
        cleanup_security_manager();
        kill(log_pid, SIGTERM);
        return 1;
    }

    if (pthread_create(&data_thread, NULL, data_manager_thread, NULL) != 0) {
        perror("pthread_create (data manager)");
        running = 0;
        pthread_join(conn_thread, NULL);
        cleanup_status_manager();
        cleanup_security_manager();
        kill(log_pid, SIGTERM);
        return 1;
    }

    if (pthread_create(&storage_thread, NULL, storage_manager_thread, NULL) != 0) {
        perror("pthread_create (storage manager)");
        running = 0;
        pthread_join(conn_thread, NULL);
        pthread_join(data_thread, NULL);
        cleanup_status_manager();
        cleanup_security_manager();
        kill(log_pid, SIGTERM);
        return 1;
    }

    /* Create command input thread */
    if (pthread_create(&cmd_thread, NULL, command_input_thread, NULL) != 0) {
        perror("pthread_create (command input)");
        running = 0;
        pthread_join(conn_thread, NULL);
        pthread_join(data_thread, NULL);
        pthread_join(storage_thread, NULL);
        cleanup_status_manager();
        cleanup_security_manager();
        kill(log_pid, SIGTERM);
        return 1;
    }

    /* Wait for threads to complete */
    pthread_join(conn_thread, NULL);
    pthread_join(data_thread, NULL);
    pthread_join(storage_thread, NULL);
    pthread_join(cmd_thread, NULL);

    /* Clean up */
    cleanup_status_manager();
    cleanup_security_manager();
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
