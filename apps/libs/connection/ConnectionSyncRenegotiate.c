/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    ConnectionSyncRenegotiate.c        

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

extern const sync_config_params default_sync_config_params;

/*****************************************************************************/
void ConnectionSyncRenegotiate(Task theAppTask, Sink sink, const sync_config_params *config_params)
{
    /* Send an internal change Synchronous packet type request */
    MAKE_CL_MESSAGE(CL_INTERNAL_SYNC_RENEGOTIATE_REQ);
    message->theAppTask = theAppTask;
    message->audio_sink = sink;
    if ( config_params )
    {
        message->config_params = *config_params;
    }
    else
    {
        message->config_params = default_sync_config_params;
    }
    
    MessageSend(connectionGetCmTask(), CL_INTERNAL_SYNC_RENEGOTIATE_REQ, message);
}

#endif
