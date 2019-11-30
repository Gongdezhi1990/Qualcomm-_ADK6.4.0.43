/*!
\copyright  Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_av.c
\brief      AV State Machines (A2DP & AVRCP)

Main AV task.
*/

/* Only compile if AV defined */
#ifdef INCLUDE_AV

#include <a2dp.h>
#include <avrcp.h>
#include <panic.h>
#include <connection.h>
#include <ps.h>
#include <file.h>
#include <transform.h>
#include <feature.h>
#include <string.h>

#include "av_headset.h"
#include "av_headset_av.h"
#include "av_headset_a2dp.h"
#include "av_headset_avrcp.h"
#include "av_headset_ui.h"
#include "av_headset_link_policy.h"

#include "av_headset_log.h"

/* Remove these once feature_if.h has been updated */
#define APTX_ADAPTIVE (40)
#define APTX_ADAPTIVE_MONO (41)


/*! Macro for creating messages */
#define MAKE_AV_MESSAGE(TYPE) \
    TYPE##_T *message = PanicUnlessNew(TYPE##_T);

/*! Code assertion that can be checked at run time. This will cause a panic. */
#define assert(x)   PanicFalse(x)

static void appAvHandleMessage(Task task, MessageId id, Message message);

/*! \brief Handle AV error

    Some error occurred in the Av state machine, to avoid the state machine
    getting stuck, drop connection and move to 'disconnected' state.
*/
static void appAvError(avTaskData *theAv, MessageId id, Message message)
{
    UNUSED(message); UNUSED(theAv);UNUSED(id);

#ifdef AV_DEBUG
    DEBUG_LOGF("appAvError %p, state=%u, id=%x", (void *)theAv, theAv->state, id);
#else
    Panic();
#endif
}

/*! \brief Check if at least one A2DP or AVRCP link is connected

    \return TRUE if either an A2DP or an AVRCP link is connected. FALSE otherwise.
*/
bool appAvHasAConnection(void)
{
    avTaskData *theAv = appGetAv();
    int instance;

    for (instance = 0; instance < AV_MAX_NUM_INSTANCES; instance++)
    {
        avInstanceTaskData *theInst = theAv->av_inst[instance];
        if (theInst)
            if (appA2dpIsConnected(theInst) || appAvrcpIsConnected(theInst))
                return TRUE;
    }

    /* No AV connections */
    return FALSE;
}

/*! \brief Check all A2DP links are disconnected

    \return TRUE if there are no connected links. FALSE if any AV link 
            has either an A2DP or an AVRCP connection.
*/
bool appAvIsDisconnected(void)
{
    avTaskData *theAv = appGetAv();
    int instance;

    for (instance = 0; instance < AV_MAX_NUM_INSTANCES; instance++)
    {
        avInstanceTaskData *theInst = theAv->av_inst[instance];
        if (theInst)
        {
            if (!appA2dpIsDisconnected(theInst) || !appAvrcpIsDisconnected(theInst))
                return FALSE;
        }
    }

    /* No AV connections */
    return TRUE;
}

/*! \brief Check if A2DP is streaming

    \return TRUE if there is an AV that is streaming
*/
bool appAvIsStreaming(void)
{
    avTaskData *theAv = appGetAv();
    int instance;

    /* Look in table to find entry that is streaming as sink */
    for (instance = 0; instance < AV_MAX_NUM_INSTANCES; instance++)
    {
        avInstanceTaskData *theInst = theAv->av_inst[instance];
        if (theInst && appA2dpIsSinkCodec(theInst) && appA2dpIsStreaming(theInst))
            return TRUE;
    }

    /* No AV connections */
    return FALSE;
}

/*! \brief Check if AV is connected as an A2DP slave

    \return TRUE If any connected AV has A2DP signalling and is
        a TWS Sink.
*/
bool appAvIsConnectedSlave(void)
{
    avTaskData *theAv = appGetAv();
    int instance;

    /* Look in table to find entry with matchin Bluetooth address */
    for (instance = 0; instance < AV_MAX_NUM_INSTANCES; instance++)
    {
        avInstanceTaskData *theInst = theAv->av_inst[instance];
        if (theInst && appA2dpIsStateConnectedSignalling(theInst->a2dp.state))
        {
            if (appA2dpIsSinkTwsCodec(theInst))
                return TRUE;
        }
    }

    /* No AV connections */
    return FALSE;
}

/*! \brief Check if A2DP is streaming as a master

    \return TRUE If any connected AV has A2DP signalling and is
                 a TWS Source.
*/
bool appAvIsConnectedMaster(void)
{
    avTaskData *theAv = appGetAv();
    int instance;

    /* Look in table to find entry with matching Bluetooth address */
    for (instance = 0; instance < AV_MAX_NUM_INSTANCES; instance++)
    {
        avInstanceTaskData *theInst = theAv->av_inst[instance];
        if (theInst && appA2dpIsStateConnectedSignalling(theInst->a2dp.state))
        {
            if (appA2dpIsSourceCodec(theInst))
                return TRUE;
        }
    }

    /* No AV connections */
    return FALSE;
}

/*! \brief Handle incoming AVRCP connection

    AVRCP task has get an incoming AVRCP connection, we receive this message
    so that we can indicate if we'd like it for AV or not.  If A2DP is not connected
    then we say we don't want AVRCP.
*/
static void appAvHandleAvAvrcpConnectIndication(avTaskData *theAv, const AV_AVRCP_CONNECT_IND_T *ind)
{
    avInstanceTaskData *theInst;
    DEBUG_LOG("appAvHandleAvAvrcpConnectIndication");
    UNUSED(theAv);

    theInst = appAvInstanceFindFromBdAddr(&ind->bd_addr);
    if (theInst)
    {
        /* Reject connection if A2DP is disconnected, accept if A2DP connected or connecting */
        if (appA2dpIsDisconnected(theInst))
        {
            DEBUG_LOG("appAvHandleAvAvrcpConnectIndication, rejecting");

            /* Reject incoming connection */
            appAvAvrcpConnectResponse(&theAv->task, &theInst->av_task, &ind->bd_addr, ind->connection_id,
                                      ind->signal_id,  AV_AVRCP_REJECT);
        }
        else
        {
            DEBUG_LOG("appAvHandleAvAvrcpConnectIndication, accepting");

            /* Accept incoming connection */
            appAvAvrcpConnectResponse(&theAv->task, &theInst->av_task, &ind->bd_addr, ind->connection_id,
                                      ind->signal_id,  AV_AVRCP_ACCEPT);
        }
    }
}

/*! \brief Confirmation of AVRCP connection

*/
static void appAvInstanceHandleAvAvrcpConnectCfm(avInstanceTaskData *theInst, AV_AVRCP_CONNECT_CFM_T *cfm)
{
    assert(theInst == cfm->av_instance);
    DEBUG_LOGF("appAvInstanceHandleAvAvrcpConnectCfm(%p), status %d", theInst, cfm->status);
 
    if (cfm->status == avrcp_success)
    {
        /* Get notified of change in playback status */
        uint16 events = 1 << avrcp_event_playback_status_changed;
        
        /* Get notified of change in volume if TWS peer device */
        if (appDeviceIsPeer(&theInst->bd_addr))
            events |= 1 << avrcp_event_volume_changed;
       
        /* Register for notifications to be sent to AV task */
        appAvrcpNotificationsRegister(theInst, events);

        appAvVolumeHandleAvrcpConnect(theInst);

        /* Cancel outstanding connect later request since we are now connected */
        MessageCancelAll(&theInst->av_task, AV_INTERNAL_AVRCP_CONNECT_LATER_REQ);
    }
}

