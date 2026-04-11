#ifndef LAYER3_IPV4_H
#define LAYER3_IPV4_H

/*
 * ipv4.h — 教学用极简 IPv4 首部（非完整 RFC791 字段布局，仅仿真所需子集）
 */

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t src_addr;
    uint32_t dst_addr;
    uint8_t ttl;
    uint8_t protocol;
} Ipv4Header;

#define IP_PROTO_UDP 17

void ipv4_addr_fmt(char *buf, size_t bufsz, uint32_t addr);

/* 解析点分 IPv4，得到与 ipv4_addr_fmt 一致的 32 位主机序值。成功返回 0。 */
int ipv4_parse(const char *s, uint32_t *out);

/* prefix_len 1..32：判断 ip 是否落在 network_host 所表示的前缀内（主机序）。 */
int ipv4_in_prefix(uint32_t ip, uint32_t network_host, int prefix_len);

/* 两地址是否在相同前缀下（用于判定同网段 / 二层直达）。prefix_len 须 1..32。 */
int ipv4_same_subnet(uint32_t a, uint32_t b, int prefix_len);

/* 连续前缀掩码（主机序），prefix_len 须 1..32。 */
uint32_t ipv4_netmask(int prefix_len);

#endif
