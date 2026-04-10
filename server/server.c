#include <stdio.h>

#include "ipv4.h"
#include "server.h"
#include "sm_payload.h"
#include "sm_session.h"

void server_receive(const SimHost *n, const SimFrame *in) {
    char dip[20];
    ipv4_addr_fmt(dip, sizeof(dip), in->ip.dst_addr);

    if (in->app.sm4_on) {
        char plain[APP_MAX_PAYLOAD];
        if (sm4_decrypt_message(SM4_PSK, in->app.iv, in->app.sm4_cipher,
                                in->app.sm4_cipher_len, plain,
                                sizeof(plain)) != 0) {
            printf("[服务端 节点 %d] SM4 解密失败（密文可能被篡改）\n", n->id);
            return;
        }
        printf("[服务端 节点 %d] 收到: 目的 IP %s，UDP %u，SM4 解密后 \"%s\"\n",
               n->id, dip, (unsigned)in->udp.dst_port, plain);
    } else {
        printf("[服务端 节点 %d] 收到: 目的 IP %s，UDP %u，明文 \"%s\"\n", n->id,
               dip, (unsigned)in->udp.dst_port, in->app.text);
    }
}
