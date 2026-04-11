#ifndef LAYER2_SWITCH_H
#define LAYER2_SWITCH_H

/*
 * switch.h — 二层交换机状态与 API
 *
 * port_count：逻辑端口数（>=2）。cam_*：自学习得到的 MAC->端口表项。
 */

#include <stdint.h>

#include "sim_frame.h"

#define SW_MAX_PORTS 8
#define SW_CAM_ENTRIES 32

typedef struct {
    int port_count;
    uint8_t cam_mac[SW_CAM_ENTRIES][6];
    int cam_port[SW_CAM_ENTRIES];
    int cam_n;
} Switch;

void switch_init(Switch *sw, int port_count);
void switch_learn(Switch *sw, const SimFrame *f, int in_port);
int switch_forward_port(Switch *sw, const SimFrame *f, int in_port,
                        const char *pdu_where);

#endif