/*! \brief Handle confirmation of AVRCP disconnection */
static void appAvInstanceHandleAvAvrcpDisconnectInd(avInstanceTaskData *theInst, AV_AVRCP_DISCONNECT_IND_T *ind)
{
    DEBUG_LOGF("appAvInstanceHandleAvAvrcpDisconnectInd(%p), status %d", theInst, ind->status);
    appAvVolumeHandleAvDisconnect(theInst);
    /* Cancel outstanding disconnect later request since we are now disconnected */
    MessageCancelAll(&theInst->av_task, AV_INTERNAL_AVRCP_DISCONNECT_LATER_REQ);
}

static void appAvInstanceHandleMessage(Task task, MessageId id, Message message)
{
    avInstanceTaskData *theInst = (avInstanceTaskData *)task;

    if (id >= AV_INTERNAL_AVRCP_BASE && id < AV_INTERNAL_AVRCP_TOP)
        appAvrcpInstanceHandleMessage(theInst, id, message);
    else if (id >= AV_INTERNAL_A2DP_BASE && id < AV_INTERNAL_A2DP_TOP)
        appA2dpInstanceHandleMessage(theInst, id, message);
    else if (id >= AVRCP_MESSAGE_BASE && id < AVRCP_MESSAGE_TOP)
        appAvrcpInstanceHandleMessage(theInst, id, message);
    else if (id >= A2DP_MESSAGE_BASE && id < A2DP_MESSAGE_TOP)
        appA2dpInstanceHandleMessage(theInst, id, message);
    else
    {
        switch (id)
        {
            case AV_AVRCP_CONNECT_CFM:
                appAvInstanceHandleAvAvrcpConnectCfm(theInst, (AV_AVRCP_CONNECT_CFM_T *)message);
                break;
                
            case AV_AVRCP_DISCONNECT_IND:
                appAvInstanceHandleAvAvrcpDisconnectInd(theInst, (AV_AVRCP_DISCONNECT_IND_T *)message);
                break;
    
            case AV_AVRCP_SET_VOLUME_IND:
                appAvInstanceHandleAvAvrcpSetVolumeInd(theInst, (AV_AVRCP_SET_VOLUME_IND_T *)message);
                break;
                
            case AV_AVRCP_VOLUME_CHANGED_IND:
                appAvInstanceHandleAvAvrcpVolumeChangedInd(theInst, (AV_AVRCP_VOLUME_CHANGED_IND_T *)message);
                break;
            
            case AV_AVRCP_PLAY_STATUS_CHANGED_IND:
                appAvInstanceHandleAvAvrcpPlayStatusChangedInd(theInst, (AV_AVRCP_PLAY_STATUS_CHANGED_IND_T *)message);
                break;

            default:
                appAvError(appGetAv(), id, message);
                break;
        }
    }
}

/*! \brief Find AV instance with A2DP state

    This function attempts to find the other AV instance with a matching A2DP state.
    The state is selected using a mask and matching state.

    \param  theInst     AV instance that we want to find a match to
    \param  mask        Mask value to be applied to the a2dp state of a connection
    \param  expected    State expected after applying the mask

    \return Pointer to the AV that matches, NULL if no matching AV was found
*/
avInstanceTaskData *appAvInstanceFindA2dpState(const avInstanceTaskData *theInst,
                                               uint8 mask, uint8 expected)
{
    avTaskData *theAv = appGetAv();
    int instance;

    PanicFalse(appAvIsValidInst((avInstanceTaskData*)theInst));

    /* Look in table to find entry with matching A2DP state */
    for (instance = 0; instance < AV_MAX_NUM_INSTANCES; instance++)
    {
        avInstanceTaskData *theOtherInst = theAv->av_inst[instance];
        if (theOtherInst != NULL && theInst != theOtherInst)
        {
            if ((theOtherInst->a2dp.state & mask) == expected)
            {
                return theOtherInst;
            }
        }
    }

    /* No match found */
    return NULL;
}

/*! \brief Find AV instance for AVRCP passthrough

    This function finds the AV instance to send a AVRCP passthrough command.
    If an AV instance has a Sink SEID as it's last used SEID, then the
    passthrough command should be sent using that instance, otherwise use
    an AV instance with no last used SEID as this will be for an AV source that
    has just paired but hasn't yet connected the A2DP media channel.

    \return Pointer to the AV to use, NULL if no appropriate AV found
*/
avInstanceTaskData *appAvInstanceFindAvrcpForPassthrough(void)
{
    avTaskData *theAv = appGetAv();
    int instance;

    for (instance = 0; instance < AV_MAX_NUM_INSTANCES; instance++)
    {
        avInstanceTaskData *theInst = theAv->av_inst[instance];
        if (theInst && appAvrcpIsConnected(theInst))
        {
            if (appA2dpIsSinkCodec(theInst))
                return theInst;
            else if (appDeviceIsHandset(&theInst->bd_addr))
                return theInst;
        }
    }

    /* No sink A2DP instance, if there's just one AVRCP, send passthrough on that instance */
    avInstanceTaskData *thePassthoughInstance = NULL;
    for (instance = 0; instance < AV_MAX_NUM_INSTANCES; instance++)
    {
        avInstanceTaskData *theInst = theAv->av_inst[instance];
        if (theInst && appAvrcpIsConnected(theInst))
        {
            /* If no previous instance, remember this one for passthrough */
            if (!thePassthoughInstance)
                thePassthoughInstance = theInst;
            else
            {
                /* Second instance found, can't do passthrough, so exit loop */
                thePassthoughInstance = NULL;
                break;
            }
        }
    }

    return thePassthoughInstance;
}


/*! \brief given a BDADDR, find an AV with a connected AVRCP to that device

    \param bd_addr  Address of bluetooth device to find a connection to

    \return    Pointer to AV data for the AV that is to the requested device
               and has an active avrcp connection.
*/
avInstanceTaskData *appAvInstanceFindAvrcpFromBdAddr(const bdaddr *bd_addr)
{
    avTaskData *theAv = appGetAv();
    int instance;

    for (instance = 0; instance < AV_MAX_NUM_INSTANCES; instance++)
    {
        avInstanceTaskData *theInst = theAv->av_inst[instance];
        if (theInst && BdaddrIsSame(&theInst->bd_addr, bd_addr))
            return appAvrcpIsConnected(theInst) ? theInst : NULL;
    }

    return NULL;
}

/*! \brief Find a different connected AV instance

    In TWS cases there may be an AV link to a headset as well as the AV to
    the peer device. Finding the other one, given details of 1 link, can be
    useful for forwarding information.

    \param theInst Instance used in check

    \returns NULL if there is no other instance that is connected, otherwise
             returns an AV instance that is connected and not the same as that 
             supplied.
 */
avInstanceTaskData *appAvInstanceFindAvrcpOther(avInstanceTaskData *theInst)
{
    avTaskData *theAv = appGetAv();
    int instance;

    PanicFalse(appAvIsValidInst(theInst));

    for (instance = 0; instance < AV_MAX_NUM_INSTANCES; instance++)
    {
        avInstanceTaskData *theOtherInst = theAv->av_inst[instance];
        if (theOtherInst && theInst != theOtherInst)
            if (appAvrcpIsConnected(theOtherInst))
                return theOtherInst;
    }

    return NULL;

}

