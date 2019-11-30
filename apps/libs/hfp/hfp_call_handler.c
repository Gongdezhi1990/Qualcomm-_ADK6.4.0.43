/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_call_handler.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_call_handler.h"
#include "hfp_common.h"
#include "hfp_indicators_handler.h"
#include "hfp_send_data.h"
#include "hfp_link_manager.h"
#include "hfp_hs_handler.h"
#include "hfp_current_calls_handler.h"
#include "hfp_multiple_calls_handler.h"

#include <panic.h>
#include <string.h>


/****************************************************************************
NAME    
    hfpHandleCallRingIndication

DESCRIPTION
    Handle any call state changes required when RING is received on link

RETURNS
    void
*/
void hfpHandleCallRingIndication(hfp_link_data* link)
{
    /* This is necessary when dealing with 0.96 AGs or HSP */
    if (!link->ag_supported_indicators.indicator_idxs.call_setup)
    {
        /* Enter incoming state if not there already */
        hfpSetLinkCallState(link, hfp_call_state_incoming);
        
        /* If phone is HSP */
        if(hfpLinkIsHsp(link))
        {
            /* Cancel and re-send any pending incoming timeout message */
            MessageId id = hfpGetLinkTimeoutMessage(link, HFP_INTERNAL_HS_INCOMING_TIMEOUT_LINK_0_IND);
            (void)MessageCancelAll(&theHfp->task, id);
            MessageSendLater(&theHfp->task, id, 0, D_SEC(5));
        }
    }
}


/****************************************************************************
NAME    
    hfpHandleCallDialComplete

DESCRIPTION
    Handle any call state changes required when dial functions complete

RETURNS
    void
*/
void hfpHandleCallDialComplete(hfp_link_data* link, hfp_lib_status status)
{
    /* If this AG doesn't support call setup indications */
    if(!link->ag_supported_indicators.indicator_idxs.call_setup)
    {
        /* If it's not an HSP AG and the dial command was successful */
        if(hfpLinkIsHfp(link) && status == hfp_success)
        {
            /* We must have an outgoing call */
            hfpSetLinkCallState(link, hfp_call_state_outgoing);
        }
    }
}


/****************************************************************************
NAME    
    hfpHandleCallVoiceRecDisabled

DESCRIPTION
    Handle any call state changes required when voice recognition disabled

RETURNS
    void
*/
void hfpHandleCallVoiceRecDisabled(hfp_link_data* link)
{
    /* If this AG doesn't support call setup indications */
    if(!link->ag_supported_indicators.indicator_idxs.call_setup)
    {
        /* If it's not an HSP AG and we don't have audio */
        if(hfpLinkIsHfp(link) && !link->audio_sink)
        {
            /* Have to manually set state back to idle */
            hfpSetLinkCallState(link, hfp_call_state_idle);
        }
    }
}


/****************************************************************************
NAME    
    hfpHandleCallAudio

DESCRIPTION
    Handle any call state changes required when audio is connected on link

RETURNS
    void
*/
void hfpHandleCallAudio(hfp_link_data* link, bool audio)
{
    /* If this is an HSP AG then change state on SCO */
    if (hfpLinkIsHsp(link))
    {
        /* Cancel any pending incoming timeout */
        MessageId id = hfpGetLinkTimeoutMessage(link, HFP_INTERNAL_HS_INCOMING_TIMEOUT_LINK_0_IND);
        (void)MessageCancelAll(&theHfp->task, id);
        
        if(audio)
        {
            /* 
               Rules on ring indications are either ignored or broken
               so we can only safely assume we have an incoming call. 
               Enter incoming call state and schedule entering active
               call state in 5 seconds if we still have audio. 
            */
            MessageSendLater(&theHfp->task, id, 0, D_SEC(5));
            
            if(link->bitfields.ag_call_state == hfp_call_state_idle)
                hfpSetLinkCallState(link, hfp_call_state_incoming);
        }
        else
        {
            /* No audio so must be no call */
            hfpSetLinkCallState(link, hfp_call_state_idle);
        }
    }
}


