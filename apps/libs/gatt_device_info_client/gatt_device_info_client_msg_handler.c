/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#include "gatt_device_info_client_private.h"

#include <gatt.h>
#include <gatt_manager.h>
#include <string.h>

#include "gatt_device_info_client_msg_handler.h"

#include "gatt_device_info_client_debug.h"

/****************************************************************************
Internal functions
****************************************************************************/
uint16 getDeviceInfoCharHandle(GDISC *const device_info_client, gatt_device_info_type_t device_info_type)
{
    if(device_info_client != NULL)
    {
        switch(device_info_type)
        {
            case gatt_device_info_client_manufacturer_name:
                return device_info_client->manufacturer_name_handle;

            case gatt_device_info_client_model_number:
                return device_info_client->model_number_handle;

            case gatt_device_info_client_serial_number:
                return device_info_client->serial_number_handle ;

            case gatt_device_info_client_hardware_revision:
                return device_info_client->hardware_revision_handle;

            case gatt_device_info_client_firmware_revision:
                return device_info_client->firmware_revision_handle;

            case gatt_device_info_client_software_revision:
                return device_info_client->software_revision_handle;

            case gatt_device_info_client_system_id:
                return device_info_client->system_id_handle;

            case gatt_device_info_client_ieee_data_list:
                return device_info_client->ieee_data_list_handle;

            case gatt_device_info_client_pnp_id:
                return device_info_client->pnp_id_handle;

            default:
                return INVALID_DEVICE_INFO_HANDLE;
        }
    }
    return INVALID_DEVICE_INFO_HANDLE;
}
/****************************************************************************/
static gatt_device_info_type_t getDeviceInfoTypeFromHandle(GDISC *const device_info_client, uint16 device_info_char_handle)
{
    GATT_DEVICE_INFO_CLIENT_DEBUG_INFO(("getDeviceInfoTypeFromHandle (Char handle= %x)\n",device_info_char_handle));
    
    if(device_info_client->manufacturer_name_handle == device_info_char_handle)
        return gatt_device_info_client_manufacturer_name;

    else if(device_info_client->model_number_handle== device_info_char_handle)
        return gatt_device_info_client_model_number;

    else if(device_info_client->serial_number_handle== device_info_char_handle)
        return gatt_device_info_client_serial_number;

    else if(device_info_client->hardware_revision_handle == device_info_char_handle)
        return gatt_device_info_client_hardware_revision;

    else if(device_info_client->firmware_revision_handle== device_info_char_handle)
        return gatt_device_info_client_firmware_revision;

    else if(device_info_client->software_revision_handle == device_info_char_handle)
        return gatt_device_info_client_software_revision;

    else if(device_info_client->system_id_handle== device_info_char_handle)
        return gatt_device_info_client_system_id;

    else if(device_info_client->ieee_data_list_handle== device_info_char_handle)
        return gatt_device_info_client_ieee_data_list;

    else if(device_info_client->pnp_id_handle== device_info_char_handle)
        return gatt_device_info_client_pnp_id;

    else
        return gatt_device_info_client_invalid_char;

}
/***************************************************************************
NAME
    supportedDeviceInfoCharHandles

DESCRIPTION
   Utility function to return bitmask of supported device info characteristics 
*/
static uint16 supportedDeviceInfoCharHandles(GDISC *const device_info_client)
{
    uint16 supported_char_mask = 0x0000;

    if(device_info_client->manufacturer_name_handle != INVALID_DEVICE_INFO_HANDLE)
        supported_char_mask |= MANUFACTURER_NAME_CHAR;

    if(device_info_client->model_number_handle != INVALID_DEVICE_INFO_HANDLE)
        supported_char_mask |= MODEL_NUMBER_CHAR;

    if(device_info_client->serial_number_handle != INVALID_DEVICE_INFO_HANDLE)
        supported_char_mask |= SERIAL_NUMBER_CHAR;

    if(device_info_client->hardware_revision_handle != INVALID_DEVICE_INFO_HANDLE)
        supported_char_mask |= HARDWARE_REVISION_CHAR;

    if(device_info_client->firmware_revision_handle != INVALID_DEVICE_INFO_HANDLE)
        supported_char_mask |= FIRMWARE_REVISION_CHAR;

    if(device_info_client->software_revision_handle != INVALID_DEVICE_INFO_HANDLE)
        supported_char_mask |= SOFTWARE_REVISION_CHAR;

    if(device_info_client->system_id_handle != INVALID_DEVICE_INFO_HANDLE)
        supported_char_mask |= SYSTEM_ID_CHAR;

    if(device_info_client->ieee_data_list_handle != INVALID_DEVICE_INFO_HANDLE)
        supported_char_mask |= IEEE_DATA_LIST_CHAR;

    if(device_info_client->pnp_id_handle!= INVALID_DEVICE_INFO_HANDLE)
        supported_char_mask |= PNP_ID_CHAR;

    return supported_char_mask;
}

