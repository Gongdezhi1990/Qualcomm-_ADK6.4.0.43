/* Copyright (c) 2015 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#include <string.h>
#include <stdio.h>

#include "gatt_hid_client_private.h"

#include "gatt_hid_client_notification.h"

#include "gatt_hid_client_utils.h"


/* HID notification value */
#define HID_LE_NOTIFICATION_VALUE   0x01

/***************************************************************************/
/*                                      Internal Helper Functions                                                            */
/***************************************************************************/

/***************************************************************************
NAME
    hid_client_register_for_notification

DESCRIPTION
   Utility function to send registration request for notification 
*/
static void hid_client_register_for_notification(GHIDC_T *const hid_client,
               uint16 handle,
               bool enable)
               
{
    uint8 value[2];

    value[0] = enable ? HID_LE_NOTIFICATION_VALUE : 0;
    value[1] = 0;
    /* Update notificationis enabled/disabled for this HID client*/
    hid_client->notifitcation_enabled =  value[0];
    GattManagerWriteCharacteristicValue((Task)&hid_client->lib_task,
                                        handle,
                                        sizeof(value),
                                        value);

}

/***************************************************************************
NAME
    hid_client_get_ccdhandle_for_reportId

DESCRIPTION
   Utility function to get the client characteristic configuration handle for an Input report from report_id
*/
static bool hid_client_get_ccdhandle_for_inputreport(GHIDC_T *const hid_client,
            uint16 report_id,
            uint16* ccd_handle,
            gatt_hid_client_mode mode)
{
   uint16 count;
   uint16 report_handle = 0;
   uint16 type =0;
   /* Get total number of characteristic descriptors are available */
   uint16 num_ccd = hid_client->num_boot_ccd+ hid_client->num_report_ccd;
   *ccd_handle = INVALID_HID_HANDLE;
   /* Get the report handle from report ID */
    if(hid_client_get_report_handle_and_type_from_id(hid_client,report_id,&report_handle,&type))
    {
        /* if the report is not an input report, return */
        if(type != gatt_hid_input_report)
            return FALSE;
        
        for(count = 0;count < num_ccd ;count++)
        {  
            /* Check any Input report handle is matching with the ccd report handle */
            if((report_handle == hid_client->ccd_handles[count].characterisitc_handle) &&
                (mode == hid_client->ccd_handles[count].ccd_type))
            {
                *ccd_handle = hid_client->ccd_handles[count].ccd;
                return TRUE;
            }
        }
    }
    
   return FALSE;
}
/***************************************************************************
NAME
    hid_client_notification_reg_cfm

DESCRIPTION
     Utility function to send set register for notification request for report id confirmation to 
     registered application task
*/
static void hid_client_notification_reg_cfm(GHIDC_T *const hid_client,
            const GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM_T *cfm)
{
    gatt_hid_client_status hid_client_status = hid_client_convert_status(cfm->status);
    bool send_status;

    /* If we recognise the request, then we want to report errors */
    send_status = (hid_client_status != gatt_hid_client_status_success);

    if(hid_client->pending_request == hid_client_write_pending_boot_notification)
    {
        hid_client->num_ccd_cfm++;

        /* Are all notification registrations completed ? */
        if (hid_client->num_boot_ccd == hid_client->num_ccd_cfm)
            send_status = TRUE;
    }
    else if(hid_client->pending_request == hid_client_write_pending_report_notification)
    {
         hid_client->num_ccd_cfm++;

         /* Are all notification registrations completed ? */
        if(hid_client->num_report_ccd == hid_client->num_ccd_cfm)
            send_status = TRUE;
    }
    else
    {
        /* We don't recognise the request */
        send_status = FALSE;
    }

    /* All notification registartions are received, then inform to registered task */
    if(send_status)
    {
        MAKE_HID_CLIENT_MESSAGE(GATT_HID_CLIENT_NOTIFICATION_REG_CFM);

        message->hid_client = hid_client;
        message->status = hid_client_status;
        message->cid = cfm->cid;
        MessageSend(hid_client->app_task, GATT_HID_CLIENT_NOTIFICATION_REG_CFM, message);
        CLEAR_PENDING_FLAG(hid_client->pending_request);
        hid_client->num_ccd_cfm = 0;
    }
}

