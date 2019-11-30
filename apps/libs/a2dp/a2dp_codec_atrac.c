/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    a2dp_codec_atrac.c

DESCRIPTION
    This file contains

NOTES

*/

/****************************************************************************
    Header files
*/
#include "a2dp.h"

#ifdef INCLUDE_ATRAC
#include "a2dp_private.h"
#include "a2dp_caps_parse.h"
#include "a2dp_codec_atrac.h"


/*************************************************************************/
void getAtracConfigSettings(const uint8 *service_caps, a2dp_codec_settings *codec_settings)
{
    if (!service_caps)
    {
        codec_settings->rate = 0;
        codec_settings->channel_mode = a2dp_mono;
        return;
    }

    if (service_caps[4] & 0x10)
    {   /* Mono */
        codec_settings->channel_mode = a2dp_mono;
    }
    else if (service_caps[4] & 0x08)
    {   /* Stereo - dual channel */
        codec_settings->channel_mode = a2dp_dual_channel;
    }
    else
    {   /* Stereo - joint */
        codec_settings->channel_mode = a2dp_joint_stereo;
    }

    /* Work out the sample rate based on the codec configuration. */
    if (service_caps[5] & 0x20)
    {
        codec_settings->rate = 44100;
    }
    else
    {
        codec_settings->rate = 48000;
    }
}


