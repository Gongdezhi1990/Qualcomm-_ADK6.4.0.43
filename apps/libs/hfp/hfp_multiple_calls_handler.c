/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_multiple_calls_handler.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_multiple_calls_handler.h"
#include "hfp_call_handler.h"
#include "hfp_parse.h"
#include "hfp_send_data.h"
#include "hfp_slc_handler.h"
#include "hfp_link_manager.h"

#include <panic.h>
#include <stdio.h>
#include <string.h>


/****************************************************************************
NAME    
    hfpCheckCallActionState

DESCRIPTION
    Helper function to check call hold action is valid for the call state of 
    a given link

RETURNS
    void
*/
static bool hfpCheckCallActionState(hfp_link_data* link, hfp_chld_action action, uint16 call_idx, hfp_at_cmd* pending_cmd)
{
    switch(action)
    {
        case hfp_chld_release_held_reject_waiting:
            switch(link->bitfields.ag_call_state)
            {
                case hfp_call_state_twc_incoming:
                case hfp_call_state_twc_outgoing:
                case hfp_call_state_held_active:
                case hfp_call_state_held_remaining:
                    *pending_cmd = hfpChldZeroCmdPending;
                    return TRUE;

                default:
                    break;
            }
            break;

        case hfp_chld_release_active_accept_other:
            switch(link->bitfields.ag_call_state)
            {
                case hfp_call_state_twc_incoming:
                case hfp_call_state_twc_outgoing:
                case hfp_call_state_held_active:
                case hfp_call_state_held_remaining:
                case hfp_call_state_multiparty:
                    *pending_cmd = (call_idx ? hfpChldOneIdxCmdPending : hfpChldOneCmdPending);
                    return TRUE;

                default:
                    break;
            }
            break;

        case hfp_chld_hold_active_accept_other:
            switch(link->bitfields.ag_call_state)
            {
                case hfp_call_state_twc_incoming:
                case hfp_call_state_twc_outgoing:
                case hfp_call_state_held_active:
                case hfp_call_state_held_remaining:
                case hfp_call_state_multiparty:
                case hfp_call_state_active:
                    *pending_cmd = (call_idx ? hfpChldTwoIdxCmdPending : hfpChldTwoCmdPending);
                    return TRUE;

                default:
                    break;
            }
            break;

        case hfp_chld_add_held_to_multiparty:
            switch(link->bitfields.ag_call_state)
            {
                case hfp_call_state_twc_incoming:
                case hfp_call_state_held_active:
                    *pending_cmd = hfpChldThreeCmdPending;
                    return TRUE;

                default:
                    break;
            }
            break;

        case hfp_chld_join_calls_and_hang_up:
            switch(link->bitfields.ag_call_state)
            {
                case hfp_call_state_held_active:
                case hfp_call_state_multiparty:
                    *pending_cmd = hfpChldFourCmdPending;
                    return TRUE;

                default:
                    break;
            }
            break;
        
        default:
            /* Action cannot be valid */
            break;
    }

    return FALSE;
}


/****************************************************************************
NAME    
    hfpSendChldCmd

DESCRIPTION
    Send AT+CHLD=action to the AG. If both AG and HF support ECC then this
    can also be used to send AT+CHLD=action call_idx.

RETURNS
    void
*/
bool hfpSendChldCmd(const HFP_INTERNAL_AT_CHLD_REQ_T* req)
{
    hfp_link_data* link    = req->link;
    hfp_chld_action action = req->action;
    uint16 call_idx        = req->index;
    hfp_at_cmd pending_cmd = hfpCmdPending;
    
    char chld[12];
    
    /* Make sure this link is valid and in a valid state for the requested action */
    if(hfpCheckCallActionState(link, action, call_idx, &pending_cmd))
    {
        /* Check HF and AG both support TWC */
        if (hfFeatureEnabled(HFP_THREE_WAY_CALLING) && agFeatureEnabled(link, AG_THREE_WAY_CALLING))
        {
            if(call_idx)
            {
                /* Check HF and AG both support ECC */
                if(hfFeatureEnabled(HFP_ENHANCED_CALL_CONTROL) && agFeatureEnabled(link, AG_ENHANCED_CALL_CONTROL))
                {
                    /* Send ECC CHLD with index */
                    sprintf(chld, "AT+CHLD=%d%d\r", action, call_idx);
                    hfpSendAtCmd(link, (uint16)strlen(chld), chld, pending_cmd);
                    return TRUE;
                }
            }
            else
            {
                /* Send normal CHLD */
                sprintf(chld, "AT+CHLD=%d\r", action);
                hfpSendAtCmd(link, (uint16)strlen(chld), chld, pending_cmd);
                return TRUE;
            }
        }
    }
    
    return FALSE;
}


