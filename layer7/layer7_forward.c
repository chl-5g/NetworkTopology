#include <stdio.h>

#include "layer7_forward.h"

void layer7_forward(const char *where, SimFrame *f) {
    (void)f;
    printf("[L7 应用层] %s: 中继透传（载荷视为不透明字节）\n", where);
}
