/****************************************************************************
Copyright (c) 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_mixer_processing_multiband_compander.h

DESCRIPTION
    Implementation of multi-band compander processing chain construction functions
*/

#ifndef _AUDIO_MIXER_PROCESSING_MULTIBAND_COMPANDER_H_
#define _AUDIO_MIXER_PROCESSING_MULTIBAND_COMPANDER_H_

#include <chain.h>

#include "audio_mixer.h"
#include "audio_mixer_processing_common.h"

/****************************************************************************
DESCRIPTION
    Function to create processing chains of 2band/3band compander.
*/
kymera_chain_handle_t mixerProcessingCreateMultiBandCompanderChain(processing_chain_t type,
                                                        bool attenuate_volume_pre_processing);

/****************************************************************************
DESCRIPTION
    Function to configure mixer operator streams in multiband compander processing chain
*/
void mixerProcessingSetMultiBandCompanderMixerStreams(kymera_chain_handle_t chain, processing_chain_t chain_type);

/****************************************************************************
DESCRIPTION
    Function to set the processing mode of multiband compander chain.
*/
void mixerProcessingSetMultiBandCompanderMode(kymera_chain_handle_t chain, music_processing_mode_t mode, processing_chain_t chain_type);

/****************************************************************************
DESCRIPTION
    Function to set the UCID of multiband compander processing chain operators
*/
void mixerProcessingSetMultiBandCompanderUcid(kymera_chain_handle_t chain, processing_chain_t chain_type);

/****************************************************************************
DESCRIPTION
    Function to register multiband compander processing operators with music processing lib
*/
void mixerProcessingRegisterMultiBandCompanderOperators(kymera_chain_handle_t chain, processing_chain_t chain_type);

/****************************************************************************
DESCRIPTION
    Function to unregister multiband compander processing operators with music processing lib
*/
void mixerProcessingUnregisterMultiBandCompanderOperators(processing_chain_t chain_type);

#endif /* _AUDIO_MIXER_PROCESSING_MULTIBAND_COMPANDER_H_ */

