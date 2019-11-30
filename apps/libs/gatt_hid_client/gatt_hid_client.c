/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#include <string.h>
#include <stdio.h>

#include "gatt_hid_client_private.h"

#include "gatt_hid_client_msg_handler.h"
#include "gatt_hid_client_discovery.h"


/****************************************************************************/
/*                                      Internal Helper Functions                                                              */                      
/****************************************************************************/

/***************************************************************************
NAME
    hid_client_start_handle_discovery

DESCRIPTION
    Utility function used to send internal message for HID server handle discovery.
*/
static void  hid_client_start_handle_discovery(GHIDC_T *const hid_client,
                const GATT_HID_CLIENT_INIT_PARAMS_T *const init_params,
                bool start_discovery)
{
    if(start_discovery)
    {
        /* Make internal message for HID Handle Discovery and Post it to Discovery Module  */
        MAKE_HID_CLIENT_MESSAGE(HID_CLIENT_INTERNAL_MSG_DISCOVER);
        /* Copy the connection ID */
        message->cid = init_params->cid;
        /*  Copy the start Handle for srevice */
        message->start_handle = init_params->start_handle;
        /* Copy the end handle for service */
        message->end_handle = init_params->end_handle;
        /* Flag has to be set before sending HID_CLIENT_INTERNAL_MSG_DISCOVER */
        hid_client-> discovery_in_progress = TRUE;
        /* Send the discover request to message handler */
        MessageSend((Task)&hid_client->lib_task,HID_CLIENT_INTERNAL_MSG_DISCOVER, message);
    }
    else
    {
        /* Make internal message for HID Handle Discovery Complete and Post it to Discovery Module  */
        MAKE_HID_CLIENT_MESSAGE(HID_CLIENT_INTERNAL_MSG_DISCOVER_COMPLETE);
        /* Copy the connection ID */
        message->cid = init_params->cid;
        MessageSend((Task)&hid_client->lib_task,HID_CLIENT_INTERNAL_MSG_DISCOVER_COMPLETE, message);
    }

    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_start_handle_discovery(),Exit\n"));
}

/***************************************************************************
NAME
    hid_client_allocate_memory_for_handles

DESCRIPTION
    Utility function used to allocate memory for handles configured by application  .
*/
static void hid_client_allocate_memory_for_handles(GHIDC_T *const hid_client,
               const GATT_HID_CLIENT_CONFIG_PARAMS_T *const cfg)
{
    unsigned ccd_entries_to_allocate = 0;
    unsigned reports_to_allocate = 0;

    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_allocate_memory_for_handles(),Entry\n"));
    /* Check handles are configured */    
    if(cfg != NULL )
    {
        /* Boot mode supported? */
        if((cfg->is_boot_mode_supported) && (cfg->max_num_bootmode_ccd > 0))
        {
            ccd_entries_to_allocate += cfg->max_num_bootmode_ccd;
            hid_client->boot_mode_supported = TRUE;
        }
        else
        {   /* Set Boot handles as NULL as client does not want to use remote device boot mode */
             hid_client->boot_mode_supported = FALSE;
        }
        ccd_entries_to_allocate += cfg->max_num_reportmode_ccd;
        reports_to_allocate = cfg->max_num_report;
    }
    /* Use Default parameters */
    else
    {
        GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_allocate_memory_for_handles(), Use Default Handle Settings \n"));

        hid_client->boot_mode_supported = TRUE;
        ccd_entries_to_allocate = MAX_NUM_BOOT_CCD+MAX_NUM_REPORT_CCD;
        reports_to_allocate = MAX_NUM_HID_REPORTS;
    }

    if (hid_client->boot_mode_supported)
    {
        memset(&hid_client->boot_handles,0,sizeof(hid_client->boot_handles));
    }
    hid_client->ccd_handles = (gatt_hid_ccd_handle_mem_t*)calloc(ccd_entries_to_allocate,sizeof(gatt_hid_ccd_handle_mem_t));
    hid_client->report_id_map = (gatt_hid_client_report_id_map_t *)calloc(reports_to_allocate,sizeof(gatt_hid_client_report_id_map_t));

    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_allocate_memory_for_handles(),Exit\n"));
}

