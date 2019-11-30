/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#include <string.h>
#include <stdio.h>

#include <gatt.h>

#include "gatt_battery_client_write.h"
#include "gatt_battery_client_notification.h"


/*******************************************************************************
 * Helper function to handle a response to a write of the client configuration descriptor.
 */
static void handleWriteClientConfigResp(GBASC *battery_client, const GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM_T *write_cfm)
{
    makeBatterySetNotificationCfmMsg(battery_client, (write_cfm->status == gatt_status_success) ? gatt_battery_client_status_success : gatt_battery_client_status_failed);
}


/****************************************************************************/
void writeClientConfigValue(GBASC *battery_client, bool notifications_enable, uint16 handle)
{
    uint8 value[2];
    
    value[0] = notifications_enable ? BATTERY_SERVICE_NOTIFICATION_VALUE : 0;
    value[1] = 0;
    
    GattManagerWriteCharacteristicValue((Task)&battery_client->lib_task, handle, sizeof(value)/sizeof(uint8), value);
}


/****************************************************************************/
void handleBatteryWriteValueResp(GBASC *battery_client, const GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM_T *write_cfm)
{
    switch (battery_client->pending_cmd)
    {
        case battery_pending_write_cconfig:
        {
            handleWriteClientConfigResp(battery_client, write_cfm);
        }
        break;

        default:
        {
            /* No other pending write values expected */
            GATT_BATTERY_CLIENT_DEBUG_PANIC(("BASC: Battery write value response not expected [0x%x]\n", battery_client->pending_cmd));
        }
        break;
    }
}