/****************************************************************************
NAME    
    hfpHandleChldAtAck

DESCRIPTION
    This function handles the receipt of an OK or ERROR response to a
    multiparty call handling request. The status argument is used to inform 
    the application whether the AT command was recognised by the AG or not. 

RETURNS
    void
*/
void hfpHandleChldAtAck(hfp_link_data* link, hfp_chld_action action, hfp_lib_status status)
{
    /* Handle special cases for CHLD Acks */
    if(action == hfp_chld_add_held_to_multiparty)
    {
        /* Enter multiparty state if AG doesn't support CLCC */
        if(status == hfp_success)
            hfpHandleCallMultiparty(link);
    }
    else if(action == hfp_chld_release_active_accept_other)
    {
        hfp_at_cmd pending_cmd = hfpCmdPending;
        
        /* Try sending AT+CHUP if AT+CHLD=1 failed and we are in TWC state (regardless of TWC support) */
        if(status == hfp_fail && hfpCheckCallActionState(link, action, 0, &pending_cmd))
        {
            /* If we already tried sending AT+CHUP give up */
            if(!link->bitfields.at_chup_retried)
            {
                /* Otherwise attempt sending AT+CHUP */
                link->bitfields.at_chup_retried = hfpHandleTerminateCall(link, pending_cmd);
                /* If we sent AT+CHUP don't send confirmation yet */
                if(link->bitfields.at_chup_retried)
                    return;
            }
        }
        /* Clear the chup retry flag */
        link->bitfields.at_chup_retried = FALSE;
    }
    
#ifndef HFP_MIN_CFM
    {
        MAKE_HFP_MESSAGE(HFP_CALL_HOLD_ACTION_CFM);
        message->priority = hfpGetLinkPriority(link);
        message->action = action;
        message->status = status;
        MessageSend(theHfp->clientTask, HFP_CALL_HOLD_ACTION_CFM, message);
    }
#endif
}


/****************************************************************************
NAME    
    hfpHandleCallWaitingNotificationEnable

DESCRIPTION
    Enable call waiting notifications from the AG.

RETURNS
    void
*/
bool hfpHandleCallWaitingNotificationEnable(const HFP_INTERNAL_AT_CCWA_REQ_T *req)
{
    hfp_link_data* link = req->link;
    
    /* Only send this message if the AG and HF support this functionality */
    if (hfFeatureEnabled(HFP_THREE_WAY_CALLING) && agFeatureEnabled(link, AG_THREE_WAY_CALLING))
    {
        /* Send the AT cmd over the air */
        char ccwa_en[11];
        sprintf(ccwa_en, "AT+CCWA=%d\r", req->enable);
        hfpSendAtCmd(link, sizeof(ccwa_en)-1, ccwa_en, hfpCcwaCmdPending);
        return TRUE;
    }
    return FALSE;
}


