/*******************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_event_queue.h

DESCRIPTION
    Routines to handle sink audio prompts/tones queue.
    
NOTES
*/

#ifndef _SINK_EVENT_QUEUE_H_
#define _SINK_EVENT_QUEUE_H_

#include "sink_events.h"

/*
    sink event queue data structure.
*/
struct queue_t{
    bool sink_queue_event_sent;
    uint16 sink_queue_events_mask;
    uint8 sink_queue_start;
    uint8 sink_queue_end;
    uint16 event[1];
};

/*
    sink event queue public APIs.
*/

/************************************************************************************************
NAME
    sinkEventQueuePresent

DESCRIPTION
    Checks if event queue is present or not and returns status.

PARAMETERS
    void

RETURNS
    TRUE if event queue is present, FALSE otherwise
*/
bool sinkEventQueuePresent(void);

/***********************************************************************************
NAME
    sinkEventQueueStore

DESCRIPTION
    Stores incoming event in the sink event queue.

PARAMETERS
    sink_event    - incoming sink event

RETURNS
    void.
*/
void sinkEventQueueStore(sinkEvents_t sink_event );

/*********************************************************************************
NAME 
    sinkEventQueueFetch

DESCRIPTION
    fetches event from sink event queue one at a time and returns it 
    back to caller.

PARAMETERS
    void

RETURNS
    Fetched event from the sink event queue OR FALSE if
    queue is empty.
*/
sinkEvents_t sinkEventQueueFetch(void);

/***********************************************************************************
NAME
    sinkEventQueueDelete
    
DESCRIPTION
    Deletes sink event queue if present.

PARAMETERS
    void

RETURNS
    void
*/
void sinkEventQueueDelete(void);

/************************************************************************************************
NAME 
    sinkEventQueueCancelVolumeUpAndDownEventsOnMute

DESCRIPTION
    Cancels already stored volume up and down sink events from the sink
    queue when EventUsrMainOutMuteToggle arrives. The mute indication
    is to be played imidiately and once system is mute then no need to
    keep volume up and down in sink queue.So this routine deletes it.

PARAMETERS
    void

RETURNS
    void.
*/
void sinkEventQueueCancelVolumeUpAndDownEventsOnMute(void);

#endif /*_SINK_EVENT_QUEUE_H_*/
