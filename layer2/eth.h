#ifndef LAYER2_ETH_H
#define LAYER2_ETH_H

/* 教学用以太网首部：无 VLAN 标签；ether_type 主机序，IPv4 常用 0x0800 */

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t dst_mac[6];
    uint8_t src_mac[6];
    uint16_t ether_type;
} EthHeader;

#define ETH_TYPE_IPV4 0x0800u

void eth_mac_fmt(char *buf, size_t bufsz, const uint8_t mac[6]);

#endif
