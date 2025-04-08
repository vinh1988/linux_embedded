#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

int main() {
    const char *name = "/posix_shm";
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, 1024); // Set size

    char *ptr = mmap(0, 1024, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    sprintf(ptr, "Hello from writer (POSIX)");

    printf("Writer wrote to shared memory.\n");

    munmap(ptr, 1024);
    close(shm_fd);
    return 0;
}
