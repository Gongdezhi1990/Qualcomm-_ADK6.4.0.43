/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_gatt_client_gatt.c

DESCRIPTION
    Routines to handle the GATT Client Service.
*/

#include "sink_ble.h"
#include "sink_gatt_client_gatt.h"

#include "sink_debug.h"
#include "sink_development.h"
#include "sink_gatt_client.h"

#include <stdlib.h>


#ifdef GATT_ENABLED


#ifdef DEBUG_GATT_SERVICE_CLIENT
#define GATT_SERVICE_CLIENT_INFO(x) DEBUG(x)
#define GATT_SERVICE_CLIENT_ERROR(x) DEBUG(x) TOLERATED_ERROR(x)
#else
#define GATT_SERVICE_CLIENT_INFO(x)
#define GATT_SERVICE_CLIENT_ERROR(x)
#endif


/*******************************************************************************
NAME
    gattServiceInitialised
    
DESCRIPTION
    Called when the GATT service is initialised.
    
PARAMETERS
    gbasc    The GATT client instance pointer
    
*/
static void gattServiceInitialised(const GGATTC *gattc)
{
    uint16 index = 0;
    gatt_client_services_t *data = NULL;
    gatt_client_connection_t *client = NULL;

    for (index = 0; index < MAX_BLE_CONNECTIONS; index++)
    {
        client = &GATT_CLIENT.connection;
        data = gattClientGetServiceData(client);
        if (data && (data->gattc == gattc))
        {
            /* GATT service has been discovered, now check for security requirements */
            gattClientProcessSecurityRequirements(client, data);
        }
    }
}


/*******************************************************************************
NAME
    sinkGattClientInitCfm
    
DESCRIPTION
    Handle the GATT_CLIENT_INIT_CFM message.
    
PARAMETERS
    cfm    The GATT_CLIENT_INIT_CFM message
    
*/
static void sinkGattClientInitCfm(const GATT_CLIENT_INIT_CFM_T *cfm)
{
    GATT_SERVICE_CLIENT_INFO(("GATT_CLIENT_INIT_CFM status[%u]\n",
                                       cfm->status));
    
    /* The service initialisation is complete */
    gattServiceInitialised(cfm->gatt_client);
}


/*******************************************************************************
NAME
    sinkGattClientServiceChangedInd
    
DESCRIPTION
    Handle the GATT_CLIENT_SERVICE_CHANGED_IND message.
    
PARAMETERS
    ind    The GATT_CLIENT_SERVICE_CHANGED_IND message
    
*/
static void sinkGattClientServiceChangedInd(const GATT_CLIENT_SERVICE_CHANGED_IND_T *ind)
{
    GATT_SERVICE_CLIENT_INFO(("GATT_CLIENT_SERVICE_CHANGED_IND cid=[%u] start=[%u] end=[%u]\n",
                                       ind->cid,
                                       ind->start_handle,
                                       ind->end_handle));
    
    gattClientServiceChanged(ind->cid);
}


/****************************************************************************/
bool sinkGattClientServiceAdd(uint16 cid, uint16 start, uint16 end)
{
    GGATTC *gattc = NULL;
    gatt_client_init_params_t init_params;
    gatt_client_services_t *client_services = NULL;
    gatt_client_connection_t *connection = gattClientFindByCid(cid);
    uint16 *service = gattClientGetServicePtr(connection, sizeof(GGATTC));
    
    if (service)
    {
        client_services = gattClientGetServiceData(connection);
        client_services->gattc = (GGATTC *)service;
        gattc = client_services->gattc;
        
        init_params.cid = cid;
        init_params.start_handle = start;
        init_params.end_handle = end;
        
        if (GattClientInit(sinkGetBleTask(), gattc, &init_params, NULL) == gatt_client_status_success)
        {            
            return TRUE;
        }
    }
    
    return FALSE;
}


/****************************************************************************/
void sinkGattClientServiceRemove(GGATTC *ggattc, uint16 cid)
{
    GattClientDestroy(ggattc, cid);
}


/******************************************************************************/
void sinkGattClientServiceMsgHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    switch(id)
    {
        case GATT_CLIENT_INIT_CFM:
        {
            sinkGattClientInitCfm((GATT_CLIENT_INIT_CFM_T*)message);
        }
        break;
        case GATT_CLIENT_SERVICE_CHANGED_IND:
        {
            sinkGattClientServiceChangedInd((GATT_CLIENT_SERVICE_CHANGED_IND_T*)message);
        }
        break;
        default:
        {
            GATT_SERVICE_CLIENT_ERROR(("Unhandled GATTC msg[%x]\n", id));
        }
        break;
    }
}


#endif /* GATT_ENABLED */

