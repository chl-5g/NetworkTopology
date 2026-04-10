#ifndef SIM_FRAME_H
#define SIM_FRAME_H

#include <stdint.h>

#include "eth.h"
#include "ipv4.h"
#include "udp.h"

#define APP_MAX_PAYLOAD 128
#define APP_SM4_CIPHER_MAX 160

typedef struct {
    uint8_t sm4_on;
    uint8_t iv[16];
    uint16_t sm4_cipher_len;
    uint8_t sm4_cipher[APP_SM4_CIPHER_MAX];
    char text[APP_MAX_PAYLOAD];
} AppPayload;

typedef struct {
    EthHeader eth;
    Ipv4Header ip;
    UdpHeader udp;
    AppPayload app;
} SimFrame;

void sim_frame_dump(const char *title, const SimFrame *f);

#define SIM_ETH_IPV4 ETH_TYPE_IPV4
#define SIM_MAX_PAYLOAD APP_MAX_PAYLOAD
#define SIM_SM4_CIPHER_MAX APP_SM4_CIPHER_MAX

#endif
