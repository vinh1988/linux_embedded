#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

int main() {
    const char *name = "/posix_shm";
    int shm_fd = shm_open(name, O_RDONLY, 0666);

    char *ptr = mmap(0, 1024, PROT_READ, MAP_SHARED, shm_fd, 0);
    printf("Reader read: %s\n", ptr);

    munmap(ptr, 1024);
    close(shm_fd);
    shm_unlink(name); // Optional: remove the shared memory object

    return 0;
}
