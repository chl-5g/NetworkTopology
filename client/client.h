#ifndef CLIENT_APP_H
#define CLIENT_APP_H

/*
 * client.h — 客户端（发送主机）对外 API
 *
 * 所有 emit 函数都会清零 SimFrame 后按序填充；载荷超长时截断至 APP_MAX_PAYLOAD。
 */

#include <stdint.h>

#include "sim_frame.h"
#include "sim_host.h"

/* 初始化 SimHost：MAC/IP/网关，供组帧时填源地址与默认下一跳 */
void client_node_init(SimHost *n, int id, const uint8_t mac[6], uint32_t ip,
                      const uint8_t gw_mac[6], uint32_t gw_ip);

/* 演示用：固定 DEMO_UDP 端口，明文消息写入 app.text */
void client_emit_frame(SimHost *n, uint32_t dst_ip, const char *msg,
                       SimFrame *out);
void client_emit_frame_sm4(SimHost *n, uint32_t dst_ip, const char *msg,
                           SimFrame *out);

/* 从二进制缓冲组帧；以太网目的 MAC 使用 n->gw_mac（跨子网场景） */
void client_emit_frame_payload(SimHost *n, uint32_t dst_ip,
                               const uint8_t *data, size_t data_len, int use_sm4,
                               uint16_t sport, uint16_t dport, SimFrame *out);

/*
 * 同网段二层直达：以太网目的地址为 dst_eth_mac（对端主机 MAC），不经网关重写。
 * 跨网段时 dst_eth_mac 应为网关 MAC，而 dst_ip 仍为最终目的主机（NODE_B_IP）。
 */
void client_emit_frame_payload_l2(SimHost *n, const uint8_t dst_eth_mac[6],
                                  uint32_t dst_ip, const uint8_t *data,
                                  size_t data_len, int use_sm4, uint16_t sport,
                                  uint16_t dport, SimFrame *out);

#endif
