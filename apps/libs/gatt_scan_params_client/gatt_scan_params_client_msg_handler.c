/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

#include <stdio.h>
#include <stdlib.h>

#include "gatt_scan_params_client_private.h"

#include "gatt_scan_params_client_msg_handler.h"
#include "gatt_scan_params_client_discovery.h"

/****************************************************************************/
static void writeScanIntervalWindow(GSPC_T *scan_params_client)
{
    uint8 value[4];

    value[0] = scan_params_client->scan_interval & 0xff;
    value[1] = (scan_params_client->scan_interval >> 8) & 0xff;
    value[2] = scan_params_client->scan_window & 0xff;
    value[3] = (scan_params_client->scan_window >> 8) & 0xff;
    
    GATT_SCAN_PARAMS_CLIENT_DEBUG_INFO(("GSPC_T: Write scan interval window handle=[0x%x]\n",
                    scan_params_client->scan_interval_window_handle));
    
    GattManagerWriteWithoutResponse((Task)&scan_params_client->lib_task,
                                    scan_params_client->scan_interval_window_handle,
                                    sizeof(value),
                                    value);
}

/***************************************************************************/
static void handleScanRefreshEnableNotificationResp(GSPC_T *scan_params_client, const GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM_T *cfm)
{
    GATT_SCAN_PARAMS_CLIENT_DEBUG_INFO(("GSPC_T: Enable indications status=[%u] cid=[0x%x]\n", 
                            cfm->status,
                            cfm->cid));
                            
    if (cfm->status == gatt_status_success)
    {
        /* Inform the application of successfull */
        scanParamsSendInitSuccessCfm(scan_params_client, scan_params_client->scan_interval_window_handle,
                scan_params_client->scan_refresh_handle);
    }
    else
    {
        scanParamsSendInitFailureCfm(scan_params_client);
    }
}

/***************************************************************************/
static void handleScanRefreshNotification(GSPC_T *scan_params_client, const GATT_MANAGER_REMOTE_SERVER_NOTIFICATION_IND_T *ind)
{
    if((ind->size_value) && (ind->handle == scan_params_client->scan_refresh_handle))
    {
        if(ind->value[0] == SCAN_REFRESH_REQUIRED)
        {
            writeScanIntervalWindow(scan_params_client);
        }
    }
}

/***************************************************************************
NAME
    handleScanParamsGattMsg

DESCRIPTION
   Handles Scan Parameters Client External messages (From GATT )
*/
static void handleScanParamsGattMsg(MessageId id)
{
    UNUSED(id);

     /* GATT unrecognised messages */
    GATT_SCAN_PARAMS_CLIENT_DEBUG_INFO(("Unknown Message 0x%x received from GATT lib\n",id));
    GATT_SCAN_PARAMS_CLIENT_DEBUG_PANIC(("...treat as fatal\n"));
}

/***************************************************************************
NAME
    handleScanParamsGattManagerMsg

DESCRIPTION
   Handles Scan Parameters Gatt Manager messages (From GATT manager )
*/
static void handleScanParamsGattManagerMsg(Task task, MessageId id, Message payload)
{
    GSPC_T *scan_params_client = (GSPC_T *)task;

    switch (id)
    {
        case GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM:
        {
            /* Characteristic handle discovery */
            handleScanParamsDiscoverCharCfm(scan_params_client,(const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM_T*)payload);
        }
        break;

        case GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM:
        {
            /* Characteristic handle discovery */
            handleScanParamsDiscoverCharDescCfm(scan_params_client, (const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T *)payload);
        }
        break;

        case GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM:
        {
            handleScanRefreshEnableNotificationResp(scan_params_client, (const GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM_T *)payload);
        }
        break;
        
        case GATT_MANAGER_WRITE_WITHOUT_RESPONSE_CFM:
        {
            /* Write Confirmation */
        }
        break;

        case GATT_MANAGER_REMOTE_SERVER_NOTIFICATION_IND:
        {
            handleScanRefreshNotification(scan_params_client, (const GATT_MANAGER_REMOTE_SERVER_NOTIFICATION_IND_T *)payload);
        }
        break;
        
        default:
        {
            /* Unrecognised GATT Manager message */
            GATT_SCAN_PARAMS_CLIENT_DEBUG_PANIC(("GSPC_T: Scan Params GattMgr Msg not handled [0x%x]\n", id));
        }
        break;
    }
}

