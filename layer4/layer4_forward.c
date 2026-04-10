#include <stdio.h>

#include "layer4_forward.h"

void layer4_forward(const char *where, SimFrame *f) {
    (void)f;
    printf("[L4 传输层] %s: 中段透传（UDP 已封装，不作端点处理）\n", where);
}
