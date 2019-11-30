/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_security_sdp.c        

DESCRIPTION
    This file contains the management entity responsible for device security

NOTES

*/


/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"

#include    <message.h>
#include    <string.h>
#include    <vm.h>



/******************************************************************************
 This function configures the security settings for SDP service record 
 browsing. It will configure incoming connections to permit service record to 
 be browsed without the need for the devices concerned to be paired. 
*/
void ConnectionSmSetSdpSecurityIn(bool enable)
{
#ifdef CONNECTION_DEBUG_LIB
    if ((enable != TRUE) && (enable != FALSE))
    {
        CL_DEBUG(("Out of range enable 0x%x\n", enable));
    }
#endif

    if (enable)
    {
        ConnectionSmRegisterIncomingService(protocol_l2cap, UUID16_SDP, sec_in_none);
    }
    else
    {
        ConnectionSmUnRegisterIncomingService(protocol_l2cap, UUID16_SDP);
    }
}


