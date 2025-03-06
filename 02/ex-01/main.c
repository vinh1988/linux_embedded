#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main() {
    const char *filename = "test.txt";
    const char *data = "Hello. ";
    int fd;

    // Open file with O_APPEND
    fd = open(filename, O_WRONLY | O_APPEND);
    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    printf("[DEBUG] File descriptor: %d\n", fd);

    // Seek to the beginning of the file
    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("Error seeking to beginning");
    } else {
        printf("[DEBUG] Seeked to the beginning of the file\n");
    }

    // Attempt to write at the beginning
    if (write(fd, data, strlen(data)) == -1) {
        perror("Error writing to file");
    } else {
        printf("[DEBUG] Data written to file: '%s'\n", data);
    }

    close(fd);
    return 0;
}
