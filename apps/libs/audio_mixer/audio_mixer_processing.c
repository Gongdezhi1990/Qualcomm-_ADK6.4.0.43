/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_mixer_processing.c
DESCRIPTION
    Implements functionality to create the processing chain
*/

#include <stream.h>
#include <stdlib.h>
#include <audio_config.h>

#include "audio_mixer_processing.h"
#include "audio_mixer_processing_chain_config.h"
#include "audio_mixer_processing_common.h"
#include "audio_mixer_processing_multiband_compander.h"
#include "audio_mixer.h"
#include "chain.h"
#include "operators.h"
#include "audio_music_processing.h"
#include "panic.h"
#include "audio_mixer_connection.h"

typedef struct __mixer_processing_data
{
    kymera_chain_handle_t  chain;
    processing_chain_t chain_type;
} mixer_processing_data_t;

/*****************************************************************************/
static bool mixerProcessingIsMusicChainType(processing_chain_t type)
{
    return ((type == mono_music_processing) || (type == stereo_music_processing) ||
            (type == mono_music_2band_mbc_processing) || (type == stereo_music_2band_mbc_processing) ||
            (type == mono_music_3band_mbc_processing) || (type == stereo_music_3band_mbc_processing));
}

/****************************************************************************
DESCRIPTION
    Helper function to register music processing operators with music processing lib
*/
static void mixerProcessingRegisterMusicProcessingOperators(mixer_processing_data_t* mixer_processing_data)
{
    if(mixerProcessingIsMusicChainType(mixer_processing_data->chain_type))
    {
        Operator compander_op;
        Operator peq_op;
        Operator dbe_op;
        Operator vse_op;

        compander_op = ChainGetOperatorByRole(mixer_processing_data->chain, processing_compander_role);
        if(compander_op)
            AudioMusicProcessingRegisterRole(audio_music_processing_compander_role, compander_op);

        peq_op = ChainGetOperatorByRole(mixer_processing_data->chain, peq_role);
        if(peq_op)
            AudioMusicProcessingRegisterRole(audio_music_processing_user_peq_role, peq_op);

        dbe_op = ChainGetOperatorByRole(mixer_processing_data->chain, dbe_role);
        if(dbe_op)
            AudioMusicProcessingRegisterRole(audio_music_processing_dynamic_bass_enhancement_role, dbe_op);

        vse_op = ChainGetOperatorByRole(mixer_processing_data->chain, vse_role);
        if(vse_op)
            AudioMusicProcessingRegisterRole(audio_music_processing_volume_spatial_enhancement_role, vse_op);
    }

    mixerProcessingRegisterMultiBandCompanderOperators(mixer_processing_data->chain, mixer_processing_data->chain_type);
}

/****************************************************************************
DESCRIPTION
    Helper function to unregister music processing operators with music processing lib 
*/
static void mixerProcessingUnregisterMusicProcessingOperators(mixer_processing_data_t* mixer_processing_data)
{
    if(mixerProcessingIsMusicChainType(mixer_processing_data->chain_type))
    {
        AudioMusicProcessingUnregisterRole(audio_music_processing_compander_role);
        AudioMusicProcessingUnregisterRole(audio_music_processing_user_peq_role);
        AudioMusicProcessingUnregisterRole(audio_music_processing_dynamic_bass_enhancement_role);
        AudioMusicProcessingUnregisterRole(audio_music_processing_volume_spatial_enhancement_role);
    }

    mixerProcessingUnregisterMultiBandCompanderOperators(mixer_processing_data->chain_type);
}

/****************************************************************************
DESCRIPTION
     Helper function to return the required chain type given key input parameters
*/
static processing_chain_t mixerProcessingGetMusicChainType(bool is_mono)
{
    compander_band_selection_t band = AudioConfigGetMusicProcessingCompanderBand();

    if(band == three_band_compander)
        return (is_mono ? mono_music_3band_mbc_processing : stereo_music_3band_mbc_processing);
    else if(band == two_band_compander)
        return (is_mono ? mono_music_2band_mbc_processing : stereo_music_2band_mbc_processing);
    else
        return (is_mono ? mono_music_processing : stereo_music_processing);
}

/****************************************************************************
DESCRIPTION
     Helper function to return the required chain type given key input parameters
*/
static processing_chain_t mixerProcessingGetChainType(connection_type_t connection_type, bool is_mono)
{
    switch (connection_type)
    {
        case CONNECTION_TYPE_MUSIC_A2DP:
        case CONNECTION_TYPE_MUSIC:
            return mixerProcessingGetMusicChainType(is_mono);

        case CONNECTION_TYPE_MUSIC_BA_TX:
        case CONNECTION_TYPE_MUSIC_BA_RX:
            return no_processing;

        case CONNECTION_TYPE_TONES:
            return (is_mono ? mono_prompt_processing : stereo_prompt_processing);

        case CONNECTION_TYPE_VOICE:
            return voice_processing;

        default:
            Panic();
            return no_processing;
    }
}

/****************************************************************************
DESCRIPTION
    Helper function to create processing chains of voice prompts, voice and music.
*/
static kymera_chain_handle_t mixerProcessingCreateProcessingChain(processing_chain_t type,
                                                        bool attenuate_volume_pre_processing)
{
    const chain_config_t* config = mixerProcessingGetChainConfig(type);

    if(config)
    {
        const operator_filters_t* filter = mixerProcessingGetChainFilter(type, attenuate_volume_pre_processing);
        return ChainCreateWithFilter(config, filter);
    }

    return NULL;
}

