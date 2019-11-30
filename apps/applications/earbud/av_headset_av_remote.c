/*!
\copyright  Copyright (c) 2008 - 2018 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_av_remote.c
\brief      AV remote control/play status handling
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



void appAvInstanceHandleAvAvrcpPlayStatusChangedInd(avInstanceTaskData *theOtherInst, AV_AVRCP_PLAY_STATUS_CHANGED_IND_T *ind)
{
    /* Look in table to find connected instance */
    for (int instance = 0; instance < AV_MAX_NUM_INSTANCES; instance++)
    {
        avInstanceTaskData *theInst = appGetAv()->av_inst[instance];
        if (theInst && (theInst != theOtherInst))
        {
            if (appAvIsAvrcpConnected(theInst) && appDeviceIsPeer(&theInst->bd_addr))
            {
                DEBUG_LOGF("appAvInstanceHandleAvAvrcpPlayStatusChangedInd, send play status %u to %p", ind->play_status, theInst);
                appAvAvrcpPlayStatusNotification(theInst, ind->play_status);
            }
        }
    }
}



/*! \brief Send remote control command

    This function is called to send an AVRCP Passthrough (remote control) command, if there are
    multiple AV instances then it will pick the instance which is an A2DP sink.
*/
static void appAvRemoteControl(avc_operation_id op_id, uint8 rstate, bool beep, uint16 repeat)
{
    avInstanceTaskData *theInst = appAvInstanceFindAvrcpForPassthrough();
    DEBUG_LOG("appAvRemoteControl");

    /* Check there is an instance to send AVRCP passthrough */
    if (theInst)
    {
        /* Send AVRCP passthrough request*/
        appAvrcpRemoteControl(theInst, op_id, rstate, beep, repeat);
    }
}

/*! \brief Send Pause command

    \param  ui  Indicates if there should be a User Interface action as a result
            of the sent pause command (that is should there be a beep).
*/
void appAvPause(bool ui)
{
    /* Check there is an instance to send AVRCP passthrough */
    avInstanceTaskData *theInst = appAvInstanceFindAvrcpForPassthrough();
    if (theInst)
    {
        DEBUG_LOGF("appAvPause(%d)", ui);
        MessageCancelAll(&theInst->av_task, AV_INTERNAL_AVRCP_PLAY_REQ);
        MessageCancelAll(&theInst->av_task, AV_INTERNAL_AVRCP_PAUSE_REQ);
        MessageSendConditionally(&theInst->av_task, AV_INTERNAL_AVRCP_PAUSE_REQ, NULL,
                                 &theInst->avrcp.playback_lock);
        if (ui)
            appUiAvRemoteControl();
    }
}

/*! \brief Send Play command

    \param  ui  Indicates if there should be a User Interface action as a result
            of the sent play command (that is should there be a beep).
*/
void appAvPlay(bool ui)
{
    /* Check there is an instance to send AVRCP passthrough */
    avInstanceTaskData *theInst = appAvInstanceFindAvrcpForPassthrough();
    if (theInst)
    {
        DEBUG_LOGF("appAvPlay(%d)", ui);
        MessageCancelAll(&theInst->av_task, AV_INTERNAL_AVRCP_PLAY_REQ);
        MessageCancelAll(&theInst->av_task, AV_INTERNAL_AVRCP_PAUSE_REQ);
        MessageSendConditionally(&theInst->av_task, AV_INTERNAL_AVRCP_PLAY_REQ, NULL,
                                 &theInst->avrcp.playback_lock);
        if (ui)
            appUiAvRemoteControl();
    }
}

/*! \brief Send Play or Pause command

    \param  ui  Indicates if there should be a User Interface action as a result
            of the sent command (that is should there be a beep).
*/
void appAvPlayToggle(bool ui)
{
    /* Check there is an instance to send AVRCP passthrough */
    avInstanceTaskData *theInst = appAvInstanceFindAvrcpForPassthrough();
    if (theInst)
    {
        DEBUG_LOG("appAvPlayToggle(%d)", ui);
        MessageCancelAll(&theInst->av_task, AV_INTERNAL_AVRCP_PLAY_REQ);
        MessageCancelAll(&theInst->av_task, AV_INTERNAL_AVRCP_PAUSE_REQ);
        MessageCancelAll(&theInst->av_task, AV_INTERNAL_AVRCP_PLAY_TOGGLE_REQ);
        MessageSendConditionally(&theInst->av_task, AV_INTERNAL_AVRCP_PLAY_TOGGLE_REQ, NULL,
                                 &theInst->avrcp.playback_lock);
        if (ui)
            appUiAvRemoteControl();
    }
}

