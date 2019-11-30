/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#include "gatt_battery_server_private.h"

#include "gatt_battery_server_access.h"
#include "gatt_battery_server_db.h"

/* Required octets for values sent to Client Configuration Descriptor */
#define GATT_CLIENT_CONFIG_OCTET_SIZE sizeof(uint8) * 2
/* Required octets for values sent to Presentation Descriptor */
#define GATT_PRESENTATION_OCTET_SIZE sizeof(uint8) * 7


/***************************************************************************
NAME
    sendBatteryAccessRsp

DESCRIPTION
    Send an access response to the GATT Manager library.
*/
static void sendBatteryAccessRsp(Task task,
                                    uint16 cid,
                                    uint16 handle,
                                    uint16 result,
                                    uint16 size_value,
                                    const uint8 *value)
{
    if (!GattManagerServerAccessResponse(task, cid, handle, result, size_value, value))
    {
        /* The GATT Manager should always know how to send this response */
        GATT_BATTERY_SERVER_DEBUG_PANIC(("Couldn't send GATT access response\n"));
    }
}

/***************************************************************************
NAME
    sendBatteryAccessErrorRsp

DESCRIPTION
    Send an error access response to the GATT Manager library.
*/
static void sendBatteryAccessErrorRsp(const GBASS *battery_server, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind, uint16 error)
{
    sendBatteryAccessRsp((Task)&battery_server->lib_task, access_ind->cid, access_ind->handle, error, 0, NULL);
}

/***************************************************************************
NAME
    batteryServiceAccess

DESCRIPTION
    Deals with access of the HANDLE_BATTERY_SERVICE handle.
*/
static void batteryServiceAccess(GBASS *battery_server, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    if (access_ind->flags & ATT_ACCESS_READ)
    {
        sendBatteryAccessRsp((Task)&battery_server->lib_task, access_ind->cid, HANDLE_BATTERY_SERVICE, gatt_status_success, 0, NULL);
    }
    else if (access_ind->flags & ATT_ACCESS_WRITE)
    {
        /* Write of battery level not allowed. */
        sendBatteryAccessErrorRsp(battery_server, access_ind, gatt_status_write_not_permitted);
    }
    else
    {
        /* Reject access requests that aren't read/write, which shouldn't happen. */
        sendBatteryAccessErrorRsp(battery_server, access_ind, gatt_status_request_not_supported);
    }
}

/***************************************************************************
NAME
    batteryLevelAccess

DESCRIPTION
    Deals with access of the HANDLE_BATTERY_LEVEL handle.
*/
static void batteryLevelAccess(GBASS *battery_server, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    if (access_ind->flags & ATT_ACCESS_READ)
    {
        /* Send read level message to app_task so it can return the current level */
        MAKE_BATTERY_MESSAGE(GATT_BATTERY_SERVER_READ_LEVEL_IND);
        message->battery_server = battery_server;     /* Pass the instance which can be returned in the response */
        message->cid = access_ind->cid;                 /* Pass the CID which can be returned in the response */
        MessageSend(battery_server->app_task, GATT_BATTERY_SERVER_READ_LEVEL_IND, message);
    }
    else if (access_ind->flags & ATT_ACCESS_WRITE)
    {
        /* Write of battery level not allowed. */
        sendBatteryAccessErrorRsp(battery_server, access_ind, gatt_status_write_not_permitted);
    }
    else
    {
        /* Reject access requests that aren't read/write, which shouldn't happen. */
        sendBatteryAccessErrorRsp(battery_server, access_ind, gatt_status_request_not_supported);
    }
}

/***************************************************************************
NAME
    batteryClientConfigAccess

DESCRIPTION
    Deals with access of the HANDLE_BATTERY_LEVEL_CLIENT_CONFIG handle.
*/
static void batteryClientConfigAccess(GBASS *battery_server, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    if (access_ind->flags & ATT_ACCESS_READ)
    {
        /* On a Read, ask the app for current client config value */
        MAKE_BATTERY_MESSAGE(GATT_BATTERY_SERVER_READ_CLIENT_CONFIG_IND);
        message->battery_server = battery_server;     /* Pass the instance which can be returned in the response */
        message->cid = access_ind->cid;                 /* Pass the CID so the client can be identified */
        MessageSend(battery_server->app_task, GATT_BATTERY_SERVER_READ_CLIENT_CONFIG_IND, message);
    }
    else if (access_ind->flags & ATT_ACCESS_WRITE)
    {
        if (access_ind->size_value == GATT_CLIENT_CONFIG_OCTET_SIZE)
        {
            /* On a Write, send new client config value to the app */
            MAKE_BATTERY_MESSAGE(GATT_BATTERY_SERVER_WRITE_CLIENT_CONFIG_IND);
            message->battery_server = battery_server;
            message->cid = access_ind->cid;
            message->config_value = (access_ind->value[0] & 0xFF) | ((access_ind->value[1] << 8) & 0xFF00);
            MessageSend(battery_server->app_task, GATT_BATTERY_SERVER_WRITE_CLIENT_CONFIG_IND, message);
            /* Library response to the access request */
            sendBatteryAccessRsp(&battery_server->lib_task, access_ind->cid, HANDLE_BATTERY_LEVEL_CLIENT_CONFIG, gatt_status_success, 0, NULL);
        }
        else
        {
            sendBatteryAccessErrorRsp(battery_server, access_ind, gatt_status_invalid_length);
        }
    }
    else
    {
        /* Reject access requests that aren't read/write, which shouldn't happen. */
        sendBatteryAccessErrorRsp(battery_server, access_ind, gatt_status_request_not_supported);
    }
}

