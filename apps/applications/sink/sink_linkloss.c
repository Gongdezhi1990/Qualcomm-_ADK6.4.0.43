/*
Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief   
    This module manages the link loss recovery mechanisms provided by the 
    HFP and A2DP libraries.
*******************************************************************************/

#include "sink_private_data.h"
#include "sink_main_task.h"
#include "sink_linkloss.h"
#include "sink_a2dp.h"
#include "sink_audio_routing.h"

#ifdef DEBUG_LINKLOSS
#define LINKLOSS_DEBUG(x) DEBUG(x)
#else
#define LINKLOSS_DEBUG(x)
#endif

/*Forward declarations */

static bool getA2dpDeviceToProtect(bdaddr *bd_addr, uint16 *a2dp_index);
static bool getBdAddrNonRoutedA2dpDevice(bdaddr *bd_addr, uint16 *a2dp_index);
static void updateHfpManagement(const bdaddr *bd_addr , bool enable_management);
static void updateA2dpManagement(const bdaddr *bd_addr , bool enable_management);
static void updateNonPeerA2dpManagement(uint16 a2dp_index);
#ifdef ENABLE_PEER
    static void updatePeerA2dpManagement(uint16 a2dp_index);
    static bool isPeerStreamingAsSource(uint16 a2dp_index);
#endif
static bool isHfpConnected(uint16 a2dp_index);


/*******************************************************************************
NAME    
    linklossProtectStreaming
    
DESCRIPTION
    This function enables/disables the stream protection from link loss 
    recovery mechanisms.
    
RETURNS
    void
*******************************************************************************/
void linklossProtectStreaming(linkloss_stream_protection_state_t new_state)
{
    uint16 non_routed_source_a2dp_index;
    bdaddr non_routed_source_bd_addr;

    LINKLOSS_DEBUG(("Linkloss: linklossProtectStreaming protection %u\n", new_state));

    switch (new_state)
    {
    case linkloss_stream_protection_on:
        {
            if(getA2dpDeviceToProtect(&non_routed_source_bd_addr, &non_routed_source_a2dp_index))
            {
                /* Set the state bit of the non-routed link */
                linkloss_stream_protection_state_t stream_protection_state = sinkDataGetStreamProtectionState();
                stream_protection_state &=  ~(1 << non_routed_source_a2dp_index);
                stream_protection_state |= (new_state << non_routed_source_a2dp_index);
                sinkDataSetStreamProtectionState(stream_protection_state);
                LINKLOSS_DEBUG(("Linkloss:  non_routed_idx %u protection_state 0x%x\n", 
                    non_routed_source_a2dp_index, sinkDataGetStreamProtectionState()));

                /* apply the new state to the non-routed link */
                linklossUpdateManagement(&non_routed_source_bd_addr);
            }
        }
        break;

    case linkloss_stream_protection_off:
        {
            uint16 i;
            sinkDataSetStreamProtectionState(0);

            for_all_a2dp(i)
            {
                if(getA2dpStatusFlag(CONNECTED, i))
                {
                    LINKLOSS_DEBUG(("Linkloss:  idx %u protection_state 0x%x\n", 
                        i, sinkDataGetStreamProtectionState()));

                    linklossUpdateManagement(getA2dpLinkBdAddr(i));
                }
            }
        }
        break;

    default:
        break;
    }
}

/*******************************************************************************
NAME    
    getA2dpDeviceToProtect
    
DESCRIPTION
    Determines the bdaddr and a2dp index of the device that needs to be
    protected.
    
RETURNS
    Returns TRUE if a device needs protecting, FALSE otherwise.
*******************************************************************************/
static bool getA2dpDeviceToProtect(bdaddr *bd_addr, uint16 *a2dp_index)
{
#ifdef ENABLE_PEER
    uint16 peer_index = 0xFFFF;
    bdaddr local_bd_address;
#endif

    if (
#ifdef ENABLE_PEER            
            a2dpGetPeerIndex(NULL) && 
#endif            
            getBdAddrNonRoutedA2dpDevice(bd_addr, a2dp_index))
    {
        return TRUE;
    }

#ifdef ENABLE_PEER
    /* No currently active audio source.
       Try to ascertain if:
        A peer is connected
        AND our bdaddr > peer bdaddr
        AND NOT streaming to peer
       Then:
        return bdaddr and index of peer link to be protected.
    */
     sinkDataGetLocalBdAddress(&local_bd_address);

    if (a2dpGetPeerIndex(&peer_index))
    {
        LINKLOSS_DEBUG(("Linkloss: getA2dpDeviceToProtect -  peer_idx %u is_master %u local_bdaddr > peer_bddadr %u state %u\n",
            peer_index, peerIsThisDeviceTwsMaster(),
            peerCompareBdAddr(&local_bd_address, getA2dpLinkBdAddr(peer_index)),
            peerCurrentRelayState()));

        if (peerCompareBdAddr(&local_bd_address, getA2dpLinkBdAddr(peer_index))
            && (peerCurrentRelayState() < RELAY_STATE_STARTING))
        {
            bdaddr *a2dpBdaddr = getA2dpLinkBdAddr(peer_index);
            *bd_addr = *a2dpBdaddr;
            *a2dp_index = peer_index;
            return TRUE;
        }
    }
#endif

    return FALSE;
}

