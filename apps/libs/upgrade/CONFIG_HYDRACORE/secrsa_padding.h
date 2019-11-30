/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    secrsa_padding.h

DESCRIPTION
    Header file for a PKCS #1 PSS verify (decode) implementation.

NOTES

*/

#ifndef __SECRSA_PADDING_H__
#define __SECRSA_PADDING_H__

/* Enumeration defining return values for the
 * ce_pkcs1_pss_padding_verify function.
 */
typedef enum {
    CE_SUCCESS,
    CE_ERROR_FAILURE,
    CE_ERROR_INVALID_ARG,
    CE_ERROR_NOT_SUPPORTED,
    CE_ERROR_MESSAGE_TOO_LONG,
    CE_ERROR_BUFFER_OVERFLOW,
    CE_ERROR_INVALID_PACKET,
    CE_ERROR_NO_MEMORY,
    CE_ERROR_INVALID_SIGNATURE
};

#define HASH_ALGORITHM_LEN_SHA256 32

/**
  PKCS #1 PSS verify.

  @param[in] msghash          The encoded data to decode
  @param[in] msghashlen       The length of the encoded data (octets)
  @param[in] sig              The signature
  @param[in] siglen           The signature length
  @param[in] saltlen          The length of salt
  @param[in] modulus_bitlen   The bit length of the RSA modulus

  @return 
   CE_SUCCESS                   - Function executes successfully. \n
   CE_ERROR_INVALID_ARG         - a parameter is invalid. \n
   CE_ERROR_INVALID_SIGNATURE   - the signature is invalid. \n
   CE_ERROR_NO_MEMORY           - out of memory. \n
   CE_ERROR_NOT_SUPPORTED       - the feature is not supported. \n
   CE_ERROR_INVALID_PACKET      - invalid packet. \n
   CE_ERROR_BUFFER_OVERFLOW     - not enough space for output. \n
   CE_ERROR_FAILURE             - ce_pkcs1_mgf1 or ce_hash failed. \n
 */
int ce_pkcs1_pss_padding_verify
(
   const unsigned char       *msghash,
   unsigned long             msghashlen,
   const unsigned char       *sig,
   unsigned long             siglen,
   unsigned long             saltlen,
   unsigned long             modulus_bitlen
);

#endif
