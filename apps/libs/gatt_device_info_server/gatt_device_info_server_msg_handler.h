/* Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_DEVICE_INFO_SERVER_MSG_HANDLER_H_
#define GATT_DEVICE_INFO_SERVER_MSG_HANDLER_H_

#include <csrtypes.h>
#include <message.h>


/***************************************************************************
NAME
    deviceInfoServerMsgHandler

DESCRIPTION
    Handler for external messages sent to the library in the server role.
*/
void deviceInfoServerMsgHandler(Task task, MessageId id, Message payload);


#endif

