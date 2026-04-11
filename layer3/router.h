#ifndef LAYER3_ROUTER_H
#define LAYER3_ROUTER_H

/*
 * router.h — 路由器数据结构
 *
 * FIB_Entry 定义见 topology.h；next_hop=-1 表示直连出接口（本仿真路由器仅用此种）。
 * arp_*：目的 IP -> 下一跳 MAC，供转发时封装以太网；须由 sim_main 预置。
 */

#include <stdint.h>

#include "sim_frame.h"
#include "topology.h"

#define RT_MAX_IF 4
#define RT_MAX_FIB 32
#define RT_MAX_ARP 16

typedef struct {
    uint8_t mac[6];
    uint32_t ip;
    char attached_cidr[24];
} RtIface;

typedef struct {
    RtIface ifs[RT_MAX_IF];
    int num_if;
    FIB_Entry fib[RT_MAX_FIB];
    int fib_n;
    uint32_t arp_ip[RT_MAX_ARP];
    uint8_t arp_mac[RT_MAX_ARP][6];
    int arp_n;
} Router;

/*
 * 两接口路由器：if0 接 A 侧 LAN，if1 接 B 侧 LAN；FIB 为两条直连前缀。
 * mac/ip/prefix 均为主机序 IPv4 与本地 MAC；prefix_len 须 1..32。
 */
void router_init_configured(Router *r, const uint8_t mac_if0[6], uint32_t ip_if0,
                            int prefix_len_if0, const uint8_t mac_if1[6],
                            uint32_t ip_if1, int prefix_len_if1);
void router_add_arp(Router *r, uint32_t ip, const uint8_t mac[6]);
int router_process(Router *r, SimFrame *f, int in_if);

#endif
