/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_slc_handler.c        

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_link_manager.h"
#include "hfp_service_manager.h"
#include "hfp_parse.h"
#include "hfp_rfc.h"
#include "hfp_sdp.h"
#include "hfp_send_data.h"
#include "hfp_slc_handler.h"
#include "hfp_indicators.h"
#include "hfp_audio_handler.h"
#include "hfp_init.h"
#include "hfp_wbs.h"

#include <bdaddr.h>
#include <panic.h>
#include <stdio.h>
#include <string.h>
#include <sink.h>
#include <print.h>

#define stringLen(string) (sizeof((string))-1)

/* Tidy up SLC on connection */
static void hfpSlcConnectTidy(hfp_link_data* link)
{
    /* Issue a request to unregister the service record if using multipoint */
    if(link->bitfields.owns_service && (theHfp->num_links == 2))
        hfpUnregisterServiceRecord(link->service);
            
    /* Set the link state to slc complete */
    hfpSetLinkSlcState(link, hfp_slc_complete);
            
    /* Disable noise reduction/echo cancelation on the AG */
    hfpSendCommonInternalMessage(HFP_INTERNAL_AT_NREC_REQ, link);
}


/****************************************************************************
NAME    
    hfpSendSlcConnectCfmToApp

DESCRIPTION
    Send a HFP_SLC_CONNECT_CFM message to the app telling it the outcome
    of the connect attempt.

RETURNS
    void
*/
void hfpSendSlcConnectCfmToApp(hfp_link_data* link, const bdaddr* bd_addr, hfp_connect_status status)
{
    if(link && (link->bitfields.ag_link_loss_state != hfp_link_loss_none))
    {
        if (status == hfp_connect_success)
            hfpSlcConnectTidy(link);
        
        /* We're trying to recover from link loss... keep going */
        hfpHandleRfcommLinkLossComplete(link, status);
    }
    else
    {
        MAKE_HFP_MESSAGE(HFP_SLC_CONNECT_CFM);
        message->status   = status;
        message->sink     = hfpGetLinkSink(link);
        message->priority = hfpGetLinkPriority(link);
        message->profile  = hfpLinkGetProfile(link);
        /* Try and get bdaddr from the link data */
        if(!hfpGetLinkBdaddr(link, &message->bd_addr) && bd_addr)
            message->bd_addr = *bd_addr;
        
        /* If the connect succeeded need to tidy up a few things */
        if(link)
        {
            if (status == hfp_connect_success)
                hfpSlcConnectTidy(link);
            else if (status != hfp_connect_failed_busy)
                hfpLinkReset(link, TRUE);
        }
        MessageSend(theHfp->clientTask, HFP_SLC_CONNECT_CFM, message);
    }
}

/****************************************************************************
NAME    
    hfpSendSlcDisconnectIndToApp

DESCRIPTION
    Send a HFP_SLC_DISCONNECT_IND message to the app notifying it that
    the SLC has been disconnected.

RETURNS
    void
*/
void hfpSendSlcDisconnectIndToApp(hfp_link_data* link, hfp_disconnect_status status)
{
    MAKE_HFP_MESSAGE(HFP_SLC_DISCONNECT_IND);
    message->status   = status;
    message->priority = hfpGetLinkPriority(link);
    (void)hfpGetLinkBdaddr(link, &message->bd_addr);
    
    /* Tidy up if valid link was disconnected */
    if(status != hfp_disconnect_no_slc)
    {
        /* Reset the connection related state */
        hfpLinkReset(link, TRUE);

        /* Cancel the AT response timeout message because we'll have no more AT cmds being sent */
        (void) MessageCancelAll(&theHfp->task, hfpGetLinkTimeoutMessage(link, HFP_INTERNAL_WAIT_AT_TIMEOUT_LINK_0_IND));
    }
    MessageSend(theHfp->clientTask, HFP_SLC_DISCONNECT_IND, message);
}


