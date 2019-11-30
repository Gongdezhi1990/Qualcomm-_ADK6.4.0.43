/****************************************************************************
Copyright (c) 2005 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_wbs.c
    
DESCRIPTION
    Interface for the Wide Band Speech Service for the HFP library.

*/

#include "hfp.h"
#include "hfp_private.h"
#include "hfp_wbs.h"
#include "hfp_link_manager.h"
#include "hfp_common.h"

#include <print.h>

/****************************************************************************
NAME    
    hfpWbsCodecMaskToCodecId

DESCRIPTION
    Convert a codec mask to a codec ID (as defined in HFP1.6 spec)

RETURNS
    The codec ID if successful, or wbs_codec_invalid if the codec
    mask passed in was invalid
*/
hfp_wbs_codec_id hfpWbsCodecMaskToCodecId(hfp_wbs_codec_mask codec_mask)
{
    switch(codec_mask)
    {
        case hfp_wbs_codec_mask_cvsd:
            return wbs_codec_cvsd;
        case hfp_wbs_codec_mask_msbc:
            return wbs_codec_msbc;
        default:
            return wbs_codec_invalid;
    }
}


/****************************************************************************
NAME    
    hfpWbsCodecIdToCodecMask

DESCRIPTION
    Convert a codec ID (as defined in HFP1.6 spec) to a codec mask

RETURNS
    The codec mask if successful, or hfp_wbs_codec_mask_none if the codec
    ID passed in was invalid
*/
hfp_wbs_codec_mask hfpWbsCodecIdToCodecMask(hfp_wbs_codec_id codec_id)
{
    switch(codec_id)
    {
        case wbs_codec_cvsd:
            return hfp_wbs_codec_mask_cvsd;
        case wbs_codec_msbc:
            return hfp_wbs_codec_mask_msbc;
        default:
            return hfp_wbs_codec_mask_none;
    }
}


/****************************************************************************
NAME    
    HfpWbsSetSupportedCodecs

DESCRIPTION
    Update the supported codec mask. If any optional WBS codec is included
    then the mandatory mSBC codec must also be included. CVSD must always be
    included. If these bits are not set correctly by the application they 
    will be overwritten by HFP.

RETURNS
    void
*/
void HfpWbsSetSupportedCodecs(hfp_wbs_codec_mask codec_mask, bool send_notification)
{
    /* If any optional codecs are enabled include mSBC */
    if(codec_mask & (~(hfp_wbs_codec_mask_msbc | hfp_wbs_codec_mask_cvsd)))
        codec_mask |= hfp_wbs_codec_mask_msbc;
    
    /* Always include CVSD */
    codec_mask |= hfp_wbs_codec_mask_cvsd;
    
    
    if(theHfp && theHfp->bitfields.initialised)
    {
        hfp_link_data* link;
        
        /* Set our codec mask */
        theHfp->bitfields.wbs_codec_mask = codec_mask;
        
        if(send_notification)
        {
            /* Send AT+BAC to any connected AG's that support codec negotiation */
            for_all_links(link)
            {
                hfpSendCommonInternalMessage(HFP_INTERNAL_AT_BAC_REQ, link);
            }
        }
    }
}
