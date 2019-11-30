/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    ConnectionSmSetTrustLevel.c        

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
void ConnectionSmSetTrustLevel(Task theAppTask, const bdaddr* bd_addr, uint16 trusted)
{
    /* Update the Trusted Device List */
    MAKE_CL_MESSAGE(CL_INTERNAL_SM_SET_TRUST_LEVEL_REQ)
    message->theAppTask = theAppTask;
    message->bd_addr = *bd_addr;
    message->trusted = trusted;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_SM_SET_TRUST_LEVEL_REQ, message);
}


