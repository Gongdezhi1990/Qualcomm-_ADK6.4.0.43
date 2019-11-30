/*
Copyright (c) 2009 - 2016 Qualcomm Technologies International, Ltd.

*/

/*!
@file
@ingroup sink_app
@brief
    Implementation for handling MAP Client library messages and functionality
*/

/****************************************************************************
    Header files
*/

#include <connection.h>
#include <hfp.h>
#include <print.h>
#include <panic.h>
#include <stdlib.h>
#include <bdaddr.h>
#include <stream.h>
#include <string.h>
#include <sink.h>
#include <source.h>

#ifdef ENABLE_MAPC

#include <mapc.h>
#include <sdp_parse.h>
#include <message.h>

#include "sink_mapc.h"
#include "sink_main_task.h"
#include "sink_malloc_debug.h"
#include "sink_events.h"
#include "sink_statemanager.h"

#ifdef DEBUG_MAPC
    #define MAPC_DEBUG(x) {printf x;}
#else
    #define MAPC_DEBUG(x)
#endif

#define MAPC_SESSION   Mas

/* Message Handler Prototypes */
static void handleMapcMnsStartCfm(const MAPC_MNS_START_CFM_T* pMsg);
static void handleMapcMnsShutdownCfm(const MAPC_MNS_SHUTDOWN_CFM_T* pMsg);
static void handleMapcMnsConnectInd(const MAPC_MNS_CONNECT_IND_T* pMsg);
static void handleMapcMnsConnectCfm(const MAPC_MNS_CONNECT_CFM_T* pMsg);
static void handleMapcMnsDisconnectInd(const MAPC_MNS_DISCONNECT_IND_T* pMsg);
static void handleMapcMnsSendEventInd(const MAPC_MNS_SEND_EVENT_IND_T* pMsg);
static void handleMapcMasConnectCfm(const MAPC_MAS_CONNECT_CFM_T* pMsg);
static void handleMapcMasDisconnectInd(const MAPC_MAS_DISCONNECT_IND_T* pMsg);
static void handleMapcMasSetNotificationCfm(const MAPC_MAS_SET_NOTIFICATION_CFM_T* pMsg);
static mapc_link_priority mapcAddDevice( const bdaddr *pAddr);
static bool handleMapcEventReport(const char* pEventReport, const uint16 vReportLen);

static const char mapcType[] = "NewMessage";

/* Prototype and task structure for MAP specific SDP searches */
static void handleSdpMessage (Task task, MessageId id, Message message);
static struct TaskData mapc_sdp_task = { handleSdpMessage };

/* Sink MAP global data */
typedef struct __sink_map_global_data_t
{
    mapcState       state[MAX_MAPC_CONNECTIONS];
    /* The SDP record handle of the MNS Service */
    uint32          sdpHandle;
    /* The Mns rfcomm Channel for the MNS Service */
    unsigned        mnsChannel:8;
}sink_map_global_data_t;

static sink_map_global_data_t map_gdata;

/****************************************************************************
NAME	
	mapcGetMapState
    
DESCRIPTION
    Get the MAP state for the id passed
    
PARAMS
    uint16 id
    
RETURNS
	mapcState*
*/
mapcState* mapcGetMapState(uint16 id)
{
    return &map_gdata.state[id];
}

/****************************************************************************
NAME	
	mapcGetSdpHandle
    
DESCRIPTION
    Get the MAP sdp record handle of the MNS Service
    
PARAMS
    void
    
RETURNS
	uint32
*/
uint32 mapcGetSdpHandle(void)
{
    return map_gdata.sdpHandle;
}

/****************************************************************************
NAME	
	mapcSetSdpHandle
    
DESCRIPTION
    Set the MAP sdp record handle of the MNS Service
    
PARAMS
    uint32 handle
    
RETURNS
	void
*/
void mapcSetSdpHandle(uint32 handle)
{
    map_gdata.sdpHandle = handle;
}

