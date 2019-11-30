/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

#include <string.h>
#include <stdio.h>

#include "gatt_client_discovery.h"
#include "gatt_client_debug.h"
#include "gatt_client_init.h"
#include "gatt_client_write.h"

#include <gatt.h>
#include <gatt_manager.h>



/*******************************************************************************
 * Helper function to record an discovery error conditions.
 */ 
static void discoverySetError(GGATTC *gatt_client)
{
    gatt_client->discovery_error = TRUE;
}


/*******************************************************************************
 * Helper function to clear discovery error conditions.
 */ 
static void discoveryClearError(GGATTC *gatt_client)
{
    gatt_client->discovery_error = FALSE;
}


/*******************************************************************************
 * Helper function to perform next function after discovering all characteristics of the service.
 */ 
static void nextAfterDiscoverCharacteristics(GGATTC *gatt_client)
{
    if (gatt_client->discovery_error)
    {
        /* Handle not set because of discovery error */
        gattClientSendInitCfm(gatt_client, 0, gatt_client_status_discovery_error);
    }
    else
    {
        if (!gatt_client->service_changed_handle)
        {
            /* No Service Changed characteristic found, initialisation complete */
            gattClientSendInitCfm(gatt_client, 0, gatt_client_status_discovery_error);
        }
        else
        {
            /* Service Changed characteristic found, find the descriptors */
            discoverAllGattCharacteristicDescriptors(gatt_client);
        }
    }
}


/*******************************************************************************
 * Helper function to process a discovered characteristic descriptor.
 */ 
static void processDiscoveredDescriptor(GGATTC *gatt_client, const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T *cfm)
{
    if (cfm->uuid_type == gatt_uuid16)
    {
        if (cfm->uuid[0] == GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_UUID)
        {
            discoveryClearError(gatt_client);
            writeGattClientConfigValue(gatt_client, cfm->handle);
        }
    }
}


/*******************************************************************************
 * Helper function to perform next function after discovering all descriptors of a characteristic.
 */ 
static void nextAfterDiscoverDescriptors(GGATTC *gatt_client)
{
    if (gatt_client->discovery_error)
    {
        /* Report discovery error */
        gattClientSendInitCfm(gatt_client, gatt_client->service_changed_handle, 
                                           gatt_client_status_discovery_error);
    }
}


/****************************************************************************/
void discoverAllGattCharacteristics(GGATTC *gatt_client)
{
    GattManagerDiscoverAllCharacteristics(&gatt_client->lib_task);
}


/****************************************************************************/
void handleDiscoverAllGattCharacteristicsResp(GGATTC *gatt_client, const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM_T *cfm)
{
    GATT_CLIENT_DEBUG_INFO(("GATTC: DiscoverAllChar Rsp status=[%u] handle=[0x%x] uuid=[0x%lx] more=[%u]\n",
                            cfm->status,
                            cfm->handle,
                            cfm->uuid[0],
                            cfm->more_to_come));
                            
    if (cfm->status == gatt_status_success)
    {
        if (cfm->uuid_type == gatt_uuid16)
        {
            if (cfm->uuid[0] == GATT_CHARACTERISTIC_UUID_SERVICE_CHANGED)
            {
                /* Service Changed UUID found so store its handle */
                gatt_client->service_changed_handle = cfm->handle;
                GATT_CLIENT_DEBUG_INFO(("GATTC: Store service changed cid=[0x%x] handle=[0x%x]\n", cfm->cid, cfm->handle));
            }
        }
    }
    else
    {
        discoverySetError(gatt_client);
    }
    
    if (!cfm->more_to_come)
    {
        nextAfterDiscoverCharacteristics(gatt_client);
    }
}


/****************************************************************************/
void discoverAllGattCharacteristicDescriptors(GGATTC *gatt_client)
{
    gatt_manager_client_service_data_t service_data;
    
    if (GattManagerGetClientData(&gatt_client->lib_task, &service_data))
    {
        /* Set error condition until the correct descriptor is found */
        discoverySetError(gatt_client);
        
        GattManagerDiscoverAllCharacteristicDescriptors(&gatt_client->lib_task,
                                                        gatt_client->service_changed_handle + 1,
                                                        service_data.end_handle);
    }
    else
    {
        GATT_CLIENT_DEBUG_PANIC(("GATTC: Internal error\n"));
        gattClientSendInitCfm(gatt_client, 0, gatt_client_status_discovery_error);
    }
}


/****************************************************************************/
void handleDiscoverAllGattCharacteristicDescriptorsResp(GGATTC *gatt_client, const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T *cfm)
{
    GATT_CLIENT_DEBUG_INFO(("GATTC: DiscoverAllDesc Rsp status=[%u] handle=[0x%x] uuid=[0x%lx] more=[%u]\n",
                            cfm->status,
                            cfm->handle,
                            cfm->uuid[0],
                            cfm->more_to_come));
    
    if (cfm->status == gatt_status_success)
    {
        processDiscoveredDescriptor(gatt_client, cfm);
    }
    
    if (!cfm->more_to_come)
    {
        nextAfterDiscoverDescriptors(gatt_client);
    }
}
