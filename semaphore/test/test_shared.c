#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include "../include/shared.h"

// A mock test to check shared memory + semaphore
int main() {
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SHM_SIZE);
    char *shm_ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }

    sem_wait(sem);
    strcpy(shm_ptr, "Test Message");
    sem_post(sem);

    // Simulate a read test
    char buffer[SHM_SIZE];
    sem_wait(sem);
    strcpy(buffer, shm_ptr);
    sem_post(sem);

    if (strcmp(buffer, "Test Message") == 0) {
        printf("✅ Test Passed: Shared memory read/write OK\n");
    } else {
        printf("❌ Test Failed: Shared memory mismatch\n");
    }

    munmap(shm_ptr, SHM_SIZE);
    close(shm_fd);
    sem_close(sem);
    sem_unlink(SEM_NAME);
    shm_unlink(SHM_NAME);

    return 0;
}
