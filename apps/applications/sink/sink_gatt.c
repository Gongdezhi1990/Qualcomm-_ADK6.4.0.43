/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_gatt.c        

DESCRIPTION
    Contains GATT functionality.
*/
#include "sink_gatt.h"

#include "sink_development.h"
#include "sink_gatt_client.h"
#include "sink_gatt_device.h"
#include "sink_gatt_server_gap.h"
#include "sink_debug.h"
#include "sink_ble_advertising.h"
#include "sink_ble.h"
#include "sink_ba_receiver.h"

#include <gatt.h>
#include <connection_no_ble.h>
#include <gatt_broadcast_server_uuids.h>
#include <ahi.h>
#include <csrtypes.h>
#include <vm.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef GATT_ENABLED


#ifdef DEBUG_GATT
#define GATT_INFO(x) DEBUG(x)
#define GATT_ERROR(x) DEBUG(x) TOLERATED_ERROR(x)
#else 
#define GATT_INFO(x) 
#define GATT_ERROR(x) 
#endif

#ifndef MIN
#define MIN(a,b)    (((a)<(b))?(a):(b))
#endif

#define SINK_GATT_DEFAULT_MTU_SIZE    (64)

static uint16 mtu_size = SINK_GATT_DEFAULT_MTU_SIZE;

/*******************************************************************************
NAME
    handleGattExchangeMtuInd
    
DESCRIPTION
    Handle when GATT_EXCHANGE_MTU_IND message was received
    
PARAMETERS
    ind Pointer to a GATT_EXCHANGE_MTU_IND message
    
RETURNS
    TRUE if the message was handled, FALSE otherwise
*/
static void handleGattExchangeMtuInd(const GATT_EXCHANGE_MTU_IND_T * ind)
{
    GATT_INFO(("GATT_EXCHANGE_MTU_IND mtu %d\n", ind->mtu));
    
    if (ind)
    {
        GattExchangeMtuResponse(ind->cid, mtu_size);
    }
}

/*******************************************************************************

    handleGattExchangeMtuCfm
            
DESCRIPTION
    Handle when GATT_EXCHANGE_MTU_CFM message was received
    
PARAMETERS
    cfm Pointer to a GATT_EXCHANGE_MTU_CFM message
*/
static void handleGattExchangeMtuCfm(const GATT_EXCHANGE_MTU_CFM_T * cfm)
{
    uint8 index;
    tp_bdaddr *tp_addr = PanicUnlessMalloc(sizeof(tp_bdaddr));
    GATT_INFO(("GATT_EXCHANGE_MTU_CFM mtu %d \n",  cfm->mtu));
    if (cfm)
    {
        gatt_client_connection_t *connection = gattClientFindByCid(cfm->cid);
        gatt_client_services_t *data = gattClientGetServiceData(connection);

        if(connection && data && data->number_discovered_services == 0)
        {
            /* Discover primary services on connection */
            gattDiscoverPrimaryServices(connection);
            /* Discovery in progress only for peripheral role,
              * as this flag is not used for central role */
            if(connection->role == ble_gap_role_peripheral)
            {
                VmGetBdAddrtFromCid(cfm->cid, tp_addr);
                index = sinkBleGapFindGapConnFlagIndexByCid(cfm->cid);
                sinkBleGapSetDiscoveryInProgress(TRUE, &tp_addr->taddr, cfm->cid, index);
            }
        }
    }
    /* Free the allocated memory */
    free(tp_addr);
}


/*******************************************************************************
NAME
    handleGattDiscoverAllPrimaryServicesCfm
    
DESCRIPTION
    Handle when GATT_DISCOVER_ALL_PRIMARY_SERVICES_CFM message was received
    
PARAMETERS
    cfm Pointer to a GATT_DISCOVER_ALL_PRIMARY_SERVICES_CFM message
    
RETURNS
    void
*/
static void handleGattDiscoverAllPrimaryServicesCfm(const GATT_DISCOVER_ALL_PRIMARY_SERVICES_CFM_T * cfm)
{
    GATT_INFO(("GATT_DISCOVER_ALL_PRIMARY_SERVICES_CFM\n"));
    
    GATT_INFO(("   cid[0x%x] Start[0x%x] End[0x%x] more[%u]\n", 
                                                                cfm->cid,
                                                                cfm->handle,
                                                                cfm->end,
                                                                cfm->more_to_come
                                                                ));
    
    gattClientStoreDiscoveredService(cfm->cid, cfm->uuid_type, (uint32*)&cfm->uuid[0], cfm->handle, cfm->end, cfm->more_to_come);
}

