/****************************************************************************
Copyright (c) 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_mixer_processing_chain_config_mbc.c

DESCRIPTION
    Implementation of multi-band compander functions to return the appropriate chain configuration.
*/

#include <operators.h>
#include <panic.h>
#include <stdlib.h>
#include <string.h>

#include "audio_mixer_processing_common.h"
#include "audio_mixer_processing_multiband_compander.h"
#include "audio_mixer_channel_roles.h"
#include "audio_mixer_latency.h"
#include "audio_mixer_gain.h"
#include <audio_plugin_ucid.h>
#include "audio_music_processing.h"

#define LEFT_TERMINAL   0
#define RIGHT_TERMINAL  1

#define CROSSOVER_OUT_LEFT_LOW_FREQ             (0)
#define CROSSOVER_OUT_LEFT_HIGH_FREQ            (1)
#define CROSSOVER_OUT_RIGHT_LOW_FREQ            (2)
#define CROSSOVER_OUT_RIGHT_HIGH_FREQ           (3)

#define SECOND_CROSSOVER_OUT_LEFT_MID_FREQ      (0)
#define SECOND_CROSSOVER_OUT_LEFT_HIGH_FREQ     (1)
#define SECOND_CROSSOVER_OUT_RIGHT_MID_FREQ     (2)
#define SECOND_CROSSOVER_OUT_RIGHT_HIGH_FREQ    (3)

#define MIXER_IN_LEFT_LOW_FREQ_2BAND            (0)
#define MIXER_IN_RIGHT_LOW_FREQ_2BAND           (1)
#define MIXER_IN_LEFT_HIGH_FREQ_2BAND           (2)
#define MIXER_IN_RIGHT_HIGH_FREQ_2BAND          (3)
#define MONO_MIXER_IN_LEFT_HIGH_FREQ_2BAND      (1)

#define MIXER_IN_LEFT_LOW_FREQ_3BAND            (0)
#define MIXER_IN_RIGHT_LOW_FREQ_3BAND           (1)
#define MIXER_IN_LEFT_MID_FREQ_3BAND            (2)
#define MIXER_IN_RIGHT_MID_FREQ_3BAND           (3)
#define MIXER_IN_LEFT_HIGH_FREQ_3BAND           (4)
#define MIXER_IN_RIGHT_HIGH_FREQ_3BAND          (5)
#define MONO_MIXER_IN_LEFT_MID_FREQ_3BAND       (1)
#define MONO_MIXER_IN_LEFT_HIGH_FREQ_3BAND      (2)

#define UNUSED_TERMINAL 0xff

/* Internal roles used within MBC chain */
#define MIXER_MUSIC_MID_FREQ_LEFT_PATH  (mixer_prompt_right_channel+1U)
#define MIXER_MUSIC_MID_FREQ_RIGHT_PATH (mixer_prompt_right_channel+2U)
#define MIXER_MUSIC_HIGH_FREQ_LEFT_PATH  (mixer_prompt_right_channel+3U)
#define MIXER_MUSIC_HIGH_FREQ_RIGHT_PATH (mixer_prompt_right_channel+4U)

#define MIXER_DEFAULT_PRIMARY_STREAM 1

#ifdef USE_SECOND_CORE
#define MAKE_OPERATOR_CONFIG_FOR_MIXER(id, role) MAKE_OPERATOR_CONFIG_P1(id, role)
#define MAKE_OPERATOR_CONFIG_FOR_MIXER_WITH_SETUP(id, role, setup_items) MAKE_OPERATOR_CONFIG_P1_WITH_SETUP(id, role, setup_items)
#else
#define MAKE_OPERATOR_CONFIG_FOR_MIXER(id, role) MAKE_OPERATOR_CONFIG(id, role)
#define MAKE_OPERATOR_CONFIG_FOR_MIXER_WITH_SETUP(id, role, setup_items) MAKE_OPERATOR_CONFIG_WITH_SETUP(id, role, setup_items)
#endif

#define GET_OP_MODE_FOR_LOWFREQ_PATH(mode) (mode)
#define GET_OP_MODE_FOR_MIDFREQ_PATH(mode) (((mode) == music_processing_mode_passthrough) ? music_processing_mode_standby : (mode))
#define GET_OP_MODE_FOR_HIGHFREQ_PATH(mode) (((mode) == music_processing_mode_passthrough) ? music_processing_mode_standby : (mode))

/* Buffer configuration for use with concurrent mixer graph */
static const operator_setup_item_t mixer_pre_processing_setup[] =
{
    OPERATORS_SETUP_STANDARD_BUFFER_SIZE_FROM_LATENCY(CONCURRENT_PRE_PROCESSING_BUFFER_MILLISECONDS)
};

