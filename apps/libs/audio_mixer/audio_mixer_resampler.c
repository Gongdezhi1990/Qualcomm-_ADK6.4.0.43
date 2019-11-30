/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_mixer_resampler.c
DESCRIPTION
    Implements functionality to create and connect a resampler operator for the audio mixer lib
NOTES
*/

#include <stream.h>
#include <stdlib.h>

#include "audio_mixer_resampler.h"
#include "audio_mixer_resampler_chain_config.h"
#include "audio_mixer_channel_roles.h"
#include "audio_mixer.h"
#include "chain.h"
#include "operators.h"
#include "panic.h"

typedef struct __mixer_resampler_data
{
    kymera_chain_handle_t  chain;
    resampler_chain_t chain_type;
} mixer_resampler_data_t;


/****************************************************************************
DESCRIPTION
    Helper function to configure operators in the resampler chain 
*/
static void mixerResamplerChainConfigure(kymera_chain_handle_t chain, uint32 input_sample_rate, uint32 output_sample_rate)
{
    Operator op = ChainGetOperatorByRole(chain, resampler_role);
    OperatorsResamplerSetConversionRate(op, input_sample_rate, output_sample_rate);
}


/****************************************************************************
DESCRIPTION
    Instantiate and configure the required operators for the resampler chain.
    A resampler chain is constructed for each input to be connected to an input
    of the mixer.
RETURNS
    a pointer to the resampler data
*/
static mixer_resampler_data_t* mixerResamplerCreateChain(connection_type_t connection_type,
                                                         unsigned inputSampleRate,
                                                         unsigned outputSampleRate)
{
    mixer_resampler_data_t* mixer_resampler_data = PanicUnlessNew(mixer_resampler_data_t);
    mixer_resampler_data->chain_type = mixerResamplerGetChainType(connection_type);
    const chain_config_t* config = mixerResamplerGetChainConfig(mixer_resampler_data->chain_type);

    mixer_resampler_data->chain = PanicNull(ChainCreate(config));
    mixerResamplerChainConfigure(mixer_resampler_data->chain, inputSampleRate, outputSampleRate);

    ChainConnect(mixer_resampler_data->chain);

    return mixer_resampler_data;
}

/******************************************************************************/
static bool isResamplerRequired(const audio_mixer_connect_t* connect_data, uint32 output_sample_rate)
{
    if(connect_data->sample_rate != output_sample_rate)
        return TRUE;
    
    if(connect_data->variable_rate)
        return TRUE;
    
    return FALSE;
}

/******************************************************************************/
mixer_resampler_context_t mixerResamplerCreate(const audio_mixer_connect_t* connect_data, 
                                               uint32 output_sample_rate)
{
    mixer_resampler_context_t mixer_resampler_data = NULL;

    if(isResamplerRequired(connect_data, output_sample_rate))
    {
        switch (connect_data->connection_type)
        {
            case CONNECTION_TYPE_MUSIC:
            case CONNECTION_TYPE_MUSIC_A2DP:
            case CONNECTION_TYPE_MUSIC_BA_TX:
            case CONNECTION_TYPE_MUSIC_BA_RX:
            case CONNECTION_TYPE_VOICE:
            case CONNECTION_TYPE_TONES:
            {
                mixer_resampler_data = mixerResamplerCreateChain(connect_data->connection_type,
                                                                 connect_data->sample_rate,
                                                                 output_sample_rate);
                break;
            }

            default:
                Panic();
                break;
        }
    }

    return mixer_resampler_data;
}

/******************************************************************************/
kymera_chain_handle_t mixerResamplerGetChain(mixer_resampler_context_t context)
{
    if(context)
        return context->chain;
    return NULL;
}

/******************************************************************************/
void mixerResamplerStop(mixer_resampler_context_t resampler_context)
{
    if(resampler_context)
        ChainStop(resampler_context->chain);
}

/******************************************************************************/
void mixerResamplerDestroy(mixer_resampler_context_t resampler_context)
{
    if(resampler_context)
    {
        ChainDestroy(resampler_context->chain);
        free(resampler_context);
    }
}

/******************************************************************************/
void mixerResamplerStart(mixer_resampler_context_t resampler_context)
{
    if(resampler_context)
        ChainStart(resampler_context->chain);
}

/******************************************************************************/
void mixerResamplerChangeSampleRate(mixer_resampler_context_t resampler_context,
                                    uint32 input_sample_rate, 
                                    uint32 output_sample_rate)
{
    if(resampler_context)
        mixerResamplerChainConfigure(resampler_context->chain, input_sample_rate, output_sample_rate);
}
