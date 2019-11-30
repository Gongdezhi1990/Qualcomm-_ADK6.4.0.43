/* Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd. */
/* Part of 6.2 */

#ifndef GATT_AMA_SERVER_MSG_HANDLER_H_
#define GATT_AMA_SERVER_MSG_HANDLER_H_


/***************************************************************************
NAME
    GattAmaServerMsgHandler

DESCRIPTION
    Handler for external messages sent to the library in the server role.
*/
void GattAmaServerMsgHandler(Task task, MessageId id, Message payload);


#endif
