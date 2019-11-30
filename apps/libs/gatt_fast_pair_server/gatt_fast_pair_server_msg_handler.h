/* Copyright (c) 2018 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_FAST_PAIR_SERVER_MSG_HANDLER_H_
#define GATT_FAST_PAIR_SERVER_MSG_HANDLER_H_

#define CLIENT_CONFIG_VALUE_SIZE    (2)

/***************************************************************************
NAME
    sendFpsServerAccessRsp

DESCRIPTION
    Send a the GATT_ACCESS_RSP to IND messages for the Fast Pair Server.
*/
void sendFpsServerAccessRsp(
        Task task,
        uint16 cid,
        uint16 handle,
        uint16 result,
        uint16 size_value,
        const uint8 *value
        );

/***************************************************************************
NAME
    fpsServerMsgHandler

DESCRIPTION
    Handler for external messages sent to the library in the server role.
*/
void fpsServerMsgHandler(Task task, MessageId id, Message msg);


#endif /* GATT_FAST_PAIR_SERVER_MSG_HANDLER_H_ */

