/****************************************************************************
Copyright (c) 2016-2017 Qualcomm Technologies International, Ltd.


FILE NAME
    rsa_decrypt.h

DESCRIPTION
    Some information is RSA-1024 signed. The structure and functions defined
    allows the signature to be decrypted using the constant information derived
    on the host from the public key.

NOTES

*/

#ifndef __RSA_DECRYPT_H__
#define __RSA_DECRYPT_H__

/*============================================================================*
Public Definitions
*============================================================================*/
/*
 * To use RSA-1024 rather than RSA-2048, replace the following definition of
 * UPGRADE_RSA_2048 with a definition of UPGRADE_RSA_1024.
 */
#define UPGRADE_RSA_2048
/*
 * To use an RSA key exponent of 3 rather than F4 (65537), replace the following
 * definition of UPGRADE_RSA_KEY_EXPONENT_F4 with a definition of
 * UPGRADE_RSA_KEY_EXPONENT_3.
 */
#define UPGRADE_RSA_KEY_EXPONENT_F4


#if defined (UPGRADE_RSA_2048)
#define RSA_SIGNATURE_SIZE (128) /* 128 * 16 = 2048 bits */
#elif defined (UPGRADE_RSA_1024)
#define RSA_SIGNATURE_SIZE (64) /* 64 * 16 = 1024 bits */
#else
#error "Neither UPGRADE_RSA_2048 nor UPGRADE_RSA_1024 defined."
#endif

typedef struct rsa_mod
{
    uint16 M[RSA_SIGNATURE_SIZE];           /* the modulus M = p1*p2 */
    /* parameters specific to the Montgomery multiplication: */
    uint16 M_dash;                          /* -M^(-1) mod b, b = 2^T */
} rsa_mod_t;

/*============================================================================*
Public Functions
*============================================================================*/

void rsa_decrypt(uint16 *start, const rsa_mod_t *mod, uint16 *A);
void c_mont_mult (uint16 *A, const uint16 *B, const rsa_mod_t *ms);

#endif /* __RSA_DECRYPY_H__ */

