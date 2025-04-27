#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <stdint.h>

/* Sensor packet format (as defined in lab 7) */
#pragma pack(1)
typedef struct {
    uint16_t node_id;
    uint16_t temperature;
    uint16_t humidity;
    uint16_t light;
} sensor_packet_t;
#pragma pack()

#endif /* SENSOR_DATA_H */
