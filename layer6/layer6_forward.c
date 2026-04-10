#include <stdio.h>

#include "layer6_forward.h"

void layer6_forward(const char *where, SimFrame *f) {
    (void)f;
    printf("[L6 表示层] %s: 透传（不在中段做加解密）\n", where);
}
