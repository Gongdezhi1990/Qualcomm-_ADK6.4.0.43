/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_hardware.h

DESCRIPTION
    Write a short description about what the module does and how it should be
    used.
*/


/*!
@file    audio_hardware.h
@brief   Write a short description about what the module does and how it
         should be used.

         This file provides documentation for the audio_hardware API.
*/


#ifndef AUDIO_HARDWARE_
#define AUDIO_HARDWARE_

#include "audio_mixer.h"

#define DEFAULT_MIC_SAMPLE_RATE     ((uint32) 8000)

typedef enum
{
    left_channel = 0,
    right_channel,
    secondary_left_channel,
    secondary_right_channel,
    wired_sub_channel,
    mic_ref_channel,
    mic_a_channel,
    mic_b_channel,
    mic_c_channel,
    max_channel
} audio_hw_channel_role_t;

typedef enum
{
    speaker_mono,
    speaker_stereo,
    speaker_stereo_bass,
    speaker_stereo_xover
} audio_hardware_speaker_config_t;

typedef struct
{
    Source primary_left;
    Source primary_right;
    Source secondary_left;
    Source secondary_right;
    Source wired_sub;
    Source aux_left;
    Source aux_right;
    connection_type_t connection_type;
    uint32 output_sample_rate;
} audio_hardware_connect_t;

typedef enum
{
    audio_hw_voice_call,
    audio_hw_voice_assistant
} audio_hw_mic_use_t ;


/****************************************************************************
DESCRIPTION
    Connect the audio_mixer_speaker to the audio hardware, Include AEC chain if applicable
*/
bool AudioHardwareConnectOutput(const audio_hardware_connect_t* hw_connect_data);


/****************************************************************************
DESCRIPTION
    Connect the microphone path to the audio hardware
*/
bool AudioHardwareConnectInput(const uint32 mic_sample_rate);


/****************************************************************************
DESCRIPTION
    Disconnect the AEC chain input path - Stop the chain and destroy if req
*/
bool AudioHardwareDisconnectInput(void);

/****************************************************************************
DESCRIPTION
    Disconnect the AEC chain output path - Stop the chain and destroy if req
*/
bool AudioHardwareDisconnectOutput(void);


/****************************************************************************
DESCRIPTION
    Return the speaker configuration
*/
audio_hardware_speaker_config_t AudioHardwareGetSpeakerConfig(void);

/****************************************************************************
DESCRIPTION
    Configure AEC Mic sample rates and update UCID to match
*/
void AudioHardwareConfigureMicSampleRate(uint32 mic_sample_rate);

/****************************************************************************
DESCRIPTION
    Mutes AEC Mic output
*/
void AudioHardwareMuteMicOutput(bool enable);

/****************************************************************************
DESCRIPTION
    Gets the AEC operator
*/
Operator AudioHardwareGetOperator(void);

/****************************************************************************
DESCRIPTION
    Sets the user of Microphone
*/
void AudioHardwareSetMicUse(audio_hw_mic_use_t mic_use);

#endif /* AUDIO_HARDWARE_ */