/****************************************************************************
NAME    
    hfpHandleCallIncomingTimeout

DESCRIPTION
    Handle HFP_INTERNAL_HS_INCOMING_TIMEOUT_LINK_0/1_IND

RETURNS
    void
*/
void hfpHandleCallIncomingTimeout(hfp_link_data* link)
{
    if(hfpLinkIsHsp(link) && link->bitfields.ag_call_state == hfp_call_state_incoming)
    {
        /* Still have audio after 5 seconds, assume active */
        if(link->bitfields.audio_state == hfp_audio_connected)
            hfpSetLinkCallState(link, hfp_call_state_active);
        else
            hfpSetLinkCallState(link, hfp_call_state_idle);
    }
}


/****************************************************************************
NAME    
    hfpHandleCallIndication

DESCRIPTION
    Handle a call notification on link

RETURNS
    void
*/
void hfpHandleCallIndication(hfp_link_data* link, hfp_call call)
{
    if (call)
    {
        switch(link->bitfields.ag_call_state)
        {
            case hfp_call_state_idle:
            case hfp_call_state_incoming:
            case hfp_call_state_outgoing:
                /* Valid to go to active call state */
                hfpSetLinkCallState(link, hfp_call_state_active);
            break;
            default:
                /* Ignore in all other states */
            break;
        }
    }
    else
    {
        switch(link->bitfields.ag_call_state)
        {
            case hfp_call_state_idle:
                /* Idle already so stay there; However, still
                explicitly set the call state in order to inform
                the app of the latest state (which the vm may
                not be aware of, e.g. if a call disconnected
                during linkloss, the vm wouldn't know) */
                hfpSendLinkCallStateNotification(link, hfp_call_state_idle);
            break;
            case hfp_call_state_twc_incoming:
                /* Drop back to incoming call state */
                hfpSetLinkCallState(link, hfp_call_state_incoming);
            break;
            case hfp_call_state_twc_outgoing:
                /* Drop back to outgoing call state */
                hfpSetLinkCallState(link, hfp_call_state_outgoing);
            break;
            default:
                /* In all other cases, drop back to idle state */
                hfpSetLinkCallState(link, hfp_call_state_idle);
            break;
        }
    }
}


/****************************************************************************
NAME    
    hfpHandleCallSetupIndication

DESCRIPTION
    Handle a call setup notification on link

RETURNS
    void
*/
void hfpHandleCallSetupIndication(hfp_link_data* link, hfp_call_setup call_setup)
{
    switch(call_setup)
    {
        case hfp_no_call_setup:
            switch(link->bitfields.ag_call_state)
            {
                case hfp_call_state_incoming:
                case hfp_call_state_outgoing:
                    /* Covered by no call indication, but as a backup */
                    hfpSetLinkCallState(link, hfp_call_state_idle);
                break;
                case hfp_call_state_twc_incoming:
                    /* Try for CLCC to work out what happened to the incoming call */
                    hfpSendCommonInternalMessage(HFP_INTERNAL_AT_CLCC_REQ, link);
                    
                    /* Incoming TWC no longer there so assume it ended active for now */
                    hfpSetLinkCallState(link, hfp_call_state_active);
                break;
                case hfp_call_state_twc_outgoing:
                    /* In outgoing TWC call was already held, go to held call */
                    hfpSetLinkCallState(link, hfp_call_state_held_active);
                break;
                default:
                    /* Ignore */
                break;
            }
        break;
        case hfp_incoming_call_setup:
            switch(link->bitfields.ag_call_state)
            {
                case hfp_call_state_idle:
                    /* We have an incoming call */
                    hfpSetLinkCallState(link, hfp_call_state_incoming);
                break;
                case hfp_call_state_active:
                case hfp_call_state_held_active:
                case hfp_call_state_held_remaining:
                case hfp_call_state_multiparty:
                    /* We have a second incoming call */
                    hfpSetLinkCallState(link, hfp_call_state_twc_incoming);
                break;
                default:
                    /* Ignore */
                break;
            }
        break;
        case hfp_outgoing_call_setup:
        case hfp_outgoing_call_alerting_setup :
            switch(link->bitfields.ag_call_state)
            {
                case hfp_call_state_idle:
                    /* We have an outgoing call */
                    hfpSetLinkCallState(link, hfp_call_state_outgoing);
                break;
                case hfp_call_state_active:
                case hfp_call_state_held_active:
                case hfp_call_state_held_remaining:
                    /* We have a second outgoing call */
                    hfpSetLinkCallState(link, hfp_call_state_twc_outgoing);
                break;
                default:
                    /* Ignore */
                break;
            }
        break;
        default:
            /* Error */
        break;
    }
}