/***************************************************************************
NAME
    hid_client_notification_reg_ccdhandle_cfm

DESCRIPTION
     Utility function to send set register for notification request for report id confirmation 
     to registered application task
*/
static void hid_client_notification_reg_ccdhandle_cfm(GHIDC_T *const hid_client,
            const GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM_T *cfm)
{
    gatt_hid_client_status hid_client_status = hid_client_convert_status(cfm->status);
    
    MAKE_HID_CLIENT_MESSAGE(GATT_HID_CLIENT_NOTIFICATION_REPROTID_REG_CFM);

    /* Fill up the information and send to registered task */
    message->hid_client = hid_client;
    message->status = hid_client_status;
    message->cid = cfm->cid;
    MessageSend(hid_client->app_task, GATT_HID_CLIENT_NOTIFICATION_REPROTID_REG_CFM, message);
    
    CLEAR_PENDING_FLAG(hid_client->pending_request);
}

/****************************************************************************/
/*     Interface to Hid client message handler for internal  Notifitcaion process requests                    */

/****************************************************************************/
/****************************************************************************/
void hid_client_notification_reg_request(GHIDC_T *const hid_client,
                        const HID_CLIENT_INTERNAL_HANDLE_NOTIFICATION_REQ_T*msg)
{
    uint16 count=0;
    uint16 handle;
    uint16 num_ccd = 0;
    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_notification_reg_request() Entry\n"));
    num_ccd = hid_client->num_boot_ccd+ hid_client->num_report_ccd;

    /*Check notification registration is for boot mode */
    if(msg->mode == hid_client_boot_mode)
    {     
        for(count = 0;count < num_ccd ;count++)
        {
            /* Find out the boot mode ccd */
            if(hid_client->ccd_handles[count].ccd_type == hid_client_boot_mode)
            {
                handle = hid_client->ccd_handles[count].ccd;
                hid_client_register_for_notification(hid_client,handle,msg->enable);
                SET_PENDING_FLAG(hid_client_write_pending_boot_notification,hid_client->pending_request);
            }
        }
    }
    /*Check notification registration is for report mode */
    else if(msg->mode == hid_client_report_mode)
    {
        for(count = 0;count < num_ccd ;count++)
        {
            /* Find out the report  mode ccd */
            if(hid_client->ccd_handles[count].ccd_type == hid_client_report_mode)
            {
                handle = hid_client->ccd_handles[count].ccd;
                GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_notification_reg_request() report_count= %d,handle = %x\n",
                                                                  count,handle));
                hid_client_register_for_notification(hid_client,handle,msg->enable);
                SET_PENDING_FLAG(hid_client_write_pending_report_notification,hid_client->pending_request);
            }
        }
    }
    /*Reset the CCD confirmaton flag */
    hid_client->num_ccd_cfm = 0;
}

/****************************************************************************/
void hid_client_notification_reg_request_ccdhandle(GHIDC_T *const hid_client,
                    const HID_CLIENT_INTERNAL_SET_NOTIFICATION_CCDHANDLE_REQ_T*msg)
{
    /* Register for notification */
    hid_client_register_for_notification(hid_client,msg->handle,msg->enable);
    SET_PENDING_FLAG(hid_client_write_pending_ccdhandle_notification,hid_client->pending_request);
}

/****************************************************************************/
/*     External Functions within HID client lib, Interface to Hid client message handler for notifitcaion   */
/****************************************************************************/

