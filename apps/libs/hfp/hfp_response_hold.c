/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_response_hold.c        

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
    HfpResponseHoldActionRequest

DESCRIPTION
    Requests that the AG performs a given response and hold action.
    
MESSAGE RETURNED
    HFP_RESPONSE_HOLD_ACTION_CFM

RETURNS
    void
*/
void HfpResponseHoldActionRequest(hfp_link_priority priority, hfp_btrh_action action)
{
    MAKE_HFP_MESSAGE(HFP_INTERNAL_AT_BTRH_REQ);
    message->link = hfpGetLinkFromPriority(priority);
    message->action = action;
    MessageSend(&theHfp->task, HFP_INTERNAL_AT_BTRH_REQ, message);
}
