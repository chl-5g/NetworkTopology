/* eth.c — MAC 地址格式化输出 */
#include <stdio.h>

#include "eth.h"

void eth_mac_fmt(char *buf, size_t bufsz, const uint8_t mac[6]) {
    snprintf(buf, bufsz, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1],
             mac[2], mac[3], mac[4], mac[5]);
}
