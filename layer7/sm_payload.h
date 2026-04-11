#ifndef SM_PAYLOAD_H
#define SM_PAYLOAD_H

/*
 * sm_payload.h — SM4-CBC 封装在「消息字符串」与「任意字节缓冲」两套 API
 *
 * 与 sim_frame 配合：加密结果写入 app.sm4_cipher，IV 写入 app.iv；解密在 server 侧完成。
 */

#include <stddef.h>
#include <stdint.h>

int sm4_encrypt_message(const uint8_t key[16], const uint8_t iv[16],
                        const char *plaintext, uint8_t *cipher_out,
                        size_t cipher_cap, size_t *cipher_len);

int sm4_encrypt_buffer(const uint8_t key[16], const uint8_t iv[16],
                       const uint8_t *plaintext, size_t plaintext_len,
                       uint8_t *cipher_out, size_t cipher_cap,
                       size_t *cipher_len);

int sm4_decrypt_message(const uint8_t key[16], const uint8_t iv[16],
                        const uint8_t *cipher, size_t cipher_len,
                        char *plain_out, size_t plain_cap);

int sm4_decrypt_buffer(const uint8_t key[16], const uint8_t iv[16],
                       const uint8_t *cipher, size_t cipher_len,
                       uint8_t *plain_out, size_t plain_cap,
                       size_t *plain_len);

#endif
