#ifndef LAYER4_UDP_H
#define LAYER4_UDP_H

#include <stdint.h>

/* RFC768 风格字段；length 为「首部+载荷」总字节数 */
typedef struct {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;
    uint16_t checksum; /* RFC 768 第 4 字段；本仿真固定 0，不参与校验 */
} UdpHeader;

#define UDP_HDR_LEN 8

_Static_assert(sizeof(UdpHeader) == UDP_HDR_LEN,
               "UdpHeader 须与 UDP_HDR_LEN 一致（RFC 768 首部 8 字节）");

void udp_header_init(UdpHeader *u, uint16_t sport, uint16_t dport,
                     uint16_t app_payload_octets);

#endif
