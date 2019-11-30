/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#include <string.h>
#include <stdio.h>

#include <gatt.h>

/* Include battery h files */
#include "gatt_battery_client_read.h"

#include "gatt_battery_client_discover.h"
#include "gatt_battery_client_notification.h"
#include "gatt_battery_client_write.h"


/*******************************************************************************
 * Helper function to send a GATT_BATTERY_CLIENT_READ_LEVEL_CFM message to the application task.
 */
static void makeBatteryReadLevelMsg(GBASC *battery_client, gatt_battery_client_status_t status, uint8 battery_level)
{
    MAKE_BATTERY_MESSAGE(GATT_BATTERY_CLIENT_READ_LEVEL_CFM);
    memset(message, 0, sizeof(GATT_BATTERY_CLIENT_READ_LEVEL_CFM_T));
    message->battery_client = battery_client;
    message->status = status;
    
    if (status == gatt_battery_client_status_success)
    {
        message->battery_level = battery_level;
    }
    
    MessageSend(battery_client->app_task, GATT_BATTERY_CLIENT_READ_LEVEL_CFM, message);
    
    battery_client->pending_cmd = battery_pending_none;
}

  
/*******************************************************************************
 * Helper function to handle Read By Value response, when the read is on the battery level characteristic.
 */  
static void handleReadValueLevelResp(GBASC *battery_client, const GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM_T *read_cfm)
{
    gatt_battery_client_status_t status = gatt_battery_client_status_failed;
    uint8 battery_level = 0;
    
    if ((read_cfm->status == gatt_status_success) && read_cfm->size_value)
    {
        /* Battery level should be 8-bit value returned */
        status = gatt_battery_client_status_success;
        battery_level = read_cfm->value[0];
    }
    else if ((read_cfm->status == gatt_status_invalid_handle) || (read_cfm->status == gatt_status_read_not_permitted))
    {
        status = gatt_battery_client_status_not_allowed;
    }
    
    /* Send read level message to application */
    makeBatteryReadLevelMsg(battery_client, status, battery_level);
}

/*******************************************************************************
 * Helper function to handle Read By Value response, when the read is on the battery level descriptor.
 */  
static void handleReadValueDescriptorResp(GBASC *battery_client, const GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM_T *read_cfm)
{
    gatt_battery_client_status_t status = gatt_battery_client_status_failed;
    
    if ((read_cfm->status == gatt_status_success) && read_cfm->size_value)
    {
        /* Battery level should be 8-bit value returned */
        status = gatt_battery_client_status_success;
    }
    else if ((read_cfm->status == gatt_status_invalid_handle) || (read_cfm->status == gatt_status_read_not_permitted))
    {
        status = gatt_battery_client_status_not_allowed;
    }
    
    /* Send read descriptor message to application */
    makeBatteryReadDescriptorMsg(battery_client, status, read_cfm->size_value, read_cfm->value);
}


/****************************************************************************
Internal functions
****************************************************************************/

/*******************************************************************************/
void makeBatteryReadDescriptorMsg(GBASC *battery_client, gatt_battery_client_status_t status, uint16 size_value, const uint8 *value)
{
    MAKE_BATTERY_MESSAGE_WITH_LEN(GATT_BATTERY_CLIENT_READ_DESCRIPTOR_CFM, size_value);
    memset(message, 0, sizeof(GATT_BATTERY_CLIENT_READ_DESCRIPTOR_CFM_T) + size_value - sizeof(uint8));
    message->battery_client = battery_client;
    message->status = status;
    message->descriptor_uuid = battery_client->pending_uuid;
    
    if (status == gatt_battery_client_status_success)
    {
        message->size_value = size_value;
        memmove(message->value, value, size_value);
    }
    
    MessageSend(battery_client->app_task, GATT_BATTERY_CLIENT_READ_DESCRIPTOR_CFM, message);
    
    /* No more pending commands */
    battery_client->pending_uuid = 0;
    battery_client->pending_cmd = 0;
}

