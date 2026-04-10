/*************************************************************************
	> File Name: routing_table_conversion.c
	> Author: caihaolun
	> Mail: 1622546034@qq.com
	> Created Time: 五  4/10 14:56:52 2026
 ************************************************************************/
#include <stdio.h>
#include <string.h>

#include "topology.h"

FIB_Entry fib_table[MAX_FIB_SIZE];
int fib_size = 0;

void parse_cidr(char *cidr, uint32_t *network, uint32_t *netmask) {
    char ip_str[20];
    int prefix_len;
    sscanf(cidr, "%19[^/]/%d", ip_str, &prefix_len);

    unsigned int a, b, c, d;
    sscanf(ip_str, "%u.%u.%u.%u", &a, &b, &c, &d);
    *network = ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)c << 8) | d;

    if (prefix_len <= 0) {
        *netmask = 0;
    } else if (prefix_len >= 32) {
        *netmask = 0xFFFFFFFFu;
    } else {
        *netmask = ~(((1u << (32 - prefix_len)) - 1u));
    }
    *network &= *netmask;
}

void build_fib_from_dijkstra(Topology *topo, int source_node) {
    fib_size = 0;
    for (int dest = 0; dest < topo->num_nodes; dest++) {
        if (routing_table[dest].cost == INF) {
            continue;
        }

        FIB_Entry entry;
        parse_cidr(topo->networks[dest], &entry.network, &entry.netmask);

        if (dest == source_node) {
            entry.next_hop = -1;
            entry.out_interface = dest;
        } else {
            int nh = routing_table[dest].next_hop;
            entry.next_hop = nh;
            entry.out_interface = nh;
        }
        fib_table[fib_size++] = entry;
    }
}

FIB_Entry *lpm_lookup_in(FIB_Entry *table, int table_size, uint32_t dest_ip) {
    FIB_Entry *best_match = NULL;
    int longest_prefix_len = -1;

    for (int i = 0; i < table_size; i++) {
        FIB_Entry *entry = &table[i];
        if ((dest_ip & entry->netmask) == entry->network) {
            int prefix_len = __builtin_popcount(entry->netmask);
            if (prefix_len > longest_prefix_len) {
                longest_prefix_len = prefix_len;
                best_match = entry;
            }
        }
    }
    return best_match;
}

FIB_Entry *lpm_lookup(uint32_t dest_ip) {
    return lpm_lookup_in(fib_table, fib_size, dest_ip);
}
