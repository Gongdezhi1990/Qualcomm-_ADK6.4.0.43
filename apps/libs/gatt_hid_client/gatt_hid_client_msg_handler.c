/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */


#include "gatt_hid_client_private.h"

#include "gatt_hid_client_msg_handler.h"
#include "gatt_hid_client_read.h"
#include "gatt_hid_client_write.h"
#include "gatt_hid_client_discovery.h"
#include "gatt_hid_client_notification.h"

/* Internal static functions */
static void handle_hid_client_gatt_msg(void);
static void handle_hid_client_internal_msg(Task task, MessageId id, Message msg);
static void handle_hid_client_gatt_manager_msg(Task task, MessageId id, Message msg);


/***************************************************************************
NAME
    handle_hid_client_read_characteristic_value_cfm

DESCRIPTION
    Handler for all read characteristic value confirm messages. 
    This function will decide which sub module need to receive the confirmation message
*/
static void handle_hid_client_read_characteristic_value_cfm(GHIDC_T *const hid_client,
                    const GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM_T* read_cfm)
{
    if(hid_client->discovery_in_progress)
    {
        /* This read is a part of discovery, handle in discovery module */
        handle_hid_client_discover_read_char_value_cfm(hid_client,read_cfm);
    }
    else
    {
        /* Handle read Response */
        handle_hid_client_read_char_value_cfm(hid_client,read_cfm);
    }
}

/***************************************************************************
NAME
    handle_hid_client_write_characteristic_value_cfm

DESCRIPTION
    Handler for all write characteristic value confirms messages. 
    This function will decide which sub module need to receive the confirmation message
*/
static void handle_hid_client_write_characteristic_value_cfm(GHIDC_T *const hid_client,
      const GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM_T* write_cfm)
{
    /* Check this reponse is for notification registration */
    if((hid_client->pending_request == hid_client_write_pending_boot_notification) ||
        (hid_client->pending_request == hid_client_write_pending_report_notification) ||
        (hid_client->pending_request == hid_client_write_pending_ccdhandle_notification))
    {
        handle_hid_client_notification_reg_cfm(hid_client,write_cfm);
    }
    else
    {
         handle_hid_client_write_char_value_cfm(hid_client,write_cfm);
    }
}

/***************************************************************************
NAME
    handle_hid_client_gatt_msg

DESCRIPTION
    Handler for all messages received from gatt lib  .
*/
static void handle_hid_client_gatt_msg(void)
{
    /* Unknown message , can not handle */
    GATT_HID_CLIENT_DEBUG_PANIC(("Func:handle_hid_client_gatt_msg():Unknown Message received  \n"));
}
/***************************************************************************
NAME
    handle_hid_client_gatt_msg

DESCRIPTION
    Handler for all messages received from gatt manager lib  .
*/
static void handle_hid_client_gatt_manager_msg(Task task, MessageId id, Message msg)
{

    GHIDC_T *const hid_client = (GHIDC_T*)task;
    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func: handle_hid_client_gatt_manager_msg(): Message ID = %x \n",id));
    
    if(hid_client != NULL)
    {
        switch(id)
        {
            /* Handle HID notifications */
            case GATT_MANAGER_REMOTE_SERVER_NOTIFICATION_IND:
            {
                handle_hid_client_notification_ind(hid_client,(const GATT_MANAGER_REMOTE_SERVER_NOTIFICATION_IND_T*)msg);
            }
            break;

            /* Discover All Characterisitcs Confirm */
            case GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM:
            {
                handle_hid_client_discover_all_char_cfm(hid_client,(const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM_T*)msg);
            }
            break;
            /* Characteristic Descriptor Confirme */
            case GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM:
            {
                handle_hid_client_discover_all_char_descriptors_cfm(hid_client,(const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T*)msg);
            }
            break;
            /* Handle Characteristic value read */
            case GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM:
            {
                handle_hid_client_read_characteristic_value_cfm(hid_client,(const GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM_T*)msg);
            }
            break;
            /* Handle Read Long Characteristic Values */
            case GATT_MANAGER_READ_LONG_CHARACTERISTIC_VALUE_CFM:
            {
                handle_hid_client_read_long_char_value_cfm(hid_client,(const GATT_MANAGER_READ_LONG_CHARACTERISTIC_VALUE_CFM_T*)msg);
            }
            break;
            /*  Handle read using UUID */
            case GATT_MANAGER_READ_USING_CHARACTERISTIC_UUID_CFM:
            {
               handle_hid_client_read_using_uuid_cfm(hid_client,(const GATT_MANAGER_READ_USING_CHARACTERISTIC_UUID_CFM_T*)msg);
            }
            break;
            /* Handle write without response */
            case GATT_MANAGER_WRITE_WITHOUT_RESPONSE_CFM:
            {
                handle_hid_client_write_without_response_cfm(hid_client,(const GATT_MANAGER_WRITE_WITHOUT_RESPONSE_CFM_T*)msg);
            }
            break;
            /* Handle write characteristic value reponse  */
            case GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM:
            {
                handle_hid_client_write_characteristic_value_cfm(hid_client,(const GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM_T*)msg);
            }
            break;

            default:
            {
                /* Unknown Message , Ignore */
            }
            break;
        }
    }
}