/*! \brief Find AV instance with Bluetooth Address

    \note This function returns the AV. It does not check for an
            active connection, or whether A2DP/AVRCP exists.

    \param  bd_addr Bluetooth address to search our AV connections for

    \return Pointer to AV data that matches the bd_addr requested. NULL if
            none was found.
*/
avInstanceTaskData *appAvInstanceFindFromBdAddr(const bdaddr *bd_addr)
{
    avTaskData *theAv = appGetAv();
    int instance;

    /* Look in table to find entry with matching Bluetooth address */
    for (instance = 0; instance < AV_MAX_NUM_INSTANCES; instance++)
    {
        avInstanceTaskData *theInst = theAv->av_inst[instance];
        if (theInst && BdaddrIsSame(bd_addr, &theInst->bd_addr))
            return theInst;
    }

    /* No match found */
    return NULL;
}

/*! \brief Query if theInst should connect its media channel.
    \param theInst The AV Instance.
    \param[out] seid If the function returns TRUE, this will be set to the
                stream endpoint ID that should be used on the media channel,
                otherwise AV_SEID_INVALID;
    \return TRUE if theInst should connect its media channel, otherwise FALSE.
    \note Checks if there is another connected media (or streaming) A2DP instance
          with a non-TWS sink codec. Since this instance is deciding whether to
          connect its media channel, it means it doesn't know its SEID yet,
          therefore the SEID category of theInst does not matter.
*/
bool appAvInstanceShouldConnectMediaChannel(const avInstanceTaskData *theInst, uint8 *seid)
{
    *seid = AV_SEID_INVALID;
    avInstanceTaskData *theOtherInst = appAvInstanceFindA2dpState(theInst,
                                                A2DP_STATE_MASK_CONNECTED_MEDIA,
                                                A2DP_STATE_CONNECTED_MEDIA);
    if (theOtherInst && appA2dpIsSinkNonTwsCodec(theOtherInst))
    {
        *seid = appA2dpConvertSeidFromSinkToSource(theOtherInst->a2dp.current_seid);
    }
    return (*seid != AV_SEID_INVALID);
}

static void appAvAvrcpConnectLaterRequest(avInstanceTaskData *theInst, uint32 delay)
{
    Task task = &theInst->av_task;
    MessageCancelAll(task, AV_INTERNAL_AVRCP_DISCONNECT_LATER_REQ);
    MessageSendLater(task, AV_INTERNAL_AVRCP_CONNECT_LATER_REQ, NULL, delay);
    DEBUG_LOGF("appAvAvrcpConnectLaterRequest(0x%x) delay=%d", theInst, delay);
}

static void appAvAvrcpDisconnectLaterRequest(avInstanceTaskData *theInst, uint32 delay)
{
    if (appAvIsAvrcpConnected(theInst))
    {
        Task task = &theInst->av_task;
        MessageCancelAll(task, AV_INTERNAL_AVRCP_CONNECT_LATER_REQ);
        MessageSendLater(task, AV_INTERNAL_AVRCP_DISCONNECT_LATER_REQ, NULL, delay);
        DEBUG_LOGF("appAvAvrcpDisconnectLaterRequest(0x%x) delay=%d", theInst, delay);
    }
}

void appAvInstanceA2dpConnected(avInstanceTaskData *theInst)
{    
    DEBUG_LOG("appAvInstanceA2dpConnected");

    /* Set link supervision timeout 5 seconds */
    //ConnectionSetLinkSupervisionTimeout(appAvGetSink(theInst), 0x1F80);
    
    /* Update most recent connected device */
    appDeviceUpdateMruDevice(&theInst->bd_addr);

    /* If A2DP was initiated by us, or AVRCP has already been brought up by someone else */
    if (theInst->a2dp.local_initiated || appAvrcpIsConnected(theInst))
    {
        DEBUG_LOG("appAvInstanceA2dpConnected, locally initiated, connecting AVRCP");
        appAvAvrcpConnectRequest(&theInst->av_task, &theInst->bd_addr);
    }
    else if (appAvrcpIsDisconnected(theInst))
    {
        DEBUG_LOG("appAvInstanceA2dpConnected, remotely initiated");
        appAvAvrcpConnectLaterRequest(theInst, appConfigAvrcpConnectDelayAfterRemoteA2dpConnectMs());
    }

    appAvVolumeHandleA2dpConnect(theInst);

    /* Tell clients we have connected */
    MAKE_AV_MESSAGE(AV_A2DP_CONNECTED_IND);
    message->av_instance = theInst;
    message->bd_addr = theInst->bd_addr;
    message->local_initiated = theInst->a2dp.local_initiated;
    appTaskListMessageSend(theInst->av_status_client_list, AV_A2DP_CONNECTED_IND, message);
}


void appAvInstanceA2dpDisconnected(avInstanceTaskData *theInst)
{
    DEBUG_LOG("appAvInstanceA2dpDisconnected");

    appAvAvrcpDisconnectLaterRequest(theInst, D_SEC(2));

    appAvVolumeHandleAvDisconnect(theInst);

    /* Tell clients we have disconnected */
    MAKE_AV_MESSAGE(AV_A2DP_DISCONNECTED_IND);
    message->av_instance = theInst;
    message->bd_addr = theInst->bd_addr;
    message->reason = theInst->a2dp.disconnect_reason;
    appTaskListMessageSend(theInst->av_status_client_list, AV_A2DP_DISCONNECTED_IND, message);
}

void appAvInstanceAvrcpConnected(avInstanceTaskData *theInst)
{
    DEBUG_LOG("appAvInstanceAvrcpConnected");

    /* Update power table */
    appLinkPolicyUpdatePowerTable(&theInst->bd_addr);

    appAvInstanceStartMediaPlayback(theInst);

    /* Tell clients we have connected */
    MAKE_AV_MESSAGE(AV_AVRCP_CONNECTED_IND);
    message->av_instance = theInst;
    message->bd_addr = theInst->bd_addr;
    message->sink = AvrcpGetSink(theInst->avrcp.avrcp);
    appTaskListMessageSend(theInst->av_status_client_list, AV_AVRCP_CONNECTED_IND, message);
   
}

void appAvInstanceAvrcpDisconnected(avInstanceTaskData *theInst)
{
    DEBUG_LOG("appAvInstanceAvrcpDisconnected");

    /* Tell clients we have disconnected */
    MAKE_AV_MESSAGE(AV_AVRCP_DISCONNECTED_IND);
    message->av_instance = theInst;
    message->bd_addr = theInst->bd_addr;
    appTaskListMessageSend(theInst->av_status_client_list, AV_AVRCP_DISCONNECTED_IND, message);
}


