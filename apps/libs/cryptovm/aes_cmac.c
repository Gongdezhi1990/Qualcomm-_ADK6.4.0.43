/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


DESCRIPTION
    Implementation of AES-CMAC as defined in RFC 4493.
*/

#include "cryptovm.h"
#include <crypto.h>

#define RB 0x87

/*! A shorter name for convenience */
#define BSIZE AES_CMAC_BLOCK_SIZE

static const uint8 zeros[BSIZE] = {0};

/*! @brief Left shift in by one bit into out: in and out may be the same address */
static void leftShiftOneBitBlock(const uint8 *in, uint8 *out)
{
    int32 i;
    uint8 overflow = 0;

    for (i = BSIZE-1; i >= 0; i--)
    {
        uint8 in_temp = in[i];
        out[i] = (in_temp << 1) | overflow;
        overflow = (in_temp & 0x80) ? 1 : 0;
    }
}

/*! @brief Perform the common shift and xor required when generating k1/k2 */
static void generateSubkeyHelper(uint8 *subkey)
{
    bool msb_set = (subkey[0] & 0x80) != 0;
    leftShiftOneBitBlock(subkey, subkey);
    if (msb_set)
    {
        subkey[BSIZE-1] ^= RB;
    }
}


/*! @brief Generate subkey k1 or k2
    @param key [IN] The secret key
    @param k1 [IN] If TRUE, generate subkey k1, otherwise generate subkey k2
    @param subkey [OUT] The generated subkey (k1 or k2)
*/
static void generateSubkey(const uint8 *key, bool k1, uint8 *subkey)
{

    CryptoAes128Ctr(key, zeros, 0, 0, zeros, BSIZE, subkey, BSIZE);
    generateSubkeyHelper(subkey);
    if (!k1)
    {
        generateSubkeyHelper(subkey);
    }
}

/*! @brief Pad the input of length unpadded_len, writing the result to out */
static void padBlock(const uint8 *in, uint8 *out, uint32 unpadded_len)
{
    uint32 amount_to_pad = BSIZE - unpadded_len;
    if (unpadded_len)
    {
        memcpy(out, in, unpadded_len);
    }
    if (amount_to_pad)
    {
        out[unpadded_len] = 0x80;
        --amount_to_pad;
    }
    if (amount_to_pad)
    {
        memset(out + unpadded_len + 1, 0, amount_to_pad);
    }
}

/*! @brief Exclusive-OR in1 and in2, writing the result to out */
static void xorBlock(const uint8 *in1, const uint8 *in2, uint8 *out)
{
    uint32 i;
    for (i = 0; i < BSIZE; i++)
    {
        out[i] = in1[i] ^ in2[i];
    }
}

/*! @brief AES-CMAC */
void CryptoVmAesCmac(const uint8 *key, const uint8 *message, uint32 message_len, uint8 *mac)
{
    uint8 subkey[BSIZE], last_block[BSIZE];

    uint32 nblocks = message_len ? (message_len + (BSIZE-1)) / BSIZE : 1;
    uint32 message_len_mod_bsize = message_len % BSIZE;
    bool complete_last_block = message_len ? (message_len_mod_bsize == 0) : FALSE;

    /* Generate k1 or k2 depending on the completeness of the last block */
    generateSubkey(key, complete_last_block, subkey);
    if (complete_last_block)
    {
        xorBlock(&message[BSIZE * (nblocks - 1)], subkey, last_block);
    }
    else
    {
        padBlock(&message[BSIZE * (nblocks - 1)], last_block, message_len_mod_bsize);
        xorBlock(last_block, subkey, last_block);
    }
    memset(mac, 0, BSIZE);
    if (nblocks > 1)
    {
        CryptoAes128Cbc(TRUE, key, mac, 0, message, (nblocks-1) * BSIZE, NULL, 0);
    }
    CryptoAes128Cbc(TRUE, key, mac, 0, last_block, BSIZE, NULL, 0);
}
