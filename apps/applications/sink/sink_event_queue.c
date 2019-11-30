/****************************************************************************
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_event_queue.c

DESCRIPTION
    Routines to handle sink audio prompts/tones queue.
*/

#include "sink_debug.h"
#include "sink_event_queue.h"
#include "sink_malloc_debug.h"

#include <string.h>

#ifdef DEBUG_AUDIO_PROMPTS_TONES_QUEUE
    #define SINK_EVT_Q_DBG(x) DEBUG(x)
#else
    #define SINK_EVT_Q_DBG(x)
#endif

/*
    sink event queue related defnitions.
*/
#define EVENT_QUEUE_SIZE                          25
#define EVENT_QUEUE_MESSAGE_SIZE                  (sizeof(uint16))

/*
    Mask for common user events
*/
#define SYSTEM_VOLUME_UP_EVENT_MASK               0x0001
#define SYSTEM_VOLUME_DOWN_EVENT_MASK             0x0002
#define SYSTEM_AVRCP_SKIP_FORWARD_EVENT_MASK      0x0004
#define SYSTEM_AVRCP_SKIP_BACKWARD_EVENT_MASK     0x0008
#define SYSTEM_MAIN_MUTE_TOGGLE_EVENT_MASK        0x0010
#define SYSTEM_AVRCP_PLAY_PAUSE_EVENT_MASK        0x0020
#define SYSTEM_VOLUME_MAX_EVENT_MASK              0x0040
#define SYSTEM_VOLUME_MIN_EVENT_MASK              0x0080

#define SINK_MAIN_EVENT                           0x00
#define SINK_PAIRED_EVENT                         0x01
#define NEXT_INDEX                                0x01

/*
    Debug Macros
*/
#define SINK_EVT_Q_ENTER(x)       SINK_EVT_Q_DBG(("\nQ: Entering %s  %s %X",__FILE__,__LINE__,x))
#define SINK_EVT_Q_EXIT()         SINK_EVT_Q_DBG(("\nQ: Exiting %s  %s",__FILE__,__LINE__))

/*
    Sink event queue head pointer
*/
struct queue_t *event_queue;

/***********************************************************************************************
NAME
    sinkEventQueueSetCommonEventMask

DESCRIPTION
    Sets mask in queue to indicate the presence of common events(e.g. Volume Up/Down ,
    skip forward/backward , Mute toggle)in queue.This routine will indicate to queue
    that common event is already present and it should not be stored again.

PARAMETERS
    sink_event    - incoming sink event

RETURNS
    voidss
*/
static void sinkEventQueueSetCommonEventMask(sinkEvents_t sink_event)
{
    SINK_EVT_Q_ENTER((sink_event));

	switch (sink_event)
	{
		case EventUsrMainOutVolumeUp:
			event_queue->sink_queue_events_mask |= SYSTEM_VOLUME_UP_EVENT_MASK;
			break;
		case EventUsrMainOutVolumeDown:
			event_queue->sink_queue_events_mask |= SYSTEM_VOLUME_DOWN_EVENT_MASK;
			break;
		case EventUsrAvrcpSkipForward:
			event_queue->sink_queue_events_mask |= SYSTEM_AVRCP_SKIP_FORWARD_EVENT_MASK;
			break;
		case EventUsrAvrcpSkipBackward:
			event_queue->sink_queue_events_mask |= SYSTEM_AVRCP_SKIP_BACKWARD_EVENT_MASK;
			break;
		case EventUsrMainOutMuteToggle:
			event_queue->sink_queue_events_mask |= SYSTEM_MAIN_MUTE_TOGGLE_EVENT_MASK;
			break;
		case EventUsrAvrcpPlayPause:
			event_queue->sink_queue_events_mask |= SYSTEM_AVRCP_PLAY_PAUSE_EVENT_MASK;
			break;
		case EventSysVolumeMax:
			event_queue->sink_queue_events_mask |= SYSTEM_VOLUME_MAX_EVENT_MASK;
			break;
		case EventSysVolumeMin:
			event_queue->sink_queue_events_mask |= SYSTEM_VOLUME_MIN_EVENT_MASK;
			break;
        default:
            break;
	}

    SINK_EVT_Q_EXIT();
}

