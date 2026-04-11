#ifndef TOPOLOGY_H
#define TOPOLOGY_H

/*
 * topology.h — 拓扑与路由表相关公共类型（Dijkstra 建 FIB 的 topo_main 与本仿真路由器共用）
 *
 * FIB_Entry：network/netmask 描述前缀；out_interface 为出接口编号；next_hop 在
 * 本仓库的 router.c 直连场景中为 -1。lpm_lookup_in 对给定表做最长前缀匹配。
 */

#include <limits.h>
#include <stdint.h>

#define MAX_NODES 20
#define MAX_FIB_SIZE 128
#define INF INT_MAX

typedef struct {
    int num_nodes;
    int adj[MAX_NODES][MAX_NODES];
    char networks[MAX_NODES][20];
} Topology;

typedef struct {
    int next_hop;
    int cost;
} RouteEntry;

typedef struct {
    uint32_t network;
    uint32_t netmask;
    int next_hop;
    int out_interface;
} FIB_Entry;

extern RouteEntry routing_table[MAX_NODES];
extern FIB_Entry fib_table[MAX_FIB_SIZE];
extern int fib_size;

void dijkstra(Topology *topo, int source_node);
void build_fib_from_dijkstra(Topology *topo, int source_node);
void parse_cidr(char *cidr, uint32_t *network, uint32_t *netmask);
FIB_Entry *lpm_lookup(uint32_t dest_ip);
FIB_Entry *lpm_lookup_in(FIB_Entry *table, int table_size, uint32_t dest_ip);

#endif