/****************************************************************************
NAME	
	mapcGetMnsChannel
    
DESCRIPTION
    Get the Mns rfcomm Channel for the MNS Service
    
PARAMS
    void
    
RETURNS
	uint8
*/
uint8 mapcGetMnsChannel(void)
{
    return map_gdata.mnsChannel;
}

/****************************************************************************
NAME	
	mapcSetMnsChannel
    
DESCRIPTION
    Set the Mns rfcomm Channel for the MNS Service
    
PARAMS
    uint8 channel
    
RETURNS
	void
*/
void mapcSetMnsChannel(uint8 channel)
{
    map_gdata.mnsChannel = channel;
}

/****************************************************************************
NAME	
	initMap

DESCRIPTION
    Register the Map Notification Server, called at boot time

PARAMS
    none

RETURNS
	void
*/
void initMap(void)
{
    /* Start the Message Notification Server, this only needs to be done once */
    MapcMnsStart(&theSink.task, TRUE, 0);
}

/****************************************************************************
NAME
	mapcShutDown

DESCRIPTION
    Unregister the Map Notification Server, called at shut down

PARAMS
    none

RETURNS
	void
*/
void mapcShutDown(void)
{
    MapcMnsShutdown( mapcGetSdpHandle(), mapcGetMnsChannel() );
}

/****************************************************************************
NAME
	mapcGetLinkFromBdAddr

DESCRIPTION
    Searches through any MAP connections looking for a match of bdaddr, if found
    returns the link associated with that bdaddr

PARAMS
    @pAddr

RETURNS
	mapc_link_priority
*/
static mapc_link_priority mapcGetLinkFromBdAddr(const bdaddr *pAddr)
{
    mapc_link_priority device_id = 0;

    /* search all possible map connections */
    for(device_id = 0; device_id < MAX_MAPC_CONNECTIONS; device_id ++)
    {
        mapcState *state = mapcGetMapState(device_id);
        /* if bdaddr matches that passed then return the appropriate link id */
        if(BdaddrIsSame(&state->bdAddr, pAddr))
            return(device_id);
    }
    /* no map connections matching the passed in bdaddr */
    return(mapc_invalid_link);
}

/****************************************************************************
NAME
	mapcGetLinkFromMapsSession

DESCRIPTION
    returns the device id associated with the session passed in

PARAMS
    @pAddr

RETURNS
	mapc_link_priority
*/
static mapc_link_priority mapcGetLinkFromMapsSession(MAPC_SESSION Session)
{
    mapc_link_priority device_id = 0;

    /* search available MAP connections */
    for(device_id = 0; device_id < MAX_MAPC_CONNECTIONS; device_id ++)
    {
        mapcState *state = mapcGetMapState(device_id);
        /* return device id if a session match is found */
        if(state->mnsHandle == (Mns)Session || state->masHandle == (Mas)Session )
            return(device_id);
    }
    /* no sessions were matched so return error */
    return(mapc_invalid_link);
}

/****************************************************************************
NAME
	mapcAddDevice
DESCRIPTION
    Add a new device for mapc mns service;

PARAMS
    @bdaddr

RETURNS
	mapc_link_priority
*/
static mapc_link_priority mapcAddDevice(const bdaddr *pAddr)
{
    mapc_link_priority priority = mapc_invalid_link;
    mapcState *state;

    /* check whether the device has been connected or connecting */
    priority = mapcGetLinkFromBdAddr(pAddr);

    /* If this device has not been added, add it first */
    if(priority == mapc_invalid_link)
    {
        uint8 device_id = 0;
        /* search for an unassigned map connection */
        for(device_id = 0; device_id < MAX_MAPC_CONNECTIONS; device_id++)
        {
            state = mapcGetMapState(device_id);
            /* if an unassigned map entry exists return link priority */
            if( state->device_state == mapc_state_idle )
            {
                priority = (mapc_link_priority)device_id;
                return priority;
            }
        }
    }
    /* this bdaddr already has a map connection associated with it, check its
       current state return error */
    else
    {
        /* check current state of link */
        state = mapcGetMapState(priority);

        /* if anything other than idle return error condition */
        if( state->device_state > mapc_state_idle )
        {
            /* This device has been connected or connecting, don't try to reconnect it */
            priority  = mapc_invalid_link;
        }
    }
    /* this connection already exists and is either in the process of connecting or
       is already connected, return its priority */
    return(priority);
}

