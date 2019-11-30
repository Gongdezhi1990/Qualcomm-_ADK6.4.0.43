/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. */
/*  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gatt_transport_discovery_server_private.h"
#include "gatt_transport_discovery_server_msg_handler.h"

/* Macros */
#define sendTdsServerAccessErrorRsp(tds, access_ind, error) sendTdsServerAccessRsp((Task)&tds->lib_task, access_ind->cid, access_ind->handle, error, 0, NULL);

/* Required octets for values sent to Client Configuration Descriptor */
#define GATT_CLIENT_CONFIG_NUM_OCTETS   2

/***************************************************************************
NAME
    sendTdsServerAccessRsp

DESCRIPTION
    Send an access response to the GATT Manager library.
*/
void sendTdsServerAccessRsp(Task task, uint16 cid, uint16 handle, uint16 result, uint16 size_value, const uint8 *value)
{
    if (!GattManagerServerAccessResponse(task, cid, handle, result, size_value, value))
    {
        /* The GATT Manager should always know how to send this response */
        GATT_TDS_SERVER_DEBUG_PANIC(("Couldn't send GATT access response\n"));
    }
}

/***************************************************************************
NAME
    tdsServerControlPointAccess

DESCRIPTION
    Deals with access of the HANDLE_TRANSPORT_DISCOVERY_CONTROL_POINT handle.
*/
static void tdsServerControlPointAccess(GTDS_T *tds, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    if (access_ind->flags & ATT_ACCESS_READ)
    {
        /* Read of TDS service control point not allowed. */
        sendTdsServerAccessErrorRsp(tds, access_ind, gatt_status_read_not_permitted);
    }
    else if (access_ind->flags & ATT_ACCESS_WRITE)
    {
        /* On a Write, send new control point value to the app */
        MAKE_TDS_SERVER_MESSAGE_WITH_LEN(GATT_TRANSPORT_DISCOVERY_SERVER_WRITE_CONTROL_POINT_IND, access_ind->size_value - 1);
        message->tds = tds;
        message->cid = access_ind->cid;
        message->handle = access_ind->handle;
        message->size_value = access_ind->size_value;
        memmove(message->value, access_ind->value, message->size_value);
        
        MessageSend(tds->app_task, GATT_TRANSPORT_DISCOVERY_SERVER_WRITE_CONTROL_POINT_IND, message);
        /* The app will handle the response to the request, whether successful or resulting in an error. */
    }
    else
    {
        /* Reject access requests that aren't read/write, which shouldn't happen. */
        sendTdsServerAccessErrorRsp(tds, access_ind, gatt_status_request_not_supported);
    }
}


/***************************************************************************
NAME
    tdsServerClientConfigAccess

DESCRIPTION
    Deals with access of the HANDLE_TRANSPORT_DISCOVERY_SERVICE_CLIENT_CONFIG handle.
*/
static void tdsServerClientConfigAccess(GTDS_T *tds, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    if (access_ind->flags & ATT_ACCESS_READ)
    {
        /* On a Read, ask the app for current client config value */
        MAKE_TDS_SERVER_MESSAGE(GATT_TRANSPORT_DISCOVERY_SERVER_READ_CLIENT_CONFIG_IND);
        message->tds = tds;     /* Pass the instance which can be returned in the response */
        message->cid = access_ind->cid;                 /* Pass the CID so the client can be identified */
        MessageSend(tds->app_task, GATT_TRANSPORT_DISCOVERY_SERVER_READ_CLIENT_CONFIG_IND, message);
    }
    else if (access_ind->flags & ATT_ACCESS_WRITE)
    {
        if (access_ind->size_value == GATT_CLIENT_CONFIG_NUM_OCTETS)
        {
            /* On a Write, send new client config value to the app */
            MAKE_TDS_SERVER_MESSAGE(GATT_TRANSPORT_DISCOVERY_SERVER_WRITE_CLIENT_CONFIG_IND);
            message->tds = tds;
            message->cid = access_ind->cid;
            message->config_value = (access_ind->value[0] & 0xFF) | ((access_ind->value[1] << 8) & 0xFF00);
            MessageSend(tds->app_task, GATT_TRANSPORT_DISCOVERY_SERVER_WRITE_CLIENT_CONFIG_IND, message);
            /* Library response to the access request */
            sendTdsServerAccessRsp(&tds->lib_task, access_ind->cid, HANDLE_TRANSPORT_DISCOVERY_SERVICE_CLIENT_CONFIG, gatt_status_success, 0, NULL);
        }
        else
        {
            sendTdsServerAccessErrorRsp(tds, access_ind, gatt_status_invalid_length);
        }
    }
    else
    {
        /* Reject access requests that aren't read/write, which shouldn't happen. */
        sendTdsServerAccessErrorRsp(tds, access_ind, gatt_status_request_not_supported);
    }
}


/***************************************************************************
NAME
    handleTdsServerAccessInd

DESCRIPTION
    Handles the GATT_MANAGER_ACCESS_IND message that was sent to the library.
*/
static void handleTdsServerAccessInd(GTDS_T *const tds, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    GATT_TDS_SERVER_DEBUG_INFO((" Function:handleTdsServerAccessInd(), Handle = %x \n",access_ind->handle));
    
    switch (access_ind->handle)
    {        
        case HANDLE_TRANSPORT_DISCOVERY_CONTROL_POINT:
        {
            tdsServerControlPointAccess(tds, access_ind);
        }
        break;
        
        case HANDLE_TRANSPORT_DISCOVERY_SERVICE_CLIENT_CONFIG:
        {
            tdsServerClientConfigAccess(tds, access_ind);
        }
        break;        
     
        default:
        {
            /* Respond to invalid handles */
            sendTdsServerAccessErrorRsp(tds, access_ind,gatt_status_invalid_handle);
        }
        break;
    }
}

/***************************************************************************/
void sendTdsServerConfigAccessRsp(const GTDS_T *tds, uint16 cid, uint16 client_config)
{
    uint8 config_resp[GATT_CLIENT_CONFIG_NUM_OCTETS];
    
    config_resp[0] = client_config & 0xFF;
    config_resp[1] = (client_config >> 8) & 0xFF;

    sendTdsServerAccessRsp((Task)&tds->lib_task, cid, HANDLE_TRANSPORT_DISCOVERY_SERVICE_CLIENT_CONFIG, gatt_status_success, GATT_CLIENT_CONFIG_NUM_OCTETS, config_resp);
}

/****************************************************************************/
void tdsServerMsgHandler(Task task, MessageId id, Message msg)
{
    GTDS_T *tds = (GTDS_T*)task;
    
    switch (id)
    {
        case GATT_MANAGER_SERVER_ACCESS_IND:
        {
            /* Read/write access to characteristic */
            handleTdsServerAccessInd(tds, (const GATT_MANAGER_SERVER_ACCESS_IND_T *)msg);
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
            GATT_TDS_SERVER_DEBUG_PANIC(("GHRS: GATT Manager Server Msg 0x%x not handled\n",id));
        }
        break;
    }
}
