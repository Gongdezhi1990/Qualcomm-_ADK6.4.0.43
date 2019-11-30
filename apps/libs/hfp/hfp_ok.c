/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_ok.c        

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_ok.h"
#include "hfp_parse.h"
#include "hfp_send_data.h"
#include "hfp_slc_handler.h"
#include "hfp_indicators_handler.h"
#include "hfp_call_handler.h"
#include "hfp_current_calls_handler.h"
#include "hfp_multiple_calls_handler.h"
#include "hfp_extended_error_handler.h"
#include "hfp_response_hold_handler.h"
#include "hfp_link_manager.h"
#include "hfp_wbs_handler.h"
#include "hfp_dial_handler.h"
#include "hfp_voice_handler.h"
#include "hfp_nrec_handler.h"
#include "hfp_response_hold_handler.h"
#include "hfp_voice_tag_handler.h"
#include "hfp_subscriber_num_handler.h"
#include "hfp_dtmf_handler.h"
#include "hfp_network_operator_handler.h"
#include "hfp_csr_features.h"
#include "hfp_caller_id_handler.h"
#include "hfp_hs_handler.h"
#include "hfp_qce_handler.h"

#include <panic.h>
#include <string.h>
#include <sink.h>
#include <print.h>


#define HFP_CME_MIN_ERROR_CODE 0
#define HFP_CME_MAX_ERROR_CODE 32

static const hfp_lib_status extendedStatusCode[ HFP_CME_MAX_ERROR_CODE - HFP_CME_MIN_ERROR_CODE + 1 ] =
{
    hfp_ag_failure,                     /* CME ERROR: 0  */
    hfp_no_connection_to_phone,         /* CME ERROR: 1  */
    hfp_fail,          
    hfp_operation_not_allowed,          /* CME ERROR: 3  */
    hfp_operation_not_supported,        /* CME ERROR: 4  */
    hfp_ph_sim_pin_required,            /* CME ERROR: 5  */
    hfp_fail,          
    hfp_fail,          
    hfp_fail,          
    hfp_fail,          
    hfp_sim_not_inserted,               /* CME ERROR: 10 */
    hfp_sim_pin_required,               /* CME ERROR: 11 */
    hfp_sim_puk_required,               /* CME ERROR: 12 */
    hfp_sim_failure,                    /* CME ERROR: 13 */
    hfp_sim_busy,                       /* CME ERROR: 14 */
    hfp_fail,          
    hfp_incorrect_password,             /* CME ERROR: 16 */
    hfp_sim_pin2_required,              /* CME ERROR: 17 */
    hfp_sim_puk2_required,              /* CME ERROR: 18 */
    hfp_fail,          
    hfp_memory_full,                    /* CME ERROR: 20 */
    hfp_invalid_index,                  /* CME ERROR: 21 */
    hfp_fail,                                 
    hfp_memory_failure,                 /* CME ERROR: 23 */
    hfp_text_string_too_long,           /* CME ERROR: 24 */
    hfp_invalid_chars_in_text_string,   /* CME ERROR: 25 */
    hfp_dial_string_too_long,           /* CME ERROR: 26 */
    hfp_invalid_chars_in_dial_string,   /* CME ERROR: 27 */
    hfp_fail,          
    hfp_fail,          
    hfp_no_network_service,             /* CME ERROR: 30 */
    hfp_network_timeout,                /* CME ERROR: 31 */
    hfp_network_not_allowed             /* CME ERROR: 32 */
};

/* Convert supplied CME ERROR code to internal HFP library status code. */
/* Unrecongnised codes are mapped to hfp_fail.                          */
static hfp_lib_status convertErrorCode ( uint16 CmeErrorCode )
{
    if ( (CmeErrorCode >= HFP_CME_MIN_ERROR_CODE) && (CmeErrorCode <= HFP_CME_MAX_ERROR_CODE) )
    {
        return extendedStatusCode[ CmeErrorCode ];
    }
    
    return hfp_fail;
}


