#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

void show_myip() {
    char hostbuffer[256];
    struct hostent *host_entry;
    char *IPbuffer;

    // Get the hostname
    if (gethostname(hostbuffer, sizeof(hostbuffer)) == -1) {
        perror("gethostname failed");
        return;
    }

    // Get host information
    host_entry = gethostbyname(hostbuffer);
    if (host_entry == NULL) {
        perror("gethostbyname failed");
        return;
    }

    // Convert the first address in the list to a readable string
    IPbuffer = inet_ntoa(*((struct in_addr *)host_entry->h_addr_list[0]));
    if (IPbuffer) {
        printf("My IP Address: %s\n", IPbuffer);
    } else {
        printf("Failed to retrieve IP address\n");
    }
}
