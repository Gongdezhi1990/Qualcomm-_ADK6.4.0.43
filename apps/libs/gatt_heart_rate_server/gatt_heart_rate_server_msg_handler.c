/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

#include <stdio.h>
#include <stdlib.h>

#include "gatt_heart_rate_server_private.h"

#include "gatt_heart_rate_server_msg_handler.h"

/* Required octets for values sent to Client Configuration Descriptor */
#define GATT_CLIENT_CONFIG_NUM_OCTETS   2

/***************************************************************************
NAME
    sendHrServerAccessRsp

DESCRIPTION
    Send an access response to the GATT Manager library.
*/
static void sendHrServerAccessRsp(Task task,
                                    uint16 cid,
                                    uint16 handle,
                                    uint16 result,
                                    uint16 size_value,
                                    const uint8 *value)
{
    if (!GattManagerServerAccessResponse(task, cid, handle, result, size_value, value))
    {
        /* The GATT Manager should always know how to send this response */
        GATT_HR_SERVER_DEBUG_PANIC(("Couldn't send GATT access response\n"));
    }
}

/***************************************************************************
NAME
    sendHrServerAccessErrorRsp

DESCRIPTION
    Send an error access response to the GATT Manager library.
*/
static void sendHrServerAccessErrorRsp(const GHRS_T *hr_sensor, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind, uint16 error)
{
    sendHrServerAccessRsp((Task)&hr_sensor->lib_task, access_ind->cid, access_ind->handle, error, 0, NULL);
}

/***************************************************************************
NAME
    hrServerServiceAccess

DESCRIPTION
    Deals with access of the HANDLE_HEART_RATE_SERVICE handle.
*/
static void hrServerServiceAccess(GHRS_T *hr_sensor, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    if (access_ind->flags & ATT_ACCESS_READ)
    {
        sendHrServerAccessRsp((Task)&hr_sensor->lib_task, access_ind->cid, HANDLE_HEART_RATE_SERVICE, gatt_status_success, 0, NULL);
    }
    else if (access_ind->flags & ATT_ACCESS_WRITE)
    {
        /* Write of Heart rate sensor service not allowed. */
        sendHrServerAccessErrorRsp(hr_sensor, access_ind, gatt_status_write_not_permitted);
    }
    else
    {
        /* Reject access requests that aren't read/write, which shouldn't happen. */
        sendHrServerAccessErrorRsp(hr_sensor, access_ind, gatt_status_request_not_supported);
    }
}

/***************************************************************************
NAME
    hrServerClientConfigAccess

DESCRIPTION
    Deals with access of the HANDLE_HEART_RATE_MEASUREMENT_CLIENT_CONFIG handle.
*/
static void hrServerClientConfigAccess(GHRS_T *hr_sensor, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    if (access_ind->flags & ATT_ACCESS_READ)
    {
        /* On a Read, ask the app for current client config value */
        MAKE_HR_SERVER_MESSAGE(GATT_HR_SERVER_READ_CLIENT_CONFIG_IND);
        message->hr_sensor = hr_sensor;     /* Pass the instance which can be returned in the response */
        message->cid = access_ind->cid;                 /* Pass the CID so the client can be identified */
        MessageSend(hr_sensor->app_task, GATT_HR_SERVER_READ_CLIENT_CONFIG_IND, message);
    }
    else if (access_ind->flags & ATT_ACCESS_WRITE)
    {
        if (access_ind->size_value == GATT_CLIENT_CONFIG_NUM_OCTETS)
        {
            /* On a Write, send new client config value to the app */
            MAKE_HR_SERVER_MESSAGE(GATT_HR_SERVER_WRITE_CLIENT_CONFIG_IND);
            message->hr_sensor = hr_sensor;
            message->cid = access_ind->cid;
            message->config_value = (access_ind->value[0] & 0xFF) | ((access_ind->value[1] << 8) & 0xFF00);
            MessageSend(hr_sensor->app_task, GATT_HR_SERVER_WRITE_CLIENT_CONFIG_IND, message);
            /* Library response to the access request */
            sendHrServerAccessRsp(&hr_sensor->lib_task, access_ind->cid, HANDLE_HEART_RATE_MEASUREMENT_CLIENT_CONFIG, gatt_status_success, 0, NULL);
        }
        else
        {
            sendHrServerAccessErrorRsp(hr_sensor, access_ind, gatt_status_invalid_length);
        }
    }
    else
    {
        /* Reject access requests that aren't read/write, which shouldn't happen. */
        sendHrServerAccessErrorRsp(hr_sensor, access_ind, gatt_status_request_not_supported);
    }
}

/***************************************************************************
NAME
    handleHrServerAccessInd

DESCRIPTION
    Handles the GATT_MANAGER_ACCESS_IND message that was sent to the library.
*/
static void handleHrServerAccessInd(GHRS_T *const hr_sensor,
              const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    GATT_HR_SERVER_DEBUG_INFO((" Func:handleHrServerAccessInd(), Handle = %x \n",access_ind->handle));
    
    switch (access_ind->handle)
    {
        case HANDLE_HEART_RATE_SERVICE:
        {
           /* Service Handle read request */
           hrServerServiceAccess(hr_sensor, access_ind);
        }
        break;

        case HANDLE_HEART_RATE_MEASUREMENT:
        {
            /* Reject any access indication for HR measurement characteristic */
            sendHrServerAccessErrorRsp(hr_sensor, access_ind, gatt_status_request_not_supported);        
         }
        break;

        /* Client configuration of the Heart Rate Measurement Characteristic is 
         * being read.
         */
        case HANDLE_HEART_RATE_MEASUREMENT_CLIENT_CONFIG:
        {
              /* This is the handle we are interested */
             hrServerClientConfigAccess(hr_sensor, access_ind);
        }
        break;
     
        default:
        {
            /* Respond to invalid handles */
            sendHrServerAccessErrorRsp(hr_sensor, access_ind,gatt_status_invalid_handle);
        }
        break;
    }
}

/***************************************************************************/
void sendHrServerConfigAccessRsp(const GHRS_T *hr_sensor, uint16 cid, uint16 client_config)
{
    uint8 config_resp[GATT_CLIENT_CONFIG_NUM_OCTETS];
    
    config_resp[0] = client_config & 0xFF;
    config_resp[1] = (client_config >> 8) & 0xFF;

    sendHrServerAccessRsp((Task)&hr_sensor->lib_task, cid, HANDLE_HEART_RATE_MEASUREMENT_CLIENT_CONFIG, gatt_status_success, GATT_CLIENT_CONFIG_NUM_OCTETS, config_resp);
}

/****************************************************************************/
void hrServerMsgHandler(Task task, MessageId id, Message msg)
{
    GHRS_T *hr_sensor = (GHRS_T*)task;
    
    switch (id)
    {
        case GATT_MANAGER_SERVER_ACCESS_IND:
        {
            /* Read/write access to characteristic */
            handleHrServerAccessInd(hr_sensor, (const GATT_MANAGER_SERVER_ACCESS_IND_T *)msg);
        }
        break;
        case GATT_MANAGER_REMOTE_CLIENT_NOTIFICATION_CFM:
        {
            /* Library just absorbs confirmation messages */
        }
        break;
        default:
        {
            /* Unrecognised GATT Manager message */
            GATT_HR_SERVER_DEBUG_PANIC(("GHRS: GATT Manager Server Msg 0x%x not handled\n",id));
        }
        break;
    }
}
