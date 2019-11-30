/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_current_calls.c        

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


/****************************************************************************
NAME    
    HfpCurrentCallsRequest

DESCRIPTION
    Requests list of current calls from the AG. An SLC for the supplied
    profile instance (hfp) must already be established before calling this
    function. Each individual response sent by the AG will be sent to the    
    application using a HFP_CURRENT_CALL_IND message.  If there are no current
    calls the application will not receive any HFP_CURRENT_CALL_IND messages.  
    When the application receives a HFP_CURRENT_CALL_CFM message it will indicate 
    that the request has been completed.

MESSAGE RETURNED
    HFP_CURRENT_CALL_CFM

RETURNS
    void
*/
void HfpCurrentCallsRequest(hfp_link_priority priority)
{
    hfpSendCommonInternalMessagePriority(HFP_INTERNAL_AT_CLCC_REQ, priority);
}
