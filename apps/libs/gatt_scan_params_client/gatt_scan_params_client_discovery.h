/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_SCAN_PARAMS_CLIENT_DISCOVERY_H_
#define GATT_SCAN_PARAMS_CLIENT_DISCOVERY_H_

#include <gatt.h> 

#include "gatt_scan_params_client.h"

/***************************************************************************
NAME
    discoverAllScanParamsCharacteristics

DESCRIPTION
    Discover all characteristics for the service.
*/
void discoverAllScanParamsCharacteristics(GSPC_T *scan_params_client);

/***************************************************************************
NAME
    handleScanParamsDiscoverCharCfm

DESCRIPTION
    Handles GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS message 
    for 'Discover All Characteristic ' response..
*/
void handleScanParamsDiscoverCharCfm(GSPC_T *scan_params_client, 
               const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM_T *cfm);

/***************************************************************************
NAME
    handleScanParamsDiscoverCharDescCfm

DESCRIPTION
    Handles GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM message 
    for 'Discover All Characteristic Descriptors' response.
*/
void handleScanParamsDiscoverCharDescCfm(GSPC_T *scan_params_client, 
                const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T *cfm);

#endif /* GATT_SCAN_PARAMS_CLIENT_DISCOVERY_H_ */
