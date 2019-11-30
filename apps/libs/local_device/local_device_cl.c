/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    local_device_cl.c

DESCRIPTION
    Interface to connection library
*/

#include "local_device_cl.h"
#include "local_device_state.h"

#include <connection.h>

static void localDeviceClMessageHandler(Task task, MessageId id, Message message);

static const TaskData local_device_cl_task_data = {localDeviceClMessageHandler};

/******************************************************************************/
static int8 getTxPower(Message message)
{
    CL_DM_BLE_READ_ADVERTISING_CHANNEL_TX_POWER_CFM_T* cfm;
    cfm = (CL_DM_BLE_READ_ADVERTISING_CHANNEL_TX_POWER_CFM_T*) message;
    return cfm->tx_power;
}

/******************************************************************************/
static bdaddr getBdAddr(Message message)
{
    CL_DM_LOCAL_BD_ADDR_CFM_T* cfm;
    cfm = (CL_DM_LOCAL_BD_ADDR_CFM_T*) message;
    return cfm->bd_addr;
}

/******************************************************************************/
static void localDeviceClMessageHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    
    switch(id)
    {
        case CL_DM_BLE_READ_ADVERTISING_CHANNEL_TX_POWER_CFM:
        {
            local_device_event_data_t data;
            data.ble_tx_power_ready.tx_power = getTxPower(message);
            localDeviceStateHandleEvent(ble_tx_power_ready_event, &data);
        }
        break;
        case CL_DM_LOCAL_BD_ADDR_CFM:
        {
            local_device_event_data_t data;
            data.bdaddr_ready.addr = getBdAddr(message);
            localDeviceStateHandleEvent(bdaddr_ready_event, &data);
        }
        break;
        
        default:
        break;
    }
}

/******************************************************************************/
void localDeviceClReadBleAdvertisingTxPower(void)
{
    ConnectionDmBleReadAdvertisingChannelTxPower((Task)&local_device_cl_task_data);
}

void localDeviceClReadBdAddr(void)
{
    ConnectionReadLocalAddr((Task)&local_device_cl_task_data);
}