/* Discrete 2band compander chain config */
static const operator_config_t ops_2band_mbc[] =
{
    MAKE_OPERATOR_CONFIG_FOR_MIXER_WITH_SETUP(capability_id_passthrough, pre_processing_role, mixer_pre_processing_setup),
    MAKE_OPERATOR_CONFIG_FOR_MIXER(capability_id_crossover_2band, crossover_2band_role),
    MAKE_OPERATOR_CONFIG_FOR_MIXER(capability_id_compander, compander_2band_low_freq_role),
    MAKE_OPERATOR_CONFIG_FOR_MIXER(capability_id_compander, compander_2band_high_freq_role),
    MAKE_OPERATOR_CONFIG_FOR_MIXER(capability_id_mixer, mixer_2band_role),
    MAKE_OPERATOR_CONFIG_FOR_MIXER(capability_id_peq, peq_role),
    MAKE_OPERATOR_CONFIG_FOR_MIXER(capability_id_vse, vse_role),
    MAKE_OPERATOR_CONFIG_FOR_MIXER(capability_id_bass_enhance, dbe_role)
};

static const operator_path_node_t left_path_low_freq_2band[] = 
{
    { pre_processing_role,           LEFT_TERMINAL,                 LEFT_TERMINAL },
    { crossover_2band_role,          LEFT_TERMINAL,                 CROSSOVER_OUT_LEFT_LOW_FREQ },
    { compander_2band_low_freq_role, LEFT_TERMINAL,                 LEFT_TERMINAL },
    { mixer_2band_role,              MIXER_IN_LEFT_LOW_FREQ_2BAND,  LEFT_TERMINAL },
    { peq_role,                      LEFT_TERMINAL,                 LEFT_TERMINAL },
    { vse_role,                      LEFT_TERMINAL,                 LEFT_TERMINAL },
    { dbe_role,                      LEFT_TERMINAL,                 LEFT_TERMINAL },
};

static const operator_path_node_t right_path_low_freq_2band[] = 
{
    { pre_processing_role,           RIGHT_TERMINAL,                RIGHT_TERMINAL },
    { crossover_2band_role,          RIGHT_TERMINAL,                CROSSOVER_OUT_RIGHT_LOW_FREQ },
    { compander_2band_low_freq_role, RIGHT_TERMINAL,                RIGHT_TERMINAL },
    { mixer_2band_role,              MIXER_IN_RIGHT_LOW_FREQ_2BAND, RIGHT_TERMINAL },
    { peq_role,                      RIGHT_TERMINAL,                RIGHT_TERMINAL },
    { vse_role,                      RIGHT_TERMINAL,                RIGHT_TERMINAL },
    { dbe_role,                      RIGHT_TERMINAL,                RIGHT_TERMINAL },
};

static const operator_path_node_t left_path_high_freq_2band[] = 
{
    { crossover_2band_role,           UNUSED_TERMINAL,              CROSSOVER_OUT_LEFT_HIGH_FREQ },
    { compander_2band_high_freq_role, LEFT_TERMINAL,                LEFT_TERMINAL },
    { mixer_2band_role,               MIXER_IN_LEFT_HIGH_FREQ_2BAND,UNUSED_TERMINAL },
};

static const operator_path_node_t right_path_high_freq_2band[] = 
{
    { crossover_2band_role,           UNUSED_TERMINAL,                  CROSSOVER_OUT_RIGHT_HIGH_FREQ },
    { compander_2band_high_freq_role, RIGHT_TERMINAL,                   RIGHT_TERMINAL },
    { mixer_2band_role,               MIXER_IN_RIGHT_HIGH_FREQ_2BAND,   UNUSED_TERMINAL },
};

static const operator_path_node_t mono_left_path_high_freq_2band[] =
{
    { crossover_2band_role,           UNUSED_TERMINAL,              CROSSOVER_OUT_LEFT_HIGH_FREQ },
    { compander_2band_high_freq_role, LEFT_TERMINAL,                LEFT_TERMINAL },
    { mixer_2band_role,               MONO_MIXER_IN_LEFT_HIGH_FREQ_2BAND,UNUSED_TERMINAL },
};

