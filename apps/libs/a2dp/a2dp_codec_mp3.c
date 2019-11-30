/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    a2dp_codec_mp3.c

DESCRIPTION
    This file contains

NOTES

*/

/****************************************************************************
    Header files
*/
#include "a2dp.h"
#include "a2dp_caps_parse.h"
#include "a2dp_private.h"
#include "a2dp_codec_mp3.h"

#ifndef A2DP_SBC_ONLY

/* Channel Mode */
/* These tables are conditions. If A & B then C.
    A & C refer to remote caps, B local.
    0x0401 Tests if caps[4] & 0x01 
*/
static const uint16 mp3CapsChannelMode[] =
{
    0x0401, 0x0401, 0x04F1, 0xFFFF,
    0x0402, 0x0402, 0x04F2, 0xFFFF,
    0x0404, 0x0404, 0x04F4, 0xFFFF,
    0x04FF, 0x04FF, 0x04F8, 0xFFFF,
    0xFFFF
};

/* Sample Rate */
static const uint16 mp3CapsSampleRate[] =
{
    0x0502, 0x0502, 0x05C2, 0xFFFF,
    0x0501, 0x0501, 0x05C1, 0xFFFF,
    0x0504, 0x0504, 0x05C4, 0xFFFF,
    0x0508, 0x0508, 0x05C8, 0xFFFF,
    0x0510, 0x0510, 0x05D0, 0xFFFF,
    0x05FF, 0x05FF, 0x05E0, 0xFFFF,
    0xFFFF
};


/* Bit Rate.                                                                                       */
/* We try and use the rate nearest to 128kbps.  In reality the source won't encode MP3 in realtime */
/* so it will probably only have a single bit set representing it's pre-compressed file.           */
static const uint16 mp3CapsBitRate[] =
{
    0x0602, 0x0602, 0x0682, 0x0700,
    0x0601, 0x0601, 0x0681, 0x0700,
    0x0604, 0x0604, 0x0684, 0x0700,
    0x0780, 0x0780, 0x0680, 0x0780,
    0x0608, 0x0608, 0x0688, 0x0700,
    0x0740, 0x0740, 0x0680, 0x0740,
    0x0610, 0x0610, 0x0690, 0x0700,
    0x0720, 0x0720, 0x0680, 0x0720,
    0x0620, 0x0620, 0x06A0, 0x0700,
    0x0710, 0x0710, 0x0680, 0x0710,
    0x0640, 0x0640, 0x06C0, 0x0700,
    0x0708, 0x0708, 0x0680, 0x0708,
    0x0704, 0x0704, 0x0680, 0x0704,
    0x0702, 0x0702, 0x0680, 0x0702,
    0xFFFF
};

static const uint16 *const mp3CapsTable[] =
{
    mp3CapsChannelMode,
    mp3CapsSampleRate,
    mp3CapsBitRate,
    (const uint16 *)0xFFFF
};

/** Process the condition tabls to update the remote caps.
 *  Each table is processed in turn, moving to the next as soon 
 *  as a match is found */
static void selectMp3Caps(const uint8 *local_codec_caps, uint8 *remote_codec_caps)
{
    const uint16 * const * caps_table = &mp3CapsTable[0];
    
    while (*caps_table != (uint16 *)0xFFFF)
    {
        const uint16 *parse_table = *caps_table;

        while (parse_table[0] != 0xFFFF)
        {
            if ((remote_codec_caps[(uint8)(parse_table[0]>>8)] & (uint8)parse_table[0]) && (local_codec_caps[(uint8)(parse_table[1]>>8)] & (uint8)parse_table[1]))
            {
                remote_codec_caps[(uint8)(parse_table[2]>>8)] &= (uint8)parse_table[2];

                if (parse_table[3] != 0xFFFF)
                {   /* Dealing with bit rate caps */
                    remote_codec_caps[(uint8)(parse_table[3]>>8)] &= (uint8)parse_table[3];
                    remote_codec_caps[6] &= (local_codec_caps[6] | 0x7F);  /* VBR */
                }

                break;
            }

            parse_table += 4;
        }

        caps_table++;
    }
}

/**************************************************************************/
void selectOptimalMp3CapsSink(const uint8 *local_codec_caps, uint8 *remote_codec_caps)
{
    if (!local_codec_caps || !remote_codec_caps)
        return;

    selectMp3Caps(local_codec_caps, remote_codec_caps);
}


/**************************************************************************/
void selectOptimalMp3CapsSource(const uint8 *local_codec_caps, uint8 *remote_codec_caps)
{
    selectOptimalMp3CapsSink(local_codec_caps, remote_codec_caps);
}


/**************************************************************************/
void getMp3ConfigSettings(const uint8 *service_caps, a2dp_codec_settings *codec_settings)
{
    if (!service_caps)
    {
        codec_settings->rate = 0;
        codec_settings->channel_mode = a2dp_mono;
        return;
    }

    /* Set sample rate for both channels based on codec configuration */
    if (service_caps[5] & 0x01)
        codec_settings->rate = 48000;
    else if (service_caps[5] & 0x02)
        codec_settings->rate = 44100;
    else if (service_caps[5] & 0x04)
        codec_settings->rate = 32000;
    else if (service_caps[5] & 0x08)
        codec_settings->rate = 24000;
    else if (service_caps[5] & 0x10)
        codec_settings->rate = 22050;
    else
        codec_settings->rate = 16000;

    if (service_caps[4] & 0x08)
        /* Mono */
        codec_settings->channel_mode = a2dp_mono;
    else if (service_caps[4] & 0x04)
        /* Stereo - dual channel */
        codec_settings->channel_mode = a2dp_dual_channel;
    else if (service_caps[4] & 0x02)
        /* Stereo */
        codec_settings->channel_mode = a2dp_stereo;
    else
        /* Stereo - joint */
        codec_settings->channel_mode = a2dp_joint_stereo;
}
#endif
