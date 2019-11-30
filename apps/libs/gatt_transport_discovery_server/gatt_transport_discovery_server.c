/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. */
/*  */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gatt_transport_discovery_server_private.h"
#include "gatt_transport_discovery_server_msg_handler.h"

/****************************************************************************/
bool GattTransportDiscoveryServerInit(Task appTask, GTDS_T *const tds, uint16 start_handle, uint16 end_handle)
{   
    /*Registration parameters for TDS library to GATT manager  */
    gatt_manager_server_registration_params_t reg_params;
    
    /* validate the input parameters */
    if ((appTask == NULL) || (tds == NULL ))
    {
        GATT_TDS_SERVER_PANIC(("GHRS: Invalid Initialization parameters"));
    }

    /* Reset all the service library memory */
    memset(tds, 0, sizeof(GTDS_T));
    
    /* Set up the library task handler for external messages
     * TDS library receives GATT manager messages here
     */
    tds->lib_task.handler = tdsServerMsgHandler;
    /*Store application message handler as application messages need to be posted here */
    tds->app_task = appTask;
    /* Fill in the registration parameters */
    reg_params.start_handle = start_handle;
    reg_params.end_handle   = end_handle;
    reg_params.task = &tds->lib_task;
    /* Try to register this instance of TDS library to GATT manager */
    if (GattManagerRegisterServer(&reg_params) == gatt_manager_status_success)
    {
        /* Mark Lib init is done */
       return TRUE;
    }
    return FALSE;
}

/****************************************************************************/
bool GattTdsServerReadClientConfigResponse( const GTDS_T *tds, uint16 cid, uint16 client_config)
{
    /* Validate the input parameters */
    if(tds == NULL)
    {
        GATT_TDS_SERVER_PANIC(("GTDS: Null instance"));
    }

    /* Validate the input parameters */
    if ( (cid == 0)  || (client_config > 0x04) )
    {
        return FALSE;
    }
    
    /* Send the client config response */
    sendTdsServerConfigAccessRsp(tds, cid, client_config);
    
    return TRUE;
}


/****************************************************************************/
bool GattTdsServerSendNotification(const GTDS_T *tds, uint16 cid, uint16 tds_ind_size, uint8 *tds_ind_data)
{
    /* Validate this instance */
    if(tds == NULL)
    {
        GATT_TDS_SERVER_PANIC(("GTDS: Null instance"));
    }

    /* Validate the input parameters */
    if ((cid == 0) || (tds_ind_size == 0) || (tds_ind_data == NULL))
    {
        return FALSE;
    }
    /* Send notification to GATT Manager */
    GattManagerRemoteClientIndicate((Task)&tds->lib_task, cid, HANDLE_TRANSPORT_DISCOVERY_CONTROL_POINT, tds_ind_size, tds_ind_data);
    return TRUE;
}


/***************************************************************************
NAME
    GattTdsServerSendResponse
    
DESCRIPTION
    Send a response through the GATT Manager library.
*/
void GattTdsServerSendResponse(const GATT_TRANSPORT_DISCOVERY_SERVER_WRITE_CONTROL_POINT_IND_T *ind, uint16 result)
{
    sendTdsServerAccessRsp((Task) &(ind->tds->lib_task), ind->cid, ind->handle, result, 0, NULL);
}

