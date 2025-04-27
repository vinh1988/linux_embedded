#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

/*Signal handler function*/
void handle_signal(int signum){
    printf("Child process %d receive signal %d\n", getpid(), signum);
}

int main(){
    pid_t child_pid = fork(); // Create children process
    if (child_pid==1){
        perror("fork failed");
        exit(1);
    }
    if (child_pid==0){
        signal(SIGUSR1, handle_signal);
        printf("child process %d waiting for signal...\n", getpid());
        kill(child_pid, SIGUSR1);
        sleep(1);
        printf("parent process exiting.\n");
    }
    return 0;
}
