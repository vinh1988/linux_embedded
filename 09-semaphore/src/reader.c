#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>
#include "../include/shared.h"

int main() {
    int shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    char *shm_ptr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);

    sem_t *sem = sem_open(SEM_NAME, 0);
    sem_wait(sem);

    printf("Reader got from shared memory: \"%s\"\n", shm_ptr);

    sem_post(sem);

    munmap(shm_ptr, SHM_SIZE);
    close(shm_fd);
    sem_close(sem);
    sem_unlink(SEM_NAME);
    shm_unlink(SHM_NAME);

    return 0;
}
