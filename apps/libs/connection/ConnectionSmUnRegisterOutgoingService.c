/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    ConnectionSmUnRegisterOutgoingService.c        

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
void ConnectionSmUnRegisterOutgoingService(const bdaddr* bd_addr, dm_protocol_id protocol_id, uint16 channel)
{
#ifdef CONNECTION_DEBUG_LIB
    if ((protocol_id != protocol_l2cap) && (protocol_id != protocol_rfcomm))
    {
        CL_DEBUG(("Out of range protocol id 0x%x\n", protocol_id));
    }

    /* TODO: Check if we should check channel range for outgoing service channel */
    if((protocol_id == protocol_rfcomm) &&
            ((channel < RFCOMM_SERVER_CHANNEL_MIN) ||
                    (channel > RFCOMM_SERVER_CHANNEL_MAX))
        )
    {
        CL_DEBUG(("Out of range RFCOMM server channel 0x%x\n", channel));
    }

    if(bd_addr == NULL)
    {
       CL_DEBUG(("Out of range Bluetooth Address 0x%p\n", (void*)bd_addr));
    }
#endif

    {
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_UNREGISTER_OUTGOING_REQ);
        message->bd_addr = *bd_addr;
        message->protocol_id = protocol_id;
        message->channel = channel;
        MessageSend(connectionGetCmTask(), CL_INTERNAL_SM_UNREGISTER_OUTGOING_REQ, message);
    }
}

