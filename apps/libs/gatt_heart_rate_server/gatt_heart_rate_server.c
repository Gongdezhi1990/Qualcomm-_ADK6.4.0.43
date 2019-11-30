/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gatt_heart_rate_server_private.h"

#include "gatt_heart_rate_server_msg_handler.h"

/* Only one instance of Heart rate Sensor is supported */
/****************************************************************************/
bool GattHrServerInit(Task appTask, GHRS_T *const hr_sensor,
						   uint16 start_handle,
						   uint16 end_handle)
{   
    /*Registration parameters for Heart rate Sensor library to GATT manager  */
    gatt_manager_server_registration_params_t reg_params;
    
    /* validate the input parameters */
    if( INPUT_PARAM_NULL(appTask,hr_sensor))
    {
        GATT_HR_SERVER_PANIC(("GHRS: Invalid Initialisation parameters"));
    }

    /* Reset all the service library memory */
    memset(hr_sensor, 0, sizeof(GHRS_T));
    /*Set up the library task handler for external messages
     * Heart rate sensor library receives gatt manager messages here
     */
    hr_sensor->lib_task.handler = hrServerMsgHandler;
    /*Store application message handler as application messages need to be posted here */
    hr_sensor->app_task = appTask;
    /* Fill in the registration parameters */
    reg_params.start_handle = start_handle;
    reg_params.end_handle   = end_handle;
    reg_params.task = &hr_sensor->lib_task;
    /* Try to register this instance of Heart rate sensor library to Gatt manager */
    if (GattManagerRegisterServer(&reg_params) == gatt_manager_status_success)
    {
        /* Mark Lib init is done */
       return TRUE;
    }
    return FALSE;
}

/****************************************************************************/
bool GattHrServerReadClientConfigResponse(const GHRS_T *hr_sensor,
                            uint16 cid, uint16 client_config)
{
    /* Validate the input parameters */
    if(hr_sensor == NULL)
    {
        GATT_HR_SERVER_PANIC(("GHRS: Null instance"));
    }

    /* Validate the input parameters */
    if ((cid == 0) || ((client_config != 0) && (client_config != 1)))
    {
        return FALSE;
    }
    
    /* Send the client config response */
    sendHrServerConfigAccessRsp(hr_sensor, cid, client_config);
    
    return TRUE;
}

/****************************************************************************/
bool GattHrServerSendNotification(const GHRS_T *hr_sensor, uint16 cid,
                                   uint16 hrm_length, uint8 *hrm_data)
{
    /* Validate this instance */
    if(hr_sensor == NULL)
    {
        GATT_HR_SERVER_PANIC(("GHRS: Null instance"));
    }

    /* Validate the input parameters */
    if ((cid == 0) || (hrm_length == 0) || (hrm_data == NULL))
    {
        return FALSE;
    }
    /* Send notification to GATT Manager */
    GattManagerRemoteClientNotify((Task)&hr_sensor->lib_task, cid, HANDLE_HEART_RATE_MEASUREMENT, hrm_length, hrm_data);
    return TRUE;
}
