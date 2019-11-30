/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt.h

DESCRIPTION
    Manage GATT messages.
    
NOTES

*/

#ifndef _SINK_GATT_H_
#define _SINK_GATT_H_

#include <csrtypes.h>
#include <message.h>

/*******************************************************************************
NAME
    sinkGattUpdateMinimumTxMtu

DESCRIPTION
    Update the minimum TX MTU if the GATT service requires an MTU that is larger than
    SINK_GATT_DEFAULT_MTU_SIZE.

PARAMETERS
    uint16 requested_mtu_size

RETURNS
    void
*/
#ifdef GATT_ENABLED
void sinkGattUpdateMinimumTxMtu(uint16 requested_mtu_size);
#else
#define sinkGattUpdateMinimumTxMtu(requested_mtu_size) ((void)(requested_mtu_size))
#endif

/*******************************************************************************
NAME
    sinkGattGetMinimumTxMtu

DESCRIPTION
    Get the minimum TX MTU size.

RETURNS
    uint16 MTU size
*/
#ifdef GATT_ENABLED
uint16 sinkGattGetMinimumTxMtu(void);
#else
#define sinkGattGetMinimumTxMtu() ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkGattMsgHandler
    
DESCRIPTION
    Handle messages from the GATT library
    
PARAMETERS
    task    The task the message is delivered
    id      The ID for the GATT message
    message The message payload
    
RETURNS
    void
*/
#ifdef GATT_ENABLED
void sinkGattMsgHandler(Task task, MessageId id, Message message);
#else
#define sinkGattMsgHandler(task, id, message) ((void)(0))
#endif


#endif /* _SINK_GATT_H_ */