static const operator_path_t paths_2band_mbc_stereo[] = 
{
    {mixer_music_left_channel,  path_with_in_and_out, ARRAY_DIM((left_path_low_freq_2band)),  left_path_low_freq_2band},
    {MIXER_MUSIC_HIGH_FREQ_LEFT_PATH,  path_with_no_in_or_out, ARRAY_DIM((left_path_high_freq_2band)),  left_path_high_freq_2band},
    {mixer_music_right_channel, path_with_in_and_out, ARRAY_DIM((right_path_low_freq_2band)), right_path_low_freq_2band},
    {MIXER_MUSIC_HIGH_FREQ_RIGHT_PATH,  path_with_no_in_or_out, ARRAY_DIM((right_path_high_freq_2band)),  right_path_high_freq_2band}
};

static const operator_path_t paths_2band_mbc_mono[] = 
{
    {mixer_music_left_channel,  path_with_in_and_out, ARRAY_DIM((left_path_low_freq_2band)),  left_path_low_freq_2band},
    {MIXER_MUSIC_HIGH_FREQ_LEFT_PATH,  path_with_no_in_or_out, ARRAY_DIM((mono_left_path_high_freq_2band)),  mono_left_path_high_freq_2band}
};

/* Chain Config Statics */
static const chain_config_t config_mono_2band_mbc =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_mixer_music_processing,
                                   audio_ucid_mixer_processing_stereo_music,
                                   ops_2band_mbc,
                                   paths_2band_mbc_mono);

static const chain_config_t config_stereo_2band_mbc =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_mixer_music_processing,
                                   audio_ucid_mixer_processing_stereo_music,
                                   ops_2band_mbc,
                                   paths_2band_mbc_stereo);

/* Discrete 3band compander chain config */
static const operator_config_t ops_3band_mbc[] =
{
    MAKE_OPERATOR_CONFIG_FOR_MIXER_WITH_SETUP(capability_id_passthrough, pre_processing_role, mixer_pre_processing_setup),
    MAKE_OPERATOR_CONFIG_FOR_MIXER(capability_id_crossover_2band, first_crossover_3band_role),
    MAKE_OPERATOR_CONFIG_FOR_MIXER(capability_id_peq, peq_3band_role),
    MAKE_OPERATOR_CONFIG_FOR_MIXER(capability_id_crossover_2band, second_crossover_3band_role),
    MAKE_OPERATOR_CONFIG_FOR_MIXER(capability_id_compander, compander_3band_low_freq_role),
    MAKE_OPERATOR_CONFIG_FOR_MIXER(capability_id_compander, compander_3band_mid_freq_role),
    MAKE_OPERATOR_CONFIG_FOR_MIXER(capability_id_compander, compander_3band_high_freq_role),
    MAKE_OPERATOR_CONFIG_FOR_MIXER(capability_id_mixer, mixer_3band_role),
    MAKE_OPERATOR_CONFIG_FOR_MIXER(capability_id_peq, peq_role),
    MAKE_OPERATOR_CONFIG_FOR_MIXER(capability_id_vse, vse_role),
    MAKE_OPERATOR_CONFIG_FOR_MIXER(capability_id_bass_enhance, dbe_role)
};

static const operator_path_node_t left_path_low_freq_3band[] =
{
    { pre_processing_role,           LEFT_TERMINAL,                 LEFT_TERMINAL },
    { first_crossover_3band_role,    LEFT_TERMINAL,                 CROSSOVER_OUT_LEFT_LOW_FREQ },
    { peq_3band_role,                LEFT_TERMINAL,                 LEFT_TERMINAL },
    { compander_3band_low_freq_role, LEFT_TERMINAL,                 LEFT_TERMINAL },
    { mixer_3band_role,              MIXER_IN_LEFT_LOW_FREQ_3BAND,  LEFT_TERMINAL },
    { peq_role,                      LEFT_TERMINAL,                 LEFT_TERMINAL },
    { vse_role,                      LEFT_TERMINAL,                 LEFT_TERMINAL },
    { dbe_role,                      LEFT_TERMINAL,                 LEFT_TERMINAL },
};

static const operator_path_node_t right_path_low_freq_3band[] =
{
    { pre_processing_role,           RIGHT_TERMINAL,                RIGHT_TERMINAL },
    { first_crossover_3band_role,    RIGHT_TERMINAL,                CROSSOVER_OUT_RIGHT_LOW_FREQ },
    { peq_3band_role,                RIGHT_TERMINAL,                RIGHT_TERMINAL },
    { compander_3band_low_freq_role, RIGHT_TERMINAL,                RIGHT_TERMINAL },
    { mixer_3band_role,              MIXER_IN_RIGHT_LOW_FREQ_3BAND, RIGHT_TERMINAL },
    { peq_role,                      RIGHT_TERMINAL,                RIGHT_TERMINAL },
    { vse_role,                      RIGHT_TERMINAL,                RIGHT_TERMINAL },
    { dbe_role,                      RIGHT_TERMINAL,                RIGHT_TERMINAL },
};

