/****************************************************************************
Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd. 


FILE NAME
    sink_gatt_server_ba.h

DESCRIPTION
    Routines to handle messages sent from the GATT Broadcast Server Task.
    
NOTES

*/

#ifndef _SINK_GATT_SERVER_BA_H_
#define _SINK_GATT_SERVER_BA_H_


#if defined GATT_ENABLED && defined ENABLE_BROADCAST_AUDIO
#define GATT_BROADCAST_SERVER
#else
#undef GATT_BROADCAST_SERVER
#endif

#include "sink_ba_common.h"

#include <gatt_broadcast_server.h>

#include <csrtypes.h>
#include <message.h>

#ifdef GATT_BROADCAST_SERVER
#define sinkGattBAServerGetSize() sizeof(GBSS)
#else
#define sinkGattBAServerGetSize() 0
#endif

/*******************************************************************************
NAME
    sinkGattBAServerInitialise
    
DESCRIPTION
    Initialise BA server task.
    
PARAMETERS
    ptr - pointer to allocated memory to store server tasks rundata.
    
RETURNS
    TRUE if the BA server task was initialised, FALSE otherwise.
*/
#ifdef GATT_BROADCAST_SERVER
bool sinkGattBAServerInitialise(uint16 **ptr);
#else
#define sinkGattBAServerInitialise(ptr) (TRUE)
#endif

/*******************************************************************************
NAME
    sinkGattBAServiceEnableAssociation
    
DESCRIPTION
    Interface function to enable/disable association data to be read from server database

PARAMETERS
    enable_assoc - Flag to enable/disable access to association data
    
RETURNS
    None

*/
#ifdef GATT_BROADCAST_SERVER
void sinkGattBAServerEnableAssociation(bool enable_assoc);
#else
#define sinkGattBAServerEnableAssociation(enable_assoc) ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkGattBAServerUpdateSecKey
    
DESCRIPTION
    Interface function to update the security key in the BA database

PARAMETERS
    key - pointer to the security key array
    key_len - lenght of the security key to be copied
    
RETURNS
    None

*/
#ifdef GATT_BROADCAST_SERVER
void sinkGattBAServerUpdateSecKey(uint16* key, uint16 key_len);
#else
#define sinkGattBAServerUpdateSecKey(key, key_len) ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkGattBAServerUpdateDeviceStatus
    
DESCRIPTION
    Interface function to update the stream ID in the BA database

PARAMETERS
    stream_id stream identifier
    
RETURNS
    None

*/
#ifdef GATT_BROADCAST_SERVER
void sinkGattBAServerUpdateStreamID(uint8 stream_id);
#else
#define sinkGattBAServerUpdateStreamID(stream_id) ((void)(0))
#endif

#endif /* _SINK_GATT_SERVER_BA_H_ */
