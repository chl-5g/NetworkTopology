/*
 * ipv4.c — IPv4 地址工具（全部使用主机字节序 uint32_t，与点分十进制互转）
 */
#include <stdio.h>

#include "ipv4.h"

int ipv4_parse(const char *s, uint32_t *out) {
    unsigned a, b, c, d;
    if (sscanf(s, "%u.%u.%u.%u", &a, &b, &c, &d) != 4) {
        return -1;
    }
    if (a > 255u || b > 255u || c > 255u || d > 255u) {
        return -1;
    }
    *out = ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)c << 8) | d;
    return 0;
}

void ipv4_addr_fmt(char *buf, size_t bufsz, uint32_t addr) {
    snprintf(buf, bufsz, "%u.%u.%u.%u", (unsigned)((addr >> 24) & 0xFF),
             (unsigned)((addr >> 16) & 0xFF), (unsigned)((addr >> 8) & 0xFF),
             (unsigned)(addr & 0xFF));
}

uint32_t ipv4_netmask(int prefix_len) {
    if (prefix_len < 1) {
        return 0;
    }
    if (prefix_len >= 32) {
        return 0xFFFFFFFFu;
    }
    return 0xFFFFFFFFu << (unsigned)(32 - prefix_len);
}

static uint32_t ipv4_prefix_mask(int prefix_len) {
    return ipv4_netmask(prefix_len);
}

int ipv4_in_prefix(uint32_t ip, uint32_t network_host, int prefix_len) {
    if (prefix_len < 1 || prefix_len > 32) {
        return 0;
    }
    uint32_t m = ipv4_prefix_mask(prefix_len);
    return (ip & m) == (network_host & m);
}

int ipv4_same_subnet(uint32_t a, uint32_t b, int prefix_len) {
    if (prefix_len < 1 || prefix_len > 32) {
        return 0;
    }
    uint32_t m = ipv4_prefix_mask(prefix_len);
    return (a & m) == (b & m);
}
