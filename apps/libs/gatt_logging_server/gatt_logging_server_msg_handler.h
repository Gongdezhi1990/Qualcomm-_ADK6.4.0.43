/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. 

*/

#ifndef GATT_LOGGING_SERVER_MSG_HANDLER_H_
#define GATT_LOGGING_SERVER_MSG_HANDLER_H_

/***************************************************************************
NAME
    LoggingServerMsgHandler

DESCRIPTION
    Handler for external messages sent to the library in the server role.
*/
void LoggingServerMsgHandler(Task task, MessageId id, Message payload);

/***************************************************************************
NAME
    sendLoggingServerConfigAccessRsp

DESCRIPTION
    Handler for Sending the Logging client configuration
    sent to the library in the server role.
*/
void sendLoggingServerConfigAccessRsp(const GLOG_T *logging, uint16 cid, uint16 client_config);

/***************************************************************************
NAME
    sendLoggingServerLoggingControlRsp

DESCRIPTION
    Assembles the new logging control into a message to be sent to the client.
*/
void sendLoggingServerLoggingControlRsp(const GLOG_T *logging, uint16 cid, uint8 logging_control);

#endif

