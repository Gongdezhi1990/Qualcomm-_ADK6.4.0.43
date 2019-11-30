/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gatt_heart_rate_client_private.h"

#include "gatt_heart_rate_client_msg_handler.h"

/***************************************************************************
NAME
    heart_rate_get_status

DESCRIPTION
   Utility function to map gatt status to gatt heart rate status
*/
static gatt_heart_rate_client_status heart_rate_get_status(const gatt_status_t status)
{
    gatt_heart_rate_client_status hr_status;
    
    switch(status)
    {
        case gatt_status_success:
            hr_status = gatt_heart_rate_client_status_success;
            break;
            
        case gatt_status_invalid_cid:
            hr_status = gatt_heart_rate_client_status_no_connection;
            break;
            
        case gatt_status_read_not_permitted:
        case gatt_status_write_not_permitted:
            hr_status = gatt_heart_rate_client_status_not_allowed;
            break;
            
        default:
            hr_status = gatt_heart_rate_client_status_failed;
            break;            
    }
    
    return hr_status;
}
/***************************************************************************
NAME
    send_heart_rate_client_init_cfm

DESCRIPTION
   Utility function to send init confirmation to application  
*/
void send_heart_rate_client_init_cfm(GHRSC_T *const heart_rate_client,
                                                const gatt_status_t status,
                                                uint8 sensor_location,
                                                bool control_point_support)
{
    MAKE_HEART_RATE_CLIENT_MESSAGE(GATT_HEART_RATE_CLIENT_INIT_CFM);

    /* Fill in client reference */
    message->heart_rate_client = heart_rate_client;     
    /* Fill in sensor location */
    message->hr_sensor_location = sensor_location;     
    /* Fill in control point support info */
    message->hr_control_point_support = (uint16) control_point_support;     
    /* Fill in the status */
    message->status = heart_rate_get_status(status);
    /* send the confirmation message to app task  */
    MessageSend(heart_rate_client->app_task, GATT_HEART_RATE_CLIENT_INIT_CFM, message);

    /*Clear the pending flag*/
    CLEAR_PENDING_FLAG(heart_rate_client->pending_cmd);
}

/***************************************************************************
NAME
    send_heart_rate_client_notification_cfm

DESCRIPTION
   Utility function to send notification confirmation to application  
*/
static void send_heart_rate_client_notification_cfm(GHRSC_T *const heart_rate_client,
                                                const gatt_status_t status)
{
    MAKE_HEART_RATE_CLIENT_MESSAGE(GATT_HEART_RATE_CLIENT_NOTIFICATION_CFM);

    /* Fill in client reference */
    message->heart_rate_client = heart_rate_client;     
    /* Fill in the status */
    message->status = heart_rate_get_status(status);
    /* Send the confirmation message to app task  */
    MessageSend(heart_rate_client->app_task, GATT_HEART_RATE_CLIENT_NOTIFICATION_CFM, message);
}

/***************************************************************************
NAME
    send_heart_rate_client_reset_ee_cfm

DESCRIPTION
   Utility function to send reset energy expended confirmation to application  
*/
static void send_heart_rate_client_reset_ee_cfm(GHRSC_T *const heart_rate_client,
                                                const gatt_status_t status)
{
    MAKE_HEART_RATE_CLIENT_MESSAGE(GATT_HEART_RATE_CLIENT_RESET_EE_CFM);

    /* Fill in client reference */
    message->heart_rate_client = heart_rate_client;     
    /* Fill in the status */
    message->status = heart_rate_get_status(status);
    /* Send the confirmation message to app task  */
    MessageSend(heart_rate_client->app_task, GATT_HEART_RATE_CLIENT_RESET_EE_CFM, message);
}

