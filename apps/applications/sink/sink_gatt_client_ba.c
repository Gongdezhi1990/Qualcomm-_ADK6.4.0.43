/****************************************************************************
Copyright (c) 2015 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_gatt_client_ba.c

DESCRIPTION
    Routines to handle the GATT BroadcastAudio Client.
*/

#include <stdlib.h>
#include <connection.h>

#include "sink_ble.h"
#include "sink_gatt_client_ba.h"

#include "sink_debug.h"
#include "sink_gatt_client.h"
#include "config_definition.h"
#include "sink_ba_receiver.h"

#ifdef GATT_BA_CLIENT
#include <config_store.h>

static const uint8 ba_ble_advertising_filter[] = {0x05, 0x79, 0x31, 0xf4, 0xce, 0xb5, 0x99, 0x4e, 0x0f, 0xa4, 0x1e, 0x4b, 0x2d, 0x12, 0xd0, 0x00};

#ifdef DEBUG_GATT_BA_CLIENT
#define GATT_BA_CLIENT_DEBUG(x) DEBUG(x)
#else
#define GATT_BA_CLIENT_DEBUG(x) 
#endif


/*******************************************************************************
NAME
    gattBAFindConnection
    
DESCRIPTION
    Finds a client connection associated with an broadcastAudio instance.
    
PARAMETERS
    gbsc    The BA client instance pointer
    
RETURNS    
    The client connection pointer associated with the BA instance. NULL if not found.   
    
*/
static gatt_client_connection_t *gattBAFindConnection(const GBSC *gbsc)
{
    uint16 index = 0;
    gatt_client_services_t *data = NULL;
    
    if (gbsc == NULL)
    {
        return NULL;
    }
    
    for (index = 0; index < MAX_BLE_CONNECTIONS; index++)
    {
        data = gattClientGetServiceData(&GATT_CLIENT.connection);
        if (data && (data->ba == gbsc))
        {
            return &GATT_CLIENT.connection;
        }
    }
    
    return NULL;
}

/*******************************************************************************
NAME
    gattBAServiceInitialised
    
DESCRIPTION
    Called when the BA service is initialised.
    
PARAMETERS
    gancs    The BA client instance pointer
    
*/
static void gattBAServiceInitialised(const GBSC *gbsc)
{
    gatt_client_connection_t *conn = gattBAFindConnection(gbsc);
    uint16 cid = gattClientFindByConnection(conn);
 
    if (conn != NULL)
    {
        /* Done with this BA client */
        gattClientDiscoveredServiceInitialised(conn);
    }
     /* Disconnect the link as we are no more interested in maintaining after the association */
     gattClientDisconnect(cid);
}

/*! @brief Handle the association data received from a broadcaster. */
static void gattBAHandleServiceInitCfm(GATT_BROADCAST_CLIENT_INIT_CFM_T* cfm)
{
    if(cfm->ba_client)
    {
         /* Check if the received association data is fine */
         sinkReceiverHandleAssocRecvCfm(cfm->assoc_data);

         /* We are done with broadcast service, just finish the service initialising procedure */
        gattBAServiceInitialised(cfm->ba_client);
    }
}

/****************************************************************************/
void sinkGattBAClientSetupAdvertisingFilter(void)
{
    GATT_BA_CLIENT_DEBUG(("Add BA scan filter\n"));
    ConnectionBleAddAdvertisingReportFilter(ble_ad_type_more_uuid16, sizeof(ba_ble_advertising_filter), sizeof(ba_ble_advertising_filter), ba_ble_advertising_filter);
    ConnectionBleAddAdvertisingReportFilter(ble_ad_type_complete_uuid16, sizeof(ba_ble_advertising_filter), sizeof(ba_ble_advertising_filter), ba_ble_advertising_filter);
}
    
/****************************************************************************/
bool sinkGattBAClientAddService(uint16 cid, uint16 start, uint16 end)
{
    GBSC *ba = NULL;
    gatt_client_services_t *client_services = NULL;
    gatt_client_connection_t *connection = gattClientFindByCid(cid);
    uint16 *service = gattClientGetServicePtr(connection, sizeof(GBSC));
    
    if (service)
    {
        GATT_BROADCAST_CLIENT_INIT_PARAMS_T init_param;
        client_services = gattClientGetServiceData(connection);
        client_services->ba = (GBSC *)service;
        ba = client_services->ba;
        
        init_param.cid = cid;
        init_param.start_handle = start;
        init_param.end_handle = end;

        if (GattBroadcastClientInit(sinkGetBleTask(), ba, &init_param) == gatt_broadcast_client_status_success)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/****************************************************************************/
void sinkGattBAClientRemoveService(GBSC *gbcs)
{
    GattBroadcastClientDestroy(gbcs);
}


/******************************************************************************/
void sinkGattBAClientMsgHandler (Task task, MessageId id, Message message)
{
    UNUSED(task);
    
    switch(id)
    {
        case GATT_BROADCAST_CLIENT_INIT_CFM:
        {
            gattBAHandleServiceInitCfm((GATT_BROADCAST_CLIENT_INIT_CFM_T*)message);
        }
        break;
        default:
        {
            GATT_BA_CLIENT_DEBUG(("Unhandled BA msg[%x]\n", id));
        }
        break;
    }
}


#endif /* GATT_BA_CLIENT */

