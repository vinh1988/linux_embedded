#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_READERS 5
#define NUM_WRITERS 2

int data = 0;
pthread_rwlock_t rwlock;

void* reader(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < 1; i++) {
        pthread_rwlock_rdlock(&rwlock);
        printf("Reader %d: Data = %d\n", id, data);
        pthread_rwlock_unlock(&rwlock);
        usleep(100000);
    }
    return NULL;
}

void* writer(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < 1; i++) {
        pthread_rwlock_wrlock(&rwlock);
        data++;
        printf("Writer %d: Incremented Data to %d\n", id, data);
        pthread_rwlock_unlock(&rwlock);
        usleep(200000);
    }
    return NULL;
}

int main() {
    pthread_t readers[NUM_READERS], writers[NUM_WRITERS];
    int reader_ids[NUM_READERS], writer_ids[NUM_WRITERS];
    
    pthread_rwlock_init(&rwlock, NULL);
    
    for (int i = 0; i < NUM_READERS; i++) {
        reader_ids[i] = i + 1;
        pthread_create(&readers[i], NULL, reader, &reader_ids[i]);
    }
    
    for (int i = 0; i < NUM_WRITERS; i++) {
        writer_ids[i] = i + 1;
        pthread_create(&writers[i], NULL, writer, &writer_ids[i]);
    }
    
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
    
    for (int i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writers[i], NULL);
    }
    
    pthread_rwlock_destroy(&rwlock);
    printf("Final Data Value: %d\n", data);
    return 0;
}
