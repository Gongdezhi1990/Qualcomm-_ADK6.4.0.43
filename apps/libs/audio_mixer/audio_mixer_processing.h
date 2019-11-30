/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_mixer_processing.h

DESCRIPTION
    Implements functionality to create the processing chain
*/

#ifndef _AUDIO_MIXER_PROCESSING_H_
#define _AUDIO_MIXER_PROCESSING_H_

#include "chain.h"
#include "audio_mixer.h"

typedef struct __mixer_processing_data * mixer_processing_context_t;

/****************************************************************************
DESCRIPTION
    Create a simple processing chain
RETURNS
    A context that can be used by the caller to refer to a specific instance of 
    processing in subsequent calls
*/
mixer_processing_context_t mixerProcessingCreate(connection_type_t connection_type,
                                                 uint32 output_sample_rate, 
                                                 bool is_mono,
                                                 bool attenuate_volume_pre_processing);

/****************************************************************************
DESCRIPTION
    Stop the processing referred to by the context parameter
*/
void mixerProcessingStop(mixer_processing_context_t context);

/****************************************************************************
DESCRIPTION
    Disconnect and destroy the processing referred to by the context parameter
*/
void mixerProcessingDestroy(mixer_processing_context_t context);

/****************************************************************************
DESCRIPTION
    Start the processing referred to by the context parameter
*/
void mixerProcessingStart(mixer_processing_context_t context);

/****************************************************************************
DESCRIPTION
    Get the chain for the specified processing
*/
kymera_chain_handle_t mixerProcessingGetChain(mixer_processing_context_t context);

/****************************************************************************
DESCRIPTION
    Changes sample rate for processing.
*/
void mixerProcessingChangeSampleRate(mixer_processing_context_t context,
                                     uint32 output_sample_rate);

#endif /* _AUDIO_MIXER_PROCESSING_H_ */
