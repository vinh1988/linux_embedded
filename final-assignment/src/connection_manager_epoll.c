#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <time.h>
#include <signal.h>
#include "../include/shared.h"
#include "../include/sensor_data.h"

/* Maximum events to handle at once in epoll */
#define MAX_EPOLL_EVENTS 64

/* Global variables */
extern shared_data_t shared_data;
extern int fifo_fd;
extern int port;
extern volatile int running;
extern system_status_t system_status;

/* Local variables */
connection_list_t connection_list; /* Changed from static to allow access from main.c */
static int epoll_fd;
static uint32_t next_connection_id = 1;

/* Forward declarations of local functions */
static void handle_new_connection(int server_fd);
static void handle_client_data(connection_session_t *session);
static void close_connection(connection_session_t *session);
static int make_socket_non_blocking(int socket_fd);
static void check_timeouts();
static bool is_ip_blacklisted_local(const char *ip_address);
static void add_ip_to_blacklist_local(const char *ip_address);

/* Initialize the connection manager */
int init_connection_manager() {
    /* Initialize connection list */
    memset(&connection_list, 0, sizeof(connection_list));
    if (pthread_mutex_init(&connection_list.mutex, NULL) != 0) {
        perror("pthread_mutex_init (connection_list)");
        return -1;
    }

    /* Create epoll instance */
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        return -1;
    }

    return 0;
}

/* Clean up the connection manager */
void cleanup_connection_manager() {
    /* Close all connections */
    pthread_mutex_lock(&connection_list.mutex);
    for (int i = 0; i < connection_list.count; i++) {
        if (connection_list.connections[i]) {
            close_connection(connection_list.connections[i]);
        }
    }
    pthread_mutex_unlock(&connection_list.mutex);

    /* Clean up epoll */
    if (epoll_fd != -1) {
        close(epoll_fd);
    }

    /* Destroy mutex */
    pthread_mutex_destroy(&connection_list.mutex);
}

/* Connection manager thread function */
void *connection_manager_thread(void *arg) {
    /* Unused parameter */
    (void)arg;

    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    struct epoll_event event, events[MAX_EPOLL_EVENTS];
    time_t last_timeout_check = time(NULL);

    printf("Connection manager thread starting...\n");

    /* Initialize connection manager */
    if (init_connection_manager() != 0) {
        printf("Failed to initialize connection manager\n");
        running = 0;
        return NULL;
    }

    /* Create server socket */
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        running = 0;
        return NULL;
    }

    /* Set socket options */
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        running = 0;
        return NULL;
    }

    /* Make server socket non-blocking */
    if (make_socket_non_blocking(server_fd) == -1) {
        perror("make_socket_non_blocking");
        close(server_fd);
        running = 0;
        return NULL;
    }

    /* Configure server address */
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    /* Bind socket */
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        running = 0;
        return NULL;
    }

    /* Listen for connections */
    if (listen(server_fd, SOMAXCONN) < 0) {
        perror("listen");
        close(server_fd);
        running = 0;
        return NULL;
    }

    /* Add server socket to epoll */
    event.events = EPOLLIN;
    event.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1) {
        perror("epoll_ctl: server_fd");
        close(server_fd);
        running = 0;
        return NULL;
    }

    printf("Connection manager listening on port %d\n", port);

    /* Main event loop */
    while (running) {
        int n, i;

        /* Check for connection timeouts periodically */
        time_t now = time(NULL);
        if (now - last_timeout_check >= 10) {  /* Check every 10 seconds */
            check_timeouts();
            last_timeout_check = now;
        }

        /* Wait for events with a timeout of 1 second */
        n = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENTS, 1000);

        /* Handle errors */
        if (n == -1) {
            if (errno == EINTR) {
                /* Interrupted by signal, just continue */
                continue;
            }
            perror("epoll_wait");
            break;
        }

        /* Process events */
        for (i = 0; i < n; i++) {
            /* Error or hang up */
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
                /* Find the connection and close it */
                connection_session_t *session = find_connection_by_socket(&connection_list, events[i].data.fd);
                if (session) {
                    close_connection(session);
                } else if (events[i].data.fd == server_fd) {
                    /* Server socket error */
                    perror("Server socket error");
                    running = 0;
                    break;
                }
                continue;
            }

            /* New connection */
            if (events[i].data.fd == server_fd) {
                handle_new_connection(server_fd);
                continue;
            }

            /* Data from existing connection */
            if (events[i].events & EPOLLIN) {
                connection_session_t *session = find_connection_by_socket(&connection_list, events[i].data.fd);
                if (session) {
                    handle_client_data(session);
                }
            }
        }
    }

    /* Clean up */
    close(server_fd);
    cleanup_connection_manager();

    printf("Connection manager thread exiting\n");
    return NULL;
}

