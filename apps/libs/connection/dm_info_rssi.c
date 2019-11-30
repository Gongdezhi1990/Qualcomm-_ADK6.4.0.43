/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_info_rssi.c

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


/*****************************************************************************/
void ConnectionGetRssi(Task theAppTask, Sink sink)
{
    /* All requests are sent through the internal state handler */
    MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_RSSI_REQ);
    message->theAppTask = theAppTask;
    message->sink = sink;
    /* message->tpaddr does not need to be set. */
    MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_READ_RSSI_REQ, message);
}

/*****************************************************************************/
void ConnectionGetRssiBdaddr(Task theAppTask, const tp_bdaddr *tpaddr)
{
    /* All requests are sent through the internal state handler */
    MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_RSSI_REQ);
    message->theAppTask = theAppTask;
    message->sink = (Sink)NULL; /* make sink to null to use tpaddr instead */
    message->tpaddr = *tpaddr;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_READ_RSSI_REQ, message);
}


