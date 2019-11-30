/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gatt_client.h"
#include "gatt_scan_params_client_private.h"

#include "gatt_scan_params_client_discovery.h"
#include "gatt_scan_params_client_msg_handler.h"

/****************************************************************************/
bool GattScanParamsClientInit(Task appTask ,
                        GSPC_T *const scan_params_client,
                        const GATT_SCAN_PARAMS_CLIENT_INIT_PARAMS_T *const client_init_params,
                        const gatt_scan_params_device_data_t *device_data)
{
    gatt_manager_client_registration_params_t reg_params;
    
    /* validate the input parameters */
    if( INPUT_PARAM_NULL(appTask,scan_params_client,client_init_params) || CLIENT_INIT_PARAM_INVALID(client_init_params))
    {
        GATT_SCAN_PARAMS_CLIENT_PANIC(("GSPC: Invalid Initialisation parameters"));
    }

    memset(&reg_params, 0, sizeof(gatt_manager_client_registration_params_t));
    
    /* Reset the Scan Params client data structure */
    memset(scan_params_client, 0, sizeof(GSPC_T));
    
    /* Keep track on application task as all the notifications and indication need to be send to here */
    scan_params_client->app_task = appTask;
    /* Store library task for external message reception */
    scan_params_client->lib_task.handler = scanParamsClientMsgHandler;
    /*Store the Scan interval window handle with Invalid Value */
    scan_params_client->scan_interval_window_handle = INVALID_SCAN_INTERVAL_WINDOW_HANDLE;
    /*Store the Scan Refresh handle with Invalid Value */
    scan_params_client->scan_refresh_handle = INVALID_SCAN_REFRESH_HANDLE;
    /*Store the Scan Interval Value */
    scan_params_client->scan_interval = client_init_params->scan_interval;
    /*Store the Scan Window Value */
    scan_params_client->scan_window = client_init_params->scan_window;

    /* Setup GATT manager registartion parameters */
    reg_params.cid = client_init_params->cid;
    reg_params.client_task = &scan_params_client->lib_task;
    reg_params.end_handle = client_init_params->end_handle;
    reg_params.start_handle = client_init_params->start_handle;

    if(GattManagerRegisterClient(&reg_params) == gatt_manager_status_success)
    {
        /* If the device is already known, get the persistent data */
        if (device_data)
        {
            /* Don't need to discover data from the device; use the data supplied instead */
            scan_params_client->scan_interval_window_handle = device_data->scan_interval_window_handle;
            scan_params_client->scan_refresh_handle = device_data->scan_refresh_handle;
            scanParamsSendInitSuccessCfm(scan_params_client, device_data->scan_interval_window_handle,
                    device_data->scan_refresh_handle);
        }
        else
        {
            /* Discover all characteristics and descriptors after successful registration */
            discoverAllScanParamsCharacteristics(scan_params_client);
        }
        return TRUE;
    }
    return FALSE;
}

/****************************************************************************/
bool GattScanParamsSetIntervalWindow(GSPC_T *scan_params_client,
                                        uint16 scan_interval, uint16 scan_window)
{
    /* Validate the input parameters */
    if(scan_params_client == NULL)
    {
        GATT_SCAN_PARAMS_CLIENT_PANIC(("GSPC: Null instance"));
    }

    /* Check if the Scan Interval window handle has been discovered already , else this request can not be allowed */
    if(scan_params_client->scan_interval_window_handle != INVALID_SCAN_INTERVAL_WINDOW_HANDLE)
    {
        /* Update only if there is any change from the existing  scan interval and scan window */
        if((scan_params_client->scan_interval != scan_interval) ||( scan_params_client->scan_window != scan_window))
        {
            /* Update the library stored scan interval and window
                which is to be sent when a scan refresh notification arrives
            */
            scan_params_client->scan_interval = scan_interval;
            scan_params_client->scan_window = scan_window;
            /* Send Internal message for updating scan interval window onto the remote scan server */
            MessageSend((Task)&scan_params_client->lib_task, SCAN_PARAMS_CLIENT_INTERNAL_MSG_SET_SCAN, NULL);
        }
        return TRUE;
    }
    /* Scan interval window handle not yet discovered */
    return FALSE;
}

/****************************************************************************/
bool GattScanParamsClientDeInit(GSPC_T *scan_params_client)
{
    bool result = FALSE;

    /* Check parameters */
    if (scan_params_client == NULL)
    {
        GATT_SCAN_PARAMS_CLIENT_PANIC(("GSPC: Null instance"));
    }
    
    /* Register with the GATT Manager and verify the result */
    if (GattManagerUnregisterClient(&scan_params_client->lib_task) == gatt_manager_status_success)
    {
        result = TRUE;
        /* Clear pending messages */
        MessageFlushTask((Task)&scan_params_client->lib_task);
    }
    
    return result;
}