/*  \brief Query if theInst should start media streaming on media open confirm.
    \param theInst The AV Instance.
    \return TRUE if theInst should start media streaming, otherwise FALSE.
    \note This function assumes the caller has already inspected its own state
          and determined it is in a state where streaming is possible.
          This function inspects the other instance's state and SEID compatibility.
          There is one case to handle:
            1. theInst is a source SEID, in which case the other instance must be
               streaming and be using a compatible non-TWS sink SEID.
          In all other cases, it is assumed the peer device will initiate streaming.
*/
bool appAvInstanceShouldStartMediaStreamingOnMediaOpenCfm(const avInstanceTaskData *theInst)
{
    /* Find another streaming instance */
    avInstanceTaskData *theOtherInst = appAvInstanceFindA2dpState(theInst,
                                            A2DP_STATE_MASK_CONNECTED_MEDIA_STREAMING,
                                            A2DP_STATE_CONNECTED_MEDIA_STREAMING);
    if (appA2dpIsSourceCodec(theInst))
    {
        return (theOtherInst && appA2dpSeidsAreCompatible(theInst, theOtherInst));
    }
    return FALSE;
}

/*! \brief Query if theInst should start media streaming on media start indication.
    \param theInst The AV Instance.
    \return TRUE if theInst should start media streaming, otherwise FALSE.
    \note This function assumes the caller has already inspected its own state
          and determined it is in a state where streaming is possible.
          This function inspects the other instance's state and SEID compatibility.
          There are a few cases to handle:
            1. theInst is a TWS sink SEID, in which case the other instance must
               not be streaming.
            2. theInst is a standard sink SEID, in which case the other instance
               must not be streaming.
            3. theInst is a (TWS) source SEID, in which case the other instance
               must be a compatible streaming standard sink SEID.
*/
bool appAvInstanceShouldStartMediaStreamingOnMediaStartInd(const avInstanceTaskData *theInst)
{
    /* Find another streaming instance */
    avInstanceTaskData *theOtherInst = appAvInstanceFindA2dpState(theInst,
                                            A2DP_STATE_MASK_CONNECTED_MEDIA_STREAMING,
                                            A2DP_STATE_CONNECTED_MEDIA_STREAMING);
    if (appA2dpIsSinkTwsCodec(theInst))
    {
        return !theOtherInst;
    }
    else if (appA2dpIsSinkNonTwsCodec(theInst))
    {
        return TRUE;
    }
    else if (appA2dpIsSourceCodec(theInst))
    {
        return theOtherInst && appA2dpSeidsAreCompatible(theInst, theOtherInst);
    }
    return FALSE;
}


/*! \brief Check if AVRCP is connected for AV usage

    \param[in]  theInst The AV Instance to be checked for an AVRCP connection.

    \return     TRUE if the AV task of this instance is registered for AVRCP messages
*/
bool appAvIsAvrcpConnected(avInstanceTaskData *theInst)
{
    return appAvrcpIsValidClient(theInst, &theInst->av_task);
}

/*! \brief Create AV instance for A2DP sink or source

    Creates an AV instance entry for the bluetooth address supplied (bd_addr).

    \note No check is made for there already being an instance 
    for this address.

    \param bd_addr Address the created instance will represent

    \return Pointer to the created instance, or NULL if it was not
        possible to create

*/
avInstanceTaskData *appAvInstanceCreate(const bdaddr *bd_addr)
{
    avTaskData *theAv = appGetAv();
    int instance;

    /* Look in table to find empty entry */
    for (instance = 0; instance < AV_MAX_NUM_INSTANCES; instance++)
    {
        if (theAv->av_inst[instance] == NULL)
        {
            avInstanceTaskData *theInst;

            /* Allocate new instance */
            theInst = PanicUnlessNew(avInstanceTaskData);
            theAv->av_inst[instance] = theInst;

            DEBUG_LOGF("appAvInstanceCreate %p", theInst);

            /* Initialise instance */
            appA2dpInstanceInit(theInst, theAv->suspend_state);
            appAvrcpInstanceInit(theInst);

            /* Set up task handler */
            theInst->av_task.handler = appAvInstanceHandleMessage;

            /* Set Bluetooth address of remote device */
            theInst->bd_addr = *bd_addr;

            /* Copy the AV status client list to instance */
            theInst->av_status_client_list = appTaskListDuplicate(theAv->av_status_client_list);

            /* Tell main application we have created new instance */
            MessageSend(appGetAppTask(), AV_CREATE_IND, NULL);

            /* Return pointer to new instance */
            return theInst;
        }
    }

    /* No free entry in table */
    return NULL;
}

/*! \brief Destroy AV instance for A2DP sink or source

    This function should only be called if the instance no longer has
    either a connected A2DP or a connected AVRCP.  If either is still 
    connected, the function will silently fail.

    The function will panic if theInst is not valid, for instance
    if already destroyed.

    \param  theInst The instance to destroy

*/
void appAvInstanceDestroy(avInstanceTaskData *theInst)
{
    avTaskData *theAv = appGetAv();
    int instance;

    PanicFalse(appAvIsValidInst(theInst));

    /* Destroy instance only both state machines are disconnected */
    if (appA2dpIsDisconnected(theInst) && appAvrcpIsDisconnected(theInst))
    {
        DEBUG_LOGF("appAvInstanceDestroy %p", theInst);

        /* Check there are no A2DP & AVRCP profile library instances */
        PanicFalse(theInst->a2dp.device_id == INVALID_DEVICE_ID);
        PanicNotNull(theInst->avrcp.avrcp);

        /* Clear client lists */
        if (theInst->avrcp.client_list)
        {
            appTaskListDestroy(theInst->avrcp.client_list);
            theInst->avrcp.client_list = NULL;
        }
        appTaskListDestroy(theInst->av_status_client_list);
        theInst->av_status_client_list = NULL;

        /* Flush any messages still pending delivery */
        MessageFlushTask(&theInst->av_task);

        /* Look in table to find matching entry */
        for (instance = 0; instance < AV_MAX_NUM_INSTANCES; instance++)
        {
            if (theAv->av_inst[instance] == theInst)
            {
                /* Clear entry and free instance */
                theAv->av_inst[instance] = NULL;
                free(theInst);

                /* Tell main application we have destroyed instance */
                MessageSend(appGetAppTask(), AV_DESTROY_IND, NULL);
                return;
            }
        }

        /* No match found, fatal error! */
        Panic();
    }
    else
    {
        DEBUG_LOGF("appAvInstanceDestroy %p, A2DP (%d) or AVRCP (%d) not disconnected",
                   theInst, !appA2dpIsDisconnected(theInst), !appAvrcpIsDisconnected(theInst));
    }
}

/*! \brief Return AV instance for A2DP sink

    This function walks through the AV instance table looking for the
    first instance which is a connected sink that can use the 
    specified codec.

    \param codec_type   Codec to look for

    \return Pointer to AV information for a connected source,NULL if none
        was found
*/
avInstanceTaskData *appAvGetA2dpSink(avCodecType codec_type)
{
    avTaskData *theAv = appGetAv();
    int instance;

    /* Look in table to find connected instance with sink SEID */
    for (instance = 0; instance < AV_MAX_NUM_INSTANCES; instance++)
    {
        avInstanceTaskData *theInst = theAv->av_inst[instance];
        if (theInst && appA2dpIsStateConnectedMedia(theInst->a2dp.state))
        {
            switch (codec_type)
            {
                case AV_CODEC_ANY:
                    if (appA2dpIsSinkCodec(theInst))
                        return theInst;
                    break;

                case AV_CODEC_TWS:
                    if (appA2dpIsSinkTwsCodec(theInst))
                        return theInst;
                    break;

                case AV_CODEC_NON_TWS:
                    if (appA2dpIsSinkNonTwsCodec(theInst))
                        return theInst;
                    break;
            }
        }
    }

    /* No sink found so return NULL */
    return NULL;
}

