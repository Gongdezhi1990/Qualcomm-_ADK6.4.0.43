/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_CLIENT_DISCOVERY_H_
#define GATT_CLIENT_DISCOVERY_H_


#include "gatt_client_private.h"

#include <gatt_manager.h>


/***************************************************************************
NAME
    discoverAllGattCharacteristics

DESCRIPTION
    Discover all characteristics for the service.
*/
void discoverAllGattCharacteristics(GGATTC *gatt_client);


/***************************************************************************
NAME
    handleDiscoverAllGattCharacteristicsResp

DESCRIPTION
    Handles GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM message for 'Discover All Characteristics' response.
*/
void handleDiscoverAllGattCharacteristicsResp(GGATTC *gatt_client, const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM_T *cfm);


/***************************************************************************
NAME
    discoverAllGattCharacteristicDescriptors

DESCRIPTION
    Discover all characteristics descriptors for the characteristic with range of start_handle to end_handle.
*/
void discoverAllGattCharacteristicDescriptors(GGATTC *gatt_client);


/***************************************************************************
NAME
    handleDiscoverAllGattCharacteristicDescriptorsResp

DESCRIPTION
    Handles GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM message  for 'Discover All Characteristic Descriptors' response..
*/
void handleDiscoverAllGattCharacteristicDescriptorsResp(GGATTC *gatt_client, const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T *cfm);


#endif
