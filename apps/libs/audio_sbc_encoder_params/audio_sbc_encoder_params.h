/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_sbc_encoder_params.h

DESCRIPTION
    Definitions of SBC Encoder Parameters.
*/

#ifndef AUDIO_SBC_ENCODER_PARAMS_H_
#define AUDIO_SBC_ENCODER_PARAMS_H_


typedef enum
{
    sbc_encoder_channel_mode_mono = 0,
    sbc_encoder_channel_mode_dual_mono,
    sbc_encoder_channel_mode_stereo,
    sbc_encoder_channel_mode_joint_stereo
} sbc_encoder_channel_mode_t;

typedef enum
{
    sbc_encoder_allocation_method_loudness = 0,
    sbc_encoder_allocation_method_snr
} sbc_encoder_allocation_method_t;

typedef struct
{
    unsigned number_of_subbands;
    unsigned number_of_blocks;
    unsigned bitpool_size;
    unsigned sample_rate;
    sbc_encoder_channel_mode_t channel_mode;
    sbc_encoder_allocation_method_t allocation_method;
} sbc_encoder_params_t;

#endif /* AUDIO_SBC_ENCODER_PARAMS_H_ */

