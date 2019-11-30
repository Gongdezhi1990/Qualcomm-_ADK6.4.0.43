/*******************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.
Part of ADK 4.0.1

FILE NAME
    sink_audio_indication.h

DESCRIPTION
    Routines to indicate and play audio prompt/tones events.
    
NOTES
*/

#ifndef _SINK_AUDIO_INDICATION_H_
#define _SINK_AUDIO_INDICATION_H_

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
void sinkAudioIndicationPlayEvent(sinkEvents_t sink_event);

#endif /*_SINK_AUDIO_INDICATION_H_*/
