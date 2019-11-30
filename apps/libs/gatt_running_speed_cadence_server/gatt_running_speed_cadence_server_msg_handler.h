/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. 

*/

#ifndef GATT_RSC_SERVER_MSG_HANDLER_H_
#define GATT_RSC_SERVER_MSG_HANDLER_H_

/***************************************************************************
NAME
    RSCServerMsgHandler

DESCRIPTION
    Handler for external messages sent to the library in the server role.
*/
void RSCServerMsgHandler(Task task, MessageId id, Message payload);

/***************************************************************************
NAME
    sendRSCServerConfigAccessRsp

DESCRIPTION
    Handler for Sending the Running Speed and Cadence client configuration 
    sent to the library in the server role.
*/
void sendRSCServerConfigAccessRsp(const GRSCS_T *rscs, uint16 cid, uint16 client_config);

/***************************************************************************
NAME
    sendRSCExtensionServerConfigAccessRsp

DESCRIPTION
    Handler for Sending the Extension client configuration
    sent to the library in the server role.
*/
void sendRSCExtensionServerConfigAccessRsp(const GRSCS_T *rscs, uint16 cid, uint16 client_config);

#endif

