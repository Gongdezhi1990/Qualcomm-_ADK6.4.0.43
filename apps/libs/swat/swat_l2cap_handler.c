/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    swat_l2cap_handler.c

DESCRIPTION
    Contains functions to handle L2CAP connections for the SWAT protocol
*/



/****************************************************************************
    Header files
*/
#include "swat.h"
#include "swat_private.h"
#include "swat_api.h"
#include "swat_device_manager.h"
#include "swat_audio_manager.h"
#include "swat_packet_handler.h"
#include "swat_command_handler.h"
#include "swat_state_manager.h"
#include "swat_l2cap_handler.h"

/* External lib includes */
#include <connection.h>
#include <bdaddr.h>
#include <service.h>
#include <region.h>

/* Firmware includes */
#include <panic.h>
#include <message.h>
#include <stream.h>

/****************************************************************************
    Data Structures
*/
static const uint16 swat_media_conftab[] =
{
    L2CAP_AUTOPT_SEPARATOR,                             /* START */
    L2CAP_AUTOPT_MTU_IN,            0x037F,             /* Maximum inbound MTU - 895 bytes */
    L2CAP_AUTOPT_MTU_OUT,           0x0032,             /* Minimum acceptable outbound MTU - 50 bytes */
    L2CAP_AUTOPT_FLUSH_IN,          0x0000, 0x0000,     /* Min acceptable remote flush timeout - zero*/
                                    0xFFFF, 0xFFFF,     /* Max acceptable remote flush timeout - infinite*/
    L2CAP_AUTOPT_FLUSH_OUT,         BKV_UINT32R(135000, 140000),  /* Acceptable local flush timeout 135ms */
    L2CAP_AUTOPT_TERMINATOR                             /* END */   
};

static const uint16 swat_signalling_conftab[] =
{
    L2CAP_AUTOPT_SEPARATOR,                             /* START */
    L2CAP_AUTOPT_MTU_IN,            0x037F,             /* Maximum inbound MTU - 895 bytes */
    L2CAP_AUTOPT_MTU_OUT,           0x0030,             /* Minimum acceptable outbound MTU - 48 bytes */
    L2CAP_AUTOPT_FLUSH_IN,          0x0000, 0x0000,     /* Min acceptable remote flush timeout - zero*/
                                    0xFFFF, 0xFFFF,     /* Max acceptable remote flush timeout - infinite*/
    L2CAP_AUTOPT_FLUSH_OUT,         0xFFFF, 0xFFFF,     /* Min local flush timeout - infinite */
                                    0xFFFF, 0xFFFF,     /* Max local flush timeout - infinite */
    L2CAP_AUTOPT_TERMINATOR                             /* END */
};

/* SWAT service search request */
static const uint8 swat_service_request[] =
{
    0x35, 0x11,         /* "Data element sequence" (Sequence length = 0x11) */
    0x1c,               /* (0x1c) Translates to: "UUID" of length 16 bytes */
    /* The SWAT UUID: 0x00001000_d10211e1_9b230002_5b00a5a5 */
    0x00,0x00,0x10,0x00,
    0xd1,0x02,0x11,0xe1,
    0x9b,0x23,0x00,0x02,
    0x5b,0x00,0xa5,0xa5,
};

/* SWAT signalling PSM request */
static const uint8 swat_sig_protocol_attribute_request[]=
{
    /* Protocol Descriptor List */
    0x35, 0x03,         /* "Data element sequence" (Sequence length 0x03) */
    0x09, 0x00, 0x04,   /* */
};

/* SWAT media PSM request */
static const uint8 swat_media_protocol_attribute_request[]=
{
    /* Protocol Descriptor List */
    0x35, 0x03,         /* "Data element sequence" (Sequence length 0x03) */
    0x09, 0x00, 0x0d,   /* */
};

/****************************************************************************
FUNCTION: 
    handleSyncRegisterCfm

DESCRIPTION:
    Private function handle the Sync register confirm message

NOTES:
    Will send the SWAT_INIT_CFM message to the client to indicate SWAT lib
    has finished initialisation.
*/
static void handleSyncRegisterCfm(CL_DM_SYNC_CONNECT_CFM_T * cfm)
{
    swatSendInitCfmToClient((cfm->status == success) ? swat_success : swat_l2cap_fail);
}


/****************************************************************************
FUNCTION: 
    swatRegisterSignallingL2cap

DESCRIPTION:
    Private function to request connection library registers the Signalling PSM.
*/
static void swatRegisterSignallingL2cap(void)
{
    ConnectionL2capRegisterRequest(&swat->l2cap_task, SWAT_SIGNALLING_PSM, 0);
}


/****************************************************************************
FUNCTION:
    swatRegisterMediaL2cap

DESCRIPTION:
    Private function to request connection library registers the Media PSM.
*/
static void swatRegisterMediaL2cap(void)
{
    ConnectionL2capRegisterRequest(&swat->l2cap_task, SWAT_MEDIA_PSM, 0);
}


