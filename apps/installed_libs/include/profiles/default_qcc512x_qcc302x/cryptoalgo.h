/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    cryptoalgo.h    

DESCRIPTION
    A cryptographic API library on P1 for applications.Note: This is to be changed later for P0 support.

*/

#ifndef _CRYPTOALGO_H__
#define _CRYPTOALGO_H__

#define AES_BLOCK_SIZE 16
#define SHA256_DIGEST_SIZE 32

#define SECP256R1_PUBLIC_KEY_SIZE       64
#define SECP256R1_PRIVATE_KEY_SIZE      32
#define SECP256R1_SHARED_SECRET_SIZE    32

/*Function to do AES128 encrypt*/
extern void aes128_encrypt(uint8 in[AES_BLOCK_SIZE], uint8 out[AES_BLOCK_SIZE], uint8 key[AES_BLOCK_SIZE]);

/*Function to do AES128 decrypt*/
extern void aes128_decrypt(uint8 in[AES_BLOCK_SIZE], uint8 out[AES_BLOCK_SIZE], uint8 key[AES_BLOCK_SIZE]);

/*Function to do sha256 hash*/
extern void sha256(const uint8 *data, uint16 len, uint8 digest[SHA256_DIGEST_SIZE]);

/*Function to do generate a shared secret key with ECDH*/
extern bool secp256r1_shared_secret(const uint8 public_key[SECP256R1_PUBLIC_KEY_SIZE],
                             const uint8 private_key[SECP256R1_PRIVATE_KEY_SIZE],
                             uint8 shared_secret[SECP256R1_SHARED_SECRET_SIZE]);

#endif