/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    a2dp_codec_aac.c

DESCRIPTION
    This file contains

NOTES

*/

/****************************************************************************
    Header files
*/
#include "a2dp.h"
#include "a2dp_private.h"
#include "a2dp_codec_aac.h"

#ifndef A2DP_SBC_ONLY

/**************************************************************************/
static void selectOptimalAacCaps (uint8 *remote_codec_caps)
{
    /* Object Type */
    if (remote_codec_caps[4] & (1 << 6))
    {   /* choose MPEG-4 AAC LC */
        remote_codec_caps[4] = (1 << 6);
    }
    else
    {   /* choose MPEG-2 AAC LC */
        remote_codec_caps[4] = (1 << 7);
    }

    /* Select Channel Mode */
    if (remote_codec_caps[6] & (1 << 2))
    {   /* choose stereo */
        remote_codec_caps[6] &= (1 << 2) | 0xf0;
    }
    else
    {   /* choose mono */
        remote_codec_caps[6] &= (1 << 3) | 0xf0;
    }

    /* Sample rate */
    if (remote_codec_caps[6] & (1 << 7))
    {   /* choose 48k */
        remote_codec_caps[6] &= (1 << 7) | 0x0f;
        remote_codec_caps[5] = 0;
    }
    else
    {
        remote_codec_caps[6] &= 0x0F;
        
        if (remote_codec_caps[5] & (1 << 0))
        {   /* choose 44k1 */
            remote_codec_caps[5] = (1 << 0);
        }
        else if (remote_codec_caps[5] & (1 << 1))
        {   /* choose 32k */
            remote_codec_caps[5] = (1 << 1);
        }
        else if (remote_codec_caps[5] & (1 << 2))
        {   /* choose 24k */
            remote_codec_caps[5] = (1 << 2);
        }
        else if (remote_codec_caps[5] & (1 << 3))
        {   /* choose 22.05k */
            remote_codec_caps[5] = (1 << 3);
        }
        else if (remote_codec_caps[5] & (1 << 4))
        {   /* choose 16k */
            remote_codec_caps[5] = (1 << 4);
        }
        else if (remote_codec_caps[5] & (1 << 5))
        {   /* choose 12k */
            remote_codec_caps[5] = (1 << 5);
        }
        else if (remote_codec_caps[5] & (1 << 6))
        {   /* choose 11.025k */
            remote_codec_caps[5] = (1 << 6);
        }
        else
        {   /* choose 8k */
            remote_codec_caps[5] = (1 << 7);
        }
    }

    /* Use VBR if available, hence leave bit alone */
}


/**************************************************************************/
void selectOptimalAacCapsSink (const uint8 *local_codec_caps, uint8 *remote_codec_caps)
{
    if (!local_codec_caps || !remote_codec_caps)
        return;

    selectOptimalAacCaps(remote_codec_caps);
}


/**************************************************************************/
void selectOptimalAacCapsSource (const uint8 *local_codec_caps, uint8 *remote_codec_caps)
{
    selectOptimalAacCapsSink (local_codec_caps, remote_codec_caps);
}


/**************************************************************************/
void getAacConfigSettings(const uint8 *service_caps, a2dp_codec_settings *codec_settings)
{
    /* Set sample rate for both channels based on codec configuration */
    if (service_caps[6] & (1 << 7))
    {
        codec_settings->rate = 48000;
    }
    else if (service_caps[5] & (1 << 0))
    {
        codec_settings->rate = 44100;
    }
    else if (service_caps[5] & (1 << 1))
    {
        codec_settings->rate = 32000;
    }
    else if (service_caps[5] & (1 << 2))
    {
        codec_settings->rate = 24000;
    }
    else if (service_caps[5] & (1 << 3))
    {
        codec_settings->rate = 22050;
    }
    else if (service_caps[5] & (1 << 4))
    {
        codec_settings->rate = 16000;
    }
    else if (service_caps[5] & (1 << 5))
    {
        codec_settings->rate = 12000;
    }
    else if (service_caps[5] & (1 << 6))
    {
        codec_settings->rate = 11025;
    }
    else
    {
        codec_settings->rate = 8000;
    }

    if (service_caps[6] & (1 << 3))
    {   /* Mono */
        codec_settings->channel_mode = a2dp_mono;
    }
    else if (service_caps[6] & ( 1 << 2))
    {   /* Stereo */
        codec_settings->channel_mode = a2dp_stereo;
    }
}
#endif /* A2DP_SBC_ONLY */

