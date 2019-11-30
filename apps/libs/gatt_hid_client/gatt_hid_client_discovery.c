/* Copyright (c) 2015 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#include <stdio.h>
#include <string.h>

#include "gatt_client.h"
#include "gatt_hid_client_private.h"
#include "gatt_hid_client_discovery.h"
#include "gatt_hid_client_utils.h"


/* Features that needs to be read from HID service, this masks will be used to read the descriptors   */
#define GATT_HID_BOOT_KEYBOARD_INPUT_SUPPORT_MASK       (0x1)
#define GATT_HID_BOOT_MOUSE_INPUT_SUPPORT_MASK          (0x2) 
#define GATT_HID_REPORT_MAP_SUPPORT_MASK                (0x4)
#define GATT_HID_REPORT_REFERENCE_SUPPORT_MASK          (0x8)

/* Macro for getting size of hid_discovery_instance */
#define SIZE_CHAR_HANDLE(num_handles) (sizeof(uint16) * (num_handles) - 1)
#define SIZE_HID_DISCOVERY_INSTANCE(num_handles) (sizeof(gatt_hid_current_discovery_instance_t) + SIZE_CHAR_HANDLE(num_handles))

/* Temporary GATT HID instance used for discovery purpose */
static gatt_hid_current_discovery_instance_t *hid_discovery_instance;

/***************************************************************************/
/*                                      Internal Helper Functions                                                            */
/***************************************************************************/

/***************************************************************************
NAME
    hid_client_init_cfm

DESCRIPTION
   Frame and send Init Confirmation message to application  
*/
static void hid_client_send_discovery_cfm(GHIDC_T *const hid_client,
    gatt_hid_current_discovery_instance_t *hid_instance,
    uint16 cid,
    gatt_hid_client_status status)
{
    /* Make Init confirmation message */   
    MAKE_HID_CLIENT_MESSAGE(GATT_HID_CLIENT_INIT_CFM);

    if(hid_instance)
    {
        hid_client->num_report_id = hid_instance->num_report_ids;
        message->status = status;
    }
    else
    {
        message->status = gatt_hid_client_status_failed;
    }
    message->hid_client = hid_client;
    message->cid = cid;
    message->num_report_id = hid_client->num_report_id;
    /* Set Discovery completed for this HID client instance */
    hid_client->discovery_in_progress = FALSE; 

    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_send_discovery_cfm(),status = %x,cid =%x,rpid =%d  \n",status,cid,message->num_report_id));

    /* Clean Up HID Discovery Instance */
    hid_client_cleanup_discovery_instance();
    /* Send the message */
    MessageSend(hid_client->app_task, GATT_HID_CLIENT_INIT_CFM, message);
}


