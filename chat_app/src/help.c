#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include "help.h"

#define PORT 8080  // Change as needed

void print_help() {
    printf("\nAvailable Commands:\n");
    printf("----------------------------------------\n");
    printf(" help               - Display this help message.\n");
    printf(" myip               - Show the IP address of this machine.\n");
    printf(" myport             - Show the port the program is listening on.\n");
    printf(" connect <ip> <port> - Connect to a peer using the given IP and port.\n");
    printf(" list               - Show the list of active connections.\n");
    printf(" terminate <id>     - Close a connection by its ID.\n");
    printf(" send <id> <message> - Send a message to a peer by ID.\n");
    printf(" exit               - Close all connections and exit the program.\n");
    printf("----------------------------------------\n");
}

// Function to get the machine's IP address
void show_myip() {
    char hostbuffer[256];
    struct hostent *host_entry;
    char *IP;

    // Get the hostname
    if (gethostname(hostbuffer, sizeof(hostbuffer)) == -1) {
        perror("gethostname");
        return;
    }

    // Get host information
    host_entry = gethostbyname(hostbuffer);
    if (host_entry == NULL) {
        perror("gethostbyname");
        return;
    }

    // Convert to IP address
    IP = inet_ntoa(*((struct in_addr *)host_entry->h_addr_list[0]));
    printf("Your IP Address: %s\n", IP);
}

// Function to show the port (assuming fixed port for now)
void show_myport() {
    printf("Listening on port: %d\n", PORT);
}
