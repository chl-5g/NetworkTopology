#ifndef LAYER4_SIM_HOST_H
#define LAYER4_SIM_HOST_H

#include <stdint.h>

typedef struct {
    int id;
    uint8_t mac[6];
    uint32_t ip;
    uint8_t gw_mac[6];
    uint32_t gw_ip;
} SimHost;

#endif
