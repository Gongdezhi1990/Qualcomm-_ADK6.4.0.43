/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_wbs.h
    
DESCRIPTION
    Interface for the Wide Band Speech Service for the HFP library.

*/

#ifndef HFP_WBS_H
#define HFP_WBS_H

#include <bdaddr_.h>
#include <message.h>

/* Codec ID's for WB-Speech. */
typedef enum
{
    wbs_codec_invalid = 0x00,
    wbs_codec_cvsd    = 0x01,
    wbs_codec_msbc    = 0x02
} hfp_wbs_codec_id;


/****************************************************************************
NAME    
    hfpWbsCodecMaskToCodecId

DESCRIPTION
    Convert a codec mask to a codec ID (as defined in HFP1.6 spec)

RETURNS
    The codec ID if successful, or wbs_codec_invalid if the codec
    mask passed in was invalid
*/
hfp_wbs_codec_id hfpWbsCodecMaskToCodecId(hfp_wbs_codec_mask codec_mask);


/****************************************************************************
NAME    
    hfpWbsCodecIdToCodecMask

DESCRIPTION
    Convert a codec ID (as defined in HFP1.6 spec) to a codec mask

RETURNS
    The codec mask if successful, or hfp_wbs_codec_mask_none if the codec
    ID passed in was invalid
*/
hfp_wbs_codec_mask hfpWbsCodecIdToCodecMask(hfp_wbs_codec_id codec_id);

#endif /* HFP_WBS_H */

