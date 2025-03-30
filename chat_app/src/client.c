#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "client.h"

#define BUFFER_SIZE 1024
#define MAX_CONNECTIONS 10  // Maximum tracked connections

// Struct to store active connections
typedef struct {
    char ip[INET_ADDRSTRLEN];
    int port;
    int sock;  // Socket descriptor
} Connection;

Connection active_connections[MAX_CONNECTIONS];
int conn_count = 0;

// Function to get the local IP address
void get_local_ip(char *buffer, size_t size) {
    struct sockaddr_in serv;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (sock == -1) {
        perror("Socket error");
        return;
    }

    serv.sin_family = AF_INET;
    serv.sin_port = htons(80);
    serv.sin_addr.s_addr = inet_addr("8.8.8.8");

    connect(sock, (struct sockaddr *)&serv, sizeof(serv));

    struct sockaddr_in local_address;
    socklen_t addr_len = sizeof(local_address);
    getsockname(sock, (struct sockaddr *)&local_address, &addr_len);

    close(sock);

    inet_ntop(AF_INET, &local_address.sin_addr, buffer, size);
}

// Function to check if a connection already exists
int is_duplicate_connection(const char *ip, int port) {
    for (int i = 0; i < conn_count; i++) {
        if (strcmp(active_connections[i].ip, ip) == 0 && active_connections[i].port == port) {
            return 1;  // Duplicate found
        }
    }
    return 0;
}

// Function to add a new connection
void add_connection(const char *ip, int port, int sock) {
    if (conn_count < MAX_CONNECTIONS) {
        strcpy(active_connections[conn_count].ip, ip);
        active_connections[conn_count].port = port;
        active_connections[conn_count].sock = sock;
        conn_count++;
    } else {
        printf("Warning: Max connections reached, unable to track more.\n");
    }
}

// Function to list all active connections
void list_connections() {
    if (conn_count == 0) {
        printf("No active connections.\n");
    } else {
        printf("Active connections:\n");
        for (int i = 0; i < conn_count; i++) {
            printf("%d. %s:%d\n", i + 1, active_connections[i].ip, active_connections[i].port);
        }
    }
}

// Function to terminate a connection by ID
void terminate_connection(int id) {
    if (id < 1 || id > conn_count) {
        printf("Error: Invalid connection ID.\n");
        return;
    }

    int index = id - 1;
    close(active_connections[index].sock);  // Close socket
    printf("Terminated connection to %s:%d\n", active_connections[index].ip, active_connections[index].port);

    // Shift the remaining connections
    for (int i = index; i < conn_count - 1; i++) {
        active_connections[i] = active_connections[i + 1];
    }
    conn_count--;
}

// Function to connect to a server
void connect_to_server(const char *ip, int port) {
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Get local IP
    char local_ip[INET_ADDRSTRLEN];
    get_local_ip(local_ip, sizeof(local_ip));

    // Check if connecting to itself
    if (strcmp(ip, local_ip) == 0) {
        printf("Error: Attempted self-connection. Exiting.\n");
        return;
    }

    // Check for duplicate connection
    if (is_duplicate_connection(ip, port)) {
        printf("Error: Duplicate connection detected to %s:%d. Exiting.\n", ip, port);
        return;
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error\n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        printf("Invalid address / Address not supported\n");
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection failed\n");
        return;
    }

    printf("Connected to server at %s:%d\n", ip, port);
    add_connection(ip, port, sock); // Store connection

    // Chat loop
    while (1) {
        printf("You: ");
        fgets(buffer, BUFFER_SIZE, stdin);

        // If the user types "list", show all active connections
        if (strncmp(buffer, "list", 4) == 0) {
            list_connections();
            continue;
        }

        // If the user types "terminate <id>", close the connection
        if (strncmp(buffer, "terminate", 9) == 0) {
            int id;
            if (sscanf(buffer, "terminate %d", &id) == 1) {
                terminate_connection(id);
            } else {
                printf("Usage: terminate <connection id>\n");
            }
            continue;
        }

        send(sock, buffer, strlen(buffer), 0);

        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Exiting chat...\n");
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(sock, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            printf("Server disconnected.\n");
            break;
        }

        printf("Server: %s", buffer);
    }
    close(sock);
}
