#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char const *argv[]) /*offering stack frame for main()*/
{
    pid_t child_pid;
    int counter = 2;
    printf("gia tri khoi tao cua counter: %d\n", counter);

    child_pid = fork();
    printf("child_pid: %d", child_pid);
    if (child_pid >= 0){
        if (0==child_pid){
            printf("\n I am the child process, counter:%d\n", ++counter);
            printf("My PID is: %d, my parent PID is: %d", getpid(), getppid());
        } else {
            printf("\nI am the the parent process, counter: %d\n", ++counter);
            printf("My PID is: %d\n", getpid());
            while(1);
        } 
    } else {
        printf("fork() unsuccessfully\n)");
    }
    return 0;
}