/****************************************************************************
NAME
	handleServiceSearchAttributeCfm

DESCRIPTION
    confirmation of the service search for MAP support, if successful the
    app will progress and attempt to connect to the message access service

PARAMS
    @cfm message

RETURNS
	void
*/
static void handleServiceSearchAttributeCfm( const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *cfm)
{
    uint8 *rfcomm_channels;
    uint8 size_rfcomm_channels = 1;
    uint8 channels_found = 0;
    mapcState *state = NULL;

    mapc_link_priority device_id = mapcGetLinkFromBdAddr( &cfm->bd_addr );

    /* ensure device_id has been correctly retrieved */
    if(device_id != mapc_invalid_link)
        state = mapcGetMapState(device_id);

    /* if the service search has been successful, parse returned entries */
    if(cfm->status == sdp_response_success)
    {
        MAPC_DEBUG(("MAPC:\tReceived SDP Response of length %d\n", cfm->size_attributes));

        rfcomm_channels = mallocPanic(size_rfcomm_channels * sizeof(uint8));

        /* parse all returned reports */
        if (SdpParseGetMultipleRfcommServerChannels(
                            (uint8)cfm->size_attributes,
                            (const uint8*)cfm->attributes,
                            1,
                            &rfcomm_channels,
                            &channels_found) )
        {
            /* If receiving multiple responses, the first record will be stored in this application */
            if(state)
            {
                state->masChannel   = rfcomm_channels[0];
                state->bdAddr       = cfm->bd_addr;
                state->device_state = mapc_sdp_searched;

                /* set the link security requirements */
                ConnectionSmSetSdpSecurityOut(TRUE, &cfm->bd_addr);

                ConnectionSmRegisterOutgoingService(&theSink.task,
                                                    &cfm->bd_addr,
                                                    protocol_rfcomm,
                                                    state->masChannel,
                                                    sec4_out_level_2);

                /* attempt to connect the message access service */
                MapcMasConnectRequest( &theSink.task, &cfm->bd_addr, state->masChannel );
            }
        }
        /* no channels were found, there reset the state of this connection */
        else
        {
            if(state)
                state->device_state = mapc_state_idle;

            MAPC_DEBUG(("MAPC:NO Channels found\n"));
        }
        /* ensure memory used in sdp record parsing is free'd */
        freePanic(rfcomm_channels);
    }
    /* the sdp record search was not successful, no further action needs to be taken */
    else
    {
        /* reset current connection state so that it can be reused */
        if(state)
            state->device_state = mapc_state_idle;

        MAPC_DEBUG(("MAPC:SDP Search Failed. Status = %x, more = %x, error = %x \n", cfm->status, cfm->more_to_come, cfm->error_code));

    }
}


/****************************************************************************
NAME
	handleSdpMessage

DESCRIPTION
    Task function to handle responses to MAP specific SDP search requests

RETURNS
	void
*/
static void handleSdpMessage (Task task, MessageId id, Message message)
{
    UNUSED(task);

    switch (id)
    {
    case CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM:
        MAPC_DEBUG(("MAPC: CL_SM_GET_INDEXED_ATTRIBUTE_CFM[%d]\n" , ((CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T*)message)->status)) ;
        handleServiceSearchAttributeCfm( (const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T*) message);
        break;

    default:
        MAPC_DEBUG(("MAPC: Unexpected message, id=0x%X\n", id));
        break;
    }
}


