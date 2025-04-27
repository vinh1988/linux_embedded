#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main() {
    pid_t child_pid;
    int status;

    child_pid = fork();

    if(child_pid<0){
        perror("Fork failed");
        exit(1);
    }

    else if (child_pid==0){
        printf("Child process (PID: %d) exiting with status: 42 \n", getpid());
        exit(2);
    }
    else {
        printf("Parent process (PID: %d) waiting for child (PID: %d)\n", getpid(), child_pid);
        wait(&status);
        if (WIFEXITED(status)) {
            printf("Child exited normally with status: %d\n", WEXITSTATUS(status));
        }else{
            printf("Child did not exit normally\n");
        }
    }
    return 0;
}

