/****************************************************************************
Copyright (c) 2005 - 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    aghfp_wbs.c
    
DESCRIPTION
	Interface for the Wide Band Speech Service for the AGHFP library.

*/

#include "aghfp_wbs.h"

/*
    Return the Codec ID given a codec mask.
*/
aghfp_wbs_codec aghfpWbsCodecMaskToCodecId(aghfp_codecs_info *codecs_info_in, uint16 codec_mask)
{
    uint16 counter_1;
    uint16 test_mask = 1;
    uint16 codec_index = 0;
    bool   codec_found = FALSE;

    for(counter_1 = 0; counter_1 < 16; counter_1++)
    {
        /* Is it this codec? */
        if(codec_mask & test_mask)
        {
            codec_found = TRUE;
            break;
        }

        test_mask <<= 1;

        /* Move codec index along if it is time to do so */
        if(codecs_info_in->ag_codecs & test_mask) codec_index++;
    }

    if(codec_found)
    {
        return(codecs_info_in->codec_ids[codec_index]);
    }
    else
    {
        return(wbs_codec_invalid);
    }
 }
 
/*
    Return the codec mask given a Codec ID.
*/
uint16 aghfpWbsCodecIdToCodecMask(aghfp_codecs_info *codecs_info_in, aghfp_wbs_codec codec_id)
 {
    uint16 counter_1;
    uint16 test_mask = 1;
    uint16 codec_index = 0;
    bool   codec_found = FALSE;
    bool   first_bit_found = FALSE;

    for(counter_1 = 0; counter_1 < 16; counter_1++)
    {
        /* Find the first bit set in the codec mask. */
        if(!first_bit_found && !(codecs_info_in->ag_codecs & test_mask))
        {
            test_mask <<= 1;
            continue;
        }

        first_bit_found = TRUE;
    
        if((codecs_info_in->codec_ids[codec_index] == codec_id) && (codecs_info_in->ag_codecs & test_mask))
        {
            codec_found = TRUE;
            break;
        }

        test_mask <<= 1;
        
        /* Move codec index along if it is time to do so */
        if(codecs_info_in->ag_codecs & test_mask) codec_index++;
    }

    if(codec_found)
    {
        return(test_mask);
    }
    else
    {
        return(0);
    }
}


