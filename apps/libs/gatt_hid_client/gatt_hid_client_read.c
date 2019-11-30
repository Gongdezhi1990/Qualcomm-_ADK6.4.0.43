/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#include <string.h>
#include <stdio.h>

#include "gatt_hid_client_private.h"

#include "gatt_hid_client_read.h"

#include "gatt_hid_client_utils.h"


/* Reoprt ID and report type byte size */
#define BYTE_SIZE_REPORT_ID 2
#define BYTE_SIZE_REPORT_TYPE 2

/***************************************************************************/
/*                                      Internal Helper Functions                                                            */
/***************************************************************************/

/***************************************************************************
NAME
    hid_client_get_protocol_cfm

DESCRIPTION
   Utility function to send get protocol confirmation to registered application task  
*/
static void hid_client_get_protocol_cfm(GHIDC_T *const hid_client, 
                    const GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM_T* cfm)
{
    /* Convert GATT lib status to HID lib status */
    gatt_hid_client_status hid_client_status = hid_client_convert_status(cfm->status);

    /* Make the protocol confirmation message */
    MAKE_HID_CLIENT_MESSAGE(GATT_HID_CLIENT_GET_PROTOCOL_CFM);

    /* Fill in confirmation message parameters */
    message->hid_client = hid_client;
    message->status = hid_client_status;
    message->cid = cfm->cid;
    /* If status is success then fill in the protocol */
    if((hid_client_status == gatt_hid_client_status_success)&&(cfm->size_value))
    {
          message->hid_mode = cfm->value[0] ;   
    }
    MessageSend(hid_client->app_task, GATT_HID_CLIENT_GET_PROTOCOL_CFM, message);
    
    CLEAR_PENDING_FLAG(hid_client->pending_request);
 }

/***************************************************************************
NAME
    hid_client_read_hidinfo_cfm

DESCRIPTION
   Utility function to send get read information confirmtion to registered application task  
*/
static void  hid_client_read_hidinfo_cfm(GHIDC_T *const hid_client, 
                const GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM_T* cfm)
{
    /* Convert GATT lib status to HID lib status */
    gatt_hid_client_status hid_client_status = hid_client_convert_status(cfm->status);

    /* Make the protocol read infor message */
    MAKE_HID_CLIENT_MESSAGE_WITH_LEN( GATT_HID_CLIENT_READ_INFO_CFM,cfm->size_value);

    /* Fill in confirmation message parameters */
    message->hid_client = hid_client;
    message->status = hid_client_status;
    message->cid = cfm->cid;
    /* If status is success, then fill in the data */
    if ((hid_client_status == gatt_hid_client_status_success)&&(cfm->size_value))
    {
        message->size_value = cfm->size_value;
        /* Copy the vendor-specific data */
        memmove(message->value, cfm->value, cfm->size_value);
    }

    MessageSend(hid_client->app_task, GATT_HID_CLIENT_READ_INFO_CFM, message);

    CLEAR_PENDING_FLAG(hid_client->pending_request);
}

/***************************************************************************
NAME
    hid_client_read_ext_reference_cfm

DESCRIPTION
   Utility function to send get read external reference confirmation to registered application task  
*/
static void hid_client_read_ext_reference_cfm(GHIDC_T *const hid_client, 
                    const GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM_T* cfm)
{
    /* Convert GATT lib status to HID lib status */
    gatt_hid_client_status hid_client_status = hid_client_convert_status(cfm->status);

    MAKE_HID_CLIENT_MESSAGE_WITH_LEN( GATT_HID_CLIENT_READ_EXT_REF_CFM,cfm->size_value);

    /* Fill in confirmation message parameters */
    message->hid_client = hid_client;
    message->status = hid_client_status;
    message->cid = cfm->cid;
    /* If status is success, then fill in the data */
    if ((hid_client_status == gatt_hid_client_status_success)&&(cfm->size_value))
    {
        message->size_value = cfm->size_value;
        /* Copy the vendor-specific data */
        memmove(message->value, cfm->value, cfm->size_value);
    }
    MessageSend(hid_client->app_task, GATT_HID_CLIENT_READ_EXT_REF_CFM, message);
    
    CLEAR_PENDING_FLAG(hid_client->pending_request);
}