/****************************************************************************
NAME    
    hfpHandleBrsfRequest

DESCRIPTION
    Send AT+BRSF to the AG.

RETURNS
    void
*/
void hfpHandleBrsfRequest(hfp_link_data* link)
{
    char brsf[15];

    /* Create the AT cmd we're sending */
    sprintf(brsf, "AT+BRSF=%d\r", theHfp->hf_supported_features);

    /* Send the AT cmd over the air */
    hfpSendAtCmd(link, (uint16)strlen(brsf), brsf, hfpBrsfCmdPending);
}


/****************************************************************************
NAME    
    hfpHandleBacRequest

DESCRIPTION
    Send AT+BAC= to the AG.

RETURNS
    void
*/
void hfpHandleBacRequest(hfp_link_data* link)
{
    /* Safety check against misbehaving AGs; only proceed if the AG supports Codec Negotiation. */
    if(agFeatureEnabled(link, AG_CODEC_NEGOTIATION) && hfFeatureEnabled(HFP_CODEC_NEGOTIATION))
    {
        char                bac_req[20];
        hfp_wbs_codec_mask  check_bit;
        
        /* Create a local copy of HF supported codecs */
        hfp_wbs_codec_mask supported_codecs = theHfp->bitfields.wbs_codec_mask;
        
        /* Start the AT+BAC command */
        unsigned char_idx = sprintf(bac_req, "AT+BAC=");
        
        /* Run through all bits of supported codec mask until no more set */
        for(check_bit=hfp_wbs_codec_mask_cvsd; supported_codecs != hfp_wbs_codec_mask_none; check_bit <<= 1)
        {
            hfp_wbs_codec_mask matching_codec = supported_codecs & check_bit;

            /* Check if bit is set in the codec mask */
            if(matching_codec)
            {
                /* Convert this bit to codec ID and put into AT+BAC */
                char_idx += sprintf(&bac_req[char_idx], "%d,", hfpWbsCodecMaskToCodecId(matching_codec));

                supported_codecs &= ~(matching_codec);
            }
        }
        
        /* Replace last ',' with '\r' */
        sprintf(&bac_req[char_idx-1], "\r");
        
        /* Send the AT cmd over the air */
        hfpSendAtCmd(link, (uint16)char_idx, bac_req, hfpBacCmdPending);
    }
}


/****************************************************************************
NAME    
    hfpHandleCindTestRequest

DESCRIPTION
    Send AT+CIND=? to the AG.

RETURNS
    void
*/
static void hfpHandleCindTestRequest(hfp_link_data* link)
{
    /* Send the AT cmd over the air */
    char cind_test[] = "AT+CIND=?\r";
    hfpSendAtCmd(link, stringLen(cind_test), cind_test, hfpCindTestCmdPending);
}


/****************************************************************************
NAME    
    hfpHandleCindReadRequest

DESCRIPTION
    Send AT+CIND? to the AG.

RETURNS
    void
*/
static void hfpHandleCindReadRequest(hfp_link_data* link)
{
    char cind_read[] = "AT+CIND?\r";

    /* Send the AT cmd over the air */
    hfpSendAtCmd(link, stringLen(cind_read), cind_read, hfpCindReadCmdPending);
}


/****************************************************************************
NAME    
    hfpHandleCmerRequest

DESCRIPTION
    Send AT+CMER to the AG.

RETURNS
    void
*/
static void hfpHandleCmerRequest(hfp_link_data* link)
{
    const char cmer[] = "AT+CMER=3, 0, 0, 1\r";

    /* Send the AT cmd over the air */
    hfpSendAtCmd(link, sizeof(cmer)-1, cmer, hfpCmerCmdPending);
}


