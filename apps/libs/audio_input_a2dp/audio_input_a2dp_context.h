/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_a2dp_context.h

DESCRIPTION
    Manages the context information of the audio plugin
*/

#ifndef AUDIO_INPUT_A2DP_CONTEXT_H_
#define AUDIO_INPUT_A2DP_CONTEXT_H_

#include <audio_input_common.h>


void audioInputA2dpCreateContextData(void);
void audioInputA2dpDeleteContextData(void);
audio_input_context_t * audioInputA2dpGetContextData(void);
kymera_chain_handle_t audioInputA2dpGetChainHandle(void);

#endif /* AUDIO_INPUT_A2DP_CONTEXT_H_ */