/***************************************************************************
NAME
    hid_client_read_ccd_cfm

DESCRIPTION
   Utility function to send get read external reference confirmation to registered application task  
*/
static void hid_client_read_ccd_cfm(GHIDC_T *const hid_client, 
                        const GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM_T* cfm)
{
    /* Convert GATT lib status to HID lib status */
    gatt_hid_client_status hid_client_status = hid_client_convert_status(cfm->status);

    MAKE_HID_CLIENT_MESSAGE_WITH_LEN( GATT_HID_CLIENT_READ_CCD_CFM,cfm->size_value);

    /* Fill in confirmation message parameters */
    message->hid_client = hid_client;
    message->status = hid_client_status;
    message->cid = cfm->cid;
    
    /* If status is success, then fill in the data */
    if ((hid_client_status == gatt_hid_client_status_success)&&(cfm->size_value))
    {    
        uint16 report_id;
        uint16 count;
        uint16 num_ccd = hid_client->num_boot_ccd+ hid_client->num_report_ccd;

        /*Get report handle from ccd handle*/
        for(count = 0;count < num_ccd ;count++)
        {  
            if(cfm->handle == hid_client->ccd_handles[count].ccd)
            {
                /* Get Report Id from report handle */
                if(hid_client_get_report_id_from_handle(hid_client, hid_client->ccd_handles[count].characterisitc_handle, &report_id))
                {
                    message->report_id  = report_id;
                }
                break;
            }
        }        

        message->size_value = cfm->size_value;
        memmove(message->value, cfm->value, cfm->size_value);
    }
    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_read_ccd_cfm() status [%x] report id [%x] \n", 
                                                    hid_client_status, message->report_id));

    MessageSend(hid_client->app_task, GATT_HID_CLIENT_READ_CCD_CFM, message);
    
    CLEAR_PENDING_FLAG(hid_client->pending_request);
}


/***************************************************************************
NAME
    hid_client_read_report_id_map

DESCRIPTION
   Utility function to send get read external reference confirmation to registered application task  
*/
static void hid_client_read_report_id_map_cfm(GHIDC_T *const hid_client)
{
    uint16 count = 0;
    uint16 index=0;
    /* Temporary instance of report map ID used to go through available reports */
    gatt_hid_client_report_id_map_t *report_id_map = NULL;

    MAKE_HID_CLIENT_MESSAGE_WITH_LEN( GATT_HID_CLIENT_REPORT_ID_MAP_CFM,
                                                                (hid_client->num_report_id*(BYTE_SIZE_REPORT_ID+BYTE_SIZE_REPORT_TYPE)));

    /* Fill in confirmation message parameters */
    message->hid_client = hid_client;
    message->num_reports = hid_client->num_report_id;
    message->status = gatt_hid_client_status_success;
    /* Find out each report ID mapping instance and fill in data to confirmation message */
    for(count=0;count< hid_client->num_report_id;count++)
    {
        report_id_map = &hid_client->report_id_map[count];

        GATT_HID_CLIENT_CONVERT_UINT16_TO_UINT8(index, report_id_map->report_id, message->value);
        GATT_HID_CLIENT_DEBUG_INFO(("Func:hid_client_read_report_id_map_cfm(),id [%x]", (message->value[index-2]<<8|message->value[index-1])));
        
        GATT_HID_CLIENT_CONVERT_UINT16_TO_UINT8(index, report_id_map->type, message->value);
        GATT_HID_CLIENT_DEBUG_INFO((" type [%x]\n",(message->value[index-2]<<8|message->value[index-1])));

    }
    MessageSend(hid_client->app_task, GATT_HID_CLIENT_REPORT_ID_MAP_CFM, message);
}

