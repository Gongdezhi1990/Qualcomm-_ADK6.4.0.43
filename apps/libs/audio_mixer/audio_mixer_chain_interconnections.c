/*******************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_mixer_chain_interconnections.c
DESCRIPTION
    Defines the interconnections between the various audio mixer chains.
NOTES
*/

#include <audio_output.h>
#include <chain.h>
#include <panic.h>
#include <stream.h>
#include <transform.h>

#include "audio_mixer.h"
#include "audio_mixer_core.h"
#include "audio_mixer_input.h"
#include "audio_mixer_resampler.h"
#include "audio_mixer_processing.h"
#include "audio_mixer_speaker.h"
#include "audio_mixer_chain_interconnections.h"
#include "audio_mixer_channel_roles.h"

/******************************************************************************/
static void getInputChains(audio_mixer_input_t input, kymera_chain_handle_t* resampler_chain, kymera_chain_handle_t* processing_chain)
{
    mixer_resampler_context_t resampler_context   = mixerInputGetResampler(input);
    mixer_processing_context_t processing_context = mixerInputGetProcessing(input);

    *resampler_chain  = mixerResamplerGetChain(resampler_context);
    *processing_chain = mixerProcessingGetChain(processing_context);
}

/******************************************************************************/
static kymera_chain_handle_t getInputChainForInput(audio_mixer_input_t input)
{
    kymera_chain_handle_t resampler_chain;
    kymera_chain_handle_t processing_chain;
    getInputChains(input, &resampler_chain, &processing_chain);

    if(resampler_chain)
        return resampler_chain;

    return processing_chain;
}

/******************************************************************************/
static kymera_chain_handle_t getMidChainForInput(audio_mixer_input_t input)
{
    kymera_chain_handle_t resampler_chain;
    kymera_chain_handle_t processing_chain;
    getInputChains(input, &resampler_chain, &processing_chain);

    if(resampler_chain)
        return processing_chain;

    return NULL;
}

/******************************************************************************/
static kymera_chain_handle_t getOutputChainForInput(audio_mixer_input_t input)
{
    kymera_chain_handle_t output_chain = mixerCoreGetChain();

    if(input == audio_mixer_input_tones)
        output_chain = mixerSpeakerGetChain();

    return output_chain;
}

/******************************************************************************/
static audio_mixer_channel_role_t getRoleForInput(audio_mixer_input_t input, audio_mixer_channel_role_t channel)
{
    switch(input)
    {
        case audio_mixer_input_music:
        {
            if(channel == mixer_left_channel)
                return mixer_music_left_channel;
            return mixer_music_right_channel;
        }

        case audio_mixer_input_voice:
        {
            return mixer_voice_channel;
        }

        case audio_mixer_input_tones:
        {
            if(channel == mixer_left_channel)
                return mixer_prompt_left_channel;
            return mixer_prompt_right_channel;
        }

        default:
        {
            Panic();
            return channel;
        }
    }
}

/******************************************************************************/
static void mixerChainConnectSource(kymera_chain_handle_t input_chain, audio_mixer_input_t input, audio_mixer_channel_role_t channel, Source source)
{
    PanicNull(StreamConnect(source, ChainGetInput(input_chain, getRoleForInput(input, channel))));
}

/******************************************************************************/
static unsigned getPathCount(Source left, Source right)
{
    unsigned path_count = 0;

     if (left != NULL)
        path_count++;

     if (right != NULL)
        path_count++;

     return path_count;
}

/******************************************************************************/
bool mixerChainConnectInput(audio_mixer_input_t input, Source left, Source right)
{
    kymera_chain_handle_t input_chain  = getInputChainForInput(input);
    kymera_chain_handle_t mid_chain    = getMidChainForInput(input);
    kymera_chain_handle_t output_chain = getOutputChainForInput(input);
    unsigned path_count = getPathCount(left, right);
    /* For tones/ prompts the speaker chain input must be connected with 1 path regardless of left and right sources */
    unsigned output_chain_path_count = (input == audio_mixer_input_tones)? 1 : path_count;

    if(mid_chain)
    {
        /* Two input chains (resampler and processing) */
        if(ChainJoinMatchingRoles(input_chain, mid_chain, path_count) != path_count)
            return FALSE;

        if(ChainJoinMatchingRoles(mid_chain, output_chain, output_chain_path_count) != output_chain_path_count)
            return FALSE;
    }
    else if(input_chain)
    {
        /* Only one input chain (resampler or processing) */
        if(ChainJoinMatchingRoles(input_chain, output_chain, output_chain_path_count) != output_chain_path_count)
            return FALSE;
    }
    else
    {
        /* No input chains (no resampler or processing) */
        input_chain = output_chain;
    }

    mixerChainConnectSource(input_chain, input, mixer_left_channel, left);

    if(right)
        mixerChainConnectSource(input_chain, input, mixer_right_channel, right);

    return TRUE;
}
