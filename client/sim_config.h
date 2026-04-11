#ifndef CLIENT_SIM_CONFIG_H
#define CLIENT_SIM_CONFIG_H

#include <stddef.h>
#include <stdint.h>

#define SIM_CONFIG_PATH_MAX 512

typedef struct {
    int node_a_id;
    int node_b_id;
    int switch_port_count;
    uint32_t node_a_ip;
    uint32_t node_b_ip;
    uint32_t node_a_gw_ip;
    uint32_t node_b_gw_ip;
    uint8_t node_a_mac[6];
    uint8_t node_b_mac[6];
    uint8_t node_a_gw_mac[6];
    uint8_t node_b_gw_mac[6];
    int route_prefix_a;
    int route_prefix_b;
    /* 主机侧 ARP 静态扩展：IP 为 0.0.0.0 表示未使用；须与 ROUTE_PREFIX_A 同子网才可命中 */
    uint32_t extra_arp1_ip;
    uint32_t extra_arp2_ip;
    uint8_t extra_arp1_mac[6];
    uint8_t extra_arp2_mac[6];
    uint16_t udp_sport;
    uint16_t udp_dport;
    int use_sm4;
    int lan_prefix_len;
    char packet_file[SIM_CONFIG_PATH_MAX];
} SimNetConfig;

/*
 * 从配置文件读取全部固定项（键名须为大写，见 config/network.conf）。
 * 不允许缺项；无内置默认值。成功返回 0。
 * LAN_PREFIX_LEN：判定 A/B 是否同网段（同网段则不经路由器）。
 * ROUTE_PREFIX_A/B、NODE_B_GW_*：跨网段时由路由器生成 FIB（须与网关、对端 IP 一致）。
 */
int sim_net_config_load(const char *path, SimNetConfig *out);

/* 读取载荷文件（按字节）。成功返回 0；允许空文件。 */
int sim_packet_load(const char *path, uint8_t *buf, size_t cap, size_t *out_len);

#endif