/***************************************************************************
NAME
    hid_client_get_char_handles

DESCRIPTION
   Find out the start handles for characteristic descriptors 
*/
static void hid_client_get_char_handles(GHIDC_T *const hid_client,
               gatt_hid_current_discovery_instance_t *hid_instance,
               uint16 *start_handle)
{
    /* Temporary instance of report map, used to navigate through available reports */
    gatt_hid_client_report_id_map_t *report_map = NULL;

    uint16 clear_bit_mask = 0;
    /* Set start handle to zero */
     *start_handle = 0;
    /* Set Boot Handle as FALSE */
    hid_instance->is_boot_handle = FALSE;
    hid_instance->characteristic_handle = INVALID_HID_HANDLE;
    /* Check any more descriptors are left to read */
    if(hid_instance->char_descriptor_mask>0)
    {
        if(hid_instance->char_descriptor_mask & GATT_HID_BOOT_KEYBOARD_INPUT_SUPPORT_MASK)
        {
            clear_bit_mask = GATT_HID_BOOT_KEYBOARD_INPUT_SUPPORT_MASK;
            *start_handle =  hid_client->boot_handles.boot_kb_input_report_handle;
            hid_instance->is_boot_handle =TRUE;
        }
        else if(hid_instance->char_descriptor_mask & GATT_HID_BOOT_MOUSE_INPUT_SUPPORT_MASK)
        {
            clear_bit_mask = GATT_HID_BOOT_MOUSE_INPUT_SUPPORT_MASK;
            *start_handle =  hid_client->boot_handles.boot_mouse_input_report_handle;
            hid_instance->is_boot_handle =TRUE;
        }
        else if(hid_instance->char_descriptor_mask & GATT_HID_REPORT_MAP_SUPPORT_MASK)
        {
            clear_bit_mask = GATT_HID_REPORT_MAP_SUPPORT_MASK;
            *start_handle = hid_client->report_map_handle;

        }
        else if(hid_instance->char_descriptor_mask & GATT_HID_REPORT_REFERENCE_SUPPORT_MASK)
        {
            GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_get_char_handles(), num Report ; %d\n",
                                                            hid_instance->num_reports_available));

            if(hid_instance->read_report_handle_count<= hid_instance->num_reports_available)
            {
                hid_instance->read_report_handle_count++;
                /* Get report map instance */
                report_map =  (hid_client->report_id_map) + (hid_instance->read_report_handle_count-1);
                *start_handle = report_map->report_handle;
                if(hid_instance->read_report_handle_count == hid_instance->num_reports_available)
                {
                    hid_instance->read_report_handle_count = 0;
                    clear_bit_mask = GATT_HID_REPORT_REFERENCE_SUPPORT_MASK;
                }
            }
           GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_get_char_handles(), read handle count ; %d\n",
                                                            hid_instance->read_report_handle_count));
        }
        else
        {
            /* Error, Wrong Bit Mask Value */
           GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_get_char_handles(),Wrong Bit Mask \n"));
        }
        /* Clear The bit mask */
        if(clear_bit_mask > 0)
        {
            hid_instance->char_descriptor_mask &= (~clear_bit_mask);
        }
        hid_instance->characteristic_handle = *start_handle;
    }
    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_get_char_handles(), Exit: start_handle = %x\n",*start_handle));
}

/***************************************************************************
NAME
    hid_client_get_char_end_handles

DESCRIPTION
   Find out the end handles for characteristic descriptors 
*/
static void hid_client_get_char_end_handles(gatt_hid_current_discovery_instance_t *hid_instance,
            uint16 start_handle,
            uint16* end_handle)
{
    uint16 count;
    bool status = FALSE;
    uint16 current = INVALID_GATT_END_HANDLE;
    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_get_char_end_handles(), Exit:CharCount = %d\n",
                                                    hid_instance->char_uuid_count));

    /* Charactristics are stored in the order we get from the remote peripheral.
     * Start  handle = handle of the characteristic
     * end handle = charateristic -1 of the first greater value than the handle.
     * (endHandle will be set to end handle of the service if endHandle is 0).
     * This should give the start and end values for charcteristic descriptor search 
     */
    for(count = 0;count < hid_instance->char_uuid_count;count++)
    {
       current = hid_instance->char_array[count];
       GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Current = %x, count = %d start_handle = %x\n",current,count,start_handle));
        if(current > start_handle) 
        {
             status = TRUE;
             break;
        }
    }
    if(status)
    {
        *end_handle = current - 1;
    }
    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_get_char_end_handles(), Exit:status = %d Handle = %x\n",
                                                    status,current-1));
}

