/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. 

*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gatt_running_speed_cadence_server_private.h"

#include "gatt_running_speed_cadence_server_msg_handler.h"

/* Only one instance of Heart rate Sensor is supported */
/****************************************************************************/
bool GattRSCServerInit(Task appTask, GRSCS_T *const rscs, uint16 start_handle, uint16 end_handle)
{   
    /*Registration parameters for Running Speed and Cadence library to GATT manager  */
    gatt_manager_server_registration_params_t reg_params;
    
    /* validate the input parameters */
    if( INPUT_PARAM_NULL(appTask,rscs))
    {
        GATT_RSC_SERVER_PANIC(("GRSCS: Invalid Initialization parameters"));
    }

    /* Reset all the service library memory */
    memset(rscs, 0, sizeof(GRSCS_T));
    
    /*Set up the library task handler for external messages Running Speed and Cadence library receives GATT manager messages here */
    rscs->lib_task.handler = RSCServerMsgHandler;
    
    /*Store application message handler as application messages need to be posted here */
    rscs->app_task = appTask;
    
    /* Fill in the registration parameters */
    reg_params.start_handle = start_handle;
    reg_params.end_handle   = end_handle;
    reg_params.task = &rscs->lib_task;
    
    /* Try to register this instance of running speed library to GATT manager */
    if (GattManagerRegisterServer(&reg_params) == gatt_manager_status_success)
    {
        /* Mark Lib init is done */
       return TRUE;
    }
    return FALSE;
}

/****************************************************************************/
bool GattRSCServerReadClientConfigResponse(const GRSCS_T *rscs, uint16 cid, uint16 client_config)
{
    /* Validate the input parameters */
    if(rscs == NULL)
    {
        GATT_RSC_SERVER_PANIC(("GRSCS: Null instance"));
    }

    /* Validate the input parameters */
    if ((cid == 0) || ((client_config != 0) && (client_config != 1)))
    {
        return FALSE;
    }
    
    /* Send the client config response */
    sendRSCServerConfigAccessRsp(rscs, cid, client_config);
    
    return TRUE;
}

/****************************************************************************/
bool GattRSCExtensionServerReadClientConfigResponse(const GRSCS_T *rscs, uint16 cid, uint16 client_config)
{
    /* Validate the input parameters */
    if(rscs == NULL)
    {
        GATT_RSC_SERVER_PANIC(("GRSCS: Null instance"));
    }

    /* Validate the input parameters */
    if ((cid == 0) || ((client_config != 0) && (client_config != 1)))
    {
        return FALSE;
    }

    /* Send the client config response */
    sendRSCExtensionServerConfigAccessRsp(rscs, cid, client_config);

    return TRUE;
}

/****************************************************************************/
bool GattRSCServerSendNotification(const GRSCS_T *rscs, uint16 cid, uint16 RSCm_length, uint8 *RSCm_data)
{
    /* Validate this instance */
    if(rscs == NULL)
    {
        GATT_RSC_SERVER_PANIC(("GRSCS: Null instance"));
    }

    /* Validate the input parameters */
    if ((cid == 0) || (RSCm_length == 0) || (RSCm_data == NULL))
    {
        return FALSE;
    }
    /* Send notification to GATT Manager */
    GattManagerRemoteClientNotify((Task)&rscs->lib_task, cid, HANDLE_RUNNING_SPEED_AND_CADENCE_MEASUREMENT, RSCm_length, RSCm_data);

    return TRUE;
}

/****************************************************************************/
bool GattRSCServerSendNotificationExtension(const GRSCS_T *rscs, uint16 cid, uint16 RSCm_length, uint8 *RSCm_data)
{
    /* Validate this instance */
    if(rscs == NULL)
    {
        GATT_RSC_SERVER_PANIC(("GRSCS: Null instance"));
    }

    /* Validate the input parameters */
    if ((cid == 0) || (RSCm_length == 0) || (RSCm_data == NULL))
    {
        return FALSE;
    }
    /* Send notification to GATT Manager */
    GattManagerRemoteClientNotify((Task)&rscs->lib_task, cid, HANDLE_RUNNING_SPEED_AND_CADENCE_EXTENSION, RSCm_length, RSCm_data);

    return TRUE;
}
