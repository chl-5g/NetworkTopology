#include <stdio.h>

#include "layer5_forward.h"

void layer5_forward(const char *where, SimFrame *f) {
    (void)f;
    printf("[L5 会话层] %s: 透传（本仿真不拆会话）\n", where);
}
