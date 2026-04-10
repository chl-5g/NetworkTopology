#include <stdio.h>

#include "ipv4.h"

void ipv4_addr_fmt(char *buf, size_t bufsz, uint32_t addr) {
    snprintf(buf, bufsz, "%u.%u.%u.%u", (unsigned)((addr >> 24) & 0xFF),
             (unsigned)((addr >> 16) & 0xFF), (unsigned)((addr >> 8) & 0xFF),
             (unsigned)(addr & 0xFF));
}
