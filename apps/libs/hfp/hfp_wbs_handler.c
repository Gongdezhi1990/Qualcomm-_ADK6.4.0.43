/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_wbs_handler.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_wbs_handler.h"
#include "hfp_slc_handler.h"
#include "hfp_send_data.h"
#include "hfp_parse.h"
#include "hfp_audio_handler.h"
#include "hfp_link_manager.h"
#include "hfp_wbs.h"

#include <panic.h>
#include <print.h>
#include <stdio.h>
#include <string.h>


/****************************************************************************
NAME
    hfpWbsEnable

DESCRIPTION
    Called during initialisation to set up HF supported codecs if the codec
    negotiation feature bit is enabled. If codec data is correctly configured
    this sets up codec data for HFP.

RETURNS
    void
*/
void hfpWbsEnable(const hfp_wbs_codec_mask supported_wbs_codecs)
{
    /* Force mandatory codec support initially (CVSD and mSBC) */
    theHfp->bitfields.wbs_codec_mask = supported_wbs_codecs | hfp_wbs_codec_mask_cvsd | hfp_wbs_codec_mask_msbc;
}


/****************************************************************************
NAME    
    hfpWbsStartCodecNegotiation

DESCRIPTION
    Start WBS codec negotiation, if supported by the HF and AG, by sending
    AT+BCC to the AG.

RETURNS
    TRUE if negotiation was started, FALSE otherwise
*/
bool hfpWbsStartCodecNegotiation(hfp_link_data* link)
{
    if(hfFeatureEnabled(HFP_CODEC_NEGOTIATION) && agFeatureEnabled(link, AG_CODEC_NEGOTIATION))
    {
        char bcc[] = "AT+BCC\r";

        link->bitfields.wbs_started_by_hf = TRUE;

        PRINT(("WBS: Send %s\n", bcc));

        /* Go ahead and send the AT+BCC command over the air. */
        hfpSendAtCmd(link, sizeof(bcc)-1, bcc, hfpBccCmdPending);
    
        return TRUE;
    }
    return FALSE;
}


/****************************************************************************
NAME    
    hfpHandleBccAtAck

DESCRIPTION
    Called when we receive OK/ ERROR in response to the AT+BCC cmd. This 
    indicates whether the AG recognised the cmd. If AG responded with OK it
    will send +BCS with the selected codec ID.

RETURNS
    void
*/
void hfpHandleBccAtAck(hfp_link_data* link, hfp_lib_status status)
{
    PRINT(("WBS: Handle AT+BCC OK 0x%x\n", status));
    
    if (status != hfp_success)
    {
        if (link->bitfields.wbs_started_by_hf)
        {
            /* AG can't start codec negotiation, send fail to app */
            link->bitfields.wbs_started_by_hf = FALSE;
            sendAudioConnectCfmFailToApp(link, hfp_audio_connect_codec_neg_fail);
        }
    }
}


/****************************************************************************
NAME    
    hfpHandleResponseWbsCodec

DESCRIPTION
    Called when +BCS is received from the AG. If the AG requested a codec we
    support then AT+BCS is sent to the AG in response. If the AG requested a
    codec we do not support then AT+BAC is sent, indicating which codecs we
    support.

RETURNS
    void
*/
void hfpHandleResponseWbsCodec(Task link_ptr, const struct hfpHandleResponseWbsCodec *codec)
{
    /* Handle AT+BCS */
    hfp_link_data* link = (hfp_link_data*)link_ptr;
    hfp_wbs_codec_id codec_id = (hfp_wbs_codec_id)codec->codec;

    PRINT(("WBS: Handle +BCS\n"));

    /* Safety check against misbehaving AGs; only proceed if the AG supports Codec Negotiation. */
    if(hfFeatureEnabled(HFP_CODEC_NEGOTIATION) && agFeatureEnabled(link, AG_CODEC_NEGOTIATION))
    {
        /* Translate from codec ID to internal bitmap. */
        hfp_wbs_codec_mask codec_mask = hfpWbsCodecIdToCodecMask(codec_id);
        
        PRINT(("WBS: AG asked for Codec ID 0x%x (Mask 0x%x)\n", codec_id, codec_mask));

        /* Does the HF support the requested codec? */
        if(theHfp->bitfields.wbs_codec_mask & codec_mask)
        {
            char bcs[15];

            /* Create the AT cmd we're sending */
            sprintf(bcs, "AT+BCS=%d\r", codec_id);

            PRINT(("WBS: Send %s\n", bcs));
            
            /* Save the codec mask */
            link->bitfields.wbs_codec_mask = codec_mask;

            /* Send the AT cmd over the air */
            hfpSendAtCmd(link, (uint16)strlen(bcs), bcs, hfpBcsCmdPending);
        }
        else
        {
            PRINT(("WBS: HF does not support codec (Mask is 0x%x)\n", theHfp->bitfields.wbs_codec_mask));
            /* Send an AT+BAC command to let the AG know which codecs the HF supports. */
            hfpSendCommonInternalMessage(HFP_INTERNAL_AT_BAC_REQ, link);
        }
    }
}


/****************************************************************************
NAME    
    hfpHandleBcsAtAck

DESCRIPTION
    Called when AG acknowledges our AT+BCS. This should always be an OK as we
    respond with the codec the AG requested. If something has gone wrong then
    we notify the application audio connection has failed.

RETURNS
    void
*/
void hfpHandleBcsAtAck(hfp_link_data* link, hfp_lib_status status)
{
    PRINT(("WBS: Handle AT+BCS OK\n"));
    if (status != hfp_success)
    {
        /* Panic in debug, this should be impossible. HF shall include
           requested codec in BCS if available and shall send AT+BAC if
           not. AG shall send OK if codec is same as requested. */
        HFP_DEBUG(("WBS: Handle AT+BCS OK Failure\n"));
        
        if (link->bitfields.wbs_started_by_hf)
        {
            /* Send fail to the app in release variant */
            sendAudioConnectCfmFailToApp(link, hfp_audio_connect_codec_neg_error);
        }
    }
    else
    {
        /* Wait for AG to initiate connection */
        PRINT(("WBS: Handle AT+BCS OK Success\n"));
    }
    link->bitfields.wbs_started_by_hf = FALSE; /* Reset flag to ensure it is correct next time */
}
