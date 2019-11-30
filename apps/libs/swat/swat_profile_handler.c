/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    swat_profile_handler.c

DESCRIPTION
    File containing the profile handler functions for the swat library.
*/

/* Application includes */
#include "swat.h"
#include "swat_private.h"
#include "swat_l2cap_handler.h"
#include "swat_command_handler.h"
#include "swat_profile_handler.h"
#include "swat_audio_manager.h"

/****************************************************************************/
void swatProfileHandler(Task task, MessageId id, Message message)
{
    switch(id)
    {
        case SWAT_INTERNAL_SIGNALLING_CONNECT_REQ:
        {
            swatHandleL2capSignallingConnectReq((SWAT_INTERNAL_SIGNALLING_CONNECT_REQ_T *)message);
        }
        break;
        
        case SWAT_INTERNAL_SIGNALLING_DISCONNECT_REQ:
        {
            swatHandleL2capSignallingDisconnectReq( ((SWAT_INTERNAL_SIGNALLING_DISCONNECT_REQ_T *)message)->device_id);
        }
        break;
        
        case SWAT_INTERNAL_MEDIA_OPEN_REQ:
        {
            swatSendMediaOpenRequest( ((SWAT_INTERNAL_MEDIA_OPEN_REQ_T *)message)->device_id, ((SWAT_INTERNAL_MEDIA_OPEN_REQ_T *)message)->media_type );
        }
        break;
        
        case SWAT_INTERNAL_MEDIA_START_REQ:
        {
            swatSendMediaStartReq( ((SWAT_INTERNAL_MEDIA_START_REQ_T *)message)->device_id, ((SWAT_INTERNAL_MEDIA_START_REQ_T *)message)->media_type );
        }
        break;
        
        case SWAT_INTERNAL_MEDIA_SUSPEND_REQ:
        {
            swatSendMediaSuspendReq( ((SWAT_INTERNAL_MEDIA_SUSPEND_REQ_T *)message)->device_id, ((SWAT_INTERNAL_MEDIA_SUSPEND_REQ_T *)message)->media_type );
        }
        break;
        
        case SWAT_INTERNAL_SET_VOLUME_REQ:
        {
            swatSendSetVolumeReq( ((SWAT_INTERNAL_SET_VOLUME_REQ_T *)message)->device_id, ((SWAT_INTERNAL_SET_VOLUME_REQ_T *)message)->volume, ((SWAT_INTERNAL_SET_VOLUME_REQ_T *)message)->sub_trim );
        }
        break;
        
        case SWAT_INTERNAL_MEDIA_CLOSE_REQ:
        {
            swatSendMediaCloseReq( ((SWAT_INTERNAL_MEDIA_CLOSE_REQ_T *)message)->device_id, ((SWAT_INTERNAL_MEDIA_CLOSE_REQ_T *)message)->media_type );
        }
        break;
        
        /* Has a media channel has become unresponsive? */
        case SWAT_INTERNAL_MEDIA_WATCHDOG_BASE:
        case SWAT_INTERNAL_MEDIA_WATCHDOG_BASE + 1:
        case SWAT_INTERNAL_MEDIA_WATCHDOG_BASE + 2:
        case SWAT_INTERNAL_MEDIA_WATCHDOG_BASE + 3:
        case SWAT_INTERNAL_MEDIA_WATCHDOG_BASE + 4:
        case SWAT_INTERNAL_MEDIA_WATCHDOG_BASE + 5:
        case SWAT_INTERNAL_MEDIA_WATCHDOG_BASE + 6:
        {
            SWAT_DEBUG(("[SWAT] SWAT_INTERNAL_MEDIA_WATCHDOG_BASE Device ID[%x]\n", id - SWAT_INTERNAL_MEDIA_WATCHDOG_BASE));
            swatHandleUnresponsiveMediaStd( id - SWAT_INTERNAL_MEDIA_WATCHDOG_BASE );
        }
        break;
        
        /* Has a low latency media channel become unresponsive? */
        case SWAT_INTERNAL_MEDIA_LL_WATCHDOG_BASE:
        case SWAT_INTERNAL_MEDIA_LL_WATCHDOG_BASE + 1:
        case SWAT_INTERNAL_MEDIA_LL_WATCHDOG_BASE + 2:
        case SWAT_INTERNAL_MEDIA_LL_WATCHDOG_BASE + 3:
        case SWAT_INTERNAL_MEDIA_LL_WATCHDOG_BASE + 4:
        case SWAT_INTERNAL_MEDIA_LL_WATCHDOG_BASE + 5:
        case SWAT_INTERNAL_MEDIA_LL_WATCHDOG_BASE + 6:
        {
            SWAT_DEBUG(("[SWAT] SWAT_INTERNAL_MEDIA_LL_WATCHDOG_BASE Device ID[%x]\n", id - SWAT_INTERNAL_MEDIA_LL_WATCHDOG_BASE));
            swatHandleUnresponsiveMediaLL( id - SWAT_INTERNAL_MEDIA_LL_WATCHDOG_BASE );
        }
        break;
        
        default:
        {
            SWAT_DEBUG(("[SWAT] Unhandled Message ID [%x]\n", id));
        }
        break;
    }
}

