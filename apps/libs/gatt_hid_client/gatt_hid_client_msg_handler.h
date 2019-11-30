/* Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd. */
/*  */


#ifndef GATT_HID_CLIENT_MSG_HANDLER_H_
#define GATT_HID_CLIENT_MSG_HANDLER_H_


/***************************************************************************
NAME
    hid_client_msg_handler

DESCRIPTION
    Handler for external message to lib and internal messages within Lib.
*/
void hid_client_msg_handler(Task task, MessageId id, Message msg);

#endif /* GATT_HID_CLIENT_MSG_HANDLER_H_ */

