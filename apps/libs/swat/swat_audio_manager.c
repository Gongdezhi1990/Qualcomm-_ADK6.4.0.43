/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    swat_audio_manager.c

DESCRIPTION
    Contains functions to manage audio connections to remote devices
*/

#include "swat.h"
#include "swat_private.h"
#include "swat_api.h"
#include "swat_device_manager.h"
#include "swat_command_handler.h"
#include "swat_state_manager.h"
#include "swat_l2cap_handler.h"
#include "swat_audio_manager.h"

#include <connection.h>


/* 
   Default eSCO config parameters for the source device 
   Force use of TeSCO = 6 and WeSCO = 2 */
const sync_config_params source_esco_config_V1 = 
{
    8000,                           /* tx_bandwidth     */
    0,                              /* rx_bandwidth     */
    7,                              /* max_latency      */
    sync_air_coding_transparent,    /* voice_settings   */
    sync_retx_link_quality,         /* retx_effort      */
    (sync_all_edr_esco - sync_2ev3) /* packet_type      */
};

/* V2 sco configuration uses Tesco = 12 Wesco = 6 */
const sync_config_params source_esco_config_V2 = 
{
    8000,                           /* tx_bandwidth     */
    0,                              /* rx_bandwidth     */
    0xffff,                         /* max_latency      */
    sync_air_coding_transparent,    /* voice_settings   */
    sync_retx_link_quality,         /* retx_effort      */
    (sync_all_edr_esco - sync_2ev3) /* packet_type      */
};

/* Default eSCO config parameters for the sink device */
const sync_config_params sink_esco_config = 
{
    0xFFFFFFFF,                     /* tx_bandwidth     */
    0xFFFFFFFF,                     /* rx_bandwidth     */
    0xFFFF,                         /* max_latency      */
    sync_air_coding_transparent,    /* voice_settings   */
    sync_retx_link_quality,         /* retx disabled    */
    (sync_all_edr_esco - sync_2ev3) /* ev5 packet type  */
};

/*****************************************************************************/
void swatRequestEsco(remoteDevice *device)
{
    /* If client supplied specific eSCO parameters, use them, otherwise use the library defaults */
    if (swat->esco_config)
    {
        ConnectionSyncConnectRequest(&swat->l2cap_task, device->signalling_sink, swat->esco_config);
    }
    else
    {
        /* look for V2 Subwoofer */
        if(device->version_major == SUBWOOFER_MAJOR_VERSION)
        {
            ConnectionSyncConnectRequest(&swat->l2cap_task, device->signalling_sink, &source_esco_config_V2);
        }
        /* legacy support of Subwoofer ROM or ADK */
        else
        {
            ConnectionSyncConnectRequest(&swat->l2cap_task, device->signalling_sink, &source_esco_config_V1);
        }
    }
    
    /* Set the transport state */
    device->transport_ll_state = swat_transport_outgoing;
}


/*****************************************************************************/
void swatHandleIncomingScoRequest(CL_DM_SYNC_CONNECT_IND_T * ind)
{
    remoteDevice * device = swatFindDeviceFromBdaddr(&ind->bd_addr);
    
    if (device)
    {
        /* Is the low latency media channel in the opening state? If not, reject the request for eSCO */
        if (device->media_ll_state == swat_media_opening)
        {
            /* Only accept connection request when local device is a Sink */
            if (swat->role == swat_role_sink)
            {
                SWAT_DEBUG(("[SWAT] Accept eSCO request from device [%x]\n", device->id));
                ConnectionSyncConnectResponse(&swat->l2cap_task, &ind->bd_addr, TRUE, &sink_esco_config);
                device->transport_ll_state = swat_transport_incoming;
                return;
            }
            else
            {
                SWAT_DEBUG(("[SWAT] Reject eSCO request from device [%x] Local role = Source; Sink cannot initiate eSCO", device->id));
                ConnectionSyncConnectResponse(&swat->l2cap_task, &ind->bd_addr, FALSE, 0);
                device->transport_ll_state = swat_transport_idle;
                return;
            }
        }
        else
        {
            SWAT_DEBUG(("[SWAT] Reject eSCO request from device [%x] media channel in wrong state [%x]\n", device->id, device->media_ll_state));
            ConnectionSyncConnectResponse(&swat->l2cap_task, &ind->bd_addr, FALSE, 0);
            device->transport_ll_state = swat_transport_idle;
            return;
        }
    }
    else
    {
        SWAT_DEBUG(("[SWAT] Reject eSCO request from unknown device\n"));
        ConnectionSyncConnectResponse(&swat->l2cap_task, &ind->bd_addr, FALSE, 0);
    }
}