/****************************************************************************
DESCRIPTION
    Helper function to configure voice prompts chain.
*/
static void mixerProcessingStereoPromptChainConfigure(kymera_chain_handle_t chain)
{
    Operator mixer_op = ChainGetOperatorByRole(chain, processing_stereo_to_mono_role);

    if (mixer_op)
        OperatorsMixerSetChannelsPerStream(mixer_op, 1, 1, 0);
}

/****************************************************************************
DESCRIPTION
    Helper function to configure operators in the processing chain 
*/
static void mixerProcessingChainConfigure(kymera_chain_handle_t chain, uint32 output_sample_rate, processing_chain_t type)
{
    mixerProcessingStereoPromptChainConfigure(chain);
    mixerProcessingSetMultiBandCompanderMixerStreams(chain, type);
    mixerProcessingSetMultiBandCompanderMode(chain, music_processing_mode_passthrough, type);
    /* Common configuration */
    ChainConfigureSampleRate(chain, output_sample_rate, NULL, 0);
}

/****************************************************************************
DESCRIPTION
    Helper function to set the UCID of the passthrough, compander, mixer,
    vse and dbe
*/
static void mixerProcessingSetUcid(kymera_chain_handle_t chain, processing_chain_t type)
{
    Operator passthrough_op;
    Operator compander_op;
    Operator vse_op;
    Operator dbe_op;
    Operator peq_op;

    passthrough_op = ChainGetOperatorByRole(chain, pre_processing_role);
    if (passthrough_op)
    {
        OperatorsStandardSetUCID(passthrough_op, ucid_passthrough_processing);
    }

    compander_op = ChainGetOperatorByRole(chain, processing_compander_role);
    if (compander_op)
    {
        OperatorsStandardSetUCID(compander_op, ucid_compander_processing);
    }

    vse_op = ChainGetOperatorByRole(chain, vse_role);
    if (vse_op)
    {
        OperatorsStandardSetUCID(vse_op, ucid_vse_processing);
    }

    dbe_op = ChainGetOperatorByRole(chain, dbe_role);
    if (dbe_op)
    {
        OperatorsStandardSetUCID(dbe_op, ucid_dbe_processing);
    }

    peq_op = ChainGetOperatorByRole(chain, peq_role);
    if (peq_op)
    {
        OperatorsStandardSetUCID(peq_op, ucid_peq_resampler_0);
    }

    mixerProcessingSetMultiBandCompanderUcid(chain, type);
}

static kymera_chain_handle_t mixerProcessingCreateChain(processing_chain_t type,
                                                        bool attenuate_volume_pre_processing)
{
    switch (type)
    {
        case voice_processing:
        case mono_prompt_processing:
        case stereo_prompt_processing:
        case mono_music_processing:
        case stereo_music_processing:
        case no_processing:
            return mixerProcessingCreateProcessingChain(type, attenuate_volume_pre_processing);

        case mono_music_2band_mbc_processing:
        case stereo_music_2band_mbc_processing:
        case mono_music_3band_mbc_processing:
        case stereo_music_3band_mbc_processing:
            return mixerProcessingCreateMultiBandCompanderChain(type, attenuate_volume_pre_processing);

        default:
            Panic();
            return NULL;
    }
}

/******************************************************************************/
mixer_processing_context_t mixerProcessingCreate(connection_type_t connection_type,
                                                 uint32 output_sample_rate,
                                                 bool is_mono,
                                                 bool attenuate_volume_pre_processing)
{
    processing_chain_t type = mixerProcessingGetChainType(connection_type, is_mono);

    kymera_chain_handle_t chain = mixerProcessingCreateChain(type, attenuate_volume_pre_processing);

    if(chain)
    {
        mixer_processing_data_t* mixer_processing_data = PanicUnlessNew(mixer_processing_data_t);

        mixer_processing_data->chain = chain;
        mixer_processing_data->chain_type = type;

        mixerProcessingChainConfigure(chain, output_sample_rate, type);
        ChainConnect(chain);
        mixerProcessingSetUcid(chain, type);
        mixerProcessingRegisterMusicProcessingOperators(mixer_processing_data);

        return mixer_processing_data;
    }
    return NULL;
}

/******************************************************************************/
kymera_chain_handle_t mixerProcessingGetChain(mixer_processing_context_t context)
{
    if(context)
        return context->chain;
    
    return NULL;
}

/******************************************************************************/
void mixerProcessingStop(mixer_processing_context_t context)
{
    if(context)
        ChainStop(context->chain);
}

/******************************************************************************/
void mixerProcessingDestroy(mixer_processing_context_t context)
{
    if(context)
    {
        mixerProcessingUnregisterMusicProcessingOperators(context);
        ChainDestroy(context->chain);
        free(context);
    }
}

/******************************************************************************/
void mixerProcessingStart(mixer_processing_context_t context)
{
    if(context)
        ChainStart(context->chain);
}

/******************************************************************************/
void mixerProcessingChangeSampleRate(mixer_processing_context_t context,
                                     uint32 output_sample_rate)
{
    if(context)
        ChainConfigureSampleRateWithoutAllocatingBuffer(context->chain, output_sample_rate, NULL, 0);
}