/*! \brief Return AV instance for A2DP source

    This function walks through the AV instance table looking for the
    first instance which is a connected source.

    \return Pointer to AV information for a connected source,NULL if none
            was found
*/
avInstanceTaskData *appAvGetA2dpSource(void)
{
    avTaskData *theAv = appGetAv();
    int instance;

    /* Look in table to find connected instance with source SEID */
    for (instance = 0; instance < AV_MAX_NUM_INSTANCES; instance++)
    {
        avInstanceTaskData *theInst = theAv->av_inst[instance];
        if (theInst && appA2dpIsStateConnectedMedia(theInst->a2dp.state) && appA2dpIsSourceCodec(theInst))
            return theInst;
    }

    /* No sink found so return NULL */
    return NULL;
}

/*! \brief Entering `Initialising A2DP` state

    This function is called when the AV state machine enters
    the 'Initialising A2DP' state, it calls the A2dpInit() function
    to initialise the A2DP profile library and register the SEPs.
*/

static void appAvEnterInitialisingA2dp(avTaskData *theAv)
{
    /* Initialise the Stream Endpoints... */
    sep_data_type seps[] = {
        /* Standard sinks */
        { .sep_config = &av_aptx_snk_sep,
          .in_use = (FeatureVerifyLicense(APTX_CLASSIC_MONO) && appConfigAptxEnabled()) ? 0 : A2DP_SEP_UNAVAILABLE,
        },
        { .sep_config = &av_aac_snk_sep,
          .in_use = appConfigAacEnabled() ? 0 : A2DP_SEP_UNAVAILABLE,
        },
        { .sep_config = &av_sbc_snk_sep,
          .in_use = 0,
        },
        /* TWS sinks */
        { .sep_config = &av_aptx_mono_tws_snk_sep,
          .in_use = FeatureVerifyLicense(APTX_CLASSIC_MONO) ? 0 : A2DP_SEP_UNAVAILABLE,
        },
        { .sep_config = &av_aac_stereo_tws_snk_sep,
          .in_use = 0,
        },
        { .sep_config = &av_sbc_mono_tws_snk_sep,
          .in_use = 0,
        },
        { .sep_config = &av_aptx_adaptive_tws_snk_sep,
          .in_use = FeatureVerifyLicense(APTX_ADAPTIVE_MONO) && appConfigAptxAdaptiveEnabled() ? 0 : A2DP_SEP_UNAVAILABLE,
        },
        /* TWS sources */
        { .sep_config = &av_aptx_mono_tws_src_sep,
          .in_use = FeatureVerifyLicense(APTX_CLASSIC_MONO) ? 0 : A2DP_SEP_UNAVAILABLE,
        },
        { .sep_config = &av_aac_stereo_tws_src_sep,
          .in_use = 0,
        },
        { .sep_config = &av_sbc_mono_tws_src_sep,
          .in_use = 0,
        },
        /* PTS sources */
        { .sep_config = &av_sbc_src_sep,
          .in_use = 0,
        },
    };
    DEBUG_LOG("appAvEnterInitialisingA2dp");

    /* Initialise the A2DP Library */
    A2dpInit(&theAv->task, A2DP_INIT_ROLE_SINK | A2DP_INIT_ROLE_SOURCE, 0,
             ARRAY_DIM(seps), seps, 0);
}

/*! \brief Entering `Initialising AVRCP` state

    This function is called when the AV state machine enters
    the 'Initialising AVRCP' state, it calls the AvrcpInit() function
    to initialise the AVRCP profile library.
*/
static void appAvEnterInitialisingAvrcp(avTaskData *theAv)
{
    const avrcp_init_params avrcpConfig =
    {
        avrcp_target_and_controller,
        AVRCP_CATEGORY_1,
        AVRCP_CATEGORY_2 | AVRCP_CATEGORY_1,
        AVRCP_VERSION_1_6
    };

    DEBUG_LOG("appAvEnterInitialisingAvrcp");

    /* Go ahead and initialise the AVRCP library */
    AvrcpInit(&theAv->task, &avrcpConfig);
}

/*! \brief Set AV FSM state

    Called to change state.  Handles calling the state entry and exit
    functions for the new and old states.
*/
static void appAvSetState(avTaskData *theAv, avState state)
{
    DEBUG_LOGF("appAvSetState(%d)", state);

    /* Set new state */
    theAv->state = state;

    /* Handle state entry functions */
    switch (state)
    {
        case AV_STATE_INITIALISING_A2DP:
            appAvEnterInitialisingA2dp(theAv);
            break;

        case AV_STATE_INITIALISING_AVRCP:
            appAvEnterInitialisingAvrcp(theAv);
            break;

        default:
            break;
    }

    /* Set new state */
    theAv->state = state;
}

/*! \brief Set AV FSM state

    Returns current state of the AV FSM.
*/
static avState appAvGetState(avTaskData *theAv)
{
    return theAv->state;
}

/*! \brief Handle A2DP Library initialisation confirmation

    Check status of A2DP Library initialisation if successful store the SEP
    list for later use and move to the 'Initialising AVRCP' state to kick
    of AVRCP Library initialisation.
*/
static void appAvHandleA2dpInitConfirm(avTaskData *theAv, const A2DP_INIT_CFM_T *cfm)
{
    DEBUG_LOG("appAvHandleA2dpInitConfirm");

    /* Check if A2DP initialised successfully */
    if (cfm->status == a2dp_success)
    {
        /* Move to 'Initialising AVRCP' state */
        appAvSetState(theAv, AV_STATE_INITIALISING_AVRCP);
    }
    else
        Panic();
}

/*! \brief Handle AVRCP Library initialisation confirmation

    Check status of AVRCP Library initialisation if successful inform the main
    task that AV initialisation has completed and move into the 'Active' state.
*/
static void appAvHandleAvrcpInitConfirm(avTaskData *theAv, const AVRCP_INIT_CFM_T *cfm)
{
    DEBUG_LOG("appAvHandleAvrcpInitConfirm");

    /* Check if AVRCP successfully initialised */
    if (cfm->status == avrcp_success)
    {
        /* Tell main application task we have initialised */
        MessageSend(appGetAppTask(), AV_INIT_CFM, NULL);

        /* Change to 'idle' state */
        appAvSetState(theAv, AV_STATE_IDLE);
    }
    else
        Panic();
}



/*! \brief Handle indication of change in a connection status.

    Some phones will disconnect the ACL without closing any L2CAP/RFCOMM
    connections, so we check the ACL close reason code to determine if this
    has happened.

    If the close reason code was not link-loss and we have an AV profiles
    on that link, mark it as detach pending, so that we can gracefully handle
    the L2CAP or RFCOMM disconnection that will follow shortly.
 */
static void appAvHandleConManagerConnectionInd(CON_MANAGER_CONNECTION_IND_T *ind)
{
    avInstanceTaskData *theInst = appAvInstanceFindFromBdAddr(&ind->bd_addr);
    if (theInst)
    {
        if (!ind->connected)
        {
            if (ind->reason != hci_error_conn_timeout)
            {
                DEBUG_LOG("appAvHandleConManagerConnectionInd, detach pending");
                theInst->detach_pending = TRUE;
            }
        }
    }
}

