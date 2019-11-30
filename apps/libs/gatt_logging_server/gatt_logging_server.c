/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. 

*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gatt_logging_server_private.h"

#include "gatt_logging_server_msg_handler.h"

/****************************************************************************/
bool GattLoggingServerInit(Task appTask, GLOG_T *const logging, uint16 start_handle, uint16 end_handle)
{   
    /*Registration parameters for logging library to GATT manager  */
    gatt_manager_server_registration_params_t reg_params;
    
    /* validate the input parameters */
    if( INPUT_PARAM_NULL(appTask,logging))
    {
        GATT_LOGGING_SERVER_PANIC(("GLOG: Invalid Initialization parameters"));
    }

    /* Reset all the service library memory */
    memset(logging, 0, sizeof(GLOG_T));
    
    /*Set up the library task handler for external messages logging library receives GATT manager messages here */
    logging->lib_task.handler = LoggingServerMsgHandler;
    
    /*Store application message handler as application messages need to be posted here */
    logging->app_task = appTask;

    /* Set the logging control to 0 */
    logging->logging_control = 0;
    
    /* Fill in the registration parameters */
    reg_params.start_handle = start_handle;
    reg_params.end_handle   = end_handle;
    reg_params.task = &logging->lib_task;
    
    /* Try to register this instance of logging library to GATT manager */
    if (GattManagerRegisterServer(&reg_params) == gatt_manager_status_success)
    {
        /* Mark Lib init is done */
       return TRUE;
    }
    return FALSE;
}

/****************************************************************************/
bool GattLoggingServerReadClientConfigResponse(const GLOG_T *logging, uint16 cid, uint16 client_config)
{
    /* Validate the input parameters */
    if(logging == NULL)
    {
        GATT_LOGGING_SERVER_PANIC(("GLOG: Null instance"));
    }

    /* Validate the input parameters */
    if ((cid == 0) || ((client_config != 0) && (client_config != 1)))
    {
        return FALSE;
    }
    
    /* Send the client config response */
    sendLoggingServerConfigAccessRsp(logging, cid, client_config);
    
    return TRUE;
}

/****************************************************************************/
bool GattLoggingServerReadLoggingControlResponse(const GLOG_T *logging, uint16 cid, uint8 logging_control)
{
    /* Validate the input parameters */
    if(logging == NULL)
    {
        GATT_LOGGING_SERVER_PANIC(("GLOG: Null instance"));
    }

    /* Validate the input parameters */
    if (cid == 0)
    {
        return FALSE;
    }

    /* Send the client config response */
    sendLoggingServerLoggingControlRsp(logging, cid, logging_control);

    return TRUE;
}

/****************************************************************************/
bool GattLoggingServerSendNotification(const GLOG_T *logging, uint16 cid, uint16 log_length, uint8 *log_data)
{
    /* Validate this instance */
    if(logging == NULL)
    {
        GATT_LOGGING_SERVER_PANIC(("GLOG: Null instance"));
    }

    /* Validate the input parameters */
    if ((cid == 0) || (log_length == 0) || (log_data == NULL))
    {
        return FALSE;
    }
    /* Send notification to GATT Manager */
    GattManagerRemoteClientNotify((Task)&logging->lib_task, cid, HANDLE_LOGGING_REPORT, log_length, log_data);
    return TRUE;
}

/****************************************************************************/
bool GattLoggingServerReadLoggingDebugConfigurationResponse(const GLOG_T *logging, uint16 cid, uint16 debug_config_length, uint8 *debug_config_data)
{
    /* Validate this instance */
    if(logging == NULL)
    {
        GATT_LOGGING_SERVER_PANIC(("GLOG: Null instance"));
    }

    /* Validate the input parameters */
    if ((cid == 0) || (debug_config_length == 0) || (debug_config_data == NULL))
    {
        return FALSE;
    }
    /* Send notification to GATT Manager */
    GattManagerServerAccessResponse((Task)&logging->lib_task, cid, HANDLE_LOGGING_DEBUG_CONFIGURATION,
                                    gatt_status_success, debug_config_length, debug_config_data);

    return TRUE;
}
