/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    broadcast_cmd_hanlder.h

DESCRIPTION
    A utility file to handle all messages coming from SCM library
*/


/*!
@file    broadcast_cmd_handler.h
@brief   This file provides the defination of functions used to handle SCM messages.
*/


#ifndef BROADCAST_CMD_HANDLER_
#define BROADCAST_CMD_HANDLER_

#include <broadcast.h>
#include <sink.h>

/****************************************************************************
 * Functions
 ****************************************************************************/
/*!
    @brief Function to handler messages from the SCM library

    @param task [IN] task to which messages are sent.
    @param id [IN] Message id which was sent.
    @param message[IN] Actual data of the message.
*/
void BroadcastCmdScmMessageHandler(Task task, MessageId id, Message message);

#endif /* BROADCAST_CMD_HANDLER_ */
