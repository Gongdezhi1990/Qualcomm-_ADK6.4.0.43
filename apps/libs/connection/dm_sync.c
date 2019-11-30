/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_sync.c        

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

/* Default parameters for a synchronous connection.  Configured to obtain best SCO link possible.
   NOTE: It's not possible to currently have both SCO and eSCO packet types in the same request 
   due to the way the firmware works. */
const sync_config_params default_sync_config_params =
{
    8000,                    /* tx_bandwidth   */
    8000,                    /* rx_bandwidth   */
    0xFF,                    /* max_latency    */
    sync_air_coding_cvsd,    /* voice_settings */
    sync_retx_disabled,      /* retx_effort    */
    sync_all_sco             /* packet_type    */
};


/*****************************************************************************/
void ConnectionSyncRegister(Task theAppTask)
{
    /* Send an internal register request message */
    MAKE_CL_MESSAGE(CL_INTERNAL_SYNC_REGISTER_REQ);
    message->theAppTask = theAppTask;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_SYNC_REGISTER_REQ, message);
}


/*****************************************************************************/
void ConnectionSyncConnectRequest(Task theAppTask, Sink sink, const sync_config_params *config_params)
{
    /* Send an internal Synchronous connect request */
    MAKE_CL_MESSAGE(CL_INTERNAL_SYNC_CONNECT_REQ);
    message->theAppTask = theAppTask;
    message->sink = sink;
    if ( config_params )
    {
        message->config_params = *config_params;
    }
    else
    {
        message->config_params = default_sync_config_params;
    }
    
    MessageSend(connectionGetCmTask(), CL_INTERNAL_SYNC_CONNECT_REQ, message);
}


/*****************************************************************************/
void ConnectionSyncConnectResponse(Task theAppTask, const bdaddr *addr, bool accept, const sync_config_params *config_params)
{
    /* Send an internal Synchronous connect response */
    MAKE_CL_MESSAGE(CL_INTERNAL_SYNC_CONNECT_RES);
    message->theAppTask = theAppTask;
    message->bd_addr = *addr;
    message->response = accept;

    if ( config_params )
    {
        message->config_params = *config_params;
    }
    else
    {
        message->config_params = default_sync_config_params;  /* If accept==false then this is redundant */
    }
    
    MessageSend(connectionGetCmTask(), CL_INTERNAL_SYNC_CONNECT_RES, message);
}


/*****************************************************************************/
void ConnectionSyncDisconnect(Sink sink, hci_status reason)
{
    /* Send an internal Synchronous disconnect request */
    MAKE_CL_MESSAGE(CL_INTERNAL_SYNC_DISCONNECT_REQ);
    message->audio_sink = sink;
    message->reason = reason;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_SYNC_DISCONNECT_REQ, message);
}

#endif