/***********************************************************************************************
NAME
    sinkEventQueueResetCommonEventMask

DESCRIPTION
    Resets masks in queue to indicate the absence of common events(e.g. Volume
    Up/Down ,skip forward/backward , Mute toggle)in queue again.

PARAMETERS
    sink_event    - incoming sink event

RETURNS
    void
*/
static void sinkEventQueueResetCommonEventMask(sinkEvents_t sink_event)
{
    SINK_EVT_Q_ENTER(sink_event);
	
	switch (sink_event)
	{
		case  EventUsrMainOutVolumeUp:
			event_queue->sink_queue_events_mask &= ~SYSTEM_VOLUME_UP_EVENT_MASK;
			break;
		case  EventUsrMainOutVolumeDown:
			event_queue->sink_queue_events_mask &= ~SYSTEM_VOLUME_DOWN_EVENT_MASK;
			break;
		case  EventUsrAvrcpSkipForward:
			event_queue->sink_queue_events_mask &= ~SYSTEM_AVRCP_SKIP_FORWARD_EVENT_MASK;
			break;
		case  EventUsrAvrcpSkipBackward:
			event_queue->sink_queue_events_mask &= ~SYSTEM_AVRCP_SKIP_BACKWARD_EVENT_MASK;
			break;
		case  EventUsrMainOutMuteToggle:
			 event_queue->sink_queue_events_mask &= ~SYSTEM_MAIN_MUTE_TOGGLE_EVENT_MASK;
			break;
		case  EventUsrAvrcpPlayPause:
			 event_queue->sink_queue_events_mask &= ~SYSTEM_AVRCP_PLAY_PAUSE_EVENT_MASK;
			break;
		case  EventSysVolumeMax:
			event_queue->sink_queue_events_mask  &= ~SYSTEM_VOLUME_MAX_EVENT_MASK;
			break;
		case  EventSysVolumeMin:
			 event_queue->sink_queue_events_mask &= ~SYSTEM_VOLUME_MIN_EVENT_MASK;
			break;
        default:
            break;
	}

    SINK_EVT_Q_EXIT();
}

/***********************************************************************************************
NAME
    sinkEventQueueCreate

DESCRIPTION
    Creates sink event queue to store audio prompts/tones events and initializes
    it.

PARAMETERS
    void

RETURNS
    TRUE if event queue created successfully, FALSE otherwise
*/
static bool sinkEventQueueCreate(void)
{
    if(sinkEventQueuePresent())
    {
        SINK_EVT_Q_DBG(("\n@Q Sink Evt Queue is already present"));
        return TRUE;
    }
    /* EVENT_QUEUE_SIZE is deducted with 1 here as struct queue_t already
     * reserves memory for 1 message at the end of the struct
     */
    event_queue = (struct queue_t*)mallocPanic(sizeof(struct queue_t)+((EVENT_QUEUE_SIZE-1)*EVENT_QUEUE_MESSAGE_SIZE));
    if(event_queue == NULL)
    {
       SINK_EVT_Q_DBG(("\n@Q Mem Allocation Failed"));
       return FALSE;
    }
    else
    {
        /*Initialize queue structure*/
        memset(event_queue, 0, (sizeof(struct queue_t)+((EVENT_QUEUE_SIZE-1)*EVENT_QUEUE_MESSAGE_SIZE)));
        return TRUE;
    }
}

/************************************************************************************************
NAME
    isSinkEventQueueFull

DESCRIPTION
    Checks if event queue is full or not and returns status.

PARAMETERS
    void

RETURNS
    TRUE if event queue is full, FALSE otherwise.
*/
static bool isSinkEventQueueFull(void)
{
    if(sinkEventQueuePresent())
    {
        if(event_queue->sink_queue_start == (event_queue->sink_queue_end % EVENT_QUEUE_SIZE)
           && (event_queue->event[event_queue->sink_queue_start] != 0))
        {
            return TRUE;
        }
    }
    return FALSE;
}

