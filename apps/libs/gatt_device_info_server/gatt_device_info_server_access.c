/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */
#include <string.h>

#include "gatt_device_info_server_private.h"

#include "gatt_device_info_server_access.h"
#include "gatt_device_info_server_db.h"

/***************************************************************************
NAME
    sendDeviceInfoAccessRsp

DESCRIPTION
    Send an access response to the GATT Manager library.
*/
void sendDeviceInfoAccessRsp(Task task,
                                    uint16 cid,
                                    uint16 handle,
                                    uint16 result,
                                    uint16 size_value,
                                    const uint8 *value)
{
    if (!GattManagerServerAccessResponse(task, cid, handle, result, size_value, value))
    {
        /* The GATT Manager should always know how to send this response */
        GATT_DEVICE_INFO_SERVER_DEBUG_PANIC(("Couldn't send GATT access response\n"));
    }
}

/***************************************************************************
NAME
    sendDeviceInfoAccessErrorRsp

DESCRIPTION
    Send an error access response to the GATT Manager library.
*/
static void sendDeviceInfoAccessErrorRsp(const gdiss_t *dev_info_server, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind, uint16 error)
{
    sendDeviceInfoAccessRsp((Task)&dev_info_server->lib_task, access_ind->cid, access_ind->handle, error, 0, NULL);
}

/***************************************************************************
NAME
    handleDeviceInfoReadRequest

DESCRIPTION
    Deals with access of Device Information Service chanracteristics.
*/

static void handleDeviceInfoReadRequest(const gdiss_t *dev_info_server, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    size_t size = 0;
    uint16 result = gatt_status_success;
    const void *value = NULL;
    gatt_dis_strings_t* dis_strings = dev_info_server->dis_params.dis_strings;
    
    switch(access_ind->handle)
    {
    
        case HANDLE_MANUFACTURER_NAME:
            if((dis_strings != NULL) && (dis_strings->manufacturer_name_string != NULL))
            {
                size = strlen(dev_info_server->dis_params.dis_strings->manufacturer_name_string);
                value = dev_info_server->dis_params.dis_strings->manufacturer_name_string;
            }
            break;

        case HANDLE_MODEL_NUMBER:
            if((dis_strings != NULL) && (dis_strings->model_num_string != NULL))
            {
                size = strlen(dev_info_server->dis_params.dis_strings->model_num_string);
                value = dev_info_server->dis_params.dis_strings->model_num_string;
            }
            break;

        case HANDLE_SERIAL_NUMBER:
            if((dis_strings != NULL) && (dis_strings->serial_num_string != NULL))
            {
                size = strlen(dev_info_server->dis_params.dis_strings->serial_num_string);
                value = dev_info_server->dis_params.dis_strings->serial_num_string;
            }
            break;

        case HANDLE_HARDWARE_REVISION:
            if((dis_strings != NULL) && (dis_strings->hw_revision_string != NULL))
            {
                size = strlen(dev_info_server->dis_params.dis_strings->hw_revision_string);
                value = dev_info_server->dis_params.dis_strings->hw_revision_string;
            }
            break;

        case HANDLE_FIRMWARE_REVISION:
            if((dis_strings != NULL) && (dis_strings->fw_revision_string != NULL))
            {
                size = strlen(dev_info_server->dis_params.dis_strings->fw_revision_string);
                value = dev_info_server->dis_params.dis_strings->fw_revision_string;
            }
            break;

        case HANDLE_SOFTWARE_REVISION:
            if((dis_strings != NULL) && (dis_strings->sw_revision_string != NULL))
            {
                size = strlen(dev_info_server->dis_params.dis_strings->sw_revision_string);
                value = dev_info_server->dis_params.dis_strings->sw_revision_string;
            }
            break;

        case HANDLE_SYSTEM_ID:
            if(dev_info_server->dis_params.system_id !=NULL)
            {
                size = sizeof(dev_info_server->dis_params.system_id);
                value = dev_info_server->dis_params.system_id;
            }
            break;

        case HANDLE_IEEE_DATA:
            if(dev_info_server->dis_params.ieee_data !=NULL)
            {
                size = sizeof(dev_info_server->dis_params.ieee_data);
                value = dev_info_server->dis_params.ieee_data;
            }
            break;

        case HANDLE_PNP_ID:
            if(dev_info_server->dis_params.pnp_id !=NULL)
            {
                size = sizeof(dev_info_server->dis_params.pnp_id);
                value = dev_info_server->dis_params.pnp_id;
            }
            break;

         default:
            break;
    }
    sendDeviceInfoAccessRsp((Task)&dev_info_server->lib_task, access_ind->cid, access_ind->handle, result, (uint16)size, value);
}

/***************************************************************************
NAME
    handleDeviceInfoServerAccess

DESCRIPTION
    Deals with access of Device Information Service chanracteristics.
*/
void handleDeviceInfoServerAccess(gdiss_t *dev_info_server, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    GATT_DEVICE_INFO_SERVER_DEBUG_INFO((" Func:handleDeviceInfoServerAccess(), Access Ind flags = %x \n",access_ind->flags));
    
    if (access_ind->flags & ATT_ACCESS_READ)
    {
        handleDeviceInfoReadRequest(dev_info_server, access_ind);
    }
    else if (access_ind->flags & ATT_ACCESS_WRITE)
    {
        /* Write is not allowed. */
        sendDeviceInfoAccessErrorRsp(dev_info_server, access_ind, gatt_status_write_not_permitted);
    }
    else
    {
        /* Reject access requests that aren't read/write, which shouldn't happen. */
        sendDeviceInfoAccessErrorRsp(dev_info_server, access_ind, gatt_status_request_not_supported);
    }
}

