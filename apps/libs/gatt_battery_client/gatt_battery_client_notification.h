/* Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_BATTERY_CLIENT_NOTIFICATION_H_
#define GATT_BATTERY_CLIENT_NOTIFICATION_H_


#include "gatt_battery_client_private.h"


#define BATTERY_SERVICE_NOTIFICATION_VALUE 0x0001


/***************************************************************************
NAME
    makeBatterySetNotificationCfmMsg

DESCRIPTION
    Sends a GATT_BATTERY_CLIENT_SET_NOTIFICATION_ENABLE_CFM message to the application task.
*/
void makeBatterySetNotificationCfmMsg(GBASC *battery_client, gatt_battery_client_status_t status);


/***************************************************************************
NAME
    handleBatteryNotification

DESCRIPTION
    Handles the internal GATT_MANAGER_NOTIFICATION_IND message.
*/
void handleBatteryNotification(GBASC *battery_client, const GATT_MANAGER_REMOTE_SERVER_NOTIFICATION_IND_T *ind);


/***************************************************************************
NAME
    batterySendNotificationRequest

DESCRIPTION
    Deals with sending a notification request to the server. This may be getting or setting the notification configuration.
*/
void batterySendNotificationRequest(GBASC *battery_client);


/***************************************************************************
NAME
    batterySetNotificationRequest

DESCRIPTION
    Handles the internal BATTERY_INTERNAL_MSG_SET_NOTIFICATION message.
*/
void batterySetNotificationRequest(GBASC *battery_client, bool notifications_enable, bool discover_if_unknown);


#endif
