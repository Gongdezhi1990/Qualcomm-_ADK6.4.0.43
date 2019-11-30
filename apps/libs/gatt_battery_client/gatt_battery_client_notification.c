/* Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd. */
/*  */

#include <string.h>
#include <stdio.h>

#include <gatt.h>

#include "gatt_battery_client_notification.h"

#include "gatt_battery_client_discover.h"
#include "gatt_battery_client_read.h"


/****************************************************************************
Internal functions
****************************************************************************/

/****************************************************************************/
void makeBatterySetNotificationCfmMsg(GBASC *battery_client, gatt_battery_client_status_t status)
{
    MAKE_BATTERY_MESSAGE(GATT_BATTERY_CLIENT_SET_NOTIFICATION_ENABLE_CFM);
    memset(message, 0, sizeof(GATT_BATTERY_CLIENT_SET_NOTIFICATION_ENABLE_CFM_T));
    message->battery_client = battery_client;
    message->status = status;
    MessageSend(battery_client->app_task, GATT_BATTERY_CLIENT_SET_NOTIFICATION_ENABLE_CFM, message);
    
    /* Read no longer pending */
    battery_client->pending_cmd = battery_pending_none;
}

/***************************************************************************/
void handleBatteryNotification(GBASC *battery_client, const GATT_MANAGER_REMOTE_SERVER_NOTIFICATION_IND_T *ind)
{
    if (ind->size_value && (ind->handle == battery_client->handle_level_start))
    {
        /* Send battery level to app if a value has been send in the notification message,
           and if the handle is the known as the battery level */
        MAKE_BATTERY_MESSAGE(GATT_BATTERY_CLIENT_LEVEL_IND);
        message->battery_client = battery_client;
        message->battery_level = ind->value[0];
        MessageSend(battery_client->app_task, GATT_BATTERY_CLIENT_LEVEL_IND, message);
    }
}

/****************************************************************************/
void batterySetNotificationRequest(GBASC *battery_client, bool notifications_enable, bool discover_if_unknown)
{
    bool success = FALSE;
    
    if (battery_client->handle_level_start)
    {
        /* Find client configuration descriptor */
        discoverAllCharacteristicDescriptors(battery_client, battery_client->handle_level_start + 1, battery_client->handle_level_end);
        success = TRUE;
    }
    else if (discover_if_unknown)
    {
        /* Discover all characteristics of service first to find level characteristic handle */
        discoverAllCharacteristics(battery_client);
        success = TRUE;
    }
    
    if (success)
    {
        if (notifications_enable)
            battery_client->pending_cmd = battery_pending_set_notify_enable;
        else
            battery_client->pending_cmd = battery_pending_set_notify_disable;
    }
    else
    {
        makeBatterySetNotificationCfmMsg(battery_client, gatt_battery_client_status_failed);
    }
}


/****************************************************************************
Public API
****************************************************************************/

/****************************************************************************/
void GattBatteryClientSetNotificationEnableRequest(const GBASC *battery_client, bool notifications_enable)
{
    /* Check parameters */
    if (battery_client == NULL)
    {
        GATT_BATTERY_CLIENT_PANIC(("GBASC: Invalid parameters - Set Notification Enable Request\n"));
        return;
    }

    {
        MAKE_BATTERY_MESSAGE(BATTERY_INTERNAL_MSG_SET_NOTIFICATION);
        message->notifications_enable = notifications_enable;
        MessageSendConditionally((Task)&battery_client->lib_task, BATTERY_INTERNAL_MSG_SET_NOTIFICATION, message, &battery_client->pending_cmd);
    }
}
