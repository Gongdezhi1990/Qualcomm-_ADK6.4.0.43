/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_dial.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"

#include <panic.h>
#include <string.h>



/****************************************************************************
NAME    
    HfpDialLastNumberRequest

DESCRIPTION
    This function issues a request to the AG to perform a last number redial.
    The request is issued on the SLC associated with the hfp profile instance 
    passed in by the application. The message returned indicates whether 
    the command was recognised by the AG or not.

MESSAGE RETURNED
    HFP_DIAL_LAST_NUMBER_CFM

RETURNS
    void
*/
void HfpDialLastNumberRequest(hfp_link_priority priority)
{
    /* Send an internal message requesting this action */
    hfpSendCommonInternalMessagePriority(HFP_INTERNAL_AT_BLDN_REQ, priority);
}
