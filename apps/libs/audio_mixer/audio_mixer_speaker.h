/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_mixer_speaker.h

DESCRIPTION
    Declarations for Mixer Speaker chain management
*/

#ifndef _AUDIO_MIXER_SPEAKER_H_
#define _AUDIO_MIXER_SPEAKER_H_

#include "audio_output.h"
#include "chain.h"
#include <operators.h>

typedef enum
{
    audio_mixer_speaker_mono,
    audio_mixer_speaker_stereo,
    audio_mixer_speaker_stereo_bass,
    audio_mixer_speaker_stereo_xover,
    audio_mixer_speaker_voice_only,
    audio_mixer_speaker_invalid
} audio_mixer_speaker_configuration_t;

/****************************************************************************
DESCRIPTION
    Create the mixer speaker object
*/
void mixerSpeakerCreate(connection_type_t connection_type, unsigned output_sample_rate);

/****************************************************************************
DESCRIPTION
    Get the handle for the internal chain
*/
kymera_chain_handle_t mixerSpeakerGetChain(void);

/****************************************************************************
DESCRIPTION
    Start processing within the speaker chain
*/
void mixerSpeakerStart(void);

/****************************************************************************
DESCRIPTION
    Stop processing within the speaker chain
*/
void mixerSpeakerStop(void);

/****************************************************************************
DESCRIPTION
    Destroy the speaker chain
*/
void mixerSpeakerDestroy(void);

/****************************************************************************
DESCRIPTION
    Set the volume
*/
void mixerSpeakerSetVolume(void);

/****************************************************************************
DESCRIPTION
    Mute the speaker output
*/
void mixerSpeakerMuteOutput(bool enable);

/****************************************************************************
DESCRIPTION
    Does the speaker chain have post processing?
*/
bool mixerSpeakerIncreasesVolumePostProcessing(void);

#endif /* _AUDIO_MIXER_SPEAKER_H_ */
