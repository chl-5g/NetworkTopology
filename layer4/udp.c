/* udp.c — UDP 首部初始化（checksum 恒为 0，仿真不计算伪首部校验和） */
#include "udp.h"

void udp_header_init(UdpHeader *u, uint16_t sport, uint16_t dport,
                     uint16_t app_payload_octets) {
    u->src_port = sport;
    u->dst_port = dport;
    u->length = (uint16_t)(UDP_HDR_LEN + app_payload_octets);
    u->checksum = 0;
}
