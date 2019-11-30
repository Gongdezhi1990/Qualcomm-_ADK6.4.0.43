/*!
\copyright  Copyright (c) 2008 - 2018 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_av_volume.c
\brief      AV volume handling (A2DP & AVRCP)
*/

/* Only compile if AV defined */
#ifdef INCLUDE_AV

#include <panic.h>
#include <string.h>

#include "av_headset.h"
#include "av_headset_av.h"
#include "av_headset_log.h"

/*! Macro for creating messages */
#define MAKE_AV_MESSAGE(TYPE) \
    TYPE##_T *message = PanicUnlessNew(TYPE##_T);

/*! Code assertion that can be checked at run time. This will cause a panic. */
#define assert(x)   PanicFalse(x)

static void appAvSetLocalVolume(uint8 volume)
{
    if (appAvGetA2dpSink(AV_CODEC_ANY))
    {
        appKymeraA2dpSetVolume(volume);
    }
}

/*! \brief Set the local volume and queue a volume store.
    \return FALSE if the volume was unset (this value should never be applied)
    otherwise TRUE.
*/
static bool appAvApplyVolume(uint8 volume)
{
    avTaskData *theAv = appGetAv();

    if (appAvVolumeIsSet(volume))
    {
        if (volume != theAv->volume)
        {
            /* Set local volume */
            appAvSetLocalVolume(volume);
            theAv->volume = volume;

            /* Cancel any pending messages */
            MessageCancelFirst(&theAv->task, AV_INTERNAL_VOLUME_STORE_REQ);

            /* Store configuration after 5 seconds */
            MessageSendLater(&theAv->task, AV_INTERNAL_VOLUME_STORE_REQ, 0, D_SEC(5));
        }
        return TRUE;
    }
    return FALSE;
}

static void appAvVolumeLoadDeviceVolumeAndSet(avInstanceTaskData *theInst)
{
    uint8 volume;

    if (!appAvVolumeIsSet(theInst->avrcp.volume))
    {
        PanicFalse(appDeviceGetVolume(&theInst->bd_addr, &volume));

        DEBUG_LOGF("appAvVolumeLoadDeviceVolumeAndSet(%p), volume=%u", theInst, volume);

        /* Forward volume to other instance if AV connected */
        appAvVolumeSet(volume, theInst);

        theInst->avrcp.volume = volume;
    }
}

/*! \brief Volume handling on A2DP connect.
    \param theInst The connecting instance.

    If A2DP connection is made to the handset before AVRCP, set the system volume
    based on the stored handset device volume.

    If the new A2DP connection is to the peer, AVRCP will also connect and
    appAvVolumeHandleAvrcpConnect will handle volume setup.
*/
void appAvVolumeHandleA2dpConnect(avInstanceTaskData *theInst)
{
    if (appDeviceIsHandset(&theInst->bd_addr))
    {
        appAvVolumeLoadDeviceVolumeAndSet(theInst);
    }
}

/*! \brief Volume handling on AVRCP Connect
    \param theInst The connecting instance.

    If a handset is connected, set the system volume based on the connected
    handset's address (but only if A2DP didn't connect first).

    If the handset is the new connector, set handset and peer volume.
    If the peer is the new connector, just set the peer volume.
*/
void appAvVolumeHandleAvrcpConnect(avInstanceTaskData *theInst)
{
    if (appDeviceIsHandset(&theInst->bd_addr))
    {
        appAvVolumeLoadDeviceVolumeAndSet(theInst);
    }
    else if (appDeviceIsPeer(&theInst->bd_addr))
    {
        /* Check if we have A2DP connection to non TWS+ handset, if so send volume to peer */
        if (appDeviceIsHandsetA2dpConnected())
        {
            bdaddr handset_bd_addr;
            if  (appDeviceGetHandsetBdAddr(&handset_bd_addr) && !appDeviceIsTwsPlusHandset(&handset_bd_addr))
            {
                /* Get volume and forward to peer, don't send to handset */
                avInstanceTaskData *handsetInst = appAvInstanceFindFromBdAddr(&handset_bd_addr);
                if (handsetInst)
                {
                    uint8 handset_volume = handsetInst->avrcp.volume;
                    DEBUG_LOGF("appAvVolumeHandleAvrcpConnect(%p), peer connected, handset already connected, volume=%u", theInst, handset_volume);

                    /* Pass in handset instance so we don't attempt to send volume to handset */
                    appAvVolumeSet(handset_volume, handsetInst);
                }
            }
        }
        else
        {
            DEBUG_LOGF("appAvVolumeHandleAvrcpConnect(%p), peer connected, no handset connected, volume=%u", theInst, theInst->avrcp.volume);

            /* Apply current volume for this instance, as peer (Master) may have already sent volume */
            appAvApplyVolume(theInst->avrcp.volume);
        }
    }
}

