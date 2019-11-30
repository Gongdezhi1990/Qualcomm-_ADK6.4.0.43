/****************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_a2dp_chain_config.h

DESCRIPTION
    Create appropriate chain for audio_input_a2dp
*/

#ifndef AUDIO_INPUT_A2DP_CHAIN_CONFIG_H_
#define AUDIO_INPUT_A2DP_CHAIN_CONFIG_H_

#include "audio_input_a2dp_chain_config_common.h"
#include <chain.h>
#include <audio_data_types.h>

/****************************************************************************
DESCRIPTION
    Create the appropriate chain based on the sink configuration (mono or stereo channel rendering)
*/
kymera_chain_handle_t audioInputA2dpCreateChain(audio_codec_t decoder);

/****************************************************************************
DESCRIPTION
    Create the appropriate chain for a broadcaster
*/
kymera_chain_handle_t audioInputA2dpCreateBroadcasterChain(audio_codec_t decoder);

#endif /* AUDIO_INPUT_A2DP_CHAIN_CONFIG_H_ */
