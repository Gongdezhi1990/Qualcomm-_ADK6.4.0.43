/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_aptx_adaptive_encoder_params.h

DESCRIPTION
    Definitions of AptX Adaptive Encoder Parameters.
*/

#ifndef AUDIO_APTX_ADAPTIVE_ENCODER_PARAMS_H_
#define AUDIO_APTX_ADAPTIVE_ENCODER_PARAMS_H_

typedef struct
{
    unsigned quality;
    unsigned dh5_dh3;
    unsigned channel;
    unsigned bitrate;
    unsigned sample_rate;
} aptxad_encoder_params_t;

#endif /* AUDIO_APTX_ADAPTIVE_ENCODER_PARAMS_H_ */