/****************************************************************************
NAME    
    hfpHandleChldStatusRequest

DESCRIPTION
    Send AT+CHLD=? to the AG.

RETURNS
    void
*/
static void hfpHandleChldStatusRequest(hfp_link_data* link)
{
    const char chld[] = "AT+CHLD=?\r";
    
    /* Only send this if both sides support TWC, otherwise SLC is complete */
    if (hfFeatureEnabled(HFP_THREE_WAY_CALLING) && agFeatureEnabled(link, AG_THREE_WAY_CALLING))
        hfpSendAtCmd(link, sizeof(chld)-1, chld, hfpChldStatusCmdPending);
}


/****************************************************************************
NAME    
    hfpHandleBindStatusRequest

DESCRIPTION
    Send AT+BIND=? to the AG.

RETURNS
    void
*/
static void hfpHandleBindStatusRequest(hfp_link_data* link)
{
    const char* bind = "AT+BIND=?\r";

    /* Send the AT cmd over the air */
    hfpSendAtCmd(link, strlen(bind), bind, hfpBindStatusCmdPending);
}


/****************************************************************************
NAME    
    hfpHandleBindIndList

DESCRIPTION
    Send AT+BIND= to the AG.

RETURNS
    void
*/
static void hfpHandleBindIndList(hfp_link_data* link)
{    
    /* Only send this if both sides support HF Indicators, otherwise SLC is complete */
    if (hfFeatureEnabled(HFP_HF_INDICATORS) && agFeatureEnabled(link, AG_HF_INDICATORS))
   {
        char   bind_req[AT_COMMAND_LENGTH];
        uint16 check_bit;
        
        /* Create a local copy of HF supported indicators */
        hfp_hf_indicators_mask supported_hf_indicators = theHfp->hf_indicators;
        
        /* Start the AT+BIND command */
        uint8  char_idx = sprintf(bind_req, "AT+BIND=");
        
        /* Run through all bits of supported indicators mask until no more set */
        for(check_bit=1; supported_hf_indicators > 0; check_bit <<= 1)
        {
            /* Check if bit is set in the indicators mask */
            if(supported_hf_indicators & check_bit)
            {
                /* Convert this bit to assigned number ID and put into AT+BIND */
                char_idx += sprintf(&bind_req[char_idx], "%d,", hfpHfIndicatorMaskToAssignedId(supported_hf_indicators & check_bit));
            }
            /* Clear this bit in supported indicators */
            supported_hf_indicators &= ~(supported_hf_indicators & check_bit);
        }
        
        /* Replace last ',' with '\r' */
        sprintf(&bind_req[char_idx-1], "\r");
        
        /* Send the AT cmd over the air */
        hfpSendAtCmd(link, char_idx, bind_req, hfpBindCmdPending);
    }

}


/****************************************************************************
NAME    
    hfpHandleBindIndStatusRequest

DESCRIPTION
    Send AT+BIND? to the AG.

RETURNS
    void
*/
static void hfpHandleBindIndStatusRequest(hfp_link_data* link)
{
    const char* bind = "AT+BIND?\r";

    /* Send the AT cmd over the air */
    hfpSendAtCmd(link, strlen(bind), bind, hfpBindIndStatusCmdPending);
}


/****************************************************************************
NAME    
    hfpSlcCheckAtAck

DESCRIPTION
    Generic handler for AT Acks during SLC establishment

RETURNS
    TRUE if Ack was successful, FALSE otherwise
*/
bool hfpSlcCheckAtAck(hfp_link_data* link, hfp_lib_status status)
{
    /* An AT command in the SLC establishment has failed */
    if(status != hfp_success)
    {
        /* If we have an RFCOMM connection we want to get rid of it */
        if(link->bitfields.ag_slc_state == hfp_slc_connected  && SinkIsValid(hfpGetLinkSink(link)))
        {
            /* We have a valid RFCOMM connection still, tear it down */
            hfpSendCommonInternalMessage(HFP_INTERNAL_SLC_DISCONNECT_REQ, link);
            /* NB. When disconnecting in hfp_slc_connected this can result in two
            disconnect requests being sent to the CL. This either results in
            the CL rejecting the request with status rfcomm_disconnect_unknown_sink
            which we ignore, or BlueStack rejecting the request with RFC_INVALID_CHANNEL
            which the CL does not forward on to us. In both cases only one 
            HFP_SLC_CONNECT_CFM will be sent to the app when disconnected */
        }
        return FALSE;
    }
    return TRUE;
}


