/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_HR_SERVER_MSG_HANDLER_H_
#define GATT_HR_SERVER_MSG_HANDLER_H_

/***************************************************************************
NAME
    hrServerMsgHandler

DESCRIPTION
    Handler for external messages sent to the library in the server role.
*/
void hrServerMsgHandler(Task task, MessageId id, Message payload);

/***************************************************************************
NAME
    sendHrServerConfigAccessRsp

DESCRIPTION
    Handler for Sending the hear rate measurement notification sent to the library in the server role.
*/
void sendHrServerConfigAccessRsp(const GHRS_T *hr_sensor, uint16 cid, uint16 client_config);

#endif