/***************************************************************************
NAME
    heart_rate_discover_char_descriptors

DESCRIPTION
   Discover heart rate characteristics descriptors
*/
static void heart_rate_discover_char_descriptors(GHRSC_T *const heart_rate_client)
{
    GattManagerDiscoverAllCharacteristicDescriptors(&heart_rate_client->lib_task,
                                                    heart_rate_client->hr_meas_handle + 1,
                                                    heart_rate_client->hr_meas_end_handle);
}

 /***************************************************************************
NAME
    handle_heart_rate_discover_char_descriptors_rsp

DESCRIPTION
   Process heart rate characteristics descriptors response
*/
static void handle_heart_rate_discover_char_descriptors_rsp(GHRSC_T *const heart_rate_client,
                            const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T *cfm)
{   
    if ((cfm->status == gatt_status_success) && (cfm->uuid_type == gatt_uuid16))
    {
        if (cfm->uuid[0] == GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_UUID)
        {
            /* Store the handle to register for notifications later */
            heart_rate_client->hr_meas_ccd_handle = cfm->handle;
            GATT_HEART_RATE_CLIENT_DEBUG_INFO(("CCD Handle = %x \n", heart_rate_client->hr_meas_ccd_handle));

            /* Read the Body sensor location and inform the app in init_cfm on read response */
            if (heart_rate_client->hr_sensor_loc_handle != INVALID_HEART_RATE_HANDLE)
            {        
                GattManagerReadCharacteristicValue((Task)&heart_rate_client->lib_task, heart_rate_client->hr_sensor_loc_handle);
            }
            else
            {     
                /* Read HR measurement characteristic descriptor value */
                GattManagerReadCharacteristicValue((Task)&heart_rate_client->lib_task, heart_rate_client->hr_meas_ccd_handle);

                /*Send init cfm, as no need to wait for above read cfm*/
                send_heart_rate_client_init_cfm(heart_rate_client, gatt_status_success, 
                                    HR_BODY_SENSOR_LOCATION_UNKNOWN, /*not supported by sensor*/
                                    HR_CHECK_CONTROL_POINT_SUPPORT(heart_rate_client));
            }
        }
        /*Ignore other descriptors*/
    }    
    
    if (!cfm->more_to_come)
    {
        /*Sending failure as mandatory characteristics is not found*/
        if (heart_rate_client->hr_meas_ccd_handle == INVALID_HEART_RATE_HANDLE)
        {
            GATT_HEART_RATE_CLIENT_DEBUG_INFO(("GHRSC: Internal error on char desc rsp\n"));
            /* Report error to application */
            send_heart_rate_client_init_cfm(heart_rate_client, gatt_status_failure, 
                                HR_BODY_SENSOR_LOCATION_UNKNOWN, HR_CONTROL_POINT_NOT_SUPPORTED);
        }
    }
}


 /***************************************************************************
NAME
    handle_heart_rate_read_response_cfm

DESCRIPTION
   Handle heart rate read response
*/
static void handle_heart_rate_read_response_cfm(GHRSC_T *const heart_rate_client,
                            const GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM_T *cfm)
{
    GATT_HEART_RATE_CLIENT_DEBUG_INFO(("Func:handle_heart_rate_read_response_cfm() status = %x \n",cfm->status));

    if ((heart_rate_client->hr_sensor_loc_handle != INVALID_HEART_RATE_HANDLE)
        && (cfm->handle == heart_rate_client->hr_sensor_loc_handle))
    {
        GATT_HEART_RATE_CLIENT_DEBUG_INFO(("\nHeart rate Sensor location = %x \n",cfm->value[0]));
        
        /*Send init cfm to application*/
        send_heart_rate_client_init_cfm(heart_rate_client, gatt_status_success, 
                                cfm->value[0], HR_CHECK_CONTROL_POINT_SUPPORT(heart_rate_client));
    }
}
 
