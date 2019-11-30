/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

#include "gatt_client_private.h"
#include "gatt_client_init.h"
#include "gatt_client_discovery.h"
#include "gatt_client_msg_handler.h"

#include <gatt_manager.h>

#include <string.h>
#include <stdio.h>


/******************************************************************************/
void gattClientSendInitCfm(GGATTC *gatt_client, uint16 service_changed_handle, 
                                                gatt_client_status_t status)
{
    MAKE_GATT_CLIENT_MESSAGE(GATT_CLIENT_INIT_CFM);
    
    message->gatt_client = gatt_client;
    message->service_changed_handle = service_changed_handle;
    message->status = status;
    
    MessageSend(gatt_client->app_task, GATT_CLIENT_INIT_CFM, message);
}


/****************************************************************************/
gatt_client_status_t GattClientInit(Task app_task,
                                    GGATTC *const gatt_client, 
                                    const gatt_client_init_params_t *client_init_params,
                                    const gatt_client_device_data_t *device_data)
{
    gatt_manager_client_registration_params_t registration_params;
    bool success = FALSE;
    
    if ((app_task == NULL) || (gatt_client == NULL) || (client_init_params == NULL))
    {
        return gatt_client_status_invalid_parameter;
    }

    memset(&registration_params, 0, sizeof(gatt_manager_client_registration_params_t));
    
    /* Set memory contents to all zeros */
    memset(gatt_client, 0, sizeof(GGATTC));
    
    /* Set up library handler for external messages */
    gatt_client->lib_task.handler = gattClientMsgHandler;
    
    /* Store the Task function parameter.
       All library messages need to be sent here */
    gatt_client->app_task = app_task;
    
    /* Record and error in discovery process */
    gatt_client->discovery_error = FALSE;
    
    /* Setup data required for Gatt Service to be registered with the GATT Manager */
    registration_params.client_task = &gatt_client->lib_task;
    registration_params.cid = client_init_params->cid;
    registration_params.start_handle = client_init_params->start_handle;
    registration_params.end_handle = client_init_params->end_handle;
    /* Register with the GATT Manager and verify the result */
    if (GattManagerRegisterClient(&registration_params) == gatt_manager_status_success)
    {
        success = TRUE;
        /* If the device is already known, get the persistent data */
        if (device_data)
        {
            /* Don't need to discover data from the device; use the data supplied instead */
            gatt_client->service_changed_handle = device_data->service_changed_handle;
            gattClientSendInitCfm(gatt_client, device_data->service_changed_handle,
                                               gatt_client_status_success);
        }
        else
        {
            /* Discover all characteristics and descriptors after successful registration */           
            discoverAllGattCharacteristics(gatt_client);
        }
    }

    return success ? gatt_client_status_success : gatt_client_status_failed;
}
