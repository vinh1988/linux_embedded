#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "../include/shared.h"
#include "../include/sensor_data.h"

#define RUNNING_AVG_WINDOW 5

extern shared_data_t shared_data;
extern int fifo_fd;
extern volatile int running;

/* Structure to store running average data */
typedef struct {
    uint16_t node_id;
    float temperatures[RUNNING_AVG_WINDOW];
    int count;
    int index;
    int too_hot;
    int too_cold;
} running_avg_t;

/* Data manager thread function */
void *data_manager_thread(void *arg) {
    running_avg_t *running_avgs = NULL;
    int num_nodes = 0;
    
    printf("Data manager thread started\n");
    
    /* Allocate initial running average data */
    running_avgs = malloc(MAX_SENSOR_NODES * sizeof(running_avg_t));
    if (!running_avgs) {
        perror("malloc");
        running = 0;
        return NULL;
    }
    
    /* Initialize running average data */
    memset(running_avgs, 0, MAX_SENSOR_NODES * sizeof(running_avg_t));
    
    /* Main loop */
    while (running) {
        /* Sleep to avoid busy waiting */
        usleep(100000); /* 100ms */
        
        /* Lock shared data */
        pthread_mutex_lock(&shared_data.mutex);
        
        /* Process each sensor node's data */
        for (int i = 0; i < shared_data.count; i++) {
            sensor_data_t *sensor = &shared_data.data[i];
            running_avg_t *avg = NULL;
            
            /* Find or create running average data for this node */
            int j;
            for (j = 0; j < num_nodes; j++) {
                if (running_avgs[j].node_id == sensor->node_id) {
                    avg = &running_avgs[j];
                    break;
                }
            }
            
            if (!avg) {
                /* New sensor node */
                if (num_nodes < MAX_SENSOR_NODES) {
                    avg = &running_avgs[num_nodes++];
                    avg->node_id = sensor->node_id;
                    avg->count = 0;
                    avg->index = 0;
                    avg->too_hot = 0;
                    avg->too_cold = 0;
                } else {
                    /* Too many sensor nodes */
                    continue;
                }
            }
            
            /* Convert raw temperature to float (assuming 0.1 degree Celsius units) */
            float temp = sensor->temperature / 10.0f;
            
            /* Update running average */
            avg->temperatures[avg->index] = temp;
            avg->index = (avg->index + 1) % RUNNING_AVG_WINDOW;
            if (avg->count < RUNNING_AVG_WINDOW) {
                avg->count++;
            }
            
            /* Calculate running average */
            float sum = 0.0f;
            for (int k = 0; k < avg->count; k++) {
                sum += avg->temperatures[k];
            }
            float running_avg = sum / avg->count;
            
            /* Check temperature thresholds */
            if (running_avg > TEMP_TOO_HOT && !avg->too_hot) {
                /* Temperature is too hot */
                avg->too_hot = 1;
                avg->too_cold = 0;
                
                /* Log event */
                char msg[100];
                sprintf(msg, "The sensor node with %d reports it's too hot", sensor->node_id);
                write_log_event(fifo_fd, LOG_TOO_HOT, sensor->node_id, running_avg, msg);
            } else if (running_avg < TEMP_TOO_COLD && !avg->too_cold) {
                /* Temperature is too cold */
                avg->too_cold = 1;
                avg->too_hot = 0;
                
                /* Log event */
                char msg[100];
                sprintf(msg, "The sensor node with %d reports it's too cold", sensor->node_id);
                write_log_event(fifo_fd, LOG_TOO_COLD, sensor->node_id, running_avg, msg);
            } else if (running_avg >= TEMP_TOO_COLD && running_avg <= TEMP_TOO_HOT) {
                /* Temperature is normal */
                avg->too_hot = 0;
                avg->too_cold = 0;
            }
        }
        
        /* Unlock shared data */
        pthread_mutex_unlock(&shared_data.mutex);
    }
    
    /* Clean up */
    free(running_avgs);
    printf("Data manager thread exiting\n");
    return NULL;
}
