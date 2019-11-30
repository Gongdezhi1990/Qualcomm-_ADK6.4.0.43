/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_gap_server_msg_handler.h
    
DESCRIPTION
    Contains the "GAP Server" Message handler, handles external messages
    sent to the "GAP Server" task.
*/

#ifndef GATT_GAP_SERVER_MSG_HANDLER_H_
#define GATT_GAP_SERVER_MSG_HANDLER_H_

/* firmware includes */
#include <message.h>


/*******************************************************************************
NAME
    gattGapServerMsgHandler
    
DESCRIPTION
    Function to handle external messages.
    
PARAMETERS
    task    The task the message is intended for
    id      The message ID
    payload The message payload
    
RETURN
    void
*/
void gattGapServerMsgHandler(Task task, MessageId id, Message payload);


#endif
