/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    a2dp_codec_csr_faststream.c

DESCRIPTION
    This file contains

NOTES

*/

/****************************************************************************
    Header files
*/
#include "a2dp.h"
#include "a2dp_private.h"
#include "a2dp_codec_csr_faststream.h"
#include "a2dp_caps_parse.h"

#ifndef A2DP_SBC_ONLY

/**************************************************************************/
void selectOptimalCsrFastStreamCapsSink(const uint8 *local_codec_caps, uint8 *remote_codec_caps)
{
    /* Choose what is supported at both sides */
    remote_codec_caps[10] = (remote_codec_caps[10] & (FASTSTREAM_MUSIC | FASTSTREAM_VOICE)) & (local_codec_caps[10] & (FASTSTREAM_MUSIC | FASTSTREAM_VOICE));

    /* Currently only support 48.0kHz, 44.1kHz for Music and 16kHz for Voice */
    remote_codec_caps[11] = (remote_codec_caps[11] & (FASTSTREAM_MUSIC_SAMP_48000 |  FASTSTREAM_MUSIC_SAMP_44100 | FASTSTREAM_VOICE_SAMP_16000)) &
                            (local_codec_caps[11] & (FASTSTREAM_MUSIC_SAMP_48000 |  FASTSTREAM_MUSIC_SAMP_44100 | FASTSTREAM_VOICE_SAMP_16000));
}


/**************************************************************************/
void selectOptimalCsrFastStreamCapsSource(const uint8 *local_codec_caps, uint8 *remote_codec_caps)
{
    /* Defer to remote_caps in both cases, so use same function */
    selectOptimalCsrFastStreamCapsSink(local_codec_caps,remote_codec_caps);
}


/**************************************************************************/
void getCsrFastStreamConfigSettings(const uint8 *service_caps, a2dp_role_type role, a2dp_codec_settings *codec_settings)
{
    if (service_caps[10] & FASTSTREAM_MUSIC)
    {
        if (service_caps[11] & FASTSTREAM_MUSIC_SAMP_48000 )
        {
            codec_settings->rate = 48000;

            switch (role)
            {
                case a2dp_source:
                    /* Configure the SBC format:
                       48.0kHz, Blocks 16, Sub-bands 8, Joint Stereo, Loudness, Bitpool = 29
                       (data rate = 212kbps, packet size = 72*3+4 = 220 = DM5).

                       Note the DSP rounds the 71byte frames to 72bytes.
                    */
                    codec_settings->codecData.format = 0xfd;
                    codec_settings->codecData.bitpool = 29;
                    break;

                case a2dp_sink:
                case a2dp_role_undefined:
                    break;
            }
        }
        else if (service_caps[11] & FASTSTREAM_MUSIC_SAMP_44100)
        {
            codec_settings->rate = 44100;

            switch (role)
            {
                case a2dp_source:
                    /* Configure the SBC format:
                       44.1kHz, Blocks 16, Sub-bands 8, Joint Stereo, Loudness, Bitpool = 29
                       (data rate = 195kbps, packet size = 72*3+4 = 220 = DM5).

                       Note the DSP rounds the 71byte frames to 72bytes.
                    */
                    codec_settings->codecData.format = 0xbd;
                    codec_settings->codecData.bitpool = 29;
                    break;

                case a2dp_sink:
                case a2dp_role_undefined:
                    break;
            }
        }
    }
    else
	{
        codec_settings->rate = 0;
	}

    if (service_caps[10] & FASTSTREAM_VOICE)
    {
        if (service_caps[11] & FASTSTREAM_VOICE_SAMP_16000)
        {
            codec_settings->codecData.voice_rate = 16000;

            switch (role)
            {
                case a2dp_sink:
                    /*
                      Configure the SBC format for the microphone data
                      16kHz, Mono, Blocks 16, Sub-bands 8, Loudness, Bitpool = 32
                      (data rate = 72kbps, packet size = 3*72 + 4 = 220 <= DM5).
                    */
                    codec_settings->codecData.format = 0x31;
                    codec_settings->codecData.bitpool = 32;
                    break;

                case a2dp_source:
                case a2dp_role_undefined:
                    break;
            }
        }
    }
    else
	{
        codec_settings->codecData.voice_rate = 0;
	}

    codec_settings->channel_mode = a2dp_joint_stereo;
}
#endif
