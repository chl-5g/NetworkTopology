#include <stdio.h>
#include <string.h>

#include "ipv4.h"
#include "layer_pdu_print.h"
#include "sim_frame.h"
#include "udp.h"

static size_t udp_payload_octets(const SimFrame *f) {
    if (f->udp.length <= UDP_HDR_LEN) {
        return 0;
    }
    size_t n = (size_t)f->udp.length - UDP_HDR_LEN;
    return n;
}

/*
 * 与 udp.length - UDP_HDR_LEN 一致的「线上 UDP 载荷」首地址。
 * SM4 时线载仅为 sm4_cipher[]（IV 在结构体里但不占 UDP 长度）；明文时为 app.text。
 */
static const uint8_t *udp_payload_wire(const SimFrame *f, size_t *out_len) {
    size_t ulen = udp_payload_octets(f);
    if (out_len != NULL) {
        *out_len = ulen;
    }
    if (ulen == 0) {
        return NULL;
    }
    if (f->app.sm4_on) {
        return f->app.sm4_cipher;
    }
    return (const uint8_t *)f->app.text;
}

size_t simframe_wire_ip_datagram(const SimFrame *f, uint8_t *out, size_t outmax) {
    size_t pay = udp_payload_octets(f);
    const uint8_t *pp = udp_payload_wire(f, NULL);
    size_t need = sizeof(f->ip) + (size_t)f->udp.length;
    if (need > outmax) {
        return 0;
    }
    memcpy(out, &f->ip, sizeof(f->ip));
    memcpy(out + sizeof(f->ip), &f->udp, sizeof(f->udp));
    if (pay > 0u && pp != NULL) {
        memcpy(out + sizeof(f->ip) + sizeof(f->udp), pp, pay);
    }
    return need;
}

void layer_pdu_print_client_emit(const uint8_t *plain, size_t plain_len,
                                 const SimFrame *f, int use_sm4,
                                 const char *where) {
    char sip[20], dip[20];
    ipv4_addr_fmt(sip, sizeof(sip), f->ip.src_addr);
    ipv4_addr_fmt(dip, sizeof(dip), f->ip.dst_addr);

    printf("[L7 应用/客户端] %s\n", where);
    sim_octets_hex_only("  收到的载荷（来自 PACKET_FILE，交本层的 SDU）", plain,
                        plain_len);
    printf("  本层添加（相对明文向下封装）:\n");
    if (use_sm4) {
        printf("    - sm4_on=1；IV（16B）仅存于 SimFrame，不计入 UDP 长度；"
               "UDP 线载仅为密文（%u 字节）\n",
               (unsigned)f->app.sm4_cipher_len);
        sim_octets_hex_only("    IV（16 字节）", f->app.iv, 16);
        sim_octets_hex_only("    密文（UDP 载荷，完整）", f->app.sm4_cipher,
                            f->app.sm4_cipher_len);
    } else {
        printf("    - 明文写入 app.text（与 SDU 一致或截断至 APP_MAX_PAYLOAD）\n");
    }
    printf("    - 由 L4 添加 UDP 首部: sport=%u dport=%u length=%u\n",
           (unsigned)f->udp.src_port, (unsigned)f->udp.dst_port,
           (unsigned)f->udp.length);
    printf("    - 由 L3 添加 IPv4 首部: %s -> %s TTL=%u proto=UDP\n", sip, dip,
           (unsigned)f->ip.ttl);
    printf("    - 由 L2 添加以太网首部（dst/src/type），见 SimFrame.eth\n");
}

void layer_pdu_print(int layer, const char *where, const SimFrame *f) {
    if (layer != 2) {
        printf("[层 %d] %s（非交换机 L2 路径，不应调用）\n", layer, where);
        return;
    }
    uint8_t ipwire[2048];
    size_t ipn = simframe_wire_ip_datagram(f, ipwire, sizeof ipwire);
    printf("[L2 数据链路层] %s\n", where);
    printf("  本层可见：以太网首部（学习与转发依据）\n");
    sim_octets_hex_only(NULL, (const uint8_t *)&f->eth, sizeof(f->eth));
    printf("  以太网载荷（L3 及以上对本实体不透明）共 %zu 字节\n",
           ipn > 0u ? ipn : (size_t)0u);
    if (ipn > 0u) {
        sim_octets_hex_only(NULL, ipwire, ipn);
    }
    printf("  说明: 交换机不解析 IP/UDP；仅按目的 MAC 泛洪或转发。\n");
}

void layer_pdu_print_l3_router(const SimFrame *f, const char *where,
                               uint8_t ttl_before_decrement) {
    const uint8_t *p_ip = (const uint8_t *)&f->ip;
    const uint8_t *p_eth = (const uint8_t *)&f->eth;
    char dip[20];
    ipv4_addr_fmt(dip, sizeof(dip), f->ip.dst_addr);

    uint8_t ipwire[2048];
    size_t ipn = simframe_wire_ip_datagram(f, ipwire, sizeof ipwire);
    size_t payn = 0;
    if (ipn > sizeof(f->ip)) {
        payn = ipn - sizeof(f->ip);
    }

    printf("[L3 网络层/路由器] %s\n", where);
    printf("  收到：已剥以太网；本层处理 IPv4 首部，IP 数据区不解析（透传至下一跳）\n");
    printf("  IPv4 首部（教学用简化结构体 %zu 字节；TTL 已 %u → %u）\n",
           sizeof(f->ip), (unsigned)ttl_before_decrement, (unsigned)f->ip.ttl);
    sim_octets_hex_only(NULL, p_ip, sizeof(f->ip));
    printf("  IP 数据区（不透明）共 %zu 字节\n", payn);
    if (payn > 0u) {
        sim_octets_hex_only(NULL, ipwire + sizeof(f->ip), payn);
    }
    printf("  本层处理: LPM；递减 TTL；为下行封装以太网首部（下一跳 MAC）\n");
    sim_octets_hex_only("  下行以太网首部（改写后）", p_eth, sizeof(f->eth));
    printf("  转发目的 IP: %s\n", dip);
}

void layer_pdu_print_l7_host(const SimFrame *f, const char *where) {
    printf("[L7 应用/服务端] %s\n", where);
    printf("  由下层递交 UDP 目的端口 %u 的数据；本层负责解密/交付（见下），此处不重复 "
           "dump 载荷。\n",
           (unsigned)f->udp.dst_port);
}
