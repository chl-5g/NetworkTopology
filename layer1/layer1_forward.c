#include <stdio.h>

#include "layer1_forward.h"

void layer1_forward(const char *where, SimFrame *f) {
    (void)f;
    printf("[L1 物理层] %s: 透传（仿真仅打印；不解析帧、不重组比特流、不附加前导/FCS）\n",
           where);
}
