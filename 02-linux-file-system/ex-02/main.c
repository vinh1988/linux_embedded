#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    const char *filename = "test.txt";
    int fd1, fd2, fd3;

    // Open file with O_TRUNC to clear content (use 0666 for Windows)
    fd1 = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd1 == -1) {
        perror("Error opening file with fd1");
        return 1;
    }

    // Open the same file again
    fd2 = open(filename, O_RDWR);
    if (fd2 == -1) {
        perror("Error opening file with fd2");
        return 1;
    }

    fd3 = open(filename, O_RDWR);
    if (fd3 == -1) {
        perror("Error opening file with fd3");
        return 1;
    }

    printf("[DEBUG] fd1 = %d, fd2 = %d, fd3 = %d\n", fd1, fd2, fd3);

    // Write "Hello," using fd1
    write(fd1, "Hello,", 6);
    printf("[DEBUG] Written 'Hello,' with fd1\n");

    // Write "world" using fd2
    write(fd2, "world", 6);
    printf("[DEBUG] Written 'world' with fd2\n");

    // Seek fd2 to the beginning
    lseek(fd2, 0, SEEK_SET);
    printf("[DEBUG] fd2 seeked to beginning\n");

    // Write "HELLO," using fd1
    write(fd1, "HELLO,", 6);
    printf("[DEBUG] Written 'HELLO,' with fd1\n");

    // Write "Gidday" using fd3
    write(fd3, "Gidday", 6);
    printf("[DEBUG] Written 'Gidday' with fd3\n");

    // Close file descriptors
    close(fd1);
    close(fd2);
    close(fd3);

    printf("[DEBUG] File operations completed. Check '%s'.\n", filename);

    return 0;
}
