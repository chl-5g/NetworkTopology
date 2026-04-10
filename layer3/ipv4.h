#ifndef LAYER3_IPV4_H
#define LAYER3_IPV4_H

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

#endif