/****************************************************************************
FUNCTION:
    handleL2capRegisterCfm
    
DESCRIPTION:
    Private function to handle the L2CAP registration CFM message.

NOTES:
    If registration is for the signalling channel and was successful, request 
    to register the media channel.
    If registration is for the media channel and was successful, request to 
    register the eSCO.
    If either L2CAP registration failed, SWAT_INIT_CFM will be sent to the
    client task.
*/
static void handleL2capRegisterCfm(const CL_L2CAP_REGISTER_CFM_T *cfm)
{
    SWAT_DEBUG(("[SWAT] CL_L2CAP_REGISTER_CFM PSM[%x] STATUS[%x]\n", cfm->psm, cfm->status));
    
    if (cfm->psm == SWAT_SIGNALLING_PSM)
    {
        if (cfm->status == success) 
        {
            swatRegisterMediaL2cap(); 
        }
        else
        {
            swatSendInitCfmToClient(swat_l2cap_fail);
        }
    }
    else if (cfm->psm == SWAT_MEDIA_PSM)
    {
        if (cfm->status == success)
        {
            ConnectionSyncRegister(&swat->l2cap_task);
        }
        else
        {
            swatSendInitCfmToClient(swat_l2cap_fail);
        }
    }
    else
    {
        swatSendInitCfmToClient(swat_unknown_psm);
    }
}


/****************************************************************************
FUNCTION:
    handleSdpRegisterCfm
    
DESCRIPTION:
    Private function to hand the SDP registration CFM message.
    If SDP registration was successful, register the SWAT L2CAP channels.
*/
static void handleSdpRegisterCfm(const CL_SDP_REGISTER_CFM_T *cfm)
{
    SWAT_DEBUG(("[SWAT] CL_SDP_REGISTER_CFM STATUS[%x]\n", cfm->status));
    
    if (cfm->status == sds_status_success)
    {
        swatRegisterSignallingL2cap();
    }
    else
    {
        swatSendInitCfmToClient(swat_sdp_fail);
    }
}


/****************************************************************************
NAME
    swatL2capHandleIncomingSignallingRequest

PARAMETERS
    bd_addr : Bluetooth address of remote device requesting connection
    connection_id: Unique signal identifier for the connection attempt
    identifier: The channel identifier

DESCRIPTION
    Handle an incoming L2CAP connection request for a SWAT signalling channel
*/
static void swatL2capHandleIncomingSignallingRequest(bdaddr bd_addr, uint16 connection_id, uint8 identifier)
{
    uint8 device_id = swatAddDevice((const bdaddr *)&bd_addr);
    
    /* Check the device is allowed to connect (max devices not reached) */
    if (device_id != ERROR_MAX_DEVICES)
    {
        remoteDevice * device = &swat->remote_devs[device_id];
        
        /* Are we already in the process of requesting a signalling connection? (Crossover connection) */
        if (device->signalling_state == swat_signalling_local_connecting)
        {
            /* We are SOURCE device so reject the connection request (SWAT Specification defines this) */
            if (swat->role == swat_role_source)
            {
                SWAT_DEBUG(("[SWAT] X-Over signalling : Reject incoming signalling request (our outgoing should be accepted)\n"));
                swatL2capSignallingConnectResponse(device_id, connection_id, identifier, FALSE);
                
                /* Wait for the L2CAP_CONNECT_CFM message in response to the rejection we just issued for the incoming signalling connection */
                swatSetSignallingState(device, swat_signalling_local_connecting_xover);
            }
            /* We are SINK device so we have to accept the connection request (SWAT Specification defines this) */
            else
            {
                SWAT_DEBUG(("[SWAT] X-Over signalling : Accept incoming signalling request (our outgoing should be rejected)\n"));
                /* Change state as our connection request should be declined by the SOURCE device */
                swatSetSignallingState(device, swat_signalling_remote_connecting);
                
                /* Accept the connection request as ours should be rejected by the SOURCE device */
                swatL2capSignallingConnectResponse(device_id, connection_id, identifier, TRUE);
            }
        }
        /* Always accept connection requests when in the idle state */
        else if (device->signalling_state == swat_signalling_idle)
        {
            /* Set state whilst the connection process is ongoing */
            swatSetSignallingState(device, swat_signalling_remote_connecting);
            
            /* Should library accept request or leave for client task to decide? */
            if (swat->auto_handle)
            {
                SWAT_DEBUG(("[SWAT] Auto acept signalling connection request from device ID[%x]\n", device_id));
                swatL2capSignallingConnectResponse(device_id, connection_id, identifier, TRUE);
            }
            else
            {
                /* Send IND to client and let them decide what to do */
                swatSendSignallingConnectIndToClient(device_id, connection_id, identifier, bd_addr);
            }
        }
        else
        {
            SWAT_DEBUG(("[SWAT] Rject signalling request from device ID[%x] signalling channel in wrong STATE[%x]\n", device_id, device->signalling_state));
            swatL2capSignallingConnectResponse(device_id, connection_id, identifier, FALSE);
        }
    }
    else
    {
        swatSendSignallingConnectCfmToClient(swat_max_connections, 0, 0);
    }
}


