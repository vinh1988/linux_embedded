#include "client.h"

void start_client(const char *server_ip, int port) {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) error_exit("Socket creation failed");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
        error_exit("Invalid address");

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        error_exit("Connection failed");

    printf("[INFO] Connected to server %s:%d\n", server_ip, port);

    while (1) {
        printf("[You]: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        send(sock, buffer, strlen(buffer), 0);
        memset(buffer, 0, BUFFER_SIZE);
        recv(sock, buffer, BUFFER_SIZE, 0);
        printf("[Server]: %s", buffer);
    }
    close(sock);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }
    start_client(argv[1], atoi(argv[2]));
    return 0;
}
