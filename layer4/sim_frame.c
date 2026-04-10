#include <stdio.h>

#include "eth.h"
#include "ipv4.h"
#include "sim_frame.h"
#include "udp.h"

static int dump_printable(const uint8_t *p, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (p[i] < 32u || p[i] > 126u) {
            return 0;
        }
    }
    return 1;
}

void sim_frame_dump(const char *title, const SimFrame *f) {
    char dm[24], sm[24], sip[20], dip[20];
    eth_mac_fmt(dm, sizeof(dm), f->eth.dst_mac);
    eth_mac_fmt(sm, sizeof(sm), f->eth.src_mac);
    ipv4_addr_fmt(sip, sizeof(sip), f->ip.src_addr);
    ipv4_addr_fmt(dip, sizeof(dip), f->ip.dst_addr);
    printf("%s\n", title);
    printf("  [L2 以太网] dst=%s src=%s type=0x%04X\n", dm, sm,
           f->eth.ether_type);
    printf("  [L3 IPv4] %s -> %s TTL=%u proto=%u\n", sip, dip, f->ip.ttl,
           f->ip.protocol);
    printf("  [L4 UDP] 端口 %u -> %u length=%u\n",
           (unsigned)f->udp.src_port, (unsigned)f->udp.dst_port,
           (unsigned)f->udp.length);
    if (f->app.sm4_on) {
        printf("  [L7 应用] SM4-CBC（国密 GB/T 32907）密文长度=%u，",
               (unsigned)f->app.sm4_cipher_len);
        printf("IV= ");
        for (int i = 0; i < 16; i++) {
            printf("%02X", f->app.iv[i]);
        }
        printf("，密文前16字节= ");
        int n = f->app.sm4_cipher_len < 16 ? f->app.sm4_cipher_len : 16;
        for (int i = 0; i < n; i++) {
            printf("%02X", f->app.sm4_cipher[i]);
        }
        printf("...\n");
    } else {
        size_t app_len = 0;
        if (f->udp.length > UDP_HDR_LEN) {
            app_len = (size_t)(f->udp.length - UDP_HDR_LEN);
        }
        if (app_len > APP_MAX_PAYLOAD) {
            app_len = APP_MAX_PAYLOAD;
        }
        printf("  [L7 应用] 明文 ");
        if (app_len == 0) {
            printf("（空）\n");
        } else if (dump_printable((const uint8_t *)f->app.text, app_len)) {
            printf("\"%.*s\"\n", (int)app_len, f->app.text);
        } else {
            printf("长度=%zu hex=", app_len);
            size_t show = app_len < 32u ? app_len : 32u;
            for (size_t i = 0; i < show; i++) {
                printf("%02X", (unsigned char)f->app.text[i]);
            }
            if (app_len > 32u) {
                printf("...");
            }
            printf("\n");
        }
    }
}