/**************************************************************************/
void selectOptimalAtracCapsSink(const uint8 *local_codec_caps, uint8 *remote_codec_caps)
{
    /* Assume ATRAC version has only one of the bits set, so don't process the version */

    /* Select Channel Mode */
    if ((remote_codec_caps[4] & 0x04) && (local_codec_caps[4] & 0x04))
    {   /* choose joint stereo */
        remote_codec_caps[4] &= 0xe4;
    }
    else if ((remote_codec_caps[4] & 0x08) && (local_codec_caps[4] & 0x08))
    {   /* choose dual channel */
        remote_codec_caps[4] &= 0xe8;
    }
    else
    {   /* choose single channel */
        remote_codec_caps[4] &= 0xf0;
    }

    /* Select sample frequency */
    if ((remote_codec_caps[5] & 0x10) && (local_codec_caps[5] & 0x10))
    {   /* choose 48k */
        remote_codec_caps[5] &= 0x1f;
    }
    else
    {   /* choose 44k1 */
        remote_codec_caps[5] &= 0x2f;
    }

    /* Use VBR if available, hence leave bit alone */

    /* Choose a bit rate index. Probably need to have different preference order. */
    if ((remote_codec_caps[5] & 0x04) && (local_codec_caps[5] & 0x04))
    {   /* 0x0000 */
        remote_codec_caps[5] &= 0x3c;
        remote_codec_caps[6] = 0;
        remote_codec_caps[7] = 0;
    }
    else if ((remote_codec_caps[5] & 0x02) && (local_codec_caps[5] & 0x02))
    {   /* 0x0001 */
        remote_codec_caps[5] &= 0x3a;
        remote_codec_caps[6] = 0;
        remote_codec_caps[7] = 0;
    }
    else if ((remote_codec_caps[5] & 0x01) && (local_codec_caps[5] & 0x01))
    {   /* 0x0002 */
        remote_codec_caps[5] &= 0x39;
        remote_codec_caps[6] = 0;
        remote_codec_caps[7] = 0;
    }
    else if ((remote_codec_caps[6] & 0x80) && (local_codec_caps[6] & 0x80))
    {   /* 0x0003 */
        remote_codec_caps[6] &= 0x80;
        remote_codec_caps[5] &= 38;
        remote_codec_caps[7] = 0;
    }
    else if ((remote_codec_caps[6] & 0x40) && (local_codec_caps[6] & 0x40))
    {   /* 0x0004 */
        remote_codec_caps[6] &= 0x40;
        remote_codec_caps[5] &= 38;
        remote_codec_caps[7] = 0;
    }
    else if ((remote_codec_caps[6] & 0x20) && (local_codec_caps[6] & 0x20))
    {   /* 0x0005 */
        remote_codec_caps[6] &= 0x20;
        remote_codec_caps[5] &= 38;
        remote_codec_caps[7] = 0;
    }
    else if ((remote_codec_caps[6] & 0x10) && (local_codec_caps[6] & 0x10))
    {   /* 0x0006 */
        remote_codec_caps[6] &= 0x10;
        remote_codec_caps[5] &= 38;
        remote_codec_caps[7] = 0;
    }
    else if ((remote_codec_caps[6] & 0x08) && (local_codec_caps[6] & 0x08))
    {   /* 0x0007 */
        remote_codec_caps[6] &= 0x08;
        remote_codec_caps[5] &= 38;
        remote_codec_caps[7] = 0;
    }
    else if ((remote_codec_caps[6] & 0x04) && (local_codec_caps[6] & 0x04))
    {   /* 0x0008 */
        remote_codec_caps[6] &= 0x04;
        remote_codec_caps[5] &= 38;
        remote_codec_caps[7] = 0;
    }
    else if ((remote_codec_caps[6] & 0x02) && (local_codec_caps[6] & 0x02))
    {   /* 0x0009 */
        remote_codec_caps[6] &= 0x02;
        remote_codec_caps[5] &= 38;
        remote_codec_caps[7] = 0;
    }
    else if ((remote_codec_caps[6] & 0x01) && (local_codec_caps[6] & 0x01))
    {   /* 0x000a */
        remote_codec_caps[6] &= 0x01;
        remote_codec_caps[5] &= 38;
        remote_codec_caps[7] = 0;
    }
    else if ((remote_codec_caps[7] & 0x80) && (local_codec_caps[7] & 0x80))
    {   /* 0x000b */
        remote_codec_caps[7] &= 0x80;
        remote_codec_caps[5] &= 38;
        remote_codec_caps[6] = 0;
    }
    else if ((remote_codec_caps[7] & 0x40) && (local_codec_caps[7] & 0x40))
    {   /* 0x000c */
        remote_codec_caps[7] &= 0x40;
        remote_codec_caps[5] &= 38;
        remote_codec_caps[6] = 0;
    }
    else if ((remote_codec_caps[7] & 0x20) && (local_codec_caps[7] & 0x20))
    {   /* 0x000d */
        remote_codec_caps[7] &= 0x20;
        remote_codec_caps[5] &= 38;
        remote_codec_caps[6] = 0;
    }
    else if ((remote_codec_caps[7] & 0x10) && (local_codec_caps[7] & 0x10))
    {   /* 0x000e */
        remote_codec_caps[7] &= 0x10;
        remote_codec_caps[5] &= 38;
        remote_codec_caps[6] = 0;
    }
    else if ((remote_codec_caps[7] & 0x08) && (local_codec_caps[7] & 0x08))
    {   /* 0x000f */
        remote_codec_caps[7] &= 0x08;
        remote_codec_caps[5] &= 38;
        remote_codec_caps[6] = 0;
    }
    else if ((remote_codec_caps[7] & 0x04) && (local_codec_caps[7] & 0x04))
    {   /* 0x0010 */
        remote_codec_caps[7] &= 0x04;
        remote_codec_caps[5] &= 38;
        remote_codec_caps[6] = 0;
    }
    else if ((remote_codec_caps[7] & 0x02) && (local_codec_caps[7] & 0x02))
    {   /* 0x0011 */
        remote_codec_caps[7] &= 0x02;
        remote_codec_caps[5] &= 38;
        remote_codec_caps[6] = 0;
    }
    else
    {   /* 0x0012 */
        remote_codec_caps[7] &= 0x01;
        remote_codec_caps[5] &= 38;
        remote_codec_caps[6] = 0;
    }
}

#endif /* INCLUDE_ATRAC */