/***************************************************************************
NAME
    hid_client_free_memory_for_handles

DESCRIPTION
    Utility function used to free the memory allocated for handles   .
*/
static void hid_client_free_memory_for_handles(GHIDC_T *const hid_client)
{
    /* Free up memory for all other handles which is allocated at time of Init */
    free(hid_client->ccd_handles);
    hid_client->ccd_handles= NULL;
    free(hid_client->report_id_map);
    hid_client->report_id_map= NULL;
}

/***************************************************************************
NAME
    hid_client_free_memory_for_handles

DESCRIPTION
    Utility function used to init hid handles    .
*/
static void hid_client_init_handles(GHIDC_T *const hid_client)
{
    /* Reset all Handles */
    hid_client->protocol_handle= INVALID_HID_HANDLE;
    hid_client->info_handle = INVALID_HID_HANDLE ;
    hid_client->control_handle = INVALID_HID_HANDLE ;
    hid_client->external_report_reference_handle = INVALID_HID_HANDLE ;
    hid_client->report_map_handle = INVALID_HID_HANDLE;
}

/****************************************************************************/
/*                                      External Interface Functions                                                          */                      
/****************************************************************************/

/****************************************************************************/
bool  GattHidClientInit(Task appTask, 
                                GHIDC_T *const hid_client,
                                const GATT_HID_CLIENT_INIT_PARAMS_T *const init_params,
                                const GATT_HID_CLIENT_CONFIG_PARAMS_T *const config_params)
{
    gatt_manager_client_registration_params_t reg_params;

    if(INPUT_PARAM_NULL(appTask,hid_client,init_params) || CONFIG_PARAM_INVALID(config_params))
    {   
       GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:GattHidClientInit(),Invalid Input Paramas \n"));
       Panic();
    }
    /* Two HID discovery cannot handle together, so check discovery status */
    if(hid_client_discovery_in_progress()) return FALSE;
    
    /* Reset the HID client data structure */
    memset(hid_client, 0, sizeof(GHIDC_T));
    /* Get memory for handles configured */
    hid_client_allocate_memory_for_handles(hid_client,config_params);
    /* Init the handles */
    hid_client_init_handles(hid_client);
    /* Store the application handle where all the confirmation and indication messages need be posted */
    hid_client->app_task = appTask;
    /* Store the Library task where all the external call backs need to be received */
    hid_client->lib_task.handler = hid_client_msg_handler;

    /* SetUp GATT manager registartion parameters */
    reg_params.cid =  init_params->cid;
    reg_params.client_task = &hid_client->lib_task;
    reg_params.end_handle = init_params->end_handle;
    reg_params.start_handle = init_params->start_handle;
    
    /* Try to register HID Client with GATT manager for notifications and indications */
    if(GattManagerRegisterClient(&reg_params) == gatt_manager_status_success)
    {
        /* Update the discovery intstance parameters */
        if(hid_client_init_discovery_instance(config_params, init_params->end_handle))
        {
            /* Start HID service handle discovery and returns status as initiated */
            hid_client_start_handle_discovery(hid_client,init_params,TRUE);
            GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:GattHidClientInit(),Sucess, Start Discovery of handles \n"));
            return TRUE;
        }
    } 
    return FALSE;

}

/****************************************************************************/
bool  GattHidClientDeInit(GHIDC_T *const hid_client)
{

    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:GattHidClientDeInit(),Entry\n"));
   
    /* Validate the Input Parameters */
    PanicNull(hid_client);

    /* Deregister the client from GATT manager */
    if(GattManagerUnregisterClient((Task)&hid_client->lib_task) == gatt_manager_status_success)
    {
        /* Free memory for all handles allocated inside HID client LIB */
        hid_client_free_memory_for_handles(hid_client);
        /* If Discovery in progress for this HID instance while DeInit, Inform this to discovery module */
        if(DISCOVERY_IN_PROGRESS(hid_client))
        {
            GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:GattHidClientDeInit(),Discovery In Progress %d\n",hid_client->discovery_in_progress));
            hid_client_cleanup_discovery_instance();
        }
        
        /* Clear pending messages */
        MessageFlushTask((Task)&hid_client->lib_task);
        return TRUE;
    }

    GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:GattHidClientDeInit(),Exit\n"));
    return FALSE;
}

