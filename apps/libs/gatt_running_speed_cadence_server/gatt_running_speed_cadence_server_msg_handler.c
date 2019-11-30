/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. 

*/

#include <stdio.h>
#include <stdlib.h>

#include "gatt_running_speed_cadence_server_private.h"

#include "gatt_running_speed_cadence_server_msg_handler.h"

/* Required octets for values sent to Client Configuration Descriptor */
#define GATT_CLIENT_CONFIG_NUM_OCTETS   2

/***************************************************************************
NAME
    sendRSCServerAccessRsp

DESCRIPTION
    Send an access response to the GATT Manager library.
*/
static void sendRSCServerAccessRsp(Task task, uint16 cid, uint16 handle, uint16 result, uint16 size_value, const uint8 *value)
{
    if (!GattManagerServerAccessResponse(task, cid, handle, result, size_value, value))
    {
        /* The GATT Manager should always know how to send this response */
        GATT_RSC_SERVER_DEBUG_PANIC(("Couldn't send GATT access response\n"));
    }
}

/***************************************************************************
NAME
    RSCServerServiceAccess

DESCRIPTION
    Deals with access of the HANDLE_RUNNING_SPEED_AND_CADENCE_SERVICE handle.
*/
static void RSCServerServiceAccess(GRSCS_T *rscs, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    if (access_ind->flags & ATT_ACCESS_READ)
    {
        sendRSCServerAccessRsp((Task)&rscs->lib_task, access_ind->cid, HANDLE_RUNNING_SPEED_AND_CADENCE_SERVICE, gatt_status_success, 0, NULL);
    }
    else if (access_ind->flags & ATT_ACCESS_WRITE)
    {
        /* Write of Running Speed and Cadence service not allowed. */
        sendRSCServerAccessRsp((Task)&rscs->lib_task, access_ind->cid, access_ind->handle, gatt_status_write_not_permitted, 0, NULL);
    }
    else
    {
        /* Reject access requests that aren't read/write, which shouldn't happen. */
        sendRSCServerAccessRsp((Task)&rscs->lib_task, access_ind->cid, access_ind->handle, gatt_status_request_not_supported, 0, NULL);
    }
}

/***************************************************************************
NAME
    RSCServerClientConfigAccess

DESCRIPTION
    Deals with access of the HANDLE_RUNNING_SPEED_AND_CADENCE_MEASUREMENT_CLIENT_CONFIG handle.
*/
static void RSCServerClientConfigAccess(GRSCS_T *rscs, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    if (access_ind->flags & ATT_ACCESS_READ)
    {
        /* On a Read, ask the app for current client config value */
        MAKE_RSCS_SERVER_MESSAGE(GATT_RSC_SERVER_READ_CLIENT_CONFIG_IND);
        message->rscs = rscs;     /* Pass the instance which can be returned in the response */
        message->cid = access_ind->cid;                 /* Pass the CID so the client can be identified */
        MessageSend(rscs->app_task, GATT_RSC_SERVER_READ_CLIENT_CONFIG_IND, message);
    }
    else if (access_ind->flags & ATT_ACCESS_WRITE)
    {
        if (access_ind->size_value == GATT_CLIENT_CONFIG_NUM_OCTETS)
        {
            /* On a Write, send new client config value to the app */
            MAKE_RSCS_SERVER_MESSAGE(GATT_RSC_SERVER_WRITE_CLIENT_CONFIG_IND);
            message->rscs = rscs;
            message->cid = access_ind->cid;
            message->config_value = (access_ind->value[0] & 0xFF) | ((access_ind->value[1] << 8) & 0xFF00);
            MessageSend(rscs->app_task, GATT_RSC_SERVER_WRITE_CLIENT_CONFIG_IND, message);
            /* Library response to the access request */
            sendRSCServerAccessRsp(&rscs->lib_task, access_ind->cid, HANDLE_RUNNING_SPEED_AND_CADENCE_CLIENT_CONFIG, gatt_status_success, 0, NULL);
        }
        else
        {
            sendRSCServerAccessRsp((Task)&rscs->lib_task, access_ind->cid, access_ind->handle, gatt_status_invalid_length, 0, NULL);
        }
    }
    else
    {
        /* Reject access requests that aren't read/write, which shouldn't happen. */
        sendRSCServerAccessRsp((Task)&rscs->lib_task, access_ind->cid, access_ind->handle, gatt_status_request_not_supported, 0, NULL);
    }
}

/***************************************************************************
NAME
    RSCExtensionServerClientConfigAccess

DESCRIPTION
    Deals with access of the HANDLE_RUNNING_SPEED_AND_CADENCE_EXTENSION_CLIENT_CONFIG handle.
*/
static void RSCExtensionServerClientConfigAccess(GRSCS_T *rscs, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    if (access_ind->flags & ATT_ACCESS_READ)
    {
        /* On a Read, ask the app for current client config value */
        MAKE_RSCS_SERVER_MESSAGE(GATT_RSC_EXTENSION_SERVER_READ_CLIENT_CONFIG_IND);
        message->rscs = rscs;     /* Pass the instance which can be returned in the response */
        message->cid = access_ind->cid;                 /* Pass the CID so the client can be identified */
        MessageSend(rscs->app_task, GATT_RSC_EXTENSION_SERVER_READ_CLIENT_CONFIG_IND, message);
    }
    else if (access_ind->flags & ATT_ACCESS_WRITE)
    {
        if (access_ind->size_value == GATT_CLIENT_CONFIG_NUM_OCTETS)
        {
            /* On a Write, send new client config value to the app */
            MAKE_RSCS_SERVER_MESSAGE(GATT_RSC_EXTENSION_SERVER_WRITE_CLIENT_CONFIG_IND);
            message->rscs = rscs;
            message->cid = access_ind->cid;
            message->config_value = (access_ind->value[0] & 0xFF) | ((access_ind->value[1] << 8) & 0xFF00);
            MessageSend(rscs->app_task, GATT_RSC_EXTENSION_SERVER_WRITE_CLIENT_CONFIG_IND, message);
            /* Library response to the access request */
            sendRSCServerAccessRsp(&rscs->lib_task, access_ind->cid, HANDLE_RUNNING_SPEED_AND_CADENCE_EXTENSION_CLIENT_CONFIG, gatt_status_success, 0, NULL);
        }
        else
        {
            sendRSCServerAccessRsp((Task)&rscs->lib_task, access_ind->cid, access_ind->handle, gatt_status_invalid_length, 0, NULL);
        }
    }
    else
    {
        /* Reject access requests that aren't read/write, which shouldn't happen. */
        sendRSCServerAccessRsp((Task)&rscs->lib_task, access_ind->cid, access_ind->handle, gatt_status_request_not_supported, 0, NULL);
    }
}

