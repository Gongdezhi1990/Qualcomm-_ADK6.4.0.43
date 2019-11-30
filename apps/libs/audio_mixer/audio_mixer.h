/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_mixer.h

DESCRIPTION
    This library instantiates the common output stage for the crescendo platform
*/

#ifndef _AUDIO_MIXER_H_
#define _AUDIO_MIXER_H_

#include <library.h>
#include <source.h>
#include <message.h>
#include <audio_plugin_if.h>

typedef enum {
    audio_mixer_input_music,
    audio_mixer_input_voice,
    audio_mixer_input_tones,
    audio_mixer_input_error_none,
    audio_mixer_input_first = audio_mixer_input_music,
    audio_mixer_input_last = audio_mixer_input_tones,
    audio_mixer_input_max = audio_mixer_input_last + 1
} audio_mixer_input_t;

typedef enum connection_type
{
    CONNECTION_TYPE_UNUSED = 0x00,
    CONNECTION_TYPE_MUSIC,
    CONNECTION_TYPE_MUSIC_A2DP,
    CONNECTION_TYPE_MUSIC_BA_TX,
    CONNECTION_TYPE_MUSIC_BA_RX,
    CONNECTION_TYPE_VOICE,
    CONNECTION_TYPE_TONES
} connection_type_t;

typedef enum
{
    AUDIO_MIXER_FADE_OUT_CFM = AUDIO_MIXER_MESSAGE_BASE,
    AUDIO_MIXER_FADE_IN_CFM,
    AUDIO_MIXER_MESSAGE_TOP
} audio_mixer_message_t;

typedef enum
{
    AUDIO_MIXER_DISCONNECT_OK,
    AUDIO_MIXER_DISCONNECT_ERROR = 0xff
} audio_mixer_disconnect_response_t;

typedef struct
{
    audio_mixer_disconnect_response_t mixerDisconnectResponse;
}AUDIO_MIXER_DISCONNECT_RESPONSE_MSG_T;

typedef struct
{
    Source                     left_src;
    Source                     right_src;
    connection_type_t          connection_type;
    uint32                     sample_rate;
    AUDIO_MUSIC_CHANNEL_MODE_T channel_mode;
    bool                       variable_rate;
} audio_mixer_connect_t;

/****************************************************************************
NAME
    AudioMixerConnect
DESCRIPTION
    Connects the supplied sources to the next available mixer input. If this is
    the first connection then all required operators will be created and started
RETURNS
    A valid mixer input, or an error code if all inputs are already in use.
*/
audio_mixer_input_t AudioMixerConnect(const audio_mixer_connect_t* audio_mixer_connect);

/****************************************************************************
NAME
    AudioMixerDisconnect
DESCRIPTION
    Disconnected the supplied mixer input
RETURNS
    A code representing success or failure
*/
audio_mixer_disconnect_response_t AudioMixerDisconnect(audio_mixer_input_t disconnectInput);

/****************************************************************************
NAME
    AudioMixerFadeIn
DESCRIPTION
    Fade In the supplied mixer input
*/
void AudioMixerFadeIn(audio_mixer_input_t fade_input, Task calling_task);

/****************************************************************************
NAME
    AudioMixerFadeOut
DESCRIPTION
    Fade Out the supplied mixer input
*/
void AudioMixerFadeOut(audio_mixer_input_t fade_input, Task calling_task);

/****************************************************************************
NAME
    AudioMixerMuteOutput
DESCRIPTION
    Mute the mixer output
*/
void AudioMixerMuteOutput(bool enable);

/****************************************************************************
NAME
    AudioMixerSetVolume
DESCRIPTION
    Set the mixer volume to the master volume from audio_config
*/
void AudioMixerSetVolume(void);

/****************************************************************************
NAME
    AudioMixerSetInputSampleRate
DESCRIPTION
    Set the input frequency for selected input.
    This can only be called for active connections.
*/
void AudioMixerSetInputSampleRate(audio_mixer_input_t input, unsigned sample_rate);

/****************************************************************************
NAME
    AudioMixerChangeMusicChannelMode
DESCRIPTION
    Change the music channel mode
*/
void AudioMixerUpdateChannelMode(AUDIO_MUSIC_CHANNEL_MODE_T channel_mode);

/****************************************************************************
NAME
    AudioMixerGetOutputSampleRate
DESCRIPTION
    Get the Audio Mixer Output Sample Rate
*/
uint32 AudioMixerGetOutputSampleRate(connection_type_t connection_type, uint32 input_sample_rate);

#ifdef HOSTED_TEST_ENVIRONMENT
/****************************************************************************
NAME
    AudioMixerTestReset
DESCRIPTION
    Reset any static variables
    This is only intended for unit test and will panic if called in a release
    build.
*/
void AudioMixerTestReset(void);
#endif

#endif /* _AUDIO_MIXER_H_ */