/* Check which AT cmd was waiting for this response */
static void atCmdAckReceived(hfp_link_data* link, hfp_lib_status status)
{
    Sink sink = hfpGetLinkSink(link);
    MessageId cfm_id = HFP_NO_CFM;
    
    /* Cancel the AT response timeout message */
    (void) MessageCancelAll(&theHfp->task, hfpGetLinkTimeoutMessage(link, HFP_INTERNAL_WAIT_AT_TIMEOUT_LINK_0_IND));

    /* Which message was waiting for this? */
    switch (link->bitfields.at_cmd_resp_pending)
    {
    case hfpCmdPending:
        /* This is a cmd which we don't particularly care about, as long as we get the ack */
        break;

    case hfpBrsfCmdPending:
        /* Have received response to AT+BRSF */
        hfpHandleBrsfAtAck(link, status);
        break;

    case hfpQacCmdPending:
        /* Have received response to AT+QAC */
        hfpHandleQacAtAck(link, status);
        break;

    case hfpQcsCmdPending:
        /* Have received response to AT+%QCS */
        hfpHandleQcsAtAck(link, status);
        break;

    case hfpBacCmdPending:
        /* Have received response to AT+BAC */
        hfpHandleBacAtAck(link, status);
        break;
        
    case hfpCindTestCmdPending:
        /* Have received response to AT+CIND=? */
        hfpHandleCindTestAtAck(link, status);
        break;
    
    case hfpCindReadCmdPending:
        /* Have received response to AT+CIND? */
        hfpHandleCindReadAtAck(link, status);
        break;
        
    case hfpCmerCmdPending:
        /* Have received response to AT+CMER */
        hfpHandleCmerAtAck(link, status);
        break;
    
    case hfpChldStatusCmdPending:
        /* Have received response to AT+CHLD=? */
        (void)hfpSlcCheckAtAck(link, status);
        break;
        
    case hfpBccCmdPending:
        /* Have received a response to AT+BCC */
        hfpHandleBccAtAck(link, status);
        break;
    
    case hfpBcsCmdPending:
        /* Have received a response to AT+BCS */
        hfpHandleBcsAtAck(link, status);
        break;
        
    case hfpBldnCmdPending:
        /* Have received a response to AT+BLDN */
        cfm_id = HFP_DIAL_LAST_NUMBER_CFM;
        /* This is necessary when dealing with 0.96 AGs */
        hfpHandleCallDialComplete(link, status);
        break;
    
    case hfpAtdNumberCmdPending:
        /* Have received a response to ATD */
        cfm_id = HFP_DIAL_NUMBER_CFM;
        /* This is necessary when dealing with 0.96 AGs */
        hfpHandleCallDialComplete(link, status);
        break;
    
    case hfpAtdMemoryCmdPending:
        /* Have received a response to ATD> */
        cfm_id = HFP_DIAL_MEMORY_CFM;
        /* This is necessary when dealing with 0.96 AGs */
        hfpHandleCallDialComplete(link, status);
        break;
        
    case hfpBvraCmdPending:
        /* Have received a response to AT+BVRA */
        cfm_id = HFP_VOICE_RECOGNITION_ENABLE_CFM;
        /* This is necessary when dealing with 0.96 AGs */
        hfpHandleCallDialComplete(link, status);
        break;
    
    case hfpNrecCmdPending:
        /* Have received a response to AT+NREC */
        hfpHandleNrecAtAck(link);
        break;
    
    case hfpBiaCmdPending:
        /* Have received a response to AT+BIA */
        hfpHandleBiaAtAck(link);
        break;
    
    case hfpCopsFormatCmdPending:
        /* Have received a response to AT+COPS= */
        cfm_id = hfpHandleCopsFormatAtAck(link, status);
        break;
        
    case hfpChldOneCmdPending:
        /* Have received response to AT+CHLD=1 */
        hfpHandleChldAtAck(link, hfp_chld_release_active_accept_other, status);
        break;
        
    case hfpChldThreeCmdPending:
        /* Have received response to AT+CHLD=3 */
        hfpHandleChldAtAck(link, hfp_chld_add_held_to_multiparty, status);
        break;
        
    case hfpBtrhStatusCmdPending:
        /* Have received a response to AT+BTRH? */
        hfpHandleBtrhStatusAtAck(link);
        break;

    case hfpBindCmdPending:
	    /* Have received a response to AT+BIND= */
	    hfpHandlerBindAtAck(link, status);
	    break;

    case hfpBindStatusCmdPending:
	    /* Have received a response to AT+BIND=?*/
	    hfpHandlerBindReadAtAck(link, status);
	    break;

    case hfpBindIndStatusCmdPending:
	    /* Have received response to AT+BIND? */
        hfpHandlerBindIndStatusAtAck(link, status);
	    break;

    case hfpBievStatusCmdPending:
        /* Have received response to AT+BIEV */
        (void)hfpSlcCheckAtAck(link, status);		
	    break;
    
    case hfpCsrSfPending:
        /* Have received a response to AT+CSRSF */
        hfpCsrFeaturesHandleCsrSfAtAck(link, status);
        break;
        
    case hfpCsrGetSmsPending:
        /* Have received a response to AT+CSRGETSMS */
        hfpCsrFeaturesHandleGetSmsAtAck(link, status);
        break;

    case hfpCmeeCmdPending:
        hfpHandleCmeeAtAck(link);
        break;

#ifndef HFP_MIN_CFM
    case hfpClipCmdPending:
        /* Have received a response to AT+CLIP= */
        cfm_id = HFP_CALLER_ID_ENABLE_CFM;
        break;

    case hfpAtaCmdPending:
        /* Have received response to ATA */
        cfm_id = HFP_CALL_ANSWER_CFM;
        break;

    case hfpChupCmdPending:
        /* Have received response to AT+CHUP */
        cfm_id = hfpHandleChupAtAck(link);
        break;
        
    case hfpCcwaCmdPending:
        /* Have received response to AT+CCWA */
        cfm_id = HFP_CALL_WAITING_ENABLE_CFM;
        break;
    
    case hfpBtrhZeroCmdPending:
        /* Have received a response to AT+BTRH=0 */
        hfpHandleBtrhAtAck(link, hfp_incoming_call_held, status);
        break;
    
    case hfpBtrhOneCmdPending:
        /* Have received a response to AT+BTRH=1 */
        hfpHandleBtrhAtAck(link, hfp_held_call_accepted, status);
        break;
    
    case hfpBtrhTwoCmdPending:
        /* Have received a response to AT+BTRH=2 */
        hfpHandleBtrhAtAck(link, hfp_held_call_rejected, status);
        break;
        
    case hfpChldZeroCmdPending:
        /* Have received response to AT+CHLD=0 */
        hfpHandleChldAtAck(link, hfp_chld_release_held_reject_waiting, status);
        break;
        
    case hfpChldOneIdxCmdPending:
        /* Have received response to AT+CHLD=1x */
        hfpHandleChldAtAck(link, hfp_chld_release_active_accept_other, status);
        break;
        
    case hfpChldTwoCmdPending:
        /* Have received response to AT+CHLD=2 */
        hfpHandleChldAtAck(link, hfp_chld_hold_active_accept_other, status);
        break;
        
    case hfpChldTwoIdxCmdPending:
        /* Have received response to AT+CHLD=2x */
        hfpHandleChldAtAck(link, hfp_chld_hold_active_accept_other, status);
        break;
        
    /* Always need to handle multiparty ack */
        
    case hfpChldFourCmdPending:
        /* Have received response to AT+CHLD=4 */
        hfpHandleChldAtAck(link, hfp_chld_join_calls_and_hang_up, status);
        break;
        
    case hfpCkpdCmdPending:
        /* Have received a response to AT+CKPD=200 */
        cfm_id = HFP_HS_BUTTON_PRESS_CFM;
        break;
    
    case hfpVtsCmdPending:
        /* Have received a response to AT+VTS */
        cfm_id = HFP_DTMF_CFM;
        break;
    
    case hfpCnumCmdPending:
        /* Have received a response to AT+CNUM */
        cfm_id = HFP_SUBSCRIBER_NUMBERS_CFM;
        break;
    
    case hfpClccCmdPending:
        /* Have received a response to AT+CLCC */
        cfm_id = HFP_CURRENT_CALLS_CFM;
        break;
    
    case hfpCopsReqCmdPending:
        /* Have received a response to AT+COPS? */
        cfm_id = HFP_NETWORK_OPERATOR_CFM;
        break;
    
    case hfpBinpCmdPending:
        /* Have received a response to AT+BINP */
        cfm_id = HFP_VOICE_TAG_NUMBER_CFM;
        break;
        
    case hfpAtCmdPending:
        cfm_id = HFP_AT_CMD_CFM;
        break;
#endif
    
    case hfpNoCmdPending:
    default:
        /* This should not happen, if we get an ack we should be waiting for it! */
        break;
    }

    /* Send _CFM message to app if necessary */
    if(cfm_id) 
        hfpSendCommonCfmMessageToApp(cfm_id, link, status);

    /* Reset the flag as we've just received a response */ 
    link->bitfields.at_cmd_resp_pending = hfpNoCmdPending;

    PRINT(("\n"));

    /* Try to send the next AT cmd pending */
    hfpSendNextAtCmd(link, SinkClaim(sink, 0), SinkMap(sink));
}


