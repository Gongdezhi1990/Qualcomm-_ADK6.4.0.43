/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. 

*/

#include <stdio.h>
#include <stdlib.h>

#include "gatt_logging_server_private.h"
#include "gatt_logging_server_msg_handler.h"

/* Required octets for values sent to Client Configuration Descriptor */
#define GATT_CLIENT_CONFIG_NUM_OCTETS   2
/* Required octets for values sent to Logging Control Descriptor */
#define GATT_LOGGING_CONTROL_NUM_OCTETS 1

/***************************************************************************
NAME
    sendLoggingServerAccessRsp

DESCRIPTION
    Send an access response to the GATT Manager library.
*/
static void sendLoggingServerAccessRsp(Task task, uint16 cid, uint16 handle, uint16 result, uint16 size_value, const uint8 *value)
{
    if (!GattManagerServerAccessResponse(task, cid, handle, result, size_value, value))
    {
        /* The GATT Manager should always know how to send this response */
        GATT_LOGGING_SERVER_DEBUG_PANIC(("Couldn't send GATT access response\n"));
    }
}

/***************************************************************************
NAME
    LoggingServerClientConfigAccess

DESCRIPTION
    Deals with access of the HANDLE_LOGGING_CLIENT_CONFIG handle.
*/
static void LoggingServerClientConfigAccess(GLOG_T *logging, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    if (access_ind->flags & ATT_ACCESS_READ)
    {
        /* On a Read, ask the app for current client config value */
        MAKE_LOGGING_SERVER_MESSAGE(GATT_LOGGING_SERVER_READ_CLIENT_CONFIG_IND);
        message->logging = logging;     /* Pass the instance which can be returned in the response */
        message->cid = access_ind->cid;                 /* Pass the CID so the client can be identified */
        MessageSend(logging->app_task, GATT_LOGGING_SERVER_READ_CLIENT_CONFIG_IND, message);
    }
    else if (access_ind->flags & ATT_ACCESS_WRITE)
    {
        if (access_ind->size_value == GATT_CLIENT_CONFIG_NUM_OCTETS)
        {
            /* On a Write, send new client config value to the app */
            MAKE_LOGGING_SERVER_MESSAGE(GATT_LOGGING_SERVER_WRITE_CLIENT_CONFIG_IND);
            message->logging = logging;
            message->cid = access_ind->cid;
            message->config_value = (access_ind->value[0] & 0xFF) | ((access_ind->value[1] << 8) & 0xFF00);
            MessageSend(logging->app_task, GATT_LOGGING_SERVER_WRITE_CLIENT_CONFIG_IND, message);
            /* Library response to the access request */
            sendLoggingServerAccessRsp(&logging->lib_task, access_ind->cid, HANDLE_LOGGING_CLIENT_CONFIG, gatt_status_success, 0, NULL);
        }
        else
        {
            sendLoggingServerAccessRsp((Task)&logging->lib_task, access_ind->cid, access_ind->handle, gatt_status_invalid_length, 0, NULL);
        }
    }
    else
    {
        /* Reject access requests that aren't read/write, which shouldn't happen. */
        sendLoggingServerAccessRsp((Task)&logging->lib_task, access_ind->cid, access_ind->handle, gatt_status_request_not_supported, 0, NULL);
    }
}
/***************************************************************************
NAME
    LoggingServerLoggingControlAccess

DESCRIPTION
    Deals with access of the HANDLE_LOGGING_CONTROL handle.
*/
static void LoggingServerLoggingControlAccess(GLOG_T *logging, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    if (access_ind->flags & ATT_ACCESS_READ)
    {
        /* On a Read, ask the app for current client config value */
        MAKE_LOGGING_SERVER_MESSAGE(GATT_LOGGING_SERVER_READ_LOGGING_CONTROL_IND);
        message->logging = logging;     /* Pass the instance which can be returned in the response */
        message->cid = access_ind->cid;                 /* Pass the CID so the client can be identified */
        MessageSend(logging->app_task, GATT_LOGGING_SERVER_READ_LOGGING_CONTROL_IND, message);
    }
    else if (access_ind->flags & ATT_ACCESS_WRITE)
    {
        if (access_ind->size_value == GATT_LOGGING_CONTROL_NUM_OCTETS)
        {
            /* On a Write, send new client config value to the app */
            MAKE_LOGGING_SERVER_MESSAGE(GATT_LOGGING_SERVER_WRITE_LOGGING_CONTROL_IND);
            message->logging = logging;
            message->cid = access_ind->cid;
            message->logging_control = (access_ind->value[0] & 0xFF);
            MessageSend(logging->app_task, GATT_LOGGING_SERVER_WRITE_LOGGING_CONTROL_IND, message);
            /* Library response to the access request */
            sendLoggingServerAccessRsp(&logging->lib_task, access_ind->cid, HANDLE_LOGGING_CONTROL, gatt_status_success, 0, NULL);
        }
        else
        {
            sendLoggingServerAccessRsp((Task)&logging->lib_task, access_ind->cid, access_ind->handle, gatt_status_invalid_length, 0, NULL);
        }
    }
    else
    {
        /* Reject access requests that aren't read/write, which shouldn't happen. */
        sendLoggingServerAccessRsp((Task)&logging->lib_task, access_ind->cid, access_ind->handle, gatt_status_request_not_supported, 0, NULL);
    }
}

