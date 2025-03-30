#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#define BUFFER_SIZE 1024
#define MAX_CONNECTIONS 10  // Limit number of pending connections

void handle_client(int client_socket, struct sockaddr_in client_addr) {
    char buffer[BUFFER_SIZE];
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(client_addr.sin_port);

    printf("Client connected from %s:%d\n", client_ip, client_port);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(client_socket, buffer, BUFFER_SIZE);
        
        if (valread <= 0) {
            printf("Client %s:%d disconnected.\n", client_ip, client_port);
            break;
        }

        printf("Received from %s:%d -> %s\n", client_ip, client_port, buffer);

        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Client %s:%d exited the chat.\n", client_ip, client_port);
            break;
        }

        printf("Server: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        send(client_socket, buffer, strlen(buffer), 0);
    }

    close(client_socket);
    printf("Connection closed for %s:%d\n", client_ip, client_port);
    exit(0);
}

void start_server(int port) {
    int server_fd;
    struct sockaddr_in address, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int opt = 1;

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Configure server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, MAX_CONNECTIONS) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", port);

    while (1) {
        int new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (new_socket < 0) {
            perror("Accept failed");
            continue;
        }

        // Fork a new process to handle the client
        pid_t pid = fork();
        if (pid == 0) {  
            // Child process handles the client
            close(server_fd); // Child doesn't need the listening socket
            handle_client(new_socket, client_addr);
        } else if (pid > 0) {  
            // Parent process closes client socket and continues accepting
            close(new_socket);
        } else {
            perror("Fork failed");
            close(new_socket);
        }
    }

    close(server_fd);
}