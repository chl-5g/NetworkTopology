#include <stdio.h>
#include <string.h>

#include "node.h"
#include "sm_payload.h"
#include "sm_session.h"

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
    memcpy(out->dst_mac, n->gw_mac, 6);
    memcpy(out->src_mac, n->mac, 6);
    out->ether_type = SIM_ETH_IPV4;
    out->src_ip = n->ip;
    out->dst_ip = dst_ip;
    out->ttl = 64;
    out->sm4_on = 0;
    strncpy(out->payload, msg, SIM_MAX_PAYLOAD - 1);
    out->payload[SIM_MAX_PAYLOAD - 1] = '\0';
}

void node_emit_frame_sm4(Node *n, uint32_t dst_ip, const char *msg,
                         SimFrame *out) {
    memset(out, 0, sizeof(*out));
    memcpy(out->dst_mac, n->gw_mac, 6);
    memcpy(out->src_mac, n->mac, 6);
    out->ether_type = SIM_ETH_IPV4;
    out->src_ip = n->ip;
    out->dst_ip = dst_ip;
    out->ttl = 64;
    out->sm4_on = 1;
    memcpy(out->iv, SM4_IV, 16);
    size_t clen;
    if (sm4_encrypt_message(SM4_PSK, SM4_IV, msg, out->sm4_cipher,
                            sizeof(out->sm4_cipher), &clen) != 0) {
        fprintf(stderr, "[节点 %d] SM4 加密失败\n", n->id);
        out->sm4_cipher_len = 0;
        return;
    }
    out->sm4_cipher_len = (uint16_t)clen;
}

void node_receive(const Node *n, const SimFrame *in) {
    char dip[20];
    sim_ip_fmt(dip, sizeof(dip), in->dst_ip);

    if (in->sm4_on) {
        char plain[SIM_MAX_PAYLOAD];
        if (sm4_decrypt_message(SM4_PSK, in->iv, in->sm4_cipher,
                                in->sm4_cipher_len, plain,
                                sizeof(plain)) != 0) {
            printf("[节点 %d] SM4 解密失败（密文可能被篡改）\n", n->id);
            return;
        }
        printf("[节点 %d] 收到帧: 目的 IP %s，SM4 解密后 \"%s\"\n", n->id, dip,
               plain);
    } else {
        printf("[节点 %d] 收到帧: 目的 IP %s，明文 \"%s\"\n", n->id, dip,
               in->payload);
    }
}