static const operator_path_node_t left_path_mid_freq_3band[] =
{
    { first_crossover_3band_role,     UNUSED_TERMINAL,              CROSSOVER_OUT_LEFT_HIGH_FREQ },
    { second_crossover_3band_role,    LEFT_TERMINAL,                SECOND_CROSSOVER_OUT_LEFT_MID_FREQ },
    { compander_3band_mid_freq_role,  LEFT_TERMINAL,                LEFT_TERMINAL },
    { mixer_3band_role,               MIXER_IN_LEFT_MID_FREQ_3BAND, UNUSED_TERMINAL },
};

static const operator_path_node_t right_path_mid_freq_3band[] =
{
    { first_crossover_3band_role,     UNUSED_TERMINAL,              CROSSOVER_OUT_RIGHT_HIGH_FREQ },
    { second_crossover_3band_role,    RIGHT_TERMINAL,               SECOND_CROSSOVER_OUT_RIGHT_MID_FREQ },
    { compander_3band_mid_freq_role,  RIGHT_TERMINAL,               RIGHT_TERMINAL },
    { mixer_3band_role,               MIXER_IN_RIGHT_MID_FREQ_3BAND,UNUSED_TERMINAL },
};

static const operator_path_node_t left_path_high_freq_3band[] =
{
    { second_crossover_3band_role,    UNUSED_TERMINAL,                SECOND_CROSSOVER_OUT_LEFT_HIGH_FREQ },
    { compander_3band_high_freq_role,  LEFT_TERMINAL,                  LEFT_TERMINAL },
    { mixer_3band_role,               MIXER_IN_LEFT_HIGH_FREQ_3BAND,  UNUSED_TERMINAL },
};

static const operator_path_node_t right_path_high_freq_3band[] =
{
    { second_crossover_3band_role,    UNUSED_TERMINAL,                SECOND_CROSSOVER_OUT_RIGHT_HIGH_FREQ },
    { compander_3band_high_freq_role,  RIGHT_TERMINAL,                 RIGHT_TERMINAL },
    { mixer_3band_role,               MIXER_IN_RIGHT_HIGH_FREQ_3BAND, UNUSED_TERMINAL },
};

static const operator_path_node_t mono_left_path_mid_freq_3band[] =
{
    { first_crossover_3band_role,     UNUSED_TERMINAL,              CROSSOVER_OUT_LEFT_HIGH_FREQ },
    { second_crossover_3band_role,    LEFT_TERMINAL,                SECOND_CROSSOVER_OUT_LEFT_MID_FREQ },
    { compander_3band_mid_freq_role,  LEFT_TERMINAL,                LEFT_TERMINAL },
    { mixer_3band_role,               MONO_MIXER_IN_LEFT_MID_FREQ_3BAND, UNUSED_TERMINAL },
};

static const operator_path_node_t mono_left_path_high_freq_3band[] =
{
    { second_crossover_3band_role,    UNUSED_TERMINAL,                SECOND_CROSSOVER_OUT_LEFT_HIGH_FREQ },
    { compander_3band_high_freq_role,  LEFT_TERMINAL,                  LEFT_TERMINAL },
    { mixer_3band_role,               MONO_MIXER_IN_LEFT_HIGH_FREQ_3BAND,  UNUSED_TERMINAL },
};

static const operator_path_t paths_3band_mbc_stereo[] =
{
    {mixer_music_left_channel,  path_with_in_and_out, ARRAY_DIM((left_path_low_freq_3band)),  left_path_low_freq_3band},
    {MIXER_MUSIC_MID_FREQ_LEFT_PATH,  path_with_no_in_or_out, ARRAY_DIM((left_path_mid_freq_3band)),  left_path_mid_freq_3band},
    {MIXER_MUSIC_HIGH_FREQ_LEFT_PATH,  path_with_no_in_or_out, ARRAY_DIM((left_path_high_freq_3band)),  left_path_high_freq_3band},
    {mixer_music_right_channel, path_with_in_and_out, ARRAY_DIM((right_path_low_freq_3band)), right_path_low_freq_3band},
    {MIXER_MUSIC_MID_FREQ_RIGHT_PATH,  path_with_no_in_or_out, ARRAY_DIM((right_path_mid_freq_3band)),  right_path_mid_freq_3band},
    {MIXER_MUSIC_HIGH_FREQ_RIGHT_PATH,  path_with_no_in_or_out, ARRAY_DIM((right_path_high_freq_3band)),  right_path_high_freq_3band}
};

