/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt_server_gap.h

DESCRIPTION
    Routines to handle messages sent from the GATT GAP Server task.
    
NOTES

*/

#ifndef _SINK_GATT_SERVER_GAP_H_
#define _SINK_GATT_SERVER_GAP_H_

#include <gatt_manager.h>
#include <gatt_gap_server.h>
#include <connection_no_ble.h>

#include <csrtypes.h>
#include <message.h>


typedef enum gap_msg_status
{
    gap_msg_success,
    gap_msg_failed,
    gap_msg_read_name_required
    
} gap_msg_status_t;


#ifdef GATT_ENABLED
#define sinkGapServerGetSize(void) sizeof(GGAPS);
#else
#define sinkGapServerGetSize(void) (0)
#endif


/*******************************************************************************
NAME
    sinkGattGapServerInitialiseTask
    
DESCRIPTION
    Initialise the GAP server task.
    NOTE: This function will modify *ptr.
    
PARAMETERS
    ptr - pointer to allocated memory to store server tasks rundata.
    
RETURNS
    TRUE if the GAP server task was initialised, FALSE otherwise.
*/
#ifdef GATT_ENABLED
bool sinkGattGapServerInitialiseTask(uint16 **ptr);
#else
#define sinkGattGapServerInitialiseTask(ptr) (FALSE)
#endif


/*******************************************************************************
NAME
    sinkGattGapServerSendLocalNameResponse
    
DESCRIPTION
    Send the local name to the device that requested it.
    
PARAMETERS
    
    
RETURN
*/
#ifdef GATT_ENABLED
void sinkGattGapServerSendLocalNameResponse(CL_DM_LOCAL_NAME_COMPLETE_T * msg);
#else
#define sinkGattGapServerSendLocalNameResponse(msg) ((void)(0))
#endif


/*******************************************************************************
NAME
    sinkGattGapServerMsgHandler
    
DESCRIPTION
    Handle messages from the GATT GAP Server Task library.
    
PARAMETERS
    task    The task the message is delivered.
    id      The ID for the GATT message.
    message The message payload.
    
RETURN
    A TRUE is returned if the GAP service requires a read of the local name. FALSE otherwise.
*/
#ifdef GATT_ENABLED
gap_msg_status_t sinkGattGapServerMsgHandler(Task task, MessageId id, Message message);
#else
#define sinkGattGapServerMsgHandler(task, id, message) (gap_msg_success)
#endif


#endif /* _SINK_GATT_SERVER_GAP_H_ */

