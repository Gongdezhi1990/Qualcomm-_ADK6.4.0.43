/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gatt_client.h"
#include "gatt_imm_alert_client_private.h"
#include "gatt_imm_alert_client_debug.h"

#include "gatt_manager.h"

#include "gatt_imm_alert_client_msg_handler.h"

/***************************************************************************
NAME
    imm_alert_client_connect_request

DESCRIPTION
    Send Internal Message for start finding out the remote server characteristic handles for immediate alert service .
*/
static void  imm_alert_client_connect_request(GIASC_T *const imm_alert_client,
                                      const GATT_IMM_ALERT_CLIENT_INIT_PARAMS_T *const init_params)
{
    /* Make Inernal Message for fidning out characterisitc handles */
    MAKE_IMM_ALERT_CLIENT_MESSAGE(IMM_ALERT_CLIENT_INTERNAL_MSG_CONNECT);
    /* Fill in the connection ID */
    message->cid = init_params->cid;     
    /* Fill In start Handle */
    message->start_handle = init_params->start_handle;
    /* Fill in end handle  */
    message->end_handle = init_params->end_handle;
    /* Send Connect Request Internal Message */
    MessageSend((Task)&imm_alert_client->lib_task, IMM_ALERT_CLIENT_INTERNAL_MSG_CONNECT, message);
}

/****************************************************************************/
bool GattImmAlertClientInit(Task appTask , 
                                        GIASC_T *const imm_alert_client,
                                        const GATT_IMM_ALERT_CLIENT_INIT_PARAMS_T *const client_init_params,
                                        gatt_imm_alert_client_device_data_t * device_data)
{
    gatt_manager_client_registration_params_t reg_params;
    /* validate the input parameters */
    if((appTask == NULL) || (imm_alert_client == NULL) || CLIENT_INIT_PARAM_INVALID(client_init_params))
    {
        GATT_IMM_ALERT_CLIENT_PANIC(("GIASC: Invalid Initialisation parameters"));  
    }

    memset(&reg_params, 0, sizeof(gatt_manager_client_registration_params_t));

    /* Reset the alert client data structure */
    memset(imm_alert_client, 0, sizeof(GIASC_T));

    /* Keep track on application task as all the notifications and indication need to be send to here */
    imm_alert_client->app_task = appTask;
    /* Store library task for external message reception */
    imm_alert_client->lib_task.handler = imm_alert_client_msg_handler;
    /*Store the alert handle with Invalid Value */
    imm_alert_client->alert_handle = INVALID_IMM_ALERT_HANDLE;

    /* Setup GATT manager registartion parameters */
    reg_params.cid =  client_init_params->cid;
    reg_params.client_task = &imm_alert_client->lib_task;
    reg_params.end_handle = client_init_params->end_handle;
    reg_params.start_handle = client_init_params->start_handle;

    if(GattManagerRegisterClient(&reg_params) == gatt_manager_status_success)
    {
        /* If the device is already known, get the persistent data */
        if(device_data)
        {
            /* Don't need to discover data from the device; use the data supplied instead */
            imm_alert_client->alert_handle = device_data->imm_alert_handle;
            /* Report success to application */
           send_imm_alert_client_cfm(imm_alert_client, reg_params.cid, imm_alert_client->alert_handle,
                                                     gatt_imm_alert_client_status_success,IMM_ALERT_CHAR_HANDLE_CFM);
        }
        else
        {
            /* Start finding out the characteristics handles for Immediate alert service */
            imm_alert_client_connect_request(imm_alert_client,client_init_params);
        }
        return TRUE;
    } 
    
    return FALSE;

}

/****************************************************************************/
bool GattImmAlertClientSetAlertLevel(const GIASC_T *const imm_alert_client,
                                        const gatt_imm_alert_set_level alert_level)
{   
    /* Validate the input parameters */
    if(imm_alert_client == NULL)
    {
        GATT_IMM_ALERT_CLIENT_PANIC(("GIASC: Null Instance"));
    }
    /* Check the alert handle as been found out already , else this request can not be allowed */
    if((imm_alert_client->alert_handle != INVALID_IMM_ALERT_HANDLE) && !(alert_level >= gatt_imm_alert_level_reserved))
    {
        /* Make Inernal message for set alert level */
        MAKE_IMM_ALERT_CLIENT_MESSAGE(IMM_ALERT_CLIENT_INTERNAL_MSG_SET_ALERT);
        /* Fill In Alert level */
        message->alert_level = alert_level;
        /* Send Internal message for set alert level */
        MessageSend((Task)&imm_alert_client->lib_task, IMM_ALERT_CLIENT_INTERNAL_MSG_SET_ALERT, message);
        return  TRUE;
    }
    return FALSE;
}

/****************************************************************************/
bool GattImmAlertClientDestroy(GIASC_T *imm_alert_client)
{
    if (imm_alert_client == NULL)
    {
        GATT_IMM_ALERT_CLIENT_PANIC(("GIASC: Null instance"));
    }
    
    /* Unregister with the GATT Manager and verify the result */
    if (GattManagerUnregisterClient(&imm_alert_client->lib_task) == gatt_manager_status_success)
    {
        /* Clear pending messages */
        MessageFlushTask((Task)&imm_alert_client->lib_task);
        return TRUE;
    }

    return FALSE;
}




