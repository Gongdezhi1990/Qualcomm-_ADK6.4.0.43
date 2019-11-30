/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    local_device_ble_tx_power.c

DESCRIPTION
    Store BLE advertising channel transmit power
*/

#include "local_device_ble_tx_power.h"

static int8 ble_tx_power;

/******************************************************************************/
void localDeviceBleTxPowerReading(int8 tx_power)
{
    ble_tx_power = tx_power;
}

/******************************************************************************/
void localDeviceBleTxPowerGetRequest(int8* tx_power)
{
    *tx_power = ble_tx_power;
} 

/******************************************************************************/
void localDeviceBleTxPowerReset(void)
{
    localDeviceBleTxPowerReading(0);
}
