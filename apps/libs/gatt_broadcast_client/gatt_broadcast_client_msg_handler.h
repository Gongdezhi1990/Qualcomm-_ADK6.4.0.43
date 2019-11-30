/* Copyright (c) 2015 - 2017  Qualcomm Technologies International, Ltd. */
/*  */

/*
FILE NAME
    gatt_broadcast_client_msg_handler.h

DESCRIPTION
    Header file for the GATT BA Client message handler
*/

#ifndef GATT_BROADCAST_CLIENT_MSG_HANDLER_H
#define GATT_BROADCAST_CLIENT_MSG_HANDLER_H

#include <library.h>
#include <broadcast.h>

#include "gatt_broadcast_client.h"

/*! Base for the broadcast client library internal messages, 0x0 by convention. */
#define BROADCAST_CLIENT_INTERNAL_MSG_BASE  (0x0)

/*! Internal message types, used to decouple external interface from handling. */
typedef enum
{
    BROADCAST_CLIENT_INTERNAL_START_FINDING_CHAR_VALUES = BROADCAST_CLIENT_INTERNAL_MSG_BASE
} BROADCAST_CLIENT_INTERNAL_MSG;

/***************************************************************************
NAME
    broadcast_client_msg_handler

DESCRIPTION
    Handler for messages sent to the library and internal messages. 
*/
void broadcast_client_msg_handler(Task task, MessageId id, Message msg);

#endif /* GATT_HEART_RATE_CLIENT_MSG_HANDLER_H */

