/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


DESCRIPTION
    A cryptographic library of functions available for use the the VM.
*/

#ifndef _CRYPTOVM_H__
#define _CRYPTOVM_H__

/*! The size of the AES CMAC block in bytes */
#define AES_CMAC_BLOCK_SIZE 16

/*!
    @brief AES-CMAC
    @param key [IN] The secret key
    @param message [IN] The message for which to calculate the MAC
    @param message_len [IN] The length of message
    @param mac [OUT] The message authentication code.

    The key and mac should point to arrays of length AES_CMAC_BLOCK_SIZE bytes.
    The message may be of any length including zero.
*/
void CryptoVmAesCmac(const uint8 *key, const uint8 *message, uint32 message_len, uint8 *mac);

/*!
    @brief The function h6 is used to convert keys of a given size from one key type to
           another key type with equivalent strength.
    @param keyW [IN] The secret key to be converted.
    @param keyID [IN] The conversion key ID.
    @param keyH6 [OUT] The converted secret key.

    keyW and keyH6 should point to arrays of length AES_CMAC_BLOCK_SIZE bytes.
*/
void CryptoVmH6(uint8 *keyW, uint32 keyId, uint8 *keyH6);

/*!
    @brief The function h7 is used to convert keys of a given size from one key type to
           another key type with equivalent strength.
    @param keyW [IN] The secret key to be converted.
    @param salt [IN] The conversion salt.
    @param keyH7 [OUT] The converted secret key.

    keyW, salt and keyH7 should point to arrays of length AES_CMAC_BLOCK_SIZE bytes.
*/
void CryptoVmH7(uint8 *keyW, uint8 *salt, uint8 *keyH7);

#endif