/****************************************************************************
NAME
    swatL2capHandleIncomingMediaRequest

PARAMETERS
    bd_addr : Bluetooth address of remote device requesting connection
    connection_id: Unique signal identifier for the connection attempt
    identifier: The channel identifier

DESCRIPTION
    Handle an incoming L2CAP connection request for a SWAT media channel
*/
static void swatL2capHandleIncomingMediaRequest(bdaddr bd_addr, uint16 connection_id, uint8 identifier)
{
    remoteDevice * device = swatFindDeviceFromBdaddr((const bdaddr *) &bd_addr);
    
    if (device)
    {
        /* Only accept the incoming L2CAP request for a media channel if the media channel is in opening state */
        if (device->media_state == swat_media_opening)
        {
            /* Only accept the incoming L2CAP request for a media channel if the transport is in idle state */
            if (device->transport_state == swat_transport_idle)
            {
                SWAT_DEBUG(("[SWAT] Accept incoming media request from device ID[%x]\n", device->id));
                device->transport_state = swat_transport_incoming;
                ConnectionL2capConnectResponse(&swat->l2cap_task, TRUE, SWAT_MEDIA_PSM, connection_id, identifier, sizeof(swat_media_conftab), (uint16 *)swat_media_conftab);
            }
            else
            {
                SWAT_DEBUG(("[SWAT] Reject incoming media request from device ID[%x] Transport in wrong STATE[%x]\n", device->id, device->transport_state));
                ConnectionL2capConnectResponse(&swat->l2cap_task, FALSE, SWAT_MEDIA_PSM, connection_id, identifier, sizeof(swat_media_conftab), (uint16 *)swat_media_conftab);
            }
        }
        else
        {
            SWAT_DEBUG(("[SWAT] Reject incoming media request from device ID[%x] Media channel in wrong STATE[%x] should be STATE[%x]\n", device->id, device->media_state, swat_media_opening));
            ConnectionL2capConnectResponse(&swat->l2cap_task, FALSE, SWAT_MEDIA_PSM, connection_id, identifier, sizeof(swat_media_conftab), (uint16 *)swat_media_conftab);
        }
    }
    else
    {
        SWAT_DEBUG(("[SWAT] LIBRARY ERROR : Device is null\n"));
    }
}

/****************************************************************************
FUNCTION:
    handleL2capConnectInd
    
DESCRIPTION:
    Private function to handle the L2CAP Connect Indication message
*/
static void handleL2capConnectInd(const CL_L2CAP_CONNECT_IND_T * ind)
{
    SWAT_DEBUG(("[SWAT] CL_L2CAP_CONNECT_IND PSM[%x]\n",ind->psm));
    
    /* Which PSM was the message recieved on? */
    if (ind->psm == SWAT_SIGNALLING_PSM)
    {
        swatL2capHandleIncomingSignallingRequest(ind->bd_addr, ind->connection_id, ind->identifier);
    }
    else if (ind->psm == SWAT_MEDIA_PSM)
    {
        swatL2capHandleIncomingMediaRequest(ind->bd_addr, ind->connection_id, ind->identifier);
    }
    else
    {
        /* Should never get here */
        SWAT_DEBUG(("[SWAT] Unknown PSM connection on PSM [%x]\n", ind->psm));
    }
}


/****************************************************************************
NAME
    swatL2capHandleL2capCfmSignalling

PARAMETERS
    status - The status parameter from the CFM message
    sink - The sink for the transport channel
    device - The remote device for the connection

DESCRIPTION
    Handle when a signalling channel has connected / failed to connect
*/
static void handleL2capConnectCfmSignalling(l2cap_connect_status status, Sink sink, remoteDevice * device)
{
    if (status == l2cap_connect_success)
    {
        swatSetSignallingState(device, swat_signalling_connected);
        device->signalling_sink = sink;
        
        SWAT_DEBUG(("[SWAT] Device ID[%x] : Signalling connected\n", device->id));
        swatSendSignallingConnectCfmToClient(swat_success, device->id, sink);
    }
    /* Was the connection request rejected due to a X-Over connection? */
    else if ((status == l2cap_connect_failed_remote_reject) && (device->signalling_state == swat_signalling_remote_connecting) )
    {
        SWAT_DEBUG(("[SWAT] Remote device ID[%x] rejected our connection request\n", device->id));
    }
    /* Connection failed for some reason that was not expected */
    else
    {
        /* If this message was sent whilst already connected, ignore it */
        if (device->signalling_sink)
        {
            return;
        }
        
        /* Is this indication for the incoming signalling request we rejected for X-Over connections? */
        if (device->signalling_state == swat_signalling_local_connecting_xover)
        {
            SWAT_DEBUG(("[SWAT] X-Over signalling : Incoming signalling request rejected\n"));
            
            /* No longer in X-Over connection state as the remote devices connection has been rejected so wait for our request to be accepted */
            swatSetSignallingState(device, swat_signalling_local_connecting);
            return;
        }
        else
        {
            SWAT_DEBUG(("[SWAT] Device ID[%x] : Signalling connection error [%x]\n", device->id, status));
            swatSendSignallingConnectCfmToClient(swat_l2cap_error, device->id, sink);
        }
        
        /* Remove the device as it's no longer valid */
        swatRemoveDevice(device->id);
    }
}