/****************************************************************************
NAME    
    hfpHandleOk

DESCRIPTION
    Handle the AT OK indication sent by the AG.

AT INDICATION
    OK

RETURNS
    void
*/
void hfpHandleOk(Task link_ptr)
{
    /* Handle this response in the cotext of the AT cmd it relates to */
    atCmdAckReceived((hfp_link_data*)link_ptr, hfp_success);
}


/****************************************************************************
NAME    
    hfpHandleError

DESCRIPTION
    Handle the AT ERROR indication sent by the AG.

AT INDICATION
    ERROR

RETURNS
    void
*/
void hfpHandleError(Task link_ptr)
{
    /* Handle this response in the cotext of the AT cmd it relates to */
    atCmdAckReceived((hfp_link_data*)link_ptr, hfp_fail);
}


/****************************************************************************
NAME
    hfpHandleExtendedError

DESCRIPTION
    Extended error result code indication sent by the AG

AT INDICATION
    +CME ERROR

RETURNS
    void
*/
void hfpHandleExtendedError(Task link_ptr, const struct hfpHandleExtendedError *ind)
{
    hfp_link_data* link = (hfp_link_data*)link_ptr;
    /* Silently ignore extended error message if we are not HFP v.15 */
    if (hfpLinkIsHfp(link))
    {
        /* Handle this response in the cotext of the AT cmd it relates to */
        atCmdAckReceived(link, convertErrorCode(ind->err));
    }
}


