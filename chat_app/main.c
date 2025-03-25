#include <stdio.h>
#include <string.h>
#include "include/help.h"

#define BUFFER_SIZE 1024

int main() {
    char buffer[BUFFER_SIZE];

    while (1) {
        printf("\nEnter command (type 'help' for available commands): ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline

        if (strcmp(buffer, "help") == 0) {
            print_help();
        } else if (strcmp(buffer, "myip") == 0) {
            show_myip();
        } else if (strcmp(buffer, "myport") == 0) {
            show_myport();
        } else if (strcmp(buffer, "exit") == 0) {
            printf("Exiting the chat system...\n");
            break;
        } else {
            printf("Unknown command. Type 'help' for available commands.\n");
        }
    }

    return 0;
}