/***************************************************************************
NAME
    hid_discover_next_char_descriptor

DESCRIPTION
   Get the next descriptor handle ranges and find the descriptors
*/
static bool  hid_discover_next_char_descriptor(GHIDC_T *const hid_client,
                gatt_hid_current_discovery_instance_t *hid_instance)
{
    uint16 start_handle = 0;
    uint16 end_handle = 0;
    /* Get the Start handle of characteritic decleration */
    hid_client_get_char_handles(hid_client,hid_instance,&start_handle);
    /* Store the end handle of service end handle as for last decleration the handle will be service endhandle */
    end_handle = hid_instance->end_handle;
    /* Find the end handle of characteristic */
    hid_client_get_char_end_handles(hid_instance,start_handle,&end_handle);
    /* If there is a valid start and end handle, then discover the characteristic Descriptor request */
    if(start_handle && end_handle)
    {
        /*As per core spec : The Attribute Protocol Find Information Request shall be used with the Starting
            Handle set to the handle of the specified characteristic value + 1 and the Ending
            Handle set to the ending handle of the specified characteristic.
         */
         /* Add +1 to start handle only if the start handle is less than end handle */
         if(start_handle<end_handle)
            start_handle+=1;
        GattManagerDiscoverAllCharacteristicDescriptors((Task)&hid_client->lib_task,start_handle,end_handle);
        GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_discover_next_char_descriptor(), Succes:Start :%x,End :%x \n",
                                                            start_handle,end_handle));
        return TRUE;
    }
    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_discover_next_char_descriptor(), Failed:Start :%x,End :%x \n",
                                                        start_handle,end_handle));
    return FALSE;
}

/***************************************************************************
NAME
    store_hid_client_ccd_handles

DESCRIPTION
   Stores the HID client characteristic configuration handles 
*/
static void store_hid_client_ccd_handles(GHIDC_T *const hid_client,
            gatt_hid_current_discovery_instance_t  *hid_instance,
            uint16 handle)
{
    uint16 index = 0;
    uint16 max_ccd_num = hid_instance->max_bootmode_ccd+hid_instance->max_reportmode_ccd;
    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:store_hid_client_ccd_handles() Entry \n"));
    /* Check what type of handle is requested */
    if(hid_instance->is_boot_handle)
    {
        if(hid_client->boot_mode_supported)
        {
            for (index = 0; index < max_ccd_num; index ++)
            {
                /* Update the Boot CCD */
                if ((!hid_client->ccd_handles[index].ccd)&& (hid_client->num_boot_ccd < hid_instance->max_bootmode_ccd))
                {
                    /* Free CCD found, update and exit */
                    hid_client->ccd_handles[index].ccd = handle;
                    hid_client->ccd_handles[index].ccd_type = hid_client_boot_mode;
                    hid_client->ccd_handles[index].characterisitc_handle = hid_instance->characteristic_handle;
                    hid_client->num_boot_ccd++;
                    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:store_hid_client_ccd_handles(),handle = %x,numBootCCD= %x \n",
                                                                        handle,hid_client->num_boot_ccd));
                    break;
                }
            }
        }
        hid_instance->is_boot_handle = FALSE;
    }
    else
    {
        /* Update in the Report CCD handles */
        for (index = 0; index < max_ccd_num; index ++)
        {
            if ((!hid_client->ccd_handles[index].ccd)&&(hid_client->num_report_ccd <hid_instance->max_reportmode_ccd ))
            {
                /* Free CCD found, update and exit */
                hid_client->ccd_handles[index].ccd = handle;
                hid_client->ccd_handles[index].ccd_type = hid_client_report_mode;
                hid_client->ccd_handles[index].characterisitc_handle = hid_instance->characteristic_handle;
                hid_client->num_report_ccd++;
                GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:store_hid_client_ccd_handles(),handle = %x,numReportCCD=%x \n",
                                                                handle,hid_client->num_report_ccd));
                break;
            }
        }
    }
}

