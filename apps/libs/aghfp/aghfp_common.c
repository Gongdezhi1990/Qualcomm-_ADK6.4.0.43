/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

*/


#include "aghfp_common.h"

#include <panic.h>


/*****************************************************************************/
void aghfpSetState(AGHFP *aghfp, aghfp_state state)
{
	aghfp->state = state;
}


/*****************************************************************************
	Create a common cfm message (many AGHFP defined messages sent to the app
	have the form of the message below and a common function can be used to
	allocate them). Send the message not forgetting to set the correct 
	message id.
*/
void aghfpSendCommonCfmMessageToApp(uint16 message_id, AGHFP *aghfp, aghfp_lib_status status)
{
	MAKE_AGHFP_MESSAGE(AGHFP_COMMON_CFM_MESSAGE);
	message->status = status;
	message->aghfp = aghfp;
	MessageSend(aghfp->client_task, message_id, message);
}


/*****************************************************************************/
bool supportedProfileIsHfp(aghfp_profile profile)
{
    switch (profile)
    {
        case aghfp_handsfree_profile:
        case aghfp_handsfree_15_profile:
        case aghfp_handsfree_16_profile:
        case aghfp_handsfree_17_profile:
            return 1;
            
        default:
            return 0;
        }
    
    return 0;
}


/*****************************************************************************/
bool supportedProfileHasHfp15Features(aghfp_profile profile)
{
    switch (profile)
    {
        case aghfp_handsfree_15_profile:
        case aghfp_handsfree_16_profile:
        case aghfp_handsfree_17_profile:
            return 1;
            
        default:
            return 0;
        }
    
    return 0;
}


/*****************************************************************************/
bool supportedProfileHasHfp17Features(aghfp_profile profile)
{
    if(profile == aghfp_handsfree_17_profile)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


/*****************************************************************************/
bool supportedProfileIsHsp(aghfp_profile profile)
{
    if (profile == aghfp_headset_profile)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*****************************************************************************/
codec_mode_bit_type aghfpConvertCodecModeIdToBit(codec_mode_id_type mode_id)
{
    /* Works on the premise that despite all Codec Mode ID values being uint16,
     * none of them are currently over the value 16. This also returns 0 if the
     * special value 0xFFFF is used.
     */
    if (mode_id > 16) 
        return 0;
    else
        return (1 << mode_id);
}

/*****************************************************************************/
codec_mode_id_type aghfpConvertCodecModeBitToId(codec_mode_bit_type mode_bit)
{
    switch (mode_bit)
    {
        case CODEC_64_2_EV3: 
            return aptx_adaptive_64_2_EV3;
        case CODEC_64_2_EV3_QHS3:
            return aptx_adaptive_64_2_EV3_QHS3;
        case CODEC_128_QHS3:
            return aptx_adaptive_128_QHS3;
        case CODEC_64_QHS3:
            return aptx_adaptive_64_QHS3;
        default:
            AGHFP_DEBUG_PANIC(("Unsupported Codec Mode ID!\n"));
            return CODEC_MODE_ID_UNSUPPORTED;
    }
}
