#ifndef LAYER4_SIM_FRAME_H
#define LAYER4_SIM_FRAME_H

#include <stddef.h>
#include <stdint.h>

#include "eth.h"
#include "ipv4.h"
#include "udp.h"

#define APP_MAX_PAYLOAD 128
#define APP_SM4_CIPHER_MAX 1024

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

/* 十六进制 + 可打印 ASCII 侧栏（用于文本载荷等） */
void sim_octets_print(const char *label, const uint8_t *p, size_t len);

/* 仅偏移 + 十六进制，无 ASCII 列，不截断 */
void sim_octets_hex_only(const char *label, const uint8_t *p, size_t len);

/* 单行输出全部字节，%02X 空格分隔，无省略 */
void sim_hex_line(const uint8_t *p, size_t len);

#define SIM_ETH_IPV4 ETH_TYPE_IPV4
#define SIM_MAX_PAYLOAD APP_MAX_PAYLOAD
#define SIM_SM4_CIPHER_MAX APP_SM4_CIPHER_MAX

#endif
