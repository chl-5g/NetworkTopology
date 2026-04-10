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
    uint8_t node_a_mac[6];
    uint8_t node_b_mac[6];
    uint8_t node_a_gw_mac[6];
    uint16_t udp_sport;
    uint16_t udp_dport;
    int use_sm4;
    char packet_file[SIM_CONFIG_PATH_MAX];
} SimNetConfig;

/*
 * 从配置文件读取全部固定项（键名须为大写，见 config/network.conf）。
 * 不允许缺项；无内置默认值。成功返回 0。
 */
int sim_net_config_load(const char *path, SimNetConfig *out);

/* 读取载荷文件（按字节）。成功返回 0；允许空文件。 */
int sim_packet_load(const char *path, uint8_t *buf, size_t cap, size_t *out_len);

#endif
