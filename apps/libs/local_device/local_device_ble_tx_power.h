/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    local_device_ble_tx_power.h

DESCRIPTION
    Store BLE advertising channel transmit power
*/

#ifndef LOCAL_DEVICE_BLE_TX_POWER_
#define LOCAL_DEVICE_BLE_TX_POWER_

#include <csrtypes.h>

/******************************************************************************/
void localDeviceBleTxPowerReading(int8 tx_power);

/******************************************************************************/
void localDeviceBleTxPowerGetRequest(int8* tx_power);

/******************************************************************************/
void localDeviceBleTxPowerReset(void);

#endif /* LOCAL_DEVICE_BLE_TX_POWER_ */
