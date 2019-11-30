/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_info_remote_version.c        

DESCRIPTION
    This file contains the management entity responsible for arbitrating 
    access to functionality in Bluestack that provides information on
    the setup of the local device or about the link to the remote device.

NOTES

*/


/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "sink.h"
#include "bdaddr.h"

/*****************************************************************************/
void ConnectionReadRemoteVersion(Task theAppTask, Sink sink)
{
    /* All requests are sent through the internal state handler */
    tp_bdaddr tpaddr;
     
    /* Check the sink is valid by trying to retrieve the associated device 
     * address. 
     */
    if (!SinkGetBdAddr(sink, &tpaddr))
    {
        /* The sink doesn't have a device address associated and so indicate
         * the error to the application.
         */
        MAKE_CL_MESSAGE(CL_DM_REMOTE_VERSION_CFM);
        message->status = hci_error_no_connection;  
        message->lmpVersion       = 0;
        message->manufacturerName = 0;
        message->lmpSubVersion    = 0;
        MessageSend(theAppTask, CL_DM_REMOTE_VERSION_CFM, message);
    }
    else
    {
        MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_REMOTE_VERSION_REQ);

        message->theAppTask = theAppTask;
        message->tpaddr = tpaddr;

        MessageSend(
                connectionGetCmTask(),
                CL_INTERNAL_DM_READ_REMOTE_VERSION_REQ,
                message
                );
    }
    
}

void ConnectionReadRemoteVersionBdaddr(Task theAppTask, const tp_bdaddr *tpaddr)
{
    /* All requests are sent through the internal state handler */
    MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_REMOTE_VERSION_REQ);
    message->theAppTask = theAppTask;
    message->tpaddr = *tpaddr;
    MessageSend(
            connectionGetCmTask(),
            CL_INTERNAL_DM_READ_REMOTE_VERSION_REQ,
            message
            );
}

