/****************************************************************************
Copyright (c) 2016-2017 Qualcomm Technologies International, Ltd.


FILE NAME
    rsa_decrypt.c

DESCRIPTION
    Some information is RSA-2048 (conditionally RSA-1024) signed. The function
    defined allows the signature to be decrypted using the constant information
    derived on the host from the public key.

NOTES

*/

#include "rsa_decrypt.h"

/****************************************************************************/
/*! \brief  decrypt a block of memory using the supplied public key and the 
 *          supplied block of memory.  
 *
 *  \param  start  Pointer to the start of the signature.
 *  \param  mod
 *  \param  A
 *
 *  \return None.
 *
 *  \detail Use the montgomery multiplication algorithm to exponentiate the
 *          block modulus the key given. The exponent is always 3, atm.
 *          More choice may follow, but not in this version of the firmware.
 *
 *          A is a lump of memory passed in initialised to R^2N mod M.
 *          See Knuth for details of the Montgomery multiplication algorithm.
 */
/****************************************************************************/
void rsa_decrypt(uint16 *start, const rsa_mod_t *mod, uint16 *A)
{
    /* A is initialised to be R^2N mod M */
#if defined (UPGRADE_RSA_KEY_EXPONENT_F4)
    c_mont_mult(A, start, mod);    /* A = x*R^N mod M */
    c_mont_mult(A, A, mod);        /* A = (x^2)*R^N mod M */
    c_mont_mult(A, A, mod);        /* A = (x^4)*R^N mod M */
    c_mont_mult(A, A, mod);        /* A = (x^8)*R^N mod M */
    c_mont_mult(A, A, mod);        /* A = (x^16)*R^N mod M */
    c_mont_mult(A, A, mod);        /* A = (x^32)*R^N mod M */
    c_mont_mult(A, A, mod);        /* A = (x^64)*R^N mod M */
    c_mont_mult(A, A, mod);        /* A = (x^128)*R^N mod M */
    c_mont_mult(A, A, mod);        /* A = (x^256)*R^N mod M */
    c_mont_mult(A, A, mod);        /* A = (x^512)*R^N mod M */
    c_mont_mult(A, A, mod);        /* A = (x^1024)*R^N mod M */
    c_mont_mult(A, A, mod);        /* A = (x^2048)*R^N mod M */
    c_mont_mult(A, A, mod);        /* A = (x^4096)*R^N mod M */
    c_mont_mult(A, A, mod);        /* A = (x^8192)*R^N mod M */
    c_mont_mult(A, A, mod);        /* A = (x^16384)*R^N mod M */
    c_mont_mult(A, A, mod);        /* A = (x^32768)*R^N mod M */
    c_mont_mult(A, A, mod);        /* A = (x^65536)*R^N mod M */
    c_mont_mult(start, A, mod);    /* x = x^65537 mod M */
#elif defined (UPGRADE_RSA_KEY_EXPONENT_3)
    c_mont_mult(A, start, mod);    /* A = x*R^N mod M */
    c_mont_mult(A, A, mod);        /* A = (x^2)*R^N mod M */
    c_mont_mult(start, A, mod);    /* x = x^3 mod M */
#else
#error "Neither UPGRADE_RSA_KEY_EXPONENT_F4 nor UPGRADE_RSA_KEY_EXPONENT_3 defined."
#endif
}

