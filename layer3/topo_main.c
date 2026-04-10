#include <stdio.h>
#include <string.h>

#include "topology.h"

int main(void) {
    Topology topo;
    topo.num_nodes = 4;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            topo.adj[i][j] = (i == j) ? 0 : INF;
        }
    }

    topo.adj[0][1] = topo.adj[1][0] = 1;
    topo.adj[0][2] = topo.adj[2][0] = 3;
    topo.adj[1][3] = topo.adj[3][1] = 1;
    topo.adj[2][3] = topo.adj[3][2] = 4;

    strcpy(topo.networks[0], "192.168.1.0/24");
    strcpy(topo.networks[1], "10.0.0.0/8");
    strcpy(topo.networks[2], "172.16.0.0/16");
    strcpy(topo.networks[3], "8.8.8.0/24");

    int myself = 0;

    printf("=== 运行迪杰斯特拉算法 (节点%d) ===\n", myself);
    dijkstra(&topo, myself);

    for (int i = 0; i < topo.num_nodes; i++) {
        printf("目标网段 %s: ", topo.networks[i]);
        if (i == myself) {
            printf("直连\n");
        } else if (routing_table[i].next_hop == -2) {
            printf("不可达\n");
        } else {
            printf("下一跳节点 %d, 总开销 %d\n", routing_table[i].next_hop,
                   routing_table[i].cost);
        }
    }

    build_fib_from_dijkstra(&topo, myself);

    printf("\n=== 转发平面查找 (LPM) ===\n");
    uint32_t test_ips[] = {
        0xC0A8010A,
        0x0A000001,
        0x08080808,
        0xAC100001,
        0x08080404,
    };

    for (int i = 0; i < 5; i++) {
        FIB_Entry *match = lpm_lookup(test_ips[i]);
        printf("目标IP: 0x%08X -> ", test_ips[i]);
        if (match == NULL) {
            printf("无匹配路由，丢弃\n");
        } else if (match->next_hop == -1) {
            printf("直连网段，出接口 %d\n", match->out_interface);
        } else {
            printf("下一跳节点 %d，出接口 %d\n", match->next_hop,
                   match->out_interface);
        }
    }

    return 0;
}