/*******************************************************************************
NAME    
    getBdAddrNonRoutedA2dpDevice
    
DESCRIPTION
    Determines the BD Address of the non-routed audio source other than 
    the one passed in.
    
RETURNS
    Returns TRUE is found, FALSE otherwise.
*******************************************************************************/
static bool getBdAddrNonRoutedA2dpDevice(bdaddr *bd_addr, uint16 *a2dp_index)
{
    uint16 a2dp_index_routed_source;
    uint16 a2dp_index_non_routed_source;
    
    if(bd_addr && a2dp_index
        && getA2dpIndexFromSink(sinkAudioGetRoutedAudioSink(), &a2dp_index_routed_source))
    {
        /* Assuming there can only be 2 A2DP connections with A2DP ids 0 and 1  */
        a2dp_index_non_routed_source = a2dp_index_routed_source ? 0 : 1; 
        
        if(getA2dpStatusFlag(CONNECTED, a2dp_index_non_routed_source))
        {            
            bdaddr *a2dpBdaddr = getA2dpLinkBdAddr(a2dp_index_non_routed_source);
            LINKLOSS_DEBUG(("Linkloss: getBdAddrNonRoutedSource: non-routed index = %u\n", a2dp_index_non_routed_source));
            
            *bd_addr = *a2dpBdaddr;
            *a2dp_index = a2dp_index_non_routed_source;
            return TRUE;
        }
    }
    
    return FALSE;    
}

/*******************************************************************************
NAME    
    linklossUpdateManagement
    
DESCRIPTION
    Enables/disabled the HFP and A2DP library managed link loss recovery 
    mechanism dependant on stream_protection_state.    
    
RETURNS
    void
*******************************************************************************/
void linklossUpdateManagement(const bdaddr *bd_addr)
{
    LINKLOSS_DEBUG(("Linkloss: linklossUpdateManagement protection 0x%x\n", sinkDataGetStreamProtectionState()));

    if(linklossIsStreamProtected(bd_addr))
    {
        /*  Streaming protection is ON, disable the linkloss management provided
            by HFP and A2DP lib for the device */
        updateHfpManagement(bd_addr , FALSE);
        updateA2dpManagement(bd_addr , FALSE);
    }
    else
    {
        /*  Enable HFP and A2DP link loss management   */ 
        updateHfpManagement(bd_addr , TRUE);
        updateA2dpManagement(bd_addr , TRUE); 
    }
}

/*******************************************************************************
NAME    
    updateHfpManagement
    
DESCRIPTION
    Enables/disabled the HFP library managed link loss recovery mechanism .    
    
RETURNS
    void
*******************************************************************************/
static void updateHfpManagement(const bdaddr *bd_addr , bool enable_management)
{
    LINKLOSS_DEBUG(("Linkloss: updateHfpManagement %u hfp index %u\n" , 
        enable_management, HfpLinkPriorityFromBdaddr(bd_addr)));
    HfpManageLinkLoss(HfpLinkPriorityFromBdaddr(bd_addr) , enable_management);
}