/************************************************************************************************
NAME
    sinkEventQueueStoreEvent

DESCRIPTION
    Stores incoming event in the sink event queue.

PARAMETERS
    sink_event    - incoming sink event

RETURNS
    void.
*/
static void sinkEventQueueStoreEvent(sinkEvents_t sink_event)
{
    SINK_EVT_Q_ENTER((sink_event));
	
    if(isSinkEventQueueFull())
    {
        SINK_EVT_Q_DBG(("\n@Q Full start=%d end=%d",event_queue->sink_queue_start,
                                                     event_queue->sink_queue_end));

       /*queue is full so remove oldest event and shift queue start to accomodate new event*/
        /*Reset mask for this event if it has been set before*/
        sinkEventQueueResetCommonEventMask(event_queue->event[event_queue->sink_queue_start]);
        event_queue->sink_queue_start++;
        if(event_queue->sink_queue_start == EVENT_QUEUE_SIZE)
        {
            event_queue->sink_queue_start %= EVENT_QUEUE_SIZE;
        }
    }

    event_queue->sink_queue_end %= EVENT_QUEUE_SIZE;
    SINK_EVT_Q_DBG(("\n@Q end=%d",event_queue->sink_queue_end));
    event_queue->event[event_queue->sink_queue_end++] = sink_event;

    SINK_EVT_Q_EXIT();
}

/************************************************************************************************
NAME
    sinkEventQueueCheckEmpty
    
DESCRIPTION
    Checks if sink event queue is empty or not and returns status.

PARAMETERS
    void

RETURNS
    TRUE if sink event queue is empty , FALSE otherwise
*/
static bool sinkEventQueueCheckEmpty(void)
{
    if(sinkEventQueuePresent()) 
    {
        if((event_queue->sink_queue_start == event_queue->sink_queue_end)
            && (event_queue->event[event_queue->sink_queue_start] == 0))
        {
            return TRUE;
        }
    }
    return FALSE;

}

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
bool sinkEventQueuePresent(void)
{
    return (event_queue != NULL);
}

/************************************************************************************************
NAME
    sinkEventQueueDelete
    
DESCRIPTION
    Deletes sink event queue if present.

PARAMETERS
    void

RETURNS
    void
*/
void sinkEventQueueDelete(void)
{
    if(sinkEventQueuePresent())
    {
        freePanic(event_queue);
        event_queue = NULL;
        SINK_EVT_Q_DBG(("\n@Q Deleted"));
    }
}

/************************************************************************************************
NAME 
    sinkEventQueueFetch

DESCRIPTION
    fetches event from sink event queue one at a time and returns it back to caller.

PARAMETERS
    void

RETURNS
    Fetched event from the sink event queue OR EventInvalid if
    queue is empty and has been deleted.
*/
sinkEvents_t sinkEventQueueFetch(void)
{
    uint16 evt = 0;

    SINK_EVT_Q_ENTER((0));
	
    if(sinkEventQueuePresent())
    {
        if(!sinkEventQueueCheckEmpty())
        {
            event_queue->sink_queue_start %= EVENT_QUEUE_SIZE;
            evt = event_queue->event[event_queue->sink_queue_start];
            event_queue->event[event_queue->sink_queue_start] = 0;

            SINK_EVT_Q_DBG(("\n@Q Start = %d",event_queue->sink_queue_start));

            event_queue->sink_queue_start++;
            sinkEventQueueResetCommonEventMask(evt);
            SINK_EVT_Q_EXIT();
            return (sinkEvents_t)evt;
        }
        else
        {
            sinkEventQueueDelete();
        }
    }

    SINK_EVT_Q_EXIT();
    return EventInvalid;
}

/***********************************************************************************************
NAME
    sinkEventQueueFindAndCancelEvent

DESCRIPTION
    Finds already present common event , cancels its first presence and adds this event at the
    last of the queue(as this event is a recent event so added at last).It also finds the paired
    event for the incoming common event and cancels this also if requested by caller.

PARAMETERS
    sink_event       - incoming sink event
    sink_event_type  - type of event ( if main common event or paired event)

RETURNS
    void
*/
static void sinkEventQueueFindAndCancelEvent(sinkEvents_t sink_event, bool sink_event_type)
{
    uint16 queue_start = event_queue->sink_queue_start;

    SINK_EVT_Q_ENTER((sink_event));

    while((event_queue->event[queue_start] != sink_event) &&
          (event_queue->event[queue_start] != 0x00))
    {
        queue_start++;
        if(queue_start == EVENT_QUEUE_SIZE)
        {
            queue_start %= EVENT_QUEUE_SIZE;
        }
    }

    /* Event is present so cancel previous and update this event in the last
       of the queue so the previous stored events will not be affected  */
    while(event_queue->event[queue_start] != 0x00)
    {
        if((event_queue->event[queue_start + NEXT_INDEX] != 0x00) &&
           (queue_start+1 !=  event_queue->sink_queue_start))
        {
            event_queue->event[queue_start] = event_queue->event[queue_start + NEXT_INDEX];
        }
        else
        {
            break;
        }
        ++queue_start;

        if(queue_start == EVENT_QUEUE_SIZE)
        {
            queue_start %= EVENT_QUEUE_SIZE;
        }
    }

    /* Write Event at last free location in queue.Since every event is shifted to one location in queue so
       this event (whose previous instance has been cancled by shifting queue) should be added at the previous
       last event location( after this queue indexes will be free(containing 0)*/
    if(sink_event_type == SINK_MAIN_EVENT)
    {
        event_queue->event[queue_start] = sink_event;
    }
    else
    {
        /* This is the case when paired event has been canceled and queue has been shifted again.
           so write 0 at the last events location in queue as it has been shifted one place up*/
        event_queue->sink_queue_end--;
        event_queue->event[queue_start] = 0;
        sinkEventQueueResetCommonEventMask(sink_event);
    }

    SINK_EVT_Q_EXIT();
}

