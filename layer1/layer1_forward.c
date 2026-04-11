/*
 * layer1_forward.c — 物理层桩
 *
 * 真实链路会处理编码、帧定界、前导码与 FCS 等；此处仅标注「比特流透传」语义并打印。
 */
#include <stdio.h>

#include "layer1_forward.h"

void layer1_forward(const char *where, SimFrame *f) {
    (void)f;
    printf("[L1 物理层] %s: 透传（仿真仅打印；不解析帧、不重组比特流、不附加前导/FCS）\n",
           where);
}
