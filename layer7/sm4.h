/*
 * SM4 block cipher API — 实现见 sm4.c（源自 GmSSL，Apache-2.0）。
 */
#ifndef SM4_H
#define SM4_H

#include <stddef.h>
#include <stdint.h>

#define SM4_KEY_SIZE 16
#define SM4_BLOCK_SIZE 16
#define SM4_NUM_ROUNDS 32

typedef struct {
    uint32_t rk[SM4_NUM_ROUNDS];
} SM4_KEY;

void sm4_set_encrypt_key(SM4_KEY *key, const uint8_t raw_key[SM4_KEY_SIZE]);
void sm4_set_decrypt_key(SM4_KEY *key, const uint8_t raw_key[SM4_KEY_SIZE]);
void sm4_encrypt(const SM4_KEY *key, const uint8_t in[SM4_BLOCK_SIZE],
                 uint8_t out[SM4_BLOCK_SIZE]);

void sm4_cbc_encrypt_blocks(const SM4_KEY *key, uint8_t iv[SM4_BLOCK_SIZE],
                            const uint8_t *in, size_t nblocks, uint8_t *out);
void sm4_cbc_decrypt_blocks(const SM4_KEY *key, uint8_t iv[SM4_BLOCK_SIZE],
                            const uint8_t *in, size_t nblocks, uint8_t *out);

#endif