/****************************************************************************
NAME    
    hfpHandleCallHeldIndication

DESCRIPTION
    Handle a held call notification on link

RETURNS
    void
*/
void hfpHandleCallHeldIndication(hfp_link_data* link, hfp_call_hold call_hold)
{
    switch(call_hold)
    {
        case hfp_no_held_call:
            switch(link->bitfields.ag_call_state)
            {
                case hfp_call_state_twc_outgoing:
                    /* When outgoing TWC starts the active call is put
                       on hold. If held call has ended then we have an
                       outgoing call left... */
                    hfpSetLinkCallState(link, hfp_call_state_outgoing);
                break;
                case hfp_call_state_held_remaining:
                    /* Remaining call unheld, assume active and call
                       none indication will take us back to idle */
                    hfpSetLinkCallState(link, hfp_call_state_active);
                break;
                case hfp_call_state_held_active:
                    /* Try for CLCC to work out what happened to the held call */
                    hfpSendCommonInternalMessage(HFP_INTERNAL_AT_CLCC_REQ, link);
                    
                    /* Held call no longer there so assume it was ended for now */
                    hfpSetLinkCallState(link, hfp_call_state_active);
                break;
                default:
                    /* Ignore */
                break;
            }
        break;
        case hfp_held_active:
            switch(link->bitfields.ag_call_state)
            {
                case hfp_call_state_active:
                case hfp_call_state_twc_incoming:
                case hfp_call_state_held_remaining:
                case hfp_call_state_multiparty:
                    /* Enter active + held state */
                    hfpSetLinkCallState(link, hfp_call_state_held_active);
                break;
                default:
                    /* Ignore */
                break;
            }
        break;
        case hfp_held_remaining:
            switch(link->bitfields.ag_call_state)
            {
                case hfp_call_state_active:
                case hfp_call_state_twc_outgoing:
                case hfp_call_state_held_active:
                case hfp_call_state_multiparty:
                    hfpSetLinkCallState(link, hfp_call_state_held_remaining);
                break;
                default:
                    /* Ignore */
                break;
            }
        break;
        default:
            /* Error */
        break;
    }
}


/****************************************************************************
NAME    
    hfpHandleCallCurrentIndication

DESCRIPTION
    Handle a current calls notification on link (+CLCC) - this tells us if 
    we have a multiparty call or not.

RETURNS
    void
*/
void hfpHandleCallCurrentIndication(hfp_link_data* link, hfp_call_multiparty multiparty)
{
    if(multiparty == hfp_multiparty_call && link->bitfields.ag_call_state != hfp_call_state_multiparty)
    {
        hfpSetLinkCallState(link, hfp_call_state_multiparty);
    }
}


/****************************************************************************
NAME    
    hfpHandleCallResponseHoldIndication

DESCRIPTION
    Handle a response and hold notification on link (+BTRH)

RETURNS
    void
*/
void hfpHandleCallResponseHoldIndication(hfp_link_data* link, hfp_response_hold_state state)
{
    switch(state)
    {
        case hfp_incoming_call_held:
            if(link->bitfields.ag_call_state == hfp_call_state_incoming)
            {
                hfpSetLinkCallState(link, hfp_call_state_incoming_held);
            }
        break;
        case hfp_held_call_accepted:
            if(link->bitfields.ag_call_state == hfp_call_state_incoming_held)
            {
                hfpSetLinkCallState(link, hfp_call_state_active);
            }
        break;
        case hfp_held_call_rejected:
            if(link->bitfields.ag_call_state == hfp_call_state_incoming_held)
            {
                hfpSetLinkCallState(link, hfp_call_state_idle);
            }
        break;
        default:
            /* Ignore */
        break;
    }
}