/*******************************************************************************
NAME    
    updateA2dpManagement
    
DESCRIPTION
    Enables/disabled the A2DP library managed link loss recovery mechanism.    
    
RETURNS
    void
*******************************************************************************/
static void updateA2dpManagement(const bdaddr *bd_addr , bool enable_management)
{
    uint16 a2dp_index ;
    if (!getA2dpIndexFromBdaddr(bd_addr, &a2dp_index))
    {
        return;
    }
    
    LINKLOSS_DEBUG(("Linkloss: updateA2dpManagement = %u , index = %u\n" , enable_management , a2dp_index));
    
    if(!enable_management)
    {                        
        A2dpDeviceManageLinkloss(getA2dpLinkDataDeviceId(a2dp_index), FALSE);
        return;
    }
    
#ifdef ENABLE_PEER
    if (getA2dpPeerRemoteDevice(a2dp_index) == remote_device_peer)
    {   /* A Peer device will not have HFP connected, thus A2DP library will need
           to be used to provide link loss management */
        updatePeerA2dpManagement(a2dp_index);
    }
    else
#endif
    {   /* Non Peer or unknown device type.  Base A2DP link loss management on 
           any HFP connection */
        updateNonPeerA2dpManagement(a2dp_index);
    }
}


#ifdef ENABLE_PEER

/*******************************************************************************
NAME    
    updatePeerA2dpManagement
    
DESCRIPTION
        This function updates the A2DP link loss management for the peer device.
        A Peer device will not have HFP connected, thus A2DP library will need to
        be used to provide link loss management
    
RETURNS
        void
*******************************************************************************/
static void updatePeerA2dpManagement(uint16 a2dp_index)
{
    /*A Peer device will not have HFP connected, thus A2DP library will need to be used to provide link loss management*/   
    if(isPeerStreamingAsSource(a2dp_index))
    {   /* Relaying audio to a Slave Peer - don't manage linkloss */
        LINKLOSS_DEBUG(("Linkloss: updatePeerA2dpManagement: Index = %u a2dp manage linkloss = FALSE\n", a2dp_index));
        A2dpDeviceManageLinkloss(getA2dpLinkDataDeviceId(a2dp_index), FALSE);
    }
    else
    {   /* Either no audio relay or acting as a Slave Peer */
        LINKLOSS_DEBUG(("Linkloss: updatePeerA2dpManagement: Index = %u a2dp  manage linkloss = TRUE\n", a2dp_index));
        A2dpDeviceManageLinkloss(getA2dpLinkDataDeviceId(a2dp_index), TRUE);
    }
}

/*******************************************************************************
NAME    
    isPeerStreamingAsSource
    
DESCRIPTION
       Determines if the device(a2dp_index) is streaming and is acting as a peer source.      
    
RETURNS
       TRUE if the device is sttreaming as a peer source, FALSE otherwise.
    
*******************************************************************************/
static bool isPeerStreamingAsSource(uint16 a2dp_index)
{
    a2dp_stream_state a2dp_state = A2dpMediaGetState(getA2dpLinkDataDeviceId(a2dp_index),
                                                     getA2dpLinkDataStreamId(a2dp_index));

    LINKLOSS_DEBUG(("LinkLoss: isPeerStreamingAsSource index %u dev_id %u stream_id %u state %u role %u playing %u\n",
        a2dp_index, getA2dpLinkDataDeviceId(a2dp_index),
        getA2dpLinkDataStreamId(a2dp_index),
        a2dp_state, getA2dpLinkRole(a2dp_index), 
        getA2dpStatusFlag(PLAYING, a2dp_index)));
    
    if (((a2dp_state == a2dp_stream_starting) || (a2dp_state == a2dp_stream_streaming))
        && ((getA2dpLinkRole(a2dp_index) == LR_CURRENT_ROLE_MASTER)
            && (getA2dpStatusFlag(PLAYING, a2dp_index))))
    {
        return TRUE;
    }
    
    return FALSE;    
}

#endif  /*ENABLE_PEER*/

/*******************************************************************************
NAME    
    updateNonPeerA2dpManagement
    
DESCRIPTION
    This function updates the A2DP link loss management for the Non Peer or
    unknown device type.
    
RETURNS
    void
*******************************************************************************/
static void updateNonPeerA2dpManagement(uint16 a2dp_index)
{   
    if(isHfpConnected(a2dp_index))
    {   /* HFP library will manage link loss */
        LINKLOSS_DEBUG(("Linkloss: updateNonPeerA2dpManagement: Id = %u (non-peer), a2dp manage linkloss = FALSE\n", a2dp_index));
        A2dpDeviceManageLinkloss(getA2dpLinkDataDeviceId(a2dp_index), FALSE);
    }
    else
    {   /* No HFP connection, so A2DP library manages link loss */
        LINKLOSS_DEBUG(("Linkloss: updateNonPeerA2dpManagement: Id = %u (non-peer) a2dp manage linkloss = TRUE\n", a2dp_index));
        A2dpDeviceManageLinkloss(getA2dpLinkDataDeviceId(a2dp_index), TRUE);
    }
}

