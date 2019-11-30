/****************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_tws_chain_config.h

DESCRIPTION
    Create appropriate chain for audio_input_tws
*/

#ifndef AUDIO_INPUT_TWS_CHAIN_CONFIG_H_
#define AUDIO_INPUT_TWS_CHAIN_CONFIG_H_

#include "audio_input_tws_chain_config_common.h"
#include <chain.h>

/****************************************************************************
DESCRIPTION
    Create the chain with the appropriate configuration filters.
*/
kymera_chain_handle_t audioInputTWSCreateChain(audio_input_tws_decoder_t decoder);

#endif /* AUDIO_INPUT_TWS_CHAIN_CONFIG_H_ */