/****************************************************************************
NAME
	handleMapcMessages

DESCRIPTION
    MAP Client Message Handler

PARAMS
    task        associated task
    pId         message id
    pMessage    message

RETURNS
	void
*/
void handleMapcMessages(Task task, MessageId pId, Message pMessage)
{
    UNUSED(task);

    switch(pId)
    {
    case MAPC_MNS_START_CFM:
        handleMapcMnsStartCfm((const MAPC_MNS_START_CFM_T*) pMessage);
        break;
    case MAPC_MNS_SHUTDOWN_CFM:
        handleMapcMnsShutdownCfm((const MAPC_MNS_SHUTDOWN_CFM_T*) pMessage);
        break;
    case MAPC_MNS_CONNECT_IND:
        handleMapcMnsConnectInd((const MAPC_MNS_CONNECT_IND_T*) pMessage);
        break;
    case MAPC_MNS_CONNECT_CFM:
        handleMapcMnsConnectCfm((const MAPC_MNS_CONNECT_CFM_T*) pMessage);
        break;
    case MAPC_MNS_DISCONNECT_IND:
        handleMapcMnsDisconnectInd((const MAPC_MNS_DISCONNECT_IND_T*) pMessage);
        break;
    case MAPC_MNS_SEND_EVENT_IND:
        handleMapcMnsSendEventInd((const MAPC_MNS_SEND_EVENT_IND_T*) pMessage);
        break;
    case MAPC_MAS_CONNECT_CFM:
        handleMapcMasConnectCfm((const MAPC_MAS_CONNECT_CFM_T*) pMessage);
        break;
    case MAPC_MAS_DISCONNECT_IND:
        handleMapcMasDisconnectInd((const MAPC_MAS_DISCONNECT_IND_T*) pMessage);
        break;
    case MAPC_MAS_SET_NOTIFICATION_CFM:
        handleMapcMasSetNotificationCfm((const MAPC_MAS_SET_NOTIFICATION_CFM_T*) pMessage);
        break;

    case MAPC_MAS_SET_FOLDER_CFM:
    case MAPC_MAS_GET_FOLDER_LISTING_CFM:
    case MAPC_MAS_GET_MESSAGES_LISTING_CFM:
    case MAPC_MAS_GET_MESSAGE_CFM:
    case MAPC_MAS_PUT_MESSAGE_CFM:
    case MAPC_MAS_UPDATE_INBOX_CFM:
    case MAPC_MAS_SET_MESSAGE_STATUS_CFM:
    break;

    case MAPC_APP_MAS_DISCONNECT:
        MAPC_DEBUG(("MAPC:MAPC_APP_MAS_DISCONNECT\n"));
    break;

    case MAPC_APP_MAS_CONNECT:
        MAPC_DEBUG(("MAPC:MAPC_APP_MAS_CONNECT\n"));
    break;

    case MAPC_APP_MNS_START:
        MAPC_DEBUG(("MAPC:MAPC_APP_MNS_START\n"));
        MapcMnsStart(&theSink.task, TRUE, 0);
    break;

    case MAPC_APP_MAS_SET_NOTIFICATION:
        MAPC_DEBUG(("MAPC:MAPC_APP_MAS_SET_NOTIFICATION\n"));
        MapcMasSetNotificationRequest(((const MAPC_APP_MAS_SET_NOTIFICATION_T*) pMessage)->masSession,
                                      ((const MAPC_APP_MAS_SET_NOTIFICATION_T*) pMessage)->action );
    break;

    default:
        MAPC_DEBUG(("MAPC:Unknown Message - %x\n", pId));
        break;
    }
}


