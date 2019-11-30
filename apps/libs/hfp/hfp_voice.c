/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_voice.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_link_manager.h"

#include <panic.h>


/****************************************************************************
NAME    
    HfpVoiceRecognitionEnableRequest

DESCRIPTION
    Enable / disable voice recognition function at the AG. An SLC for the 
    supplied profile instance (hfp) must already be established before calling 
    this function. The enable flag determines whether a request will be made 
    to the AG to enabe or disable its voice recognition function. The message 
    returned indicates whether the command was recognised by the AG or not. 
    
    The AG may autonomously notify the HFP device of a change in the state of
    its voice recognition function. This notification will be passed on to the 
    application using a HFP_VOICE_RECOGNITION_IND message.

MESSAGE RETURNED
    HFP_VOICE_RECOGNITION_ENABLE_CFM

RETURNS
    void
*/
void HfpVoiceRecognitionEnableRequest(hfp_link_priority priority, bool enable)
{
    /* Send an internal message to kick off SLC creation */
    MAKE_HFP_MESSAGE(HFP_INTERNAL_AT_BVRA_REQ);
    message->link = hfpGetLinkFromPriority(priority);
    message->enable = (uint16)enable;
    MessageSend(&theHfp->task, HFP_INTERNAL_AT_BVRA_REQ, message);
}

/****************************************************************************
NAME    
    HfpVoiceRecognitionIsSupported

DESCRIPTION
    Check if voice recognition is support on the given hfp link.

    Both the HF and the AG must support voice recognition for this function
    to return TRUE.

RETURNS
    bool TRUE if voice recognition supported, FALSE otherwise.
*/
bool HfpVoiceRecognitionIsSupported(hfp_link_priority priority)
{
    hfp_link_data *link = hfpGetLinkFromPriority(priority);
    
    return (hfFeatureEnabled(HFP_VOICE_RECOGNITION) && agFeatureEnabled(link, AG_VOICE_RECOGNITION));
}