/* Handle a new connection */
static void handle_new_connection(int server_fd) {
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int client_socket;

    /* Accept the connection */
    client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
    if (client_socket == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            /* No more connections to accept */
            return;
        }
        perror("accept");
        return;
    }

    /* Make the socket non-blocking */
    if (make_socket_non_blocking(client_socket) == -1) {
        perror("make_socket_non_blocking");
        close(client_socket);
        return;
    }

    /* Get client IP address as string */
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

    /* Check connection limits */
    if (!check_connection_limit(client_ip, &connection_list)) {
        /* Too many connections from this IP */
        printf("Connection limit exceeded for IP %s\n", client_ip);
        write_log_event(fifo_fd, LOG_SECURITY_VIOLATION, 0, 0.0,
                       "Connection limit exceeded");
        close(client_socket);
        return;
    }

    /* Check if IP is blacklisted */
    if (is_ip_blacklisted_local(client_ip)) {
        printf("Connection attempt from blacklisted IP %s\n", client_ip);
        write_log_event(fifo_fd, LOG_SECURITY_VIOLATION, 0, 0.0,
                       "Connection attempt from blacklisted IP");
        close(client_socket);
        return;
    }

    /* Create a new connection session */
    connection_session_t *session = create_connection_session(client_socket, &client_addr);
    if (!session) {
        perror("create_connection_session");
        close(client_socket);
        return;
    }

    /* Add to connection list */
    if (add_connection(&connection_list, session) != 0) {
        printf("Failed to add connection to list\n");
        free_connection_session(session);
        return;
    }

    /* Add to epoll */
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;  /* Edge-triggered */
    event.data.fd = client_socket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event) == -1) {
        perror("epoll_ctl: client_socket");
        remove_connection(&connection_list, session->connection_id);
        free_connection_session(session);
        return;
    }

    /* Log connection opened event */
    char msg[100];
    sprintf(msg, "A sensor node from %s:%d has opened a new connection",
            session->ip_str, session->port);
    write_log_event(fifo_fd, LOG_CONNECTION_OPENED, 0, 0.0, msg);

    printf("New connection from %s:%d (ID: %u)\n",
           session->ip_str, session->port, session->connection_id);

    /* Update system status */
    pthread_mutex_lock(&system_status.mutex);
    system_status.total_connections++;
    system_status.active_connections++;
    pthread_mutex_unlock(&system_status.mutex);
}