static const operator_path_t paths_3band_mbc_mono[] =
{
    {mixer_music_left_channel,  path_with_in_and_out, ARRAY_DIM((left_path_low_freq_3band)),  left_path_low_freq_3band},
    {MIXER_MUSIC_MID_FREQ_LEFT_PATH,  path_with_no_in_or_out, ARRAY_DIM((mono_left_path_mid_freq_3band)),  mono_left_path_mid_freq_3band},
    {MIXER_MUSIC_HIGH_FREQ_LEFT_PATH,  path_with_no_in_or_out, ARRAY_DIM((mono_left_path_high_freq_3band)),  mono_left_path_high_freq_3band}
};

/* Chain Config Statics */
static const chain_config_t config_mono_3band_mbc =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_mixer_music_processing,
                                   audio_ucid_mixer_processing_stereo_music,
                                   ops_3band_mbc,
                                   paths_3band_mbc_mono);

static const chain_config_t config_stereo_3band_mbc =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_mixer_music_processing,
                                   audio_ucid_mixer_processing_stereo_music,
                                   ops_3band_mbc,
                                   paths_3band_mbc_stereo);


/* Filter out all music processing operators except PEQ */
static const operator_config_t processing_ops_low_power_filter[] = 
{
    MAKE_OPERATOR_CONFIG(capability_id_none, pre_processing_role),
    MAKE_OPERATOR_CONFIG(capability_id_none, crossover_2band_role),
    MAKE_OPERATOR_CONFIG(capability_id_none, compander_2band_low_freq_role),
    MAKE_OPERATOR_CONFIG(capability_id_none, compander_2band_high_freq_role),
    MAKE_OPERATOR_CONFIG(capability_id_none, mixer_2band_role),
    MAKE_OPERATOR_CONFIG(capability_id_none, peq_3band_role),
    MAKE_OPERATOR_CONFIG(capability_id_none, second_crossover_3band_role),
    MAKE_OPERATOR_CONFIG(capability_id_none, compander_3band_low_freq_role),
    MAKE_OPERATOR_CONFIG(capability_id_none, compander_3band_mid_freq_role),
    MAKE_OPERATOR_CONFIG(capability_id_none, compander_3band_high_freq_role),
    MAKE_OPERATOR_CONFIG(capability_id_none, mixer_3band_role),
    MAKE_OPERATOR_CONFIG(capability_id_none, vse_role),
    MAKE_OPERATOR_CONFIG(capability_id_none, dbe_role)
};

static const operator_filters_t processing_low_power_filter = 
{
    ARRAY_DIM((processing_ops_low_power_filter)),  processing_ops_low_power_filter
};

/* Filter out vse operator for mono input */
static const operator_config_t processing_ops_mono_music_input_filter[] =
{
    MAKE_OPERATOR_CONFIG(capability_id_none, vse_role)
};

static const operator_filters_t processing_mono_music_input_filter =
{
    ARRAY_DIM((processing_ops_mono_music_input_filter)),  processing_ops_mono_music_input_filter
};

/******************************************************************************/
static const chain_config_t* mixerProcessingGetMultiBandCompanderChainConfig(processing_chain_t chain_type)
{
    switch (chain_type)
    {
        case mono_music_2band_mbc_processing:
            return &config_mono_2band_mbc;

        case stereo_music_2band_mbc_processing:
            return &config_stereo_2band_mbc;

        case mono_music_3band_mbc_processing:
            return &config_mono_3band_mbc;

        case stereo_music_3band_mbc_processing:
            return &config_stereo_3band_mbc;

        default:
            return NULL;
    }
}

/******************************************************************************/
static const operator_filters_t *mixerProcessingGetMultiBandCompanderChainFilter(processing_chain_t chain_type,
                                                        bool attenuate_volume_pre_processing)
{
    if(!attenuate_volume_pre_processing)
        return &processing_low_power_filter;

    if((chain_type == mono_music_2band_mbc_processing) ||(chain_type == mono_music_3band_mbc_processing))
        return &processing_mono_music_input_filter;

    return NULL;
}

/******************************************************************************/
static bool mixerProcessingIs2BandCompanderType(processing_chain_t type)
{
    return ((type == mono_music_2band_mbc_processing) || (type == stereo_music_2band_mbc_processing));
}

static bool mixerProcessingIs3BandCompanderType(processing_chain_t type)
{
    return ((type == mono_music_3band_mbc_processing) || (type == stereo_music_3band_mbc_processing));
}