/***************************************************************************
NAME
    store_hid_client_characteristic_handles

DESCRIPTION
   Stores the HID characteristic handles for future use 
*/
static void store_hid_client_characteristic_handles(GHIDC_T *const hid_client,
                 gatt_hid_current_discovery_instance_t *hid_instance,
                 uint16 uuid,
                 uint16 handle)
{
     /* Create a report map instance for handling HID report   */
    gatt_hid_client_report_id_map_t *report_id_map;
    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:store_hid_client_characteristic_handles(),UUID : %x, handle : %x\n",
                                                        uuid,handle));
    /* Store the characteristic handles  */
    switch(uuid)
    {
        /* HID Control Point */
        case GATT_HID_CONTROL_POINT_UUID:
        {
            hid_client->control_handle = handle;
        }
        break;
        /* Protocol Mode*/
        case GATT_HID_PROTOCOL_MODE_UUID:
        {
            hid_client->protocol_handle = handle;
        }
        break;
        /* Boot KeyBoard Input Report */
        case GATT_HID_BOOT_KB_INPUT_UUID:
        {
            if(hid_client->boot_mode_supported)
            {
                hid_client->boot_handles.boot_kb_input_report_handle = handle;
                /* Update Bit mask for reading charecteristic descriptors for keyboard input report */
                hid_instance->char_descriptor_mask |= GATT_HID_BOOT_KEYBOARD_INPUT_SUPPORT_MASK;
            }
        }
        break;
        /* Boot Mouse Output Report */
        case GATT_HID_BOOT_MOUSE_INPUT_UUID:
        {
            if(hid_client->boot_mode_supported)
            {
                hid_client->boot_handles.boot_mouse_input_report_handle = handle;
                /* Update Bit mask for reading charecteristic descriptors for mouse input report */
                hid_instance->char_descriptor_mask |= GATT_HID_BOOT_MOUSE_INPUT_SUPPORT_MASK;
            }
        }
        break;
        /* Boot KeyBoard Output Report */
        case GATT_HID_BOOT_OUTPUT_UUID:
        {
            if(hid_client->boot_mode_supported)
            {
                hid_client->boot_handles.boot_output_report_handle = handle;
            }
        }
        break;
        /* The below UUIDs are valid only for Report Mode Device */
        /* HID Report */
        case GATT_HID_REPORT_UUID:
        {
            if(hid_instance->num_reports_available < hid_instance->max_report)
            {
                /* Get the report memory for this instance of report */
                report_id_map = (hid_client->report_id_map)+(hid_instance->num_reports_available);
                report_id_map->report_handle = handle;
                report_id_map->report_reference_handle = GATT_HID_INVALID_HANDLE;
                /* Update the number of reports available */
                hid_instance->num_reports_available++;
            }
            GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:store_hid_client_characteristic_handles(),NumReports : %d\n",
                                                            hid_instance->num_reports_available));
            /* Update Bit mask for reading characteristic descriptors for report reference */
            hid_instance->char_descriptor_mask |= GATT_HID_REPORT_REFERENCE_SUPPORT_MASK;
        }
        break;
        /* HID Report Map */
        case GATT_HID_REPORT_MAP_UUID:
        {
            hid_client->report_map_handle= handle;
            /* Update Bit mask for reading characteristic descriptors for report map */
            hid_instance->char_descriptor_mask |= GATT_HID_REPORT_MAP_SUPPORT_MASK;
        }
        break;
        /* HID Information  */
        case GATT_HID_INFORMATION_UUID:
        {
            hid_client->info_handle = handle;
        }
        break;

        default:
        {
           /* Unknown Handle, Should not reach here */
           GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:store_hid_client_characteristic_handles(),UnKnwnHandle\n"));
        }
        break;
    }
     GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:store_hid_client_characteristic_handles(),Exit \n"));
}

/***************************************************************************
NAME
    store_hid_client_char_descriptors

DESCRIPTION
   Stores the HID characteristic descriptor handles for future use 
*/
static void store_hid_client_char_descriptors(GHIDC_T *const hid_client,
                gatt_hid_current_discovery_instance_t *hid_instance,
                uint16 uuid,
                uint16 handle)
{
    /* Temporary instance of report map, used to navigate through available reports */
    gatt_hid_client_report_id_map_t *report_id_map = NULL;

    switch (uuid)
    {
        /* Report Reference */
        case GATT_HID_REPORT_REFERENCE_UUID:
        {
            if(hid_instance->num_report_ids < hid_instance->max_report)
            {
                /* Get the instance for report id  map */
                report_id_map = hid_client->report_id_map + (hid_instance->num_report_ids);
                if(report_id_map->report_reference_handle == GATT_HID_INVALID_HANDLE)
                {
                   /* Store the handle of report reference descriptor. 
                    * It is required to read the report type and id later
                    */
                    report_id_map->report_reference_handle = handle;
                    hid_instance->num_report_ids++;
                }
            }
           GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:store_hid_client_char_descriptors(),numreportId %d \n",
                                                            hid_instance->num_report_ids));
        }
        break;
        /* External Report Reference */
        case GATT_HID_EXT_REPORT_REFERENCE_UUID:
        {
            hid_client->external_report_reference_handle =  handle;
            GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:store_hid_client_char_descriptors(),ext ref handle = %x \n",handle));

        }
        break;
        /* Client Charecteristic Configuration */
        case GATT_HID_CHARACTERISTIC_CONFIGURATION_UUID:
        {
            /* Notification handles need to be stored for future use 
            * as application need to register for notification 
            */
            GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:store_hid_client_char_descriptors(),StoreCCD Handle=%x \n",handle));
            store_hid_client_ccd_handles(hid_client,hid_instance,handle);
        }
        break;

        default:
        {
           /* Ignore */
           GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:store_hid_client_char_descriptors(),UnKnown UUID \n"));
        }
        break;
    }
}