/****************************************************************************
NAME
    swatL2capHandleL2capCfmMedia

PARAMETERS
    status - The status parameter from the CFM message
    sink - The sink for the transport channel
    device - The remote device for the connection

DESCRIPTION
    Handle when a media channel has connected / failed to connect
*/
static void handleL2capConnectCfmMedia(l2cap_connect_status status, Sink sink, remoteDevice * device)
{
    if (status == l2cap_connect_success)
    {
        SWAT_DEBUG(("[SWAT] Device ID[%x] : Media connected (standard latency)\n", device->id));
        swatSetMediaState(device, SWAT_MEDIA_STANDARD, swat_media_open);
        device->transport_state = swat_transport_connected;
        device->media_sink = sink;
        
        /* Ensure any media data sent over the channel is ignored */
        StreamConnectDispose(StreamSourceFromSink(sink));
        
        /* Don't send CFM message to client if going to disconnect immediatley */
        if (!device->disconnecting_signalling)
        {
            swatSendMediaOpenCfmToClient(swat_success, device->id, SWAT_MEDIA_STANDARD, sink);
        }
    }
    else
    {
        SWAT_DEBUG(("[SWAT] Device ID[%x] : Media (standard latency) connection error STATUS[%x]\n", device->id, status));
        swatSetMediaState(device, SWAT_MEDIA_STANDARD, swat_media_closed);
        device->transport_state = swat_transport_idle;
        
        /* Don't send CFM message to client if going to disconnect immediatley */
        if (!device->disconnecting_signalling)
        {
            swatSendMediaOpenCfmToClient(swat_l2cap_error, device->id, SWAT_MEDIA_STANDARD, 0);
        }
    }
    
    /* Was this connection started before signalling disconnection process? */
    if (device->disconnecting_signalling)
    {
        swatHandleL2capSignallingDisconnectReq(device->id);
    }
    
    /* Command has been handled; can now release command block to process next queued command (if one exists) */
    SWAT_DEBUG(("[SWAT] SWAT_OPEN_CMD Handled\n"));
    device->signalling_block = 0;
}


/****************************************************************************
FUNCTION:
    handleL2capConnectCfm
    
DESCRIPTION:
    Private function to handle the L2CAP Connect Confirmation message
*/
static void handleL2capConnectCfm(const CL_L2CAP_CONNECT_CFM_T * cfm)
{
    remoteDevice * device = swatFindDeviceFromBdaddr((const bdaddr *)&cfm->addr);
    
    if (device)
    {
        if (cfm->status == l2cap_connect_pending)
        {
            SWAT_DEBUG(("[SWAT] L2CAP Connection pending on PSM[%x]\n", cfm->psm_local));
        }
        else
        {
            /* Which PSM was the message recieved on? */
            if (cfm->psm_local == SWAT_SIGNALLING_PSM)
            {
                handleL2capConnectCfmSignalling(cfm->status, cfm->sink, device);
            }
            else if (cfm->psm_local == SWAT_MEDIA_PSM)
            {
                handleL2capConnectCfmMedia(cfm->status, cfm->sink, device);
            }
            else
            {
                /* Should never get here */
                SWAT_DEBUG(("[SWAT] CL_L2CAP_CONNECT_CFM : Unrecognised PSM [%x]\n", cfm->psm_local));
            }
        }
    }
    else
    {
        SWAT_DEBUG(("[SWAT] LIBRARY ERROR : CL_L2CAP_CONNECT_CFM - Device is NULL\n"));
    }
}


/****************************************************************************
FUNCTION:
    handleSignallingDisconnectInd
    
DESCRIPTION:
    Helper function for handleL2capDisconnectInd (for signalling disconnect IND)
*/
static void handleSignallingDisconnectInd(CL_L2CAP_DISCONNECT_IND_T * ind, remoteDevice * device)
{
    SWAT_DEBUG(("[SWAT] CL_L2CAP_DISCONNECT_IND_T (Signalling) Device ID[%x]\n", device->id));
    
    if (ind->status != l2cap_disconnect_link_loss)
    {
        /* Inform the app the signalling has successfully disconnected */
        
        /* TODO : THIS SHOULD BE AN IND NOT A CFM MESSAGE */
        swatSendSignallingDisconnectCfmToClient(swat_success, device->id);
    }
    else
    {
        /* Inform app the signalling has disconnected due to a link loss */
        
        /* TODO : THIS SHOULD BE AN IND NOT A CFM MESSAGE */
        swatSendSignallingDisconnectCfmToClient(swat_disconnect_link_loss, device->id);
    }
    
    /* Remove the device as it's no longer valid */
    swatRemoveDevice(device->id);
}


/****************************************************************************
FUNCTION:
    handleMediaDisconnectInd
    
DESCRIPTION:
    Helper function for handleL2capDisconnectInd (for media disconnect IND)
*/
static void handleMediaDisconnectInd(CL_L2CAP_DISCONNECT_IND_T * ind, remoteDevice * device)
{
    SWAT_DEBUG(("[SWAT] CL_L2CAP_DISCONNECT_IND_T (Media) Device ID[%x]\n", device->id));
    
    device->media_sink = NULL;
    swatSetMediaState(device, SWAT_MEDIA_STANDARD, swat_media_closed);
    device->transport_state = swat_transport_idle;
    
    /* Inform client task that the remote device is closing the media channel */
    swatSendMediaCloseIndToClient(device->id, SWAT_MEDIA_STANDARD);
    
    /* CLOSE_CMD has now completed, release block to action next command (if one exists) */
    SWAT_DEBUG(("[SWAT] SWAT_CLOSE_CMD Handled\n"));
    device->signalling_block = 0;
    
    /* Is this disconnection required as part of signalling disconnection process? */
    if (device->disconnecting_signalling)
    {
        /* Disconnect the signalling channel */
        swatHandleL2capSignallingDisconnectReq(device->id);
    }
}


