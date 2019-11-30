/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

/*
FILE NAME
    gatt_broadcast_server_msg_handler.h

DESCRIPTION
    Header file for the GATT BA Server message handler
*/

#ifndef GATT_BROADCAST_SERVER_MSG_HANDLER_H
#define GATT_BROADCAST_SERVER_MSG_HANDLER_H

#include <message.h>

/***************************************************************************
NAME
    broadcast_server_msg_handler

DESCRIPTION
    Handler for external messages sent to the library in the server role.
*/
void broadcast_server_msg_handler(Task task, MessageId id, Message message);

#endif /* GATT_BROADCAST_SERVER_MSG_HANDLER_H */

