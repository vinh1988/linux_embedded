#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

int main() {
    // Create an anonymous shared memory region
    char *shared_mem = mmap(NULL, 1024, PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_mem == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        sleep(1); // Let parent write first
        printf("Child read: %s\n", shared_mem);
    } else {
        // Parent process
        strcpy(shared_mem, "Hello from parent (anonymous)");
        wait(NULL);
    }

    munmap(shared_mem, 1024);
    return 0;
}
