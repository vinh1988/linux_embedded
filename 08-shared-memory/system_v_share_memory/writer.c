// writer.c
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

int main() {
    key_t key = ftok("shmfile", 65); // Create unique key
    int shmid = shmget(key, 1024, 0666 | IPC_CREAT); // Create shared memory segment

    char *str = (char*) shmat(shmid, NULL, 0); // Attach to shared memory

    strcpy(str, "Hi from writer process!"); // Write to shared memory

    printf("Writer: Data written to shared memory: \"%s\"\n", str);

    shmdt(str); // Detach from shared memory

    return 0;
}