/****************************************************************************
NAME
	handleMapcMnsStartCfm

DESCRIPTION
    confirmation of start of the message notification service

PARAMS
    pMessage    message

RETURNS
	void
*/
static void handleMapcMnsStartCfm(const MAPC_MNS_START_CFM_T* pMsg)
{
    MAPC_DEBUG(("MAPC:MAPC_MNS_START_CFM- status:[%d] -- ", pMsg->status));

    /* if the notification service started succesfully, store its details which
       are not stored by the MAP library */
    if(pMsg->status == mapc_success)
    {
        MAPC_DEBUG(("MAPC:success. channel = %x, handle = %ld \n", pMsg->mnsChannel, pMsg->sdpHandle));

        /* store server channel and handle values */
        mapcSetMnsChannel(pMsg->mnsChannel);
        mapcSetSdpHandle(pMsg->sdpHandle);

        /* set link encryption requirements */
        ConnectionSmSetSdpSecurityIn(TRUE);
        ConnectionSmRegisterIncomingService(protocol_rfcomm, pMsg->mnsChannel, sec4_in_level_2 );
    }
    else
    {
        MAPC_DEBUG(("MAPC:MAPC Mns start fails\n"));
    }
}

/****************************************************************************
NAME
	handleMapcMnsShutdownCfm

DESCRIPTION
    handle after receiving MAPC_MNS_SHUTDOWN_CFM_T

PARAMS
    pMessage    message

RETURNS
	void
*/
static void handleMapcMnsShutdownCfm(const MAPC_MNS_SHUTDOWN_CFM_T* pMsg)
{
    UNUSED(pMsg);

    MAPC_DEBUG(("MAPC:MAPC_MNS_SHUTDOWN_CFM: \n"));

    /* reset the Message notification service handles after disconnection */
    mapcSetMnsChannel(0);
    mapcSetSdpHandle(0);

}

/****************************************************************************
NAME
	handleMapcMnsConnectInd

DESCRIPTION
    handle after receiving MAPC_MNS_CONNECT_IND_T

PARAMS
    pMessage    message

RETURNS
	void
*/
static void handleMapcMnsConnectInd(const MAPC_MNS_CONNECT_IND_T* pMsg)
{
    MAPC_DEBUG(("MAPC:MAPC_MNS_CONNECT_IND: Connect Indication received\n"));

    MapcMnsConnectResponse( &theSink.task, &pMsg->addr, pMsg->mnsChannel, TRUE, pMsg->connectID );
}

/****************************************************************************
NAME
	handleMapcMnsConnectCfm

DESCRIPTION
    handle after receiving MAPC_MNS_CONNECT_CFM_T

PARAMS
    pMessage    message

RETURNS
	void
*/
static void handleMapcMnsConnectCfm(const MAPC_MNS_CONNECT_CFM_T* pMsg)
{
    mapc_link_priority device_id = mapcGetLinkFromBdAddr( &pMsg->addr );
    mapcState * state = NULL;

    /* ensure device_id has been correctly retrieved */
    if(device_id != mapc_invalid_link)
        state = mapcGetMapState(device_id);

    MAPC_DEBUG(("MAPC:MAPC_MNS_CONNECT_CFM:"));

    if(pMsg->status == mapc_pending)
    {
        MAPC_DEBUG(("Connection Pending. channel = %x \n", pMsg->mnsChannel));
        if(state)
            state->mnsHandle     = pMsg->mnsSession;
    }
    else if(pMsg->status == mapc_success)
    {
        MAPC_DEBUG(("Connection Successfull \n"));

        if(state)
        {
            state->mnsHandle     = pMsg->mnsSession;
            state->mnsChannel    = pMsg->mnsChannel;

            /* Send an message to indicate the mns service connection success */
            MessageSend(&theSink.task, EventSysMapcMnsSuccess, 0);
        }
    }
    else
    {
        MAPC_DEBUG(("MnS Connection Failed\n"));

        /* Send an message to indicate the mns service connection success */
        MessageSend(&theSink.task, EventSysMapcMnsFailed, 0);
    }
}