/****************************************************************************
FUNCTION:
    handleL2capDisconnectInd
    
DESCRIPTION:
    Private function to handle the L2CAP Disconnect Indication message
*/
static void handleL2capDisconnectInd(CL_L2CAP_DISCONNECT_IND_T * ind)
{
    remoteDevice * device = swatFindLocalDeviceFromSink(ind->sink);
    
    /* Respond to the L2CAP_DISCONNECT_IND message (The L2CAP channel is gone from this point on - and can't be stopped) */
    ConnectionL2capDisconnectResponse(ind->identifier, ind->sink);
    
    if (device)
    {
        /* Was it signalling or media that disconnected? Handle appropriately */
        if (device->signalling_sink == ind->sink)
        {
            handleSignallingDisconnectInd(ind, device);
        }
        else if (device->media_sink == ind->sink)
        {
            handleMediaDisconnectInd(ind, device);
        }
    }
    else
    {
        SWAT_DEBUG(("[SWAT] LIBRARY ERROR : Should not get L2CAP_DISCONNECT_IND for unknown device\n"));
    }
}


/****************************************************************************
FUNCTION:
    handleL2capDisconnectCfm
    
DESCRIPTION:
    Private function to handle the L2CAP Disconnect Confirmation message
*/
static void handleL2capDisconnectCfm(const CL_L2CAP_DISCONNECT_CFM_T * cfm)
{
    remoteDevice * device = swatFindLocalDeviceFromSink(cfm->sink);
    
    if (device)
    {
        /* Which connection is the L2CAP disconnect for? */
        if (cfm->sink == device->signalling_sink)
        {
            SWAT_DEBUG(("[SWAT] CL_L2CAP_DISCONNECT_CFM for signalling channel to device ID[%x] STATUS[%x]\n", device->id, cfm->status));
            swatSendSignallingDisconnectCfmToClient(swat_success, device->id);
            /* All SWAT channels to device are now closed so remove the device */
            swatRemoveDevice(device->id);
        }
        else if (cfm->sink == device->media_sink)
        {
            SWAT_DEBUG(("[SWAT] CL_L2CAP_DISCONNECT_CFM for media channel to device ID[%x] STATUS[%x]\n", device->id, cfm->status));
            swatSetMediaState(device, SWAT_MEDIA_STANDARD, swat_media_closed);
            device->transport_state = swat_transport_idle;
            device->media_sink = NULL;
            swatSendMediaCloseCfmToClient(swat_success, device->id, SWAT_MEDIA_STANDARD);
            
            /* Is this disconnection required as part of signalling disconnection process? */
            if (device->disconnecting_signalling)
            {
                /* Disconnect the signalling channel */
                swatHandleL2capSignallingDisconnectReq(device->id);
            }
        }
        else
        {
            /* Should never get here */
            SWAT_DEBUG(("[SWAT] SWAT LIBRARY ERROR : CL_L2CAP_DISCONNECT_CFM - Unrecognised SINK\n"));
        }
    }
    else
    {
        SWAT_DEBUG(("[SWAT] LIBRARY ERROR : Should not get CL_L2CAP_DISCONNECT_CFM for unknown device\n"));
    }
}

/****************************************************************************
FUNCTION:
    swatGetL2capPSM
    
DESCRIPTION:
    Find L2CAP PSM in the SDP record.
*/
static bool swatGetL2capPSM(const uint8 *begin, const uint8 *end, uint16 *psm, uint16 id)
{
    ServiceDataType type;
    Region record, protocols, protocol, value;
    record.begin = begin;
    record.end   = end;

    while(ServiceFindAttribute(&record, id, &type, &protocols))
        if(type == sdtSequence)
            while(ServiceGetValue(&protocols, &type, &protocol))
            if(type == sdtSequence
               && ServiceGetValue(&protocol, &type, &value)
               && type == sdtUUID
               && RegionMatchesUUID32(&value, (uint32) 0x0100)
               && ServiceGetValue(&protocol, &type, &value)
               && type == sdtUnsignedInteger)
            {
                *psm = (uint16) RegionReadUnsigned(&value);
                return TRUE;
            }

    return FALSE;
}