/************************************************************************************
NAME
    sinkEventQueueValidateEvent

DESCRIPTION
    Checks whether the incoming sink event is already present in queue or not.

PARAMETERS
    sink_event  - incoming sink event

RETURNS
    Returns TRUE if event is present in queue otherwise returns FALSE.
*/
static bool sinkEventQueueValidateEvent(sinkEvents_t sink_event)
{
    bool ret_val = FALSE;

    SINK_EVT_Q_ENTER((sink_event));

    /* Check incoming event MASK is set or not . If set means event is present*/
    if((sink_event == EventUsrMainOutVolumeUp) && 
       (event_queue->sink_queue_events_mask & SYSTEM_VOLUME_UP_EVENT_MASK))
    {
        ret_val = TRUE;
    }
    else if((sink_event == EventUsrMainOutVolumeDown) &&
            (event_queue->sink_queue_events_mask & SYSTEM_VOLUME_DOWN_EVENT_MASK))
    {
        ret_val = TRUE;
    }
    else if((sink_event == EventUsrAvrcpSkipForward) &&
            (event_queue->sink_queue_events_mask & SYSTEM_AVRCP_SKIP_FORWARD_EVENT_MASK))
    {
        ret_val = TRUE;
    }
    else if((sink_event == EventUsrAvrcpSkipBackward) &&
            (event_queue->sink_queue_events_mask & SYSTEM_AVRCP_SKIP_BACKWARD_EVENT_MASK))
    {
        ret_val = TRUE;
    }
    else if((sink_event == EventUsrMicrophoneMuteToggle) &&
            (event_queue->sink_queue_events_mask & SYSTEM_MAIN_MUTE_TOGGLE_EVENT_MASK))
    {
        ret_val = TRUE;
    }
    else if((sink_event == EventUsrAvrcpPlayPause) &&
            (event_queue->sink_queue_events_mask & SYSTEM_AVRCP_PLAY_PAUSE_EVENT_MASK))
    {
        ret_val = TRUE;
    }
    else if((sink_event == EventSysVolumeMax) &&
            (event_queue->sink_queue_events_mask & SYSTEM_VOLUME_MAX_EVENT_MASK))
    {
        ret_val = TRUE;
    }
    else if((sink_event == EventSysVolumeMin) &&
            (event_queue->sink_queue_events_mask & SYSTEM_VOLUME_MIN_EVENT_MASK))
    {
        ret_val = TRUE;
    }

    SINK_EVT_Q_EXIT();
    return ret_val;
}


