/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gatt_client.h"
#include "gatt_heart_rate_client_private.h"
#include "gatt_heart_rate_client_msg_handler.h"


/***************************************************************************
NAME
    gatt_heart_rate_client_discover_request

DESCRIPTION
    Send Internal Message for start finding out the remote server characteristic handles for HR service .
*/
static void  gatt_heart_rate_client_discover_request(GHRSC_T *const heart_rate_client,
                                      const GATT_HEART_RATE_CLIENT_INIT_PARAMS_T *const init_params)
{
    /* Make Internal Message for finding out characterisitc handles */
    MAKE_HEART_RATE_CLIENT_MESSAGE(HEART_RATE_CLIENT_INTERNAL_MSG_DISCOVER);
    /* Fill in the connection ID */
    message->cid = init_params->cid;
    /* Fill in start Handle */
    message->start_handle = init_params->start_handle;
    /* Fill in end handle  */
    message->end_handle = init_params->end_handle;
    /* Send Internal Message to discover characteristics */
    MessageSendConditionally((Task)&heart_rate_client->lib_task, HEART_RATE_CLIENT_INTERNAL_MSG_DISCOVER, 
                                            message, &heart_rate_client->pending_cmd);
}

/****************************************************************************/
bool GattHeartRateClientInit(Task appTask , 
                                        GHRSC_T *const heart_rate_client,
                                        const GATT_HEART_RATE_CLIENT_INIT_PARAMS_T *const client_init_params,
                                        const gatt_heart_rate_client_device_data_t *device_data)
{
    gatt_manager_client_registration_params_t reg_params;
    
    /* validate the input parameters */
    if((appTask == NULL) || (heart_rate_client == NULL) || CLIENT_INIT_PARAM_INVALID(client_init_params))
    {
        GATT_HEART_RATE_CLIENT_DEBUG_PANIC(("GHRC: Invalid Initialisation parameters"));
    }

    memset(&reg_params, 0, sizeof(gatt_manager_client_registration_params_t));

    /* Reset the heart rate client data structure */
    memset(heart_rate_client, 0, sizeof(GHRSC_T));
    
    /* Keep track on application task as all the notifications and indication need to be send to here */
    heart_rate_client->app_task = appTask;
    /* Store library task for external message reception */
    heart_rate_client->lib_task.handler = heart_rate_client_msg_handler;
    /*Store the HR measurement/control point handle with Invalid Value */
    heart_rate_client->hr_meas_handle = INVALID_HEART_RATE_HANDLE;
    heart_rate_client->hr_meas_ccd_handle = INVALID_HEART_RATE_HANDLE;
    heart_rate_client->hr_sensor_loc_handle = INVALID_HEART_RATE_HANDLE;
    heart_rate_client->hr_control_point_handle = INVALID_HEART_RATE_HANDLE;
    /* Set the end handle for heart rate measurement characteristics to endhandle of the service */
    heart_rate_client->hr_meas_end_handle = client_init_params->end_handle;
    /* Setup GATT manager registration parameters */
    reg_params.cid =  client_init_params->cid;
    reg_params.client_task = &heart_rate_client->lib_task;
    reg_params.end_handle = client_init_params->end_handle;
    reg_params.start_handle = client_init_params->start_handle;

    if(GattManagerRegisterClient(&reg_params) == gatt_manager_status_success)
    {
        /* If the device is already known, get the persistent data */
        if (device_data)
        {
            /* Don't need to discover data from the device; use the data supplied instead */
            heart_rate_client->hr_meas_ccd_handle = device_data->hr_meas_ccd_handle;
            heart_rate_client->hr_control_point_handle = device_data->hr_control_point_handle;
            
            /*Send init cfm to application*/
            send_heart_rate_client_init_cfm(heart_rate_client, gatt_status_success, 
                device_data->hr_sensor_location, HR_CHECK_CONTROL_POINT_SUPPORT(heart_rate_client));
        }
        else
        {
            /* Start finding out the characteristics handles for heart rate service */
            gatt_heart_rate_client_discover_request(heart_rate_client, client_init_params);
        }
        return TRUE;
    } 
    
    return FALSE;
}


/****************************************************************************/
bool GattHeartRateClientDeInit(GHRSC_T *heart_rate_client)
{
    /* Validate the Input Parameters */
    if (heart_rate_client == NULL)
    {
        GATT_HEART_RATE_CLIENT_DEBUG_PANIC(("GHRC: Null instance"));
    }

    /* Unregister with the GATT Manager and verify the result */
    if (GattManagerUnregisterClient(&heart_rate_client->lib_task) == gatt_manager_status_success)
    {  
        /* Clear pending messages */
        MessageFlushTask((Task)&heart_rate_client->lib_task);
        return TRUE;
    }
    
    return FALSE;
}

/****************************************************************************/
bool GattHeartRateRegisterForNotification(const GHRSC_T *heart_rate_client,  bool enable)
{
    /* Validate the Input Parameters */
    if (heart_rate_client == NULL)
    {
        GATT_HEART_RATE_CLIENT_DEBUG_PANIC(("GHRC: Null instance"));
    }

    if (heart_rate_client->hr_meas_ccd_handle != INVALID_HEART_RATE_HANDLE)
    {
        MAKE_HEART_RATE_CLIENT_MESSAGE(HEART_RATE_CLIENT_INTERNAL_MSG_NOTIFICATION_REQ);
        message->enable = enable;
        GATT_HEART_RATE_CLIENT_DEBUG_INFO(("Func:GattHeartRateRegisterForNotification() enable = %x\n", message->enable));

        MessageSendConditionally((Task)&heart_rate_client->lib_task, HEART_RATE_CLIENT_INTERNAL_MSG_NOTIFICATION_REQ, 
                                                message, &heart_rate_client->pending_cmd);
        return TRUE;
    }

    return FALSE;
}

bool GattHeartRateResetEnergyExpendedReq(const GHRSC_T *heart_rate_client)
{
    /* Validate the Input Parameters */
    if (heart_rate_client == NULL)
    {
        GATT_HEART_RATE_CLIENT_DEBUG_PANIC(("GHRC: Null instance"));
    }

    if (heart_rate_client->hr_control_point_handle != INVALID_HEART_RATE_HANDLE)
    {
        MessageSendConditionally((Task)&heart_rate_client->lib_task, HEART_RATE_CLIENT_INTERNAL_MSG_RESET_EE_REQ, 
                                                NULL, &heart_rate_client->pending_cmd);
        return TRUE;
    }

    return FALSE;
}

