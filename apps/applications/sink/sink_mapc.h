/*
Copyright (c) 2009 - 2016 Qualcomm Technologies International, Ltd.

*/

/*!
@file
@ingroup sink_app
@brief   
	Interface definition for handling MAP Client library messages and functionality
*/

#ifndef SINK_MAPC_H
#define SINK_MAPC_H

#ifdef ENABLE_MAPC

#include <message.h>
#include <bdaddr.h>
#include <mapc.h>

#define MAX_MAPC_CONNECTIONS    2 

/* Mapc link priority is used to identify different mapc links to
   AG devices using the order in which the devices were connected. 
*/
typedef enum
{
    /*! First mapc link */
    mapc_primary_link,
    /*! Second mapc link */
    mapc_secondary_link, 
    /*! Invalid mapc link */
    mapc_invalid_link
} mapc_link_priority;

/* Mapc Session Connection state */
typedef enum
{
    /* mapc session idle */
    mapc_state_idle,
    /* mapc mas session connecting */
    mapc_mas_connecting,
    /* mapc sdp search completed */
    mapc_sdp_searched,
    /* mapc mas session connected */
    mapc_mas_connected,
    /* mapc mns session registering */
    mapc_mns_registering,
    /* mapc mns session registered */
    mapc_mns_registered,
    /* mapc mas session disconnecting */
    mapc_mas_disconnecting,
    /* mapc mns session unregistering */
    mapc_mns_unregistering
    
} mapc_state;

/*!
    @brief Global data for mapc features.
*/
struct __mapcState
{
    /* Server Bluetooth address */
    bdaddr          bdAddr;
    /* The Mapc Mas Session */
    Mas             masHandle;
    /* The Mapc Mns Session */
    Mns             mnsHandle;
    /* The Mns rfcomm Channel for the MNS Service */
    uint8           mnsChannel;
    /* The Mas rfcomm Channel for the MAS Service */
    uint8           masChannel;
    /* Device id for mapc connection */
    mapc_state      device_state;
};

typedef struct __mapcState mapcState;

#define MAPC_APP_MESSAGE_BASE (MAPC_API_MESSAGE_END + 1)

typedef enum
{
    /* Initialization */
    MAPC_APP_MNS_START = MAPC_APP_MESSAGE_BASE,
    MAPC_APP_MAS_CONNECT,
    MAPC_APP_MAS_DISCONNECT,
    MAPC_APP_MAS_SET_NOTIFICATION,
    
    MAPC_APP_MESSAGE_TOP 

} MapcAppMessageId;


typedef struct{
    bdaddr   bdAddr;
    uint16   device_id;
}MAPC_APP_MAS_CONNECT_T;

typedef struct{
    Mas         masSession;    /*!< The session handle. Invalid on failure */ 
    bool        action;
}MAPC_APP_MAS_SET_NOTIFICATION_T;

typedef struct{
    Mas         masSession;    /*!< The session handle. Invalid on failure */ 
}MAPC_APP_MAS_DISCONNECT_T;


/****************************************************************************
NAME	
	initMap
    
DESCRIPTION
    Register the Map Notification Service
    
PARAMS
    none
    
RETURNS
	void
*/
void initMap(void);

/****************************************************************************
NAME	
	mapcShutDown
    
DESCRIPTION
    Unregister the Map Notification Service, called at shut down
    
PARAMS
    none
    
RETURNS
	void
*/
void mapcShutDown(void);

/****************************************************************************
NAME	
	mapcEnableMns
DESCRIPTION
    Enable Map Message Notification Service
    
PARAMS
    void
    
RETURNS
	void
*/
void mapcEnableMns(void);

/****************************************************************************
NAME	
	mapcDisconnectMns
DESCRIPTION
    Disaable Map Message Notification Service
    
PARAMS
    void
    
RETURNS
	void
*/
void mapcDisconnectMns(void);

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
void mapcDisconnectDeviceMns(const bdaddr *bd_addr);

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
void handleMapcMessages(Task task, MessageId pId, Message pMessage);

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
void mapcMasConnectRequest(bdaddr * pAddr);

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
mapcState* mapcGetMapState(uint16 id);

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
uint32 mapcGetSdpHandle(void);

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
void mapcSetSdpHandle(uint32 handle);

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
uint8 mapcGetMnsChannel(void);

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
void mapcSetMnsChannel(uint8 channel);

#endif /*ENABLE_MAPC*/

#endif /* SINK_MAPC_H */

