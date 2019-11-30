/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gatt_scan_params_client_private.h"

#include "gatt_scan_params_client_discovery.h"
#include "gatt_scan_params_client_msg_handler.h"

/*******************************************************************************
 * Helper function to discover client characteristic descriptors of the service.
 */ 
static void discoverAllScanParamsCharDesc(GSPC_T *scan_params_client)
{
    gatt_manager_client_service_data_t service_data;
    memset(&service_data,0,sizeof(service_data));
    if (GattManagerGetClientData(&scan_params_client->lib_task, &service_data))
    {
        GattManagerDiscoverAllCharacteristicDescriptors(&scan_params_client->lib_task,
                                                        scan_params_client->scan_refresh_handle + 1,
                                                        service_data.end_handle);
    }
    else
    {
        GATT_SCAN_PARAMS_CLIENT_DEBUG_PANIC(("GSPC_T: Internal error\n"));
        /* Report error to application */
        scanParamsSendInitFailureCfm(scan_params_client);
    }
}

/*******************************************************************************
 * Helper function to perform next function after discovering all characteristics of the service.
 */ 
static void nextAfterDiscoverCharacteristics(GSPC_T *scan_params_client)
{
    if(scan_params_client->scan_interval_window_handle != INVALID_SCAN_INTERVAL_WINDOW_HANDLE)
    {
        if (scan_params_client->scan_refresh_handle == INVALID_SCAN_REFRESH_HANDLE)
        {
            /* No Scan Refresh characteristic found, initialisation complete */
            scanParamsSendInitSuccessCfm(scan_params_client, scan_params_client->scan_interval_window_handle, 0);
        }
        else
        {
            /* Scan Refresh characteristic found, find the descriptors */
            discoverAllScanParamsCharDesc(scan_params_client);
        }
    }
    else
    {
        /* Report error to application */
        scanParamsSendInitFailureCfm(scan_params_client);
    }
}

/*******************************************************************************
 * Helper function to process a discovered characteristic descriptor.
 */ 
static void processDiscoveredDescriptor(GSPC_T *scan_params_client, 
            const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T *cfm)
{
    if (cfm->uuid_type == gatt_uuid16)
    {
        if (cfm->uuid[0] == GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_UUID)
        {
            scanRefreshEnableNotification(scan_params_client, cfm->handle);
        }
        else
        {
            /* Report success to application as the scan refresh is invalid and is not a mandatory for Scan params */
            scanParamsSendInitSuccessCfm(scan_params_client, scan_params_client->scan_interval_window_handle, 0);
        }
    }
}

/****************************************************************************/
void discoverAllScanParamsCharacteristics(GSPC_T *scan_params_client)
{
    GattManagerDiscoverAllCharacteristics(&scan_params_client->lib_task);
}

/****************************************************************************/
void handleScanParamsDiscoverCharDescCfm(GSPC_T *scan_params_client, 
            const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T *cfm)
{
    GATT_SCAN_PARAMS_CLIENT_DEBUG_INFO(("GSPC_T: DiscoverAllDesc Rsp status=[%u] handle=[0x%x] uuid=[0x%lx] more=[%u]\n",
                            cfm->status,
                            cfm->handle,
                            cfm->uuid[0],
                            cfm->more_to_come));
    
    if (cfm->status == gatt_status_success)
    {
        processDiscoveredDescriptor(scan_params_client, cfm);
    }
    else
    {
        /* Report error to application */
        scanParamsSendInitFailureCfm(scan_params_client);
    }
}

/****************************************************************************/
void handleScanParamsDiscoverCharCfm(GSPC_T *scan_params_client,
               const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM_T *char_cfm)
{
    if(char_cfm->status == gatt_status_success)
    {
        /* Verify the char UIID is of what Scan Parameters client is interested, else ignore */
        if(char_cfm->uuid_type == gatt_uuid16)
        {
            switch(char_cfm->uuid[0])
            {
                /* Scan Interval Window*/
                case SCAN_INTERVAL_WINDOW_CHAR_UUID:
                {
                    /* Store the Scan Interval Window handle for future use */
                    scan_params_client->scan_interval_window_handle = char_cfm->handle;
                }
                break;

                /* Scan Refresh*/
                case SCAN_REFRESH_CHAR_UUID:
                {
                    /* Store the Scan refresh handle for future use */
                    scan_params_client->scan_refresh_handle = char_cfm->handle;
                }
                break;

                default:
                {
                   /* Unknown Handle, Should not reach here */
                   GATT_SCAN_PARAMS_CLIENT_DEBUG_INFO(("Func:handleScanParamsDiscoverCharCfm(),UnKnwnHandle\n"));
                }
                break;
            }
        }
        /* Ignore other char UUID's */
        if (!char_cfm->more_to_come)
        {
            nextAfterDiscoverCharacteristics(scan_params_client);
        }
    }
    else
    {
       /* Report error to application */
       scanParamsSendInitFailureCfm(scan_params_client);
    }
}

