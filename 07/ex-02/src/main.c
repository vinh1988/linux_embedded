#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 100

int main() {
    int pipe1[2], pipe2[2];  // Two pipes
    pid_t pid1, pid2;

    // Create the first pipe
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe failed");
        exit(1);
    }

    // First fork - Create child 1
    pid1 = fork();

    if (pid1 < 0) {
        perror("fork failed");
        exit(1);
    }

    if (pid1 == 0) {  
        // Child Process 1
        close(pipe1[1]);  // Close write end of pipe1
        close(pipe2[0]);  // Close read end of pipe2
        
        char buffer[BUFFER_SIZE];
        read(pipe1[0], buffer, BUFFER_SIZE);  // Read from parent
        close(pipe1[0]);  // Close read end after reading

        // Modify the message
        strcat(buffer, " -> Modified by Child 1");

        write(pipe2[1], buffer, strlen(buffer) + 1);  // Send to Child 2
        close(pipe2[1]);  // Close write end after sending
        exit(0);
    }

    // Second fork - Create child 2
    pid2 = fork();

    if (pid2 < 0) {
        perror("fork failed");
        exit(1);
    }

    if (pid2 == 0) {  
        // Child Process 2
        close(pipe2[1]);  // Close write end of pipe2

        char buffer[BUFFER_SIZE];
        read(pipe2[0], buffer, BUFFER_SIZE);  // Read from Child 1
        close(pipe2[0]);  // Close read end after reading

        printf("Child 2 received: %s\n", buffer);
        exit(0);
    }

    // Parent Process
    close(pipe1[0]);  // Close read end of pipe1
    close(pipe2[0]);  // Close read end of pipe2
    close(pipe2[1]);  // Close write end of pipe2

    char message[] = "Hello from Parent!";
    write(pipe1[1], message, strlen(message) + 1);
    close(pipe1[1]);  // Close write end after sending

    return 0;
}
