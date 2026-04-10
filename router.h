#ifndef ROUTER_H
#define ROUTER_H

#include <stdint.h>

#include "sim_frame.h"
#include "topology.h"

#define RT_MAX_IF 4
#define RT_MAX_FIB 32
#define RT_MAX_ARP 16

typedef struct {
    uint8_t mac[6];
    uint32_t ip;
    char attached_cidr[24];
} RtIface;

typedef struct {
    RtIface ifs[RT_MAX_IF];
    int num_if;
    FIB_Entry fib[RT_MAX_FIB];
    int fib_n;
    uint32_t arp_ip[RT_MAX_ARP];
    uint8_t arp_mac[RT_MAX_ARP][6];
    int arp_n;
} Router;

void router_init(Router *r);
void router_add_arp(Router *r, uint32_t ip, const uint8_t mac[6]);
int router_process(Router *r, SimFrame *f, int in_if);

#endif
