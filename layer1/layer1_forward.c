#include <stdio.h>

#include "layer1_forward.h"

void layer1_forward(const char *where, SimFrame *f) {
    (void)f;
    printf("[L1 物理层] %s: 比特流传输（仿真直通）\n", where);
}
