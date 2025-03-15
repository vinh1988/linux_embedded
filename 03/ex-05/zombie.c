#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } 
    else if (pid == 0) {
        // Child process exits immediately
        printf("Child process (PID: %d) exiting to become a zombie\n", getpid());
        exit(0);
    } 
    else {
        // Parent process sleeps, keeping the child in zombie state
        printf("Parent process (PID: %d) created a child (PID: %d) and is now sleeping.\n", getpid(), pid);
        printf("Run this command in another terminal to see the zombie process:\n");
        printf("  ps aux | grep 'Z'\n");
        
        sleep(10);  // Give time to observe zombie state
        printf("Parent is now cleaning up the zombie process.\n");
        wait(NULL);  // Clean up zombie
        printf("Zombie process is now reaped. Check again with `ps aux`.\n");
    }

    return 0;
}
