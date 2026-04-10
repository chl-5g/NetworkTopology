#include <stdio.h>
#include <string.h>

#include "switch.h"

static int mac_equal(const uint8_t a[6], const uint8_t b[6]) {
    return memcmp(a, b, 6) == 0;
}

static int cam_find(const Switch *sw, const uint8_t mac[6]) {
    for (int i = 0; i < sw->cam_n; i++) {
        if (mac_equal(sw->cam_mac[i], mac)) {
            return sw->cam_port[i];
        }
    }
    return -1;
}

static void cam_insert(Switch *sw, const uint8_t mac[6], int port) {
    for (int i = 0; i < sw->cam_n; i++) {
        if (mac_equal(sw->cam_mac[i], mac)) {
            sw->cam_port[i] = port;
            return;
        }
    }
    if (sw->cam_n >= SW_CAM_ENTRIES) {
        return;
    }
    memcpy(sw->cam_mac[sw->cam_n], mac, 6);
    sw->cam_port[sw->cam_n] = port;
    sw->cam_n++;
}

void switch_init(Switch *sw, int port_count) {
    memset(sw, 0, sizeof(*sw));
    sw->port_count = port_count;
}

void switch_learn(Switch *sw, const SimFrame *f, int in_port) {
    cam_insert(sw, f->src_mac, in_port);
}

int switch_forward_port(Switch *sw, const SimFrame *f, int in_port) {
    switch_learn(sw, f, in_port);
    int egress = cam_find(sw, f->dst_mac);
    if (egress >= 0) {
        printf("[交换机] CAM 命中: 目的 MAC -> 端口 %d\n", egress);
        return egress;
    }
    printf("[交换机] 目的 MAC 未知，单端口泛洪到除 %d 外的端口\n", in_port);
    for (int p = 0; p < sw->port_count; p++) {
        if (p != in_port) {
            return p;
        }
    }
    return -1;
}