/*! \brief Initialise AV task

    Called at start up to initialise the main AV task..
    Clear the AV instance table and initialises the state-machine.
*/
void appAvInit(void)
{
    avTaskData *theAv = appGetAv();
    int instance;

    /* Set up task handler */
    theAv->task.handler = appAvHandleMessage;

    /* Default to unset volume. The volume will be set when A2DP or AVRCP connects. */
    theAv->volume = VOLUME_UNSET;

    /* Clear instance table */
    for (instance = 0; instance < AV_MAX_NUM_INSTANCES; instance++)
    {
        theAv->av_inst[instance] = NULL;
    }

    /* Initialise state */
    theAv->suspend_state = 0;
    theAv->state = AV_STATE_NULL;
    appAvSetState(theAv, AV_STATE_INITIALISING_A2DP);

    /* Initialise client list */
    theAv->avrcp_client_list = appTaskListInit();
    appAvAvrcpClientRegister(&theAv->task, 0);

    /* Initialse client list for sending AV status indications */
    theAv->av_status_client_list = appTaskListInit();

    /* Register to receive notifications of (dis)connections */
    appConManagerRegisterConnectionsClient(&theAv->task);
}


/*! \brief Register a task to receive avrcp messages from the av module

    \note This function can be called multiple times for the same task. It 
      will only appear once on a list.

    \param  client_task Task to be added to the list of registered clients
    \param  interests   Not used at present
 */
void appAvAvrcpClientRegister(Task client_task, uint8 interests)
{
    avTaskData *theAv = appGetAv();
    UNUSED(interests);

    /* Add client task to list */
    appTaskListAddTask(theAv->avrcp_client_list, client_task);
}


/*! \brief Register a task to receive AV status messages

    \note This function can be called multiple times for the same task. It 
      will only appear once on a list.

    \param  client_task Task to be added to the list of registered clients
 */
void appAvStatusClientRegister(Task client_task)
{
    avTaskData *theAv = appGetAv();

    /* Add client task to list */
    appTaskListAddTask(theAv->av_status_client_list, client_task);
}


/*! \brief Connect A2DP to a specific Bluetooth device 

    This function requests an A2DP connection. If there is no AV entry 
    for the device, it will be created. If the AV already exists any
    pending link destructions for AVRCP and A2DP will be cancelled.

    If there is an existing A2DP connection, then the function will 
    return FALSE.

    \param  bd_addr     Bluetooth address of device to connect to
    \param  a2dp_flags  Flags to apply to connection. Can be combined as a bitmask.

    \return TRUE if the connection has been requested. FALSE otherwise, including
        when a connection already exists.
*/
bool appAvA2dpConnectRequest(const bdaddr *bd_addr, appAvA2dpConnectFlags a2dp_flags)
{
    avInstanceTaskData *theInst;

    /* Check if AV instance to this device already exists */
    theInst = appAvInstanceFindFromBdAddr(bd_addr);
    if (theInst == NULL)
    {
        /* No AV instance for this device, so create new instance */
        theInst = appAvInstanceCreate(bd_addr);
    }
    else
    {
        /* Make sure there's no pending destroy message */
        MessageCancelAll(&theInst->av_task, AV_INTERNAL_A2DP_DESTROY_REQ);
        MessageCancelAll(&theInst->av_task, AV_INTERNAL_AVRCP_DESTROY_REQ);
    }

    /* Now check we have an AV instance */
    if (theInst)
    {
        /* Check A2DP is not already connected */
        if (!appA2dpIsConnected(theInst))
        {
            /* Send AV_INTERNAL_A2DP_CONNECT_REQ to start A2DP connection */
            MAKE_AV_MESSAGE(AV_INTERNAL_A2DP_CONNECT_REQ);

            DEBUG_LOGF("appAvConnectWithBdAddr A2DP, %p, %x %x %lx",
                         (void *)theInst, bd_addr->nap, bd_addr->uap, bd_addr->lap);

            /* Send message to newly created AV instance to connect A2DP */
            message->num_retries = 2;
            message->flags = (unsigned)a2dp_flags;
            MessageCancelFirst(&theInst->av_task, AV_INTERNAL_A2DP_CONNECT_REQ);
            MessageSendConditionally(&theInst->av_task, AV_INTERNAL_A2DP_CONNECT_REQ, message,
                                     appConManagerCreateAcl(&theInst->bd_addr));

            return TRUE;
        }
    }

    return FALSE;
}

/*! \brief Connect A2DP Media to a specific Bluetooth device

    This function requests an A2DP Media connection.

    If there is not an existing A2DP connection, then the function will
    return FALSE.

    \param  bd_addr     Bluetooth address of device to connect to

    \return TRUE if the meida connection has been requested. FALSE otherwise,
    including when a connection doesn't exists.
*/
bool appAvA2dpMediaConnectRequest(const bdaddr *bd_addr)
{
    avInstanceTaskData *theInst;

    /* Check if AV instance to this device already exists */
    theInst = appAvInstanceFindFromBdAddr(bd_addr);
    if (theInst != NULL)
    {
        /* Check A2DP is already connected */
        if (appA2dpIsConnected(theInst))
        {
            /* Connect media channel */
            MAKE_AV_MESSAGE(AV_INTERNAL_A2DP_CONNECT_MEDIA_REQ);
            message->seid = AV_SEID_INVALID;
            MessageCancelFirst(&theInst->av_task, AV_INTERNAL_A2DP_CONNECT_MEDIA_REQ);
            MessageSendConditionally(&theInst->av_task, AV_INTERNAL_A2DP_CONNECT_MEDIA_REQ, message,&appA2dpGetLock(theInst));
            return TRUE;
        }
    }

    return FALSE;
}

/*! \brief Connect AVRCP to a specific Bluetooth device

    This function requests an AVRCP connection.
    If there is no AV entry for the device, it will be created. No check is
    made for an existing AVRCP connection, but if the AV already exists any
    pending link destructions for AVRCP and A2DP will be cancelled.

    If the function returns TRUE, then the client_task should receive an
    AV_AVRCP_CONNECT_CFM message whether the connection succeeds or not. See note.

    \note If there was no existing AV entry for the device, and hence no ACL,
    then the AV_AVRCP_CONNECT_CFM message will not be sent if the ACL could not
    be created.

    \param  client_task Task to receive response messages
    \param  bd_addr     Bluetooth address of device to connect to

    \return TRUE if the connection has been requested, FALSE otherwise
*/
bool appAvAvrcpConnectRequest(Task client_task, const bdaddr *bd_addr)
{
    avInstanceTaskData *theInst;

    /* Check if AV instance to this device already exists */
    theInst = appAvInstanceFindFromBdAddr(bd_addr);
    if (theInst == NULL)
    {
        /* No AV instance for this device, so create new instance */
        theInst = appAvInstanceCreate(bd_addr);
    }
    else
    {
        /* Make sure there's no pending disconnect/destroy message */
        MessageCancelAll(&theInst->av_task, AV_INTERNAL_A2DP_DESTROY_REQ);
        MessageCancelAll(&theInst->av_task, AV_INTERNAL_AVRCP_DESTROY_REQ);
        if (client_task == &theInst->av_task)
        {
            MessageCancelAll(&theInst->av_task, AV_INTERNAL_AVRCP_DISCONNECT_LATER_REQ);
        }
    }

    /* Now check we have an AV instance */
    if (theInst)
    {
        /* Send AV_INTERNAL_AVRCP_CONNECT_REQ to start AVRCP connection */
        MAKE_AV_MESSAGE(AV_INTERNAL_AVRCP_CONNECT_REQ);

        DEBUG_LOGF("appAvConnectWithBdAddr AVRCP, %p, %x %x %lx",
                     (void *)theInst, bd_addr->nap, bd_addr->uap, bd_addr->lap);

        /* Send message to newly created AV instance to connect AVRCP */
        message->client_task = client_task;
        MessageCancelFirst(&theInst->av_task, AV_INTERNAL_AVRCP_CONNECT_REQ);
        MessageSendConditionally(&theInst->av_task, AV_INTERNAL_AVRCP_CONNECT_REQ, message,
                                 appConManagerCreateAcl(&theInst->bd_addr));

        return TRUE;
    }

    return FALSE;
}


