#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <math.h>

#define DEFAULT_SERVER_PORT 1245
#define DEFAULT_SERVER_IP "127.0.0.1"
#define DEFAULT_SENSOR_ID 1
#define BUFFER_SIZE 1024
#define DEFAULT_INTERVAL 30  // Default interval in seconds

// Global variables for signal handling
volatile int running = 1;
int sock = -1;

// Sensor parameters
typedef struct {
    int sensor_id;
    char server_ip[16];
    int server_port;
    int interval;
    float base_temp;
    float base_humidity;
    float base_pressure;
    float temp_variation;
    float humidity_variation;
    float pressure_variation;
    char username[32];
    char password[32];
} sensor_config_t;

// Function to handle signals
void signal_handler(int signo) {
    if (signo == SIGINT || signo == SIGTERM) {
        printf("\nReceived signal %d, shutting down...\n", signo);
        running = 0;
        if (sock >= 0) {
            close(sock);
            sock = -1;
        }
    }
}

// Function to send authentication data
int send_auth(int sock, const char *username, const char *password) {
    char auth_buffer[BUFFER_SIZE];
    snprintf(auth_buffer, BUFFER_SIZE, "AUTH %s %s", username, password);
    
    printf("Sending authentication: %s\n", auth_buffer);
    return send(sock, auth_buffer, strlen(auth_buffer), 0);
}

// Function to send sensor data
int send_sensor_data(int sock, sensor_config_t *config) {
    // Generate realistic sensor data with some variation
    float variation = ((float)rand() / RAND_MAX) * 2.0 - 1.0;  // -1.0 to 1.0
    float temperature = config->base_temp + variation * config->temp_variation;
    
    variation = ((float)rand() / RAND_MAX) * 2.0 - 1.0;
    float humidity = config->base_humidity + variation * config->humidity_variation;
    
    variation = ((float)rand() / RAND_MAX) * 2.0 - 1.0;
    float pressure = config->base_pressure + variation * config->pressure_variation;
    
    // Format the data as a string
    char data_buffer[BUFFER_SIZE];
    snprintf(data_buffer, BUFFER_SIZE, "DATA %d %.2f %.2f %.2f %u", 
             config->sensor_id, temperature, humidity, pressure, (uint32_t)time(NULL));
    
    printf("Sending data: %s\n", data_buffer);
    return send(sock, data_buffer, strlen(data_buffer), 0);
}

// Function to connect to the server
int connect_to_server(sensor_config_t *config) {
    struct sockaddr_in server_addr;
    
    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }
    
    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(config->server_port);
    
    // Convert IP address from text to binary
    if (inet_pton(AF_INET, config->server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sock);
        return -1;
    }
    
    // Connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return -1;
    }
    
    printf("Connected to server at %s:%d as sensor node %d\n", 
           config->server_ip, config->server_port, config->sensor_id);
    
    // Send authentication data
    if (send_auth(sock, config->username, config->password) < 0) {
        perror("Authentication failed");
        close(sock);
        return -1;
    }
    
    return sock;
}

// Function to print usage information
void print_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS]\n\n", program_name);
    printf("Options:\n");
    printf("  -i, --id ID            Sensor ID (default: %d)\n", DEFAULT_SENSOR_ID);
    printf("  -s, --server IP        Server IP address (default: %s)\n", DEFAULT_SERVER_IP);
    printf("  -p, --port PORT        Server port (default: %d)\n", DEFAULT_SERVER_PORT);
    printf("  -t, --interval SEC     Data transmission interval in seconds (default: %d)\n", DEFAULT_INTERVAL);
    printf("  -u, --username USER    Username for authentication (default: sensor_user)\n");
    printf("  -w, --password PASS    Password for authentication (default: password123)\n");
    printf("  -h, --help             Display this help message\n");
    printf("\n");
    printf("Example:\n");
    printf("  %s --id 2 --server 192.168.1.100 --port 1245 --interval 15\n", program_name);
}

// Function to parse command line arguments
void parse_args(int argc, char *argv[], sensor_config_t *config) {
    // Set default values
    config->sensor_id = DEFAULT_SENSOR_ID;
    strncpy(config->server_ip, DEFAULT_SERVER_IP, sizeof(config->server_ip));
    config->server_port = DEFAULT_SERVER_PORT;
    config->interval = DEFAULT_INTERVAL;
    config->base_temp = 22.0;        // 22°C base temperature
    config->base_humidity = 50.0;    // 50% base humidity
    config->base_pressure = 1013.0;  // 1013 hPa base pressure
    config->temp_variation = 5.0;    // ±5°C variation
    config->humidity_variation = 10.0; // ±10% variation
    config->pressure_variation = 10.0; // ±10 hPa variation
    strncpy(config->username, "sensor_user", sizeof(config->username));
    strncpy(config->password, "password123", sizeof(config->password));
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--id") == 0) {
            if (i + 1 < argc) {
                config->sensor_id = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--server") == 0) {
            if (i + 1 < argc) {
                strncpy(config->server_ip, argv[++i], sizeof(config->server_ip) - 1);
            }
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0) {
            if (i + 1 < argc) {
                config->server_port = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--interval") == 0) {
            if (i + 1 < argc) {
                config->interval = atoi(argv[++i]);
                if (config->interval < 1) config->interval = 1;
            }
        } else if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--username") == 0) {
            if (i + 1 < argc) {
                strncpy(config->username, argv[++i], sizeof(config->username) - 1);
            }
        } else if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--password") == 0) {
            if (i + 1 < argc) {
                strncpy(config->password, argv[++i], sizeof(config->password) - 1);
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            exit(0);
        }
    }
}

int main(int argc, char *argv[]) {
    sensor_config_t config;
    time_t last_send_time = 0;
    int reconnect_delay = 5;  // Initial reconnect delay in seconds
    
    // Parse command line arguments
    parse_args(argc, argv, &config);
    
    // Set up signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Seed random number generator
    srand(time(NULL) + config.sensor_id);
    
    printf("Sensor Simulator starting (ID: %d, Server: %s:%d, Interval: %d sec)\n",
           config.sensor_id, config.server_ip, config.server_port, config.interval);
    
    // Main loop
    while (running) {
        // Connect to server if not connected
        if (sock < 0) {
            printf("Connecting to server...\n");
            sock = connect_to_server(&config);
            
            if (sock < 0) {
                printf("Connection failed, retrying in %d seconds...\n", reconnect_delay);
                sleep(reconnect_delay);
                
                // Exponential backoff for reconnect delay (max 60 seconds)
                reconnect_delay = (reconnect_delay * 2 > 60) ? 60 : reconnect_delay * 2;
                continue;
            }
            
            // Reset reconnect delay after successful connection
            reconnect_delay = 5;
        }
        
        // Check if it's time to send data
        time_t now = time(NULL);
        if (now - last_send_time >= config.interval) {
            // Send sensor data
            if (send_sensor_data(sock, &config) < 0) {
                perror("Failed to send data");
                close(sock);
                sock = -1;
                continue;
            }
            
            last_send_time = now;
            
            // Calculate time until next transmission
            time_t next_send = last_send_time + config.interval;
            struct tm *next_tm = localtime(&next_send);
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%H:%M:%S", next_tm);
            
            printf("Next transmission at %s (%d seconds from now)\n", 
                   time_str, config.interval);
        }
        
        // Sleep for a short time to avoid busy waiting
        // but be responsive to signals
        sleep(1);
    }
    
    // Clean up
    if (sock >= 0) {
        close(sock);
    }
    
    printf("Sensor simulator shutdown complete\n");
    return 0;
}