/***************************************************************************
NAME
    hid_client_read_get_report_cfm

DESCRIPTION
   Utility function to send get report confirmation to registered application task  
*/
static void hid_client_read_get_report_cfm(GHIDC_T *const hid_client,
                 const GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM_T* cfm)
{
    uint16 report_id;
   /* Convert GATT lib status to HID lib status */
    gatt_hid_client_status hid_client_status = hid_client_convert_status(cfm->status);

    MAKE_HID_CLIENT_MESSAGE_WITH_LEN( GATT_HID_CLIENT_GET_REPORT_CFM,cfm->size_value);

    /* Fill in confirmation message parameters */
    message->hid_client = hid_client;
    message->status = hid_client_status;
    message->cid = cfm->cid;

    /* If status is success, then fill in the data */
    if ((hid_client_status == gatt_hid_client_status_success)&&(cfm->size_value))
    {
        /* Get Report handle form Report ID */
        if(hid_client_get_report_id_from_handle(hid_client,cfm->handle,&report_id))
        {
            message->report_id  = report_id;
        }
        message->size_value = cfm->size_value;
        /* Copy the vendor-specific data */
        memmove(message->value, cfm->value, cfm->size_value);
    }
    
    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_read_report_id_map_cfm(),size [%d] value [%x]\n",
                                                    cfm->size_value,message->value[0]));
    MessageSend(hid_client->app_task, GATT_HID_CLIENT_GET_REPORT_CFM, message);

    CLEAR_PENDING_FLAG(hid_client->pending_request);
}

/***************************************************************************
NAME
    hid_client_read_report_map_cfm

DESCRIPTION
   Utility function to send read report map confirmation to registered application task  
*/
static void   hid_client_read_report_map_cfm(GHIDC_T *const hid_client,
                 const GATT_MANAGER_READ_LONG_CHARACTERISTIC_VALUE_CFM_T* cfm)
{
   /* Convert GATT lib status to HID lib status */
    gatt_hid_client_status hid_client_status = hid_client_convert_status(cfm->status);

    MAKE_HID_CLIENT_MESSAGE_WITH_LEN( GATT_HID_CLIENT_READ_REPORT_MAP_CFM,cfm->size_value);

   /* Fill in confirmation message parameters */
    message->hid_client = hid_client;
    message->status = hid_client_status;
    message->cid = cfm->cid;
    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_read_report_map_cfm() cfm->size_value = %x\n",
                                                     cfm->size_value));

    /* If status is success, then fill in the data */
    if ((hid_client_status == gatt_hid_client_status_success)&&(cfm->size_value))
    {
        message->more_to_come = cfm->more_to_come;
        message->offset = cfm->offset;
        message->size_value = cfm->size_value;
        memmove(message->value, cfm->value, cfm->size_value);
    }

    MessageSend(hid_client->app_task, GATT_HID_CLIENT_READ_REPORT_MAP_CFM, message);
    
    /* Clear pending flag only if status is failed or no more data has to come */
    if((!cfm->more_to_come) || (cfm->status != gatt_status_success))
    {
        CLEAR_PENDING_FLAG(hid_client->pending_request);
    }
}

/***************************************************************************
NAME
    hid_client_read_boot_report_value_cfm

DESCRIPTION
   Utility function to send read boot report confirmation to registered application task  
*/
static void   hid_client_read_boot_report_value_cfm(GHIDC_T *const hid_client,
                                const GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM_T* cfm)
{
   /* Convert GATT lib status to HID lib status */
    gatt_hid_client_status hid_client_status = hid_client_convert_status(cfm->status);

    MAKE_HID_CLIENT_MESSAGE_WITH_LEN( GATT_HID_CLIENT_READ_BOOT_REPORT_CFM,cfm->size_value);

   /* Fill in confirmation message parameters */
    message->hid_client = hid_client;
    message->status = hid_client_status;
    message->cid = cfm->cid;

    /* If status is success, then fill in the data */
    if ((hid_client_status == gatt_hid_client_status_success)&&(cfm->size_value))
    {
        message->more_to_come = FALSE;
        message->size_value = cfm->size_value;
        memmove(message->value, cfm->value, cfm->size_value);
    }
    
    MessageSend(hid_client->app_task, GATT_HID_CLIENT_READ_BOOT_REPORT_CFM, message);
    CLEAR_PENDING_FLAG(hid_client->pending_request);
}

