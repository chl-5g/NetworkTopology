#include <stdio.h>
#include <string.h>

#include "app_node.h"
#include "ipv4.h"
#include "sm_payload.h"
#include "sm_session.h"
#include "udp.h"

#define DEMO_UDP_SPORT 49152u
#define DEMO_UDP_DPORT 50000u

static void frame_fill_l2_l3(Node *n, uint32_t dst_ip, SimFrame *out) {
    memcpy(out->eth.dst_mac, n->gw_mac, 6);
    memcpy(out->eth.src_mac, n->mac, 6);
    out->eth.ether_type = ETH_TYPE_IPV4;
    out->ip.src_addr = n->ip;
    out->ip.dst_addr = dst_ip;
    out->ip.ttl = 64;
    out->ip.protocol = IP_PROTO_UDP;
}

void node_init(Node *n, int id, const uint8_t mac[6], uint32_t ip,
               const uint8_t gw_mac[6], uint32_t gw_ip) {
    n->id = id;
    memcpy(n->mac, mac, 6);
    n->ip = ip;
    memcpy(n->gw_mac, gw_mac, 6);
    n->gw_ip = gw_ip;
}

void node_emit_frame(Node *n, uint32_t dst_ip, const char *msg, SimFrame *out) {
    memset(out, 0, sizeof(*out));
    frame_fill_l2_l3(n, dst_ip, out);
    out->app.sm4_on = 0;
    strncpy(out->app.text, msg, APP_MAX_PAYLOAD - 1);
    out->app.text[APP_MAX_PAYLOAD - 1] = '\0';
    udp_header_init(&out->udp, DEMO_UDP_SPORT, DEMO_UDP_DPORT,
                    (uint16_t)strlen(out->app.text));
}

void node_emit_frame_sm4(Node *n, uint32_t dst_ip, const char *msg,
                         SimFrame *out) {
    memset(out, 0, sizeof(*out));
    frame_fill_l2_l3(n, dst_ip, out);
    out->app.sm4_on = 1;
    memcpy(out->app.iv, SM4_IV, 16);
    size_t clen;
    if (sm4_encrypt_message(SM4_PSK, SM4_IV, msg, out->app.sm4_cipher,
                            sizeof(out->app.sm4_cipher), &clen) != 0) {
        fprintf(stderr, "[节点 %d] SM4 加密失败\n", n->id);
        out->app.sm4_cipher_len = 0;
        udp_header_init(&out->udp, DEMO_UDP_SPORT, DEMO_UDP_DPORT, 0);
        return;
    }
    out->app.sm4_cipher_len = (uint16_t)clen;
    udp_header_init(&out->udp, DEMO_UDP_SPORT, DEMO_UDP_DPORT,
                    out->app.sm4_cipher_len);
}

void node_receive(const Node *n, const SimFrame *in) {
    char dip[20];
    ipv4_addr_fmt(dip, sizeof(dip), in->ip.dst_addr);

    if (in->app.sm4_on) {
        char plain[APP_MAX_PAYLOAD];
        if (sm4_decrypt_message(SM4_PSK, in->app.iv, in->app.sm4_cipher,
                              in->app.sm4_cipher_len, plain,
                              sizeof(plain)) != 0) {
            printf("[L7/节点 %d] SM4 解密失败（密文可能被篡改）\n", n->id);
            return;
        }
        printf("[L7/节点 %d] 收到: 目的 IP %s，UDP %u，SM4 解密后 \"%s\"\n",
               n->id, dip, (unsigned)in->udp.dst_port, plain);
    } else {
        printf("[L7/节点 %d] 收到: 目的 IP %s，UDP %u，明文 \"%s\"\n", n->id,
               dip, (unsigned)in->udp.dst_port, in->app.text);
    }
}
