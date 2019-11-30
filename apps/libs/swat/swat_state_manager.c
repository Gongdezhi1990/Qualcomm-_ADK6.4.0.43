/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    swat_state_manager.c

DESCRIPTION
    Contains functions to manage SWAT media connection states
*/

#include <source.h>
#include <stream.h>
#include <sink.h>

#include "swat.h"
#include "swat_private.h"
#include "swat_state_manager.h"
#include "swat_packet_handler.h"


/****************************************************************************/
void swatSetMediaState(remoteDevice *device, swatMediaType media_type, swatMediaState new_state)
{
    /* Change the media channel state and cancel the appropriate watchdog timer */
    if (media_type == SWAT_MEDIA_STANDARD)
    {
        SWAT_DEBUG(("[SWAT] Device ID[%x] SWAT_MEDIA_STANDARD Changed state from [%x] to [%x]\n", device->id, device->media_state, new_state));
        device->media_state = new_state;
        (void) MessageCancelAll(&swat->profile_task, SWAT_INTERNAL_MEDIA_WATCHDOG_BASE + device->id);
    }
    else if (media_type == SWAT_MEDIA_LOW_LATENCY)
    {
        SWAT_DEBUG(("[SWAT] Device ID[%x] SWAT_MEDIA_LOW_LATENCY Changed state from [%x] to [%x]\n", device->id, device->media_ll_state, new_state));
        device->media_ll_state = new_state;
        (void) MessageCancelAll(&swat->profile_task, SWAT_INTERNAL_MEDIA_LL_WATCHDOG_BASE + device->id);
    }
    
    /* Do we need to restart the watchdog? */
    switch(new_state)
    {
        case swat_media_closed:
        case swat_media_open:
        case swat_media_streaming:
            break;
        
        case swat_media_opening:
        case swat_media_closing:
        case swat_media_starting:
        case swat_media_suspending:
        {
            /* Start the appropriate watchdog timer */
            if (media_type == SWAT_MEDIA_STANDARD)
            {
                MessageSendLater(&swat->profile_task, SWAT_INTERNAL_MEDIA_WATCHDOG_BASE + device->id, 0, SWAT_MESSAGE_TIMEOUT);
            }
            else if (media_type == SWAT_MEDIA_LOW_LATENCY)
            {
                MessageSendLater(&swat->profile_task, SWAT_INTERNAL_MEDIA_LL_WATCHDOG_BASE + device->id, 0, SWAT_MESSAGE_TIMEOUT);
            }
        }
        break;
        
        default:
        {
            /* handle invalid type for media_state quietly */
            SWAT_DEBUG(("[SWAT] swatSetMediaState new_state:[%x]\n", new_state));
        }
        break;
    }
}


/****************************************************************************/
void swatSetSignallingState(remoteDevice *device, swatSignallingState new_state)
{
    SWAT_DEBUG(("[SWAT] Device ID[%x] Signalling channel changed state from [%x] to [%x]\n", device->id, device->signalling_state, new_state));
    device->signalling_state = new_state;
}
