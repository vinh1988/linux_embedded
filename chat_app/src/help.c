#include <stdio.h>
#include "help.h"
#include <netinet/in.h>

void print_help() {
    printf("\nAvailable commands:\n");
    printf("help - Display this help message.\n");
    printf("myip - Show the IP address of this machine.\n");
    printf("myport - Show the port the program is listening on.\n");
    printf("\n");
}

void show_myport(int port) {
    printf("Listening on port: %d\n", port);
}