/***************************************************************************
NAME
    handle_heart_rate_discover_char_cfm

DESCRIPTION
   Handles heart rate discover char confirmation 
*/
static void handle_heart_rate_discover_char_cfm(GHRSC_T *const heart_rate_client,
               const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM_T*const char_cfm)
{
    if ((heart_rate_client != NULL) && (char_cfm->status == gatt_status_success))
    {
        /* Verify the char UUID is of interest to HRP, else ignore */
        if(char_cfm->uuid_type == gatt_uuid16)
        {
            switch(char_cfm->uuid[0])
            {
                /* Heart Rate measurement */
                case GATT_CHARACTERISTIC_UUID_HEART_RATE_MEASUREMENT:
                {
                    /* Store the HR measurement level handle for setting notification */
                    heart_rate_client->hr_meas_handle = char_cfm->handle;
                }
                break;

                /* Body Sensor Location */
                case GATT_CHARACTERISTIC_UUID_BODY_SENSOR_LOCATION:
                {
                    /* The heart rate sensor location is static during the connection.*/
                    heart_rate_client->hr_sensor_loc_handle = char_cfm->handle;
                }
                break;
                
                /* Heart rate control point */
                case GATT_CHARACTERISTIC_UUID_HEART_RATE_CONTROL_POINT:
                {
                    /* Store the HR control point handle to reset energy expended field on request from app */
                    heart_rate_client->hr_control_point_handle = char_cfm->handle;
                }
                break;

                default:
                {
                   /* Unknown Handle, Should not reach here */
                   GATT_HEART_RATE_CLIENT_DEBUG_INFO(("Func:handle_heart_rate_discover_char_cfm(), UnKnownHandle\n"));
                }
                break;
            }

            /* Update end handle for heart rate measurement characteristics */
            if ((heart_rate_client->hr_meas_handle!= INVALID_HEART_RATE_HANDLE)  &&
                (char_cfm->handle > heart_rate_client->hr_meas_handle) && 
                (char_cfm->handle < heart_rate_client->hr_meas_end_handle))
            {
                heart_rate_client->hr_meas_end_handle = char_cfm->handle-1;
            }
        }
        
        if (!char_cfm->more_to_come)
        {
            heart_rate_discover_char_descriptors(heart_rate_client);
        }
    }
    else
    {
        /* Report error to application */
        send_heart_rate_client_init_cfm(heart_rate_client, char_cfm->status, 
                            HR_BODY_SENSOR_LOCATION_UNKNOWN, HR_CONTROL_POINT_NOT_SUPPORTED);
    }
}

/***************************************************************************
NAME
    handle_heart_rate_write_response_cfm

DESCRIPTION
   Handles heart rate write response confirmation 
*/
static void handle_heart_rate_write_response_cfm(GHRSC_T *const heart_rate_client,
              const GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM_T *const write_cfm)
{
    if (heart_rate_client != NULL)
    {
        if (heart_rate_client->pending_cmd == heart_rate_client_write_pending_reset_ee)
        {
            send_heart_rate_client_reset_ee_cfm(heart_rate_client, write_cfm->status);
        }
        else
        {
            send_heart_rate_client_notification_cfm(heart_rate_client, write_cfm->status);
        }
        
        CLEAR_PENDING_FLAG(heart_rate_client->pending_cmd);
    }    
}

/***************************************************************************
NAME
    find_heart_rate_rrinterval_len

DESCRIPTION
   Find heart rate notify length based on flags RR interval present, HR measurment format and EE present
   Return length in bytes
*/
static uint8 find_heart_rate_rrinterval_len(uint8 flag, uint16 size_value)
{
    unsigned rr_len=0;

    /* Check if RR interval values are present from the flag */
    if(flag & HR_RR_INTERVAL_PRESENT)
    {
        /* Based on the HR measurement format of uint16 or uint8 the rr_len would change */
        if (flag & HR_MEASUREVAL_FORMAT_UINT16)
        {
            rr_len = size_value -NOTIFY_LEN_WITHOUT_RRINTERVAL-1;
        }
        else
        {
            rr_len = size_value -NOTIFY_LEN_WITHOUT_RRINTERVAL;
        }

        /* Finally check if Energy expended value is present. 
            If present, substract 2 bytes as it is a UINT16 value received as uint8 */
        if (flag & HR_ENERGY_EXP_PRESENT)
        {
            rr_len = rr_len-2;/*EE in uint8*/
        }
    }

    return (uint8)rr_len;
}

