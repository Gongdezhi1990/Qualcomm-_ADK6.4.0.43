/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

 
FILE NAME
    sink_ba_broadcaster_association.h
 
DESCRIPTION
    Broadcast association API for LE and GATT.
*/

#ifndef SINK_BA_BROADCASTER_ASSOCIATION_H_
#define SINK_BA_BROADCASTER_ASSOCIATION_H_

#include <csrtypes.h>
#include <gatt_manager.h>

/***************************************************************************
NAME
    sinkBroadcasterHandleReceiverConnectCfm

DESCRIPTION
    Interface to allow broadcaster association for the connected receiver

PARAMS
    cfm Gatt Managers connection response of receiver
RETURNS
    void
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkBroadcasterHandleReceiverConnectCfm(GATT_MANAGER_REMOTE_SERVER_CONNECT_CFM_T* cfm);
#else
#define sinkBroadcasterHandleReceiverConnectCfm(cfm) ((void)(0))
#endif

/***************************************************************************
NAME
    sinkBroadcasterHandleReceiverDisconnectInd

DESCRIPTION
    Interface to indicate receiver disconnected the LE link

PARAMS
    ind Gatt Managers disconnect indication of receiver
RETURNS
    void
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkBroadcasterHandleReceiverDisconnectInd(GATT_MANAGER_DISCONNECT_IND_T* ind);
#else
#define sinkBroadcasterHandleReceiverDisconnectInd(ind) ((void)(0))
#endif

/***************************************************************************
NAME
    sinkBroadcasterIsReceiverCid

DESCRIPTION
    Utility function to check if the provided CID is that of receiver link

PARAMS
    cid GATT connection ID of the receiver
RETURNS
    TRUE if receiver's CID else FALSE
*/
#ifdef ENABLE_BROADCAST_AUDIO
bool sinkBroadcasterIsReceiverCid(uint16 cid);
#else
#define sinkBroadcasterIsReceiverCid(cid) FALSE
#endif

/***************************************************************************
NAME
    sinkBroadcasterSetReceiverAddress

DESCRIPTION
    Stores the receiver's public BD address in the global data

    Note: In case receiver BD address is RANDOM, we would have bonded with.
            So post association, it is required to remove it from PDL so that PS store won't overflow
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkBroadcasterSetReceiverAddress(const bdaddr* addr);
#else
#define sinkBroadcasterSetReceiverAddress(addr) ((void)(0))
#endif

#endif /* SINK_BA_BROADCASTER_ASSOCIATION_H_ */
