/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt_manager.h

DESCRIPTION
    Manage GATT Manager messages.
    
NOTES

*/

#ifndef _SINK_GATT_MANAGER_H_
#define _SINK_GATT_MANAGER_H_


#include <bdaddr.h>
#include <csrtypes.h>
#include <message.h>


/*******************************************************************************
NAME
    sinkGattManagerStartAdvertising
    
DESCRIPTION
    Makes a GATT Manager request to start advertising and wait for a client
    device to connect.
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef GATT_ENABLED
void sinkGattManagerStartAdvertising(void);
#else
#define sinkGattManagerStartAdvertising() ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkGattManagerStopAdvertising
    
DESCRIPTION
    If the device is advertising, this shall stop the advertising and cancel
    the pending GATT connection (waiting for a client to connect). 
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef GATT_ENABLED
void sinkGattManagerStopAdvertising(void);
#else
#define sinkGattManagerStopAdvertising()
#endif


/*******************************************************************************
NAME
    sinkGattManagerMsgHandler
    
DESCRIPTION
    Handle messages from the GATT Manager library
    
PARAMETERS
    task    The task the message is delivered
    id      The ID for the GATT message
    message The message payload
    
RETURNS
    None
*/
#ifdef GATT_ENABLED
void sinkGattManagerMsgHandler(Task task, MessageId id, Message message);
#else
#define sinkGattManagerMsgHandler(task, id, message) ((void)(0))
#endif


/*******************************************************************************
NAME
    sinkGattManagerStartConnection
    
DESCRIPTION
    Connect to remote device. 
    
PARAMETERS
    addr    Address of the device to connect with.
    
RETURNS
    None
*/
#ifdef GATT_ENABLED
void sinkGattManagerStartConnection(const typed_bdaddr *addr);
#else
#define sinkGattManagerStartConnection(addr) ((void)(0))
#endif


#endif /* _SINK_GATT_MANAGER_H_ */
