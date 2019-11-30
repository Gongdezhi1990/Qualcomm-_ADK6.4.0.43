/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_subscriber_num.c        

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
    HfpSubscriberNumbersRequest

DESCRIPTION
    Requests subscriber number information from the AG. An SLC for the supplied
    profile priority must already be established before calling this
    function. Each individual subscriber number received from the AG will be
    sent to the application using a HFP_SUBSCRIBER_NUMBER_IND message. If no
    subscriber number information is available the application will not receive
    any HFP_SUBSCRIBER_NUMBER_IND messages.  When the application receives a 
    HFP_SUBSCRIBER_NUMBERS_CFM message it will indicate that the request has been
    completed.

MESSAGE RETURNED
    HFP_SUBSCRIBER_NUMBERS_CFM

RETURNS
    void
*/
void HfpSubscriberNumbersRequest(hfp_link_priority priority)
{
    /* Send an internal message */
    hfpSendCommonInternalMessagePriority(HFP_INTERNAL_AT_CNUM_REQ, priority);
}
