#include <stdio.h>
#include <string.h>

#include "sim_frame.h"

void sim_mac_fmt(char *buf, size_t bufsz, const uint8_t mac[6]) {
    snprintf(buf, bufsz, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1],
             mac[2], mac[3], mac[4], mac[5]);
}

void sim_ip_fmt(char *buf, size_t bufsz, uint32_t ip) {
    snprintf(buf, bufsz, "%u.%u.%u.%u", (unsigned)((ip >> 24) & 0xFF),
             (unsigned)((ip >> 16) & 0xFF), (unsigned)((ip >> 8) & 0xFF),
             (unsigned)(ip & 0xFF));
}

void sim_frame_dump(const char *title, const SimFrame *f) {
    char dm[24], sm[24], sip[20], dip[20];
    sim_mac_fmt(dm, sizeof(dm), f->dst_mac);
    sim_mac_fmt(sm, sizeof(sm), f->src_mac);
    sim_ip_fmt(sip, sizeof(sip), f->src_ip);
    sim_ip_fmt(dip, sizeof(dip), f->dst_ip);
    printf("%s\n", title);
    printf("  以太网: dst=%s src=%s type=0x%04X\n", dm, sm, f->ether_type);
    printf("  IPv4: %s -> %s TTL=%u\n", sip, dip, f->ttl);
    if (f->sm4_on) {
        printf("  载荷: SM4-CBC（国密 GB/T 32907）密文长度=%u，",
               (unsigned)f->sm4_cipher_len);
        printf("IV= ");
        for (int i = 0; i < 16; i++) {
            printf("%02X", f->iv[i]);
        }
        printf("，密文前16字节= ");
        int n = f->sm4_cipher_len < 16 ? f->sm4_cipher_len : 16;
        for (int i = 0; i < n; i++) {
            printf("%02X", f->sm4_cipher[i]);
        }
        printf("...\n");
    } else {
        printf("  载荷: 明文 \"%s\"\n", f->payload);
    }
}
