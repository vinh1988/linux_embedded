#include "common.h"
#include <ifaddrs.h>
#include <netdb.h>

void error_exit(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void show_my_ip() {
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
            inet_ntop(AF_INET, &sa->sin_addr, host, NI_MAXHOST);
            printf("[INFO] My IP: %s\n", host);
            break;
        }
    }

    freeifaddrs(ifaddr);
}

void show_myport(int port) {
    printf("Listening on port: %d\n", port);
}

void show_help() {
    printf("\nAvailable Commands:\n");
    printf("help       - Show this help message\n");
    printf("myip       - Show the IP address of this machine\n");
    printf("connect <destination> <port> - Connect to a peer\n");
    printf("exit       - Disconnect all peers and close the program\n\n");
}
