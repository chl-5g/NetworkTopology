#include <stdio.h>

#include "client.h"
#include "layer1_forward.h"
#include "layer4_forward.h"
#include "layer5_forward.h"
#include "layer6_forward.h"
#include "layer7_forward.h"
#include "router.h"
#include "server.h"
#include "sim_frame.h"
#include "switch.h"

#define IP_A 0xC0A8010Au
#define IP_B 0x0A000014u

int main(void) {
    uint8_t mac_a[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x0A};
    uint8_t mac_b[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x0B};
    uint8_t mac_gw0[] = {0x00, 0x00, 0x00, 0x00, 0x01, 0x01};

    SimHost node_a, node_b;
    client_node_init(&node_a, 1, mac_a, IP_A, mac_gw0, 0xC0A80101u);
    client_node_init(&node_b, 2, mac_b, IP_B, mac_b, 0);

    Switch sw1, sw2;
    switch_init(&sw1, 2);
    switch_init(&sw2, 2);

    Router router;
    router_init(&router);
    router_add_arp(&router, IP_A, mac_a);
    router_add_arp(&router, IP_B, mac_b);

    printf("架构: 客户端/服务端 + L1–L7 中间转发（L2 交换、L3 路由为真实逻辑，"
           "其余层演示透传）\n");
    printf("拓扑: 节点A --(SW1)-- 路由器 --(SW2)-- 节点B\n");
    printf("A=192.168.1.10  B=10.0.0.20（跨网段）\n");
    printf("载荷: L4 UDP + 客户端 SM4-CBC + PKCS#7（演示固定 PSK/IV）\n\n");

    SimFrame f;
    client_emit_frame_sm4(&node_a, IP_B, "hello from A", &f);

    printf("========== ① 客户端组帧（应用 + L4 封装）==========\n");
    sim_frame_dump("帧内容", &f);

    printf("\n========== ② L1 物理层 ==========\n");
    layer1_forward("A 出站 → SW1", &f);

    printf("\n========== ③ L2 交换机 SW1 ==========\n");
    int p1 = switch_forward_port(&sw1, &f, 0);
    if (p1 != 1) {
        printf("预期: 未知单播泛洪到端口1（接路由器）\n");
    }

    printf("\n========== ④ L4 传输层（中段透传）=========\n");
    layer4_forward("SW1 侧", &f);

    printf("\n========== ⑤ L3 路由器 ==========\n");
    int out_if = router_process(&router, &f, 0);
    if (out_if < 0) {
        return 1;
    }
    sim_frame_dump("路由后帧", &f);

    printf("\n========== ⑥ L4 传输层（中段透传）=========\n");
    layer4_forward("SW2 侧", &f);

    printf("\n========== ⑦ L2 交换机 SW2 ==========\n");
    int p2 = switch_forward_port(&sw2, &f, 0);
    if (p2 != 1) {
        printf("注意: CAM 未命中时泛洪到端口1\n");
    }

    printf("\n========== ⑧ L1 物理层 ==========\n");
    layer1_forward("SW2 → B", &f);

    printf("\n========== ⑨ L5 / L6 / L7（对端接入链路透传）=========\n");
    layer5_forward("B 侧", &f);
    layer6_forward("B 侧", &f);
    layer7_forward("B 侧", &f);

    printf("\n========== ⑩ 服务端交付 ==========\n");
    server_receive(&node_b, &f);

    return 0;
}
