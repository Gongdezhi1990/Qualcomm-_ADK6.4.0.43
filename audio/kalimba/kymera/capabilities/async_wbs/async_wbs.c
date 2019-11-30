/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  async_wbs.c
 * \ingroup  operators
 *
 *  WBS_ENC/WBS_DEC operator common code
 *
 */

/****************************************************************************
Include Files
*/
#include "async_wbs.h"

#include "async_wbs_private.h"
#include "wbs.h"
#include "patch/patch.h"


/****************************************************************************
Public Function Definitions - shared by encoder and decoder
*/

/*
 * Calculate the encoded frame size from the encoder parameters
 */
unsigned awbs_encode_frame_size(sbc_codec *codec_data)
{
    unsigned out_size_octets, bitpool;

    if (codec_data->enc_setting_bitpool > 0)
    {
        bitpool = codec_data->enc_setting_bitpool;
    }
    else
    {
        bitpool = codec_data->bitpool;
    }

    /* nrof_channels = 1;
     * nrof_subbands = 8;
     * nrof_blocks = 15;
     * frame_length = 4 + (4 * nrof_subbands * nrof_channels) / 8 +  ceil((nrof_blocks * nrof_channels * bitpool) / 8);
     * frame_length = 4 + (4 * 8 * 1 ) / 8 + ((15 * 1 * bitpool) + 7) / 8;
     *  */
    out_size_octets = 8 + ((15 * bitpool) + 7) / 8;

    /* The wbs encoded packet contains a 2-byte header apart from the mSBC frame  */
    out_size_octets += 2;

    /* round up to words (16 bit) */
    if (out_size_octets & 0x01)
    {
        out_size_octets ++;
    }

    return out_size_octets;
}
