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

/* 解析点分 IPv4，得到与 ipv4_addr_fmt 一致的 32 位主机序值。成功返回 0。 */
int ipv4_parse(const char *s, uint32_t *out);

#endif
