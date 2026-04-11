#ifndef CLIENT_SIM_ARP_H
#define CLIENT_SIM_ARP_H

#include <stdint.h>

#include "sim_config.h"

/*
 * 从节点 A（NODE_A_IP）视角：为发往 dst_ip_l3 的 IPv4 包解析以太网下一跳 MAC。
 * 与 NODE_A 同 ROUTE_PREFIX_A 子网：解析目的主机 / 网关 / EXTRA_ARP*；否则 ARP 网关。
 * 跨子网：模拟 ARP 解析默认网关 NODE_A_GW_IP。
 * 成功返回 0 并写入 eth_dst；无映射返回 -1。
 */
int sim_arp_resolve_from_node_a(const SimNetConfig *cfg, uint32_t dst_ip_l3,
                                uint8_t eth_dst[6]);

#endif
