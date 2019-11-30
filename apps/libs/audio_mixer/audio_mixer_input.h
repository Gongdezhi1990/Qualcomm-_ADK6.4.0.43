/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_mixer_input.h

DESCRIPTION
    Represents a logical input to the audio mixer lib.
*/

#ifndef _AUDIO_MIXER_INPUT_H_
#define _AUDIO_MIXER_INPUT_H_

#include "audio_mixer.h"
#include "audio_mixer_resampler.h"
#include "audio_mixer_processing.h"

/*******************************************************************************
DESCRIPTION
    Initialise mixer inputs.
*/
void mixerInputInit(void);

/*******************************************************************************
RETURNS
    Is the mixer input muted?
*/
bool mixerInputIsMuted(audio_mixer_input_t input);

/*******************************************************************************
RETURNS
    Is the mixer input requested available to connect to?
*/
bool mixerInputIsAvailable(audio_mixer_input_t input);

/*******************************************************************************
RETURNS
    Is the mixer input requested already connected?
*/
bool mixerInputIsConnected(audio_mixer_input_t input);

/*******************************************************************************
RETURNS
    Is the mixer input valid and currently in use?
*/
bool mixerInputCanDisconnect(audio_mixer_input_t input);

/****************************************************************************
RETURNS
    Is there any active input currently connected to the mixer?
*/
bool mixerInputAnyConnected(void);

/****************************************************************************
RETURNS
    Is the input the last one connected to the mixer?
*/
bool mixerInputIsLastConnected(audio_mixer_input_t last_input);

/*******************************************************************************
RETURNS
    The resampler context associated with a mixer input.
*/
mixer_resampler_context_t mixerInputGetResampler(audio_mixer_input_t input);

/*******************************************************************************
DESCRIPTION
    Set the resampler context associated with a mixer input.
*/
void mixerInputSetResampler(audio_mixer_input_t input, mixer_resampler_context_t context);

/*******************************************************************************
RETURNS
    The processing context associated with a mixer input.
*/
mixer_processing_context_t mixerInputGetProcessing(audio_mixer_input_t input);

/*******************************************************************************
DESCRIPTION
    Set the processing context associated with a mixer input.
*/
void mixerInputSetProcessing(audio_mixer_input_t input, mixer_processing_context_t context);

/*******************************************************************************
DESCRIPTION
    Set the mute state of a mixer input.
*/
void mixerInputMute(audio_mixer_input_t input, bool set_mute);

/*******************************************************************************
DESCRIPTION
    Set the mute state of all the mixer inputs at once.
*/
void mixerInputMuteAll(bool set_mute);

/****************************************************************************
DESCRIPTION
    Change input frequency of the resampler.
*/
void mixerInputChangeSampleRate(audio_mixer_input_t input, unsigned in_rate, unsigned out_rate);

/****************************************************************************
DESCRIPTION
    Set connected state of the input
*/
void mixerInputSetConnected(audio_mixer_input_t input, bool connected);

/****************************************************************************
DESCRIPTION
    Set hd_audio state of the input
*/
void mixerInputSetIsHd(audio_mixer_input_t input, bool is_input_hd);

#endif /* _AUDIO_MIXER_INPUT_H_ */
