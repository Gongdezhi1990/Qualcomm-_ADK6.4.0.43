/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    ConnectionSmGetAuthDevice.c        

DESCRIPTION
    This file contains the management entity responsible for device security

NOTES

*/


/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"

#include <message.h>
#include <string.h>
#include <vm.h>


/*****************************************************************************/
void ConnectionSmGetAuthDevice(Task theAppTask, const bdaddr *peer_bd_addr)
{
#ifdef CONNECTION_DEBUG_LIB
    if(peer_bd_addr == NULL)
    {
       CL_DEBUG(("Out of range Bluetooth Address 0x%p\n", (void*)peer_bd_addr));
    }
#endif

    {
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_GET_AUTH_DEVICE_REQ);
        message->theAppTask = theAppTask;
        message->bd_addr = *peer_bd_addr;
        MessageSend(connectionGetCmTask(), CL_INTERNAL_SM_GET_AUTH_DEVICE_REQ, message);
    }
}
