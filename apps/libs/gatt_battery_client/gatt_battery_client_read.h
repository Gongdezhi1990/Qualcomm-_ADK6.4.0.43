/* Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_BATTERY_CLIENT_READ_H_
#define GATT_BATTERY_CLIENT_READ_H_


#include "gatt_battery_client_private.h"


/***************************************************************************
NAME
    makeBatteryReadDescriptorMsg

DESCRIPTION
    Sends a GATT_BATTERY_CLIENT_READ_DESCRIPTOR_CFM message to the application task.
*/
void makeBatteryReadDescriptorMsg(GBASC *battery_client, gatt_battery_client_status_t status, uint16 size_value, const uint8 *value);


/***************************************************************************
NAME
    batteryReadLevelRequest

DESCRIPTION
    Handles the internal BATTERY_INTERNAL_MSG_READ_LEVEL message.
*/
void batteryReadLevelRequest(GBASC *battery_client, uint16 level_handle, bool discover_if_unknown);


/***************************************************************************
NAME
    handleBatteryReadValueResp

DESCRIPTION
    Handles the GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM message that was sent to the battery library.
*/
void handleBatteryReadValueResp(GBASC *battery_client, const GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM_T *read_cfm);


/***************************************************************************
NAME
    batteryReadPresentationRequest

DESCRIPTION
    Handles the internal BATTERY_INTERNAL_MSG_READ_DESCRIPTOR message.
*/
void batteryReadDescriptor(GBASC *battery_client, uint16 descriptor_uuid, bool discover_if_unknown);


/***************************************************************************
NAME
    readCharacteristicValue

DESCRIPTION
    Reads characteristic value using known handle.
*/
void readCharacteristicValue(GBASC *battery_client, uint16 handle);

/***************************************************************************
NAME
    readCharacteristicDescriptorValue

DESCRIPTION
    Reads characteristic descriptor value depending on the pending function.
*/
void readCharacteristicDescriptorValue(GBASC *battery_client, uint16 handle);


#endif
