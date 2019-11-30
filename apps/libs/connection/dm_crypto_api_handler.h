/****************************************************************************
Copyright (c) 2017 - 2019 Qualcomm Technologies International, Ltd 


FILE NAME
    dm_crypto_api_handler.h        

DESCRIPTION
    This file contains the prototypes of functions for handling the confirmation messages from the 
    Bluestack regarding cryptography.

NOTES

*/

#include "connection.h"

 
/****************************************************************************
NAME    
    connectionBluestackHandlerDmCrypto

DESCRIPTION
    Handler for Crypto messages

RETURNS
    TRUE if message handled, otherwise false
*/
bool connectionBluestackHandlerDmCrypto(const DM_UPRIM_T *message);
