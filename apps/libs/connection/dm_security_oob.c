/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_security_oob.c        

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
#include <vm.h>


/*****************************************************************************/
void ConnectionSmReadLocalOobData(Task theAppTask, TRANSPORT_T transport)
{
    MAKE_CL_MESSAGE(CL_INTERNAL_SM_READ_LOCAL_OOB_DATA_REQ);
    message->task = theAppTask;
    message->transport = transport;
    MessageSend(
            connectionGetCmTask(), 
            CL_INTERNAL_SM_READ_LOCAL_OOB_DATA_REQ, 
            message
            );
}

