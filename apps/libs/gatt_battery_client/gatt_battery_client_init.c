/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#include <string.h>
#include <stdio.h>

#include "gatt_battery_client_init.h"

#include "gatt_battery_client_discover.h"
#include "gatt_battery_client_msg_handler.h"


/******************************************************************************/
void gattBatteryInitSendInitCfm(GBASC *battery_client, uint16 handle, gatt_battery_client_status_t status)
{
    MAKE_BATTERY_MESSAGE(GATT_BATTERY_CLIENT_INIT_CFM);
    message->battery_client = battery_client;
    message->battery_level_handle = handle;
    message->status = status;
    
    MessageSend(battery_client->app_task, GATT_BATTERY_CLIENT_INIT_CFM, message);
}


/****************************************************************************/
bool GattBatteryClientInit(GBASC *battery_client, 
                           Task app_task,
                           uint16 cid,
                           uint16 start_handle,
                           uint16 end_handle,
                           gatt_battery_client_device_data_t * device_data)
{
    gatt_manager_client_registration_params_t registration_params;

    /* Check parameters */
    if ((app_task == NULL) || (battery_client == NULL))
	{
        GATT_BATTERY_CLIENT_PANIC(("GBASC: Invalid Initialisation parameters"));
        return FALSE;
	}

    memset(&registration_params, 0, sizeof(gatt_manager_client_registration_params_t));
        
    /* Set memory contents to all zeros */
    memset(battery_client, 0, sizeof(GBASC));
        
    /* Set up library handler for external messages */
    battery_client->lib_task.handler = batteryClientMsgHandler;
        
    /* Store the Task function parameter.
       All library messages need to be sent here */
    battery_client->app_task = app_task;
        
    /* Set the level characteristic start to be unknown */
    battery_client->handle_level_start = 0;
    /* Assume the level characteristic ends at the end of the service (ie. only one battery level characteristic) */
    battery_client->handle_level_end = end_handle;
        
    /* Setup data required for Battery Service to be registered with the GATT Manager */
    registration_params.client_task = &battery_client->lib_task;
    registration_params.cid = cid;
    registration_params.start_handle = start_handle;
    registration_params.end_handle = end_handle;
    /* Register with the GATT Manager and verify the result */
    if (GattManagerRegisterClient(&registration_params) == gatt_manager_status_success)
    {
        /* If the device is already known, get the persistent data, */
        if (device_data)
        {
            /* Don't need to discover data from the device; use the data supplied instead */
            battery_client->handle_level_start = device_data->battery_level_handle;
            gattBatteryInitSendInitCfm(battery_client, device_data->battery_level_handle, gatt_battery_client_status_success);
        }
        else
        {
            /* Discover all characteristics and descriptors after successful registration */
            discoverAllCharacteristics(battery_client);
            battery_client->pending_cmd = battery_pending_discover_all_characteristics;
        }
        return TRUE;
    }

    return FALSE;
}