/****************************************************************************
FUNCTION:
    swatHandleClSdpServiceSearchAttributeCfm
    
DESCRIPTION:
    Handle an SDP search for the remote PSM of the SWAT signalling or media
    channel.

    Request a connection to setup the signalling or media channel if
    successful.
*/
static void swatHandleClSdpServiceSearchAttributeCfm(CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *cfm)
{
    remoteDevice *device = swatFindDeviceFromBdaddr(&cfm->bd_addr);
    uint16 psm = 0;
    bool success = FALSE;

    /* if we haven't got a valid device, there's nothing to do, regardless of the SDP
     * search result. Without a remoteDevice we also don't know which type of search we
     * were doing,in order to send a sensible error message to the application. The *current*
     * implementation is such that if this has happened, the application has already been
     * informed, just exit, it's all we can do.
     */
    if (!device)
    {
        return;
    }

    /* was the search a success? */
    if (cfm->status == sdp_response_success)
    {
        /* we're searching for a signalling channel PSM */
        if (device->psm_search_type == swat_psm_search_sig)
        {
            /* are we still in a valid state to continue initiating this connection? */
            if ((device->signalling_state == swat_signalling_local_connecting) ||
                    (device->signalling_state == swat_signalling_local_connecting_xover))
            {
                /* find the PSM in the returned attributes */
                if (swatGetL2capPSM(cfm->attributes, cfm->attributes+cfm->size_attributes, &psm, saProtocolDescriptorList))
                {
                    SWAT_DEBUG(("[SWAT] SIG remote PSM[%x] ConnectReq\n", psm));

                    /* request connection on PSM */
                    ConnectionL2capConnectRequest(&swat->l2cap_task, &cfm->bd_addr, SWAT_SIGNALLING_PSM, psm, sizeof(swat_signalling_conftab), (uint16*)swat_signalling_conftab);
                    
                    /* send a L2CAP connect request, make sure we don't generate an error below */
                    success = TRUE;
                }
            }
        }
        /* we're searching for a signalling channel PSM */
        else if (device->psm_search_type == swat_psm_search_media)
        {
            /* check for correct state to open media channel? */
            if (device->transport_state == swat_transport_idle)
            {
                /* find the PSM in the returned attributes */
                if (swatGetL2capPSM(cfm->attributes, cfm->attributes+cfm->size_attributes, &psm, 0x000d))
                {
                    SWAT_DEBUG(("[SWAT] MEDIA remote PSM[%x] ConnectReq\n", psm));

                    /* Send request to connect the L2CAP media channel */
                    device->transport_state = swat_transport_outgoing;
                    ConnectionL2capConnectRequest(&swat->l2cap_task, &device->bd_addr, SWAT_MEDIA_PSM, psm, sizeof(swat_media_conftab), (uint16*)swat_media_conftab);
                    
                    /* send a L2CAP connect request, make sure we don't generate an error below */
                    success = TRUE;
                }
            }
        }
    }

    /* if we didn't get to send a L2CAP connect request above, generate an error message to the
     * application, either a signalling connect cfm error or a media open cfm error */
    if (!success)
    {
        if (device->psm_search_type == swat_psm_search_sig)
        {
            swatSendSignallingConnectCfmToClient(swat_l2cap_error, device->id, 0);
            
            /* failued to connect signalling, need to clean up the remote device structure
             * so that a reconnect attempt by the app works */
            swatRemoveDevice(device->id);
        }
        else
        {
            swatSendMediaOpenCfmToClient(swat_l2cap_error, device->id, SWAT_MEDIA_STANDARD, 0);
        }
    }
}

/*****************************************************************************/
static void swatSendL2capSignallingConnectRequest(bdaddr addr)
{
    remoteDevice *device = swatFindDeviceFromBdaddr(&addr);

    /* belt'n'braces check, at the moment there isn't a code path that
     * can make this device structure disappear before we get here */
    if (device)
    {
        SWAT_DEBUG(("[SWAT] SDP search for remote signalling channel PSM\n"));

        /* remember we're searching for a signalling channel psm */
        device->psm_search_type = swat_psm_search_sig;

        /* find out what the remote signalling channel PSM is */
        ConnectionSdpServiceSearchAttributeRequest(&swat->l2cap_task, &addr, 32, sizeof(swat_service_request), (uint8*)swat_service_request, sizeof(swat_sig_protocol_attribute_request), swat_sig_protocol_attribute_request);
    }
}


/*****************************************************************************/
void swatHandleL2capSignallingConnectReq(const SWAT_INTERNAL_SIGNALLING_CONNECT_REQ_T *req)
{
    /* Is connection request to a device we already know? */
    remoteDevice *device = swatFindDeviceFromBdaddr(&req->addr);
    
    /* If the device is a known device, ensure connection request is allowed */
    if (device != NULL)
    {
        switch(device->signalling_state)
        {
            case swat_signalling_idle:
            {
                /* Make an outgoing connection request */
                swatSendL2capSignallingConnectRequest(req->addr);

                /* Update the signalling state for the device */
                swatSetSignallingState(device, swat_signalling_local_connecting);
            }
            break;
            case swat_signalling_remote_connecting:
            case swat_signalling_local_connecting:
            case swat_signalling_local_connecting_xover:
            {
                /* Signalling is already in the process of connecting */
                swatSendSignallingConnectCfmToClient(swat_signalling_already_connecting, device->id, 0);
            }
            break;
            case swat_signalling_connected:
            {
                /* Signalling is already connected */
                swatSendSignallingConnectCfmToClient(swat_signalling_already_connected, device->id, 0);
            }
            break;
            case swat_signalling_disconnecting:
            {
                /* Signalling is disconnecting (Could implement action to connect after disconnect) */
                swatSendSignallingConnectCfmToClient(swat_signalling_is_disconnecting, device->id, 0);
            }
            break;
        }
    }
    /* Make an outgoing request to a new device */
    else
    {
        uint16 d_id = swatAddDevice(&req->addr);
        
        if (d_id != ERROR_MAX_DEVICES)
        {
            /* Able to support a new connection */
            swatSendL2capSignallingConnectRequest(req->addr);
            
            /* Update the signalling state for the device */
            swatSetSignallingState(&swat->remote_devs[d_id], swat_signalling_local_connecting);
        }
        else
        {
            SWAT_DEBUG(("[SWAT] Cannot support a connection to a new device\n"));
        }
    }
}