/****************************************************************************
NAME
	handleMapcMnsDisconnectInd

DESCRIPTION
    handle after receiving MAPC_MNS_DISCONNECT_IND_T

PARAMS
    pMessage    message

RETURNS
	void
*/
static void handleMapcMnsDisconnectInd(const MAPC_MNS_DISCONNECT_IND_T* pMsg)
{
    uint8 device_id  = mapcGetLinkFromMapsSession((MAPC_SESSION)(pMsg->mnsSession));

    MAPC_DEBUG(("MAPC:MAPC_MNS_DISCONNECT_IND\n"));

    /* process the message notification service disconnect indication */
    MapcMnsDisconnectResponse( pMsg->mnsSession );
    /* reset connected state if link is valid */
    if(device_id != mapc_invalid_link)
    {
        mapcState *state = mapcGetMapState(device_id);
        memset( state, 0, sizeof(mapcState) );
    }

}

/****************************************************************************
NAME
	handleMapcMasDisconnectInd

DESCRIPTION
    handle after receiving MAPC_MAS_DISCONNECT_IND_T

PARAMS
    MAPC_MAS_DISCONNECT_IND_T pMsg

RETURNS
	void
*/
static void handleMapcMasDisconnectInd(const MAPC_MAS_DISCONNECT_IND_T* pMsg)
{
    uint8 device_id  = mapcGetLinkFromMapsSession((MAPC_SESSION)(pMsg->masSession));

    MAPC_DEBUG(("MAPC:MAPC_MAS_DISCONNECT_IND.\n"));
    MapcMasDisconnectResponse( pMsg->masSession );

    /* reset connected state if link is valid */
    if(device_id != mapc_invalid_link)
    {
        mapcState *state = mapcGetMapState(device_id);
        memset( state, 0, sizeof(mapcState) );
    }
}

/****************************************************************************
NAME
	handleMapcMasConnectCfm

DESCRIPTION
    Confirmation of the connection of the Message Access Service, if successful
    the next step is to register notification requests

PARAMS
    MAPC_MAS_CONNECT_CFM_T pMsg

RETURNS
	void
*/
static void handleMapcMasConnectCfm(const MAPC_MAS_CONNECT_CFM_T* pMsg)
{
    mapc_link_priority device_id = mapcGetLinkFromBdAddr(&pMsg->addr);
    mapcState * state = NULL;

    /* ensure device_id has been correctly retrieved */
    if(device_id != mapc_invalid_link)
        state = mapcGetMapState(device_id);

    MAPC_DEBUG(("MAPC:MAPC_MAS_CONNECT_CFM- status:[%d] -- ", pMsg->status));

    /* connection still in progress */
    if(pMsg->status == mapc_pending)
    {
        MAPC_DEBUG(("MAPC: Connection Pending. channel = %x \n", pMsg->masChannel));
        if(state)
            state->masHandle   = pMsg->masSession;
    }
    /* connection succeeded, store parameters for link management and then
       attempt to register for message notifications */
    else if(pMsg->status == mapc_success)
    {
        MAPC_DEBUG(("MAPC: Connection successful. channel = %x \n", pMsg->masChannel));

        /* update app stored link parameters required for subsequent map functions */
        if(state)
        {
            state->masHandle     = pMsg->masSession;
            state->bdAddr        = pMsg->addr;
            state->masChannel    = pMsg->masChannel;
            state->device_state  = mapc_mas_connected;

            /* attempt to register for message notifications */
            /* Register notification */
            MAPC_DEBUG(("MAPC:Set Mas Notification: TRUE\n"));
            state->device_state = mapc_mns_registering;
            MapcMasSetNotificationRequest(state->masHandle, TRUE);
        }
    }
    /* message access service failed to connect for whatever reason */
    else
    {
        MAPC_DEBUG(("MAPC: MAS Connection Failed\n"));

        /* Reset the MAPC state of current device to allow subsequent connection attempts */
        if(state)
            memset(state, 0, sizeof(mapcState));
    }
}


