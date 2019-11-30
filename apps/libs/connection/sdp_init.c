/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sdp_init.c        

DESCRIPTION
    This file contains the functions to initialise the SDP component of 
    the connection library    

NOTES

*/

/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "common.h"
#include "init.h"
#include "sdp_init.h"

#include <bdaddr.h>


/****************************************************************************
NAME
    connectionSdpInit

DESCRIPTION
    This Function is called to initialise Sdp

RETURNS

*/
void connectionSdpInit(connectionSdpState *state)
{
    /* Set the address to all zeroes */
    BdaddrSetZero(&state->sdpServerAddr);

     /* Confirm object has been initialised */
    connectionSendInternalInitCfm(connectionInitSdp);
}