/*******************************************************************************
NAME    
    isHfpConnected
    
DESCRIPTION
    This function determines if HFP is also connected to the provided device 
    which is connected to A2DP.
    
RETURNS
    TRUE if HFP is connected to the device, FALSE otherwise.
*******************************************************************************/
static bool isHfpConnected(uint16 a2dp_index)
{
    bdaddr HfpAddr;
    
    if( (HfpLinkGetBdaddr(hfp_primary_link, &HfpAddr) && 
         BdaddrIsSame(getA2dpLinkBdAddr(a2dp_index), &HfpAddr)) ||       
        (HfpLinkGetBdaddr(hfp_secondary_link, &HfpAddr) && 
         BdaddrIsSame(getA2dpLinkBdAddr(a2dp_index), &HfpAddr)) )
    {
        LINKLOSS_DEBUG(("Linkloss:isHfpConnected: Index = %u (non-peer),  connected \n", a2dp_index));
        return TRUE;  
    }
    
    LINKLOSS_DEBUG(("Linkloss:isHfpConnected: Index = %u (non-peer),  not connected \n", a2dp_index));
    return FALSE;
}

/*************************************************************************
NAME    
    linklossSendLinkLossTone

DESCRIPTION
    Send a one-off event to trigger a link loss tone for a given device.

RETURNS

**************************************************************************/
void linklossSendLinkLossTone(bdaddr *bd_addr, uint32 delay)
{
    LINKLOSS_DEBUG(("Linkloss: linklossSendLinkLossTone [%x:%x:%lx]\n", 
        bd_addr ? bd_addr->uap : 0, bd_addr ? bd_addr->nap : 0, bd_addr ? bd_addr->lap : 0));
    sinkDataSetLinkLossBDaddr(bd_addr);
    if (delay)
    {
        MessageSendLater(&theSink.task,  EventSysLinkLoss, 0, delay);
    }
    else
    {
        MessageSend(&theSink.task,  EventSysLinkLoss, 0);
    }
}

/*************************************************************************
NAME    
    linklossCancelLinkLossTone

DESCRIPTION
    Cancel any queued link loss tone events.
    Note this will cancel *all* events and not just for a particular device.

RETURNS

**************************************************************************/
void linklossCancelLinkLossTone(void)
{
    LINKLOSS_DEBUG(("Linkloss: linklossCancelLinkLossTone\n"));
    sinkDataSetLinkLossBDaddr(NULL);
    MessageCancelAll(&theSink.task, EventSysLinkLoss);
}

/*************************************************************************
NAME    
    linklossIsStreamProtected

DESCRIPTION
    Check if the a2dp stream for a device is marked as protected.

RETURNS
    TRUE if the stream is protected, FALSE otherwise

**************************************************************************/
bool linklossIsStreamProtected(const bdaddr *bd_addr)
{
    uint16 a2dp_index = 0;
    uint8 stream_protection_state = sinkDataGetStreamProtectionState();

    if (bd_addr && !getA2dpIndexFromBdaddr(bd_addr, &a2dp_index))
        return FALSE;

    LINKLOSS_DEBUG(("LinkLoss: linklossIsStreamProtected idx %u protection 0x%x return %u\n",
        a2dp_index, stream_protection_state,
        !!(stream_protection_state & (linkloss_stream_protection_on << a2dp_index)) ));

    return (!!(stream_protection_state & (linkloss_stream_protection_on << a2dp_index)));
}

/*************************************************************************
NAME    
    linklossResetStreamProtection

DESCRIPTION
    Clear the stream protection flag for the given a2dp link.
    This is intended to be used only when a new a2dp link connects.
    In any other case linklossProtectStreaming should be called.

**************************************************************************/
void linklossResetStreamProtection(uint16 a2dp_index)
{
    linkloss_stream_protection_state_t stream_protection;
    stream_protection = sinkDataGetStreamProtectionState();
    stream_protection &= ~(1 << a2dp_index);
    sinkDataSetStreamProtectionState(stream_protection);

    LINKLOSS_DEBUG(("Linkloss: linklossResetStreamProtection idx %u protection_state 0x%x\n", 
        a2dp_index, sinkDataGetStreamProtectionState()));
}
