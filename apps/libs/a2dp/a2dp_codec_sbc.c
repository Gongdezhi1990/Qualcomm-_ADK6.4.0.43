/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    a2dp_codec_sbc.c

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
#include "a2dp_codec_sbc.h"



/* SBC header defines */
#define SBC_HEADER_CHANNEL_SHIFT    (0x02)
#define SBC_HEADER_CHANNEL_MASK     (0x0c)
#define SBC_HEADER_BLOCKS_SHIFT     (0x04)
#define SBC_HEADER_BLOCKS_MASK      (0x30)
#define SBC_HEADER_FREQUENCY_SHIFT  (0x06)
#define SBC_HEADER_FREQUENCY_MASK   (0xc0)
#define SBC_HEADER_SUBBANDS_8       (0x01)
#define SBC_HEADER_ALLOCATION_SNR   (0x02)



typedef struct
{
    uint8                   sub_bands;
    uint8                   blocks;
    uint16                  frequency;
    a2dp_channel_mode       mode;
} sbc_parameters;



/*****************************************************************************/
static void extractSbcParameters(sbc_parameters *sbc, uint8 sbc_format)
{
    /* Takes an SBC header and decodes the parameters. */
    const uint16 freq_lut[] = {16000,32000,44100,48000};
    sbc->sub_bands = (sbc_format & SBC_HEADER_SUBBANDS_8?8:4);
    sbc->blocks = (uint8)(4*(1+((sbc_format & SBC_HEADER_BLOCKS_MASK) >> SBC_HEADER_BLOCKS_SHIFT)));
    sbc->frequency  = freq_lut[(sbc_format & SBC_HEADER_FREQUENCY_MASK) >> SBC_HEADER_FREQUENCY_SHIFT];
    sbc->mode = (a2dp_channel_mode) ((sbc_format & SBC_HEADER_CHANNEL_MASK) >> SBC_HEADER_CHANNEL_SHIFT);
}


/*****************************************************************************/
static uint16 frameLengthFromRate(const sbc_parameters *sbc, uint32 rate)
{
    /* Returns the frame length needed to produce data at the specified rate */
    return (uint16)((rate * (uint32)sbc->sub_bands * (uint32)sbc->blocks) / ((uint32)8 * (uint32)sbc->frequency));
}


/*****************************************************************************/
static uint8 bitpoolFromFrameLength(const sbc_parameters *sbc, uint16 frame_length)
{
    /* Calculates the bitpool required to produce the requested frame length */
    unsigned bitpool;

    /* Equations derived from A2DP Spec, Section 12.9. */
    switch(sbc->mode)
    {
        default:
        case a2dp_mono:
            bitpool = (8*(frame_length - 4) - 4*sbc->sub_bands) / sbc->blocks;
            break;

        case a2dp_dual_channel:
            bitpool = (8*(frame_length - 4) - 8*sbc->sub_bands) / (2*sbc->blocks);
            break;

        case a2dp_stereo:
            bitpool = (8*(frame_length - 4) - 8*sbc->sub_bands) / sbc->blocks;
            break;

        case a2dp_joint_stereo:
            bitpool = (8*(frame_length - 4) - 9*sbc->sub_bands) / sbc->blocks;
            break;
    }

    return (uint8)bitpool;
}


