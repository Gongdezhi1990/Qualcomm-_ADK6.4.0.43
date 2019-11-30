/****************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_a2dp_chain_config_stereo.h

DESCRIPTION
    Stereo chain configuration
*/

#ifndef AUDIO_INPUT_A2DP_CHAIN_CONFIG_STEREO_H_
#define AUDIO_INPUT_A2DP_CHAIN_CONFIG_STEREO_H_

#include <chain.h>
#include <audio_data_types.h>

kymera_chain_handle_t audioInputA2dpCreateStereoChain(audio_codec_t decoder);
kymera_chain_handle_t audioInputA2dpCreateStereoChainWithoutSplitter(audio_codec_t decoder);

#endif /* AUDIO_INPUT_A2DP_CHAIN_CONFIG_STEREO_H_ */
