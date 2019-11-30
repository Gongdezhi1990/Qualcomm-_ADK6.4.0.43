/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_mixer_resampler.c

DESCRIPTION
    Implements functionality to create and connect a resampler operator for the 
    audio mixer lib.
*/

#ifndef _AUDIO_MIXER_RESAMPLER_H_
#define _AUDIO_MIXER_RESAMPLER_H_

#include "chain.h"
#include "audio_mixer.h"

typedef struct __mixer_resampler_data * mixer_resampler_context_t;

/****************************************************************************
DESCRIPTION
    Create a simple resampler chain
RETURNS
    A context that can be used by the caller to refer to a specific instance of 
    resampler in subsequent calls
*/
mixer_resampler_context_t mixerResamplerCreate(const audio_mixer_connect_t* connect_data, 
                                               uint32 output_sample_rate);

/****************************************************************************
DESCRIPTION
    Stop the resampler referred to by the context parameter
*/
void mixerResamplerStop(mixer_resampler_context_t resampler_context);

/****************************************************************************
DESCRIPTION
    Disconnect and destroy the resampler referred to by the context parameter
*/
void mixerResamplerDestroy(mixer_resampler_context_t resampler_context);

/****************************************************************************
DESCRIPTION
    Start the resampler referred to by the context parameter
*/
void mixerResamplerStart(mixer_resampler_context_t resampler_context);

/****************************************************************************
DESCRIPTION
    Get the chain for the specified resampler
*/
kymera_chain_handle_t mixerResamplerGetChain(mixer_resampler_context_t context);

/****************************************************************************
DESCRIPTION
    Changes input and output frequencies for selected resampler.
*/
void mixerResamplerChangeSampleRate(mixer_resampler_context_t resampler_context,
                                    uint32 input_sample_rate, 
                                    uint32 output_sample_rate);

#endif /* _AUDIO_MIXER_RESAMPLER_H_ */