static void selectOptimalCommonSbcCaps(const uint8 *local_codec_caps, uint8 *remote_codec_caps)
{
    /* Select Channel Mode. Only mono is mandatory at the source. */
    if ((remote_codec_caps[4] & 0x01) && (local_codec_caps[4] & 0x01))
    {   /* choose joint stereo */
        remote_codec_caps[4] &= 0xf1;
    }
    else if ((remote_codec_caps[4] & 0x02) && (local_codec_caps[4] & 0x02))
    {   /* choose stereo */
        remote_codec_caps[4] &= 0xf2;
    }
    else if ((remote_codec_caps[4] & 0x04) && (local_codec_caps[4] & 0x04))
    {   /* choose dual channel */
        remote_codec_caps[4] &= 0xf4;
    }
    else
    {   /* choose mono */
        remote_codec_caps[4] &= 0xf8;
    }

    /* Select Block Length. All lengths are mandatory at Source and Sink. */
    if (remote_codec_caps[5] & 0x10)
    {   /* choose 16 */
        remote_codec_caps[5] &= 0x1f;
    }
    else if (remote_codec_caps[5] & 0x20)
    {   /* choose 12 */
        remote_codec_caps[5] &= 0x2f;
    }
    else if (remote_codec_caps[5] & 0x40)
    {   /* choose 8 */
        remote_codec_caps[5] &= 0x4f;
    }
    else
    {   /* choose 4 */
        remote_codec_caps[5] &= 0x8f;
    }

    /* Select Subbands. 8 subbands is mandatory at both Source and Sink. */
    if (remote_codec_caps[5] & 0x04)
    {   /* choose 8 */
        remote_codec_caps[5] &= 0xf7;
    }
    else
    {   /* choose 4 */
        remote_codec_caps[5] &= 0xfb;
    }

    /* Select Allocation Method. Loudness is mandatory at both Source and Sink. */
    if (remote_codec_caps[5] & 0x01)
    {   /* choose Loudness */
        remote_codec_caps[5] &= 0xfd;
    }
    else
    {   /* choose SNR */
        remote_codec_caps[5] &= 0xfe;
    }
        
    /* Clamp min bitpool size */
    if (local_codec_caps[6] > remote_codec_caps[6])
    {
        remote_codec_caps[6] = local_codec_caps[6];
    }
    
    /* Clamp max bitpool size */
    if (local_codec_caps[7] < remote_codec_caps[7])
    {
        remote_codec_caps[7] = local_codec_caps[7];
    }
}


/**************************************************************************/
void selectOptimalSbcCapsSink(const uint8 *local_codec_caps, uint8 *remote_codec_caps)
{
    /* All codec remote_codec_caps are mandatory at the Sink end except the sample rates where only
        48k and 44.1k are mandatory, but at least one of these must be supported at the Source end.
    */
    
    /* Select sample frequency */
    if (remote_codec_caps[4] & 0x20)
    {   /* choose 44k1 */
        remote_codec_caps[4] &= 0x2f;
    }
    else if (remote_codec_caps[4] & 0x10)
    {   /* choose 48k */
        remote_codec_caps[4] &= 0x1f;
    }
	else if (remote_codec_caps[4] & 0x40)
    {   /* choose 32k */
        remote_codec_caps[4] &= 0x4f;
    }
    else
    {   /* choose 16k */
        remote_codec_caps[4] &= 0x8f;
    }

    selectOptimalCommonSbcCaps(local_codec_caps, remote_codec_caps);
}


/**************************************************************************/
void selectOptimalSbcCapsSource(const uint8 *local_codec_caps, uint8 *remote_codec_caps)
{
    /*
        Select SBC parameters for optimal performance. We now own the
        malloc block containing the configure so clear the unwanted
        fields and pass it back. Note that the library validates that
        our capabilities are compatible hence there is also at least
        one overlapping bit per field.
    */

    /* Select sample rates based on what is supported at boths ends.
        At least one of 48k and 44.1k must be supported at both ends.*/
    if ((remote_codec_caps[4] & 0x10) && (local_codec_caps[4] & 0x10))
    {   /* choose 48kHz */
        remote_codec_caps[4] &= 0x1f;
    }
    else if ((remote_codec_caps[4] & 0x20) && (local_codec_caps[4] & 0x20))
    {   /* choose 44kHz */
        remote_codec_caps[4] &= 0x2f;
    }
    else if ((remote_codec_caps[4] & 0x40) && (local_codec_caps[4] & 0x40))
    {   /* choose 32kHz */
        remote_codec_caps[4] &= 0x4f;
    }
    else
    {   /* choose 16kHz */
        remote_codec_caps[4] &= 0x8f;
    }

    selectOptimalCommonSbcCaps(local_codec_caps, remote_codec_caps);
}


