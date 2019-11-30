/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_voice_tag.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"


/****************************************************************************
NAME    
    HfpVoiceTagNumberRequest

DESCRIPTION
    Request a number from the AG to attach to a voice tag. The request is 
    issued on the SLC associated with the hfp profile priority passed in by 
    the application. The message returned indicates whether the command was 
    recognised by the AG or not and the number supplied by the AG (if any).

MESSAGE RETURNED
    HFP_VOICE_TAG_NUMBER_CFM

RETURNS
    void
*/
void HfpVoiceTagNumberRequest(hfp_link_priority priority)
{
    hfpSendCommonInternalMessagePriority(HFP_INTERNAL_AT_BINP_REQ, priority);
}
