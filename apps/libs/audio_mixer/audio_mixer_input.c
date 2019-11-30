/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_mixer_input.c

DESCRIPTION
    Represents a logical input to the audio mixer lib.
*/

#include <string.h>
#include <panic.h>

#include "audio_mixer.h"
#include "audio_mixer_input.h"
#include "audio_mixer_resampler.h"
#include <audio_processor.h>

#define forEachInput(i) for(i=audio_mixer_input_first; i<=audio_mixer_input_last; i++)

/* Defines a logical input */
typedef struct mixer_input_instance
{
    mixer_resampler_context_t   resampler;  /* Resampler for the input */
    mixer_processing_context_t  processing; /* Processing for the input */
    bool                        muted;      /* Whether the input is muted */
    bool                        connected;  /* Whether the input is connected */
    bool                        hd_audio;   /* Whether the input is HD audio or not */
} mixer_input_instance_t;

/* Mixer input local data */
typedef struct mixer_input_data
{
    mixer_input_instance_t      instance[audio_mixer_input_max];    /* The inputs */
} mixer_input_data_t;

static mixer_input_data_t input_data;

/******************************************************************************/
void mixerInputInit(void)
{
    memset(&input_data, 0, sizeof(mixer_input_data_t));

    /* Start everything muted */
    mixerInputMuteAll(TRUE);
}

/******************************************************************************/
bool mixerInputIsMuted(audio_mixer_input_t input)
{
    return input_data.instance[input].muted;
}

/******************************************************************************/
bool mixerInputIsConnected(audio_mixer_input_t input)
{
    return input_data.instance[input].connected;
}

/******************************************************************************/
bool mixerInputIsAvailable(audio_mixer_input_t input)
{
    return !mixerInputIsConnected(input);
}

/******************************************************************************/
bool mixerInputCanDisconnect(audio_mixer_input_t input)
{
    if(input > audio_mixer_input_last)
        return FALSE;

    if(mixerInputIsAvailable(input))
        return FALSE;

    return TRUE;
}

/******************************************************************************/
bool mixerInputAnyConnected(void)
{
    audio_mixer_input_t input;

    forEachInput(input)
    {
        if(mixerInputIsConnected(input))
        {
            return TRUE;
        }
    }
    return FALSE;
}

/******************************************************************************/
bool mixerInputIsLastConnected(audio_mixer_input_t last_input)
{
    audio_mixer_input_t input;

    forEachInput(input)
    {
        if(mixerInputIsConnected(input) && (input != last_input))
        {
            return FALSE;
        }
    }
    return TRUE;
}

/******************************************************************************/
mixer_resampler_context_t mixerInputGetResampler(audio_mixer_input_t input)
{
   return input_data.instance[input].resampler;
}

/******************************************************************************/
void mixerInputSetResampler(audio_mixer_input_t input, mixer_resampler_context_t context)
{
    input_data.instance[input].resampler = context;
}

/******************************************************************************/
mixer_processing_context_t mixerInputGetProcessing(audio_mixer_input_t input)
{
   return input_data.instance[input].processing;
}

/******************************************************************************/
void mixerInputSetProcessing(audio_mixer_input_t input, mixer_processing_context_t context)
{
    input_data.instance[input].processing = context;
}

/******************************************************************************/
void mixerInputMute(audio_mixer_input_t input, bool set_mute)
{
    input_data.instance[input].muted = set_mute;
}

/******************************************************************************/
void mixerInputMuteAll(bool set_mute)
{
    audio_mixer_input_t input;

    forEachInput(input)
    {
        mixerInputMute(input, set_mute);
    }
}

/******************************************************************************/
void mixerInputChangeSampleRate(audio_mixer_input_t input, unsigned in_rate, unsigned out_rate)
{
    if (input >= audio_mixer_input_max || input_data.instance[input].resampler == NULL)
    {
        Panic();
    }

    mixerResamplerChangeSampleRate(input_data.instance[input].resampler, in_rate, out_rate);
    mixerProcessingChangeSampleRate(input_data.instance[input].processing, out_rate);
}

/******************************************************************************/
void mixerInputSetConnected(audio_mixer_input_t input, bool connected)
{
    input_data.instance[input].connected = connected;
    if (connected)
    {
        input_data.instance[input].hd_audio = FALSE;
    }
}

/******************************************************************************/
void mixerInputSetIsHd(audio_mixer_input_t input, bool is_input_hd)
{
    int index, count = 0;
    /*
     * Determine how many elements had hd_audio set before modification.
     */
    forEachInput(index)
    {
        if (input_data.instance[index].hd_audio)
        {
            count++;
        }
    }

    /*
     * Modify the required element of the input_data for hd_audio.
     */
    input_data.instance[input].hd_audio = is_input_hd;

    if (is_input_hd && (count == 0))
    {
        /*
         * There were no elements with hd_audio set, and now there is, so add
         * the audio_ucid_hd_audio use-case.
         */
        AudioProcessorAddUseCase(audio_ucid_hd_audio);
    }
    else if (!is_input_hd && (count == 1))
    {
        /*
         * There was one element with hd_audio set and one has been cleared...
         */
        count = 0;
        forEachInput(index)
        {
            if (input_data.instance[index].hd_audio)
            {
                count++;
            }
        }

        if (count == 0)
        {
            /*
             * ...and now there isn't an element with hd_audio set, so remove
             * the audio_ucid_hd_audio use-case.
             */
            AudioProcessorRemoveUseCase(audio_ucid_hd_audio);
        }
    }
}

