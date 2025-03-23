#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_ITERATIONS 10


int data = 0;  // Shared global variable
int data_ready = 0;  // Flag to indicate data availability

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void* producer(void* arg){
    for (int i=1;i<NUM_ITERATIONS+1;i++){
        // generate ramdom number
        int random_number = rand()%10 +1;

        pthread_mutex_lock(&mutex);
        data = random_number;
        data_ready = 1;
        printf("Producer: generated %d \n", data);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
        sleep(1);

    }
    return NULL;
}

void* consumer(void* arg){
    for (int i =1; i<NUM_ITERATIONS+1;i++){
        pthread_mutex_lock(&mutex);
        while (!data_ready){
            pthread_cond_wait(&cond, &mutex); // sleep &cond and release &mutex

        }
        printf("Consumer: Read %d \n", data);
        data_ready = 0; // reset flag
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(){
    srand(time(NULL)); // seed random 
    pthread_t producer_thread, consumer_thread;

    //create thread
    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    // wait for thread to finish
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    // Destroy mutex & condition variable
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    return 0;
}