/***************************************************************************
NAME
    deviceInfoClientConnect

DESCRIPTION
   Start Connection Procedure by initiating discover all characterisitc handle request with Device Info Service
*/
static void deviceInfoClientConnect(GDISC *const device_info_client)
{
    /* Start Discovering Characteristic handles */
    GattManagerDiscoverAllCharacteristics(&device_info_client->lib_task);
}

/***************************************************************************
NAME
    sendDeviceInfoClientDiscoverCharCfm

DESCRIPTION
   Utility function to send confirmation responses to application  
*/
static void sendDeviceInfoClientDiscoverCharCfm(GDISC *const device_info_client,
              const uint16 cid, gatt_device_info_client_status_t status)
{
    /* Characteristic handle discovery confirmation */
    MAKE_DEVICE_INFO_CLIENT_MESSAGE(GATT_DEVICE_INFO_CLIENT_INIT_CFM);
    /* Fill in client reference */
    message->device_info_client = device_info_client;
    /* Fill in the connection ID */
    message->cid = cid;
    /* Fill in the status */
    message->status = status;
    /* Fill in the supported device info characteristic handle mask */
    message->supported_char_mask = supportedDeviceInfoCharHandles(device_info_client);
    /* send the confirmation message to app task  */
    MessageSend(device_info_client->app_task, GATT_DEVICE_INFO_CLIENT_INIT_CFM, message);
}

/***************************************************************************
NAME
    sendDeviceInfoClientReadCharCfm

DESCRIPTION
   Utility function to send confirmation responses to application  
*/
static void sendDeviceInfoClientReadCharCfm(GDISC *const device_info_client,
                    gatt_device_info_client_status_t status,
                    const GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM_T *read_cfm)
{
    MAKE_DEVICE_INFO_CLIENT_MESSAGE_WITH_LEN(GATT_DEVICE_INFO_CLIENT_READ_CHAR_CFM,
                               read_cfm->size_value);
    /* Fill in client reference */
    message->device_info_client = device_info_client;
    message->device_info_type = getDeviceInfoTypeFromHandle(device_info_client, read_cfm->handle);
    /* Fill in the status */
    message->status = status;
    message->size = read_cfm->size_value;
    memmove(message->value, read_cfm->value, read_cfm->size_value);

    /* send the confirmation message to app task  */
    MessageSend(device_info_client->app_task, GATT_DEVICE_INFO_CLIENT_READ_CHAR_CFM, message);
}

/***************************************************************************
NAME
    handleDeviceInfoDiscoverCharCfm

DESCRIPTION
   Handles Device Information characteristics handle confirmation 
*/
static void handleDeviceInfoDiscoverCharCfm(GDISC *device_info_client,
               const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM_T*const char_cfm)
{
    if(!device_info_client)
        return;

    if(char_cfm->status == gatt_status_success)
    {
        /* Verify the char UIID is of what immediate alert client is interested, else ignore */
        if(char_cfm->uuid_type == gatt_uuid16)
        {
            switch(char_cfm->uuid[0])
            {
                case GATT_CHARACTERISTIC_UUID_MANUFACTURER_NAME_STRING:
                    device_info_client->manufacturer_name_handle = char_cfm->handle;
                    break;

                case GATT_CHARACTERISTIC_UUID_MODEL_NUMBER_STRING:
                    device_info_client->model_number_handle = char_cfm->handle;
                    break;

                case GATT_CHARACTERISTIC_UUID_SERIAL_NUMBER_STRING:
                    device_info_client->serial_number_handle = char_cfm->handle;
                    break;

                case GATT_CHARACTERISTIC_UUID_HARDWARE_REVISION_STRING:
                    device_info_client->hardware_revision_handle = char_cfm->handle;
                    break;

                case GATT_CHARACTERISTIC_UUID_FIRMWARE_REVISION_STRING:
                    device_info_client->firmware_revision_handle = char_cfm->handle;
                    break;

                case GATT_CHARACTERISTIC_UUID_SOFTWARE_REVISION_STRING:
                    device_info_client->software_revision_handle = char_cfm->handle;
                    break;

                case GATT_CHARACTERISTIC_UUID_SYSTEM_ID:
                    device_info_client->system_id_handle = char_cfm->handle;
                    break;

                case GATT_CHARACTERISTIC_UUID_IEEE_11073:
                    device_info_client->ieee_data_list_handle = char_cfm->handle;
                    break;

                case GATT_CHARACTERISTIC_UUID_PNP_ID:
                    device_info_client->pnp_id_handle = char_cfm->handle;
                    break;

                default:
                    GATT_DEVICE_INFO_CLIENT_DEBUG_INFO(("GATT: Invalid characteristic Handle received for Device Info Service\n"));
                    break;
           }

            if(char_cfm->more_to_come == FALSE)
            {
                /* Report success to application */
                sendDeviceInfoClientDiscoverCharCfm(device_info_client,char_cfm->cid,
                                                     gatt_device_info_client_status_success);
            }
        }
        /* Ignore other char UUID's */
    }
    else
    {
        if(char_cfm->more_to_come == FALSE)
        {
            /* Report failure to application */
            sendDeviceInfoClientDiscoverCharCfm(device_info_client,char_cfm->cid,
                                            gatt_device_info_client_status_failed);
        }
    }
}