/*! \brief Volume handling on AVRCP/A2DP disconnect
    \param theInst The disconnecting instance.
*/
void appAvVolumeHandleAvDisconnect(avInstanceTaskData *theInst)
{
    avTaskData *theAv = appGetAv();

    if (appDeviceIsHandset(&theInst->bd_addr))
    {
        if (MessageCancelFirst(&theAv->task, AV_INTERNAL_VOLUME_STORE_REQ))
        {
            appAvVolumeAttributeStore(theAv);
        }
    }
}

/*! \brief Set new volume

    Set a new volume, and synchronise across other AV links if
    required.

    This function only has any effect if we are the sink for an
    audio stream.

    \param      volume      The new volume
    \param      theOtherInst The instance requesting the volume to be set.
*/
void appAvVolumeSet(uint8 volume, avInstanceTaskData *theOtherInst)
{
    DEBUG_LOGF("appAvVolumeSet, volume %u", volume);

    /* Set local volume, never set a unset volume. */
    PanicFalse(appAvApplyVolume(volume));

    /* Look in table to find connected instance */
    for (int instance = 0; instance < AV_MAX_NUM_INSTANCES; instance++)
    {
        avInstanceTaskData *theInst = appGetAv()->av_inst[instance];
        bdaddr *bd_addr = &theInst->bd_addr;
        if (theInst && (theInst != theOtherInst))
        {
            if (appAvIsAvrcpConnected(theInst))
            {
                if (appDeviceIsHandset(bd_addr))
                {
                    /* Send new volume to source (Handset or TWS Master) */
                    DEBUG_LOGF("appAvVolumeSet, notify volume %u to handset %p", volume, theInst);
                    appAvAvrcpVolumeNotification(theInst, volume);
                }
                else if (appDeviceIsPeer(bd_addr))
                {
                    /* Send new volume to peer, which could be master or slave */
                    if (appDeviceIsHandsetAvrcpConnected())
                    {
                        DEBUG_LOGF("appAvVolumeSet, absolute volume request %u to slave %p", volume, theInst);
                        AvrcpSetAbsoluteVolumeRequest(theInst->avrcp.avrcp, volume);
                    }
                    else
                    {
                        DEBUG_LOGF("appAvVolumeSet, notify volume %u to master %p", volume, theInst);
                        appAvAvrcpVolumeNotification(theInst, volume);
                    }
                }
                theInst->avrcp.volume = volume;
            }
            else
            {
                DEBUG_LOGF("appAvVolumeSet, can't send volume %u to %p, AVRCP not connected", volume, theInst);
            }
        }
    }
}

/*! \brief Make volume change

    \note A2DP/AVRCP volume range is 0-127. 
    \note Increasing the volume by 100 from 100 will return TRUE as the 
    volume changed but the level set will be the maximum - 127

    \param  step    Relative amount to adjust the volume by. 
                    This can be negative.

    \return TRUE if the volume was changed by at least 1 level, 
            FALSE in all other cases
*/
bool appAvVolumeChange(int16 step)
{
    avInstanceTaskData *theInst = appAvGetA2dpSink(AV_CODEC_ANY);
    DEBUG_LOGF("appAvVolumeChange, step %d", step);

    if (theInst)
    {
        uint8 volume = appAvVolumeGet();

        /* Check if increasing volume */
        if (step > 0)
        {
            /* Adjust volume if not at limit */
            if (volume < VOLUME_MAX)
                volume = ((volume + step) <= VOLUME_MAX) ? volume + step : VOLUME_MAX;
            else
                return FALSE;
        }
        else
        {
            /* Adjust volume if not at limit */
            if (volume > 0)
                volume = (volume >= -step) ? volume + step : 0;
            else
                return FALSE;
        }

        appAvVolumeSet(volume, NULL);

        /* Return indicating volume changed successfully */
        return TRUE;
    }
    else
        return FALSE;
}