/***************************************************************************
NAME
    LoggingServerLoggingDebugConfigurationAccess

DESCRIPTION
    Deals with access of the HANDLE_LOGGING_DEBUG_CONFIGURATION handle.
*/
static void LoggingServerLoggingDebugConfigurationAccess(GLOG_T *logging, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    uint8 index = 0;
    if (access_ind->flags & ATT_ACCESS_READ)
    {
        /* On a Read, ask the app for current client config value */
        MAKE_LOGGING_SERVER_MESSAGE(GATT_LOGGING_SERVER_READ_DEBUG_CONFIGURATION_IND);
        message->logging = logging;     /* Pass the instance which can be returned in the response */
        message->cid = access_ind->cid;                 /* Pass the CID so the client can be identified */
        MessageSend(logging->app_task, GATT_LOGGING_SERVER_READ_DEBUG_CONFIGURATION_IND, message);
    }
    else if (access_ind->flags & ATT_ACCESS_WRITE)
    {
        if (access_ind->size_value == LOGGING_DEBUG_CONFIGURATION_DATA_SIZE)
        {
            /* On a Write, send new client config value to the app */
            MAKE_LOGGING_SERVER_MESSAGE(GATT_LOGGING_SERVER_WRITE_DEBUG_CONFIGURATION_IND);
            message->logging = logging;
            message->cid = access_ind->cid;  
            while (index < LOGGING_DEBUG_CONFIGURATION_DATA_SIZE)
            {
                message->p_debug_configuration[index] = access_ind->value[index];
                index ++;
            }

            MessageSend(logging->app_task, GATT_LOGGING_SERVER_WRITE_DEBUG_CONFIGURATION_IND, message);
            /* Library response to the access request */
            sendLoggingServerAccessRsp(&logging->lib_task, access_ind->cid, access_ind->handle, gatt_status_success, 0, NULL);
        }
        else
        {
            sendLoggingServerAccessRsp((Task)&logging->lib_task, access_ind->cid, access_ind->handle, gatt_status_invalid_length, 0, NULL);
        }
    }
    else
    {
        /* Reject access requests that aren't read/write, which shouldn't happen. */
        sendLoggingServerAccessRsp((Task)&logging->lib_task, access_ind->cid, access_ind->handle, gatt_status_request_not_supported, 0, NULL);
    }
}