/***************************************************************************/
/* Interface functions inside HID lib, HID message Handler uses this to update discovery confirmation*/
/***************************************************************************/

/****************************************************************************/
void handle_hid_client_discover_read_char_value_cfm(GHIDC_T *const hid_client,
             const GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM_T *const cfm )
{
    uint16 count = 0;
    gatt_hid_client_report_id_map_t *report_id_map = NULL;
    /* get the discovery instance */
    gatt_hid_current_discovery_instance_t *hid_instance = hid_discovery_instance;

    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:handle_hid_client_discover_read_char_value_cfm(), Entry\n"));

    if(CHECK_GATT_SUCCESS(cfm->status) && (hid_instance))
    {
        /* Find out the report id map instance and update the same */
        for(count = 0;count < hid_instance->num_report_ids;count++)
        {
            report_id_map = (hid_client->report_id_map )+(count);
            if(report_id_map->report_reference_handle == cfm->handle)
            {
                /* Update report ID and Type */
                report_id_map->report_id = cfm->value[0];
                report_id_map->type = cfm->value[1];
                hid_instance->read_report_id_count++;
                GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:handle_hid_client_discover_read_char_value_cfm(), ID: %x, Type: %x\n",report_id_map->report_id,report_id_map->type ));
            }
        }
       /* If all the report ID and Types are found out, then inform HID client LIB init is completed to registred task */
       if(hid_instance->num_report_ids == hid_instance->read_report_id_count)
       {
            hid_client_send_discovery_cfm(hid_client,hid_instance,cfm->cid,gatt_hid_client_status_success);
       }
   }
   else
   {
     /* Error in finding report ID, inform this to application */
      hid_client_send_discovery_cfm(hid_client,hid_instance,cfm->cid,hid_client_convert_status(cfm->status));    
   }
}