/***************************************************************************
NAME
    handleRSCServerAccessInd

DESCRIPTION
    Handles the GATT_MANAGER_ACCESS_IND message that was sent to the library.
*/
static void handleRSCServerAccessInd(GRSCS_T *const rscs,
              const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    GATT_RSC_SERVER_DEBUG_INFO((" Func:handleRSCServerAccessInd(), Handle = %x \n",access_ind->handle));
    
    switch (access_ind->handle)
    {
        case HANDLE_RUNNING_SPEED_AND_CADENCE_SERVICE:
        {
           /* Service Handle read request */
           RSCServerServiceAccess(rscs, access_ind);
        }
        break;

        case HANDLE_RUNNING_SPEED_AND_CADENCE_MEASUREMENT:
        {
            /* Reject any access indication for RSC measurement characteristic */
            sendRSCServerAccessRsp((Task)&rscs->lib_task, access_ind->cid, access_ind->handle, gatt_status_request_not_supported, 0, NULL);        
         }
        break;

        case HANDLE_RUNNING_SPEED_AND_CADENCE_EXTENSION:
        {
            /* Reject any access indication for measurement characteristic */
            sendRSCServerAccessRsp((Task)&rscs->lib_task, access_ind->cid, access_ind->handle, gatt_status_request_not_supported, 0, NULL);
        }
        break;

        /* Client configuration of the Running Speed and Cadence Measurement Characteristic is 
         * being read.
         */
        case HANDLE_RUNNING_SPEED_AND_CADENCE_CLIENT_CONFIG:
        {
              /* This is the handle we are interested */
             RSCServerClientConfigAccess(rscs, access_ind);
        }
        break;

        /* Client configuration of the Running Speed and Cadence Extension Characteristic is
         * being read.
         */
        case HANDLE_RUNNING_SPEED_AND_CADENCE_EXTENSION_CLIENT_CONFIG:
        {
              /* This is the handle we are interested */
             RSCExtensionServerClientConfigAccess(rscs, access_ind);
        }
        break;
     
        default:
        {
            /* Respond to invalid handles */
            sendRSCServerAccessRsp((Task)&rscs->lib_task, access_ind->cid, access_ind->handle,gatt_status_invalid_handle, 0, NULL);
        }
        break;
    }
}

/***************************************************************************
NAME
    sendRSCServerConfigAccessRsp

DESCRIPTION
    Assembles the new client configuration into a message to be sent to the client.
*/
void sendRSCServerConfigAccessRsp(const GRSCS_T *rscs, uint16 cid, uint16 client_config)
{
    uint8 config_resp[GATT_CLIENT_CONFIG_NUM_OCTETS];
    
    config_resp[0] = client_config & 0xFF;
    config_resp[1] = (client_config >> 8) & 0xFF;

    sendRSCServerAccessRsp((Task)&rscs->lib_task, cid, HANDLE_RUNNING_SPEED_AND_CADENCE_CLIENT_CONFIG, gatt_status_success, GATT_CLIENT_CONFIG_NUM_OCTETS, config_resp);
}

/***************************************************************************
NAME
    sendRSCExtensionServerConfigAccessRsp

DESCRIPTION
    Assembles the new client configuration into a message to be sent to the client.
*/
void sendRSCExtensionServerConfigAccessRsp(const GRSCS_T *rscs, uint16 cid, uint16 client_config)
{
    uint8 config_resp[GATT_CLIENT_CONFIG_NUM_OCTETS];

    config_resp[0] = client_config & 0xFF;
    config_resp[1] = (client_config >> 8) & 0xFF;

    sendRSCServerAccessRsp((Task)&rscs->lib_task, cid, HANDLE_RUNNING_SPEED_AND_CADENCE_EXTENSION_CLIENT_CONFIG, gatt_status_success, GATT_CLIENT_CONFIG_NUM_OCTETS, config_resp);
}

/****************************************************************************
NAME
    RSCServerMsgHandler

DESCRIPTION
    Handles any messages sent to the RSCS library by the GATT Manager.
*/
void RSCServerMsgHandler(Task task, MessageId id, Message msg)
{
    GRSCS_T *rscs = (GRSCS_T*)task;
    
    switch (id)
    {
        case GATT_MANAGER_SERVER_ACCESS_IND:
        {
            /* Read/write access to characteristic */
            handleRSCServerAccessInd(rscs, (const GATT_MANAGER_SERVER_ACCESS_IND_T *)msg);
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
            GATT_RSC_SERVER_DEBUG_PANIC(("GRSCS: GATT Manager Server Msg 0x%x not handled\n",id));
        }
        break;
    }
}