/****************************************************************************/
void swatL2capSignallingConnectResponse(uint16 device_id, uint16 connection_id, uint8 identifier, bool accept)
{
    /* If rejecting the signalling connection, also remove the device */
    /* Don't remove the device if we are in local_connecting state as means it's the crossover connection from the SINK device that's being rejected */
    if ( !accept && (swat->remote_devs[device_id].signalling_state != swat_signalling_local_connecting) )
    {
        /* Remove the device */
        swatRemoveDevice(device_id);
    }
    
    ConnectionL2capConnectResponse(&swat->l2cap_task, accept, SWAT_SIGNALLING_PSM, connection_id, identifier, sizeof(swat_signalling_conftab), (uint16 *)swat_signalling_conftab);
}


/*****************************************************************************/
void swatHandleL2capSignallingDisconnectReq(uint16 device_id)
{
    remoteDevice * device = &swat->remote_devs[device_id];
    
    if (device)
    {
        /* Ensure connection is in correct state to disconnect */
        switch(device->signalling_state)
        {
            case swat_signalling_idle:
            {
                /* Shouldn't ever get here as the device should be removed once disconnected so remove device */
                swatRemoveDevice(device->id);
                swatSendSignallingDisconnectCfmToClient(swat_wrong_state, 0);
            }
            break;
            case swat_signalling_remote_connecting:
            case swat_signalling_local_connecting:
            case swat_signalling_local_connecting_xover:
            {
                /* TODO : Disconnect once connect has completed */
                swatSendSignallingDisconnectCfmToClient(swat_wrong_state, 0);
            }
            break;
            case swat_signalling_connected:
            case swat_signalling_disconnecting:
            {
                device->disconnecting_signalling = 1; /* Set the flag as there may be media channel(s) that need disconnecting */
                
                /* Update the signalling state for the device */
                swatSetSignallingState(device, swat_signalling_disconnecting);
                
                /* Check if there are any open media channels */
                if ((device->media_state != swat_media_closed) && swatL2capMediaCloseReq(device))
                {
                    SWAT_DEBUG(("[SWAT] (Signalling disconnect) Disconnect standard media channel to device ID[%x]\n", device->id));
                    return;
                }
                else if ((device->media_ll_state != swat_media_closed) && swatRequestEscoDisconnect(device))
                {
                    SWAT_DEBUG(("[SWAT] (Signalling disconnect) Disconnect low latency media channel to device ID[%x]\n", device->id));
                    return;
                }            
                else
                {
                    /* Neither media channels are open, so just disconnect the signalling channel */
                    SWAT_DEBUG(("[SWAT] Disconnect signalling channel to device ID[%x]\n", device->id));
                    ConnectionL2capDisconnectRequest(&swat->l2cap_task, device->signalling_sink);
                    /* No open media channels, so reset the flag and wait for the L2CAP_DISCONNECT_CFM message before removing device */
                    device->disconnecting_signalling = FALSE;
                }
            }
            break;
        }
    }
    else
    {
        swatSendSignallingDisconnectCfmToClient(swat_bad_data, 0);
    }
}


/*****************************************************************************/
void swatL2capMediaConnectReq(remoteDevice * device)
{
    if (device)
    {
        SWAT_DEBUG(("[SWAT] SDP search for remote media channel PSM\n"));

        /* remember we're searching for a signalling channel psm */
        device->psm_search_type = swat_psm_search_media;

        /* find out what the remote media channel PSM is */
        ConnectionSdpServiceSearchAttributeRequest(&swat->l2cap_task, &device->bd_addr, 32, sizeof(swat_service_request), (uint8*)swat_service_request, sizeof(swat_media_protocol_attribute_request), swat_media_protocol_attribute_request);
    }
}


/*****************************************************************************/
bool swatL2capMediaCloseReq(remoteDevice * device)
{
    bool retval = FALSE;
    
    /* Check the state of the L2CAP channel before disconnecting */
    switch(device->transport_state)
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
            retval = TRUE;    /* Wait for connect_CFM message on media channel before closing */
        }
        break;
        case swat_transport_connected:
        {
            /* Start the L2CAP disconnection process */
            device->transport_state = swat_transport_closing;
            ConnectionL2capDisconnectRequest(&swat->l2cap_task, device->media_sink);
            retval = TRUE;    /* Wait for disconnect_CFM message on media channel */
        }
        break;
    }
    
    return retval;
}


