/*
 * switch.c — 二层交换机简化模型
 *
 * CAM（Content-Addressable Memory）：源 MAC 自学习（入端口 -> 源 MAC 绑定）。
 * 转发：若目的 MAC 在 CAM 中则单播到对应端口；否则向除入端口外的所有端口泛洪。
 * 本仿真在泛洪时仍只保留「一条」SimFrame 继续往下走，取编号最小的出端口作为
 * 代表路径（见打印「仍沿首出端口继续后续逻辑」）。
 */
#include <stdio.h>
#include <string.h>

#include "layer_pdu_print.h"
#include "sim_frame.h"
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
    cam_insert(sw, f->eth.src_mac, in_port);
}

int switch_forward_port(Switch *sw, const SimFrame *f, int in_port,
                        const char *pdu_where) {
    switch_learn(sw, f, in_port);
    int egress = cam_find(sw, f->eth.dst_mac);
    if (egress >= 0) {
        printf("[数据链路层/交换机] CAM 命中: 目的 MAC -> 端口 %d\n", egress);
        if (pdu_where != NULL) {
            layer_pdu_print(2, pdu_where, f);
        }
        return egress;
    }
    printf("[数据链路层/交换机] 目的 MAC 未知，向除入端口 %d 外的所有端口泛洪\n",
           in_port);
    int first_egress = -1;
    for (int p = 0; p < sw->port_count; p++) {
        if (p == in_port) {
            continue;
        }
        if (first_egress < 0) {
            first_egress = p;
        }
        printf("  -> 端口 %d\n", p);
        if (pdu_where != NULL) {
            layer_pdu_print(2, pdu_where, f);
        }
    }
    if (first_egress >= 0) {
        printf("  （每端口一份副本；仿真仍沿首出端口 %d 继续后续逻辑）\n",
               first_egress);
        return first_egress;
    }
    if (pdu_where != NULL) {
        layer_pdu_print(2, pdu_where, f);
    }
    return -1;
}