/***************************************************************************
NAME
    hid_client_read_boot_report_value_error

DESCRIPTION
   Utility function to send read boot report confirmation to registered application task when an internal error has occurred
*/
static void   hid_client_read_boot_report_value_error(GHIDC_T *const hid_client)
{
    MAKE_HID_CLIENT_MESSAGE( GATT_HID_CLIENT_READ_BOOT_REPORT_CFM);

   /* Fill in confirmation message parameters */
    message->hid_client = hid_client;
    message->status = gatt_hid_client_status_failed;
    message->cid = 0; /* CID is unknown */
    
    MessageSend(hid_client->app_task, GATT_HID_CLIENT_READ_BOOT_REPORT_CFM, message);
    CLEAR_PENDING_FLAG(hid_client->pending_request);
    
    GATT_HID_CLIENT_DEBUG_PANIC(("HID Client read boot report internal error!\n"));
}

/***************************************************************************
NAME
    hid_client_read_boot_report_uuid_cfm

DESCRIPTION
   Utility function to send read boot report confirmation to registered application task  
*/
static void hid_client_read_boot_report_uuid_cfm(GHIDC_T *const hid_client,
                                    const GATT_MANAGER_READ_USING_CHARACTERISTIC_UUID_CFM_T* cfm)
{
 /* Convert GATT lib status to HID lib status */
    gatt_hid_client_status hid_client_status = hid_client_convert_status(cfm->status);

    MAKE_HID_CLIENT_MESSAGE_WITH_LEN(GATT_HID_CLIENT_READ_BOOT_REPORT_CFM,cfm->size_value);

   /* Fill in confirmation message parameters */
    message->hid_client = hid_client;
    message->status = hid_client_status;
    message->cid = cfm->cid;

    /* If status is success, then fill in the data */
    if ((hid_client_status == gatt_hid_client_status_success)&&(cfm->size_value))
    {
        message->more_to_come = cfm->more_to_come;
        message->size_value = cfm->size_value;
        memmove(message->value, cfm->value, cfm->size_value);
    }
     MessageSend(hid_client->app_task, GATT_HID_CLIENT_READ_BOOT_REPORT_CFM, message);
     
    /* Clear pending flag only if status is failed or no more data has to come */
    if((!cfm->more_to_come) || (cfm->status != gatt_status_success))
    {
        CLEAR_PENDING_FLAG(hid_client->pending_request);
    }
}

/****************************************************************************
NAME
    hid_client_read_ccd

DESCRIPTION
   Utility function to process the read ccd request  
*/
static bool hid_client_read_ccd(GHIDC_T *const hid_client, gatt_hid_client_mode mode)
{
    bool ret_val = FALSE;
    uint16 count;
    uint16 num_ccd = hid_client->num_boot_ccd+ hid_client->num_report_ccd;
    uint16 ccd_handle = INVALID_HID_HANDLE;

    for(count = 0;count < num_ccd ;count++)
    {
        /* Match the requested mode */
        if(hid_client->ccd_handles[count].ccd_type == mode)
        {
            ccd_handle = hid_client->ccd_handles[count].ccd;
            GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_read_ccd() report_count= %d,handle = %x\n",
                                                            count, ccd_handle));
            {
                /* Make internal message */
                MAKE_HID_CLIENT_MESSAGE(HID_CLIENT_INTERNAL_MSG_READ_CCD);
                message->ccd_handle = ccd_handle;
                MessageSendConditionally((Task)&hid_client->lib_task, HID_CLIENT_INTERNAL_MSG_READ_CCD, 
                                                     message, &hid_client->pending_request);
            }
            ret_val = TRUE;
        }
    }        
    
    return ret_val;
}


/****************************************************************************/
/* Interface Functions within HID client, Interface to Hid client message handler for handling responses */
/****************************************************************************/