/****************************************************************************
NAME    
    hfpHandleWaitAtTimeout

DESCRIPTION
    Waiting for OK/ ERROR response to AT cmd timeout has expired. This means 
    that we have not received a response for the last AT cmd. So we don't
    get completely stuck, send out the next cmd anyway.

RETURNS
    void
*/
void hfpHandleWaitAtTimeout(hfp_link_data* link)
{
    /* AG failed to respond, handle in the context of the AT cmd that timed
     out - ignore if sink is NULL as this means the link has already been 
     tidied up (should never happen but might as well be graceful if it does)
     */
    atCmdAckReceived(link, hfp_timeout);
}


/****************************************************************************
NAME    
    hfpHandleNoCarrierInd

DESCRIPTION
    Handle the AT NO CARRIER indication sent by the AG.

AT INDICATION
    NO CARRIER

RETURNS
    void
*/
void hfpHandleNoCarrierInd(Task link_ptr)
{
    /* For the moment just ignore this */
    UNUSED(link_ptr);
}


/****************************************************************************
NAME    
    handleUnrecognised

DESCRIPTION
    Called when we receive data that cannot be recognised by the AT cmd 
    parser. To stop us from panicking if the AG is spamming us with 
    unrecognised data e.g. some out of spec AG is sending us the wrong
    commands, we only send the unrecognised data to the app if we have the
    resources, otherwise we silently ignore it!

RETURNS
    void
*/
void handleUnrecognised(const uint8 *data, uint16 length, Task link_ptr)
{
    if (!length)
        return;

    /* 
        Create a message and send it directly to the app. 
        No point going through the state machine because we don't 
        know what this data is anyway.
    */
    {
        MAKE_HFP_MESSAGE_WITH_LEN(HFP_UNRECOGNISED_AT_CMD_IND, length);
        
        if (message)
        {
            message->priority = hfpGetLinkPriority((hfp_link_data*)link_ptr);
            message->size_data = length;
            memmove(message->data, data, length);
            MessageSend(theHfp->clientTask, HFP_UNRECOGNISED_AT_CMD_IND, message);    
        }
        /* If we didn't alloc the mesage don't panic, just ignore this we only send these up if we have spare resources */
    }
}

