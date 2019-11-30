/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_security_authenticate.c        

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
void ConnectionSmAuthenticate(Task theAppTask, const bdaddr* bd_addr, uint16 timeout)
{
#ifdef CONNECTION_DEBUG_LIB
    if ((timeout < MIN_AUTHENTICATION_TIMEOUT) || (timeout >  MAX_AUTHENTICATION_TIMEOUT))
    {
        CL_DEBUG(("Out of range timeout 0x%x\n", timeout));
    }

    if(bd_addr == NULL)
    {
       CL_DEBUG(("Out of range Bluetooth Address 0x%p\n", (void*)bd_addr));
    }
#endif

    {
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_AUTHENTICATION_REQ);
        message->task = theAppTask;
        message->bd_addr = *bd_addr;
        message->timeout = (uint32) D_SEC(timeout);
        MessageSend(connectionGetCmTask(), CL_INTERNAL_SM_AUTHENTICATION_REQ, message);
    }
}


/*****************************************************************************/
void ConnectionSmCancelAuthenticate(Task theAppTask, bool force)
{
    MAKE_CL_MESSAGE(CL_INTERNAL_SM_CANCEL_AUTHENTICATION_REQ);
    message->task = theAppTask;
    message->force = force;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_SM_CANCEL_AUTHENTICATION_REQ, message);
}