/*! \brief Send Stop command

    \param  ui  Indicates if there should be a User Interface action as a result
                of the sent stop command (that is should there be a beep).
*/
void appAvStop(bool ui)
{
    DEBUG_LOGF("appAvStop(%d)", ui);

    appAvRemoteControl(opid_stop, 0, ui, 0);
    appAvRemoteControl(opid_stop, 1, FALSE, 0);

    appAvHintPlayStatus(avrcp_play_status_stopped);
}

/*! \brief Send Forward command
*/
void appAvForward(void)
{
    DEBUG_LOG("appAvForward");
    appAvRemoteControl(opid_forward, 0, TRUE, 0);
    appAvRemoteControl(opid_forward, 1, FALSE, 0);
}

/*! \brief Send Backward command
*/
void appAvBackward(void)
{
    DEBUG_LOG("appAvBackward");
    appAvRemoteControl(opid_backward, 0, TRUE, 0);
    appAvRemoteControl(opid_backward, 1, FALSE, 0);
}

/*! \brief Send Volume Up start command
*/
void appAvVolumeUpRemoteStart(void)
{
    DEBUG_LOG("appAvVolumeUpRemoteStart");
    appAvRemoteControl(opid_volume_up, 0, FALSE, APP_UI_VOLUME_REPEAT_TIME);
}

/*! \brief Send Volume Up stop command
*/
void appAvVolumeUpRemoteStop(void)
{
    DEBUG_LOG("appAvVolumeUpRemoteStop");
    appAvRemoteControl(opid_volume_up, 1, FALSE, APP_UI_VOLUME_REPEAT_TIME);
}

/*! \brief Send Volume Down start command
*/
void appAvVolumeDownRemoteStart(void)
{
    DEBUG_LOG("appAvVolumeDownRemoteStart");
    appAvRemoteControl(opid_volume_down, 0, FALSE, APP_UI_VOLUME_REPEAT_TIME);
}

/*! \brief Send Volume Down stop command
*/
void appAvVolumeDownRemoteStop(void)
{
    DEBUG_LOG("appAvVolumeDownRemoteStop");
    appAvRemoteControl(opid_volume_down, 1, FALSE, APP_UI_VOLUME_REPEAT_TIME);
}

/*! \brief Send Fast Forward start command
*/
void appAvFastForwardStart(void)
{
    DEBUG_LOG("appAvFastForwardStart");
    appAvRemoteControl(opid_fast_forward, 0, TRUE, D_SEC(1));
}

/*! \brief Send Fast Forward stop command
*/
void appAvFastForwardStop(void)
{
    DEBUG_LOG("appAvFastForwardStop");
    appAvRemoteControl(opid_fast_forward, 1, TRUE, 0);
}

/*! \brief Send Rewind start command
*/
void appAvRewindStart(void)
{
    DEBUG_LOG("appAvRewindStart");
    appAvRemoteControl(opid_rewind, 0, TRUE, D_SEC(1));
}

/*! \brief Send Rewind stop command
*/
void appAvRewindStop(void)
{
    DEBUG_LOG("appAvRewindStop");
    appAvRemoteControl(opid_rewind, 1, TRUE, 0);
}


/*! \brief Get the current play status of the AV

    \return The play status, the status of the stream from a headset, 
            which can include avrcp_play_status_error. 
            avrcp_play_status_error is also returned if there is no
            current link.
*/
avrcp_play_status appAvPlayStatus(void)
{
    avTaskData *theAv = appGetAv();
    int instance;

    /* First of all in table to find entry that is streaming as sink */
    for (instance = 0; instance < AV_MAX_NUM_INSTANCES; instance++)
    {
        avInstanceTaskData *theInst = theAv->av_inst[instance];
        if (theInst && appA2dpIsSinkCodec(theInst) && appA2dpIsConnectedMedia(theInst))
        {
            if (theInst->avrcp.play_status != avrcp_play_status_error)
                return theInst->avrcp.play_status;
            else
                return theInst->avrcp.play_hint;
        }
    }

    return avrcp_play_status_error;
}

/*! \brief Set the play status if the real status is not known

    The AV should know whether we are playing audio, based on AVRCP
    status messages. This information can be missing, in which case
    this function allows you to set a status. It won't override
    a known status.
    
    \param status   The status hint to be used
 */
void appAvHintPlayStatus(avrcp_play_status status)
{
    avTaskData *theAv = appGetAv();
    int instance;

    /* First of all in table to find entry that is streaming as sink */
    for (instance = 0; instance < AV_MAX_NUM_INSTANCES; instance++)
    {
        avInstanceTaskData *theInst = theAv->av_inst[instance];
        if (theInst && appA2dpIsSinkCodec(theInst) && appA2dpIsConnectedMedia(theInst))
        {
            if (theInst->avrcp.play_status == avrcp_play_status_error)
                theInst->avrcp.play_hint = status;

            return;
        }
    }
}



#endif