/****************************************************************************
NAME    
    sendCallWaitingNotificationToApp

DESCRIPTION
    Send call waiting notification to the application

RETURNS
    void
*/
static void sendCallWaitingNotificationToApp(hfp_link_data* link, uint16 type, unsigned size_number, const uint8 *number, unsigned size_name, const uint8* name)
{
    /* Don't exceed max length */
    if(size_number >= HFP_MAX_ARRAY_LEN)
        size_number = HFP_MAX_ARRAY_LEN - 1;
    
    if(size_name >= (HFP_MAX_ARRAY_LEN - size_number))
        size_name = (HFP_MAX_ARRAY_LEN - size_number - 1);
        
    /* Only pass this info to the app if the HFP supports this functionality */
    if(hfFeatureEnabled(HFP_THREE_WAY_CALLING))
    {
        /* Allow room for NULLs */
        MAKE_HFP_MESSAGE_WITH_LEN(HFP_CALL_WAITING_IND, size_number + size_name + 2);
        message->priority = hfpGetLinkPriority(link);
        message->number_type = hfpConvertNumberType(type);
        /* Set offsets */
        message->offset_number = 0;
        message->offset_name   = (uint16)(size_number + 1);
        /* Don't report NULLs in sizes */
        message->size_number   = (uint16)size_number;
        message->size_name     = (uint16)size_name;
        /* Copy data into the message */
        memmove(message->caller_info + message->offset_number, number, size_number);
        memmove(message->caller_info + message->offset_name,   name,   size_name);
        /* NULL terminate strings */
        message->caller_info[message->offset_number + size_number] = '\0';
        message->caller_info[message->offset_name + size_name]     = '\0';
        
        MessageSend(theHfp->clientTask, HFP_CALL_WAITING_IND, message);
    }
}


/****************************************************************************
NAME    
    hfpHandleCallWaitingNotification

DESCRIPTION
    This function handles the +CCWA notification received from the AG 
    indicating there is an incoming third party call. The profileTask 
    argument specifies which profile instance this indication was sent to.
    The call_notification contains the calling number of the remote party
    that the AG sent in the +CCWA indication. The received information is 
    put into a HFP_CALL_WAITING_IND message and sent to the application.

AT INDICATION
    +CCWA

RETURNS
    void
*/
void hfpHandleCallWaitingNotification(Task link_ptr, const struct hfpHandleCallWaitingNotification *call)
{
    sendCallWaitingNotificationToApp((hfp_link_data*)link_ptr, call->type, call->num.length, call->num.data, 0, NULL);
}


/****************************************************************************
NAME    
    hfpHandleCallWaitingNotificationWithName

DESCRIPTION
    This function handles the +CCWA notification received from the AG 
    indicating there is an incoming third party call. The profileTask 
    argument specifies which profile instance this indication was sent to.
    The call_notification contains the calling number of the remote party
    that the AG sent in the +CCWA indication. The received information is 
    put into a HFP_CALL_WAITING_IND message and sent to the application.

AT INDICATION
    +CCWA

RETURNS
    void
*/
void hfpHandleCallWaitingNotificationWithName(Task link_ptr, const struct hfpHandleCallWaitingNotificationWithName *call)
{
    sendCallWaitingNotificationToApp((hfp_link_data*)link_ptr, call->type, call->num.length, call->num.data, call->name.length, call->name.data);
}


/****************************************************************************
NAME    
    hfpHandleCallWaitingNotificationIllegal

DESCRIPTION
    This function handles the +CCWA notification received from the AG 
    indicating there is an incoming third party call. This function is
    identical to hfpHandleCallWaitingNotification but has been added to handle
    AGs which illegally send the +CCWA indication with fewer arguments
    than specified by the HFP and GSM07.07 specs. Since this is a minor spec
    violation we've got this workaround to handle it.

AT INDICATION
    +CCWA

RETURNS
    void
*/
void hfpHandleCallWaitingNotificationIllegal(Task link_ptr, const struct hfpHandleCallWaitingNotificationIllegal *call)
{
    sendCallWaitingNotificationToApp((hfp_link_data*)link_ptr, 0, call->num.length, call->num.data, 0, NULL);
}
