#ifndef SIM_FRAME_H
#define SIM_FRAME_H

#include <stddef.h>
#include <stdint.h>

#define SIM_ETH_IPV4 0x0800u
#define SIM_MAX_PAYLOAD 128
#define SIM_SM4_CIPHER_MAX 160

typedef struct {
    uint8_t dst_mac[6];
    uint8_t src_mac[6];
    uint16_t ether_type;
    uint32_t src_ip;
    uint32_t dst_ip;
    uint8_t ttl;
    uint8_t sm4_on;
    uint8_t iv[16];
    uint16_t sm4_cipher_len;
    uint8_t sm4_cipher[SIM_SM4_CIPHER_MAX];
    char payload[SIM_MAX_PAYLOAD];
} SimFrame;

void sim_mac_fmt(char *buf, size_t bufsz, const uint8_t mac[6]);
void sim_ip_fmt(char *buf, size_t bufsz, uint32_t ip);
void sim_frame_dump(const char *title, const SimFrame *f);

#endif