/****************************************************************************/
void handle_hid_client_read_char_value_cfm(GHIDC_T *const hid_client, 
                            const GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM_T* read_cfm)
{
    /*Check pending requests and invoke appropriate handler functions */
   switch(hid_client->pending_request)
   {
        case hid_client_read_pending_protocol:
        {
            hid_client_get_protocol_cfm(hid_client,read_cfm);
        }
        break;

        case hid_client_read_pending_info:
        {
            hid_client_read_hidinfo_cfm(hid_client,read_cfm);
        }
        break;

        case hid_client_read_pending_ext_reference:
        {
            hid_client_read_ext_reference_cfm(hid_client,read_cfm);
        }
        break;        

        case hid_client_read_pending_ccdhandle:
        {
            hid_client_read_ccd_cfm(hid_client,read_cfm);
        }
        break;

        case hid_client_read_pending_get_report:
        {
            hid_client_read_get_report_cfm(hid_client,read_cfm);
        }
        break;
        
        case hid_client_read_pending_boot_report:
        {
            hid_client_read_boot_report_value_cfm(hid_client,read_cfm);
        }
        break;
        
        default:
            /* Noting pending, Ignore */
        break;

   }  
}

/****************************************************************************/
void handle_hid_client_read_long_char_value_cfm(GHIDC_T *const hid_client,
            const GATT_MANAGER_READ_LONG_CHARACTERISTIC_VALUE_CFM_T *read_lng_cfm)
{  
    /*Check pending requests and invoke appropriate handler functions */
    switch(hid_client->pending_request)
    {
        case hid_client_read_pending_read_report_map:
        {
            hid_client_read_report_map_cfm(hid_client,read_lng_cfm);
        }
        break;

        default:
            /* Noting pending, Ignore */
        break;
     }
}

void handle_hid_client_read_using_uuid_cfm(GHIDC_T *const hid_client,
            const GATT_MANAGER_READ_USING_CHARACTERISTIC_UUID_CFM_T* read_uuid_cfm)
{
    /*Check pending requests and invoke appropriate handler functions */
    switch(hid_client->pending_request)
    {
        case hid_client_read_pending_boot_report:
        {
            hid_client_read_boot_report_uuid_cfm(hid_client,read_uuid_cfm);
        }
        break;

        default:
            /* Nothing Pending,Ignore */
        break;
    }
}

/****************************************************************************/
/*    Interface Functions within HID client lib, Interface to Hid client message Handler for read request */
/****************************************************************************/

/****************************************************************************/
void hid_client_read_get_protocol(GHIDC_T *const hid_client)
{
      GattManagerReadCharacteristicValue((Task)&hid_client->lib_task,hid_client->protocol_handle);
      /* Set Read is pending */
      SET_PENDING_FLAG(hid_client_read_pending_protocol,hid_client->pending_request);
}

/****************************************************************************/
void hid_client_read_hidinfo(GHIDC_T *const hid_client)
{
    GattManagerReadCharacteristicValue((Task)&hid_client->lib_task, hid_client->info_handle);
    /* Set Read is pending */
    SET_PENDING_FLAG(hid_client_read_pending_info,hid_client->pending_request);
}

/****************************************************************************/
void hid_client_read_external_reference(GHIDC_T *const hid_client)
{
    GattManagerReadCharacteristicValue((Task)&hid_client->lib_task,hid_client->external_report_reference_handle);
    /* Set Read is pending */
    SET_PENDING_FLAG(hid_client_read_pending_ext_reference,hid_client->pending_request);
}

/****************************************************************************/
void hid_client_read_ccd_handle(GHIDC_T *const hid_client, 
        const HID_CLIENT_INTERNAL_MSG_READ_CCD_T* msg)
{  
    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_read_ccd_handle() handle = %x\n",msg->ccd_handle));

    GattManagerReadCharacteristicValue((Task)&hid_client->lib_task,msg->ccd_handle);
    /* Set Read is pending */
    SET_PENDING_FLAG(hid_client_read_pending_ccdhandle,hid_client->pending_request);
}