/***********************************************************************************************
NAME
    sinkEventQueueCancelEvent

DESCRIPTION
    Cancels the incoming common event as it is already present in the queue and adds it back 
    at end of the queue.Cancels the paired event of the main incoming event also if it is 
    present in the queue.

PARAMETERS
    sink_event  - incoming sink event

RETURNS
    void
*/
static void sinkEventQueueCancelEvent(sinkEvents_t sink_event)
{
    SINK_EVT_Q_ENTER((sink_event));

    if(sink_event == EventUsrMainOutVolumeUp )
    {
        /* Check if incoming event is already present in queue and if yes
           then cancel it and store this incoming event at the end of queue
           to be processed later.*/

        sinkEventQueueFindAndCancelEvent(sink_event,SINK_MAIN_EVENT);

        /*Incoming event was present in queue so have been moved to back of the
          queue and the previous occurence canceled . Now check if paired event
          is also present then cancel it also*/
        if(sinkEventQueueValidateEvent(EventUsrMainOutVolumeDown) == TRUE)
        {
            sinkEventQueueFindAndCancelEvent(EventUsrMainOutVolumeDown,SINK_PAIRED_EVENT);
        }
    }
    else if(sink_event ==EventUsrMainOutVolumeDown)
    {
        sinkEventQueueFindAndCancelEvent(sink_event,SINK_MAIN_EVENT);
        if(sinkEventQueueValidateEvent(EventUsrMainOutVolumeUp) == TRUE)
        {
            /*canceled incoming event . check paired event*/
            sinkEventQueueFindAndCancelEvent(EventUsrMainOutVolumeUp,SINK_PAIRED_EVENT);
        }
    }
    else if(sink_event == EventUsrAvrcpSkipForward)
    {
        sinkEventQueueFindAndCancelEvent(sink_event,SINK_MAIN_EVENT);
        if(sinkEventQueueValidateEvent(EventUsrAvrcpSkipBackward) == TRUE)
        {
            /*canceled incoming event . check paired event*/
            sinkEventQueueFindAndCancelEvent(EventUsrAvrcpSkipBackward,SINK_PAIRED_EVENT);
         }
    }
    else if(sink_event ==EventUsrAvrcpSkipBackward)
    {
        sinkEventQueueFindAndCancelEvent(sink_event,SINK_MAIN_EVENT);
        if(sinkEventQueueValidateEvent(EventUsrAvrcpSkipForward) == TRUE)
        {
            /*canceled incoming event . check paired event*/
            sinkEventQueueFindAndCancelEvent(EventUsrAvrcpSkipForward,SINK_PAIRED_EVENT);
        }
    }

    SINK_EVT_Q_EXIT();
}

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
void sinkEventQueueCancelVolumeUpAndDownEventsOnMute(void)
{
    SINK_EVT_Q_DBG(("Entering sinkEventQueueCancelVolumeUpAndDownEventsOnMute() \n"));
    /*cancel vol up if present*/
    if(sinkEventQueueValidateEvent(EventUsrMainOutVolumeUp) == TRUE)
    {
        /*canceled incoming event . check paired event*/
        sinkEventQueueFindAndCancelEvent(EventUsrMainOutVolumeUp,SINK_PAIRED_EVENT);
    }
    /*cancel vol down if present*/
    if(sinkEventQueueValidateEvent(EventUsrMainOutVolumeDown) == TRUE)
    {
        /*canceled incoming event . check paired event*/
        sinkEventQueueFindAndCancelEvent(EventUsrMainOutVolumeDown,SINK_PAIRED_EVENT);
    }
     SINK_EVT_Q_EXIT();
}
/************************************************************************************************
NAME 
    sinkEventQueueStore

DESCRIPTION
    Checks if incoming event has audio prompt/tone attached if yes then creates
    sink event queue (if not present) and then stores event in it after checking
    if event is to be stored or not.If event is already present then previous
    occurence of that event will be cancled and it will be added in the last of
    the queue as it is recent event.

PARAMETERS
    sink_event    - incoming sink event

RETURNS
    void.
*/
void sinkEventQueueStore(sinkEvents_t sink_event )
{
    if(!sinkEventQueuePresent())
    {
        /*Event queue is not present(means either all previous events have been processed
          and queue deleted or first time queue is getting created*/
        if(!sinkEventQueueCreate())
        {
            /*Memory allocation failed.*/
            Panic();
        }
    }

    /*Event to be added so check if it is already present in queue or not
      if not present then add this event in queue*/
    if(!sinkEventQueueValidateEvent(sink_event))
    {
        /*set mask in queue for common user events so there presence in queue
          may be marked*/
        sinkEventQueueSetCommonEventMask(sink_event);

        /*Store event in queue*/
        sinkEventQueueStoreEvent(sink_event);

        /*event added successfully in queue*/
        SINK_EVT_Q_DBG(("\n@Q Stored %X \n",sink_event));
    }
    else
    {
        /*event is already present in the queue so we need to cancel its first presence
          and add it last in queue so that it may be played at proper time. Additionally
          cancel any paired event (if any for incoming event)also*/
        sinkEventQueueCancelEvent(sink_event);
    }
    SINK_EVT_Q_EXIT();
    SINK_EVT_Q_DBG(("\n\n"));
}
