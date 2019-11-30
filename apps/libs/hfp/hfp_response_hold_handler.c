/****************************************************************************
Copyright (c) 2005 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_response_hold_handler.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_parse.h"
#include "hfp_response_hold_handler.h"
#include "hfp_common.h"
#include "hfp_send_data.h"
#include "hfp_link_manager.h"
#include "hfp_call_handler.h"

#include <panic.h>
#include <string.h>
#include <stdio.h>

/****************************************************************************
NAME    
    hfpHandleBtrhStatusReq

DESCRIPTION
    Request response hold status from the AG.

RETURNS
    void
*/
void hfpHandleBtrhStatusReq(hfp_link_data* link)
{
    /* Send the AT cmd over the air */
    char btrh[] = "AT+BTRH?\r";
    hfpSendAtCmd(link, sizeof(btrh)-1, btrh, hfpBtrhStatusCmdPending);
}


/****************************************************************************
NAME    
    hfpHandleBtrhStatusAtAck

DESCRIPTION
    Request to AG to get response and hold status has been acknowledged 
    (completed) by the AG.

RETURNS
    void
*/
void hfpHandleBtrhStatusAtAck(hfp_link_data* link)
{
    /* Request current calls information if supported */
    hfpSendCommonInternalMessage(HFP_INTERNAL_AT_CLCC_REQ, link);
    /* Turn off optional indicators the application doesn't want to know about */
    hfpSendCommonInternalMessage(HFP_INTERNAL_AT_BIA_REQ, link);
}


/****************************************************************************
NAME    
    hfpHandleBtrhHoldReq

DESCRIPTION
    Request to AG to perform a response and hold action.

RETURNS
    void
*/
bool hfpHandleBtrhReq(const HFP_INTERNAL_AT_BTRH_REQ_T* req)
{
    hfp_link_data*  link   = req->link;
    hfp_btrh_action action = req->action;
    bool action_is_valid   = FALSE;
    hfp_at_cmd pending_cmd = hfpCmdPending;
    
    /* Check action is valid for link */
    switch(action)
    {
        case hfp_hold_incoming_call:
            if(link->bitfields.ag_call_state == hfp_call_state_incoming)
            {
                pending_cmd = hfpBtrhZeroCmdPending;
                action_is_valid = TRUE;
            }
        break;
        case hfp_accept_held_incoming_call:
            if(link->bitfields.ag_call_state == hfp_call_state_incoming_held)
            {
                pending_cmd = hfpBtrhOneCmdPending;
                action_is_valid = TRUE;
            }
        break;
        case hfp_reject_held_incoming_call:
            if(link->bitfields.ag_call_state == hfp_call_state_incoming_held)
            {
                pending_cmd = hfpBtrhTwoCmdPending;
                action_is_valid = TRUE;
            }
        break;
        default:
        break;
    }
    
    if(action_is_valid)
    {
        /* Send the AT command over the air */
        char btrh[11];
        sprintf(btrh, "AT+BTRH=%d\r", action);
        hfpSendAtCmd(link, sizeof(btrh)-1, btrh, pending_cmd);
    }

    return action_is_valid;
}


#ifndef HFP_MIN_CFM
/****************************************************************************
NAME    
    hfpHandleBtrhAtAck

DESCRIPTION
    Request to AG to perform a response and hold action has been acknowledged 
    (completed) by the AG.

RETURNS
    void
*/
void hfpHandleBtrhAtAck(hfp_link_data* link, hfp_btrh_action action, hfp_lib_status status)
{
    MAKE_HFP_MESSAGE(HFP_RESPONSE_HOLD_ACTION_CFM);
    message->priority = hfpGetLinkPriority(link);
    message->action   = action;
    message->status   = status;
    MessageSend(theHfp->clientTask, HFP_RESPONSE_HOLD_ACTION_CFM, message);
}
#endif


/****************************************************************************
NAME    
    hfpHandleResponseHold

DESCRIPTION
    Response hold indication received from the AG

AT INDICATION
    +BTRH

RETURNS
    void
*/
void hfpHandleResponseHold(Task link_ptr, const struct hfpHandleResponseHold *ind)
{
    hfp_link_data* link = (hfp_link_data*)link_ptr;
    
    /* Silently ignore AT command if we are not HFP v.15 */
    if(hfpLinkIsHfp(link))
    {
        /* Tell the call handler */
        hfpHandleCallResponseHoldIndication(link, (hfp_response_hold_state)ind->state);
    }
}
