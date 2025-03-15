#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void create_zombie() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } 
    else if (pid == 0) {
        // Child process
        printf("Child process (PID: %d) exiting to become a zombie\n", getpid());
        exit(0);
    } 
    else {
        // Parent process exits early, leaving child as a zombie
        printf("Parent process (PID: %d) exiting early\n", getpid());
        sleep(5);  // Give time to observe zombie state using `ps aux`
    }
}

void create_orphan() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } 
    else if (pid == 0) {
        // Child process
        sleep(2); // Ensure parent exits first
        printf("Orphan process (PID: %d, Parent PID: %d)\n", getpid(), getppid());
    } 
    else {
        // Parent process exits before child
        printf("Parent process (PID: %d) exiting early, child becomes orphan\n", getpid());
        exit(0);
    }
}

int main() {
    printf("Creating a zombie process...\n");
    create_zombie();

    sleep(2);  // Short delay before next process creation

    printf("Creating an orphan process...\n");
    create_orphan();

    return 0;
}
