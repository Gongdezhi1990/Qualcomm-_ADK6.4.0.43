/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_mixer_core.h

DESCRIPTION
       Declarations for core mixer components chain management
*/

#ifndef _AUDIO_MIXER_CORE_H_
#define _AUDIO_MIXER_CORE_H_

#include <gain_utils.h>
#include <audio_plugin_if.h>
#include <chain.h>

#include "audio_mixer.h"

#define FADE_OUT_DELAY_MS 50

/****************************************************************************
DESCRIPTION
    Creates the core mixer capability
*/
void mixerCoreCreate(connection_type_t connection_type, AUDIO_MUSIC_CHANNEL_MODE_T channel_mode, uint32 output_sample_rate);

/****************************************************************************
DESCRIPTION
    Destroy the core mixer capability
*/
void mixerCoreDestroy(void);

/****************************************************************************
DESCRIPTION
    Get the chain for the core mixer capability
*/
kymera_chain_handle_t mixerCoreGetChain(void);

/****************************************************************************
DESCRIPTION
    Start processing within the core mixer capability
*/
void mixerCoreStart(void);

/****************************************************************************
DESCRIPTION
    Stop processing within the core mixer capability
*/
void mixerCoreStop(void);

/****************************************************************************
DESCRIPTION
    Process any changes to the gain settings on the mixer inputs
*/
void mixerCoreSetInputGains(void);

/****************************************************************************
DESCRIPTION
    Update the music channel mode.
*/
void mixerCoreConfigureChannelMode(AUDIO_MUSIC_CHANNEL_MODE_T channel_mode, uint32 sample_rate);

/****************************************************************************
DESCRIPTION
    Get the number of downmix music inputs required by the mixer capability
RETURNS
    Number of downmix inputs
*/
unsigned mixerCoreGetNumberOfDownmixChannels(connection_type_t connection_type, bool mono_path_enabled);

#endif /* _AUDIO_MIXER_CORE_H_ */
