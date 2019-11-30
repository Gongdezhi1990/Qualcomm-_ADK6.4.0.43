/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    local_device.c

DESCRIPTION
    Interface from application to local_device
*/

#include "local_device.h"
#include "local_device_state.h"

#include <vmtypes.h>
#include <message.h>
#include <stdlib.h>

/******************************************************************************/
void LocalDeviceInit(Task task)
{
    local_device_event_data_t data;
    data.init_request.app_task = task;
    localDeviceStateHandleEvent(init_request_event, &data);
}

/******************************************************************************/
void LocalDeviceDestroy(void)
{
    localDeviceStateHandleEvent(destroy_request_event, NULL);
}

/******************************************************************************/
int8 LocalDeviceGetAdvertisingTransmitPower(void)
{
    local_device_event_data_t data;
    localDeviceStateHandleEvent(ble_tx_power_read_request_event, &data);
    return data.ble_tx_power_read_request.tx_power;
}

/******************************************************************************/
bdaddr LocalDeviceGetBdAddr(void)
{
    local_device_event_data_t data;
    localDeviceStateHandleEvent(bdaddr_read_request_event, &data);
    return data.bdaddr_read_request.addr;
}

/******************************************************************************/
void LocalDeviceEnableFeatures(uint16 features)
{
    local_device_event_data_t data;
    data.enable_features.features = features;
    localDeviceStateHandleEvent(enable_features_event, &data);
}
