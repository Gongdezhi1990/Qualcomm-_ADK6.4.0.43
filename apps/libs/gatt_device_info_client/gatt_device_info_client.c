/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#include "gatt_device_info_client_private.h"

#include <string.h>
#include <stdio.h>

#include <gatt_manager.h>
#include "gatt_device_info_client_debug.h"
#include "gatt_device_info_client_msg_handler.h"
#include "gatt_client.h"

/***************************************************************************
NAME
    resetDeviceInfoHandles

DESCRIPTION
    Initialize all the Device info service characteristic handles to Invalid.
*/
static void resetDeviceInfoHandles( GDISC *const device_info_client)
{
    if(device_info_client != NULL)
    {
        /* Reset the Device information service characteristic handles with Invalid Value */
        device_info_client->manufacturer_name_handle = INVALID_DEVICE_INFO_HANDLE;
        device_info_client->model_number_handle= INVALID_DEVICE_INFO_HANDLE;
        device_info_client->serial_number_handle= INVALID_DEVICE_INFO_HANDLE;
        device_info_client->hardware_revision_handle= INVALID_DEVICE_INFO_HANDLE;
        device_info_client->firmware_revision_handle= INVALID_DEVICE_INFO_HANDLE;
        device_info_client->software_revision_handle= INVALID_DEVICE_INFO_HANDLE;
        device_info_client->system_id_handle= INVALID_DEVICE_INFO_HANDLE;
        device_info_client->ieee_data_list_handle = INVALID_DEVICE_INFO_HANDLE;
        device_info_client->pnp_id_handle= INVALID_DEVICE_INFO_HANDLE;
    }
}

/***************************************************************************
NAME
    deviceInfoClientDiscoverCharacteristics

DESCRIPTION
    start finding out the remote server characteristic handles for Device Info service .
*/
static void deviceInfoDiscoverCharacteristics(GDISC *const device_info_client,
                                const GATT_DEVICE_INFO_CLIENT_INIT_PARAMS_T *const client_init_params)
{
    /* Make Inernal Message for finding out characteristic handles */
    MAKE_DEVICE_INFO_CLIENT_MESSAGE(DEVICE_INFO_CLIENT_INTERNAL_MSG_CONNECT);
    /* Fill in the connection ID */
    message->cid = client_init_params->cid;
    /* Fill In start Handle */
    message->start_handle = client_init_params->start_handle;
    /* Fill in end handle  */
    message->end_handle = client_init_params->end_handle;
    /* Send Connect Request Internal Message */
    MessageSend((Task)&device_info_client->lib_task, DEVICE_INFO_CLIENT_INTERNAL_MSG_CONNECT, message);
}

/****************************************************************************/
bool GattDeviceInfoClientInit(Task app_task, 
                              GDISC *const device_info_client,  
                              const GATT_DEVICE_INFO_CLIENT_INIT_PARAMS_T *const client_init_params)
{
    gatt_manager_client_registration_params_t registration_params;

    /* Check parameters */
    if((app_task == NULL) || (device_info_client == NULL) || CLIENT_INIT_PARAM_INVALID(client_init_params))
    {
        GATT_DEVICE_INFO_CLIENT_PANIC(("GDISC: Invalid Initialisation parameters"));
        return FALSE;
    }
    /* Set memory contents to all zeros */
    memset(device_info_client, 0, sizeof(GDISC));

    /* Set up library handler for external messages */
    device_info_client->lib_task.handler = deviceInfoClientMsgHandler;
    /* Keep track on application task as all the notifications and indication need to be sent here */
    device_info_client->app_task = app_task;
    /* intialize device inforamation service characteristic handles to invalid value */
    resetDeviceInfoHandles(device_info_client);

    /* Setup data required for Device Information Service to be registered with the GATT Manager */
    registration_params.client_task = &device_info_client->lib_task;
    registration_params.cid = client_init_params->cid;
    registration_params.start_handle = client_init_params->start_handle;
    registration_params.end_handle = client_init_params->end_handle;

    /* Register with the GATT Manager and verify the result */
    if (GattManagerRegisterClient(&registration_params) == gatt_manager_status_success)
    {
        /* Start finding out the characteristics handles for Device Information service */
        deviceInfoDiscoverCharacteristics(device_info_client, client_init_params);
        return TRUE;
    }
    return FALSE;
}
/****************************************************************************/
bool GattDeviceInfoClientReadCharRequest( GDISC *device_info_client,
                                                gatt_device_info_type_t device_info_type)
{
    uint16 device_info_char_handle = INVALID_DEVICE_INFO_HANDLE;

    /* Validate the input parameters */
    if(device_info_client == NULL)
    {
        GATT_DEVICE_INFO_CLIENT_PANIC(("GDISC: Invalid parameters for ReadChar request\n"));
        return FALSE;
    }

    device_info_char_handle = getDeviceInfoCharHandle(device_info_client, device_info_type);

    if(device_info_char_handle != INVALID_DEVICE_INFO_HANDLE)
    {
        /* Make Internal message for read characteristic value reqquest */
        MAKE_DEVICE_INFO_CLIENT_MESSAGE(DEVICE_INFO_CLIENT_INTERNAL_MSG_READ_CHAR);
        /* Fill In Requested device info type */
        message->device_info_type = device_info_type;
        /* Fill In Requested device info char handle */
        message->device_info_char_handle = device_info_char_handle;

        /* Send Internal message for read device info */
        MessageSend((Task)&device_info_client->lib_task, DEVICE_INFO_CLIENT_INTERNAL_MSG_READ_CHAR, message);
        /* Return Initiated */
        return  TRUE;
    }
    return FALSE;
}

/****************************************************************************/
bool GattDeviceInfoClientDestroy(GDISC *device_info_client)
{
    bool result = FALSE;
    /* Check parameters */
    if(device_info_client == NULL)
    {
        GATT_DEVICE_INFO_CLIENT_PANIC(("GDISC: Invalid parameters during Destroy\n"));
        return FALSE;
    }

    /* Register with the GATT Manager and verify the result */
    result = (GattManagerUnregisterClient(&device_info_client->lib_task) == gatt_manager_status_success);

    /* Clear pending messages */
    MessageFlushTask((Task)&device_info_client->lib_task);
    return result;
}


