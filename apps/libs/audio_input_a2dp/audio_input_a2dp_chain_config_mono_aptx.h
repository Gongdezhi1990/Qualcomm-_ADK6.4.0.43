/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_a2dp_chain_config_mono_aptx.h

DESCRIPTION
    Get the appropriate aptx mono chain
*/

#ifndef AUDIO_INPUT_A2DP_CHAIN_CONFIG_MONO_APTX_H_
#define AUDIO_INPUT_A2DP_CHAIN_CONFIG_MONO_APTX_H_

#include <chain.h>
#include <audio_data_types.h>

kymera_chain_handle_t audioInputA2dpCreateMonoAptxChain(audio_codec_t decoder);

#endif /* AUDIO_INPUT_A2DP_CHAIN_CONFIG_MONO_APTX_H_ */
