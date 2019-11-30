/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_multiple_calls.c

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


void HfpCallHoldActionRequest(hfp_link_priority priority, hfp_chld_action action, uint16 index)
{
    MAKE_HFP_MESSAGE(HFP_INTERNAL_AT_CHLD_REQ);
    message->link = hfpGetLinkFromPriority(priority);
    message->action = action;
    message->index = index;
    MessageSend(&theHfp->task, HFP_INTERNAL_AT_CHLD_REQ, message);
}
