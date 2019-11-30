/* Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_BATTERY_CLIENT_DISCOVER_H_
#define GATT_BATTERY_CLIENT_DISCOVER_H_

#include "gatt_battery_client_private.h"

/***************************************************************************
NAME
    discoverAllCharacteristics

DESCRIPTION
    Discover all characteristics for the service.
*/
void discoverAllCharacteristics(GBASC *battery_client);


/***************************************************************************
NAME
    handleDiscoverAllCharacteristicsResp

DESCRIPTION
    Handles GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM message for 'Discover All Characteristics' response.
*/
void handleDiscoverAllCharacteristicsResp(GBASC *battery_client, const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM_T *cfm);


/***************************************************************************
NAME
    discoverAllCharacteristics

DESCRIPTION
    Discover all characteristics descriptors for the characteristic with range of start_handle to end_handle.
*/
void discoverAllCharacteristicDescriptors(GBASC *battery_client, uint16 start_handle, uint16 end_handle);


/***************************************************************************
NAME
    handleDiscoverAllCharacteristicDescriptorsResp

DESCRIPTION
    Handles GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM message  for 'Discover All Characteristic Descriptors' response..
*/
void handleDiscoverAllCharacteristicDescriptorsResp(GBASC *battery_client, const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T *cfm);


#endif
