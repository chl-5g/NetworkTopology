/*
 * sim_frame.c — SimFrame 调试输出
 *
 * sim_frame_dump 只 dump「逻辑上已用」的字节范围（以太网头起至 UDP 载荷末），
 * 避免把 app 里未使用的大缓冲区整段打印出来。
 */
#include <stdio.h>

#include "eth.h"
#include "ipv4.h"
#include "sim_frame.h"

/* 从 SimFrame 首字节到「应用层已用区」末尾（不含 sm4_cipher/text 数组余量） */
static size_t simframe_dump_used_octets(const SimFrame *f) {
    if (f->app.sm4_on) {
        size_t clen = (size_t)f->app.sm4_cipher_len;
        if (clen > APP_SM4_CIPHER_MAX) {
            clen = APP_SM4_CIPHER_MAX;
        }
        const uint8_t *end = f->app.sm4_cipher + clen;
        return (size_t)(end - (const uint8_t *)f);
    }
    size_t ulen = 0;
    if (f->udp.length > UDP_HDR_LEN) {
        ulen = (size_t)f->udp.length - UDP_HDR_LEN;
    }
    if (ulen > APP_MAX_PAYLOAD) {
        ulen = APP_MAX_PAYLOAD;
    }
    const uint8_t *end = (const uint8_t *)f->app.text + ulen;
    return (size_t)(end - (const uint8_t *)f);
}

void sim_hex_line(const uint8_t *p, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (i > 0) {
            printf(" ");
        }
        printf("%02X", p[i]);
    }
    printf("\n");
}

void sim_octets_hex_only(const char *label, const uint8_t *p, size_t len) {
    if (label != NULL && label[0] != '\0') {
        printf("%s\n", label);
    }
    if (len == 0) {
        printf("  （0 字节）\n");
        return;
    }
    for (size_t i = 0; i < len; i += 16) {
        printf("  %04zx ", (unsigned long)i);
        for (size_t j = 0; j < 16; j++) {
            if (i + j < len) {
                printf("%02X ", p[i + j]);
            } else {
                printf("   ");
            }
        }
        printf("\n");
    }
}

void sim_octets_print(const char *label, const uint8_t *p, size_t len) {
    if (label != NULL && label[0] != '\0') {
        printf("%s\n", label);
    }
    if (len == 0) {
        printf("  （0 字节）\n");
        return;
    }
    for (size_t i = 0; i < len; i += 16) {
        printf("  %04zx ", (unsigned long)i);
        for (size_t j = 0; j < 16; j++) {
            if (i + j < len) {
                printf("%02X ", p[i + j]);
            } else {
                printf("   ");
            }
        }
        printf(" |");
        for (size_t j = 0; j < 16 && i + j < len; j++) {
            unsigned char c = p[i + j];
            printf("%c", (c >= 32u && c < 127u) ? (char)c : '.');
        }
        printf("|\n");
    }
}

void sim_frame_dump(const char *title, const SimFrame *f) {
    char dm[24], sm[24], sip[20], dip[20];
    eth_mac_fmt(dm, sizeof(dm), f->eth.dst_mac);
    eth_mac_fmt(sm, sizeof(sm), f->eth.src_mac);
    ipv4_addr_fmt(sip, sizeof(sip), f->ip.src_addr);
    ipv4_addr_fmt(dip, sizeof(dip), f->ip.dst_addr);
    printf("%s\n", title);
    printf("  摘要 L2/L3/L4: eth dst=%s src=%s | %s -> %s TTL=%u | UDP %u->%u "
           "udp_len=%u | app.sm4_on=%u\n",
           dm, sm, sip, dip, (unsigned)f->ip.ttl, (unsigned)f->udp.src_port,
           (unsigned)f->udp.dst_port, (unsigned)f->udp.length,
           (unsigned)f->app.sm4_on);
    size_t used = simframe_dump_used_octets(f);
    printf("  SimFrame 内存布局：sizeof=%zu（含大数组与对齐）；以下 hexdump 仅「已用区」"
           "%zu 字节（eth+ip+udp+app 至密文末或 text 已用末），避免刷屏\n",
           sizeof(*f), used);
    sim_octets_hex_only(NULL, (const uint8_t *)f, used);
    printf("  SimFrame 单行连续 HEX（同上，已用区）:\n    ");
    sim_hex_line((const uint8_t *)f, used);
}