/******************************************************************************/
static void mixerProcessingSet2BandCompanderMixerStreams(kymera_chain_handle_t chain, processing_chain_t type)
{
    Operator mixer_op = ChainGetOperatorByRole(chain, mixer_2band_role);

    if (mixer_op)
    {
        uint16 num_music_channels = ((type == mono_music_2band_mbc_processing) ? 1 : 2);

        OperatorsMixerSetChannelsPerStream(mixer_op, num_music_channels, num_music_channels, 0);
        OperatorsMixerSetPrimaryStream(mixer_op, MIXER_DEFAULT_PRIMARY_STREAM);
    }
}

/******************************************************************************/
static void mixerProcessingSet3BandCompanderMixerStreams(kymera_chain_handle_t chain, processing_chain_t type)
{
    Operator mixer_op = ChainGetOperatorByRole(chain, mixer_3band_role);

    if (mixer_op)
    {
        uint16 num_music_channels = ((type == mono_music_3band_mbc_processing) ? 1U : 2U);

        OperatorsMixerSetChannelsPerStream(mixer_op, num_music_channels, num_music_channels, num_music_channels);
        OperatorsMixerSetPrimaryStream(mixer_op, MIXER_DEFAULT_PRIMARY_STREAM);
    }
}

/******************************************************************************/
static void mixerProcessingSet2BandCompanderMode(kymera_chain_handle_t chain, music_processing_mode_t mode)
{
    Operator xover_op;
    Operator compander_low_freq_op;
    Operator compander_high_freq_op;
    Operator mixer_op;

/* Low-freq path operators */
    xover_op = ChainGetOperatorByRole(chain, crossover_2band_role);
    if (xover_op)
    {
        OperatorsSetMusicProcessingMode(xover_op, GET_OP_MODE_FOR_LOWFREQ_PATH(mode));
    }

    compander_low_freq_op = ChainGetOperatorByRole(chain, compander_2band_low_freq_role);
    if (compander_low_freq_op)
    {
        OperatorsSetMusicProcessingMode(compander_low_freq_op, GET_OP_MODE_FOR_LOWFREQ_PATH(mode));
    }

    mixer_op = ChainGetOperatorByRole(chain, mixer_2band_role);
    if (mixer_op)
    {
        OperatorsMixerSetGains(mixer_op, GAIN_UNITY, GAIN_UNITY, GAIN_UNITY);
    }

/* High-freq path operator */
    compander_high_freq_op = ChainGetOperatorByRole(chain, compander_2band_high_freq_role);
    if (compander_high_freq_op)
    {
        OperatorsSetMusicProcessingMode(compander_high_freq_op, GET_OP_MODE_FOR_HIGHFREQ_PATH(mode));
    }
}

/******************************************************************************/
static void mixerProcessingSet3BandCompanderMode(kymera_chain_handle_t chain, music_processing_mode_t mode)
{
    Operator first_xover_op;
    Operator peq_op;
    Operator second_xover_op;
    Operator compander_low_freq_op;
    Operator compander_mid_freq_op;
    Operator compander_high_freq_op;
    Operator mixer_op;

/* Low-freq path operator */
    first_xover_op = ChainGetOperatorByRole(chain, first_crossover_3band_role);
    if (first_xover_op)
    {
        OperatorsSetMusicProcessingMode(first_xover_op, GET_OP_MODE_FOR_LOWFREQ_PATH(mode));
    }

    peq_op = ChainGetOperatorByRole(chain, peq_3band_role);
    if (peq_op)
    {
        OperatorsSetMusicProcessingMode(peq_op, GET_OP_MODE_FOR_LOWFREQ_PATH(mode));
    }

    compander_low_freq_op = ChainGetOperatorByRole(chain, compander_3band_low_freq_role);
    if (compander_low_freq_op)
    {
        OperatorsSetMusicProcessingMode(compander_low_freq_op, GET_OP_MODE_FOR_LOWFREQ_PATH(mode));
    }

    mixer_op = ChainGetOperatorByRole(chain, mixer_3band_role);
    if (mixer_op)
    {
        OperatorsMixerSetGains(mixer_op, GAIN_UNITY, GAIN_UNITY, GAIN_UNITY);
    }

/* Mid-freq path operator */
    second_xover_op = ChainGetOperatorByRole(chain, second_crossover_3band_role);
    if (second_xover_op)
    {
        OperatorsSetMusicProcessingMode(second_xover_op, GET_OP_MODE_FOR_MIDFREQ_PATH(mode));
    }

    compander_mid_freq_op = ChainGetOperatorByRole(chain, compander_3band_mid_freq_role);
    if (compander_mid_freq_op)
    {
        OperatorsSetMusicProcessingMode(compander_mid_freq_op, GET_OP_MODE_FOR_MIDFREQ_PATH(mode));
    }

/* High-freq path operator */
    compander_high_freq_op = ChainGetOperatorByRole(chain, compander_3band_high_freq_role);
    if (compander_high_freq_op)
    {
        OperatorsSetMusicProcessingMode(compander_high_freq_op, GET_OP_MODE_FOR_HIGHFREQ_PATH(mode));
    }
}

