#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    int socket;
    struct sockaddr_in addr;
} Peer;

Peer connections[MAX_CLIENTS];
int connection_count = 0;
int server_socket, listening_port;

// Function to display help
void print_help() {
    printf("Commands:\n");
    printf("help - Show available commands\n");
    printf("myip - Display the IP address\n");
    printf("myport - Display the listening port\n");
    printf("connect <IP> <port> - Connect to another peer\n");
    printf("list - Show all active connections\n");
    printf("terminate <id> - Close a connection\n");
    printf("send <id> <message> - Send a message to a peer\n");
    printf("exit - Close all connections and terminate\n");
}

// Function to get and display the machine's IP address
void show_my_ip() {
    char host[256];
    struct hostent *host_entry;
    gethostname(host, sizeof(host));
    host_entry = gethostbyname(host);
    printf("IP Address: %s\n", inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0])));
}

// Function to display the listening port
void show_my_port() {
    printf("Listening on port: %d\n", listening_port);
}

// Function to list active connections
void list_connections() {
    printf("ID  IP Address      Port\n");
    for (int i = 0; i < connection_count; i++) {
        printf("%d:  %s   %d\n", i + 1, inet_ntoa(connections[i].addr.sin_addr), ntohs(connections[i].addr.sin_port));
    }
}

// Function to terminate a connection
void terminate_connection(int id) {
    if (id < 1 || id > connection_count) {
        printf("Invalid connection ID.\n");
        return;
    }
    
    id--; // Convert to array index
    close(connections[id].socket);
    printf("Connection with %s:%d terminated.\n",
           inet_ntoa(connections[id].addr.sin_addr), ntohs(connections[id].addr.sin_port));

    // Shift connections
    for (int i = id; i < connection_count - 1; i++) {
        connections[i] = connections[i + 1];
    }
    connection_count--;
}

// Function to connect to a peer
void connect_to_peer(char *ip, int port) {
    if (connection_count >= MAX_CLIENTS) {
        printf("Max connections reached.\n");
        return;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket error");
        return;
    }

    struct sockaddr_in peer_addr;
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &peer_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return;
    }

    printf("Connected to %s:%d\n", ip, port);
    connections[connection_count].socket = sock;
    connections[connection_count].addr = peer_addr;
    connection_count++;
}

// Function to send message to a peer
void send_message(int id, char *message) {
    if (id < 1 || id > connection_count) {
        printf("Invalid connection ID.\n");
        return;
    }

    id--; // Convert to array index
    send(connections[id].socket, message, strlen(message), 0);
    printf("Message sent to %s:%d\n", inet_ntoa(connections[id].addr.sin_addr), ntohs(connections[id].addr.sin_port));
}

#include <pthread.h>  // Include this for threads

void *receive_messages(void *socket_desc) {
    int sock = *(int *)socket_desc;
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            printf("Connection closed by peer.\n");
            close(sock);
            pthread_exit(NULL);
        }
        printf("\nMessage received: \"%s\"\n> ", buffer);
        fflush(stdout);  // Ensure immediate printing
    }
}

void accept_connection() {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
    
    if (new_socket < 0) {
        perror("Accept failed");
        return;
    }

    if (connection_count >= MAX_CLIENTS) {
        printf("Max connections reached. Rejecting new client.\n");
        close(new_socket);
        return;
    }

    connections[connection_count].socket = new_socket;
    connections[connection_count].addr = client_addr;
    connection_count++;

    printf("New connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // Create a thread to listen for messages from this peer
    pthread_t recv_thread;
    int *new_sock = malloc(sizeof(int));
    *new_sock = new_socket;
    pthread_create(&recv_thread, NULL, receive_messages, (void *)new_sock);
}


void *server_loop(void *arg) {
    (void)arg;
    while (1) {
        accept_connection();  // Wait for incoming connections
    }
    return NULL;
}

void start_server(int port) {
    listening_port = port;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d...\n", port);

    // Start a new thread to continuously accept connections
    pthread_t server_thread;
    pthread_create(&server_thread, NULL, server_loop, NULL);
}

// Function to handle user commands
void handle_commands() {
    char command[BUFFER_SIZE];

    while (1) {
        printf("> ");
        fgets(command, BUFFER_SIZE, stdin);
        command[strcspn(command, "\n")] = 0;

        if (strcmp(command, "help") == 0) {
            print_help();
        } else if (strcmp(command, "myip") == 0) {
            show_my_ip();
        } else if (strcmp(command, "myport") == 0) {
            show_my_port();
        } else if (strncmp(command, "connect", 7) == 0) {
            char ip[16];
            int port;
            sscanf(command, "connect %s %d", ip, &port);
            connect_to_peer(ip, port);
        } else if (strcmp(command, "list") == 0) {
            list_connections();
        } else if (strncmp(command, "terminate", 9) == 0) {
            int id;
            sscanf(command, "terminate %d", &id);
            terminate_connection(id);
        } else if (strncmp(command, "send", 4) == 0) {
            int id;
            char message[BUFFER_SIZE];
            sscanf(command, "send %d %[^\n]", &id, message);
            send_message(id, message);
        } else if (strcmp(command, "exit") == 0) {
            for (int i = 0; i < connection_count; i++) {
                close(connections[i].socket);
            }
            close(server_socket);
            printf("Exiting...\n");
            exit(0);
        } else {
            printf("Invalid command. Type 'help' for options.\n");
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    start_server(port);
    handle_commands();

    return 0;
}