/***************************************************************************
NAME
    handleScanParamsInternalMsg

DESCRIPTION
   Handles Scan Parameters Client service internal messages 
*/
static void  handleScanParamsInternalMsg(Task task, MessageId id)
{
    GSPC_T *scan_params_client = (GSPC_T *)task;
    
    GATT_SCAN_PARAMS_CLIENT_DEBUG_INFO(("handleScanParamsInternalMsg(%d) \n",id));
    
    switch(id)
    {
        case SCAN_PARAMS_CLIENT_INTERNAL_MSG_SET_SCAN:
        {
            /* Update the Scan interval window to remote scan server */
            writeScanIntervalWindow(scan_params_client);
        }
        break;

        default:
        {
            /* Internal unrecognised messages */
            GATT_SCAN_PARAMS_CLIENT_DEBUG_PANIC(("Unknown Message received from Internal To lib \n"));
        }
        break;
    }
}

/****************************************************************************/
void scanRefreshEnableNotification(GSPC_T *scan_params_client, uint16 handle)
{
    uint8 value[2];
    
    value[0] = ENABLE_SCAN_REFRESH_NOTIFICATION;
    value[1] = 0;
    
    GATT_SCAN_PARAMS_CLIENT_DEBUG_INFO(("GSPC_T: Enable scan refresh Notification handle=[0x%x]\n", handle));
    
    GattManagerWriteCharacteristicValue((Task)&scan_params_client->lib_task, handle, sizeof(value), value);
}

/******************************************************************************/
void scanParamsSendInitSuccessCfm(GSPC_T *scan_params_client, uint16 scan_interval_window_handle, uint16 scan_refresh_handle)
{
    MAKE_SCAN_PARAMS_CLIENT_MESSAGE(GATT_SCAN_PARAMS_CLIENT_INIT_CFM);

    /* Write the scan interval window onto the remote device */
    writeScanIntervalWindow(scan_params_client);

    /* Inform the application */
    message->scan_params_client = scan_params_client;
    message->scan_interval_window_handle = scan_interval_window_handle;
    message->scan_refresh_handle = scan_refresh_handle;
    message->status = gatt_scan_params_client_status_success;
    
    MessageSend(scan_params_client->app_task, GATT_SCAN_PARAMS_CLIENT_INIT_CFM, message);
}

/******************************************************************************/
void scanParamsSendInitFailureCfm(GSPC_T *scan_params_client)
{
    MAKE_SCAN_PARAMS_CLIENT_MESSAGE(GATT_SCAN_PARAMS_CLIENT_INIT_CFM);
    
    message->scan_params_client = scan_params_client;
    message->scan_interval_window_handle = 0;
    message->scan_refresh_handle = 0;
    message->status = gatt_scan_params_client_status_failed;
    
    MessageSend(scan_params_client->app_task, GATT_SCAN_PARAMS_CLIENT_INIT_CFM, message);
}

/****************************************************************************/
void scanParamsClientMsgHandler(Task task, MessageId id, Message payload)
{
    if ((id >= GATT_MANAGER_MESSAGE_BASE) && (id < GATT_MANAGER_MESSAGE_TOP))
    {
        handleScanParamsGattManagerMsg(task, id, payload);
    }
    else if ((id >= GATT_MESSAGE_BASE) && (id < GATT_MESSAGE_TOP))
    {
        handleScanParamsGattMsg(id);
    }
    /* Check if this is an internal Message */
    else if ((id >= SCAN_PARAMS_CLIENT_INTERNAL_MSG_BASE) && (id < SCAN_PARAMS_CLIENT_INTERNAL_MSG_TOP))
    {
        handleScanParamsInternalMsg(task, id);
    }
    else
    {
        GATT_SCAN_PARAMS_CLIENT_DEBUG_PANIC(("GSPC_T: Client Msg not handled [0x%x]\n", id));
    }
}
