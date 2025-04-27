#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 100

int main() {
    int fd[2];  // Pipe file descriptors
    pid_t pid;
    
    // Create pipe
    if (pipe(fd) == -1) {
        perror("pipe failed");
        exit(1);
    }

    // Fork process
    pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }

    if (pid > 0) {  
        // Parent Process
        close(fd[0]);  // Close unused read end
        char message[] = "Hello from parent!";
        write(fd[1], message, strlen(message) + 1);
        close(fd[1]);  // Close write end
    } else {  
        // Child Process
        close(fd[1]);  // Close unused write end
        char buffer[BUFFER_SIZE];
        read(fd[0], buffer, BUFFER_SIZE);
        printf("Child received: %s\n", buffer);
        close(fd[0]);  // Close read end
    }

    return 0;
}
