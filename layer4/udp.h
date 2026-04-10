#ifndef LAYER4_UDP_H
#define LAYER4_UDP_H

#include <stdint.h>

typedef struct {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;
} UdpHeader;

#define UDP_HDR_LEN 8

void udp_header_init(UdpHeader *u, uint16_t sport, uint16_t dport,
                     uint16_t app_payload_octets);

#endif
