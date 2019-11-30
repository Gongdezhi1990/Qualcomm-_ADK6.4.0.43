/*******************************************************************************
Copyright (c) 2014 - 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_gatt_server_ama.h

DESCRIPTION
    Routines to handle messages sent from the GATT AMA Server Task.
    
NOTES

*/


#ifndef _SINK_GATT_SERVER_AMA_H_
#define _SINK_GATT_SERVER_AMA_H_

#include "gatt_manager.h"

#include <csrtypes.h>
#include <message.h>

#ifdef GATT_AMA_SERVER
#define SINK_BLE_AMA_CONN_PARAM_SLAVE_CONN_INTERVAL_MIN     12
#define SINK_BLE_AMA_CONN_PARAM_SLAVE_CONN_INTERVAL_MAX     12
#define SINK_BLE_AMA_PARAM_SLAVE_CONN_LATENCY               4
#define SINK_BLE_AMA_PARAM_SUPERVISION_TIMEOUT              200
#define SINK_BLE_AMA_PARAM_CE_LENGTH_MIN                    0
#define SINK_BLE_AMA_PARAM_CE_LENGTH_MAX                    80
#endif

/*******************************************************************************
NAME
    sinkGattAmaServerCalculateSize

DESCRIPTION
    Returns the size of data used by a AMA server entry.

PARAMETERS
    None

RETURNS
    The size of the AMA server data.    
*/
#ifdef GATT_AMA_SERVER
uint16 sinkGattAmaServerCalculateSize(void);   
#else
#define sinkGattAmaServerCalculateSize() (0)
#endif

/*******************************************************************************
NAME
    sinkGattAMAServerInitialise

DESCRIPTION
    Initialise the AMA server tasks.
    NOTE: This function will modify *ptr.

PARAMETERS
    ptr - pointer to allocated memory to store server tasks rundata.

RETURNS
    TRUE if the Battery server was initialised correctly, FALSE otherwise.
*/
#ifdef GATT_AMA_SERVER
bool sinkGattAmaServerInitialise(uint16 **ptr);
#else
#define sinkGattAmaServerInitialise(ptr) (TRUE)
#endif

/*******************************************************************************
sinkGattAmaServerMsgHandler
    sinkGattBleTestServerMsgHandler

DESCRIPTION
    Handle messages from the GATT AMA Server library

PARAMETERS
    task    The task the message is delivered
    id      The ID for the GATT message
    payload The message payload

RETURNS
    void
*/
#ifdef GATT_AMA_SERVER
void sinkGattAmaServerMsgHandler(Task task, MessageId id, Message message);
#else
#define sinkGattAmaServerMsgHandler(task, id, message) ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkGattAmaServiceEnabled
    
DESCRIPTION
    Returns whether AMA service is enabled or not.
*/
#ifdef GATT_AMA_SERVER
#define sinkGattAmaServiceEnabled(void) (TRUE)
#else
#define sinkGattAmaServiceEnabled(void) (FALSE)
#endif

#ifdef GATT_AMA_SERVER
bool sinkAmaServerNotify(uint16 cid, uint16 handle, uint16 size, uint8* p_data);
#else
#define sinkAmaServerNotify(a, b, c, d)  (0)
#endif


#ifdef GATT_AMA_SERVER
void sinkAmaServerConnectEvent(GATT_MANAGER_REMOTE_CLIENT_CONNECT_CFM_T * cfm);
#else
#define sinkAmaServerConnectEvent(a) 
#endif

#ifdef GATT_AMA_SERVER
void sinkAmaServerDisconnectEvent(uint16 cid, uint16 reason);
#else
#define sinkAmaServerDisconnectEvent(a, b) 
#endif

#ifdef GATT_AMA_SERVER
void sinkAmaServerNotification(uint16 cid, uint16 count, uint16 handle, uint16 size,  uint8* data);
#else
#define sinkAmaServerNotification(a, b, c, d ) 
#endif

#ifdef GATT_AMA_SERVER
void sinkAmaServerConnectionParameterUpdate(bool fast);
#else
#define sinkAmaServerConnectionParameterUpdate(a)
#endif

#endif /* _SINK_GATT_SERVER_AMA_H_ */
