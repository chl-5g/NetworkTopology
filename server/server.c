#include <stdio.h>

#include "ipv4.h"
#include "server.h"
#include "sm_payload.h"
#include "sm_session.h"
#include "udp.h"

static int payload_all_printable(const uint8_t *p, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (p[i] < 32u || p[i] > 126u) {
            return 0;
        }
    }
    return 1;
}

static void print_payload_line(const char *prefix, const uint8_t *p, size_t len) {
    printf("%s", prefix);
    if (len == 0) {
        printf("（空载荷）\n");
        return;
    }
    if (payload_all_printable(p, len)) {
        printf("\"%.*s\"\n", (int)len, (const char *)p);
        return;
    }
    printf("长度=%zu hex=", len);
    size_t show = len < 64u ? len : 64u;
    for (size_t i = 0; i < show; i++) {
        printf("%02X", p[i]);
    }
    if (len > 64u) {
        printf("...");
    }
    printf("\n");
}

void server_receive(const SimHost *n, const SimFrame *in) {
    char dip[20];
    ipv4_addr_fmt(dip, sizeof(dip), in->ip.dst_addr);

    if (in->app.sm4_on) {
        uint8_t plain[APP_MAX_PAYLOAD];
        size_t plen;
        if (sm4_decrypt_buffer(SM4_PSK, in->app.iv, in->app.sm4_cipher,
                               in->app.sm4_cipher_len, plain, sizeof(plain),
                               &plen) != 0) {
            printf("[服务端 节点 %d] SM4 解密失败（密文可能被篡改）\n", n->id);
            return;
        }
        printf("[服务端 节点 %d] 收到: 目的 IP %s，UDP %u，SM4 解密后 ",
               n->id, dip, (unsigned)in->udp.dst_port);
        print_payload_line("", plain, plen);
    } else {
        size_t app_len = 0;
        if (in->udp.length > UDP_HDR_LEN) {
            app_len = (size_t)(in->udp.length - UDP_HDR_LEN);
        }
        if (app_len > APP_MAX_PAYLOAD) {
            app_len = APP_MAX_PAYLOAD;
        }
        printf("[服务端 节点 %d] 收到: 目的 IP %s，UDP %u，明文 ",
               n->id, dip, (unsigned)in->udp.dst_port);
        print_payload_line("", (const uint8_t *)in->app.text, app_len);
    }
}
