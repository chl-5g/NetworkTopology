/*
 * client.c — 发送端（节点 A）组帧逻辑
 *
 * SimFrame 在本项目里是「一整块内存里的协议栈」：自下而上依次有 EthHeader、
 * Ipv4Header、UdpHeader、AppPayload。本文件负责：
 *   - 填写 L2/L3 常用字段（源/目的 MAC、源 IP、目的 IP、TTL、协议号 UDP）；
 *   - 将 PACKET_FILE 或字符串载荷放入 L7 区（明文 text[] 或 SM4 密文区）；
 *   - 设置 UDP length = 8 + 应用载荷长度。
 *
 * client_emit_frame_payload_l2 与同名的非 _l2 版本区别：前者由 ARP 结果直接指定
 * 以太网目的 MAC（同网段写对端主机；跨网段写网关），后者默认把目的 MAC 写成 gw_mac。
 */
#include <stdio.h>
#include <string.h>

#include "client.h"
#include "ipv4.h"
#include "sm_payload.h"
#include "sm_session.h"
#include "udp.h"

#define DEMO_UDP_SPORT 49152u
#define DEMO_UDP_DPORT 50000u

/* eth_dst_mac 非 NULL 时用其作为以太网目的地址；否则用主机配置的默认网关 MAC */
static void frame_fill_l2_l3(SimHost *n, uint32_t dst_ip, SimFrame *out,
                             const uint8_t *eth_dst_mac) {
    if (eth_dst_mac != NULL) {
        memcpy(out->eth.dst_mac, eth_dst_mac, 6);
    } else {
        memcpy(out->eth.dst_mac, n->gw_mac, 6);
    }
    memcpy(out->eth.src_mac, n->mac, 6);
    out->eth.ether_type = ETH_TYPE_IPV4;
    out->ip.src_addr = n->ip;
    out->ip.dst_addr = dst_ip;
    out->ip.ttl = 64;
    out->ip.protocol = IP_PROTO_UDP;
}

void client_node_init(SimHost *n, int id, const uint8_t mac[6], uint32_t ip,
                      const uint8_t gw_mac[6], uint32_t gw_ip) {
    n->id = id;
    memcpy(n->mac, mac, 6);
    n->ip = ip;
    memcpy(n->gw_mac, gw_mac, 6);
    n->gw_ip = gw_ip;
}

void client_emit_frame(SimHost *n, uint32_t dst_ip, const char *msg,
                       SimFrame *out) {
    memset(out, 0, sizeof(*out));
    frame_fill_l2_l3(n, dst_ip, out, NULL);
    out->app.sm4_on = 0;
    strncpy(out->app.text, msg, APP_MAX_PAYLOAD - 1);
    out->app.text[APP_MAX_PAYLOAD - 1] = '\0';
    udp_header_init(&out->udp, DEMO_UDP_SPORT, DEMO_UDP_DPORT,
                    (uint16_t)strlen(out->app.text));
}

void client_emit_frame_sm4(SimHost *n, uint32_t dst_ip, const char *msg,
                           SimFrame *out) {
    memset(out, 0, sizeof(*out));
    frame_fill_l2_l3(n, dst_ip, out, NULL);
    out->app.sm4_on = 1;
    memcpy(out->app.iv, SM4_IV, 16);
    size_t clen;
    if (sm4_encrypt_message(SM4_PSK, SM4_IV, msg, out->app.sm4_cipher,
                           sizeof(out->app.sm4_cipher), &clen) != 0) {
        fprintf(stderr, "[客户端 节点 %d] SM4 加密失败\n", n->id);
        out->app.sm4_cipher_len = 0;
        udp_header_init(&out->udp, DEMO_UDP_SPORT, DEMO_UDP_DPORT, 0);
        return;
    }
    out->app.sm4_cipher_len = (uint16_t)clen;
    udp_header_init(&out->udp, DEMO_UDP_SPORT, DEMO_UDP_DPORT,
                    out->app.sm4_cipher_len);
}

void client_emit_frame_payload(SimHost *n, uint32_t dst_ip,
                               const uint8_t *data, size_t data_len, int use_sm4,
                               uint16_t sport, uint16_t dport, SimFrame *out) {
    if (data_len > APP_MAX_PAYLOAD) {
        fprintf(stderr, "[客户端 节点 %d] 载荷超过 APP_MAX_PAYLOAD，已截断\n",
                n->id);
        data_len = APP_MAX_PAYLOAD;
    }
    memset(out, 0, sizeof(*out));
    frame_fill_l2_l3(n, dst_ip, out, NULL);
    if (!use_sm4) {
        out->app.sm4_on = 0;
        memcpy(out->app.text, data, data_len);
        if (data_len < APP_MAX_PAYLOAD) {
            out->app.text[data_len] = '\0';
        }
        udp_header_init(&out->udp, sport, dport, (uint16_t)data_len);
        return;
    }
    out->app.sm4_on = 1;
    memcpy(out->app.iv, SM4_IV, 16);
    size_t clen;
    if (sm4_encrypt_buffer(SM4_PSK, SM4_IV, data, data_len, out->app.sm4_cipher,
                         sizeof(out->app.sm4_cipher), &clen) != 0) {
        fprintf(stderr, "[客户端 节点 %d] SM4 加密失败\n", n->id);
        out->app.sm4_cipher_len = 0;
        udp_header_init(&out->udp, sport, dport, 0);
        return;
    }
    out->app.sm4_cipher_len = (uint16_t)clen;
    udp_header_init(&out->udp, sport, dport, out->app.sm4_cipher_len);
}

void client_emit_frame_payload_l2(SimHost *n, const uint8_t dst_eth_mac[6],
                                  uint32_t dst_ip, const uint8_t *data,
                                  size_t data_len, int use_sm4, uint16_t sport,
                                  uint16_t dport, SimFrame *out) {
    if (data_len > APP_MAX_PAYLOAD) {
        fprintf(stderr, "[客户端 节点 %d] 载荷超过 APP_MAX_PAYLOAD，已截断\n",
                n->id);
        data_len = APP_MAX_PAYLOAD;
    }
    memset(out, 0, sizeof(*out));
    frame_fill_l2_l3(n, dst_ip, out, dst_eth_mac);
    if (!use_sm4) {
        out->app.sm4_on = 0;
        memcpy(out->app.text, data, data_len);
        if (data_len < APP_MAX_PAYLOAD) {
            out->app.text[data_len] = '\0';
        }
        udp_header_init(&out->udp, sport, dport, (uint16_t)data_len);
        return;
    }
    out->app.sm4_on = 1;
    memcpy(out->app.iv, SM4_IV, 16);
    size_t clen;
    if (sm4_encrypt_buffer(SM4_PSK, SM4_IV, data, data_len, out->app.sm4_cipher,
                           sizeof(out->app.sm4_cipher), &clen) != 0) {
        fprintf(stderr, "[客户端 节点 %d] SM4 加密失败\n", n->id);
        out->app.sm4_cipher_len = 0;
        udp_header_init(&out->udp, sport, dport, 0);
        return;
    }
    out->app.sm4_cipher_len = (uint16_t)clen;
    udp_header_init(&out->udp, sport, dport, out->app.sm4_cipher_len);
}
