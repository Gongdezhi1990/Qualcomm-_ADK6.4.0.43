/*******************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_mixer_chain_interconnections.h
DESCRIPTION
    Defines the interconnections between the various audio mixer chains.
NOTES
*/

#ifndef _AUDIO_MIXER_CHAIN_INTERCONNECTIONS_H_
#define _AUDIO_MIXER_CHAIN_INTERCONNECTIONS_H_

#include <chain.h>

/*******************************************************************************
DESCRIPTION
    Connect the resampler chain to the specified mixer input
*/
bool mixerChainConnectInput(audio_mixer_input_t input, Source left, Source right);

#endif /* _AUDIO_MIXER_CHAIN_INTERCONNECTIONS_H_ */
