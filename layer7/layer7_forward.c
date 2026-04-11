/*
 * layer7_forward.c — 接收端主机上 L5–L7 桩
 *
 * 真实实现会涉及端口多路复用、TLS、应用协议状态机等；本仿真把解密与交付集中在
 * server_receive()，此处只打印「透传」说明。
 */
#include <stdio.h>

#include "layer7_forward.h"

void layer7_forward(const char *where, SimFrame *f) {
    (void)f;
    printf("[端系统] %s: L5/L6/L7 透传（仿真仅打印；不拆 PDU、不组帧、无状态）\n",
           where);
}
