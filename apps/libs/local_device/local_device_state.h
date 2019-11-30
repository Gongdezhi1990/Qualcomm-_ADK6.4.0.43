/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    local_device_state.h

DESCRIPTION
    State machine for local_device
*/

#ifndef LOCAL_DEVICE_STATE_
#define LOCAL_DEVICE_STATE_

#include <bdaddr.h>
#include <message.h>

typedef enum
{
    init_request_event,
    destroy_request_event,
    ble_tx_power_ready_event,
    ble_tx_power_read_request_event,
    bdaddr_ready_event,
    bdaddr_read_request_event,
    enable_features_event
} local_device_event_t;

typedef struct
{
    Task app_task;
} init_request_event_t;

typedef struct
{
    int8 tx_power;
} ble_tx_power_ready_event_t;

typedef struct
{
    int8 tx_power;
} ble_tx_power_read_request_event_t;

typedef struct
{
    bdaddr addr;
} bdaddr_ready_event_t;

typedef struct
{
    bdaddr addr;
} bdaddr_read_request_event_t;

typedef struct
{
    uint16 features;
} enable_features_event_t;

typedef union
{
    init_request_event_t                init_request;
    ble_tx_power_ready_event_t          ble_tx_power_ready;
    ble_tx_power_read_request_event_t   ble_tx_power_read_request;
    bdaddr_ready_event_t                bdaddr_ready;
    bdaddr_read_request_event_t         bdaddr_read_request;
    enable_features_event_t            enable_features;
} local_device_event_data_t;

/******************************************************************************/
void localDeviceStateHandleEvent(local_device_event_t event, local_device_event_data_t* data);

#endif /* LOCAL_DEVICE_STATE_ */
