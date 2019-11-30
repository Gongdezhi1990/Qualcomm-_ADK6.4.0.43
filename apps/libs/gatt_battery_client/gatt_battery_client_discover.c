/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#include <string.h>
#include <stdio.h>

#include <gatt.h>

#include "gatt_battery_client_discover.h"

#include "gatt_battery_client_init.h"
#include "gatt_battery_client_notification.h"
#include "gatt_battery_client_read.h"
#include "gatt_battery_client_write.h"


/*******************************************************************************
 * Helper function to perform next function after discovering all characteristics of the service.
 */ 
static void nextAfterDiscoverCharacteristics(GBASC *battery_client)
{
    switch (battery_client->pending_cmd)
    {
        case battery_pending_read_level:
        {
            /* Read battery level next */
            batteryReadLevelRequest(battery_client, battery_client->handle_level_start, FALSE);
        }
        break;
        
        case battery_pending_discover_all_characteristics:
        {
            /* Discover Characteristic descriptors next as part of initialisation */
            if (battery_client->handle_level_start)
            {
                discoverAllCharacteristicDescriptors(battery_client, battery_client->handle_level_start + 1, battery_client->handle_level_end);
                battery_client->pending_cmd = battery_pending_discover_all_characteristic_descriptors;
            }
            else
            {
                gattBatteryInitSendInitCfm(battery_client, 0, gatt_battery_client_status_failed);
                battery_client->pending_cmd = battery_pending_none;
            }
        }
        break;
        
        case battery_pending_set_notify_enable:
        case battery_pending_set_notify_disable:
        {
            bool notify_is_pending = (battery_client->pending_cmd == battery_pending_set_notify_enable);

            /* Set notification next */
            batterySetNotificationRequest(battery_client, notify_is_pending, FALSE);
        }
        break;
        
        case battery_pending_discover_descriptor:
        {
            batteryReadDescriptor(battery_client, battery_client->pending_uuid, FALSE);
        }
        break;
        
        default:
        {
            GATT_BATTERY_CLIENT_DEBUG_PANIC(("BASC: No action after discover characteristics [0x%x]\n", battery_client->pending_cmd));
        }
        break;
    }
}


/*******************************************************************************
 * Helper function to process a discovered characteristic descriptor.
 */ 
static void processDiscoveredDescriptor(GBASC *battery_client, const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T *cfm)
{
    switch (battery_client->pending_cmd)
    {
        case battery_pending_discover_all_characteristic_descriptors:
        {
            /* Expected discovered descriptor, wait for more */
        }
        break;
        
        case battery_pending_discover_descriptor:
        {
            if (cfm->uuid_type == gatt_uuid16)
            {
                if (cfm->uuid[0] == battery_client->pending_uuid)
                {
                    readCharacteristicDescriptorValue(battery_client, cfm->handle);
                    
                    battery_client->pending_cmd = battery_pending_read_descriptor;
                }
            }
        }
        break;
        
        case battery_pending_set_notify_enable:
        case battery_pending_set_notify_disable:
        {
            if (cfm->uuid_type == gatt_uuid16)
            {
                if (cfm->uuid[0] == GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_UUID)
                {
                    bool notify_is_pending = (battery_client->pending_cmd == battery_pending_set_notify_enable);

                    writeClientConfigValue(battery_client, notify_is_pending, cfm->handle);

                    battery_client->pending_cmd = battery_pending_write_cconfig;
                }
            }
        }
        break;
        
        default:
        {
            GATT_BATTERY_CLIENT_DEBUG_PANIC(("BASC: Processing descriptor in wrong state [0x%x]\n", battery_client->pending_cmd));
        }
        break;
    }
}


/*******************************************************************************
 * Helper function to perform next function after discovering all descriptors of a characteristic.
 */ 
static void nextAfterDiscoverDescriptors(GBASC *battery_client)
{
    switch (battery_client->pending_cmd)
    {
        case battery_pending_discover_all_characteristic_descriptors:
        {
            battery_client->pending_cmd = battery_pending_none;
            gattBatteryInitSendInitCfm(battery_client, battery_client->handle_level_start, gatt_battery_client_status_success);
        }
        break;
        
        case battery_pending_discover_descriptor:
        {
            makeBatteryReadDescriptorMsg(battery_client, gatt_battery_client_status_not_allowed, 0, NULL);
        }
        break;
        
        case battery_pending_set_notify_enable:
        case battery_pending_set_notify_disable:
        {
            makeBatterySetNotificationCfmMsg(battery_client, gatt_battery_client_status_not_allowed);
        }
        break;
        
        case battery_pending_read_descriptor:
        {
            /* No action needed as read of descriptor will happen next */
        }
        break;
        
        case battery_pending_write_cconfig:
        {
            /* No action needed as write of client configuration descriptor will happen next */
        }
        break;

        default:
        {
            GATT_BATTERY_CLIENT_DEBUG_PANIC(("BASC: No action after discover descriptors [0x%x]\n", battery_client->pending_cmd));
        }
        break;
    }
}


/****************************************************************************/
void discoverAllCharacteristics(GBASC *battery_client)
{
    GattManagerDiscoverAllCharacteristics(&battery_client->lib_task);
}


/****************************************************************************/
void handleDiscoverAllCharacteristicsResp(GBASC *battery_client, const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM_T *cfm)
{
    if (cfm->status == gatt_status_success)
    {
        if (cfm->uuid_type == gatt_uuid16)
        {
            if (cfm->uuid[0] == GATT_CHARACTERISTIC_UUID_BATTERY_LEVEL)
            {
                if (!battery_client->handle_level_start) /* Only store battery level handle once */
                {
                    /* Battery Level UUID found so store its handle */
                    battery_client->handle_level_start = cfm->handle;
                }
            }
        }
        
        /* If by some reason another characteristic is defined,
            then ignore it, but the end handle of the stored battery level
            characteristic can be updated to end here instead of at the end
            of the service definition.
        */
        if (    battery_client->handle_level_start 
            && (battery_client->handle_level_start < cfm->handle) 
            && (cfm->handle < battery_client->handle_level_end))
        {
            battery_client->handle_level_end = cfm->handle - 1;
        }
    }
    
    if (!cfm->more_to_come)
    {
        nextAfterDiscoverCharacteristics(battery_client);
    }
}


/****************************************************************************/
void discoverAllCharacteristicDescriptors(GBASC *battery_client, uint16 start_handle, uint16 end_handle)
{
    GattManagerDiscoverAllCharacteristicDescriptors(&battery_client->lib_task,
                                                   start_handle,
                                                   end_handle);
}


/****************************************************************************/
void handleDiscoverAllCharacteristicDescriptorsResp(GBASC *battery_client, const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T *cfm)
{
    if (cfm->status == gatt_status_success)
    {
        processDiscoveredDescriptor(battery_client, cfm);
    }
    
    if (!cfm->more_to_come)
    {
        nextAfterDiscoverDescriptors(battery_client);
    }
}
