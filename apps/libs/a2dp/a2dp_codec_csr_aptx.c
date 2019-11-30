/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    a2dp_codec_csr_aptx.c

DESCRIPTION
    This file contains

NOTES

*/


/****************************************************************************
    Header files
*/
#include "a2dp.h"
#include "a2dp_private.h"
#include "a2dp_caps_parse.h"
#include "a2dp_codec_csr_aptx.h"

#ifndef A2DP_SBC_ONLY

/**************************************************************************/
void selectOptimalCsrAptxCapsSink(const uint8 *local_codec_caps, uint8 *remote_codec_caps)
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
    
    if ((remote_codec_caps[10] & 0x02) && (local_codec_caps[10] & 0x02))
    {   /* choose stereo */
        remote_codec_caps[10] &= 0xf2;
    }
    else
    {
        /* choose mono */
        remote_codec_caps[10] &= 0xf8;
    }
}


/**************************************************************************/
void selectOptimalCsrAptxCapsSource(const uint8 *local_codec_caps, uint8 *remote_codec_caps)
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

    if ((remote_codec_caps[10] & 0x02) && (local_codec_caps[10] & 0x02))
    {   /* choose stereo */
        remote_codec_caps[10] &= 0xf2;
    }
    else
    {
        /* choose mono */
        remote_codec_caps[10] &= 0xf8;
    }
}

/*************************************************************************/
void getCsrAptxConfigSettings(const uint8 *service_caps, a2dp_codec_settings *codec_settings)
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
    { /* aptX only supports 44.1Khz and 48Khz */
        codec_settings->rate = 44100;
    }
    
    if (service_caps[10] & 0x02)
    {
        codec_settings->channel_mode = a2dp_stereo;
    }
    else
    {
        codec_settings->channel_mode = a2dp_mono;
    }
}

#endif /* A2DP_SBC_ONLY */
