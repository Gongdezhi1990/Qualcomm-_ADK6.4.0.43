/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_volume.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_link_manager.h"
#include "hfp_volume_handler.h"

#include <panic.h>

/****************************************************************************
NAME    
    HfpVolumeSyncSpeakerGainRequest

DESCRIPTION
    Request to send the local speaker setting (volume) to the AG. The HFP
    specification limits the value of volume to be in the range 0-15 and
    the Hfp profile library will enforce this. The request is issued on the 
    SLC associated with the hfp profile priority passed in by the application. 
    The message returned indicates whether the command was recognised by the 
    AG or not. 

    The AG may autonomously send volume gain indications to the HFP device,
    the application will be notified of these using the HFP_VOLUME_SYNC_SPEAKER_GAIN_IND
    message.

RETURNS
    TRUE if volume was sent successfully, FALSE otherwise. This does not take
    account of the AG's response. HFP will limit the volume sent to be in the
    range 0-15. The uint8 pointed to by volume will be updated with the actual 
    volume that was sent to the AG if successful.
*/
bool HfpVolumeSyncSpeakerGainRequest(hfp_link_priority priority, uint8* volume)
{
    /* Get the link */
    hfp_link_data* link = hfpGetLinkFromPriority(priority);
    
    /* Check we are in valid state (as this function bypasses profile handler) */
    if(link && link->bitfields.ag_slc_state == hfp_slc_complete)
    {
        hfpHandleVgsRequest(link, *volume);
        *volume = (uint8)link->bitfields.at_vol_setting;
        return TRUE;
    }
    /* Priority/state was invalid or HFP not initialised */
    return FALSE;
}


/****************************************************************************
NAME    
    HfpVolumeSyncMicrophoneGainRequest

DESCRIPTION
    Request to send the local microphone setting (volume) to the AG. The HFP
    specification limits the value of volume to be in the range 0-15 and
    the Hfp profile library will enforce this. The request is issued on the 
    SLC associated with the hfp profile priority passed in by the application. 
    The message returned indicates whether the command was recognised by the 
    AG or not.

    The AG may autonomously send microphone gain indications to the HFP device.
    The application will be notified of these using the 
    HFP_VOLUME_SYNC_MICROPHONE_GAIN_IND message.

RETURNS
    TRUE if volume was sent successfully, FALSE otherwise. This does not take
    account of the AG's response. HFP will limit the volume sent to be in the
    range 0-15. The uint8 pointed to by volume will be updated with the actual 
    volume that was sent to the AG if successful.
*/
bool HfpVolumeSyncMicrophoneGainRequest(hfp_link_priority priority, uint8* volume)
{
    /* Get the link */
    hfp_link_data* link = hfpGetLinkFromPriority(priority);
    
    /* Check we are in valid state (as this function bypasses profile handler) */
    if(link && link->bitfields.ag_slc_state == hfp_slc_complete)
    {
        hfpHandleVgmRequest(link, *volume);
        return TRUE;
    }
    /* Priority/state was invalid or HFP not initialised */
    return FALSE;
}