/* Handle data from a client */
static void handle_client_data(connection_session_t *session) {
    char buffer[1024];
    ssize_t bytes_read;

    /* Update last activity time */
    update_connection_activity(session);

    /* Read data from client */
    bytes_read = recv(session->socket_fd, buffer, sizeof(buffer) - 1, 0);

    /* Ensure null termination */
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
    }

    /* Handle errors and connection closed */
    if (bytes_read <= 0) {
        if (bytes_read == 0 || errno != EAGAIN) {
            /* Connection closed or error */
            if (bytes_read < 0) {
                perror("recv");
                pthread_mutex_lock(&session->mutex);
                session->errors++;

                /* If too many errors, consider blacklisting the IP */
                if (session->errors > 10) {
                    char ip_copy[INET_ADDRSTRLEN];
                    strncpy(ip_copy, session->ip_str, INET_ADDRSTRLEN);
                    ip_copy[INET_ADDRSTRLEN - 1] = '\0';
                    pthread_mutex_unlock(&session->mutex);

                    /* Log and blacklist */
                    char msg[100];
                    sprintf(msg, "IP %s blacklisted after %u connection errors",
                            ip_copy, session->errors);
                    write_log_event(fifo_fd, LOG_SECURITY_VIOLATION, 0, 0.0, msg);
                    add_ip_to_blacklist_local(ip_copy);
                } else {
                    pthread_mutex_unlock(&session->mutex);
                }
            } else {
                /* Normal close, no need to blacklist */
            }
            close_connection(session);
        }
        return;
    }

    /* Update statistics */
    pthread_mutex_lock(&session->mutex);
    session->bytes_received += bytes_read;
    session->msg_received++;
    pthread_mutex_unlock(&session->mutex);

    /* Update system status */
    pthread_mutex_lock(&system_status.mutex);
    system_status.total_messages_received++;
    system_status.total_bytes_received += bytes_read;
    pthread_mutex_unlock(&system_status.mutex);

    /* Check authentication state */
    pthread_mutex_lock(&session->mutex);
    auth_state_t auth_state = session->auth_state;
    pthread_mutex_unlock(&session->mutex);

    /* Process the received data based on the command */
    if (strncmp(buffer, "AUTH ", 5) == 0) {
        /* Authentication request */
        char username[32] = {0};
        char password[32] = {0};

        /* Parse username and password */
        if (sscanf(buffer + 5, "%31s %31s", username, password) == 2) {
            /* In a real implementation, we would validate credentials */
            /* For simplicity, we'll accept any credentials */
            set_connection_authenticated(session, true);

            /* Store username */
            pthread_mutex_lock(&session->mutex);
            strncpy(session->username, username, MAX_USERNAME_LENGTH - 1);
            session->username[MAX_USERNAME_LENGTH - 1] = '\0';
            pthread_mutex_unlock(&session->mutex);

            /* Log successful authentication */
            char msg[100];
            sprintf(msg, "User %s authenticated from %s", username, session->ip_str);
            write_log_event(fifo_fd, LOG_AUTH_SUCCESS, 0, 0.0, msg);
        } else {
            /* Invalid authentication format */
            set_connection_authenticated(session, false);

            /* Log failed authentication */
            char msg[100];
            sprintf(msg, "Invalid authentication format from %s", session->ip_str);
            write_log_event(fifo_fd, LOG_AUTH_FAILED, 0, 0.0, msg);
        }
        return;
    } else if (strncmp(buffer, "DATA ", 5) == 0) {
        /* Data packet */
        if (auth_state != AUTH_STATE_SUCCESS) {
            /* Authentication required but not completed */
            char msg[100];
            sprintf(msg, "Unauthenticated data received from %s", session->ip_str);
            write_log_event(fifo_fd, LOG_SECURITY_VIOLATION, 0, 0.0, msg);
            return;
        }

        /* Parse sensor data */
        int node_id;
        float temperature, humidity, pressure;
        unsigned int timestamp;

        if (sscanf(buffer + 5, "%d %f %f %f %u", &node_id, &temperature, &humidity, &pressure, &timestamp) != 5) {
            /* Invalid data format */
            char msg[100];
            sprintf(msg, "Invalid data format from %s", session->ip_str);
            write_log_event(fifo_fd, LOG_INVALID_SENSOR_ID, 0, 0.0, msg);

            /* Track invalid data in the session */
            pthread_mutex_lock(&session->mutex);
            session->errors++;
            pthread_mutex_unlock(&session->mutex);
            return;
        }

        /* Validate sensor node ID */
        if (node_id <= 0 || node_id > MAX_SENSOR_NODES) {
            char msg[100];
            sprintf(msg, "Received sensor data with invalid sensor node ID %d from %s",
                    node_id, session->ip_str);
            write_log_event(fifo_fd, LOG_INVALID_SENSOR_ID, node_id, 0.0, msg);

            /* Track invalid data in the session */
            pthread_mutex_lock(&session->mutex);
            session->errors++;

            /* If multiple invalid data packets, consider blacklisting */
            if (session->errors > 5) {
                char ip_copy[INET_ADDRSTRLEN];
                strncpy(ip_copy, session->ip_str, INET_ADDRSTRLEN);
                ip_copy[INET_ADDRSTRLEN - 1] = '\0';
                pthread_mutex_unlock(&session->mutex);

                sprintf(msg, "IP %s blacklisted after sending multiple invalid data packets", ip_copy);
                write_log_event(fifo_fd, LOG_SECURITY_VIOLATION, 0, 0.0, msg);
                add_ip_to_blacklist_local(ip_copy);
            } else {
                pthread_mutex_unlock(&session->mutex);
            }

            return;
        }

        /* Add data to shared data structure */
        pthread_mutex_lock(&shared_data.mutex);

        /* Find existing entry or use a new one */
        int idx = -1;
        for (int i = 0; i < shared_data.count; i++) {
            if (shared_data.data[i].node_id == node_id) {
                idx = i;
                break;
            }
        }

        if (idx == -1) {
            /* New sensor node */
            if (shared_data.count < MAX_SENSOR_NODES) {
                idx = shared_data.count++;
            } else {
                /* No space for new sensor node */
                pthread_mutex_unlock(&shared_data.mutex);
                return;
            }
        }

        /* Update sensor data */
        shared_data.data[idx].node_id = node_id;
        shared_data.data[idx].temperature = (int)(temperature * 100);  /* Store as fixed-point */
        shared_data.data[idx].humidity = (int)(humidity * 100);       /* Store as fixed-point */
        shared_data.data[idx].light = (int)(pressure * 100);          /* Use light field for pressure */
        shared_data.data[idx].timestamp = time(NULL);

        pthread_mutex_unlock(&shared_data.mutex);

        /* Debug output */
        printf("Received data from sensor node %d: temp=%.2f, humidity=%.2f, pressure=%.2f\n",
               node_id, temperature, humidity, pressure);
    } else {
        /* Unknown command */
        char msg[100];
        sprintf(msg, "Unknown command received from %s", session->ip_str);
        write_log_event(fifo_fd, LOG_SECURITY_VIOLATION, 0, 0.0, msg);

        /* Track invalid data in the session */
        pthread_mutex_lock(&session->mutex);
        session->errors++;
        pthread_mutex_unlock(&session->mutex);
    }
}

