/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

 
FILE NAME
    sink_ba_broadcaster_association.c
 
DESCRIPTION
    Broadcast association API for LE and GATT.
*/

#include "sink_ba_broadcaster_association.h"
#include "sink_debug.h"

#ifdef ENABLE_BROADCAST_AUDIO

#include "sink_gatt_server_ba.h"

#ifdef DEBUG_BA_BROADCASTER
#define DEBUG_BROADCASTER(x) DEBUG(x)
#else
#define DEBUG_BROADCASTER(x)
#endif

/* \brief Application data */
typedef struct _broadcaster_t {
    /* Connection ID of link to GATT peer. */
    uint16 receiver_cid;
    /* BD Address of associated receiver. */
    bdaddr  rx_addr;
}broadcaster_t;

static broadcaster_t broadcaster_assoc_data = {INVALID_CID, {0}};

void sinkBroadcasterHandleReceiverConnectCfm(GATT_MANAGER_REMOTE_SERVER_CONNECT_CFM_T* cfm)
{
    if (cfm->status == gatt_status_success)
    {
        DEBUG_BROADCASTER(("Broadcaster: Gatt Remote server Connect Success->Start Assoc\n"));
        broadcaster_assoc_data.receiver_cid = cfm->cid;
        /* found and connected to a csb_receiver, allow it to read the broadcaster's association data */
        sinkGattBAServerEnableAssociation(TRUE);
    }
    else
    {
        DEBUG_BROADCASTER(("Broadcaster: Gatt Remote server Connect Failed (0x%x)->Retry\n", cfm->status));
    }
}

/******************************************************************************/
void sinkBroadcasterHandleReceiverDisconnectInd(GATT_MANAGER_DISCONNECT_IND_T* ind)
{
    UNUSED(ind);

    DEBUG_BROADCASTER(("Broadcaster: Gatt Manger DisconnectInd\n"));
    broadcaster_assoc_data.receiver_cid = INVALID_CID;
    sinkGattBAServerEnableAssociation(FALSE);
#ifdef ENABLE_RANDOM_ADDR
    if(!BdaddrIsZero((const bdaddr *)&broadcaster_assoc_data.rx_addr))
    {
        /* Remove the receiver from PDL as association is complete */
        ConnectionSmDeleteAuthDeviceReq(TYPED_BDADDR_PUBLIC, (const bdaddr *)&broadcaster_assoc_data.rx_addr);
        memset(&broadcaster_assoc_data.rx_addr, 0x0, sizeof(broadcaster_assoc_data.rx_addr));
    }
#endif
    /* Send system message to stop associating led pattern LedAssociating(FALSE);*/
}

/******************************************************************************/
bool sinkBroadcasterIsReceiverCid(uint16 cid)
{
    bool cid_belongs_to_receiver = FALSE;

    if(broadcaster_assoc_data.receiver_cid != INVALID_CID)
    {
        cid_belongs_to_receiver = (cid == broadcaster_assoc_data.receiver_cid);
    }

    return cid_belongs_to_receiver;
}

/******************************************************************************/
void sinkBroadcasterSetReceiverAddress(const bdaddr* addr)
{
    /* Store the receiver's public BD Address as we will have to remove it from PDL post association */
    broadcaster_assoc_data.rx_addr = *addr;
}

#endif