/****************************************************************************
NAME
	handleMapcMasSetNotificationCfm

DESCRIPTION
    confirmation of the request to receive message notifcations

PARAMS
    MAPC_MAS_SET_NOTIFICATION_CFM_T pMsg

RETURNS
	void
*/
static void handleMapcMasSetNotificationCfm(const MAPC_MAS_SET_NOTIFICATION_CFM_T* pMsg)
{
    MAPC_DEBUG(("MAPC:MAPC_MAS_SET_NOTIFICATION_CFM- "));

    /* message access service registration of notifications complete */
    if(pMsg->status == mapc_success)
    {
        uint8 device_id  = mapcGetLinkFromMapsSession((MAPC_SESSION)(pMsg->masSession));
        mapcState *state = mapcGetMapState(device_id);

        MAPC_DEBUG(("MAPC:SetNotificationCfm Success\n"));

        if ((state) && device_id != mapc_invalid_link)
        {

            if(state->device_state == mapc_mns_registering)
            {
                 /* update state of this device to indicate registration success */
                 state->device_state = mapc_mns_registered;
            }
            else if(state->device_state == mapc_mns_unregistering)
            {
                 /* update state of this device to indicate registration success */
                 state->device_state = mapc_mas_connected;

                 /* since was an unregister request, disconnect the MAS */
                 MapcMasDisconnectRequest( pMsg->masSession );
            }

        }
    }
    /* message access service registration of notifications failed */
    else
    {
        MAPC_DEBUG(("MAPC:SetNotificationCfm Failure %x\n",pMsg->status));

        /* failed to register message access server notifications,
           therefore disconnect the message access connection as no
           longer in use */
        MapcMasDisconnectRequest( pMsg->masSession );
    }
}

/****************************************************************************
NAME
	handleMapcEventReport

DESCRIPTION
    handle data and get the metadata after receiving MAPC_MNS_SEND_EVENT_IND_T

PARAMS
    @buffer
    @buffer_size

RETURNS
	bool
*/
static bool handleMapcEventReport(const char* buffer, const uint16 buffer_size )
{
    uint16 count = (uint16)strlen(mapcType);
    char *p      = (char *)memchr(buffer, mapcType[0], buffer_size);

    while ( p && ((p + count) <= (buffer + buffer_size)) )
    {
        if(memcmp(p, mapcType, count) == 0)
        {
            return TRUE;
        }
        p += 1;
        p = (char *)memchr(p, mapcType[0], (uint16)(buffer + buffer_size - p));
    }

    return FALSE;
}

static void handleMapcMnsSendEventInd(const MAPC_MNS_SEND_EVENT_IND_T* pMsg)
{
    MapcResponse response= (pMsg->moreData)? mapc_pending: mapc_success;

    MAPC_DEBUG(("MAPC:MAPC_MNS_SEND_EVENT_IND - Mas - %d Len = %d\n",
                   pMsg->masInstanceId,
                   pMsg->sourceLen ));

    if(pMsg->sourceLen)
    {
        MAPC_DEBUG(("MAPC:***** Parsing the Event Object ******\n"));
        MAPC_DEBUG(("MAPC:***** Or do other works        ******\n"));

#ifdef DEBUG_MAPC
    {
        uint16 i;
        const uint8 *lSource = SourceMap(pMsg->eventReport);

        MAPC_DEBUG(("MAPC:The Event Report Object is: "));

        for(i = 0; i < pMsg->sourceLen; i++)
            MAPC_DEBUG(("%c", *(lSource + i)));

        MAPC_DEBUG(("\n"));
    }
#endif

        if(handleMapcEventReport((const char*) SourceMap(pMsg->eventReport), (const uint16) pMsg->sourceLen))
        {
            /* If a new message has been received by the MSE device with type "NewMessage", */
            /* generate a tone or vp for sms message */
            MessageSend(&theSink.task, EventSysMapcMsgNotification, 0);
        }
    }

    /* The application shall not access the source buffer received in the
       indication after calling this API.
    */
    MapcMnsSendEventResponse(pMsg->mnsSession, response);
}