/* Close a connection */
static void close_connection(connection_session_t *session) {
    if (!session) return;

    /* Remove from epoll */
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, session->socket_fd, NULL);

    /* Close socket */
    close(session->socket_fd);

    /* Log connection closed event */
    char msg[100];
    sprintf(msg, "The sensor node from %s:%d has closed the connection",
            session->ip_str, session->port);
    write_log_event(fifo_fd, LOG_CONNECTION_CLOSED, 0, 0.0, msg);

    printf("Connection closed: %s:%d (ID: %u)\n",
           session->ip_str, session->port, session->connection_id);

    /* Update system status */
    pthread_mutex_lock(&system_status.mutex);
    system_status.active_connections--;
    pthread_mutex_unlock(&system_status.mutex);

    /* Remove from connection list and free */
    remove_connection(&connection_list, session->connection_id);
    free_connection_session(session);
}

/* Make a socket non-blocking */
static int make_socket_non_blocking(int socket_fd) {
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1) {
        return -1;
    }

    flags |= O_NONBLOCK;
    if (fcntl(socket_fd, F_SETFL, flags) == -1) {
        return -1;
    }

    return 0;
}

/* Check for connection timeouts */
static void check_timeouts() {
    time_t now = time(NULL);

    pthread_mutex_lock(&connection_list.mutex);

    for (int i = 0; i < connection_list.count; i++) {
        connection_session_t *session = connection_list.connections[i];
        if (!session) continue;

        pthread_mutex_lock(&session->mutex);
        time_t last_activity = session->last_activity;
        pthread_mutex_unlock(&session->mutex);

        if (now - last_activity > CONNECTION_TIMEOUT) {
            /* Connection timed out */
            char msg[100];
            sprintf(msg, "Connection from %s:%d timed out after %d seconds of inactivity",
                    session->ip_str, session->port, CONNECTION_TIMEOUT);
            write_log_event(fifo_fd, LOG_CONNECTION_TIMEOUT, 0, 0.0, msg);

            printf("Connection timeout: %s:%d (ID: %u)\n",
                   session->ip_str, session->port, session->connection_id);

            /* Close the connection */
            close_connection(session);

            /* Adjust index since we removed an element */
            i--;
        }
    }

    pthread_mutex_unlock(&connection_list.mutex);
}

/* Create a new connection session */
connection_session_t* create_connection_session(int socket_fd, struct sockaddr_in *addr) {
    connection_session_t *session = (connection_session_t*)malloc(sizeof(connection_session_t));
    if (!session) return NULL;

    memset(session, 0, sizeof(connection_session_t));

    session->socket_fd = socket_fd;
    session->connection_id = next_connection_id++;
    memcpy(&session->addr, addr, sizeof(struct sockaddr_in));

    /* Convert IP address to string */
    inet_ntop(AF_INET, &addr->sin_addr, session->ip_str, INET_ADDRSTRLEN);
    session->port = ntohs(addr->sin_port);

    /* Set timestamps */
    session->connect_time = time(NULL);
    session->last_activity = session->connect_time;

    /* Set initial state */
    session->state = CONN_STATE_NEW;
    session->auth_state = AUTH_STATE_NONE;

    /* Initialize mutex */
    if (pthread_mutex_init(&session->mutex, NULL) != 0) {
        free(session);
        return NULL;
    }

    return session;
}

