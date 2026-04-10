#ifndef LAYER2_ETH_H
#define LAYER2_ETH_H

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
