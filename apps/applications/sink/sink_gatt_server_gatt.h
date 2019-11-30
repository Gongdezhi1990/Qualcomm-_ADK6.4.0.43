/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt_server_gatt.h

DESCRIPTION
    Routines to handle messages sent from the GATT Server task.
    
NOTES

*/


#ifndef _SINK_GATT_SERVER_GATT_H_
#define _SINK_GATT_SERVER_GATT_H_

#include <gatt_manager.h>
#include <gatt_server.h>

#include <csrtypes.h>
#include <message.h>


#ifdef GATT_ENABLED
#define sinkGattServerGetSize(void) sizeof(GGATTS);
#else
#define sinkGattServerGetSize() (0)
#endif


/*******************************************************************************
NAME
    sinkGattServerInitialiseTask
    
DESCRIPTION
    Initialise the GATT server task.
    NOTE: This function will modify *ptr.
    
PARAMETERS
    ptr - pointer to allocated memory to store server tasks rundata.
    
RETURNS
    TRUE if the GATT server task was initialised, FALSE otherwise.
*/
#ifdef GATT_ENABLED
bool sinkGattServerInitialiseTask(uint16 **ptr);
#else
#define sinkGattServerInitialiseTask(ptr) (FALSE)
#endif


/*******************************************************************************
NAME
    sinkGattServerMsgHandler
    
DESCRIPTION
    Handle messages from the GATT Server Task library
    
PARAMETERS
    task    The task the message is delivered
    id      The ID for the GATT message
    message The message payload
    
RETURNS
    TRUE if the message was handled, FALSE otherwise
*/
#ifdef GATT_ENABLED
void sinkGattServerMsgHandler(Task task, MessageId id, Message message);
#else
#define sinkGattServerMsgHandler(task, id, message) ((void)(0))
#endif


/*******************************************************************************
NAME
    sinkGattServerSendServiceChanged
    
DESCRIPTION
    Send Service Changed Indication to remote device
    
PARAMETERS
    cid     The connection ID
    
RETURNS
    TRUE if the message was handled, FALSE otherwise
*/
#ifdef GATT_ENABLED
void sinkGattServerSendServiceChanged(uint16 cid);
#else
#define sinkGattServerSendServiceChanged(cid) ((void)(0))
#endif


#endif /* _SINK_GATT_SERVER_GATT_H_ */