/***************************************************************************/
void handle_hid_client_discover_all_char_descriptors_cfm(GHIDC_T *const hid_client,
                const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T *const cfm)
{

    bool discovery_pending = TRUE;
    uint16 count = 0;
    /* Temporary instance of report map, used to navigate through available reports */
    gatt_hid_client_report_id_map_t *report_id_map = NULL;
    /* Get Hid Discovery Instance */
    gatt_hid_current_discovery_instance_t *hid_instance = hid_discovery_instance;

    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:handle_hid_client_discover_all_char_descriptors_cfm(), Entry\n"));
    
   /* Check the status of characteristic descriptor discovery, if it is failed, then check the descriptor is not mandatory.
    * In case failure happened for a non-mandatory characteristic descriptor, proceed to next descriptor discovery
    */
    if((CHECK_GATT_SUCCESS(cfm->status) || CHECK_DESCRIPTOR_NOT_MANDATORY((uint16)cfm->uuid[0])) && (hid_instance))
    {
        /* Store the handle for future use only if the status is success */
        if(CHECK_GATT_SUCCESS(cfm->status))
            store_hid_client_char_descriptors(hid_client,hid_instance,(uint16)cfm->uuid[0],cfm->handle);
        
        if(!cfm->more_to_come)
        {
            discovery_pending = FALSE;
            /* Check any more descriptors are required to read */
             if(hid_instance->char_descriptor_mask)
            {
                /* Start finding next descriptor */
                GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:handle_hid_client_discover_all_char_descriptors_cfm(), Find Next\n"));
                discovery_pending = hid_discover_next_char_descriptor(hid_client,hid_instance);
            }
        }
        GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:handle_hid_client_discover_all_char_descriptors_cfm(), discovery_pending : %d, mask = %d\n",
                                                        discovery_pending,hid_instance->char_descriptor_mask));
        /* Check Discovery is pending */
        if(!discovery_pending) 
        {
            /* Read report ID's */
            GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:handle_hid_client_discover_all_char_descriptors_cfm(),NumID = %d \n",
                                                              hid_instance->num_report_ids));
            for(count = 0;count < hid_instance->num_report_ids;count++)
            {
                /* Find out report id map instance */
                report_id_map = (hid_client->report_id_map )+(count);
                GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:handle_hid_client_discover_all_char_descriptors_cfm(),: FindreportId: handle = %x \n",
                                                                report_id_map->report_reference_handle));
                GattManagerReadCharacteristicValue((Task)&hid_client->lib_task,
                                                                        report_id_map->report_reference_handle);

            }
            /* Reports are optional as per HID, so may be there could be no reports available */
            if(hid_instance->num_report_ids == 0)
            {
                GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:handle_hid_client_discover_all_char_descriptors_cfm(), Send Inti CFM to app\n"));
                hid_client_send_discovery_cfm(hid_client,hid_instance,cfm->cid,gatt_hid_client_status_success);
            }
        }
    }
    else
    {
       /* Error, inform to the registered task */
        hid_client_send_discovery_cfm(hid_client,hid_instance,cfm->cid,hid_client_convert_status(cfm->status));    
    }
}

/***************************************************************************/
void handle_hid_client_discover_all_char_cfm(GHIDC_T *const hid_client,
               const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM_T *const cfm)
{
    /* UUID returned from characteristic handle discovery */
    uint16 uuid = 0;
    /* Still characteristic handle descovery is on  */
    bool discovery_pending = TRUE;
    /* Get the Discovery instance */
    gatt_hid_current_discovery_instance_t *hid_instance = hid_discovery_instance;

    /*Verify the confirmation status and discovery is still in progress */
    if(CHECK_GATT_SUCCESS(cfm->status) && (hid_instance))
    {
        if(cfm->uuid_type == gatt_uuid16)
        {
            /* Extract Charecteristic UUID */
            uuid = cfm->uuid[0];
            /* Check expected UUID handles are received */
            if(hid_instance->char_uuid_count < hid_instance->max_char_handles)
            {
                /* Update Handle Array for using them in descriptor querry  */
                hid_instance->char_array[hid_instance->char_uuid_count++] = cfm->declaration;
            }
            /* Store the handle for future use  */
            store_hid_client_characteristic_handles(hid_client,hid_instance,uuid,cfm->handle);
            /* No More characteristic pending  */
            if(!cfm->more_to_come)
            {
                discovery_pending = FALSE;
                /* Any descriptors need to be read ?, then proceed for the same */
                if(hid_instance->char_descriptor_mask)
                {
                    /* Start reading Characterisitic Descriptors */
                    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:handle_hid_client_discover_all_char_cfm(), Find descriptors \n"));
                    discovery_pending = hid_discover_next_char_descriptor(hid_client,hid_instance);
                }
            }
        }  
        /* Check Discovery Completed */
        if(!discovery_pending)
        {
            GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:handle_hid_client_discover_all_char_cfm(), Discovery Completed \n"));
            /* Report discovery complete to registered task   */
            hid_client_send_discovery_cfm(hid_client,hid_instance,cfm->cid,gatt_hid_client_status_success);
        }
    }
    else
    {
        /* Error In Characteristic Discovery, can not proceed further , Report to registered task */
        hid_client_send_discovery_cfm(hid_client,hid_instance,cfm->cid,hid_client_convert_status(cfm->status));
    }
}