/***************************************************************************
NAME
    handle_heart_rate_client_notification_ind

DESCRIPTION
   Handles heart rate notification ind from peer
*/
static void handle_heart_rate_client_notification_ind(GHRSC_T *const heart_rate_client,
              const GATT_MANAGER_REMOTE_SERVER_NOTIFICATION_IND_T *const ind)
{  
    if ((heart_rate_client != NULL) && (heart_rate_client->notification_enabled) && (ind->size_value))
    {
        uint8    count=0; /*to extract the ind values*/
        uint8    rr_len=0; /*to calculate the message length based on rr_interval present*/
        uint8    flags;
        unsigned alloc_len=0;

#ifdef GATT_HEART_RATE_CLIENT_DEBUG_LIB
        uint8 n; /*to print notification ind values*/

        for (n=0; n<ind->size_value;n++)
        {
           GATT_HEART_RATE_CLIENT_DEBUG_INFO(("\n Heart rate notification ind = [0x%x] \n",ind->value[n]));
        }
#endif

        /* Get heart rate flags - first element of notification value */
        flags = ind->value[count];

        /* Calculate allocation length */
        rr_len = find_heart_rate_rrinterval_len(flags, ind->size_value);
        alloc_len =  (rr_len)?(rr_len-sizeof(uint8)):(0);
        GATT_HEART_RATE_CLIENT_DEBUG_INFO(("\n RR interval length [%d], Alloc length [%d] \n", rr_len, alloc_len));

        /* Allocate and fill notification indication message to be sent to app */
        {               
            MAKE_HEART_RATE_CLIENT_MESSAGE_WITH_LEN(GATT_HEART_RATE_CLIENT_NOTIFICATION_IND, alloc_len);

            /* Fill in client reference */
            message->heart_rate_client = heart_rate_client;
                
            /* Fill in the flags */
            message->heart_rate_characteristic_flags = (uint16) flags;
            
            /* Fill in the HR measurement value */
            if (flags & HR_MEASUREVAL_FORMAT_UINT16)
            {
                HR_FILL_HIGH_AND_LOW_BYTE(count,  ind->value, message->heart_rate_value);
            }
            else
            {                
                message->heart_rate_value = (uint16) ind->value[++count];
            }

            /* Fill in the HR Energy Expended value, if present */
            if (flags & HR_ENERGY_EXP_PRESENT)
            {
                HR_FILL_HIGH_AND_LOW_BYTE(count,  ind->value, message->energy_expended);
            }

            /* Fill in the HR RR interval value, if present */
            if (flags & HR_RR_INTERVAL_PRESENT)
            {
                message->size_rr_interval = rr_len; 
                memmove(message->rr_interval, &ind->value[++count], rr_len);
            }

            /* send the confirmation message to app task  */
            MessageSend(heart_rate_client->app_task, GATT_HEART_RATE_CLIENT_NOTIFICATION_IND, message);
        }
    }
}

/***************************************************************************
NAME
    handle_heart_rate_client_gatt_manager_msg

DESCRIPTION
    Handles Heart Rate Service messages (From GATT manager)
*/
static void handle_heart_rate_client_gatt_manager_msg(Task task, MessageId id, Message msg)
{
    GHRSC_T *const heart_rate_client = (GHRSC_T*)task;

    GATT_HEART_RATE_CLIENT_DEBUG_INFO(("handle_heart_rate_client_gatt_manager_msg(%d) \n",id));

    switch(id)
    {
        case GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM:
            /* Characteristic handle discovery */
            handle_heart_rate_discover_char_cfm(heart_rate_client,
                                        (const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM_T*)msg);
            break;

        case GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM:
            /* Characteristic Descriptor handle discovery */
            handle_heart_rate_discover_char_descriptors_rsp(heart_rate_client,
                                        (const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T*)msg);
            break;

        case GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM:
            /* Write/Notification Confirmation */
            handle_heart_rate_write_response_cfm(heart_rate_client,
                                        (const GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM_T*)msg);
            break;

        case GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM:
            /* Read Confirmation */
            handle_heart_rate_read_response_cfm(heart_rate_client,
                                        (const GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM_T*)msg);
            break;

        case GATT_MANAGER_REMOTE_SERVER_NOTIFICATION_IND:
            /* Heart rate meas notification from user */
            handle_heart_rate_client_notification_ind(heart_rate_client, 
                                        (const GATT_MANAGER_REMOTE_SERVER_NOTIFICATION_IND_T*)msg);
            break;
            
        default:
            break;
    }

}

/***************************************************************************
NAME
    handle_heart_rate_ext_message

DESCRIPTION
    Handles Heart Rate Service External messages (From GATT)
*/
static void handle_heart_rate_ext_message(MessageId id)
{
    UNUSED(id); /* debug only */

    /* GATT unrecognised messages */
    GATT_HEART_RATE_CLIENT_DEBUG_PANIC(("Unknown Message 0x%x received from GATT lib \n",id));
}

