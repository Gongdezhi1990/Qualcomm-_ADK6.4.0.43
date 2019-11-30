/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_output_tws_chain_config.h

DESCRIPTION
    TWS chain config
*/

#ifndef AUDIO_OUTPUT_TWS_CHAIN_CONFIG_H_
#define AUDIO_OUTPUT_TWS_CHAIN_CONFIG_H_

#include "chain.h"
#include "audio_data_types.h"

typedef enum
{
    splitter_role,
    decoder_role,
    encoder_role,
    second_encoder_role,
    ttp_role,
    passthrough_role
} tws_operator_role_t;

typedef enum
{
    left_decoded_audio_input,
    right_decoded_audio_input,
    split_encoded_audio,
    decode_audio,
    encoded_audio_output,
    left_decoded_audio_output,
    right_decoded_audio_ouput,
    mono_encode
} chain_path_role_t;

const chain_config_t* getChainConfig(audio_codec_t source_codec, audio_codec_t dest_codec);


#endif /* AUDIO_OUTPUT_TWS_CHAIN_CONFIG_H_ */
