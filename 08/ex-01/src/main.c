#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>
#include <string.h>

#define QUEUE_NAME "/my_message_queue"
#define MAX_SIZE 1024
#define MSG_STOP "exit"

int main() {
    pid_t pid;
    mqd_t mq;
    struct mq_attr attr;
    char buffer[MAX_SIZE];

    // Set message queue attributes
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_curmsgs = 0;

    // Create the message queue
    mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);
    if (mq == -1) {
        perror("mq_open failed");
        exit(1);
    }

    // Fork to create child process
    pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) {  
        // Child process: Receive message
        sleep(1);  // Ensure parent sends message first

        if (mq_receive(mq, buffer, MAX_SIZE, NULL) == -1) {
            perror("mq_receive failed");
            exit(1);
        }

        printf("Child received: \"%s\"\n", buffer);

        // Close and remove message queue
        mq_close(mq);
        mq_unlink(QUEUE_NAME);
        exit(0);
    } else {  
        // Parent process: Send message
        char message[] = "Hello from Parent!";
        if (mq_send(mq, message, strlen(message) + 1, 0) == -1) {
            perror("mq_send failed");
            exit(1);
        }

        printf("Parent sent: \"%s\"\n", message);

        // Close message queue (child will unlink it)
        mq_close(mq);
    }

    return 0;
}