/****************************************************************************
NAME
	mapcMasConnect

DESCRIPTION
    Start Mas connection, including add device and then sdp search;

PARAMS
    @bdaddr

RETURNS
	void
*/
static void mapcMasConnect( mapc_link_priority device_id, const bdaddr *pAddr )
{
    if(device_id != mapc_invalid_link)
    {
        mapcState *state  = mapcGetMapState(device_id);

        MAPC_DEBUG(("MAPC:Start SDP ATTR Search for MAP....\n"));

        /* Store the address */
        state->bdAddr       = *pAddr;
        state->device_state = mapc_mas_connecting;

        /* start sdp search */
        MapcMasSdpAttrSearchRequest( (Task)&mapc_sdp_task, pAddr );
    }
    else
    {
        MAPC_DEBUG(("MAPC:The connection has been started or\nThe number of connection has been MAX_MAPC_CONNECTIONS...\n"));
    }
}

/****************************************************************************
NAME
	mapcMasConnectRequest

DESCRIPTION
    Start Mas connection, including add device and then sdp search, for the
    HFP and A2DP connected devices

PARAMS
    @bdaddr

RETURNS
	void
*/
void mapcMasConnectRequest(bdaddr * pAddr)
{
    /* Start SDP Search and MAS connection */
    mapc_link_priority device_id;

    /* ensure there is a free map connection before attempting a new connection */
    if((device_id = mapcAddDevice(pAddr)) != mapc_invalid_link)
    {
        mapcMasConnect(device_id, pAddr);
    }
}


/****************************************************************************
NAME
	mapcDisableMns

DESCRIPTION
    Disable Map Message Notification Service

PARAMS
    void

RETURNS
	void
*/
void mapcDisconnectMns(void)
{
    mapc_link_priority device_id = 0;

    MAPC_DEBUG(("MAPC:Disconnect All\n"));

    /* Set Notification request to FALSE */
    for(device_id = 0; device_id < MAX_MAPC_CONNECTIONS; device_id ++)
    {
        mapcState *state = mapcGetMapState(device_id);

        /* ensure connection is valid before attempting to disconnect */
        if(state->device_state > mapc_state_idle)
        {
            /* Unregister notification */
            MAPC_DEBUG(("MAPC:Set Mas Notification: FALSE\n"));
            /* update state to indicate unregistering, this will trigger a MAS
               disconnect when the cfm is processed */
            state->device_state = mapc_mns_unregistering;
            /* request the notifications requests are cancelled */
            MapcMasSetNotificationRequest( state->masHandle, FALSE );
        }
    }
}

/****************************************************************************
NAME
	mapcDisconnectDeviceMns

DESCRIPTION
    Disable Map Message Notification Service for the provided device.

PARAMS
    void

RETURNS
    void
*/
void mapcDisconnectDeviceMns(const bdaddr *bd_addr)
{
    mapc_link_priority device_id = mapcGetLinkFromBdAddr(bd_addr);

    if(device_id != mapc_invalid_link)
    {
        mapcState *state = mapcGetMapState(device_id);

        /* ensure connection is valid before attempting to disconnect */
        if(state->device_state > mapc_state_idle)
        {
            /* Unregister notification */
            MAPC_DEBUG(("MAPC:Set Mas Notification: FALSE\n"));
            /* update state to indicate unregistering, this will trigger a MAS
                        disconnect when the cfm is processed */
            state->device_state = mapc_mns_unregistering;
            /* request the notifications requests are cancelled */
            MapcMasSetNotificationRequest( state->masHandle, FALSE );
        }
    }
}


#endif /*ENABLE_MAPC*/
