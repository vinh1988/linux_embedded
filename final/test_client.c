#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define SERVER_PORT 1245
#define SERVER_IP "127.0.0.1"
#define SENSOR_ID 1
#define BUFFER_SIZE 1024

// Simple structure to simulate sensor data
typedef struct {
    uint16_t node_id;
    float temperature;
    float humidity;
    float pressure;
    uint32_t timestamp;
} sensor_packet_t;

// Function to send authentication data
int send_auth(int sock, const char *username, const char *password) {
    char auth_buffer[BUFFER_SIZE];
    snprintf(auth_buffer, BUFFER_SIZE, "AUTH %s %s", username, password);

    return send(sock, auth_buffer, strlen(auth_buffer), 0);
}

// Function to send sensor data
int send_sensor_data(int sock, uint16_t node_id, float temp, float humidity, float pressure) {
    // Format the data as a string to avoid binary format issues
    char data_buffer[BUFFER_SIZE];
    snprintf(data_buffer, BUFFER_SIZE, "DATA %d %.1f %.1f %.1f %u",
             node_id, temp, humidity, pressure, (uint32_t)time(NULL));

    return send(sock, data_buffer, strlen(data_buffer), 0);
}

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int node_id = SENSOR_ID;

    // Parse command line arguments
    if (argc > 1) {
        node_id = atoi(argv[1]);
    }

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

    printf("Connected to server at %s:%d as sensor node %d\n", SERVER_IP, SERVER_PORT, node_id);

    // Send authentication data
    printf("Sending authentication...\n");
    if (send_auth(sock, "sensor_user", "password123") < 0) {
        perror("Authentication failed");
        close(sock);
        return 1;
    }

    // Wait for a moment
    sleep(1);

    // Send sensor data in a loop
    int count = 0;
    while (count < 10) {
        // Generate random sensor data
        float temperature = 20.0 + ((float)rand() / RAND_MAX) * 10.0;  // 20-30°C
        float humidity = 40.0 + ((float)rand() / RAND_MAX) * 20.0;     // 40-60%
        float pressure = 1000.0 + ((float)rand() / RAND_MAX) * 20.0;   // 1000-1020 hPa

        printf("Sending data: Temp=%.1f°C, Humidity=%.1f%%, Pressure=%.1f hPa\n",
               temperature, humidity, pressure);

        if (send_sensor_data(sock, node_id, temperature, humidity, pressure) < 0) {
            perror("Failed to send data");
            break;
        }

        count++;
        sleep(2);  // Send data every 2 seconds
    }

    // Close the socket
    close(sock);
    printf("Test completed, connection closed\n");

    return 0;
}