/****************************************************************************/
void handle_hid_client_notification_ind(GHIDC_T *const hid_client,
                const GATT_MANAGER_REMOTE_SERVER_NOTIFICATION_IND_T* ind)
{
    uint16 report_id;
    /* Check notifications has been enabled from application, else ignore the notifications */
    if(hid_client->notifitcation_enabled) 
    {
        /* Validate the notification handle as this is what we have registered for */
        if(hid_client_validate_notification_handle(hid_client,ind->handle,&report_id))
        {
            /* Ensure the application has enabled notifications for this report. If not,
             * ignore the notification because it shouldn't have been sent.
             */
            if (hid_client_get_notifications_for_report_id(hid_client, report_id))
            {
                MAKE_HID_CLIENT_MESSAGE_WITH_LEN( GATT_HID_CLIENT_NOTIFICATION_IND,ind->size_value);
                message->hid_client = hid_client;
                message->cid = ind->cid;
                message->report_id = report_id;
                message->size_value = ind->size_value;
                memmove(message->value, ind->value, ind->size_value);
                GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_notification_reg_request() report_id = %x\n",
                                                                message->report_id));

                MessageSend(hid_client->app_task, GATT_HID_CLIENT_NOTIFICATION_IND, message);
            }
        }       
    }
}

/****************************************************************************/

void handle_hid_client_notification_reg_cfm(GHIDC_T *const hid_client,
                    const GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM_T* cfm)
{
    switch(hid_client->pending_request)
    {
        case hid_client_write_pending_report_notification:
        case hid_client_write_pending_boot_notification:
        {      
            hid_client_notification_reg_cfm(hid_client,cfm);
        }
        break;
        
        case hid_client_write_pending_ccdhandle_notification:
        {
            hid_client_notification_reg_ccdhandle_cfm(hid_client,cfm);
        }
        break;
        
        default:
            /* Noting pending, Ignore */
        break;           
    }
}

/****************************************************************************/
/*                              External Interface function                                                                   */
/****************************************************************************/
/****************************************************************************/
bool GattHidRegisterForNotification(GHIDC_T *const hid_client,
                                    bool enable,
                                    gatt_hid_client_mode hid_mode)
{
    /* Validate the Input Parameters */
    PanicNull(hid_client);
    
    /*Check Discovery in progress Setting or boot mode CCD is supported only if boot mode is configured  */
    if (    hid_client->discovery_in_progress
        || (    hid_mode == hid_client_boot_mode
            && !hid_client->boot_mode_supported))
    {
        return FALSE;
    }

    /* Make internal message and Process the request */ 
    {
        MAKE_HID_CLIENT_MESSAGE(HID_CLIENT_INTERNAL_HANDLE_NOTIFICATION_REQ);
        message->mode = hid_mode;
        message->enable = enable;
        GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:GattHidRegisterForNotification(),mode = %d, enable = %x\n",
                                                         message->mode,message->enable));
        MessageSendConditionally((Task)&hid_client->lib_task, HID_CLIENT_INTERNAL_HANDLE_NOTIFICATION_REQ,
                                            message,&hid_client->pending_request);
    }
    return TRUE;
}

/****************************************************************************/
bool GattHidRegisterNotificationForReportID(GHIDC_T *const hid_client,
                                    bool enable,
                                    uint16 report_id,
                                    gatt_hid_client_mode hid_mode)
{
    uint16 ccd_handle = 0;
    
    /* Validate the Input Parameters */
    PanicNull(hid_client);
    
    /*Check Discovery in progress or Boot mode request is received when boot mode is not supported */
    if (    hid_client->discovery_in_progress
        || (     hid_mode == hid_client_boot_mode
             && !hid_client->boot_mode_supported))
    {
        return FALSE;
    }

    /* Validate the report id and verify the report id is for an input report */
    if(!hid_client_get_ccdhandle_for_inputreport(hid_client,report_id,&ccd_handle,hid_mode)) 
        return FALSE;

    /* Make internal message and Process the request */ 
    {
        MAKE_HID_CLIENT_MESSAGE(HID_CLIENT_INTERNAL_SET_NOTIFICATION_CCDHANDLE_REQ);
        message->handle = ccd_handle;
        message->enable = enable;
        GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:GattHidRegisterNotificationForReportID(),handle = %d, enable = %x\n",
                                                        message->handle,message->enable));
        MessageSendConditionally((Task)&hid_client->lib_task, HID_CLIENT_INTERNAL_SET_NOTIFICATION_CCDHANDLE_REQ,
                                            message,&hid_client->pending_request);
    }
    hid_client_set_notifications_for_report_id(hid_client, report_id, enable);

    return TRUE;
}


