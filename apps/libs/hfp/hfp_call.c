/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_call.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_link_manager.h"
#include "hfp_common.h"

/****************************************************************************
NAME    
    HfpCallAnswerRequest

DESCRIPTION
    This function is used to answer an incoming call. The AT command to 
    answer the call will be sent out on the SLC corresponding to the 
    priority passed into the function from the application. The message 
    returned indicates whether the command was recognised by the AG or not. 

MESSAGE RETURNED
    HFP_CALL_ANSWER_CFM

RETURNS
    void
*/
void HfpCallAnswerRequest(hfp_link_priority priority, bool accept)
{
    MAKE_HFP_MESSAGE(HFP_INTERNAL_AT_ANSWER_REQ);
    message->link = hfpGetLinkFromPriority(priority);
    message->accept = accept;
    MessageSend(&theHfp->task, HFP_INTERNAL_AT_ANSWER_REQ, message);
}


/****************************************************************************
NAME    
    HfpCallTerminateRequest

DESCRIPTION
    This function is used to reject an incoming call, hang up an active call 
    or terminate an outgoing call process before it has been completed. The 
    AT command will be sent out on the SLC corresponding to the priority
    passed into this function from the application. The message returned 
    indicates whether the command was recognised by the AG or not. 

MESSAGE RETURNED
    HFP_CALL_TERMINATE_CFM

RETURNS
    void
*/
void HfpCallTerminateRequest(hfp_link_priority priority)
{
    hfpSendCommonInternalMessagePriority(HFP_INTERNAL_AT_TERMINATE_REQ, priority);
}