/*! \brief Start a repeated volume change

    Change the volume by the supplied step, and then request a repeated
    volume increase.

    If the volume is already at a limit (max or min) then a UI tone 
    will be played, and there will not be a repeat.

    \param  step    Relative amount to adjust the volume by. 
                    This can be negative.

    \return TRUE if the volume was changed by at least 1 level, 
            FALSE in all other cases
*/
bool appAvVolumeRepeat(int16 step)
{
    /* Handle volume change locally */
    if (appAvVolumeChange(step))
    {
        MAKE_AV_MESSAGE(AV_INTERNAL_VOLUME_REPEAT);

        /* Send repeat message later */
        message->step = step;
        MessageSendLater(&appGetAv()->task, step > 0 ? AV_INTERNAL_VOLUME_UP_REPEAT : AV_INTERNAL_VOLUME_DOWN_REPEAT,
                         message, APP_UI_VOLUME_REPEAT_TIME);
        appGetAv()->volume_repeat = 1;

        /* Return indicating volume changed */
        return TRUE;
    }

    /* Play volume limit tone */
    appUiAvVolumeLimit();

    /* Return indicating volume not changed */
    return FALSE;
}

/*! \brief Start volume change

    Start a repeated volume change either remotely (tell the handset)
    or locally.

    \param step Relative amount to adjust the volume by. 
     This can be negative.
*/
void appAvVolumeStart(int16 step)
{
    DEBUG_LOGF("appAvVolumeStart(%d)", step);

    /* TODO: Check if using legacy or AbsoluteVolume & Notifications */
    if (1)
    {
        if (appAvVolumeRepeat(step))
        {
            appGetAv()->volume_repeat = 0;
            if (step > 0)
                appUiAvVolumeUp();
            else
                appUiAvVolumeDown();
        }
    }
    else
    {
        /* Use AVRCP to send volume to TWS source */
        if (step > 0)
            appAvVolumeUpRemoteStart();
        else
            appAvVolumeDownRemoteStart();
    }
}

/*! \brief Stop volume change

    Stop a repeated volume change either remotely (tell the handset)
    or locally.

    \param step The step is required so that the correct direction of 
    volume can be cancelled at the handset.
*/
void appAvVolumeStop(int16 step)
{
    /* TODO: Check if using legacy or AbsoluteVolume & Notifications */
    if (1)
    {
        if (MessageCancelFirst(&appGetAv()->task, AV_INTERNAL_VOLUME_UP_REPEAT))
        {
            if (appGetAv()->volume_repeat)
                appUiAvVolumeUp();
        }
        if (MessageCancelFirst(&appGetAv()->task, AV_INTERNAL_VOLUME_DOWN_REPEAT))
        {
            if (appGetAv()->volume_repeat)
                appUiAvVolumeDown();
        }
    }
    else
    {
        if (step > 0)
            appAvVolumeUpRemoteStop();
        else
            appAvVolumeDownRemoteStop();
    }
}

/* 
 * TWS+: Set volume from phone to Earbud.
 * Phone -> Earbud (TWS+)
 * 
 * Standard TWS:  Set volume from phone to Master, and from Master to Slave
 * Phone -> Earbud (Master) -> Earbud (Slave)
 */
void appAvInstanceHandleAvAvrcpSetVolumeInd(avInstanceTaskData *theInst, AV_AVRCP_SET_VOLUME_IND_T *ind)
{
    assert(theInst == ind->av_instance);
    DEBUG_LOGF("appAvInstanceHandleAvAvrcpSetVolumeInd(%p), volume %u", (void *)theInst, ind->volume);

    /* Set volume and forward to slave if connected */
    appAvVolumeSet(ind->volume, theInst);
}


/* 
 * Standard TWS:  Set volume from Slave to Master
 * Earbud (Slave) -> Earbud (Master) -> Phone
 */
void appAvInstanceHandleAvAvrcpVolumeChangedInd(avInstanceTaskData *theInst, AV_AVRCP_VOLUME_CHANGED_IND_T *ind)
{
    assert(theInst == ind->av_instance);
    DEBUG_LOGF("appAvInstanceHandleAvAvrcpVolumeChangedInd(%p), volume %u", (void *)theInst, ind->volume);

    /* Set volume and forward to phone if connected */
    appAvVolumeSet(ind->volume, theInst);
}


void appAvVolumeAttributeStore(avTaskData *theAv)
{
    bdaddr bd_addr;
    bool status;
    if (appDeviceIsHandsetConnected())
    {
        PanicFalse(appDeviceGetHandsetBdAddr(&bd_addr));
    }
    else
    {
        appPeerSyncGetPeerHandsetAddr(&bd_addr);
    }
    status = appDeviceSetVolume(&bd_addr, theAv->volume);
    DEBUG_LOGF("appAvVolumeAttributeStore addr=%x,%x,%x vol=%u status=%d",
                bd_addr.nap, bd_addr.uap, bd_addr.lap, theAv->volume, status);
}

#endif
