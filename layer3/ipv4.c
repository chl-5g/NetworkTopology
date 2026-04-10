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