/****************************************************************************
NAME    
    hfpHandleSupportedFeaturesNotification

DESCRIPTION
    Store the AG's supported features.

RETURNS
    void
*/
void hfpHandleSupportedFeaturesNotification(hfp_link_data* link, uint16 features)
{
    /* Store the AG's supported features */
    link->ag_supported_features = features;
}


/****************************************************************************
NAME    
    hfpHandleSupportedFeaturesInd

DESCRIPTION
    Handle the supported features sent by the AG.

AT INDICATION
    +BRSF

RETURNS
    void
*/
void hfpHandleSupportedFeaturesInd(Task link_ptr, const struct hfpHandleSupportedFeaturesInd *ind)
{
    hfp_link_data* link = (hfp_link_data*)link_ptr;

    /* Deal with supported Features */
    hfpHandleSupportedFeaturesNotification(link, ind->supportedfeat);
    
    /* Send AT+BAC if supported */
    hfpHandleBacRequest(link);
}


/****************************************************************************
NAME
    hfpHandleQacRequest

DESCRIPTION
    Send the Qualcomm Codec Modem IDs support from HF (Q2Q feature)

AT INDICATION
    AT+%QAC=[<n>[,<n>[,...<n>]]]

RETURNS
    void
*/
static void hfpHandleQacRequest(hfp_link_data* link)
{
    char qac[30];
    int bit;
    uint16 hf_codec_modes = theHfp->hf_codec_modes;

    /* Create the AT cmd we're sending */
    uint8 char_idx = sprintf(qac, "AT+%%QAC=");

    for (bit=0; bit<16 && hf_codec_modes; bit++)
    {
        uint16 bit_mask = 1 << bit;

        /* If the codec bit isn't set, go to the next one. */
        if ( !(bit_mask & hf_codec_modes))
            continue;

        char_idx += sprintf(&qac[char_idx], "%d", hfpConvertCodecModeBitToId(bit_mask));

        hf_codec_modes ^= bit_mask;
        /* Are there still more Codec Mode IDs (bits) to process?
         * Add a comma before the next Codec Mode ID.
         */
        if (hf_codec_modes)
        {
            char_idx += sprintf(&qac[char_idx], ",");
        }
    }

    sprintf(&qac[char_idx], "\r");

    /* Send the AT cmd over the air */
    hfpSendAtCmd(link, (uint16)strlen(qac), qac, hfpQacCmdPending);
}


/****************************************************************************
NAME    
    hfpHandleBrsfAtAck

DESCRIPTION
    Called when we receive OK/ ERROR in response to the AT+BRSF cmd. This 
    indicates whether the AG recognised the cmd.

RETURNS
    void
*/
void hfpHandleBrsfAtAck(hfp_link_data* link, hfp_lib_status status)
{
    /* Perform SDP search if BRSF failed */
    if (status != hfp_success)
        hfpGetAgSupportedFeatures(link);

    /* If Qualcomm Codec Extension Mode IDs have been defined then
     * HF is in QCE mode and should send AT+QAC to indicate the modes
     * supported.
     */
    if (theHfp->hf_codec_modes)
    {
        hfpHandleQacRequest(link);
    }
    else
    {
        /* Send AT+CIND=? (or queue if AT+BAC was sent) */
        hfpHandleCindTestRequest(link);
        /* Queue up AT+CIND? */
        hfpHandleCindReadRequest(link);
    }
}


