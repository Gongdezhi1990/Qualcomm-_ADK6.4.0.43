/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    a2dp_codec_csr_aptxhd.c

DESCRIPTION
    This file contains aptX-HD specific code.

NOTES

*/

#ifndef A2DP_SBC_ONLY

/****************************************************************************
    Header files
*/
#include "a2dp.h"
#include "a2dp_private.h"
#include "a2dp_caps_parse.h"
#include "a2dp_codec_csr_aptxhd.h"


/**************************************************************************/
void selectOptimalCsrAptxHDCapsSink(const uint8 *local_codec_caps, uint8 *remote_codec_caps)
{
    /* Choose what is supported at both sides */
    remote_codec_caps[10] = (remote_codec_caps[10]) & (local_codec_caps[10]);
    
    /* Select sample frequency */
    if (remote_codec_caps[10] & 0x20)
    {   /* choose 44k1 */
        remote_codec_caps[10] &= 0x2f;
    }
    else 
    {   /* choose 48k - aptX only supports 44.1Khz and 48Khz */
        remote_codec_caps[10] &= 0x1f;
    }
    
    /*Choose stereo */
    remote_codec_caps[10] &= 0xf2;
}


/**************************************************************************/
void selectOptimalCsrAptxHDCapsSource(const uint8 *local_codec_caps, uint8 *remote_codec_caps)
{
    /* Choose what is supported at both sides */
    remote_codec_caps[10] = (remote_codec_caps[10]) & (local_codec_caps[10]);
    
    /* Select sample frequency */
    if (remote_codec_caps[10] & 0x10)
    {   /* choose 48k */
        remote_codec_caps[10] &= 0x1f;
    }
    else
    {   /* choose 44k1 - aptX only supports 44.1Khz and 48Khz */
        remote_codec_caps[10] &= 0x2f;
    }

    /*Choose stereo */
    remote_codec_caps[10] &= 0xf2;
}

/*************************************************************************/
void getCsrAptxHDConfigSettings(const uint8 *service_caps, a2dp_codec_settings *codec_settings)
{
    codec_settings->codecData.packet_size = 668;
    
    if (!service_caps)
    {
        codec_settings->rate = 44100;
        codec_settings->channel_mode = a2dp_stereo;
        return;
    }
    
    /* Work out the sample rate based on the codec configuration. */
    if (service_caps[10] & 0x10)
    {
        codec_settings->rate = 48000;
    }
    else
    { /* aptX-HD only supports 44.1Khz and 48Khz */
        codec_settings->rate = 44100;
    }
    
    codec_settings->channel_mode = a2dp_stereo;
}

#else
    static const int dummy;
#endif /* A2DP_SBC_ONLY */