/*! \brief Application response to a connection request

    After a connection request has been received and processed by the
    application, this function should be called to accept or reject the
    request.

    \param[in] ind-task       Task that received ind, that will no longer receive subsequent messages
    \param[in] client_task    Task responding, that will receive subsequent messages
    \param[in] bd_addr        Bluetooth address of connected device 
    \param     connection_id  Connection ID
    \param     signal_id      Signal ID
    \param     accept         Whether to accept or reject the connection
 */
void appAvAvrcpConnectResponse(Task ind_task, Task client_task, const bdaddr *bd_addr,
                               uint16 connection_id, uint16 signal_id, avAvrcpAccept accept)
{
    avInstanceTaskData *av_inst;

    /* Get AV instance for this device */
    av_inst = appAvInstanceFindFromBdAddr(bd_addr);
    if (av_inst)
    {
        MAKE_AV_MESSAGE(AV_INTERNAL_AVRCP_CONNECT_RES);
        message->ind_task = ind_task;
        message->client_task = client_task;
        message->accept = accept;
        message->connection_id = connection_id;
        message->signal_id = signal_id;
        MessageSend(&av_inst->av_task, AV_INTERNAL_AVRCP_CONNECT_RES, message);
    }
    else
    {
        Panic();
    }
}


/*! \brief Request disconnection of A2DP from specified AV

    \param[in] av_inst  Instance to disconnect A2DPfrom

    \return TRUE if disconnect has been requested
 */
