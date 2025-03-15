#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } 
    else if (pid == 0) {
        // Child sleeps to ensure the parent exits first
        sleep(5);
        printf("Orphan process (PID: %d) now has parent PID: %d\n", getpid(), getppid());
        printf("Run this command to see the orphan process:\n");
        printf("  ps -ef | grep %d\n", getpid());
        sleep(10);  // Keep running for observation
    } 
    else {
        // Parent exits early
        printf("Parent process (PID: %d) exiting early, child (PID: %d) will become an orphan.\n", getpid(), pid);
        exit(0);
    }

    return 0;
}
