/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    frame_info.c
*/

#include "frame_info.h"

#define SBC_HEADER_LENGTH           4
#define SBC_HEADER0_SYNC            0x9C
#define SBC_HEADER1_FREQUENCY_MASK  0xC0
#define SBC_HEADER1_FREQUENCY_SHIFT 6
#define SBC_HEADER1_BLOCKS_MASK     0x30
#define SBC_HEADER1_BLOCKS_SHIFT    4
#define SBC_HEADER1_CHANNEL_MASK    0x0C
#define SBC_HEADER1_CHANNEL_MONO    0x00
#define SBC_HEADER1_CHANNEL_DUAL    0x04
#define SBC_HEADER1_CHANNEL_STEREO  0x08
#define SBC_HEADER1_CHANNEL_JOINT   0x0C
#define SBC_HEADER1_ALLOCATION_SNR  0x02
#define SBC_HEADER1_SUBBANDS        0x01
#define bits_to_bytes(num_bits) (((num_bits) + 7) / 8)

/*! Populate frame_info with the result of a historic calculation if a match is found.
    Returns true if a historic match is found. */
static bool frameInfoFromHistorySBC(const uint8 *sbc, frame_info_t *frame_info, frame_info_history_sbc_t *history)
{
    if (history && (sbc[1] == history->last_header1) && (sbc[2] == history->last_header2))
    {
        *frame_info = history->frame_info;
        return TRUE;
    }
    return FALSE;
}

/*! Update frame info history from the current calculation */
static void frameInfoUpdateHistorySBC(const uint8 *sbc, frame_info_t *frame_info, frame_info_history_sbc_t *history)
{
    if (history)
    {
        history->last_header1 = sbc[1];
        history->last_header2 = sbc[2];
        history->frame_info = *frame_info;
    }
}

/*! Calculate the frame info for a SBC frame */
bool frameInfoSBC(const uint8 *sbc, uint32 len, frame_info_t *frame_info, frame_info_history_sbc_t *history)
{
    if (sbc && frame_info && len >= SBC_HEADER_LENGTH)
    {
        if (sbc[0] == SBC_HEADER0_SYNC)
        {
            if (!frameInfoFromHistorySBC(sbc, frame_info, history))
            {
                uint32 length, channels = 2;
                /* extract useful bits */
                uint32 nrof_subbands = (sbc[1] & SBC_HEADER1_SUBBANDS ? 8 : 4);
                uint32 nrof_blocks = 4 * (1 + ((sbc[1] & SBC_HEADER1_BLOCKS_MASK) >> SBC_HEADER1_BLOCKS_SHIFT));

                /* calculate frame length (see A2DP 1.3 specification, section 12.9) */
                length = SBC_HEADER_LENGTH;
                switch (sbc[1] & SBC_HEADER1_CHANNEL_MASK)
                {
                    case SBC_HEADER1_CHANNEL_MONO:
                        channels = 1;
                    case SBC_HEADER1_CHANNEL_DUAL:
                        length += bits_to_bytes(nrof_blocks * channels * sbc[2]);
                        break;

                    case SBC_HEADER1_CHANNEL_STEREO:
                        length += bits_to_bytes(nrof_blocks * sbc[2]);
                        break;

                    case SBC_HEADER1_CHANNEL_JOINT:
                        length += bits_to_bytes(nrof_subbands + nrof_blocks * sbc[2]);
                        break;
                }
                length += bits_to_bytes(4 * nrof_subbands * channels);

                /* calculate number of samples in this frame */
                frame_info->samples = nrof_blocks * nrof_subbands;
                frame_info->length = length;
                
                frameInfoUpdateHistorySBC(sbc, frame_info, history);
            }
            return TRUE;
        }
    }
    return FALSE;
}

bool frameInfoAptx(uint32 len, frame_info_t *frame_info)
{
    /* Standard aptX has a 4:1 compression ratio. This means there is a 1:1
       ratio between the number of octets and the number of 16-bit stereo
       samples in the packet (a 16-bit stereo sample is 4 octets). */
    frame_info->length = len;
    frame_info->samples = len;
    return !!len;
}
