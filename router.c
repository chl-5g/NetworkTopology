#include <stdio.h>
#include <string.h>

#include "router.h"

static int mac_equal(const uint8_t a[6], const uint8_t b[6]) {
    return memcmp(a, b, 6) == 0;
}

static int router_is_local_ip(const Router *r, uint32_t ip) {
    for (int i = 0; i < r->num_if; i++) {
        if (r->ifs[i].ip == ip) {
            return 1;
        }
    }
    return 0;
}

static const uint8_t *router_resolve_mac(const Router *r, uint32_t ip) {
    for (int i = 0; i < r->arp_n; i++) {
        if (r->arp_ip[i] == ip) {
            return r->arp_mac[i];
        }
    }
    return NULL;
}

void router_add_arp(Router *r, uint32_t ip, const uint8_t mac[6]) {
    for (int i = 0; i < r->arp_n; i++) {
        if (r->arp_ip[i] == ip) {
            memcpy(r->arp_mac[i], mac, 6);
            return;
        }
    }
    if (r->arp_n >= RT_MAX_ARP) {
        return;
    }
    r->arp_ip[r->arp_n] = ip;
    memcpy(r->arp_mac[r->arp_n], mac, 6);
    r->arp_n++;
}

void router_init(Router *r) {
    memset(r, 0, sizeof(*r));
    r->num_if = 2;

    uint8_t mac0[] = {0x00, 0x00, 0x00, 0x00, 0x01, 0x01};
    uint8_t mac1[] = {0x00, 0x00, 0x00, 0x00, 0x02, 0x02};
    memcpy(r->ifs[0].mac, mac0, 6);
    r->ifs[0].ip = 0xC0A80101u;
    strcpy(r->ifs[0].attached_cidr, "192.168.1.0/24");

    memcpy(r->ifs[1].mac, mac1, 6);
    r->ifs[1].ip = 0x0A000001u;
    strcpy(r->ifs[1].attached_cidr, "10.0.0.0/8");

    r->fib_n = 0;
    parse_cidr("192.168.1.0/24", &r->fib[r->fib_n].network,
               &r->fib[r->fib_n].netmask);
    r->fib[r->fib_n].next_hop = -1;
    r->fib[r->fib_n].out_interface = 0;
    r->fib_n++;

    parse_cidr("10.0.0.0/8", &r->fib[r->fib_n].network,
               &r->fib[r->fib_n].netmask);
    r->fib[r->fib_n].next_hop = -1;
    r->fib[r->fib_n].out_interface = 1;
    r->fib_n++;
}

int router_process(Router *r, SimFrame *f, int in_if) {
    if (f->ether_type != SIM_ETH_IPV4) {
        printf("[路由器] 非 IPv4，丢弃\n");
        return -1;
    }
    if (in_if < 0 || in_if >= r->num_if) {
        return -1;
    }
    if (!mac_equal(f->dst_mac, r->ifs[in_if].mac)) {
        printf("[路由器] 目的 MAC 非本接口，丢弃\n");
        return -1;
    }

    if (router_is_local_ip(r, f->dst_ip)) {
        printf("[路由器] 目的 IP 为本机接口，上送控制平面（此处仅打印）\n");
        return 0;
    }

    if (f->ttl <= 1) {
        printf("[路由器] TTL 耗尽，丢弃\n");
        return -1;
    }
    f->ttl--;

    FIB_Entry *hit = lpm_lookup_in(r->fib, r->fib_n, f->dst_ip);
    if (hit == NULL) {
        printf("[路由器] 无匹配路由，丢弃\n");
        return -1;
    }

    int out_if = hit->out_interface;
    if (out_if == in_if) {
        printf("[路由器] 出口与入口相同，丢弃\n");
        return -1;
    }

    const uint8_t *nh_mac = router_resolve_mac(r, f->dst_ip);
    if (nh_mac == NULL) {
        printf("[路由器] 无 ARP，无法解析下一跳二层地址\n");
        return -1;
    }

    memcpy(f->dst_mac, nh_mac, 6);
    memcpy(f->src_mac, r->ifs[out_if].mac, 6);

    char dip[20];
    sim_ip_fmt(dip, sizeof(dip), f->dst_ip);
    printf("[路由器] LPM 命中 -> 出接口 %d，转发到 %s（已重写以太网首部）\n",
           out_if, dip);
    return out_if;
}
