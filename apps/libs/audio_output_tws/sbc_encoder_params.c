/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

 
FILE NAME
    sbc_encoder_params.c
 
DESCRIPTION
    Implementation of SBC encoder parameters converter.
*/

#include "sbc_encoder_params.h"
#include <print.h>

static unsigned sbcEncoderParamsGetNumberOfSubbands(unsigned settings);
static unsigned sbcEncoderParamsGetNumberOfBlocks(unsigned settings);
static unsigned sbcEncoderParamsGetSampleRate(unsigned settings);
static sbc_encoder_channel_mode_t sbcEncoderParamsGetChannelMode(unsigned settings);
static sbc_encoder_allocation_method_t sbcEncoderParamsGetAllocationMethod(unsigned settings);

sbc_encoder_params_t ConvertSbcEncoderParams(unsigned bitpool, unsigned settings)
{
    sbc_encoder_params_t sbc_params;

    sbc_params.number_of_subbands = sbcEncoderParamsGetNumberOfSubbands(settings);
    sbc_params.number_of_blocks = sbcEncoderParamsGetNumberOfBlocks(settings);
    sbc_params.bitpool_size = bitpool;
    sbc_params.sample_rate = sbcEncoderParamsGetSampleRate(settings);
    sbc_params.channel_mode = sbcEncoderParamsGetChannelMode(settings);
    sbc_params.allocation_method = sbcEncoderParamsGetAllocationMethod(settings);

    PRINT(("SBC sample rate 0x%x, channel_mode 0x%x, bitpool_size 0x%x \n",  sbc_params.sample_rate, sbc_params.channel_mode, sbc_params.bitpool_size));
    PRINT(("SBC number_of_blocks 0x%x, subbands 0x%x, allocation_method 0x%x\n",  sbc_params.number_of_blocks, sbc_params.number_of_subbands, sbc_params.allocation_method));

    return sbc_params;
}

/****************************************************************************
DESCRIPTION
    Extracts number of subbands from sink application's format of SBC encoder settings.
*/
static unsigned sbcEncoderParamsGetNumberOfSubbands(unsigned settings)
{
    return (settings & 0x1) ? 8 : 4;;
}

/****************************************************************************
DESCRIPTION
    Extracts number of blocks from sink application's format of SBC encoder settings.
*/
static unsigned sbcEncoderParamsGetNumberOfBlocks(unsigned settings)
{
    unsigned number_of_blocks = 0;

    switch((settings >> 4) & 0x3)
    {
    case 0:
        number_of_blocks = 4;
        break;
    case 1:
        number_of_blocks = 8;
        break;
    case 2:
        number_of_blocks = 12;
        break;
    case 3:
        number_of_blocks = 16;
        break;
    default:
        break;
    }

    return number_of_blocks;
}

/****************************************************************************
DESCRIPTION
    Extracts sample rate from sink application's format of SBC encoder settings.
*/
static unsigned sbcEncoderParamsGetSampleRate(unsigned settings)
{
    unsigned sample_rate = 0;

    switch((settings >> 6) & 0x3)
    {
    case 0:
        sample_rate = 16000;
        break;
    case 1:
        sample_rate = 32000;
        break;
    case 2:
        sample_rate = 44100;
        break;
    case 3:
        sample_rate = 48000;
        break;
    default:
        break;
    }

    return sample_rate;
}

/****************************************************************************
DESCRIPTION
    Extracts channel mode from sink application's format of SBC encoder settings.
*/
static sbc_encoder_channel_mode_t sbcEncoderParamsGetChannelMode(unsigned settings)
{
    sbc_encoder_channel_mode_t channel_mode = 0;

    switch((settings >> 2) & 0x3)
    {
    case 0:
        channel_mode = sbc_encoder_channel_mode_mono;
        break;
    case 1:
        channel_mode = sbc_encoder_channel_mode_dual_mono;
        break;
    case 2:
        channel_mode = sbc_encoder_channel_mode_stereo;
        break;
    case 3:
        channel_mode = sbc_encoder_channel_mode_joint_stereo;
        break;
    default:
        break;
    }

    return channel_mode;
}

/****************************************************************************
DESCRIPTION
    Extracts allocation method from sink application's format of SBC encoder settings.
*/
static sbc_encoder_allocation_method_t sbcEncoderParamsGetAllocationMethod(unsigned settings)
{
    return ((settings >> 1) & 0x1) ?
            sbc_encoder_allocation_method_snr : sbc_encoder_allocation_method_loudness;
}
