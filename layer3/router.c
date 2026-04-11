/*
 * router.c — 双接口路由器转发
 *
 * 初始化时 FIB 仅含两条「直连路由」（各接口所在前缀），转发表命中后：
 *   校验 TTL、递减、根据目的 IP 查静态 ARP 表得到下一跳 MAC、改写以太网 src/dst。
 * 未实现的现实细节：ICMP、分片、校验和、动态 ARP、反向路径检查等。
 */
#include <stdio.h>
#include <string.h>

#include "ipv4.h"
#include "layer_pdu_print.h"
#include "router.h"
#include "sim_frame.h"

static void router_dump_simframe(const char *ctx, const SimFrame *f) {
    char t[160];
    snprintf(t, sizeof t, "[L3 网络层/路由器] SimFrame（%s）", ctx);
    sim_frame_dump(t, f);
}

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

static void fmt_attached_cidr(char *buf, size_t bufsz, uint32_t any_ip,
                              int prefix_len) {
    uint32_t m = ipv4_netmask(prefix_len);
    uint32_t net = any_ip & m;
    ipv4_addr_fmt(buf, bufsz, net);
    size_t n = strlen(buf);
    if (n + 6 < bufsz) {
        snprintf(buf + n, bufsz - n, "/%d", prefix_len);
    }
}

static void fib_add_connected(Router *r, uint32_t any_ip_on_subnet,
                              int prefix_len, int out_if) {
    uint32_t m = ipv4_netmask(prefix_len);
    r->fib[r->fib_n].network = any_ip_on_subnet & m;
    r->fib[r->fib_n].netmask = m;
    r->fib[r->fib_n].next_hop = -1;
    r->fib[r->fib_n].out_interface = out_if;
    r->fib_n++;
}

void router_init_configured(Router *r, const uint8_t mac_if0[6], uint32_t ip_if0,
                            int prefix_len_if0, const uint8_t mac_if1[6],
                            uint32_t ip_if1, int prefix_len_if1) {
    memset(r, 0, sizeof(*r));
    r->num_if = 2;
    memcpy(r->ifs[0].mac, mac_if0, 6);
    r->ifs[0].ip = ip_if0;
    fmt_attached_cidr(r->ifs[0].attached_cidr, sizeof(r->ifs[0].attached_cidr),
                      ip_if0, prefix_len_if0);

    memcpy(r->ifs[1].mac, mac_if1, 6);
    r->ifs[1].ip = ip_if1;
    fmt_attached_cidr(r->ifs[1].attached_cidr, sizeof(r->ifs[1].attached_cidr),
                      ip_if1, prefix_len_if1);

    r->fib_n = 0;
    fib_add_connected(r, ip_if0, prefix_len_if0, 0);
    fib_add_connected(r, ip_if1, prefix_len_if1, 1);
}

/*
 * in_if：帧从哪个路由器接口进入（0=A 侧 LAN，1=B 侧 LAN，与 sim_main 中调用一致）。
 * 返回值：成功转发出接口索引；失败 -1；目的 IP 为本机接口时返回 0（本仿真不上送真实协议栈）。
 */
int router_process(Router *r, SimFrame *f, int in_if) {
    if (f->eth.ether_type != ETH_TYPE_IPV4) {
        printf("[网络层/路由器] 非 IPv4，丢弃\n");
        router_dump_simframe("丢弃：非 IPv4", f);
        return -1;
    }
    if (in_if < 0 || in_if >= r->num_if) {
        router_dump_simframe("丢弃：入口无效", f);
        return -1;
    }
    if (!mac_equal(f->eth.dst_mac, r->ifs[in_if].mac)) {
        printf("[网络层/路由器] 目的 MAC 非本接口，丢弃\n");
        router_dump_simframe("丢弃：目的 MAC 非本接口", f);
        return -1;
    }

    if (router_is_local_ip(r, f->ip.dst_addr)) {
        printf("[网络层/路由器] 目的 IP 为本机接口，上送控制平面（此处仅打印）\n");
        router_dump_simframe("本机接口上送", f);
        return 0;
    }

    if (f->ip.ttl <= 1) {
        printf("[网络层/路由器] TTL 耗尽，丢弃\n");
        router_dump_simframe("丢弃：TTL 耗尽", f);
        return -1;
    }
    uint8_t ttl_before = f->ip.ttl;
    f->ip.ttl--;

    FIB_Entry *hit = lpm_lookup_in(r->fib, r->fib_n, f->ip.dst_addr);
    if (hit == NULL) {
        printf("[网络层/路由器] 无匹配路由，丢弃\n");
        router_dump_simframe("丢弃：无匹配路由", f);
        return -1;
    }

    int out_if = hit->out_interface;
    if (out_if == in_if) {
        printf("[网络层/路由器] 出口与入口相同，丢弃\n");
        router_dump_simframe("丢弃：出口与入口相同", f);
        return -1;
    }

    char dip[20];
    ipv4_addr_fmt(dip, sizeof(dip), f->ip.dst_addr);

    const uint8_t *nh_mac = router_resolve_mac(r, f->ip.dst_addr);
    if (nh_mac == NULL) {
        printf(
            "[网络层/路由器] 转发表命中出接口 %d，但对目的 IP %s 无 ARP 表项；"
            "现实中应在此网段发 ARP Request 解析该 IP 的 MAC（或走已学"
            "习/静态邻居）。本仿真未动态收包学习，可在配置中预置 "
            "EXTRA_ARP1/2_IP+MAC 或在启动时补全 router_add_arp。\n",
            out_if, dip);
        router_dump_simframe("丢弃：无 ARP", f);
        return -1;
    }

    memcpy(f->eth.dst_mac, nh_mac, 6);
    memcpy(f->eth.src_mac, r->ifs[out_if].mac, 6);
    printf("[网络层/路由器] LPM 命中 -> 出接口 %d，转发到 %s（已重写以太网首部）\n",
           out_if, dip);
    layer_pdu_print_l3_router(f, "路由器转发", ttl_before);
    return out_if;
}