/******************************************************************************/
static void mixerProcessingSet2BandCompanderUcid(kymera_chain_handle_t chain)
{
    Operator crossover_op;
    Operator compander_low_freq_op;
    Operator compander_high_freq_op;

    crossover_op = ChainGetOperatorByRole(chain, crossover_2band_role);
    if (crossover_op)
    {
        OperatorsStandardSetUCID(crossover_op, ucid_crossover_2band_mbc_processing);
    }

    compander_low_freq_op = ChainGetOperatorByRole(chain, compander_2band_low_freq_role);
    if (compander_low_freq_op)
    {
        OperatorsStandardSetUCID(compander_low_freq_op, ucid_compander_low_freq_2band_mbc_processing);
    }

    compander_high_freq_op = ChainGetOperatorByRole(chain, compander_2band_high_freq_role);
    if (compander_high_freq_op)
    {
        OperatorsStandardSetUCID(compander_high_freq_op, ucid_compander_high_freq_2band_mbc_processing);
    }
}

/******************************************************************************/
static void mixerProcessingSet3BandCompanderUcid(kymera_chain_handle_t chain)
{
    Operator first_crossover_op;
    Operator peq_op;
    Operator second_crossover_op;
    Operator compander_low_freq_op;
    Operator compander_mid_freq_op;
    Operator compander_high_freq_op;

    first_crossover_op = ChainGetOperatorByRole(chain, first_crossover_3band_role);
    if (first_crossover_op)
    {
        OperatorsStandardSetUCID(first_crossover_op, ucid_crossover_first_3band_mbc_processing);
    }

    peq_op = ChainGetOperatorByRole(chain, peq_3band_role);
    if (peq_op)
    {
        OperatorsStandardSetUCID(peq_op, ucid_peq_3band_mbc_processing);
    }

    second_crossover_op = ChainGetOperatorByRole(chain, second_crossover_3band_role);
    if (second_crossover_op)
    {
        OperatorsStandardSetUCID(second_crossover_op, ucid_crossover_second_3band_mbc_processing);
    }

    compander_low_freq_op = ChainGetOperatorByRole(chain, compander_3band_low_freq_role);
    if (compander_low_freq_op)
    {
        OperatorsStandardSetUCID(compander_low_freq_op, ucid_compander_low_freq_3band_mbc_processing);
    }

    compander_mid_freq_op = ChainGetOperatorByRole(chain, compander_3band_mid_freq_role);
    if (compander_mid_freq_op)
    {
        OperatorsStandardSetUCID(compander_mid_freq_op, ucid_compander_mid_freq_3band_mbc_processing);
    }

    compander_high_freq_op = ChainGetOperatorByRole(chain, compander_3band_high_freq_role);
    if (compander_high_freq_op)
    {
        OperatorsStandardSetUCID(compander_high_freq_op, ucid_compander_high_freq_3band_mbc_processing);
    }
}


/******************************************************************************/
kymera_chain_handle_t mixerProcessingCreateMultiBandCompanderChain(processing_chain_t type,
                                                        bool attenuate_volume_pre_processing)
{
    const chain_config_t* config = mixerProcessingGetMultiBandCompanderChainConfig(type);

    if(config)
    {
        const operator_filters_t* filter = mixerProcessingGetMultiBandCompanderChainFilter(type, attenuate_volume_pre_processing);
        return ChainCreateWithFilter(config, filter);
    }

    return NULL;
}

/******************************************************************************/
void mixerProcessingSetMultiBandCompanderMixerStreams(kymera_chain_handle_t chain, processing_chain_t chain_type)
{
    if(mixerProcessingIs2BandCompanderType(chain_type))
    {
        mixerProcessingSet2BandCompanderMixerStreams(chain, chain_type);
    }

    if(mixerProcessingIs3BandCompanderType(chain_type))
    {
        mixerProcessingSet3BandCompanderMixerStreams(chain, chain_type);
    }
}

/******************************************************************************/
void mixerProcessingSetMultiBandCompanderMode(kymera_chain_handle_t chain, music_processing_mode_t mode, processing_chain_t chain_type)
{
    if(mixerProcessingIs2BandCompanderType(chain_type))
    {
        mixerProcessingSet2BandCompanderMode(chain, mode);
    }

    if(mixerProcessingIs3BandCompanderType(chain_type))
    {
        mixerProcessingSet3BandCompanderMode(chain, mode);
    }
}

