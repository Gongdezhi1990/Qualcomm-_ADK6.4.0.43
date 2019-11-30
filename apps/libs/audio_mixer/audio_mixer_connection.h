/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_mixer_connection.h

DESCRIPTION
    Helper functions relating to supported connection types
*/

#ifndef _AUDIO_MIXER_CONNECTION_H_
#define _AUDIO_MIXER_CONNECTION_H_

#include "audio_mixer.h"

/*******************************************************************************
DESCRIPTION
    Can mixer support concurrent voice and music inputs
*/
bool audioMixerConnectionOfConcurrentVoiceAndMusicEnabled(void);

/*******************************************************************************
DESCRIPTION
    Can mixer support concurrent low power music connections (note this is 
    mutually exclusive with conncurrent voice and music)
*/
bool audioMixerConnectionOfLowPowerMusicEnabled(void);

/*******************************************************************************
DESCRIPTION
    Is connection_type a voice connection
*/
bool audioMixerConnectionIsVoice(const connection_type_t connection_type);

/*******************************************************************************
DESCRIPTION
    Is connection_type a voice connection with no requirement to support mixing
    with music.
*/
bool audioMixerConnectionIsVoiceOnly(const connection_type_t connection_type);

/*******************************************************************************
DESCRIPTION
    Is connection_type a music connection
*/
bool audioMixerConnectionIsMusic(const connection_type_t connection_type);

/*******************************************************************************
DESCRIPTION
    Is connection_type a voice connection with no requirement to support mixing
    with music and with low power music enabled
*/
bool audioMixerConnectionIsLowPowerMusic(const connection_type_t connection_type);

/*******************************************************************************
DESCRIPTION
    Is connection_type a prompt connection
*/
bool audioMixerConnectionIsPrompt(const connection_type_t connection_type);

#endif /* _AUDIO_MIXER_CONNECTION_H_ */