/***************************************************************************/

static void deviceInfoReadCharRequest(GDISC *device_info_client,
                    const DEVICE_INFO_CLIENT_INTERNAL_MSG_READ_CHAR_T *read_char_req)
{
    /* Read Device Info Characteristic value direct, as handle is known */
    GattManagerReadCharacteristicValue((Task)&device_info_client->lib_task, read_char_req->device_info_char_handle);
}

/***************************************************************************/
static void handleDeviceInfoReadCharResp(GDISC *device_info_client, 
                const GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM_T *read_cfm)
{
    gatt_device_info_client_status_t status = gatt_device_info_client_status_failed;
    
    if(device_info_client != NULL)
    {
        GATT_DEVICE_INFO_CLIENT_DEBUG_INFO(("handleDeviceInfoReadCharResp (Size_Value= %d, Status = %d)\n",read_cfm->size_value, read_cfm->status));

        if ((read_cfm->status == gatt_status_success) && (read_cfm->size_value))
        {
            status = gatt_device_info_client_status_success;
        }
        else if ((read_cfm->status == gatt_status_invalid_handle) || (read_cfm->status == gatt_status_read_not_permitted))
        {
            status = gatt_device_info_client_status_not_allowed;
        }
        /* Send read characteristic confirmation to application */
        sendDeviceInfoClientReadCharCfm(device_info_client, status, read_cfm);
    }
}
/****************************************************************************/
static void handleGattMsg(MessageId id)
{
    UNUSED(id);

    /* Unrecognised GATT message */
    GATT_DEVICE_INFO_CLIENT_DEBUG_PANIC(("Client Gatt Msg not handled\n"));
}
/****************************************************************************/
static void handleGattManagerMsg(Task task, MessageId id, Message payload)
{
    GDISC *device_info_client = (GDISC *)task;

    GATT_DEVICE_INFO_CLIENT_DEBUG_INFO(("handleGattManagerMsg (ID= %x)\n",id));

    if (!device_info_client)
        return;

    switch (id)
    {
        case GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM:
            handleDeviceInfoDiscoverCharCfm(device_info_client, (const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM_T *)payload);
            break;

        case GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM:
            handleDeviceInfoReadCharResp(device_info_client, (const GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM_T *)payload);
            break;

        default:
            /* Unrecognised GATT message */
            GATT_DEVICE_INFO_CLIENT_DEBUG_PANIC(("Client GattMgr Msg not handled\n"));
            break;
    }
}
/****************************************************************************/
static void handleInternalDeviceInfoMsg(Task task, MessageId id, Message msg)
{
    GDISC *device_info_client = (GDISC *)task;

     GATT_DEVICE_INFO_CLIENT_DEBUG_INFO(("handleInternalDeviceInfoMsg (ID= %x)\n",id));

    if(!device_info_client)
        return;

    switch (id)
    {
        case DEVICE_INFO_CLIENT_INTERNAL_MSG_CONNECT:
            deviceInfoClientConnect(device_info_client);
            break;
        
        case DEVICE_INFO_CLIENT_INTERNAL_MSG_READ_CHAR:
            deviceInfoReadCharRequest(device_info_client, (const DEVICE_INFO_CLIENT_INTERNAL_MSG_READ_CHAR_T*)msg);
            break;
        
        default:
            /* Unrecognised DIS client internal message */
            GATT_DEVICE_INFO_CLIENT_DEBUG_PANIC(("Client Internal Msg not handled\n"));
            break;
    }
}
/****************************************************************************/
void deviceInfoClientMsgHandler(Task task, MessageId id, Message payload)
{
     GATT_DEVICE_INFO_CLIENT_DEBUG_INFO(("deviceInfoClientMsgHandler (ID= %x)\n",id));

    if ((id >= GATT_MANAGER_MESSAGE_BASE) && (id < GATT_MANAGER_MESSAGE_TOP))
    {
        handleGattManagerMsg(task, id, payload);
    }
    else if ((id >= GATT_MESSAGE_BASE) && (id < GATT_MESSAGE_TOP))
    {
        handleGattMsg(id);
    }
    else
    {
        handleInternalDeviceInfoMsg(task, id, payload);
    }
}

