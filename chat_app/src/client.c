#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "client.h"

#define BUFFER_SIZE 1024

// Function to get the local IP address
void get_local_ip(char *buffer, size_t size) {
    struct sockaddr_in serv;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (sock == -1) {
        perror("Socket error");
        return;
    }

    serv.sin_family = AF_INET;
    serv.sin_port = htons(80);  // Use a common external port
    serv.sin_addr.s_addr = inet_addr("8.8.8.8");  // Google's DNS server

    connect(sock, (struct sockaddr *)&serv, sizeof(serv));

    struct sockaddr_in local_address;
    socklen_t addr_len = sizeof(local_address);
    getsockname(sock, (struct sockaddr *)&local_address, &addr_len);

    close(sock);

    inet_ntop(AF_INET, &local_address.sin_addr, buffer, size);
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

    // Chat loop
    while (1) {
        printf("You: ");
        fgets(buffer, BUFFER_SIZE, stdin);
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
