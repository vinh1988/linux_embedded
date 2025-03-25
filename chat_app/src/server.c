#include "server.h"

/* Function prototypes */
void error_exit(const char *message);
void handle_client(int client_socket);
void start_server(void);

/* Main entry point */
int main(void) {
    start_server();
    return 0;
}

/* Initialize and run the server */
void start_server(void) {
    int server_fd;
    int client_socket;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t addr_size;
    
    /* Initialize socket */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        error_exit("Socket creation failed");
    }

    /* Configure server address */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    /* Bind socket */
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error_exit("Bind failed");
    }

    /* Listen for connections */
    if (listen(server_fd, 5) < 0) {
        error_exit("Listen failed");
    }

    printf("[INFO] Server listening on port %d...\n", PORT);

    /* Accept and handle client connections */
    addr_size = sizeof(client_addr);
    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);
        if (client_socket < 0) {
            perror("[ERROR] Client accept failed");
            continue;
        }

        printf("[INFO] Connection established with %s:%d\n",
               inet_ntoa(client_addr.sin_addr), 
               ntohs(client_addr.sin_port));

        handle_client(client_socket);
    }

    close(server_fd);
}

/* Handle communication with a connected client */
void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received;

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        
        if (bytes_received <= 0) {
            printf("[INFO] Client disconnected.\n");
            break;
        }

        printf("[Client]: %s\n", buffer);
        send(client_socket, buffer, bytes_received, 0);
    }

    close(client_socket);
}
