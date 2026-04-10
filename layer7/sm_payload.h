#ifndef SM_PAYLOAD_H
#define SM_PAYLOAD_H

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
