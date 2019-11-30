/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#include <string.h>
#include <stdio.h>

#include "gatt_battery_server_private.h"

#include "gatt_battery_server_msg_handler.h"
#include "gatt_battery_server_db.h"


/****************************************************************************/
bool GattBatteryServerInit(GBASS *battery_server, 
                           Task app_task,
                           const gatt_battery_server_init_params_t *init_params,
                           uint16 start_handle,
                           uint16 end_handle)
{
    gatt_manager_server_registration_params_t registration_params;

    if ((app_task == NULL) || (battery_server == NULL))
	{
        GATT_BATTERY_SERVER_PANIC(("GBASS: Invalid Initialisation parameters"));
        return FALSE;
	}
	
    /* Set up library handler for external messages */
    battery_server->lib_task.handler = batteryServerMsgHandler;
        
    /* Store the Task function parameter.
       All library messages need to be sent here */
    battery_server->app_task = app_task;
        
    /* Check optional initialisation parameters */
    /* When GATT_MANAGER_USE_CONST_DB is enabled then it is the callers responsibility
     * to register the appropriate GATT battery server configuration when the 
     * const database is registered.
     */
    if (init_params)
    {
        /* Store notifications enable flag */
        battery_server->notifications_enabled = init_params->enable_notifications;
    }
    else
    {
        battery_server->notifications_enabled = FALSE;
    }
        
    /* Setup data required for Battery Service to be registered with the GATT Manager */
    registration_params.task = &battery_server->lib_task;
    registration_params.start_handle = start_handle;
    registration_params.end_handle = end_handle;
        
    /* Register with the GATT Manager and verify the result */
    return (GattManagerRegisterServer(&registration_params) == gatt_manager_status_success);
}
