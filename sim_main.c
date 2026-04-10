#include <stdio.h>

#include "app_node.h"
#include "router.h"
#include "sim_frame.h"
#include "switch.h"

#define IP_A 0xC0A8010Au
#define IP_B 0x0A000014u

int main(void) {
    uint8_t mac_a[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x0A};
    uint8_t mac_b[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x0B};
    uint8_t mac_gw0[] = {0x00, 0x00, 0x00, 0x00, 0x01, 0x01};

    Node node_a, node_b;
    node_init(&node_a, 1, mac_a, IP_A, mac_gw0, 0xC0A80101u);
    node_init(&node_b, 2, mac_b, IP_B, mac_b, 0);

    Switch sw1, sw2;
    switch_init(&sw1, 2);
    switch_init(&sw2, 2);

    Router router;
    router_init(&router);
    router_add_arp(&router, IP_A, mac_a);
    router_add_arp(&router, IP_B, mac_b);

    printf("分层: L2 交换机 -> L3 路由器 -> L2；载荷经 L4 UDP + L7（SM4）\n");
    printf("拓扑: 节点A --(SW1)-- 路由器 --(SW2)-- 节点B\n");
    printf("A=192.168.1.10  B=10.0.0.20（跨网段）\n");
    printf("L7: 国密 SM4-CBC + PKCS#7（演示固定 PSK/IV）\n\n");

    SimFrame f;
    node_emit_frame_sm4(&node_a, IP_B, "hello from A", &f);

    printf("========== ① L7 组帧发出 ==========\n");
    sim_frame_dump("帧内容", &f);

    printf("\n========== ② L2 交换机 SW1 ==========\n");
    int p1 = switch_forward_port(&sw1, &f, 0);
    if (p1 != 1) {
        printf("预期: 未知单播泛洪到端口1（接路由器）\n");
    }

    printf("\n========== ③ L3 路由器 ==========\n");
    int out_if = router_process(&router, &f, 0);
    if (out_if < 0) {
        return 1;
    }
    sim_frame_dump("路由后帧", &f);

    printf("\n========== ④ L2 交换机 SW2 ==========\n");
    int p2 = switch_forward_port(&sw2, &f, 0);
    if (p2 != 1) {
        printf("注意: CAM 未命中时泛洪到端口1\n");
    }

    printf("\n========== ⑤ L7 节点 B 接收 ==========\n");
    node_receive(&node_b, &f);

    return 0;
}
