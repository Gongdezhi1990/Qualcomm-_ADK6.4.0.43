/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_crypto_api_handler.c        

DESCRIPTION
    This file contains functions for handling the confirmation messages from the Bluestack 
    regarding cryptography.Unused for fastpair.

NOTES

*/

#include <vm.h>

#include "connection.h"
#include "connection_private.h"
#include "common.h"
#include "dm_crypto_api_handler.h"

#ifndef UNUSED
#define UNUSED(x)       ((void)x)
#endif
bool connectionBluestackHandlerDmCrypto(const DM_UPRIM_T *message)
{
     UNUSED(message);
     return FALSE;
}