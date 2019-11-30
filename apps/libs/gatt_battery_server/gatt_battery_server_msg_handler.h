/* Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_BATTERY_SERVER_MSG_HANDLER_H_
#define GATT_BATTERY_SERVER_MSG_HANDLER_H_


/***************************************************************************
NAME
    batteryServerMsgHandler

DESCRIPTION
    Handler for external messages sent to the library in the server role.
*/
void batteryServerMsgHandler(Task task, MessageId id, Message payload);


#endif
