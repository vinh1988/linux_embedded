#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "server.h"

#define BUFFER_SIZE 1024

void start_server(int port) {
    int server_fd, new_socket;
    struct sockaddr_in address, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int opt = 1;
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", port);

    // Accept client connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    // Get client IP and port
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(client_addr.sin_port);

    printf("Client connected from %s:%d\n", client_ip, client_port);

    // Chat loop
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            printf("Client %s:%d disconnected.\n", client_ip, client_port);
            break;
        }

        // Display message with sender information
        printf("Received from %s:%d -> %s\n", client_ip, client_port, buffer);

        // Exit chat if client sends "exit"
        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Client %s:%d exited the chat.\n", client_ip, client_port);
            break;
        }

        // Send response
        char response[BUFFER_SIZE];
        printf("Server: ");
        fgets(response, BUFFER_SIZE, stdin);
        send(new_socket, response, strlen(response), 0);
    }

    // Close sockets
    close(new_socket);
    close(server_fd);
}
