#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 100

int main() {
    int pipefd[2];  // Pipe file descriptors
    pid_t pid;
    char message[] = "Hello, Pipe Communication!";  // Sample message

    // Create pipe
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        exit(1);
    }

    // Fork child process
    pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) {  
        // Child process
        close(pipefd[1]);  // Close write end

        char buffer[BUFFER_SIZE];
        read(pipefd[0], buffer, BUFFER_SIZE);  // Read from pipe
        close(pipefd[0]);  // Close read end

        // Count characters
        int char_count = strlen(buffer);
        printf("Child received: \"%s\"\n", buffer);
        printf("Character count: %d\n", char_count);
        
        exit(0);
    } else {  
        // Parent process
        close(pipefd[0]);  // Close read end
        write(pipefd[1], message, strlen(message) + 1);  // Write message to pipe
        close(pipefd[1]);  // Close write end
    }

    return 0;
}
