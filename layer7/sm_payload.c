#include <string.h>

#include "sm4.h"
#include "sm_payload.h"

static int pkcs7_pad(const uint8_t *in, size_t inlen, uint8_t *out,
                     size_t *outlen, size_t cap) {
    size_t pad = SM4_BLOCK_SIZE - (inlen % SM4_BLOCK_SIZE);
    if (pad == 0) {
        pad = SM4_BLOCK_SIZE;
    }
    if (inlen + pad > cap) {
        return -1;
    }
    memcpy(out, in, inlen);
    for (size_t i = 0; i < pad; i++) {
        out[inlen + i] = (uint8_t)pad;
    }
    *outlen = inlen + pad;
    return 0;
}

static int pkcs7_unpad(const uint8_t *data, size_t len, size_t *plainlen) {
    if (len == 0 || (len % SM4_BLOCK_SIZE) != 0) {
        return -1;
    }
    uint8_t pad = data[len - 1];
    if (pad == 0 || pad > SM4_BLOCK_SIZE) {
        return -1;
    }
    for (size_t i = 0; i < (size_t)pad; i++) {
        if (data[len - 1 - i] != pad) {
            return -1;
        }
    }
    *plainlen = len - pad;
    return 0;
}

int sm4_encrypt_buffer(const uint8_t key[16], const uint8_t iv[16],
                       const uint8_t *plaintext, size_t plaintext_len,
                       uint8_t *cipher_out, size_t cipher_cap,
                       size_t *cipher_len) {
    if (plaintext_len > 200) {
        return -1;
    }
    uint8_t buf[256];
    size_t blen;
    if (pkcs7_pad(plaintext, plaintext_len, buf, &blen, sizeof(buf)) != 0) {
        return -1;
    }
    size_t nblk = blen / SM4_BLOCK_SIZE;
    if (cipher_cap < blen) {
        return -1;
    }
    uint8_t iv_work[SM4_BLOCK_SIZE];
    memcpy(iv_work, iv, SM4_BLOCK_SIZE);
    SM4_KEY enc;
    sm4_set_encrypt_key(&enc, key);
    sm4_cbc_encrypt_blocks(&enc, iv_work, buf, nblk, cipher_out);
    *cipher_len = blen;
    return 0;
}

int sm4_encrypt_message(const uint8_t key[16], const uint8_t iv[16],
                        const char *plaintext, uint8_t *cipher_out,
                        size_t cipher_cap, size_t *cipher_len) {
    return sm4_encrypt_buffer(key, iv, (const uint8_t *)plaintext,
                              strlen(plaintext), cipher_out, cipher_cap,
                              cipher_len);
}

int sm4_decrypt_buffer(const uint8_t key[16], const uint8_t iv[16],
                       const uint8_t *cipher, size_t cipher_len,
                       uint8_t *plain_out, size_t plain_cap,
                       size_t *plain_len) {
    if (cipher_len == 0 || (cipher_len % SM4_BLOCK_SIZE) != 0) {
        return -1;
    }
    uint8_t buf[256];
    if (cipher_len > sizeof(buf)) {
        return -1;
    }
    uint8_t iv_work[SM4_BLOCK_SIZE];
    memcpy(iv_work, iv, SM4_BLOCK_SIZE);
    SM4_KEY dec;
    sm4_set_decrypt_key(&dec, key);
    sm4_cbc_decrypt_blocks(&dec, iv_work, cipher, cipher_len / SM4_BLOCK_SIZE,
                           buf);
    size_t plen;
    if (pkcs7_unpad(buf, cipher_len, &plen) != 0) {
        return -1;
    }
    if (plen > plain_cap) {
        return -1;
    }
    memcpy(plain_out, buf, plen);
    *plain_len = plen;
    return 0;
}

int sm4_decrypt_message(const uint8_t key[16], const uint8_t iv[16],
                        const uint8_t *cipher, size_t cipher_len,
                        char *plain_out, size_t plain_cap) {
    size_t plen;
    if (sm4_decrypt_buffer(key, iv, cipher, cipher_len, (uint8_t *)plain_out,
                           plain_cap, &plen) != 0) {
        return -1;
    }
    if (plen + 1 > plain_cap) {
        return -1;
    }
    plain_out[plen] = '\0';
    return 0;
}
