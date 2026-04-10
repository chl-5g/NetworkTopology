#ifndef TOPOLOGY_H
#define TOPOLOGY_H

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
