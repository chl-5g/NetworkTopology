#ifndef NODE_H
#define NODE_H

#include <stdint.h>

#include "sim_frame.h"

typedef struct {
    int id;
    uint8_t mac[6];
    uint32_t ip;
    uint8_t gw_mac[6];
    uint32_t gw_ip;
} Node;

void node_init(Node *n, int id, const uint8_t mac[6], uint32_t ip,
               const uint8_t gw_mac[6], uint32_t gw_ip);
void node_emit_frame(Node *n, uint32_t dst_ip, const char *msg, SimFrame *out);
void node_emit_frame_sm4(Node *n, uint32_t dst_ip, const char *msg,
                         SimFrame *out);
void node_receive(const Node *n, const SimFrame *in);

#endif
