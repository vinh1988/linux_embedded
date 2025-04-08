// reader.c
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main() {
    key_t key = ftok("shmfile", 65); // Same key as writer
    int shmid = shmget(key, 1024, 0666); // Get shared memory segment

    char *str = (char*) shmat(shmid, NULL, 0); // Attach

    printf("Reader: Data read from shared memory: \"%s\"\n", str);

    shmdt(str); // Detach
    shmctl(shmid, IPC_RMID, NULL); // Remove shared memory

    return 0;
}
