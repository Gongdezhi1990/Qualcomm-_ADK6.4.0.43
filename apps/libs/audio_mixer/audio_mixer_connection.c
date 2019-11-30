/****************************************************************************
Copyright (c) 2017 - 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_mixer_connection.c

DESCRIPTION
    Helper functions relating to supported connection types
*/

#include <audio_config.h>

#include "audio_mixer_connection.h"

/******************************************************************************/
bool audioMixerConnectionOfConcurrentVoiceAndMusicEnabled(void)
{
    if(AudioConfigGetMaximumConcurrentInputs() > 1)
    {
        return TRUE;
    }
    return FALSE;
}

/******************************************************************************/
bool audioMixerConnectionOfLowPowerMusicEnabled(void)
{
    if(!audioMixerConnectionOfConcurrentVoiceAndMusicEnabled())
    {
        if(AudioConfigGetQuality(audio_stream_music) == audio_quality_low_power)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/******************************************************************************/
bool audioMixerConnectionIsVoice(const connection_type_t connection_type)
{
    return (connection_type == CONNECTION_TYPE_VOICE);
}

/******************************************************************************/
bool audioMixerConnectionIsVoiceOnly(const connection_type_t connection_type)
{
    if(audioMixerConnectionIsVoice(connection_type))
    {
        if(!audioMixerConnectionOfConcurrentVoiceAndMusicEnabled())
            return TRUE;
    }
    return FALSE;
}

/******************************************************************************/
bool audioMixerConnectionIsMusic(const connection_type_t connection_type)
{
    switch(connection_type)
    {
        case CONNECTION_TYPE_MUSIC:
        case CONNECTION_TYPE_MUSIC_A2DP:
        case CONNECTION_TYPE_MUSIC_BA_TX:
        case CONNECTION_TYPE_MUSIC_BA_RX:
            return TRUE;
        
        default:
            return FALSE;
    }
}

/******************************************************************************/
bool audioMixerConnectionIsLowPowerMusic(const connection_type_t connection_type)
{
    if(audioMixerConnectionIsMusic(connection_type))
    {
        if(audioMixerConnectionOfLowPowerMusicEnabled())
            return TRUE;
    }
    return FALSE;
}

/******************************************************************************/
bool audioMixerConnectionIsPrompt(const connection_type_t connection_type)
{
    return (connection_type == CONNECTION_TYPE_TONES);
}
