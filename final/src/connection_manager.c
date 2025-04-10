#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include "../include/shared.h"
#include "../include/sensor_data.h"

extern shared_data_t shared_data;
extern int fifo_fd;
extern int port;
extern volatile int running;

/* Function to handle a sensor node connection */
void handle_sensor_connection(int client_socket, struct sockaddr_in client_addr) {
    char client_ip[INET_ADDRSTRLEN];
    sensor_packet_t packet;
    int bytes_read;
    
    /* Get client IP address */
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    
    /* Log connection opened event */
    write_log_event(fifo_fd, LOG_CONNECTION_OPENED, 0, 0.0, 
                   "A sensor node has opened a new connection");
    
    /* Receive data from sensor node */
    while (running) {
        /* Read sensor packet */
        bytes_read = recv(client_socket, &packet, sizeof(packet), 0);
        if (bytes_read <= 0) {
            /* Connection closed or error */
            break;
        }
        
        /* Validate sensor node ID */
        if (packet.node_id == 0 || packet.node_id > MAX_SENSOR_NODES) {
            char msg[100];
            sprintf(msg, "Received sensor data with invalid sensor node ID %d", packet.node_id);
            write_log_event(fifo_fd, LOG_INVALID_SENSOR_ID, packet.node_id, 0.0, msg);
            continue;
        }
        
        /* Add data to shared data structure */
        pthread_mutex_lock(&shared_data.mutex);
        
        /* Find existing entry or use a new one */
        int idx = -1;
        for (int i = 0; i < shared_data.count; i++) {
            if (shared_data.data[i].node_id == packet.node_id) {
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
                continue;
            }
        }
        
        /* Update sensor data */
        shared_data.data[idx].node_id = packet.node_id;
        shared_data.data[idx].temperature = packet.temperature;
        shared_data.data[idx].humidity = packet.humidity;
        shared_data.data[idx].light = packet.light;
        shared_data.data[idx].timestamp = time(NULL);
        
        pthread_mutex_unlock(&shared_data.mutex);
        
        /* Debug output */
        printf("Received data from sensor node %d: temp=%d, humidity=%d, light=%d\n",
               packet.node_id, packet.temperature, packet.humidity, packet.light);
    }
    
    /* Log connection closed event */
    char msg[100];
    sprintf(msg, "The sensor node has closed the connection");
    write_log_event(fifo_fd, LOG_CONNECTION_CLOSED, 0, 0.0, msg);
    
    /* Close client socket */
    close(client_socket);
}

/* Connection manager thread function */
void *connection_manager_thread(void *arg) {
    int server_fd;
    struct sockaddr_in address, client_addr;
    int opt = 1;
    socklen_t addrlen = sizeof(client_addr);
    
    /* Create socket */
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
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        running = 0;
        return NULL;
    }
    
    printf("Connection manager listening on port %d\n", port);
    
    /* Accept connections */
    while (running) {
        int client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }
        
        /* Handle sensor connection */
        handle_sensor_connection(client_socket, client_addr);
    }
    
    /* Clean up */
    close(server_fd);
    printf("Connection manager thread exiting\n");
    return NULL;
}
