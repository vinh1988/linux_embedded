#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define SIZE 100
int numbers[SIZE];
int even_count = 0;
int odd_count = 0;

void* count_even(void* arg){
    for (int i=0;i<SIZE; i++){
        if (numbers[i]%2==0){
        even_count += numbers[i];
    }

    }
    pthread_exit(NULL);
}

void* count_odd(void* arg){
    for (int i=0;i<SIZE; i++){
        if (numbers[i]%2==1){
        odd_count+= numbers[i];
    }

    }
    pthread_exit(NULL);
}

int main(){
    pthread_t even_thread, odd_thread;
    // seed random number generator
    srand(time(NULL));
    // generate randdom number between 1 & 100
    printf("Generate Numbers:\n");
    for (int i=0;i<SIZE;i++){
        numbers[i]= rand()%100+1;
        printf("%d\n", numbers[i]);
    }
    printf("\n");
    // create 2 threads
    pthread_create(&even_thread, NULL, count_even, NULL);
    pthread_create(&odd_thread, NULL, count_odd, NULL);

    // waiting for 2 thread until finish
    pthread_join(even_thread, NULL);
    pthread_join(odd_thread, NULL);

    // print results
    printf("Total even numbers: %d\n", even_count);
    printf("Total odd numbers: %d\n", odd_count);

    return 0;

}