/***************************************************************************/
/* Interface functions inside HID lib, HID message Handler uses this to process internal messages     */
/***************************************************************************/

/****************************************************************************/
void  hid_client_start_discovery(GHIDC_T *const hid_client)
{
        GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_start_discovery(), Entry\n"));

        /* Initiate discovery of all charecteristic of the HID service instance */
        GattManagerDiscoverAllCharacteristics((Task)&hid_client->lib_task);
}

/****************************************************************************/
void hid_client_complete_discovery(GHIDC_T *const hid_client, uint16 cid)
{
    hid_client->discovery_in_progress = FALSE;
    hid_client_send_discovery_cfm(hid_client,NULL,cid,gatt_hid_client_status_success);
}
/***************************************************************************/
/* Utility Interface functions inside HDI lib, This can be used to update discovery instance              */
/***************************************************************************/
/****************************************************************************/
bool hid_client_discovery_in_progress(void)
{
    if(hid_discovery_instance)
        return TRUE;
    else
        return FALSE;
}

/****************************************************************************/
bool hid_client_init_discovery_instance(const GATT_HID_CLIENT_CONFIG_PARAMS_T *const cfg,
        uint16 end_handle)
{
    uint16 hid_instance_size = 0;
    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_init_discovery_instance(), Entry\n"));

    /* Reset Discoevery Instance ,This is a temporary instance used only for discovery, 
    once the discovery is completed, this instance is not valid*/
    

    /* Check cfg params are NULL, then use default settings */
    if(cfg == NULL)
    {
        hid_instance_size = SIZE_HID_DISCOVERY_INSTANCE(MAX_CHAR_UUID_HANDLE);
        hid_discovery_instance = (gatt_hid_current_discovery_instance_t*)PanicNull(calloc(1,hid_instance_size));

        GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_init_discovery_instance(), Config Defualt values\n"));
         /*Set Default Values */
        hid_discovery_instance->max_report = MAX_NUM_HID_REPORTS;
        hid_discovery_instance->max_reportmode_ccd = MAX_NUM_REPORT_CCD;
        hid_discovery_instance->max_bootmode_ccd = MAX_NUM_BOOT_CCD;
        hid_discovery_instance->max_char_handles = MAX_CHAR_UUID_HANDLE;
        hid_discovery_instance->boot_mode_supported = TRUE;          
    }
    else
    {
        hid_instance_size = SIZE_HID_DISCOVERY_INSTANCE(cfg->max_num_char_handles);

        hid_discovery_instance = (gatt_hid_current_discovery_instance_t*)PanicNull(calloc(1,hid_instance_size));

        /* Use configured settings */
        hid_discovery_instance->max_report = cfg->max_num_report; 
        hid_discovery_instance->max_reportmode_ccd = cfg->max_num_reportmode_ccd;
        /*Check boot mode supported */
        if((cfg->max_num_bootmode_ccd) && (cfg->is_boot_mode_supported))
        {
            hid_discovery_instance->max_bootmode_ccd = cfg->max_num_bootmode_ccd;
            hid_discovery_instance->boot_mode_supported = TRUE;
        }
        else
        {
            hid_discovery_instance->max_bootmode_ccd = 0;
            hid_discovery_instance->boot_mode_supported = FALSE;
        }
        hid_discovery_instance->max_char_handles = cfg->max_num_char_handles;

    }
    hid_discovery_instance->char_descriptor_mask = 0;
    hid_discovery_instance->characteristic_handle = INVALID_HID_HANDLE;
    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_init_discovery_instance(),End handle %x\n",end_handle));
    hid_discovery_instance->end_handle = end_handle;
    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_init_discovery_instance(), Exit\n"));

    return TRUE;
}

/****************************************************************************/
void hid_client_cleanup_discovery_instance(void)
{
    if(hid_discovery_instance != NULL)
    {
        free(hid_discovery_instance);
        hid_discovery_instance = NULL;
    }
}