/* Free a connection session */
void free_connection_session(connection_session_t *session) {
    if (!session) return;

    pthread_mutex_destroy(&session->mutex);
    free(session);
}

/* Add a connection to the list */
int add_connection(connection_list_t *list, connection_session_t *session) {
    if (!list || !session) return -1;

    pthread_mutex_lock(&list->mutex);

    /* Check if list is full */
    if (list->count >= MAX_CONNECTIONS) {
        pthread_mutex_unlock(&list->mutex);
        return -1;
    }

    /* Find an empty slot */
    int idx = -1;
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (list->connections[i] == NULL) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        pthread_mutex_unlock(&list->mutex);
        return -1;
    }

    /* Add to list */
    list->connections[idx] = session;
    list->count++;

    pthread_mutex_unlock(&list->mutex);
    return 0;
}

/* Remove a connection from the list */
int remove_connection(connection_list_t *list, uint32_t connection_id) {
    if (!list) return -1;

    pthread_mutex_lock(&list->mutex);

    /* Find the connection */
    int idx = -1;
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (list->connections[i] && list->connections[i]->connection_id == connection_id) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        pthread_mutex_unlock(&list->mutex);
        return -1;
    }

    /* Remove from list */
    list->connections[idx] = NULL;
    list->count--;

    pthread_mutex_unlock(&list->mutex);
    return 0;
}

/* Find a connection by ID */
connection_session_t* find_connection_by_id(connection_list_t *list, uint32_t connection_id) {
    if (!list) return NULL;

    pthread_mutex_lock(&list->mutex);

    connection_session_t *session = NULL;
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (list->connections[i] && list->connections[i]->connection_id == connection_id) {
            session = list->connections[i];
            break;
        }
    }

    pthread_mutex_unlock(&list->mutex);
    return session;
}

/* Find a connection by socket */
connection_session_t* find_connection_by_socket(connection_list_t *list, int socket_fd) {
    if (!list) return NULL;

    pthread_mutex_lock(&list->mutex);

    connection_session_t *session = NULL;
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (list->connections[i] && list->connections[i]->socket_fd == socket_fd) {
            session = list->connections[i];
            break;
        }
    }

    pthread_mutex_unlock(&list->mutex);
    return session;
}

/* Update connection activity timestamp */
void update_connection_activity(connection_session_t *session) {
    if (!session) return;

    pthread_mutex_lock(&session->mutex);
    session->last_activity = time(NULL);
    session->state = CONN_STATE_ACTIVE;
    pthread_mutex_unlock(&session->mutex);
}

/* Set connection authentication state */
void set_connection_authenticated(connection_session_t *session, bool authenticated) {
    if (!session) return;

    pthread_mutex_lock(&session->mutex);

    if (authenticated) {
        session->auth_state = AUTH_STATE_SUCCESS;
        write_log_event(fifo_fd, LOG_AUTH_SUCCESS, 0, 0.0,
                       "Connection authenticated successfully");
    } else {
        session->auth_state = AUTH_STATE_FAILED;
        write_log_event(fifo_fd, LOG_AUTH_FAILED, 0, 0.0,
                       "Connection authentication failed");
    }

    pthread_mutex_unlock(&session->mutex);
}

/* Check if connection limit for an IP is exceeded */
bool check_connection_limit(const char *ip_address, connection_list_t *list) {
    if (!ip_address || !list) return false;

    pthread_mutex_lock(&list->mutex);

    int count = 0;
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (list->connections[i] && strcmp(list->connections[i]->ip_str, ip_address) == 0) {
            count++;
        }
    }

    pthread_mutex_unlock(&list->mutex);

    return count < MAX_CONNECTIONS_PER_IP;
}

/* Check if an IP is blacklisted (local implementation) */
static bool is_ip_blacklisted_local(const char *ip_address) {
    /* Use the global blacklist function from security_manager.c */
    return is_ip_blacklisted(ip_address);
}

/* Get connection list - function to allow access from main.c */
connection_list_t* get_connection_list() {
    return &connection_list;
}

/* Add an IP to the blacklist (local implementation) */
static void add_ip_to_blacklist_local(const char *ip_address) {
    /* Use the global blacklist function from security_manager.c */
    add_ip_to_blacklist(ip_address);
}