/*************************************************************************/
void getSbcConfigSettings(const uint8 *service_caps, a2dp_codec_settings *codec_settings)
{
    if (!service_caps)
    {
        codec_settings->rate = 0;
        codec_settings->channel_mode = a2dp_mono;
        return;
    }

    /* Work out the sample rate based on the codec configuration. */
    if (service_caps[4] & 0x10)
    {
        codec_settings->rate = 48000;
    }
    else if (service_caps[4] & 0x20)
    {
        codec_settings->rate = 44100;
    }
    else if (service_caps[4] & 0x40)
    {
        codec_settings->rate = 32000;
    }
    else
    {
        codec_settings->rate = 16000;
    }

    if (service_caps[4] & 0x08)
    {   /* Mono */
        codec_settings->channel_mode = a2dp_mono;
    }
    else if (service_caps[4] & 0x04)
    {   /* Stereo - dual channel */
        codec_settings->channel_mode = a2dp_dual_channel;
    }
    else if (service_caps[4] & 0x02)
    {   /* Stereo */
        codec_settings->channel_mode = a2dp_stereo;
    }
    else
    {   /* Stereo - joint */
        codec_settings->channel_mode = a2dp_joint_stereo;
    }
}


/*****************************************************************************/
uint8 a2dpSbcFormatFromConfig(const uint8 *config)
{
    uint8 sbc_format;

    /* sample frequency */
    if (config[4] & 0x80)
    {
        sbc_format = 0 << SBC_HEADER_FREQUENCY_SHIFT;
    }
    else if (config[4] & 0x40)
    {
        sbc_format = 1 << SBC_HEADER_FREQUENCY_SHIFT;
    }
    else if (config[4] & 0x20)
    {
        sbc_format = 2 << SBC_HEADER_FREQUENCY_SHIFT;
    }
    else
    {
        sbc_format = 3 << SBC_HEADER_FREQUENCY_SHIFT;
    }

    /* channel mode */
    if (config[4] & 0x08)
    {
        sbc_format |= ((uint8) a2dp_mono) << SBC_HEADER_CHANNEL_SHIFT;
    }
    else if (config[4] & 0x04)
    {
        sbc_format |= ((uint8) a2dp_dual_channel) << SBC_HEADER_CHANNEL_SHIFT;
    }
    else if (config[4] & 0x02)
    {
        sbc_format |= ((uint8) a2dp_stereo) << SBC_HEADER_CHANNEL_SHIFT;
    }
    else
    {
        sbc_format |= ((uint8) a2dp_joint_stereo) << SBC_HEADER_CHANNEL_SHIFT;
    }

    /* block length */
    if (config[5] & 0x80)
    {
        sbc_format |= 0 << SBC_HEADER_BLOCKS_SHIFT;
    }
    else if (config[5] & 0x40)
    {
        sbc_format |= 1 << SBC_HEADER_BLOCKS_SHIFT;
    }
    else if (config[5] & 0x20)
    {
        sbc_format |= 2 << SBC_HEADER_BLOCKS_SHIFT;
    }
    else
    {
        sbc_format |= 3 << SBC_HEADER_BLOCKS_SHIFT;
    }

    /* sub bands */
    if (config[5] & 0x04)
    {
        sbc_format |= SBC_HEADER_SUBBANDS_8;
    }

    /* allocation method */
    if (config[5] & 0x02)
    {
        sbc_format |= SBC_HEADER_ALLOCATION_SNR;
    }

    return sbc_format;
}


/*****************************************************************************/
uint8 a2dpSbcSelectBitpool(uint8 sbc_header, uint32 rate, uint16 pdu)
{
    sbc_parameters sbc;
    unsigned frame_length;
    unsigned frames;

    /* Extract sbc_header into more usual structure */
    extractSbcParameters(&sbc, sbc_header);

    /*
       In order to use the radio bandwidth most efficiently we want to send full
       L2CAP packets.  To achieve this we round down the frame length so that
       an integer multiple of frames fills the PDU.
    */
    frame_length = frameLengthFromRate(&sbc, rate);

    /* Deduct RTP and SBC payload headers */
    pdu -= 13;

    /* Calculate how many frames fit - rounding up */
    frames = (pdu + frame_length-1) / frame_length;

    /* Adjust the frame length so it fills the packet */
    frame_length = pdu / frames;

    /* Get bitpool to achieve this frame length */
    return bitpoolFromFrameLength(&sbc, (uint16)frame_length);
}
