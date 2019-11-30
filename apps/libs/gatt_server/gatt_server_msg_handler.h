/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_server_msg_handler.h
    
DESCRIPTION
    Contains the "GATT Server" Message handler, handles external messages
    sent to the "GATT Server" task.
*/

#ifndef GATT_SERVER_MSG_HANDLER_H_
#define GATT_SERVER_MSG_HANDLER_H_

/* firmware includes */
#include <message.h>


/*******************************************************************************
NAME
    gattServerMsgHandler
    
DESCRIPTION
    Function to handle external messages.
    
PARAMETERS
    task    The task the message is intended for
    id      The message ID
    payload The message payload
    
RETURN
    void
*/
void gattServerMsgHandler(Task task, MessageId id, Message payload);


#endif