/****************************************************************************/
void hid_client_read_report_id_map(GHIDC_T *const hid_client)
{
    hid_client_read_report_id_map_cfm(hid_client);
}

/****************************************************************************/
void  hid_client_read_get_report(GHIDC_T *const hid_client, 
        const HID_CLIENT_INTERNAL_MSG_GET_REPORT_T* msg)
{
    GattManagerReadCharacteristicValue((Task)&hid_client->lib_task,msg->report_handle);
    /* Set Read is pending */
    SET_PENDING_FLAG(hid_client_read_pending_get_report,hid_client->pending_request);
}

/****************************************************************************/
void hid_client_read_report_map(GHIDC_T *const hid_client)
{
    GattManagerReadLongCharacteristicValue((Task)&hid_client->lib_task, hid_client->report_map_handle);
    /* Set Read is pending */
    SET_PENDING_FLAG(hid_client_read_pending_read_report_map,hid_client->pending_request);
}

/****************************************************************************/
void hid_client_read_boot_report(GHIDC_T *const hid_client,
                                 const HID_CLIENT_INTERNAL_MSG_READ_BOOT_REPORT_T *msg)
{
    uint16 handle = INVALID_HID_HANDLE;
    gatt_uuid_t uuid[4];
    uuid[0] = GATT_HID_INVALID_UUID;

    switch(msg->report_type)
    {
        case hid_client_kb_ouput_report:
        {
            handle = hid_client->boot_handles.boot_output_report_handle;
            if(handle == INVALID_HID_HANDLE)
                uuid[0] = GATT_HID_BOOT_OUTPUT_UUID;
        }
        break;

        case hid_client_kb_input_report:
        {
           handle = hid_client->boot_handles.boot_kb_input_report_handle;
            if(handle == INVALID_HID_HANDLE)
                uuid[0] =  GATT_HID_BOOT_KB_INPUT_UUID;
        }
        break;

        case hid_client_mouse_input_report:
        {
            handle = hid_client->boot_handles.boot_mouse_input_report_handle;
            if(handle == INVALID_HID_HANDLE)
                uuid[0] = GATT_HID_BOOT_KB_INPUT_UUID;
        }
        break;

        default:
        {
           /* Nothing to Do */
        }
        break;
   }

    if (uuid[0] != GATT_HID_INVALID_UUID)
    {
        gatt_manager_client_service_data_t service_data;
        
        if (GattManagerGetClientData(&hid_client->lib_task, &service_data))
        {
            GattManagerReadUsingCharacteristicUuid((Task)&hid_client->lib_task,
                                                    service_data.start_handle,
                                                    service_data.end_handle,
                                                    gatt_uuid16,
                                                    &uuid[0]);
        }
        else
        {
            hid_client_read_boot_report_value_error(hid_client);
        }
    }
    else
    {
        GattManagerReadCharacteristicValue((Task)&hid_client->lib_task,handle);
    }

    /* Set Read is pending if there is a valid handle or UUID */
    SET_PENDING_FLAG(hid_client_read_pending_boot_report,hid_client->pending_request);
}

/****************************************************************************/
/*              External Interface Functions for the application for reading handles                              */
/****************************************************************************/


/****************************************************************************/
bool GattHidGetProtocol(GHIDC_T *const hid_client)
{
    PanicNull(hid_client);

    /* Can not proceed with this command as GATT discovery is in progress with current client */
    if(DISCOVERY_IN_PROGRESS(hid_client)) return FALSE;

    /* Try to process the request from application */
    MessageSendConditionally((Task)&hid_client->lib_task, HID_CLIENT_INTERNAL_MSG_GET_PROTOCOL, NULL,&hid_client->pending_request);   
    return TRUE;
}

/****************************************************************************/
bool GattHidReadInformation(GHIDC_T *const hid_client)
{
    PanicNull(hid_client);

    /* Can not proceed this request if discovery is in progress */
    if(DISCOVERY_IN_PROGRESS(hid_client)) return FALSE;

    /* Try to process the request from application */
    MessageSendConditionally((Task)&hid_client->lib_task, HID_CLIENT_INTERNAL_MSG_READ_INFO, NULL,&hid_client->pending_request);
    return TRUE;
}

