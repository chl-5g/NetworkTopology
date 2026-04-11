/*************************************************************************
	> File Name: dijkstra.c
	> Author: caihaolun
	> Mail: 1622546034@qq.com
	> Created Time: 五  4/10 14:53:40 2026
 ************************************************************************/
/*
 * dijkstra.c — 在抽象拓扑上计算从 source_node 到各节点的最短路
 *
 * 结果写入全局 routing_table[]：next_hop 为下一跳节点下标；不可达时约定为 -2（见 topo_main 打印分支）。
 */
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "topology.h"

RouteEntry routing_table[MAX_NODES];

void dijkstra(Topology *topo, int source_node) {
    int dist[MAX_NODES];
    bool visited[MAX_NODES] = {false};
    int parent[MAX_NODES];

    for (int i = 0; i < topo->num_nodes; i++) {
        dist[i] = INF;
        parent[i] = -1;
    }
    dist[source_node] = 0;

    for (int count = 0; count < topo->num_nodes; count++) {
        int u = -1;
        int min_dist = INF;
        for (int i = 0; i < topo->num_nodes; i++) {
            if (!visited[i] && dist[i] < min_dist) {
                min_dist = dist[i];
                u = i;
            }
        }
        if (u == -1) {
            break;
        }
        visited[u] = true;

        for (int v = 0; v < topo->num_nodes; v++) {
            if (!visited[v] && topo->adj[u][v] != INF) {
                int new_dist = dist[u] + topo->adj[u][v];
                if (new_dist < dist[v]) {
                    dist[v] = new_dist;
                    parent[v] = u;
                }
            }
        }
    }

    for (int dest = 0; dest < topo->num_nodes; dest++) {
        if (dest == source_node) {
            routing_table[dest].next_hop = -1;
            routing_table[dest].cost = 0;
            continue;
        }
        if (dist[dest] == INF) {
            routing_table[dest].next_hop = -2;
            routing_table[dest].cost = INF;
            continue;
        }

        int curr = dest;
        while (parent[curr] != source_node && parent[curr] != -1) {
            curr = parent[curr];
        }
        if (parent[curr] == -1) {
            routing_table[dest].next_hop = -2;
        } else {
            routing_table[dest].next_hop = curr;
        }
        routing_table[dest].cost = dist[dest];
    }
}
