/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#include <string.h>
#include <stdio.h>

#include "gatt_battery_client_private.h"

#include "gatt_battery_client_msg_handler.h"


/****************************************************************************/
bool GattBatteryClientDestroy(GBASC *battery_client)
{
    bool result = FALSE;

    /* Check parameters */
    if (battery_client == NULL)
    {
        GATT_BATTERY_CLIENT_PANIC(("GBASC: Invalid parameters - Destroy\n"));
        return FALSE;
    }

    /* Register with the GATT Manager and verify the result */
    result = (GattManagerUnregisterClient(&battery_client->lib_task) == gatt_manager_status_success);
        
    /* Clear pending messages */
    MessageFlushTask(&battery_client->lib_task);
    
    return result;
}
