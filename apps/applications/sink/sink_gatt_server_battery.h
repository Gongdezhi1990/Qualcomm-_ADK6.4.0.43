/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt_server_battery.h

DESCRIPTION
    Routines to handle messages sent from the GATT Battery Server Task.
    
NOTES

*/


#ifndef _SINK_GATT_SERVER_BATTERY_H_
#define _SINK_GATT_SERVER_BATTERY_H_


#ifndef GATT_ENABLED
#undef GATT_BATTERY_SERVER
#endif


#include <gatt_battery_server.h>

#include <csrtypes.h>
#include <message.h>

typedef enum
{
    GATT_BATTERY_SERVER_LOCAL_ID,
    GATT_BATTERY_SERVER_REMOTE_ID,
    GATT_BATTERY_SERVER_PEER_ID
}gatt_server_battery_id;

#define GATT_SERVER_BATTERY_UPDATE_TIME    30000

#define LOCAL_UPDATE_REQD(conex)  (((conex).client_config.battery_local) & 1)
#define PEER_UPDATE_REQD(conex)   (((conex).client_config.battery_peer) & 1)
#define REMOTE_UPDATE_REQD(conex) (((conex).client_config.battery_remote) & 1)

/*******************************************************************************
NAME
    sinkBleBatteryLevelReadSendAndRepeat
    
DESCRIPTION
    Handle received BLE_INTERNAL_MESSAGE_BATTERY_READ_TIMER.
    
PARAMETERS
    None
    
RETURNS
    None.
*/
#ifdef GATT_BATTERY_SERVER
void sinkBleBatteryLevelReadSendAndRepeat(void);
#else
#define sinkBleBatteryLevelReadSendAndRepeat() ((void)(0)) 
#endif

/*******************************************************************************
NAME
    sinkGattBatteryServerCalculateSize

DESCRIPTION
    Returns the size of data used by a battery server entry.

PARAMETERS
    None

RETURNS
    The size of the battery server data.
*/
#ifdef GATT_BATTERY_SERVER
uint16 sinkGattBatteryServerCalculateSize(void);
#else
#define sinkGattBatteryServerCalculateSize() (0)
#endif


/*******************************************************************************
NAME
    sinkGattBatteryServerInitialise

DESCRIPTION
    Initialise the Battery server tasks.
    NOTE: This function will modify *ptr.

PARAMETERS
    ptr - pointer to allocated memory to store server tasks rundata.

RETURNS
    TRUE if the Battery server was initialised correctly, FALSE otherwise.
*/
#ifdef GATT_BATTERY_SERVER
bool sinkGattBatteryServerInitialise(uint16 **ptr);
#else
#define sinkGattBatteryServerInitialise(ptr) (TRUE)
#endif


/*******************************************************************************
NAME
    sinkGattBatteryServerMsgHandler

DESCRIPTION
    Handle messages from the GATT Battery Server library

PARAMETERS
    task    The task the message is delivered
    id      The ID for the GATT message
    message The message payload

RETURNS
    void
*/
#ifdef GATT_BATTERY_SERVER
void sinkGattBatteryServerMsgHandler(Task task, MessageId id, Message message);
#else
#define sinkGattBatteryServerMsgHandler(task, id, message) ((void)(0))
#endif


/*******************************************************************************
NAME
    sinkGattBatteryServiceEnabled
    
DESCRIPTION
    Returns whether battery service is enabled or not.
*/
bool sinkGattBatteryServiceEnabled(void);

#endif /* _SINK_GATT_SERVER_BATTERY_H_ */