/***************************************************************************
NAME
    heart_rate_client_register_for_notification

DESCRIPTION
   Utility function to send registration request for notification 
*/
static void heart_rate_client_register_for_notification(GHRSC_T *const heart_rate_client, 
                                                        bool enable)               
{
    uint8 value[2];

    value[0] = enable ? HEART_RATE_NOTIFICATION_VALUE : 0;
    value[1] = 0;

    heart_rate_client->notification_enabled =  value[0];
    GattManagerWriteCharacteristicValue((Task)&heart_rate_client->lib_task,
                                        heart_rate_client->hr_meas_ccd_handle,
                                        sizeof(value),
                                        value);
}

/***************************************************************************
NAME
    heart_rate_client_reset_energy_expended

DESCRIPTION
   Utility function to reset energy expended
*/
static void heart_rate_client_reset_energy_expended(GHRSC_T *const heart_rate_client)            
{
    uint8 value[2];

    value[0] = HEART_RATE_NOTIFICATION_VALUE;/*use 0x01 to reset*/
    value[1] = 0;

    GattManagerWriteCharacteristicValue((Task)&heart_rate_client->lib_task,
                                            heart_rate_client->hr_control_point_handle,
                                            sizeof(value),
                                            value);
}

/***************************************************************************
NAME
    handle_heart_rate_internal_message

DESCRIPTION
    Handles Heart Rate Service internal messages 
*/
static void  handle_heart_rate_internal_message(Task task, MessageId id, Message msg)
{
    GHRSC_T *const heart_rate_client = (GHRSC_T*)task;
    
    GATT_HEART_RATE_CLIENT_DEBUG_INFO(("handle_heart_rate_internal_messge(%d) \n",id));

    if (heart_rate_client != NULL)
    {
        switch(id)
        {
            case HEART_RATE_CLIENT_INTERNAL_MSG_DISCOVER:
            {
                /* Start by discovering Characteristic handles */
                GattManagerDiscoverAllCharacteristics((Task)&heart_rate_client->lib_task);
                SET_PENDING_FLAG(heart_rate_client_init_pending, heart_rate_client->pending_cmd);
            }
            break;

            case HEART_RATE_CLIENT_INTERNAL_MSG_NOTIFICATION_REQ:
            {
                heart_rate_client_register_for_notification(heart_rate_client, 
                        ((const HEART_RATE_CLIENT_INTERNAL_MSG_NOTIFICATION_REQ_T*)msg)->enable);
                SET_PENDING_FLAG(heart_rate_client_write_pending_notification, heart_rate_client->pending_cmd);
            }
            break;
            
            case HEART_RATE_CLIENT_INTERNAL_MSG_RESET_EE_REQ:
            {
                heart_rate_client_reset_energy_expended(heart_rate_client);
                SET_PENDING_FLAG(heart_rate_client_write_pending_reset_ee, heart_rate_client->pending_cmd);
            }
            break;

            default:
            {
                /* Internal unrecognised messages */
                GATT_HEART_RATE_CLIENT_DEBUG_PANIC(("Unknown Message received from Internal To lib \n"));
            }
            break;
        }
    }
}

/****************************************************************************/
void heart_rate_client_msg_handler(Task task, MessageId id, Message msg)
{
     GATT_HEART_RATE_CLIENT_DEBUG_INFO(("heart_rate_client_msg_handler (ID= %d)\n",id));
     /* Check mesage is from GATT Manager */
    if((id >= GATT_MANAGER_MESSAGE_BASE) && (id < GATT_MANAGER_MESSAGE_TOP))
    {
        handle_heart_rate_client_gatt_manager_msg(task,id,msg);
    }
    else if ((id >= GATT_MESSAGE_BASE) && (id < GATT_MESSAGE_TOP))
    {
        handle_heart_rate_ext_message(id);
    }
    /* Check message is internal Message */
    else if((id > HEART_RATE_CLIENT_INTERNAL_MSG_BASE) && (id < HEART_RATE_CLIENT_INTERNAL_MSG_TOP))
    {
        handle_heart_rate_internal_message(task,id,msg);
    }
    else
    {
        /* Unknown message, cannot handle */
        GATT_HEART_RATE_CLIENT_DEBUG_PANIC(("Unknown Message received  \n"));
    }
}

