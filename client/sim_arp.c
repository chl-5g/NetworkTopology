#include <stdio.h>
#include <string.h>

#include "ipv4.h"
#include "sim_arp.h"

int sim_arp_resolve_from_node_a(const SimNetConfig *cfg, uint32_t dst_ip_l3,
                                uint8_t eth_dst[6]) {
    char dsts[20], gws[20], bs[20], as[20];
    ipv4_addr_fmt(dsts, sizeof(dsts), dst_ip_l3);
    ipv4_addr_fmt(gws, sizeof(gws), cfg->node_a_gw_ip);
    ipv4_addr_fmt(bs, sizeof(bs), cfg->node_b_ip);
    ipv4_addr_fmt(as, sizeof(as), cfg->node_a_ip);

    /* 与 A 侧接口掩码一致：用 ROUTE_PREFIX_A（而非 LAN_PREFIX_LEN）判定是否直连子网 */
    if (ipv4_same_subnet(cfg->node_a_ip, dst_ip_l3, cfg->route_prefix_a)) {
        if (dst_ip_l3 == cfg->node_b_ip) {
            printf(
                "[ARP 仿真] A 在子网内广播: Who has %s? Tell %s\n"
                "           B 单播应答: %s is-at "
                "%02x:%02x:%02x:%02x:%02x:%02x\n",
                dsts, as, bs, (unsigned)cfg->node_b_mac[0],
                (unsigned)cfg->node_b_mac[1], (unsigned)cfg->node_b_mac[2],
                (unsigned)cfg->node_b_mac[3], (unsigned)cfg->node_b_mac[4],
                (unsigned)cfg->node_b_mac[5]);
            memcpy(eth_dst, cfg->node_b_mac, 6);
            return 0;
        }
        if (dst_ip_l3 == cfg->node_a_gw_ip) {
            printf(
                "[ARP 仿真] A 在子网内广播: Who has %s? Tell %s\n"
                "           网关单播应答: %s is-at "
                "%02x:%02x:%02x:%02x:%02x:%02x\n",
                gws, as, gws, (unsigned)cfg->node_a_gw_mac[0],
                (unsigned)cfg->node_a_gw_mac[1],
                (unsigned)cfg->node_a_gw_mac[2],
                (unsigned)cfg->node_a_gw_mac[3],
                (unsigned)cfg->node_a_gw_mac[4],
                (unsigned)cfg->node_a_gw_mac[5]);
            memcpy(eth_dst, cfg->node_a_gw_mac, 6);
            return 0;
        }
        if (cfg->extra_arp1_ip != 0u && dst_ip_l3 == cfg->extra_arp1_ip) {
            memcpy(eth_dst, cfg->extra_arp1_mac, 6);
            printf(
                "[ARP 仿真] A 在子网内广播: Who has %s? Tell %s\n"
                "           静态表应答: %s is-at "
                "%02x:%02x:%02x:%02x:%02x:%02x\n",
                dsts, as, dsts, (unsigned)eth_dst[0], (unsigned)eth_dst[1],
                (unsigned)eth_dst[2], (unsigned)eth_dst[3], (unsigned)eth_dst[4],
                (unsigned)eth_dst[5]);
            return 0;
        }
        if (cfg->extra_arp2_ip != 0u && dst_ip_l3 == cfg->extra_arp2_ip) {
            memcpy(eth_dst, cfg->extra_arp2_mac, 6);
            printf(
                "[ARP 仿真] A 在子网内广播: Who has %s? Tell %s\n"
                "           静态表应答: %s is-at "
                "%02x:%02x:%02x:%02x:%02x:%02x\n",
                dsts, as, dsts, (unsigned)eth_dst[0], (unsigned)eth_dst[1],
                (unsigned)eth_dst[2], (unsigned)eth_dst[3], (unsigned)eth_dst[4],
                (unsigned)eth_dst[5]);
            return 0;
        }
        fprintf(stderr,
                "[ARP 仿真] 同网段但无 %s 的邻居映射（预置 NODE_B / 网关 / "
                "EXTRA_ARP*）\n",
                dsts);
        return -1;
    }

    printf(
        "[ARP 仿真] A 在子网内广播: Who has %s? Tell %s\n"
        "           网关单播应答: %s is-at "
        "%02x:%02x:%02x:%02x:%02x:%02x\n",
        gws, as, gws, (unsigned)cfg->node_a_gw_mac[0],
        (unsigned)cfg->node_a_gw_mac[1], (unsigned)cfg->node_a_gw_mac[2],
        (unsigned)cfg->node_a_gw_mac[3], (unsigned)cfg->node_a_gw_mac[4],
        (unsigned)cfg->node_a_gw_mac[5]);
    memcpy(eth_dst, cfg->node_a_gw_mac, 6);
    return 0;
}
