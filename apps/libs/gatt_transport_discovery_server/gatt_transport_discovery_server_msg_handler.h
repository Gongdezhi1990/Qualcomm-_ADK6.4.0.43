/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_TDS_SERVER_MSG_HANDLER_H_
#define GATT_TDS_SERVER_MSG_HANDLER_H_

/***************************************************************************
NAME
    tdsServerMsgHandler

DESCRIPTION
    Handler for external messages sent to the library in the server role.
*/
void tdsServerMsgHandler(Task task, MessageId id, Message payload);


/***************************************************************************
NAME
    sendTdsServerConfigAccessRsp

DESCRIPTION
    Handler for Sending the TDS notification sent to the library in the server role.
*/
void sendTdsServerConfigAccessRsp(const GTDS_T *tds, uint16 cid, uint16 client_config);


/***************************************************************************
NAME
    sendTdsServerAccessRsp

DESCRIPTION
    Handler for Sending any errors encountered to the TDS client.
*/
void sendTdsServerAccessRsp(Task task, uint16 cid, uint16 handle, uint16 result, uint16 size_value, const uint8 *value);

#endif

