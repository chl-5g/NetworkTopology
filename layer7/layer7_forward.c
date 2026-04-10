#include <stdio.h>

#include "layer7_forward.h"

void layer7_forward(const char *where, SimFrame *f) {
    (void)f;
    printf("[端系统] %s: L5/L6/L7 透传（仿真仅打印；不拆 PDU、不组帧、无状态）\n",
           where);
}
