/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    ConnectionSmRegisterOutgoingService.c        

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

/*****************************************************************************/

void ConnectionSmRegisterOutgoingService(Task theAppTask, const bdaddr* bd_addr, dm_protocol_id protocol_id, uint16 channel, dm_security_out security)
{
    MAKE_CL_MESSAGE(CL_INTERNAL_SM_REGISTER_OUTGOING_REQ);

#ifdef CONNECTION_DEBUG_LIB
    if ((protocol_id != protocol_l2cap) && (protocol_id != protocol_rfcomm))
    {
        CL_DEBUG(("Out of range protocol id 0x%x\n", protocol_id));
    }
    else if ((protocol_id == protocol_rfcomm) && !theAppTask)
    {
        CL_DEBUG(("App task undefined for RFCOMM\n"));
    }

    if(bd_addr == NULL)
    {
       CL_DEBUG(("Out of range Bluetooth Address 0x%p\n", (void*)bd_addr));
    }

    /* Are any bits other then valid dm_security_in bits are set. */
    if (security & ~sec_out_bitmask)
    {
        CL_DEBUG(("Invalid dm_security_out bits set 0x%x\n", (security & ~sec_out_bitmask)));
    }

#endif

    message->theAppTask = theAppTask;
    message->bd_addr = *bd_addr;
    message->protocol_id = protocol_id;
    message->remote_channel = channel;
    message->outgoing_security_level = security;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_SM_REGISTER_OUTGOING_REQ, message);
}



