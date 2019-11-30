/****************************************************************************
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_audio_indication.c

DESCRIPTION
    Routines to indicate and play audio prompt/tones events.
*/

#include "sink_main_task.h"
#include "sink_event_queue.h"
#include "sink_audio_prompts.h"
#include "sink_audio_indication.h"
#include "sink_tones.h"
#include "sink_statemanager.h"
#include "sink_anc.h"
#include "sink_volume.h"

#include <audio.h>

#include <stddef.h>

#ifdef DEBUG_AUDIO_INDICATION
    #define AUDIO_IND(x) DEBUG(x)
#else
    #define AUDIO_IND(x)
#endif

#define AUDIO_IND_ENTER(x)       AUDIO_IND(("AUDIO_IND: Entering %s %s %X\n",__FILE__,__LINE__,x))
#define AUDIO_IND_EXIT()         AUDIO_IND(("AUDIO_IND: Exiting %s %s\n",__FILE__,__LINE__))

/***********************************************************************************************
NAME
    sinkAudioIndicationEventCanBeQueued

DESCRIPTION
    Checks if the incoming sink event can be queued or not.

PARAMETERS
    sink_event           - incoming sink event.
    event_has_tone       - if event has tone attached.

RETURNS
    TRUE    if sink event can be queued
    FALSE   if sink event can not be queued
*/
static bool sinkAudioIndicationEventCanBeQueued(sinkEvents_t sink_event, bool event_has_tone)
{
    /*event has tone but tone queueing is disabled in config*/
    if (event_has_tone && !sinkTonesCanQueueEventTones())
    {
        return FALSE;
    }
    return  ((sink_event != EventSysMuteReminder) &&
             (sink_event != EventSysRingtone1) &&
             (sink_event != EventSysRingtone2) &&
             (sink_event != EventSysPromptsTonesQueueCheck));
}

/***********************************************************************************************
NAME
    sinkAudioIndicationEventBypassesMute

DESCRIPTION
    Checks if the incoming sink event is to be played anyway even if the system is muted.

PARAMETERS
    sink_event           - incoming sink event.

RETURNS
    TRUE    if sink event can bypass mute state.
    FALSE   if sink event can not bypass mute state.
*/
static bool sinkAudioIndicationEventBypassesMute(sinkEvents_t sink_event)
{
    return ((sink_event == EventSysMuteReminder) || 
            (sink_event == EventUsrAuxOutMuteOn) || 
            (sink_event == EventUsrMainOutMuteOn));
}

/***********************************************************************************************
NAME
    sinkAudioIndicationCheckConditions

DESCRIPTION
    Checks the condtions before playing or storing the event in the queue. It checks if the
    event is having any audio prompt or tone attached to it as well as event qualifies to be
    stored in the queueu or to be played imidiately or not to be played at all.

PARAMETERS
    sink_event           - incoming sink event
    event_has_prompt     - flag to have indication that incoming event has audio prompt
                           attached to it or not.
    event_has_tone       - flag to have indication that incoming event has audio tone
                           attached to it or not.
    event_can_be_queued  - if the event can be queued or not.

RETURNS
    
*/
static bool sinkAudioIndicationCheckConditions(sinkEvents_t sink_event, bool *event_has_prompt,
                                               bool *event_has_tone,bool *event_can_be_queued)
{
    /* Don't play tone or prompt for Mute Toggle, this event is auto-converted to
       either EventUsrMicrophoneMuteOn or EventUsrMicrophoneMuteOff */
    if (sink_event == EventUsrMicrophoneMuteToggle)
    {
        return FALSE;
    }

    /*if state manager is ready or not*/
    if(!stateManagerIsReadyForAudio())
    {
        return FALSE;
    }

    if(sinkAncIsTuningModeActive())
    {
        return FALSE;
    }

    /*check if event has audio prompt attached to it */
    if(SinkAudioPromptsIsEnabled())
    {
        *event_has_prompt = IsAudioPromptConfiguredForEvent(sink_event);
    }

    /*check if event has any tone attached to it */
    if(!(*event_has_prompt))
    {
        *event_has_tone = IsToneConfiguredForEvent(sink_event);
    }

    if(!(*event_has_prompt || *event_has_tone))
    {
        return FALSE;
    }
    *event_can_be_queued = sinkAudioIndicationEventCanBeQueued(sink_event,*event_has_tone);

    return TRUE;
}

/***********************************************************************************************
NAME
    sinkAudioIndicationScheduleQueueCheck

DESCRIPTION
   Instructs message looper to send EventSysPromptsTonesQueueCheck to the system
   back when audio is not busy so that further sink queue may be processed.

PARAMETERS
    void

RETURNS
    void
*/
static void sinkAudioIndicationScheduleQueueCheck(void)
{   
    /*Send message to looper to tell it that when AUDIO is free then it should
      send back EventSysPromptsTonesQueueCheck event back to sink app*/

    MessageCancelAll(&theSink.task, EventSysPromptsTonesQueueCheck);
    MessageSendConditionally ( &theSink.task, EventSysPromptsTonesQueueCheck,
                               NULL ,(const uint16 *)AudioBusyPtr());
}
/***********************************************************************************************
NAME
    sinkAudioIndicationPlayEvent

DESCRIPTION
    Checks if the incoming event has any audio prompt or tone attached to it. If it has audio
    prompt or tone attached , then it will be played further if audio plugin is free . If audio
    plugin is busy then it will store sink event in the sink event queue and then this event will
    be played later when audio plugin will be free.

PARAMETERS
    sink_event    - incoming sink event

RETURNS
    void
*/
void sinkAudioIndicationPlayEvent(sinkEvents_t sink_event)
{
    bool event_has_prompt = FALSE;
    bool event_has_tone = FALSE;
    bool event_can_be_queued = TRUE; 

    AUDIO_IND_ENTER((sink_event));

    if (!sinkAudioIndicationCheckConditions(sink_event,&event_has_prompt,
                                            &event_has_tone,&event_can_be_queued))
    {
        return;
    }

    /*Check if AUDIO_BUSY is set and then queue event to be processed later*/
    if(IsAudioBusy())
    {
        if (event_can_be_queued)
        {
            /*Check if it is mute toggle and we are processign queue , then we need to cancel 
              volume up and down events queued previously as they will not have any effect
              further so no need to keep them in queue.Find and delete them from the queue*/
            if(sink_event == EventUsrMainOutMuteToggle)
            {
                sinkEventQueueCancelVolumeUpAndDownEventsOnMute();
            }
            sinkEventQueueStore(sink_event);
        }
    }
    else
    {
       /* Ensure sink is not in a completely muted state, unless trying to
          play the mute reminder or a 'MuteOn' tone */
       if (!VolumeAllOutputsMuted() || sinkAudioIndicationEventBypassesMute(sink_event))
       {
            /*Play audio prompt or tone*/
            if(event_has_prompt)
            {
                AudioPromptPlayEvent(sink_event, event_can_be_queued);
            }
            else if(event_has_tone)
            {
                AudioTonePlayEvent(sink_event, event_can_be_queued);
            }
        }
    }

    /*Indicate looper to send EventSysPromptsTonesQueueCheck later 
      when audio is free and sink queue is to-- processed.*/
    if(sinkEventQueuePresent() && (sink_event != EventInvalid))
    {
        /*Schedule event to get sink event queue processed further*/
        sinkAudioIndicationScheduleQueueCheck();
    }
    AUDIO_IND_EXIT();
}