/***************************************************************************
NAME
    handleLoggingServerAccessInd

DESCRIPTION
    Handles the GATT_MANAGER_ACCESS_IND message that was sent to the library.
*/
static void handleLoggingServerAccessInd(GLOG_T *const logging, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    GATT_LOGGING_SERVER_DEBUG_INFO((" Func:handleLoggingServerAccessInd(), Handle = %x \n",access_ind->handle));
    
    switch (access_ind->handle)
    {
        case HANDLE_LOGGING_REPORT:
        {
            /* Reject any access indication for Logging Report characteristic */
            sendLoggingServerAccessRsp((Task)&logging->lib_task, access_ind->cid, access_ind->handle, gatt_status_request_not_supported, 0, NULL);
         }
        break;

        /* Client configuration of the Logging Report Characteristic is
         * being read.
         */
        case HANDLE_LOGGING_CLIENT_CONFIG:
        {
              /* This is the handle we are interested */
             LoggingServerClientConfigAccess(logging, access_ind);
        }
        break;

        /* Logging control characteristic is being read/write */
        case HANDLE_LOGGING_CONTROL:
        {
              /* This is the handle we are interested */
             LoggingServerLoggingControlAccess(logging, access_ind);
        }
        break;

        /* Logging Debug configuration characteristic is being read/write */
        case HANDLE_LOGGING_DEBUG_CONFIGURATION:
        {
              /* This is the handle we are interested */
             LoggingServerLoggingDebugConfigurationAccess(logging, access_ind);
        }
        break;
        default:
        {
            /* Respond to invalid handles */
            sendLoggingServerAccessRsp((Task)&logging->lib_task, access_ind->cid, access_ind->handle,gatt_status_invalid_handle, 0, NULL);
        }
        break;
    }
}

/***************************************************************************
NAME
    sendLoggingServerConfigAccessRsp

DESCRIPTION
    Assembles the new client configuration into a message to be sent to the client.
*/
void sendLoggingServerConfigAccessRsp(const GLOG_T *logging, uint16 cid, uint16 client_config)
{
    uint8 config_resp[GATT_CLIENT_CONFIG_NUM_OCTETS];
    
    config_resp[0] = client_config & 0xFF;
    config_resp[1] = (client_config >> 8) & 0xFF;

    sendLoggingServerAccessRsp((Task)&logging->lib_task, cid, HANDLE_LOGGING_CLIENT_CONFIG, gatt_status_success, GATT_CLIENT_CONFIG_NUM_OCTETS, config_resp);
}

/***************************************************************************
NAME
    sendLoggingServerLoggingControlRsp

DESCRIPTION
    Assembles the new logging control into a message to be sent to the client.
*/
void sendLoggingServerLoggingControlRsp(const GLOG_T *logging, uint16 cid, uint8 logging_control)
{
    uint8 config_resp[GATT_LOGGING_CONTROL_NUM_OCTETS];

    config_resp[0] = logging_control;

    sendLoggingServerAccessRsp((Task)&logging->lib_task, cid, HANDLE_LOGGING_CONTROL, gatt_status_success, GATT_LOGGING_CONTROL_NUM_OCTETS, config_resp);
}

/****************************************************************************
NAME
    LoggingServerMsgHandler

DESCRIPTION
    Handles any messages sent to the Logging library by the GATT Manager.
*/
void LoggingServerMsgHandler(Task task, MessageId id, Message msg)
{
    GLOG_T *logging = (GLOG_T*)task;
    
    switch (id)
    {
        case GATT_MANAGER_SERVER_ACCESS_IND:
        {
            /* Read/write access to characteristic */
            handleLoggingServerAccessInd(logging, (const GATT_MANAGER_SERVER_ACCESS_IND_T *)msg);
        }
        break;
        case GATT_MANAGER_REMOTE_CLIENT_NOTIFICATION_CFM:
        {
            /* Library just absorbs confirmation messages */
        }
        break;
        default:
        {
            /* Unrecognized GATT Manager message */
            GATT_LOGGING_SERVER_DEBUG_PANIC(("GLOG: GATT Manager Server Msg 0x%x not handled\n",id));
        }
        break;
    }
}
