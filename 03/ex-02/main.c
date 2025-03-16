#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>


int main(int argc, char *argv[])
{
    pid_t pid = fork(); 
    if (pid < 0){
        perror("FFork failed");
        exit(1);
    }
    if (pid==0){
        char *env_var = getenv("EXEC_CMD");
        if (env_var==NULL){
            fprintf(stderr, "EXEC_CMD environment variable not ste.\n");
            exit(1);
        }
        printf("child process executing command based on EXEC_CMD=%s\n", env_var);
        if (strcmp(env_var, "1")==0){
            execlp("ls", "ls", NULL);
        }else if (strcmp(env_var, "12")==0){
            execlp("date", "date", NULL);
        }
        else{
            fprintf(stderr, "Invalid EXEC_CMD value. Use 1 for 'ls' or 2 for 'date'.\n");
            exit(1);
        }
        perror("exec failed");
        exit(1);
    }else{
        wait(NULL);
        printf("Child process finished.\n");
    }
    return 0;
}