/****************************************************************************
NAME
    hfpHandleQacAtAck

DESCRIPTION
    Called when we receive OK/ ERROR in response to the AT+QAC cmd. This
    indicates whether the AG recognised the cmd.

RETURNS
    void
*/
void hfpHandleQacAtAck(hfp_link_data* link, hfp_lib_status status)
{
    if (status != hfp_success)
        /* No Qualcomm Codec support */
        do { } while(0); /* STUB */

    /* Send AT+CIND=? (or queue if AT+BAC was sent) */
    hfpHandleCindTestRequest(link);
    /* Queue up AT+CIND? */
    hfpHandleCindReadRequest(link);
}


/****************************************************************************
NAME    
    hfpHandleBacAtAck

DESCRIPTION
    Called when we receive OK/ ERROR in response to the AT+BAC cmd. This 
    indicates whether the AG recognised the cmd.

RETURNS
    void
*/
void hfpHandleBacAtAck(hfp_link_data* link, hfp_lib_status status)
{
    /* AG didn't understand AT+BAC so clear its codec negotiation bit */
    if(link->bitfields.ag_slc_state == hfp_slc_connected && status != hfp_success)
        link->ag_supported_features &= ~AG_CODEC_NEGOTIATION;
}


/****************************************************************************
NAME    
    hfpHandleCindTestAtAck

DESCRIPTION
    Called when we receive OK/ ERROR in response to the AT+CIND=? cmd. This 
    indicates whether the AG recognised the cmd.

RETURNS
    void
*/
void hfpHandleCindTestAtAck(hfp_link_data* link, hfp_lib_status status)
{
    /* Queue AT+CMER if successful */
    if(hfpSlcCheckAtAck(link, status))
        hfpHandleCmerRequest(link);
}


/****************************************************************************
NAME    
    hfpHandleCindReadAtAck

DESCRIPTION
    Called when we receive OK/ ERROR in response to the AT+CIND? cmd. This 
    indicates whether the AG recognised the cmd.

RETURNS
    void
*/
void hfpHandleCindReadAtAck(hfp_link_data* link, hfp_lib_status status)
{
    /* Queue AT+CHLD=? if successful */
    if(hfpSlcCheckAtAck(link, status))
        hfpHandleChldStatusRequest(link);
}


/****************************************************************************
NAME    
    hfpHandleCmerAtAck

DESCRIPTION
    Called when we receive OK/ ERROR in response to the AT+CMER cmd. If we're
    not getting call hold params from the AG then the SLC is complete.

RETURNS
    void
*/
void hfpHandleCmerAtAck(hfp_link_data* link, hfp_lib_status status)
{
    if (hfpSlcCheckAtAck(link, status))
    {
        /* If TWC and HF Indicator not enabled this is the last AT command in the SLC establishment 
            f = a'b' + b'c' + c'd' + d'a'*/
        if((!hfFeatureEnabled(HFP_THREE_WAY_CALLING) & !hfFeatureEnabled(HFP_HF_INDICATORS)) ||
			(!hfFeatureEnabled(HFP_HF_INDICATORS) & !agFeatureEnabled(link, AG_THREE_WAY_CALLING)) ||
			(!agFeatureEnabled(link, AG_THREE_WAY_CALLING) & !agFeatureEnabled(link, AG_HF_INDICATORS)) ||
			(!agFeatureEnabled(link, AG_HF_INDICATORS) & !hfFeatureEnabled(HFP_THREE_WAY_CALLING) ))
        {
	     hfpSendSlcConnectCfmToApp(link, NULL, hfp_connect_success);
        }
        /* Queue AT+BIND=*/	
        else
        {
	     hfpHandleBindIndList(link);
        }
    }
}



/****************************************************************************
NAME    
    hfpHandlerBindAtAck

DESCRIPTION
    Called when we receive OK/ ERROR in response to the AT+BIND cmd. If we're
    not getting HF Indicators Ack from the AG then the SLC is complete.

RETURNS
    void
*/
void hfpHandlerBindAtAck(hfp_link_data* link, hfp_lib_status status)
{
    if (hfpSlcCheckAtAck(link, status))
    {
       /* Request for AG supported HF Indicators AT+BIND=? if successful */
       hfpHandleBindStatusRequest(link);
    }
}