/****************************************************************************
NAME    
    hfpHandleCallMultiparty

DESCRIPTION
    If the link doesn't support CLCC then we can only guess at multiparty
    state based on the headset sending AT+CHLD=3. 

RETURNS
    void
*/
void hfpHandleCallMultiparty(hfp_link_data* link)
{
    /* If either HF or AG has enhanced call status disabled */
    if(!hfFeatureEnabled(HFP_ENHANCED_CALL_STATUS) || !agFeatureEnabled(link, AG_ENHANCED_CALL_STATUS))
    {
        /* Enter multiparty state */
        hfpHandleCallCurrentIndication(link, hfp_multiparty_call);
    }
}


/****************************************************************************
NAME    
    hfpHandleAnswerCall

DESCRIPTION
    Answer an incoming call.

RETURNS
    void
*/
bool hfpHandleAnswerCall(const HFP_INTERNAL_AT_ANSWER_REQ_T* req)
{
    hfp_link_data* link = req->link;
    bool accept         = req->accept;
    
    if(link->bitfields.ag_call_state == hfp_call_state_incoming)
    {
        /* Send the AT cmd over the air */
        const char* cmd = accept ? "ATA\r" : "AT+CHUP\r";
        hfpSendAtCmd(link, (uint16)strlen(cmd), cmd, hfpAtaCmdPending);
        return TRUE;
    }
    
    return FALSE;
}


/****************************************************************************
NAME    
    hfpHandleAnswerHspCall

DESCRIPTION
    Answer an incoming call from HSP AG.

RETURNS
    void
*/
bool hfpHandleAnswerHspCall(const HFP_INTERNAL_AT_ANSWER_REQ_T* req)
{
    hfp_link_data* link = req->link;
    
    if(link->bitfields.ag_call_state == hfp_call_state_incoming)
    {
        /* Cancel any pending incoming timeout */
        MessageId id = hfpGetLinkTimeoutMessage(link, HFP_INTERNAL_HS_INCOMING_TIMEOUT_LINK_0_IND);
        (void)MessageCancelAll(&theHfp->task, id);
        /* Call has been answered - enter active call state now */
        hfpSetLinkCallState(link, hfp_call_state_active);
        /* Send button press */
        hfpSendHsButtonPress(link, hfpAtaCmdPending);
        return TRUE;
    }
    return FALSE;
}


/****************************************************************************
NAME    
    hfpHandleTerminateCall

DESCRIPTION
    Terminate an ongoing call process.

RETURNS
    void
*/
bool hfpHandleTerminateCall(hfp_link_data* link, hfp_at_cmd pending_cmd)
{
    if(link->bitfields.ag_call_state > hfp_call_state_incoming)
    {
        /* Send the AT cmd over the air */
        char chup[] = "AT+CHUP\r";
        hfpSendAtCmd(link, (uint16)strlen(chup), chup, pending_cmd);
        return TRUE;
    }
    return FALSE;
}


/****************************************************************************
NAME    
    hfpHandleTerminateHspCall

DESCRIPTION
    Terminate an ongoing call process.

RETURNS
    void
*/
bool hfpHandleTerminateHspCall(hfp_link_data* link)
{
    if(link->bitfields.ag_call_state > hfp_call_state_incoming)
    {
        hfpSendHsButtonPress(link, hfpChupCmdPending);
        return TRUE;
    }
    return FALSE;
}


#ifndef HFP_MIN_CFM
/****************************************************************************
NAME    
    hfpHandleChupAtAck

DESCRIPTION
    Work out the confirmation message to send on receiving ACK from AT+CHUP

RETURNS
    void
*/
MessageId hfpHandleChupAtAck(hfp_link_data* link)
{
    UNUSED(link);

    return HFP_CALL_TERMINATE_CFM;
}
#endif
