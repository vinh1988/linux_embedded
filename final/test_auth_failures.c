#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_PORT 1237
#define SERVER_IP "127.0.0.1"

// Simulates a client that sends invalid authentication data
int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[1024];
    
    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }
    
    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    
    // Convert IP address from text to binary
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return 1;
    }
    
    // Connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }
    
    printf("Connected to server at %s:%d\n", SERVER_IP, SERVER_PORT);
    
    // Send invalid authentication data multiple times
    for (int i = 0; i < 5; i++) {
        // Create an empty username/password (will fail authentication)
        memset(buffer, 0, sizeof(buffer));
        
        // Send the data
        if (send(sock, buffer, 10, 0) < 0) {
            perror("Send failed");
            break;
        }
        
        printf("Sent invalid authentication data (attempt %d)\n", i+1);
        sleep(1);
    }
    
    // Close the socket
    close(sock);
    printf("Test completed\n");
    
    return 0;
}
