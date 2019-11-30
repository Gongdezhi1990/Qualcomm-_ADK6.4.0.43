/****************************************************************************
Copyright (c) 2005 - 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    aghfp_wbs.h
    
DESCRIPTION
    Interface for the Wide Band Speech Service for the AGHFP library.

*/

#ifndef AGHFP_WBS_H
#define AGHFP_WBS_H

#include "aghfp.h"

/****************************************************************************
NAME    
    aghfpWbsCodecMaskToCodecId

DESCRIPTION
    Convert a codec mask to a codec ID (as defined in HFP1.6 spec)

RETURNS
    The codec ID if successful, or wbs_codec_invalid if the codec
    mask passed in was invalid
*/
aghfp_wbs_codec aghfpWbsCodecMaskToCodecId(aghfp_codecs_info *codecs_info_in, uint16 codec_mask);

/*
    Return the codec mask given a Codec ID.
*/
/****************************************************************************
NAME    
    aghfpWbsCodecIdToCodecMask

DESCRIPTION
    Convert a codec ID (as defined in HFP1.6 spec) to a codec mask

RETURNS
    The codec mask if successful, or 0 if the codec ID passed in was invalid
*/
uint16 aghfpWbsCodecIdToCodecMask(aghfp_codecs_info *codecs_info_in, aghfp_wbs_codec codec_id);


#endif /* AGHFP_WBS_H */