/***************************************************************************
NAME
    handle_hid_client_internal_msg

DESCRIPTION
    Handler for All messaegs posted internally within client .
*/
static void handle_hid_client_internal_msg(Task task, MessageId id, Message msg)
{
    GHIDC_T *const hid_client = (GHIDC_T*)task;
    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func: handle_hid_client_internal_msg(): Message ID = %x \n",id));

    if(hid_client != NULL)
    {
        switch(id)
        {
            /* Internal Discovery requests  */
            case HID_CLIENT_INTERNAL_MSG_DISCOVER:
            {
                hid_client_start_discovery(hid_client);
            }
            break;

            case HID_CLIENT_INTERNAL_MSG_DISCOVER_COMPLETE:
            {
                hid_client_complete_discovery(hid_client, ((const HID_CLIENT_INTERNAL_MSG_DISCOVER_T*)msg)->cid);
            }
            break;
            /* Internal Read requests */
            case HID_CLIENT_INTERNAL_MSG_GET_PROTOCOL:
            {
                hid_client_read_get_protocol(hid_client);
            }
            break;

            case HID_CLIENT_INTERNAL_MSG_READ_EXT_REFERENCE:
            {
                hid_client_read_external_reference(hid_client);
            }
            break;                

            case HID_CLIENT_INTERNAL_MSG_READ_CCD:
            {
                hid_client_read_ccd_handle(hid_client, (const HID_CLIENT_INTERNAL_MSG_READ_CCD_T*)msg);
            }
            break;        

            case HID_CLIENT_INTERNAL_MSG_READ_INFO:
            {
                hid_client_read_hidinfo(hid_client);
            }
            break;

            case HID_CLIENT_INTERNAL_MSG_GET_REPORT_ID_MAP:
            {
                hid_client_read_report_id_map(hid_client);
            }
            break;

            case HID_CLIENT_INTERNAL_MSG_GET_REPORT:
            {
                hid_client_read_get_report(hid_client,(const HID_CLIENT_INTERNAL_MSG_GET_REPORT_T*)msg);
            }
            break;

            case HID_CLIENT_INTERNAL_MSG_READ_REPORT_MAP:
            {
                hid_client_read_report_map(hid_client);
            }
            break;

            case HID_CLIENT_INTERNAL_MSG_READ_BOOT_REPORT:
            {
                hid_client_read_boot_report(hid_client,(const HID_CLIENT_INTERNAL_MSG_READ_BOOT_REPORT_T*)msg);
            }
            break;
            /* Internal Write Requests */
            case HID_CLIENT_INTERNAL_SET_PROTOCOL:
            {
                hid_client_write_set_protocol(hid_client,(const HID_CLIENT_INTERNAL_SET_PROTOCOL_T*)msg);
            }
            break;
            
            case HID_CLIENT_INTERNAL_SET_CTRL_POINT:
            {
                hid_client_write_set_control_point(hid_client,(const HID_CLIENT_INTERNAL_SET_CTRL_POINT_T*)msg);
            }
            break;

            case HID_CLIENT_INTERNAL_MSG_SET_REPORT:
            {
                hid_client_write_set_report_request(hid_client,(const HID_CLIENT_INTERNAL_MSG_SET_REPORT_T*)msg);
            }
            break;

            case HID_CLIENT_INTERNAL_MSG_WRITE_BOOT_REPORT:
            {
                hid_client_write_boot_report_request(hid_client,(const HID_CLIENT_INTERNAL_MSG_WRITE_BOOT_REPORT_T*)msg);
            }
            break;

            /* Internal Notification requests */
            case HID_CLIENT_INTERNAL_HANDLE_NOTIFICATION_REQ:
            {
                hid_client_notification_reg_request(hid_client,(const HID_CLIENT_INTERNAL_HANDLE_NOTIFICATION_REQ_T*)msg);
            }
            break;

            case HID_CLIENT_INTERNAL_SET_NOTIFICATION_CCDHANDLE_REQ:
            {
                hid_client_notification_reg_request_ccdhandle(hid_client,(const HID_CLIENT_INTERNAL_SET_NOTIFICATION_CCDHANDLE_REQ_T*)msg);
            }
            break;
            
            default:
            {
                /* Unknown Internal message ,Should not reach here */
                GATT_HID_CLIENT_DEBUG_PANIC(("Func:handle_hid_client_internal_msg():Unknown Message received  \n"));
            }
            break;
        }
    }
}

/****************************************************************************/
void hid_client_msg_handler(Task task, MessageId id, Message msg)
{
    /* Check mesage is from GATT lib to HID Client LIB */
    if((id >=GATT_MESSAGE_BASE )&& (id < GATT_MESSAGE_TOP))
    {
         handle_hid_client_gatt_msg();
    }
    /* Check mesage is from GATT Manager lib to HID Client LIB */
    else if((id >= GATT_MANAGER_MESSAGE_BASE) && (id < GATT_MANAGER_MESSAGE_TOP))
    {
        handle_hid_client_gatt_manager_msg(task,id,msg);
    }
    /* Check mesage is from Internal to HID Client LIB */
    else if((id >= HID_CLIENT_INTERNAL_MSG_BASE) && (id < HID_CLIENT_INTERNAL_MSG_TOP))
    {
        handle_hid_client_internal_msg(task,id,msg);
    }
    else
    {
        /* Unknown message , can not handle */
        GATT_HID_CLIENT_DEBUG_PANIC(("Func:hid_client_msg_handler():Unknown Message received  \n"));
    }

}