/******************************************************************************/
void mixerProcessingSetMultiBandCompanderUcid(kymera_chain_handle_t chain, processing_chain_t chain_type)
{
    if(mixerProcessingIs2BandCompanderType(chain_type))
    {
        mixerProcessingSet2BandCompanderUcid(chain);
    }

    if(mixerProcessingIs3BandCompanderType(chain_type))
    {
        mixerProcessingSet3BandCompanderUcid(chain);
    }
}

/******************************************************************************/
void mixerProcessingRegisterMultiBandCompanderOperators(kymera_chain_handle_t chain, processing_chain_t chain_type)
{
    if(mixerProcessingIs2BandCompanderType(chain_type))
    {
        Operator crossover_op = ChainGetOperatorByRole(chain, crossover_2band_role);
        Operator compander_low_freq_op = ChainGetOperatorByRole(chain, compander_2band_low_freq_role);
        Operator compander_high_freq_op = ChainGetOperatorByRole(chain, compander_2band_high_freq_role);

        AudioMusicProcessingRegisterMultiBandCompanderRole(audio_music_processing_2band_mbc_crossover_role, crossover_op);
        AudioMusicProcessingRegisterMultiBandCompanderRole(audio_music_processing_2band_mbc_low_freq_compander_role, compander_low_freq_op);
        AudioMusicProcessingRegisterMultiBandCompanderRole(audio_music_processing_2band_mbc_high_freq_compander_role, compander_high_freq_op);
    }

    if(mixerProcessingIs3BandCompanderType(chain_type))
    {
        Operator first_crossover_op = ChainGetOperatorByRole(chain, first_crossover_3band_role);
        Operator peq_op = ChainGetOperatorByRole(chain, peq_3band_role);
        Operator second_crossover_op = ChainGetOperatorByRole(chain, second_crossover_3band_role);
        Operator compander_low_freq_op = ChainGetOperatorByRole(chain, compander_3band_low_freq_role);
        Operator compander_mid_freq_op = ChainGetOperatorByRole(chain, compander_3band_mid_freq_role);
        Operator compander_high_freq_op = ChainGetOperatorByRole(chain, compander_3band_high_freq_role);

        AudioMusicProcessingRegisterMultiBandCompanderRole(audio_music_processing_3band_mbc_first_crossover_role, first_crossover_op);
        AudioMusicProcessingRegisterMultiBandCompanderRole(audio_music_processing_3band_mbc_peq_role, peq_op);
        AudioMusicProcessingRegisterMultiBandCompanderRole(audio_music_processing_3band_mbc_second_crossover_role, second_crossover_op);
        AudioMusicProcessingRegisterMultiBandCompanderRole(audio_music_processing_3band_mbc_low_freq_compander_role, compander_low_freq_op);
        AudioMusicProcessingRegisterMultiBandCompanderRole(audio_music_processing_3band_mbc_mid_freq_compander_role, compander_mid_freq_op);
        AudioMusicProcessingRegisterMultiBandCompanderRole(audio_music_processing_3band_mbc_high_freq_compander_role, compander_high_freq_op);
    }
}

/******************************************************************************/
void mixerProcessingUnregisterMultiBandCompanderOperators(processing_chain_t chain_type)
{
    if(mixerProcessingIs2BandCompanderType(chain_type))
    {
        AudioMusicProcessingUnregisterMultiBandCompanderRole(audio_music_processing_2band_mbc_crossover_role);
        AudioMusicProcessingUnregisterMultiBandCompanderRole(audio_music_processing_2band_mbc_low_freq_compander_role);
        AudioMusicProcessingUnregisterMultiBandCompanderRole(audio_music_processing_2band_mbc_high_freq_compander_role);
    }

    if(mixerProcessingIs3BandCompanderType(chain_type))
    {
        AudioMusicProcessingUnregisterMultiBandCompanderRole(audio_music_processing_3band_mbc_first_crossover_role);
        AudioMusicProcessingUnregisterMultiBandCompanderRole(audio_music_processing_3band_mbc_peq_role);
        AudioMusicProcessingUnregisterMultiBandCompanderRole(audio_music_processing_3band_mbc_second_crossover_role);
        AudioMusicProcessingUnregisterMultiBandCompanderRole(audio_music_processing_3band_mbc_low_freq_compander_role);
        AudioMusicProcessingUnregisterMultiBandCompanderRole(audio_music_processing_3band_mbc_mid_freq_compander_role);
        AudioMusicProcessingUnregisterMultiBandCompanderRole(audio_music_processing_3band_mbc_high_freq_compander_role);
    }
}
