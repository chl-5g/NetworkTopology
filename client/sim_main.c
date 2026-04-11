#include <stdio.h>

#include "client.h"
#include "ipv4.h"
#include "layer1_forward.h"
#include "layer7_forward.h"
#include "layer_pdu_print.h"
#include "router.h"
#include "server.h"
#include "sim_config.h"
#include "sim_frame.h"
#include "switch.h"

int main(int argc, char **argv) {
    const char *cfg_path =
        (argc > 1) ? argv[1] : "config/network.conf";
    SimNetConfig cfg;
    if (sim_net_config_load(cfg_path, &cfg) != 0) {
        fprintf(stderr, "无法读取配置文件: %s\n", cfg_path);
        return 1;
    }

    uint8_t payload[APP_MAX_PAYLOAD];
    size_t plen;
    if (sim_packet_load(cfg.packet_file, payload, sizeof payload, &plen) != 0) {
        fprintf(stderr, "无法读取数据包文件: %s\n", cfg.packet_file);
        return 1;
    }

    printf("======== PACKET_FILE 读入的原始数据包（%zu 字节）========\n", plen);
    sim_octets_print(NULL, payload, plen);
    if (cfg.use_sm4) {
        printf(
            "（启用 USE_SM4=1 时，以上为加密前明文；下方 SimFrame 中 L7 为密文。）\n");
    }
    printf("\n");

    int same_l2 = ipv4_same_subnet(cfg.node_a_ip, cfg.node_b_ip,
                                   cfg.lan_prefix_len);
    if (!same_l2) {
        if (!ipv4_in_prefix(cfg.node_a_ip, 0xC0A80100u, 24) ||
            !ipv4_in_prefix(cfg.node_b_ip, 0x0A000000u, 8)) {
            fprintf(stderr,
                    "跨网段模式：须 A 在 192.168.1.0/24、B 在 10.0.0.0/8（与内置"
                    "路由器 FIB 一致）。\n");
            return 1;
        }
    }

    SimHost node_client, node_server;
    client_node_init(&node_client, cfg.node_a_id, cfg.node_a_mac, cfg.node_a_ip,
                      cfg.node_a_gw_mac, cfg.node_a_gw_ip);
    client_node_init(&node_server, cfg.node_b_id, cfg.node_b_mac, cfg.node_b_ip,
                      cfg.node_b_mac, 0);

    Switch sw1;
    switch_init(&sw1, cfg.switch_port_count);

    Switch sw2;
    Router router;
    if (!same_l2) {
        switch_init(&sw2, cfg.switch_port_count);
        router_init(&router);
        router_add_arp(&router, cfg.node_a_ip, cfg.node_a_mac);
        router_add_arp(&router, cfg.node_b_ip, cfg.node_b_mac);
    }

    char sa[20], sb[20];
    ipv4_addr_fmt(sa, sizeof(sa), cfg.node_a_ip);
    ipv4_addr_fmt(sb, sizeof(sb), cfg.node_b_ip);

    printf("配置: %s | 数据包: %s (%zu 字节)\n", cfg_path, cfg.packet_file, plen);
    printf("架构: 交换机 L2、路由器 L3；L1 与 B 侧 L5–L7 透传仅打印（无组帧等操作）；"
           "发送端组帧、接收端应用见打印\n");
    if (same_l2) {
        printf("拓扑: 节点A --(SW1)-- 节点B（同网段 /%d，不经路由器）\n",
               cfg.lan_prefix_len);
    } else {
        printf("拓扑: 节点A --(SW1)-- 路由器 --(SW2)-- 节点B\n");
    }
    printf("A=%s  B=%s | 同网段判定: LAN_PREFIX_LEN=%d → %s\n", sa, sb,
           cfg.lan_prefix_len, same_l2 ? "是（二层直达）" : "否（经路由）");
    printf("载荷: L4 UDP %u→%u，%s\n\n", (unsigned)cfg.udp_sport,
           (unsigned)cfg.udp_dport, cfg.use_sm4 ? "SM4-CBC" : "明文");

    SimFrame f;
    if (same_l2) {
        client_emit_frame_payload_l2(&node_client, cfg.node_b_mac, cfg.node_b_ip,
                                    payload, plen, cfg.use_sm4, cfg.udp_sport,
                                    cfg.udp_dport, &f);
    } else {
        client_emit_frame_payload(&node_client, cfg.node_b_ip, payload, plen,
                                  cfg.use_sm4, cfg.udp_sport, cfg.udp_dport, &f);
    }

    printf("========== ① 客户端组帧（应用 + L4 封装）==========\n");
    layer_pdu_print_client_emit(payload, plen, &f, cfg.use_sm4, "组帧完成");
    sim_frame_dump("[客户端组帧后] SimFrame 摘要与已用区 hexdump", &f);

    printf("\n========== ② L1 物理层 ==========\n");
    layer1_forward("A 出站 → SW1", &f);

    printf("\n========== ③ L2 交换机 SW1 ==========\n");
    int p1 = switch_forward_port(&sw1, &f, 0, "SW1");
    if (same_l2) {
        if (p1 != 1) {
            printf("说明: 未知单播泛洪到端口1（接节点B）\n");
        }
    } else {
        if (p1 != 1) {
            printf("预期: 未知单播泛洪到端口1（接路由器）\n");
        }
    }

    if (!same_l2) {
        printf("\n========== ④ L3 路由器 ==========\n");
        int out_if = router_process(&router, &f, 0);
        if (out_if < 0) {
            return 1;
        }

        printf("\n========== ⑤ L2 交换机 SW2 ==========\n");
        int p2 = switch_forward_port(&sw2, &f, 0, "SW2");
        if (p2 != 1) {
            printf("注意: CAM 未命中时泛洪到端口1\n");
        }

        printf("\n========== ⑥ L1 物理层 ==========\n");
        layer1_forward("SW2 → B", &f);
    } else {
        printf("\n========== ④ L3 路由器 ==========\n");
        printf("同网段：省略路由器与 SW2。\n");

        printf("\n========== ⑤ L1 物理层 ==========\n");
        layer1_forward("SW1 → B", &f);
    }

    printf("\n========== ⑦ 端系统上层（L5–L7）透传 ==========\n");
    layer7_forward("B 侧", &f);

    printf("\n========== ⑧ 服务端交付 ==========\n");
    server_receive(&node_server, &f);

    return 0;
}