bool appAvA2dpDisconnectRequest(avInstanceTaskData *av_inst)
{
    if (av_inst)
    {
        MAKE_AV_MESSAGE(AV_INTERNAL_A2DP_DISCONNECT_REQ);
        message->flags = 0;
        PanicFalse(appAvIsValidInst(av_inst));
        MessageSendConditionally(&av_inst->av_task, AV_INTERNAL_A2DP_DISCONNECT_REQ,
                                 message, &appA2dpGetLock(av_inst));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*! \brief Request disconnection of AVRCP notifying the specified client. 

    \param[in] client_task  Task to receive disconnect confirmation
    \param[in] av_inst      Instance to disconnect AVRCP from

    \return TRUE if disconnect has been requested
 */
bool appAvAvrcpDisconnectRequest(Task client_task, avInstanceTaskData *av_inst)
{
    if (av_inst)
    {
        MAKE_AV_MESSAGE(AV_INTERNAL_AVRCP_DISCONNECT_REQ);
        message->client_task = client_task;
        PanicFalse(appAvIsValidInst(av_inst));
        MessageSendConditionally(&av_inst->av_task, AV_INTERNAL_AVRCP_DISCONNECT_REQ,
                                 message, &appAvrcpGetLock(av_inst));
        DEBUG_LOGF("appAvAvrcpDisconnectRequest(0x%x)", client_task);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/*! \brief Connect to peer device

    \return TRUE if a connection was requested, FALSE in all other cases
      including when there is already a connection 
 */
bool appAvConnectPeer(void)
{
    bdaddr bd_addr;

    /* Get peer device address */
    if (appDeviceGetPeerBdAddr(&bd_addr))
    {
        return appAvA2dpConnectRequest(&bd_addr, A2DP_CONNECT_NOFLAGS);
    }
    else
    {
        return FALSE;
    }
}

/*! \brief Disconnect AVRCP and A2DP from the peer earbud. 

    \return TRUE if a disconnection was requested and is pending, FALSE
        otherwise.
*/
bool appAvDisconnectPeer(void)
{
    bdaddr bd_addr;

    /* Get peer device address */
    if (appDeviceGetPeerBdAddr(&bd_addr))
    {
        avInstanceTaskData *av_inst = appAvInstanceFindFromBdAddr(&bd_addr);
        return appAvA2dpDisconnectRequest(av_inst) || appAvAvrcpDisconnectRequest(&av_inst->av_task, av_inst);
    }
    else
    {
        return FALSE;
    }
}


/*! \brief Connect to a handset 

    This connects to the first handset from device manager if that handset
    supports A2DP (which is likely).

    \param play AV will attempt to start media playback after AV connects

    \return TRUE if a connection was requested, FALSE in all other cases
      including when there is already a connection 
 */
bool appAvConnectHandset(bool play)
{
    bdaddr bd_addr;

    /* Get handset device address */
    if (appDeviceGetHandsetBdAddr(&bd_addr) && appDeviceIsA2dpSupported(&bd_addr))
    {
        appAvA2dpConnectFlags flags = A2DP_CONNECT_MEDIA;
        if (play)
        {
            flags |= A2DP_START_MEDIA_PLAYBACK;
        }
        return appAvA2dpConnectRequest(&bd_addr, flags);
    }

    return FALSE;
}

/*! \brief Connect the A2dp Media channel on a connected handset

    This connects the A2dp Media channel on an already connectd handest.

    \return TRUE if a connection was requested, FALSE in all other cases
      including when there is not a connection
 */
bool appAvConnectHandsetA2dpMedia(void)
{
    bdaddr bd_addr;

    /* Get handset device address */
    if (appDeviceGetHandsetBdAddr(&bd_addr) && appDeviceIsA2dpSupported(&bd_addr))
    {
        return appAvA2dpMediaConnectRequest(&bd_addr);
    }

    return FALSE;
}

/*! \brief Disconnect AVRCP and A2DP from the handset. 

    Disconnect any AVRCP or A2DP connections from the handset.

    \return TRUE if a disconnection was requested and is pending, FALSE
        otherwise.
 */
bool appAvDisconnectHandset(void)
{
    bdaddr bd_addr;

    if (appDeviceGetHandsetBdAddr(&bd_addr))
    {
        avInstanceTaskData *av_inst = appAvInstanceFindFromBdAddr(&bd_addr);
        return appAvA2dpDisconnectRequest(av_inst) || appAvAvrcpDisconnectRequest(&av_inst->av_task, av_inst);
    }
    else
    {
        return FALSE;
    }
}

/*! \brief Initiate an AV connection to a Bluetooth address

    AV connections are started with an A2DP connection.

    \param  bd_addr Address to connect to

    \return TRUE if a connection has been initiated. FALSE in all other cases
            included if there is an existing connection.
 */
bool appAvConnectWithBdAddr(const bdaddr *bd_addr)
{
    /* See if known */
    if (appDeviceFindBdAddrAttributes(bd_addr, NULL))
    {
        return appAvA2dpConnectRequest(bd_addr, A2DP_CONNECT_MEDIA);
    }
    else
    {
        return FALSE;
    }
}

/*! \brief Suspend AV link

    This function is called whenever a module in the headset has a reason to
    suspend AV streaming.  An internal message is sent to every AV
    instance, if the instance is currently streaming it will attempt to
    suspend.

    \note There may be multple reasons that streaming is suspended at any time.

    \param  reason  Why streaming should be suspended. What activity has started.
*/

void appAvStreamingSuspend(avSuspendReason reason)
{
    int instance;
    avTaskData *theAv = appGetAv();
    unsigned suspend_state_pre = theAv->suspend_state;
    DEBUG_LOGF("appAvStreamingSuspend(0x%x, 0x%x)", suspend_state_pre, reason);

    /* Update suspend state for any newly created AV instances */
    theAv->suspend_state |= reason;

    /* Only send suspend messages if the suspend state has changed */
    if (theAv->suspend_state != suspend_state_pre)
    {
        /* Send suspend to all instances in the table */
        for (instance = 0; instance < AV_MAX_NUM_INSTANCES; instance++)
        {
            avInstanceTaskData *av_inst = theAv->av_inst[instance];
            if (av_inst != NULL)
            {
                MAKE_AV_MESSAGE(AV_INTERNAL_A2DP_SUSPEND_MEDIA_REQ);

                /* Send message to AV instance */
                message->reason = reason;
                MessageSendConditionally(&av_inst->av_task, AV_INTERNAL_A2DP_SUSPEND_MEDIA_REQ,
                                         message, &appA2dpGetLock(av_inst));
            }
        }
    }
}

/*! \brief Resume AV link

    This function is called whenever a module in the headset has cleared it's
    reason to suspend AV streaming.  An internal message is sent to every AV
    instance.

    \note There may be multple reasons why streaming is currently suspended.
      All suspend reasons have to be cleared before the AV instance will
      attempt to resume streaming.

    \param  reason  Why streaming can now be resumed. What activity has completed.
*/
void appAvStreamingResume(avSuspendReason reason)
{
    int instance;
    avTaskData *theAv = appGetAv();
    unsigned suspend_state_pre = theAv->suspend_state;
    DEBUG_LOGF("appAvStreamingResume(0x%x, 0x%x)", suspend_state_pre, reason);

    /* Update suspend state for any newly created AV instances */
    theAv->suspend_state &= ~reason;

    /* Only send resume messages if the suspend state has changed */
    if (theAv->suspend_state != suspend_state_pre)
    {
        /* Send suspend to all instances in the table */
        for (instance = 0; instance < AV_MAX_NUM_INSTANCES; instance++)
        {
            avInstanceTaskData *av_inst = theAv->av_inst[instance];
            if (av_inst != NULL)
            {
                MAKE_AV_MESSAGE(AV_INTERNAL_A2DP_RESUME_MEDIA_REQ);

                /* Send message to AV instance */
                message->reason = reason;
                MessageSendConditionally(&av_inst->av_task, AV_INTERNAL_A2DP_RESUME_MEDIA_REQ,
                                        message, &appA2dpGetLock(av_inst));
            }
        }
    }
}

/*! \brief Check if an instance is valid 

    Checks if the instance passed is still a valid AV. This allows 
    you to check whether theInst is still valid.

    \param  theInst Instance to check

    \returns TRUE if the instance is valid, FALSE otherwise
 */
bool appAvIsValidInst(avInstanceTaskData* theInst)
{
    avTaskData *theAv = appGetAv();
    int instance;

    if (!theInst)
        return FALSE;

    for (instance = 0; instance < AV_MAX_NUM_INSTANCES; instance++)
    {
        if (theInst == theAv->av_inst[instance])
            return TRUE;
    }
    return FALSE;
}

/*! \brief Schedules media playback if in correct AV state and flag is set.
    \param  theInst The AV instance.
    \return TRUE if media play is scheduled, otherwise FALSE.
 */
bool appAvInstanceStartMediaPlayback(avInstanceTaskData *theInst)
{
    if (appA2dpIsConnectedMedia(theInst) && appAvIsAvrcpConnected(theInst))
    {
        if (theInst->a2dp.flags & A2DP_START_MEDIA_PLAYBACK)
        {
            DEBUG_LOGF("appAvInstanceStartMediaPlayback(%p)", theInst);
            theInst->a2dp.flags &= ~A2DP_START_MEDIA_PLAYBACK;
            MessageSendLater(&theInst->av_task, AV_INTERNAL_AVRCP_PLAY_REQ, NULL,
                             appConfigHandoverMediaPlayDelay());
            return TRUE;
        }
    }
    return FALSE;
}

/*! \brief Message Handler

    This function is the main message handler for the AV module, every
    message is handled in it's own seperate handler function.  The switch
    statement is broken into seperate blocks to reduce code size, if execution
    reaches the end of the function then it is assumed that the message is
    unhandled.
*/
void appAvHandleMessage(Task task, MessageId id, Message message)
{
    avTaskData *theAv = (avTaskData *)task;
    avState state = appAvGetState(theAv);

    /* Handle connection manager messages */
    switch (id)
    {
        case CON_MANAGER_CONNECTION_IND:
            appAvHandleConManagerConnectionInd((CON_MANAGER_CONNECTION_IND_T *)message);
            return;
    }

    /* Handle A2DP messages */
    switch (id)
    {
        case A2DP_INIT_CFM:
        {
            switch (state)
            {
                case AV_STATE_INITIALISING_A2DP:
                    appAvHandleA2dpInitConfirm(theAv, (A2DP_INIT_CFM_T *)message);
                    return;
                default:
                    appHandleUnexpected(id);
                    return;
            }
        }

        case AVRCP_INIT_CFM:
        {
            switch (state)
            {
                case AV_STATE_INITIALISING_AVRCP:
                    appAvHandleAvrcpInitConfirm(theAv, (AVRCP_INIT_CFM_T *)message);
                    return;
                default:
                    appHandleUnexpected(id);
                    return;
            }
        }

        case A2DP_SIGNALLING_CONNECT_IND:
        {
            switch (state)
            {
                case AV_STATE_IDLE:
                    appA2dpSignallingConnectIndicationNew(theAv, (A2DP_SIGNALLING_CONNECT_IND_T *)message);
                    return;
                default:
                    appA2dpRejectA2dpSignallingConnectIndicationNew(theAv, (A2DP_SIGNALLING_CONNECT_IND_T *)message);
                    return;
            }
        }

        case AVRCP_CONNECT_IND:
        {
            switch (state)
            {
                case AV_STATE_IDLE:
                    appAvrcpHandleAvrcpConnectIndicationNew(theAv, (AVRCP_CONNECT_IND_T *)message);
                    return;
                default:
                    appAvrcpRejectAvrcpConnectIndicationNew(theAv, (AVRCP_CONNECT_IND_T *)message);
                    return;
            }
        }

        case AV_INTERNAL_VOLUME_UP_REPEAT:
        case AV_INTERNAL_VOLUME_DOWN_REPEAT:
            appAvVolumeRepeat(((AV_INTERNAL_VOLUME_REPEAT_T *)message)->step);
            return;

        case AV_INTERNAL_VOLUME_STORE_REQ:
            appAvVolumeAttributeStore(theAv);
            return;

        case AV_AVRCP_CONNECT_IND:
            appAvHandleAvAvrcpConnectIndication(theAv, (AV_AVRCP_CONNECT_IND_T *)message);
            return;

        default:
            appAvError(theAv, id, message);
            return;
    }
}

#endif
