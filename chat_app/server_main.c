#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "help.h"
#include "common.h"
#include "server.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <command>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "help") == 0) {
        print_help();
    } else if (strcmp(argv[1], "myip") == 0) {
        show_myip();
    } else if (strcmp(argv[1], "myport") == 0) {
        show_myport(8080);
    } else if (strcmp(argv[1], "startserver") == 0) {
        start_server(8080);
    } else {
        printf("Invalid command for server. Try:\n");
        printf("  %s startserver\n", argv[0]);
    }

    return 0;
}
