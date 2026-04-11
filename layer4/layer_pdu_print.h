#ifndef LAYER4_LAYER_PDU_PRINT_H
#define LAYER4_LAYER_PDU_PRINT_H

/*
 * layer_pdu_print.h — 各层 PDU 教学打印（与真实内核/硬件解析范围对应）
 */

#include <stddef.h>
#include <stdint.h>

#include "sim_frame.h"

/* 客户端 L7：明文 SDU + 本层向下做的封装（SM4/明文 + 说明 L4/L3/L2 首部） */
void layer_pdu_print_client_emit(const uint8_t *plain, size_t plain_len,
                                 const SimFrame *f, int use_sm4,
                                 const char *where);

/* 将 IP|UDP|载荷 拷贝为线序连续字节（供 L2/L3 打印不透明区；非「透传层」逻辑） */
size_t simframe_wire_ip_datagram(const SimFrame *f, uint8_t *out, size_t outmax);

/* 仅交换机路径：L2 只解析以太网首部，载荷为不透明 IP 数据报 */
void layer_pdu_print(int layer, const char *where, const SimFrame *f);

void layer_pdu_print_l3_router(const SimFrame *f, const char *where,
                               uint8_t ttl_before_decrement);

/* 目的端主机 L7：收到 UDP 载荷，无再封装 */
void layer_pdu_print_l7_host(const SimFrame *f, const char *where);

#endif