/***************************************************************************/
void batteryReadLevelRequest(GBASC *battery_client, uint16 level_handle, bool discover_if_unknown)
{
    bool success = FALSE;
    
    if (level_handle)
    {
        /* Read battery level value direct, as handle is known */
        readCharacteristicValue(battery_client, level_handle);
        success = TRUE;
    }
    else if (discover_if_unknown)
    {
        /* Discover all characteristics of service first to find level characteristic handle */
        discoverAllCharacteristics(battery_client);
        success = TRUE;
    }
    
    if (success)
    {
        /* Store pending read so that only one operation happens at a time */
        battery_client->pending_cmd = battery_pending_read_level;
    }
    else
    {
        /* Send read level message to application */
        makeBatteryReadLevelMsg(battery_client, gatt_battery_client_status_not_allowed, 0);
    }
}


/****************************************************************************/
void handleBatteryReadValueResp(GBASC *battery_client, const GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM_T *read_cfm)
{
    switch (battery_client->pending_cmd)
    {
        case battery_pending_read_level:
        {
            handleReadValueLevelResp(battery_client, read_cfm);
        }
        break;
        
        case battery_pending_read_descriptor:
        {
            handleReadValueDescriptorResp(battery_client, read_cfm);
        }
        break;

        default:
        {
            /* No other pending read values expected */
            GATT_BATTERY_CLIENT_DEBUG_PANIC(("BASC: Battery read value response not expected [0x%x]\n", battery_client->pending_cmd));
        }
        break;
    }
}


/***************************************************************************/
void batteryReadDescriptor(GBASC *battery_client, uint16 descriptor_uuid, bool discover_if_unknown)
{
    bool success = FALSE;
    
    battery_client->pending_uuid = descriptor_uuid;
    
    if (battery_client->handle_level_start)
    {
        discoverAllCharacteristicDescriptors(battery_client, battery_client->handle_level_start + 1, battery_client->handle_level_end);
        success = TRUE;
    }
    else if (discover_if_unknown)
    {
        discoverAllCharacteristics(battery_client);
        success = TRUE;
    }
    
    if (success)
    {
        /* Store pending read so that only one operation happens at a time */
        battery_client->pending_cmd = battery_pending_discover_descriptor;
    }
    else
    {
        /* Send read level message to application */
        makeBatteryReadDescriptorMsg(battery_client, gatt_battery_client_status_not_allowed, 0, NULL);
    }
}

/****************************************************************************/
void readCharacteristicValue(GBASC *battery_client, uint16 handle)
{   
    GattManagerReadCharacteristicValue((Task)&battery_client->lib_task, handle);
}

/****************************************************************************/
void readCharacteristicDescriptorValue(GBASC *battery_client, uint16 handle)
{
    GattManagerReadCharacteristicValue((Task)&battery_client->lib_task, handle);
}


/****************************************************************************
Public API
****************************************************************************/

/****************************************************************************/
void GattBatteryClientReadLevelRequest(const GBASC *battery_client)
{
    if (battery_client == NULL)
    {
        GATT_BATTERY_CLIENT_PANIC(("GBASC: Invalid parameters - Read Level Request\n"));
    }
    MessageSendConditionally((Task)&battery_client->lib_task, BATTERY_INTERNAL_MSG_READ_LEVEL, NULL, &battery_client->pending_cmd);
}


/****************************************************************************/
void GattBatteryClientReadDescriptorRequest(const GBASC *battery_client, uint16 descriptor_uuid)
{
    /* Check parameters */
    if (battery_client == NULL)
    {
        GATT_BATTERY_CLIENT_PANIC(("GBASC: Invalid parameters - Read Descriptor Request\n"));
        return;
    }
    
    {
        MAKE_BATTERY_MESSAGE(BATTERY_INTERNAL_MSG_READ_DESCRIPTOR);
        message->descriptor_uuid = descriptor_uuid;
        MessageSendConditionally((Task)&battery_client->lib_task, BATTERY_INTERNAL_MSG_READ_DESCRIPTOR, message, &battery_client->pending_cmd);
    }
}
/****************************************************************************/
