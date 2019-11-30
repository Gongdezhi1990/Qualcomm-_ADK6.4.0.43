/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    local_device.state.c

DESCRIPTION
    State machine for local_device
*/

#include "local_device_state.h"

#include "local_device.h"
#include "local_device_app.h"
#include "local_device_bdaddr.h"
#include "local_device_ble_tx_power.h"
#include "local_device_cl.h"

#include <panic.h>
#include <stdlib.h>

typedef enum
{
    uninitialised,
    initialising,
    initialised
} local_device_state_t;

typedef struct
{
    local_device_state_t state;
    unsigned features_supported;
    unsigned feature_request;
} local_device_t;

static local_device_t local_device = {uninitialised, 
                                      (LOCAL_DEVICE_FEATURE_BLE_TX_POWER | LOCAL_DEVICE_FEATURE_BDADDR),
                                      0};

/******************************************************************************/
static void localDeviceStateUninitialised(local_device_event_t event, local_device_event_data_t* data)
{
    switch(event)
    {
        case init_request_event:
            localDeviceAppSetTask(data->init_request.app_task);
            local_device.feature_request = 0;
            if (local_device.features_supported & LOCAL_DEVICE_FEATURE_BLE_TX_POWER)
            {
                local_device.feature_request |= LOCAL_DEVICE_FEATURE_BLE_TX_POWER;
                localDeviceClReadBleAdvertisingTxPower();
            }
            if (local_device.features_supported & LOCAL_DEVICE_FEATURE_BDADDR)
            {
                local_device.feature_request |= LOCAL_DEVICE_FEATURE_BDADDR;
                localDeviceClReadBdAddr();
            }
            local_device.state = initialising;
            break;
        
        case destroy_request_event:
            /* Nothing to destroy yet */
            break;
            
        case enable_features_event:
            local_device.features_supported = data->enable_features.features;
            break;
        
        default:
            Panic();
            break;
    }
}

static void sendDeviceAppInitCfm(void)
{
    if (local_device.feature_request == 0)
    {
        local_device.state = initialised;
        localDeviceAppSendInitCfm();
    }
}

/******************************************************************************/
static void localDeviceStateInitialising(local_device_event_t event, local_device_event_data_t* data)
{
    switch(event)
    {
        case ble_tx_power_ready_event:
            localDeviceBleTxPowerReading(data->ble_tx_power_ready.tx_power);
            local_device.feature_request  &= ~LOCAL_DEVICE_FEATURE_BLE_TX_POWER;
            sendDeviceAppInitCfm();  
            break;
            
        case bdaddr_ready_event:
            localDeviceBdAddrSetRequest(data->bdaddr_ready.addr);
            local_device.feature_request &= ~LOCAL_DEVICE_FEATURE_BDADDR;
            sendDeviceAppInitCfm(); 
            break;
        
        case destroy_request_event:
            localDeviceAppSetTask(NULL);
            localDeviceBleTxPowerReset();
            local_device.state = uninitialised;
            break;
        
        default:
            Panic();
            break;
    }
}

/******************************************************************************/
static void localDeviceStateInitialised(local_device_event_t event, local_device_event_data_t* data)
{
    switch(event)
    {
        case ble_tx_power_read_request_event:
            localDeviceBleTxPowerGetRequest(&data->ble_tx_power_read_request.tx_power);
            break;
            
        case bdaddr_read_request_event:
            localDeviceBdAddrGetRequest(&data->bdaddr_read_request.addr);
            break;
        
        case destroy_request_event:
            localDeviceAppSetTask(NULL);
            localDeviceBleTxPowerReset();
            local_device.state = uninitialised;
            break;
        
        default:
            Panic();
            break;
    }
}

/******************************************************************************/
void localDeviceStateHandleEvent(local_device_event_t event, local_device_event_data_t* data)
{
    switch(local_device.state)
    {
        case uninitialised:
            localDeviceStateUninitialised(event, data);
            break;
        
        case initialising:
            localDeviceStateInitialising(event, data);
            break;
        
        case initialised:
            localDeviceStateInitialised(event, data);
            break;
        
        default:
            Panic();
            break;
    }
}