/*******************************************************************************
NAME
    handleGattDiscoverPrimaryServicesCfm
    
DESCRIPTION
    Handle when GATT_DISCOVER_PRIMARY_SERVICES_CFM message was received
    
PARAMETERS
    cfm Pointer to a GATT_DISCOVER_ALL_PRIMARY_SERVICES_CFM message
    
RETURNS
    void
*/
static void handleGattDiscoverPrimaryServicesCfm(const GATT_DISCOVER_PRIMARY_SERVICE_CFM_T * cfm)
{
    GATT_INFO(("GATT_DISCOVER_PRIMARY_SERVICES_CFM\n"));
    
    GATT_INFO(("   cid[0x%x] Start[0x%x] End[0x%x] more[%u] status[%u]\n", 
                                                                cfm->cid,
                                                                cfm->handle,
                                                                cfm->end,
                                                                cfm->more_to_come,
                                                                cfm->status
                                                                ));
    if((cfm->status != gatt_status_success) &&
        BA_RECEIVER_MODE_ACTIVE && sinkReceiverIsBroadcasterCid(cfm->cid))
    {
        /* Check if receiver triggered the request for Broadcast Service */
        if(cfm->uuid[0] == UUID_BROADCAST_SERVICE)
        {
            /* Looks like this UUID is not supported. Lets check if the broadcaster
                supports the other UUID */
            gatt_uuid_t ba_uuid = UUID_BROADCAST_SERVICE_2 & 0x0000FFFF;
            GattDiscoverPrimaryServiceRequest(sinkGetBleTask(), 
                                                                  cfm->cid, 
                                                                  gatt_uuid16,
                                                                  &ba_uuid);
        }
        else
        {
            /* The remote connected device is actually not broadcaster. We are expecting
                broadcaster to connect for association. So, no point in continuing the connection */
            gattClientDisconnect(cfm->cid);
        }
    }
    else
    {
        gattClientStoreDiscoveredService(cfm->cid, cfm->uuid_type, (uint32*)&cfm->uuid[0], cfm->handle, cfm->end, cfm->more_to_come);
    }
}

/******************************************************************************/
void sinkGattUpdateMinimumTxMtu(uint16 requested_mtu_size)
{
    if (requested_mtu_size > mtu_size)
        mtu_size = requested_mtu_size;
}

/******************************************************************************/
uint16 sinkGattGetMinimumTxMtu(void)
{
    return mtu_size;
}

/******************************************************************************/
void sinkGattMsgHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    
    switch(id)
    {
        case GATT_EXCHANGE_MTU_IND:
        {
            handleGattExchangeMtuInd((GATT_EXCHANGE_MTU_IND_T*)message);
        }
        break;
        case GATT_EXCHANGE_MTU_CFM:
        {
            handleGattExchangeMtuCfm((GATT_EXCHANGE_MTU_CFM_T*)message);
        }
        break;
        case GATT_DISCOVER_ALL_PRIMARY_SERVICES_CFM:
        {
            handleGattDiscoverAllPrimaryServicesCfm((GATT_DISCOVER_ALL_PRIMARY_SERVICES_CFM_T*)message);
        }
        break;
        case GATT_DISCOVER_PRIMARY_SERVICE_CFM:
        {
            handleGattDiscoverPrimaryServicesCfm((GATT_DISCOVER_PRIMARY_SERVICE_CFM_T*)message);
        }
        break;
        default:
        {
            GATT_ERROR(("GATT Unhandled msg[%x]\n", id));
        }
        break;
    }
}


#endif /* GATT_ENABLED */