/*****************************************************************************/
void swatHandleUnresponsiveMediaStd(uint16 device_id)
{
    SWAT_DEBUG(("[SWAT] Device ID[%x] did not respond to the last SWAT command [%x]\n", device_id, swat->remote_devs[device_id].media_state));
    
    /* What state is the media channel in? */
    switch(swat->remote_devs[device_id].media_state)
    {
        case swat_media_opening:
        {
            /* Remote device failed to respond to an OPEN request */
            swatSendMediaOpenCfmToClient(swat_signalling_no_response, device_id, SWAT_MEDIA_STANDARD, NULL);
            
            /* Revert media state as it failed to open */
            swatSetMediaState(&swat->remote_devs[device_id], SWAT_MEDIA_STANDARD, swat_media_closed);
        }
        break;
        case swat_media_starting:
        {
            /* Remote device failed to respond to a START request*/
            swatSendMediaStartCfmToClient(swat_signalling_no_response, device_id, SWAT_MEDIA_STANDARD);
            
            /* Revert media state as it failed to open */
            swatSetMediaState(&swat->remote_devs[device_id], SWAT_MEDIA_STANDARD, swat_media_open);
        }
        break;
        case swat_media_suspending:
        {
            /* Remote device failed to respond to a SUSPEND request */
            swatSendMediaSuspendCfmToClient(swat_signalling_no_response, device_id, SWAT_MEDIA_STANDARD);
            
            /* Revert media state as it failed to streaming */
            swatSetMediaState(&swat->remote_devs[device_id], SWAT_MEDIA_STANDARD, swat_media_streaming);
        }
        break;
        case swat_media_closing:
        {
            /* Remote device failed to respond to a CLOSE request */
            swatSendMediaCloseCfmToClient(swat_signalling_no_response, device_id, SWAT_MEDIA_STANDARD);
            
            /* Revert media state as it failed to open */
            swatSetMediaState(&swat->remote_devs[device_id], SWAT_MEDIA_STANDARD, swat_media_open);
        }
        break;
        
        /* Media channel shouldn't become unresponsive in these stable states */
        case swat_media_streaming:
        case swat_media_closed:
        case swat_media_open:
        {
            SWAT_DEBUG(("[SWAT] Library error : unresponsive low latency media channel in media STATE[%x]\n", swat->remote_devs[device_id].media_state));
            return;
        }
        break;
        
        default:
        {
            /* handle invalid type for media_state quietly */
            SWAT_DEBUG(("[SWAT] swatHandleUnresponsiveMediaStd media_state:[%x]\n", swat->remote_devs[device_id].media_state));
        }
        break;
    }
}


/*****************************************************************************/
void swatL2capHandler(Task task, MessageId id, Message message)
{
    switch(id)
    {
        /* L2CAP messages */
        case CL_SDP_REGISTER_CFM:
        {
            handleSdpRegisterCfm((CL_SDP_REGISTER_CFM_T *) message);
        }
        break;
        case CL_L2CAP_REGISTER_CFM:
        {
            handleL2capRegisterCfm((CL_L2CAP_REGISTER_CFM_T *)message);
        }
        break;
        case CL_L2CAP_CONNECT_IND:
        {
            handleL2capConnectInd((CL_L2CAP_CONNECT_IND_T *)message);
        }
        break;
        case CL_L2CAP_CONNECT_CFM:
        {
            handleL2capConnectCfm((CL_L2CAP_CONNECT_CFM_T *)message);
        }
        break;
        case CL_L2CAP_TIMEOUT_IND:
        {
            /* Don't need to handle this message as the CL library will send a CFM with failure status */
            SWAT_DEBUG(("[SWAT] CL_L2CAP_TIMEOUT_IND\n"));
        }
        break;
        case CL_L2CAP_DISCONNECT_IND:
        {
            handleL2capDisconnectInd((CL_L2CAP_DISCONNECT_IND_T *)message);
        }
        break;
        case CL_L2CAP_DISCONNECT_CFM:
        {
            handleL2capDisconnectCfm((CL_L2CAP_DISCONNECT_CFM_T *)message);
        }
        break;
        
        /* Messages for Synchronous Connections */
        case CL_DM_SYNC_REGISTER_CFM:
        {
            handleSyncRegisterCfm((CL_DM_SYNC_CONNECT_CFM_T *)message);
        }
        break;
        case CL_DM_SYNC_UNREGISTER_CFM:
        {
            SWAT_DEBUG(("[SWAT] CL_DM_SYNC_UNREGISTER_CFM\n"));
        }
        break;
        case CL_DM_SYNC_CONNECT_CFM:
        {
            swatHandleScoConnectCfm((CL_DM_SYNC_CONNECT_CFM_T *)message);
        }
        break;
        case CL_DM_SYNC_CONNECT_IND:
        {
            swatHandleIncomingScoRequest((CL_DM_SYNC_CONNECT_IND_T *)message);
        }
        break;
        case CL_DM_SYNC_DISCONNECT_IND:
        {
            swatHandleIncomingScoDisconnect( ((CL_DM_SYNC_DISCONNECT_IND_T *)message)->audio_sink, ((CL_DM_SYNC_DISCONNECT_IND_T *)message)->status );
        }
        break;
        case CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM:
        {
            swatHandleClSdpServiceSearchAttributeCfm((CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *)message);
        }
        break;
        default:
        {
            /* System messages */
            switch(id)
            {
                case MESSAGE_MORE_DATA:
                {
                    swatHandleSwatSignallingData(((MessageMoreData *)message)->source);
                }
                break;
                case MESSAGE_MORE_SPACE:
                case MESSAGE_SOURCE_EMPTY:
                case MESSAGE_STREAM_DISCONNECT:
                {
                    /* Ignore these messages */
                }
                break;
                default:
                {
                    SWAT_DEBUG(("[SWAT] L2CAP Handler unhandled Message, ID: [%x]\n", id));
                }
            }
        }
    }
}
