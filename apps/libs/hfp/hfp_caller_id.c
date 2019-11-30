/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_caller_id.c        

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
    HfpCallerIdEnableRequest

DESCRIPTION
    Enable/ disable caller id indications from the AG. An SLC for the supplied
    profile instance (hfp) must already be established before calling this
    function. The enable flag determines whether a command will be sent to
    the AG to enable or disable caller id notifications. The message returned
    indicates whether the command was recognised by the AG or not. When enabled, 
    every caller id notification received from the AG will be sent to the 
    application using a HFP_CALLER_ID_IND message.

MESSAGE RETURNED
    HFP_CALLER_ID_ENABLE_CFM

RETURNS
    void
*/
void HfpCallerIdEnableRequest(hfp_link_priority priority, bool enable)
{
    /* Send an internal message */
    MAKE_HFP_MESSAGE(HFP_INTERNAL_AT_CLIP_REQ);
    message->link = hfpGetLinkFromPriority(priority);
    message->enable = enable;
    MessageSend(&theHfp->task, HFP_INTERNAL_AT_CLIP_REQ, message);
}
