#include "server.h"

void start_server() {
    int server_fd, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    (void)buffer;  // Suppress warning if unused

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) error_exit("Socket creation failed");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        error_exit("Bind failed");

    if (listen(server_fd, 5) < 0) error_exit("Listen failed");

    printf("[INFO] Server listening on port %d...\n", PORT);

    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);
        if (client_socket < 0) {
            perror("Client accept failed");
            continue;
        }
        printf("[INFO] Connection established with %s:%d\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        handle_client(client_socket);
    }

    close(server_fd);
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            printf("[INFO] Client disconnected.\n");
            break;
        }
        printf("[Client]: %s\n", buffer);
        send(client_socket, buffer, bytes_received, 0);
    }
    close(client_socket);
}

int main() {
    start_server();
    return 0;
}
