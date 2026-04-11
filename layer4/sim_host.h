#ifndef LAYER4_SIM_HOST_H
#define LAYER4_SIM_HOST_H

#include <stdint.h>

/* 端系统最小属性：发端用 gw_* 作为默认 L2 下一跳；收端可把 gw_mac 填成本机 MAC */
typedef struct {
    int id;
    uint8_t mac[6];
    uint32_t ip;
    uint8_t gw_mac[6];
    uint32_t gw_ip;
} SimHost;

#endif