/****************************************************************************
NAME    
    hfpHandlerBindReadAtAck

DESCRIPTION
    Called when we receive OK/ ERROR in response to the AT+BIND=? cmd. If we're
    not getting HF Indicators Ack from the AG then the SLC is complete.

RETURNS
    void
*/
void hfpHandlerBindReadAtAck(hfp_link_data* link, hfp_lib_status status)
{
    if (hfpSlcCheckAtAck(link, status))
    {
       /* Request for AG enabled/disabled status of Indicators, AT+BIND? if successful */
	   hfpHandleBindIndStatusRequest(link);    
    }
}


/****************************************************************************
NAME    
    hfpHandlerBindIndStatusAtAck

DESCRIPTION
    Called when we receive OK/ ERROR in response to the AT+BIND? cmd. If we're
    not getting HF Indicators Ack from the AG then the SLC is complete.

RETURNS
    void
*/
void hfpHandlerBindIndStatusAtAck(hfp_link_data* link, hfp_lib_status status)
{	
    if (hfpSlcCheckAtAck(link, status))
    {
        /* Inform the app the SLC has been established */
        hfpSendSlcConnectCfmToApp(link, NULL, hfp_connect_success);   
    }
}

/****************************************************************************
NAME    
hfpHandleCallHoldSupportInd

DESCRIPTION
    Call hold parameters received and parsed. 

RETURNS
    void
*/
static void hfpHandleCallHoldSupportInd(Task link_ptr)
{
    hfp_link_data* link = (hfp_link_data*)link_ptr;
    if (!hfFeatureEnabled(HFP_HF_INDICATORS) || !agFeatureEnabled(link, AG_HF_INDICATORS))
    {
        /* Inform the app the SLC has been established */
        hfpSendSlcConnectCfmToApp(link, NULL, hfp_connect_success);
    }
}


/****************************************************************************
NAME    
    hfpHandleCallHoldInfo

DESCRIPTION
    Generic call hold parameter handler. Called when we don't have a 
    dictionary match for the call hold string.

AT INDICATION
    +CHLD

RETURNS
    void
*/
void hfpHandleCallHoldInfo(Task link_ptr, const struct hfpHandleCallHoldInfo *ind)
{
    UNUSED(ind);

    hfpHandleCallHoldSupportInd(link_ptr);
}


/****************************************************************************
NAME    
    hfpHandleCallHoldInfoCommon

DESCRIPTION
    Call hold parameter handler for one very common set of call hold parameters -
    namely when the string looks like "\r\n+CHLD: (0,1,1x,2,2x,3,4)\r\n". 

AT INDICATION
    +CHLD

RETURNS
    void
*/
void hfpHandleCallHoldInfoCommon(Task link_ptr)
{
    hfpHandleCallHoldSupportInd(link_ptr);
}


/****************************************************************************
NAME    
    hfpHandleCallHoldInfoRange

DESCRIPTION
    Generic call hold parameter handler used when the params are specified as 
    a range of values. Called when we don't have a dictionary match for the 
    call hold string.

AT INDICATION
    +CHLD

RETURNS
    void
*/
void hfpHandleCallHoldInfoRange(Task link_ptr, const struct hfpHandleCallHoldInfoRange *ind)
{
    UNUSED(ind);

    hfpHandleCallHoldSupportInd(link_ptr);
}