/*****************************************************************************/
void swatHandleScoConnectCfm(CL_DM_SYNC_CONNECT_CFM_T * cfm)
{
    remoteDevice * dev = swatFindDeviceFromBdaddr(&cfm->bd_addr);
    
    if (dev)
    {
        if (cfm->status == hci_success)
        {
            SWAT_DEBUG(("[SWAT] Device ID[%x] : Media connected (low latency)\n", dev->id));
            swatSetMediaState(dev, SWAT_MEDIA_LOW_LATENCY, swat_media_open);
            dev->media_ll_sink = cfm->audio_sink;
            dev->transport_ll_state = swat_transport_connected;
            
            /* Don't send the CFM to client task if going to close immediatley after */
            if (!dev->disconnecting_signalling)
            {
                swatSendMediaOpenCfmToClient(cfm->status, dev->id, SWAT_MEDIA_LOW_LATENCY, cfm->audio_sink);
            }
        }
        else
        {
            SWAT_DEBUG(("[SWAT] Device ID[%x] : Media (low latency) connection error STATUS[%x]\n", dev->id, cfm->status));
            swatSetMediaState(dev, SWAT_MEDIA_LOW_LATENCY, swat_media_closed);
            dev->transport_ll_state = swat_transport_idle;
            
            /* Don't send the CFM to client task if going to close immediatley after (even though it failed to open) */
            if (!dev->disconnecting_signalling)
            {
                swatSendMediaOpenCfmToClient(swat_esco_fail, 0, 0, 0);
            }
        }
        
        /* Command has been handled; can now release command block to process next queued command (if one exists) */
        SWAT_DEBUG(("[SWAT] SWAT_OPEN_CMD Handled\n"));
        dev->signalling_block = 0;

        /* Was this connection started before signalling disconnection process? */
        if (dev->disconnecting_signalling)
        {
            swatHandleL2capSignallingDisconnectReq(dev->id);
        }
    }
    else
    {
        /* Should never get here */
        SWAT_DEBUG(("[SWAT] LIBRARY ERROR : Device is Null\n"));
    }
    
}


/*****************************************************************************/
bool swatRequestEscoDisconnect(remoteDevice *dev)
{
    bool retval = FALSE;
    
    switch(dev->transport_ll_state)
    {
        case swat_transport_idle:
        {
            /* Don't need to wait for anything to disconnect */
        }
        break;
        case swat_transport_closing:
        {
            retval = TRUE;    /* Wait for disconnect_IND or disconnect_CFM message on media channel */
        }
        break;
        case swat_transport_incoming:
        case swat_transport_outgoing:
        {
            retval = TRUE;                          /* Wait for connect_CFM message on media channel before closing */
        }
        break;
        case swat_transport_connected:
        {
            /* Start the eSCO disconnection process */
            dev->transport_ll_state = swat_transport_closing;
            ConnectionSyncDisconnect(dev->media_ll_sink, hci_error_oetc_user);
            retval = TRUE;    /* Wait for disconnect_CFM message on media channel */
        }
        break;
    }
    
    return retval;
}