/***************************************************************************
NAME
    batteryPresentationAccess

DESCRIPTION
    Deals with access of the HANDLE_BATTERY_LEVEL_PRESENTATION handle.
*/
static void batteryPresentationAccess(GBASS *battery_server, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    if (access_ind->flags & ATT_ACCESS_READ)
    {  
        /* Send read level message to app_task so it can return the current level */
        MAKE_BATTERY_MESSAGE(GATT_BATTERY_SERVER_READ_PRESENTATION_IND);        
        message->battery_server = battery_server;     /* Pass the instance which can be returned in the response */
        message->cid = access_ind->cid;                 /* Pass the CID which can be returned in the response */
        MessageSend(battery_server->app_task, GATT_BATTERY_SERVER_READ_PRESENTATION_IND, message);
    }
    else if (access_ind->flags & ATT_ACCESS_WRITE)
    {  
        /* Write of battery level not allowed. */
        sendBatteryAccessErrorRsp(battery_server, access_ind, gatt_status_write_not_permitted);
    }
    else
    {
        /* Reject access requests that aren't read/write, which shouldn't happen. */
        sendBatteryAccessErrorRsp(battery_server, access_ind, gatt_status_request_not_supported);
    }
}

/***************************************************************************/
void handleBatteryAccess(GBASS *battery_server, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    switch (access_ind->handle)
    {
        case HANDLE_BATTERY_SERVICE:
        {
            batteryServiceAccess(battery_server, access_ind);
        }
        break;
        
        case HANDLE_BATTERY_LEVEL:
        {
            batteryLevelAccess(battery_server, access_ind);
        }
        break;
        
        case HANDLE_BATTERY_LEVEL_CLIENT_CONFIG:
        {
            if (battery_server->notifications_enabled)
            {
                batteryClientConfigAccess(battery_server, access_ind);
            }
            else
            {
                /* Handle shouldn't be accessed if notifications disabled */
                sendBatteryAccessErrorRsp(battery_server, access_ind, gatt_status_invalid_handle);
            }
        }
        break;
        
        case HANDLE_BATTERY_LEVEL_PRESENTATION:
        {
            batteryPresentationAccess(battery_server, access_ind);
        }
        break;
        
        default:
        {
            /* Respond to invalid handles */
            sendBatteryAccessErrorRsp(battery_server, access_ind, gatt_status_invalid_handle);
        }
        break;
    }
}


/***************************************************************************/
void sendBatteryLevelAccessRsp(const GBASS *battery_server, uint16 cid, uint8 battery_level, uint16 result)
{
    sendBatteryAccessRsp((Task)&battery_server->lib_task, cid, HANDLE_BATTERY_LEVEL, result, 1, &battery_level);
}


/***************************************************************************/
void sendBatteryConfigAccessRsp(const GBASS *battery_server, uint16 cid, uint16 client_config)
{
    uint8 config_resp[GATT_CLIENT_CONFIG_OCTET_SIZE];
    
    config_resp[0] = client_config & 0xFF;
    config_resp[1] = (client_config >> 8) & 0xFF;

    sendBatteryAccessRsp((Task)&battery_server->lib_task, cid, HANDLE_BATTERY_LEVEL_CLIENT_CONFIG, gatt_status_success, GATT_CLIENT_CONFIG_OCTET_SIZE, config_resp);
}


/***************************************************************************/
void sendBatteryPresentationAccessRsp(const GBASS *battery_server, uint16 cid, uint8 name_space, uint16 description)
{
    uint8 presentation[GATT_PRESENTATION_OCTET_SIZE];
    
    /* Fill in Presentation Attribute Value */
    /* Format - 1 octet */
    presentation[0] = 0x04; /* unsigned 8-bit integer */
    /* Exponent - 1 octet */
    presentation[1] = 0; /* actual value = characteristic value */
    /* Unit - 2 octets */
    presentation[2] = 0xAD; /* % lower 8-bits */
    presentation[3] = 0x27; /* % upper 8-bits */
    /* Name Space - 1 octet */
    presentation[4] = name_space;
    /* Description - 2 octets */
    presentation[5] = description & 0xFF;
    presentation[6] = (description >> 8) & 0xFF;
    /* Send complete presentation response */
    sendBatteryAccessRsp((Task)&battery_server->lib_task, cid, HANDLE_BATTERY_LEVEL_PRESENTATION, gatt_status_success, GATT_PRESENTATION_OCTET_SIZE, presentation);
}
