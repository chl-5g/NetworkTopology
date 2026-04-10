#ifndef CLIENT_APP_H
#define CLIENT_APP_H

#include <stdint.h>

#include "sim_frame.h"
#include "sim_host.h"

void client_node_init(SimHost *n, int id, const uint8_t mac[6], uint32_t ip,
                      const uint8_t gw_mac[6], uint32_t gw_ip);
void client_emit_frame(SimHost *n, uint32_t dst_ip, const char *msg,
                       SimFrame *out);
void client_emit_frame_sm4(SimHost *n, uint32_t dst_ip, const char *msg,
                           SimFrame *out);

#endif
