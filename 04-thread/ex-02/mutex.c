#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#define NUM_THREADS 3
#define INCREMENTS_PER_THREAD 1000000

int counter = 0;  // Shared global counter
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* increasement_counter(void* arg){
    for (int i=0;i<INCREMENTS_PER_THREAD; i++){
        pthread_mutex_lock(&mutex);
        counter++;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(){
    pthread_t threads[NUM_THREADS];

    //create threads
    for (int i=0;i<NUM_THREADS; i++){
        pthread_create(&threads[i], NULL, increasement_counter, NULL);
    }
    // wait for threads to finish
    for (int i=0;i<NUM_THREADS; i++){
        pthread_join(threads[i], NULL);
    }
    pthread_mutex_destroy(&mutex);

    printf("Final counter value: %d\n", counter);
    return 0;
}