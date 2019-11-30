/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


DESCRIPTION
    Implementation of some of the functions in the Bluetooth cryptographic toolbox.
*/

#include "cryptovm.h"

/*! @brief The function h6 is used to convert keyW to keyH6 with equivalent strength. */
void CryptoVmH6(uint8 *keyW, uint32 keyId, uint8 *keyH6)
{
    CryptoVmAesCmac(keyW, (uint8*)&keyId, sizeof(keyId), keyH6);
}

/*! @brief The function h7 is used to convert keyW to keyH7 with equivalent strength. */
void CryptoVmH7(uint8 *keyW, uint8 *salt, uint8 *keyH7)
{
    CryptoVmAesCmac(salt, keyW, AES_CMAC_BLOCK_SIZE, keyH7);
}