/****************************************************************************
NAME    
    hfpHandleBindInfo

DESCRIPTION
    AG supported indicators  -
    namely when the string looks like "\r\n+BIND:(1,2, , ...n)\r\n". 

AT INDICATION
    +BIND

RETURNS
    void
*/
void hfpHandleBindInfo(Task link_ptr, const struct hfpHandleBindInfo *hf_indicators)
{    	
    hfp_link_data* link = (hfp_link_data*)link_ptr;
    uint16 hf_indicators_mask=0;
    uint16 num_of_entries = hf_indicators->indicators.count;
    uint16 counter;
    struct value_hfpHandleBindInfo_indicators value;

    /* No need to save the HF indicator supported by AG in hfp library, it is tracked by
    * HF indicator status send by AG. Just notify the app about the HF Indicators supported by AG.
    */
    for (counter = 0; counter < num_of_entries; counter++)
    {
        /* Get the next HF Indicator ID. */
	 value = get_hfpHandleBindInfo_indicators(&hf_indicators->indicators, counter);

	 hf_indicators_mask |= hfpAssignedIdToHfindicatorMask(value.indicator);
    }
	
    /* Notify the app about HF indicators supported by AG */
    sendHfIndicatorsToApp(link, hf_indicators_mask, num_of_entries );
}


/****************************************************************************
NAME    
    hfpHandleBindIndicatorStatus

DESCRIPTION
    AG enabled/disabled status of indicators  -
    namely when the string looks like "\r\n+BIND: a, state\r\n". 

AT INDICATION
    +BIND

RETURNS
    void
*/
void hfpHandleBindIndicatorStatus(Task link_ptr, const struct hfpHandleBindIndicatorStatus *ind)
{
    /* Update the AG enabled/disabled status of hf indicators */
    hfp_link_data* link = (hfp_link_data*)link_ptr;

    /* Update the AG supported hf indicator state */
    if(ind->state == hfp_hf_indicator_on)
    {
        /* Set the HF Indicator state */
        link->ag_supported_hf_indicators.hf_indicators_status |= (hfp_hf_indicator_on << ind->assignedId);
    }
    else
    {
        /* Clear the HF Indicator state */
        link->ag_supported_hf_indicators.hf_indicators_status &= ~(hfp_hf_indicator_on << ind->assignedId);
    }

    /* Notify the app about HF indicator state */
    sendHfIndicatorSignalToApp(link, ind->assignedId, ind->state);
}


/****************************************************************************
NAME
    hfpHandleDisconnectRequest

DESCRIPTION
    We're in the right state and have received a disconnect request, 
    handle it here.

RETURNS
    void
*/
void hfpHandleDisconnectRequest(hfp_link_data* link)
{
    if (link->audio_sink)
    {
        MAKE_HFP_MESSAGE(HFP_INTERNAL_SLC_DISCONNECT_REQ);
        message->link = link;
        /* If we have a SCO/eSCO active need to tear that down first */
        hfpHandleAudioDisconnectReq(link);
        /* Queue up the SLC disconnect message */
        MessageSendConditionally(&theHfp->task, HFP_INTERNAL_SLC_DISCONNECT_REQ, message, (uint16 *) &link->audio_sink);    /*lint !e740 */
    }
    else 
    {
        /* If recovering from link loss or timed out we need to be sure to force disconnect */
        if (link->bitfields.ag_link_loss_state == hfp_link_loss_recovery || link->bitfields.ag_link_loss_state == hfp_link_loss_timeout)
        {
            /* Link was recovering from link loss, stop the procedure */
            MessageId message_id = hfpGetLinkTimeoutMessage(link, HFP_RFCOMM_LINK_LOSS_TIMEOUT_LINK_0_IND);
            MessageCancelFirst(&theHfp->task, message_id);
            hfpHandleRfcommLinkLossAbort(link, TRUE);
        }
        /* Either no link loss or we aborted link loss recovery. Now bring down the link. */
        else
        {
            /* Can only get here from searching/outgoing/incoming/connected/complete */
            if(link->bitfields.ag_slc_state == hfp_slc_searching)
            {
                /* Notify application of connection failure (link will be reset so SDP results ignored) */
                hfpSendSlcConnectCfmToApp(link, NULL, hfp_connect_sdp_fail);
            }
            else
            {
                /* Request the connection lib aborts/disconnects the RFCOMM connection */
                ConnectionRfcommDisconnectRequest(&theHfp->task, hfpGetLinkSink(link));
            }
        }
    }
}
