/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    ConnectionSyncUnregister.c        

DESCRIPTION
    This file contains the functions responsible for managing the setting up 
    and tearing down of Synchronous connections.

NOTES

*/


/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"

#ifndef CL_EXCLUDE_SYNC

/*****************************************************************************/
void ConnectionSyncUnregister(Task theAppTask)
{
    /* Send an internal unregister request message */
    MAKE_CL_MESSAGE(CL_INTERNAL_SYNC_UNREGISTER_REQ);
    message->theAppTask = theAppTask;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_SYNC_UNREGISTER_REQ, message);
}

#endif