/*****************************************************************************/
void swatHandleIncomingScoDisconnect(Sink sink, hci_status status)
{
    remoteDevice * dev = swatFindLocalDeviceFromSink(sink);
    if (dev)
    {
        SWAT_DEBUG(("[SWAT] Low latency media channel to Device ID[%x] was disconnected\n", dev->id));
        swatSetMediaState(dev, SWAT_MEDIA_LOW_LATENCY, swat_media_closed);
        dev->transport_ll_state = swat_transport_idle;
        dev->media_ll_sink = NULL;
        
        /* If it was the local device that initiated the eSCO disconnection, send a _CFM message to the client to indicate the media channel has now been closed, otherwise send an _IND message to inform the remote device is closing the channel */
        if (dev->initiated_esco_disconnect)
        {
            /* Inform the client task the SWAT media channel has now been closed */
            swatSendMediaCloseCfmToClient(swat_success, dev->id, SWAT_MEDIA_LOW_LATENCY);
            dev->initiated_esco_disconnect = 0;
        }
        else
        {
            /* Inform the client task the SWAT media channel has now been closed */
            swatSendMediaCloseIndToClient(dev->id, SWAT_MEDIA_LOW_LATENCY);
        }
        
        /* CLOSE_CMD has now completed, release block to action next command (if one exists) */
        SWAT_DEBUG(("[SWAT] SWAT_CLOSE_CMD Handled\n"));
        dev->signalling_block = 0;
        
        /* Is this disconnection required as part of signalling disconnection process? */
        if (dev->disconnecting_signalling)
        {
            /* Disconnect the signalling channel */
            swatHandleL2capSignallingDisconnectReq(dev->id);
        }
    }
    else
    {
        /* Should never get here */
        SWAT_DEBUG(("[SWAT] LIBRARY ERROR : Device is Null\n"));
    }
}


/*****************************************************************************/
void swatHandleUnresponsiveMediaLL(uint16 device_id)
{
    SWAT_DEBUG(("[SWAT] Device ID[%x] did not respond to the last SWAT command\n", device_id));
    
    /* What state is the media channel in? */
    switch(swat->remote_devs[device_id].media_ll_state)
    {
        case swat_media_opening:
        {
            /* Remote device failed to respond to an OPEN request */
            swatSendMediaOpenCfmToClient(swat_signalling_no_response, device_id, SWAT_MEDIA_LOW_LATENCY, NULL);
            
            /* Revert media state as it failed to open */
            swatSetMediaState(&swat->remote_devs[device_id], SWAT_MEDIA_LOW_LATENCY, swat_media_closed);
        }
        break;
        case swat_media_starting:
        {
            /* Remote device failed to respond to a START request*/
            swatSendMediaStartCfmToClient(swat_signalling_no_response, device_id, SWAT_MEDIA_LOW_LATENCY);
            
            /* Revert media state as it failed to open */
            swatSetMediaState(&swat->remote_devs[device_id], SWAT_MEDIA_LOW_LATENCY, swat_media_open);
        }
        break;
        case swat_media_suspending:
        {
            /* Remote device failed to respond to a SUSPEND request */
            swatSendMediaSuspendCfmToClient(swat_signalling_no_response, device_id, SWAT_MEDIA_LOW_LATENCY);
            
            /* Revert media state as it failed to streaming */
            swatSetMediaState(&swat->remote_devs[device_id], SWAT_MEDIA_LOW_LATENCY, swat_media_streaming);
        }
        break;
        case swat_media_closing:
        {
            /* Remote device failed to respond to a CLOSE request */
            swatSendMediaCloseCfmToClient(swat_signalling_no_response, device_id, SWAT_MEDIA_LOW_LATENCY);
            
            /* Revert media state as it failed to open */
            swatSetMediaState(&swat->remote_devs[device_id], SWAT_MEDIA_LOW_LATENCY, swat_media_open);
        }
        break;
        
        /* Media channel shouldn't become unresponsive in these stable states */
        case swat_media_streaming:
        case swat_media_closed:
        case swat_media_open:
        {
            SWAT_DEBUG(("[SWAT] Library error : unresponsive low latency media channel in media STATE[%x]\n", swat->remote_devs[device_id].media_ll_state));
            return;
        }
        break;
        
        default:
        {
            /* handle swat_media_invalid state quietly */
            SWAT_DEBUG(("[SWAT] swatHandleUnresponsiveMediaLL media_ll_state:[%x]\n", swat->remote_devs[device_id].media_ll_state));
        }
        break;
    }
}
