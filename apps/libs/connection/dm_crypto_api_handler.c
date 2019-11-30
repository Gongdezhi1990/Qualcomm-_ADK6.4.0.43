/****************************************************************************
Copyright (c) 2018 - 2019 Qualcomm Technologies International, Ltd 


FILE NAME
    dm_crypto_api_handler.c        

DESCRIPTION
    This file is a stub for Bluestack cryptography functionality.
    Builds requiring this should include CONFIG_CRYPTO in the
    makefile. e.g.
        CONFIG_FEATURES:=CONFIG_CRYPTO
    
NOTES

*/

#include <vm.h>

#include "connection.h"
#include "connection_private.h"
#include "common.h"
#include "dm_crypto_api_handler.h"
 
/****************************************************************************
NAME    
    connectionBluestackHandlerDmCrypto

DESCRIPTION
    Handler for Crypto messages

RETURNS
    void
*/
bool connectionBluestackHandlerDmCrypto(const DM_UPRIM_T *message)
{
    UNUSED(message);
    /* Not supported */
    return FALSE;
}
