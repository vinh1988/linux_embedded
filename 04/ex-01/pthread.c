#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#define NUMBER_THREADS 2

void* print_message(void* arg){
    int thread_id = *(int*)arg;
    for (int i=0; i<NUMBER_THREADS; i++){}
    printf("Thread %d: Hello from thread\n", thread_id);
}


int main(){
    pthread_t threads[NUMBER_THREADS];
    for (int i=0; i<NUMBER_THREADS+1; i++){
        int* thread_id = malloc(sizeof(int));
        *thread_id = i+1;
        pthread_create(&threads[i],NULL, print_message,thread_id);

    }

    // waiting for all threads finish
    for (int i=1; i<NUMBER_THREADS;i++){
        pthread_join(threads[i], NULL);
    }

    // destroy all threads
    printf("All threads completed!\n");

}