/****************************************************************************/
bool GattHidReadExternalReportReference(GHIDC_T *const hid_client)
{
    PanicNull(hid_client);

    /* Can not proceed this request if discovery is in progress */
    if(DISCOVERY_IN_PROGRESS(hid_client)|| (!IS_HANDLE_AVAILABLE(hid_client->external_report_reference_handle))) 
        return FALSE;

    /* Try to process the request from application */
    MessageSendConditionally((Task)&hid_client->lib_task, HID_CLIENT_INTERNAL_MSG_READ_EXT_REFERENCE, NULL,&hid_client->pending_request);
    return TRUE; 
}

/****************************************************************************/
bool  GattHidReadCCD(GHIDC_T *const hid_client, 
                                    gatt_hid_client_mode hid_mode)
{
    PanicNull(hid_client);

    /* Can not proceed this request if discovery is in progress or boot mode requested while boot mode is not supported */
    if (    hid_client->discovery_in_progress
        || (   hid_mode == hid_client_boot_mode
            && !hid_client->boot_mode_supported))
    {
        return FALSE;
    }

    return hid_client_read_ccd(hid_client, hid_mode);
}
/****************************************************************************/
bool GattHidGetReportIDMap(GHIDC_T *const hid_client)
{
    PanicNull(hid_client);

    /* Can not proceed this request if discovery is in progress */
    if(DISCOVERY_IN_PROGRESS(hid_client)) return FALSE;

    /* Process the request from application */
    MessageSend((Task)&hid_client->lib_task, HID_CLIENT_INTERNAL_MSG_GET_REPORT_ID_MAP, NULL);
    return TRUE;
}

/****************************************************************************/
bool GattHidGetReport(GHIDC_T *const hid_client,
                                    uint16 report_id)
{
    uint16 type = 0;
    uint16 report_handle = 0;

    PanicNull(hid_client);

    /* Can not proceed this request if discovery is in progress */
    if(DISCOVERY_IN_PROGRESS(hid_client)) return FALSE; 
    /* Get the report handle for report_id provided */
    if(!hid_client_get_report_handle_and_type_from_id(hid_client,report_id,&report_handle,&type))
        return FALSE;

    /* Make internal message and Process the request */ 
    {
        MAKE_HID_CLIENT_MESSAGE(HID_CLIENT_INTERNAL_MSG_GET_REPORT);
        message->report_handle = report_handle;
        MessageSendConditionally((Task)&hid_client->lib_task, HID_CLIENT_INTERNAL_MSG_GET_REPORT,
                                            message,&hid_client->pending_request);
    }
    return TRUE;
    
}

/****************************************************************************/
bool GattHidReadReportMap(GHIDC_T *const hid_client)
{
    PanicNull(hid_client);

    /* Can not proceed this request if discovery is in progress */
    if(DISCOVERY_IN_PROGRESS(hid_client)) return FALSE;
    
    MessageSendConditionally((Task)&hid_client->lib_task, HID_CLIENT_INTERNAL_MSG_READ_REPORT_MAP,
                                        NULL,&hid_client->pending_request);
    return TRUE;
}

/****************************************************************************/
bool GattHidReadBootReport(GHIDC_T *const hid_client, 
                                    gatt_hid_client_boot_mode_report_type report_type)
{
    PanicNull(hid_client);

    /* Can not proceed this request if discovery is in progress or boot mode is not supported */
    if(DISCOVERY_IN_PROGRESS(hid_client) || (!hid_client->boot_mode_supported))
        return FALSE;

    /* Make internal message and Process the request */ 
    {
        MAKE_HID_CLIENT_MESSAGE(HID_CLIENT_INTERNAL_MSG_READ_BOOT_REPORT);
        message->report_type = report_type;
        MessageSendConditionally((Task)&hid_client->lib_task, HID_CLIENT_INTERNAL_MSG_READ_BOOT_REPORT,
                                            message,&hid_client->pending_request);
    }
    return TRUE;
}

