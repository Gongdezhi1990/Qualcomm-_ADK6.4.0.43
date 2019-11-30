/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_voice_common_config.c

DESCRIPTION
    Implementation of chain construction functions.
*/

#include "audio_voice_common_config.h"

#include <audio_config.h>

#include "audio_voice_common_dsp.h"

#define UNCONNECTED           (255)

#define CVC_OPS_NB \
    MAKE_OPERATOR_CONFIG_PRIORITY_MEDIUM(capability_id_none, receive_role), \
    MAKE_OPERATOR_CONFIG_PRIORITY_HIGH(capability_id_none, send_role), \
    MAKE_OPERATOR_CONFIG_PRIORITY_MEDIUM(capability_id_cvc_receive_nb, cvc_receive_role), \
    MAKE_OPERATOR_CONFIG(capability_id_none, rate_adjustment_send_role)
    

#define CVC_OPS_WB \
    MAKE_OPERATOR_CONFIG_PRIORITY_MEDIUM(capability_id_none, receive_role), \
    MAKE_OPERATOR_CONFIG_PRIORITY_HIGH(capability_id_none, send_role), \
    MAKE_OPERATOR_CONFIG_PRIORITY_MEDIUM(capability_id_cvc_receive_wb, cvc_receive_role), \
    MAKE_OPERATOR_CONFIG(capability_id_none, rate_adjustment_send_role)
    

#define CVC_OPS_UWB \
    MAKE_OPERATOR_CONFIG_PRIORITY_MEDIUM(capability_id_none, receive_role), \
    MAKE_OPERATOR_CONFIG_PRIORITY_HIGH(capability_id_none, send_role), \
    MAKE_OPERATOR_CONFIG_PRIORITY_MEDIUM(capability_id_cvc_receive_uwb, cvc_receive_role), \
    MAKE_OPERATOR_CONFIG(capability_id_none, rate_adjustment_send_role)


#define CVC_OPS_SWB \
    MAKE_OPERATOR_CONFIG_PRIORITY_MEDIUM(capability_id_none, receive_role), \
    MAKE_OPERATOR_CONFIG_PRIORITY_HIGH(capability_id_none, send_role), \
    MAKE_OPERATOR_CONFIG_PRIORITY_MEDIUM(capability_id_cvc_receive_swb, cvc_receive_role), \
    MAKE_OPERATOR_CONFIG(capability_id_none, rate_adjustment_send_role)

static operator_config_t ops_nb_1mic_hs[] =
{
    CVC_OPS_NB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_hs_1mic_nb, cvc_send_role)
};

static operator_config_t ops_nb_1mic_speaker[] =
{
    CVC_OPS_NB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_spk_1mic_nb, cvc_send_role)
};

static operator_config_t ops_nb_1mic_handsfree[] =
{
    CVC_OPS_NB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_hf_1mic_nb, cvc_send_role)
};

static const operator_config_t ops_wb_1mic_hs[] =
{
    CVC_OPS_WB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_hs_1mic_wb, cvc_send_role)
};

static const operator_config_t ops_wb_1mic_speaker[] =
{
    CVC_OPS_WB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_spk_1mic_wb, cvc_send_role)
};

static const operator_config_t ops_wb_1mic_handsfree[] =
{
    CVC_OPS_WB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_hf_1mic_wb, cvc_send_role)
};


static const operator_config_t ops_nb_2mic_hs[] =
{
    CVC_OPS_NB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_hs_2mic_90deg_nb, cvc_send_role)
};

static const operator_config_t ops_nb_2mic_speaker[] =
{
    CVC_OPS_NB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_spk_2mic_0deg_nb, cvc_send_role)
};

static const operator_config_t ops_nb_2mic_handsfree[] =
{
    CVC_OPS_NB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_hf_2mic_nb, cvc_send_role)
};

static const operator_config_t ops_wb_2mic_hs[] =
{
    CVC_OPS_WB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_hs_2mic_90deg_wb, cvc_send_role)
};

static const operator_config_t ops_wb_2mic_speaker[] =
{
    CVC_OPS_WB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_spk_2mic_0deg_wb, cvc_send_role)
};

static const operator_config_t ops_wb_2mic_handsfree[] =
{
    CVC_OPS_WB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_hf_2mic_wb, cvc_send_role)
};

static const operator_config_t ops_binaural_nb_2mic_hs[] =
{
    CVC_OPS_NB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_hs_2mic_0deg_nb, cvc_send_role)
};

static const operator_config_t ops_binaural_wb_2mic_hs[] =
{
    CVC_OPS_WB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_hs_2mic_0deg_wb, cvc_send_role)
};

static const operator_config_t ops_uwb_1mic_hs[] =
{
    CVC_OPS_UWB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_hs_1_mic_uwb, cvc_send_role)
};

static const operator_config_t ops_uwb_2mic_hs[] =
{
    CVC_OPS_UWB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_hs_2_mic_uwb, cvc_send_role)
};

static const operator_config_t ops_uwb_2mic_binaural_hs[] =
{
    CVC_OPS_UWB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_hs_2_mic_binaural_uwb, cvc_send_role)
};

static const operator_config_t ops_uwb_1mic_hf[] =
{
    CVC_OPS_UWB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_hf_1_mic_uwb, cvc_send_role)
};

static const operator_config_t ops_uwb_2mic_hf[] =
{
    CVC_OPS_UWB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_hf_2_mic_uwb, cvc_send_role)
};

static const operator_config_t ops_uwb_1mic_speaker[] =
{
    CVC_OPS_UWB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_spk_1_mic_uwb, cvc_send_role)
};

static const operator_config_t ops_uwb_2mic_speaker[] =
{
    CVC_OPS_UWB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_spk_2_mic_uwb, cvc_send_role)
};

static const operator_config_t ops_swb_1mic_hs[] =
{
    CVC_OPS_SWB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_hs_1_mic_swb, cvc_send_role)
};

static const operator_config_t ops_swb_2mic_hs[] =
{
    CVC_OPS_SWB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_hs_2_mic_swb, cvc_send_role)
};

static const operator_config_t ops_swb_2mic_binaural_hs[] =
{
    CVC_OPS_SWB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_hs_2_mic_binaural_swb, cvc_send_role)
};

static const operator_config_t ops_swb_1mic_hf[] =
{
    CVC_OPS_SWB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_hf_1_mic_swb, cvc_send_role)
};

static const operator_config_t ops_swb_2mic_hf[] =
{
    CVC_OPS_SWB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_hf_2_mic_swb, cvc_send_role)
};

static const operator_config_t ops_swb_1mic_speaker[] =
{
    CVC_OPS_SWB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_spk_1_mic_swb, cvc_send_role)
};

static const operator_config_t ops_swb_2mic_speaker[] =
{
    CVC_OPS_SWB,
    MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(capability_id_cvc_spk_2_mic_swb, cvc_send_role)
};

static const operator_config_t ops_nb_no_cvc[] =
{
    MAKE_OPERATOR_CONFIG_PRIORITY_MEDIUM(capability_id_none, receive_role),
    MAKE_OPERATOR_CONFIG_PRIORITY_HIGH(capability_id_none, send_role)
};

static const operator_config_t ops_wb_no_cvc[] =
{
    MAKE_OPERATOR_CONFIG_PRIORITY_MEDIUM(capability_id_none, receive_role),
    MAKE_OPERATOR_CONFIG_PRIORITY_HIGH(capability_id_none, send_role)
};

static const operator_config_t ops_uwb_no_cvc[] =
{
    MAKE_OPERATOR_CONFIG_PRIORITY_MEDIUM(capability_id_none, receive_role),
    MAKE_OPERATOR_CONFIG_PRIORITY_HIGH(capability_id_none, send_role)
};

static const operator_config_t ops_swb_no_cvc[] =
{
    MAKE_OPERATOR_CONFIG_PRIORITY_MEDIUM(capability_id_none, receive_role),
    MAKE_OPERATOR_CONFIG_PRIORITY_HIGH(capability_id_none, send_role)
};

static const operator_setup_item_t sample_rate_ra_nb[] =
{
    OPERATORS_SETUP_STANDARD_SAMPLE_RATE(NBS_SAMPLE_RATE)
};

static const operator_setup_item_t sample_rate_ra_wb[] =
{
    OPERATORS_SETUP_STANDARD_SAMPLE_RATE(WBS_SAMPLE_RATE)
};

static const operator_setup_item_t sample_rate_ra_uwb[] =
{
    OPERATORS_SETUP_STANDARD_SAMPLE_RATE(UWBS_SAMPLE_RATE)
};

static const operator_setup_item_t sample_rate_ra_swb[] =
{
    OPERATORS_SETUP_STANDARD_SAMPLE_RATE(SWBS_SAMPLE_RATE)
};

static operator_config_t ra_nb[] =
{
    MAKE_OPERATOR_CONFIG_WITH_SETUP(capability_id_rate_adjustment, audio_voice_rate_adjustment_send_role, sample_rate_ra_nb)
};

static operator_config_t ra_wb[] =
{
    MAKE_OPERATOR_CONFIG_WITH_SETUP(capability_id_rate_adjustment, audio_voice_rate_adjustment_send_role, sample_rate_ra_wb)
};

static operator_config_t ra_uwb[] =
{
    MAKE_OPERATOR_CONFIG_WITH_SETUP(capability_id_rate_adjustment, audio_voice_rate_adjustment_send_role, sample_rate_ra_uwb)
};

static operator_config_t ra_swb[] =
{
    MAKE_OPERATOR_CONFIG_WITH_SETUP(capability_id_rate_adjustment, audio_voice_rate_adjustment_send_role, sample_rate_ra_swb)
};

/* --------------------------------------------------------------*/

static const operator_path_node_t receive[] =
{
    {receive_role, 0, 0},
    {cvc_receive_role, 0, 0},
};

static const operator_path_node_t send_mic1[] =
{
    {cvc_send_role, 1, 0},
    {rate_adjustment_send_role, 0, 0},
    {send_role, 0, 0}
};

static const operator_path_node_t send_mic2[] =
{
    {cvc_send_role, 2, UNCONNECTED},
};

static const operator_path_node_t aec_ref[] =
{
    {cvc_send_role, 0, UNCONNECTED}
};

/* --------------------------------------------------------------*/

static const operator_path_t paths[] =
{
    {path_receive, path_with_in_and_out, ARRAY_DIM((receive)), receive},
    {path_send_mic1, path_with_in_and_out, ARRAY_DIM((send_mic1)), send_mic1},
    {path_send_mic2, path_with_input, ARRAY_DIM((send_mic2)), send_mic2},
    {path_aec_ref, path_with_input, ARRAY_DIM((aec_ref)), aec_ref}
};

/*Headset varaints*/
static const chain_config_t audio_voice_hfp_config_1mic_nb_hs =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_cvc_headset, ops_nb_1mic_hs, paths);

static const chain_config_t audio_voice_hfp_config_1mic_wb_hs =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_cvc_headset, ops_wb_1mic_hs, paths);

static const chain_config_t audio_voice_hfp_config_2mic_nb_hs =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_cvc_headset, ops_nb_2mic_hs, paths);

static const chain_config_t audio_voice_hfp_config_2mic_wb_hs =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_cvc_headset, ops_wb_2mic_hs, paths);

static const chain_config_t audio_voice_hfp_config_2mic_binaural_nb_hs =
        MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_cvc_headset, ops_binaural_nb_2mic_hs, paths);

static const chain_config_t audio_voice_hfp_config_2mic_binaural_wb_hs =
        MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_cvc_headset, ops_binaural_wb_2mic_hs, paths);

/*Handsfree varaints*/
static const chain_config_t audio_voice_hfp_config_1mic_nb_handsfree =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_cvc_handsfree, ops_nb_1mic_handsfree, paths);

static const chain_config_t audio_voice_hfp_config_1mic_wb_handsfree =
        MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_cvc_handsfree, ops_wb_1mic_handsfree, paths);

static const chain_config_t audio_voice_hfp_config_2mic_nb_handsfree =
        MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_cvc_handsfree, ops_nb_2mic_handsfree, paths);

static const chain_config_t audio_voice_hfp_config_2mic_wb_handsfree =
        MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_cvc_handsfree, ops_wb_2mic_handsfree, paths);

/*Speaker varaints*/
static const chain_config_t audio_voice_hfp_config_1mic_nb_speaker =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_cvc_speaker, ops_nb_1mic_speaker, paths);

static const chain_config_t audio_voice_hfp_config_1mic_wb_speaker =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_cvc_speaker, ops_wb_1mic_speaker, paths);

static const chain_config_t audio_voice_hfp_config_2mic_nb_speaker =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_cvc_speaker, ops_nb_2mic_speaker, paths);

static const chain_config_t audio_voice_hfp_config_2mic_wb_speaker =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_cvc_speaker, ops_wb_2mic_speaker, paths);


static const chain_config_t audio_voice_hfp_config_1mic_uwb_hs =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_swb_and_uwb, ops_uwb_1mic_hs, paths);

static const chain_config_t audio_voice_hfp_config_2mic_uwb_hs =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_swb_and_uwb, ops_uwb_2mic_hs, paths);

static const chain_config_t audio_voice_hfp_config_2mic_binaural_uwb_hs =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_swb_and_uwb, ops_uwb_2mic_binaural_hs, paths);

static const chain_config_t audio_voice_hfp_config_1mic_uwb_handsfree =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_swb_and_uwb, ops_uwb_1mic_hf, paths);

static const chain_config_t audio_voice_hfp_config_2mic_uwb_handsfree =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_swb_and_uwb, ops_uwb_2mic_hf, paths);

static const chain_config_t audio_voice_hfp_config_1mic_uwb_speaker =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_swb_and_uwb, ops_uwb_1mic_speaker, paths);

static const chain_config_t audio_voice_hfp_config_2mic_uwb_speaker =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_swb_and_uwb, ops_uwb_2mic_speaker, paths);

static const chain_config_t audio_voice_hfp_config_1mic_swb_hs =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_swb_and_uwb, ops_swb_1mic_hs, paths);

static const chain_config_t audio_voice_hfp_config_2mic_swb_hs =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_swb_and_uwb, ops_swb_2mic_hs, paths);

static const chain_config_t audio_voice_hfp_config_2mic_binaural_swb_hs =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_swb_and_uwb, ops_swb_2mic_binaural_hs, paths);

static const chain_config_t audio_voice_hfp_config_1mic_swb_handsfree =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_swb_and_uwb, ops_swb_1mic_hf, paths);

static const chain_config_t audio_voice_hfp_config_2mic_swb_handsfree =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_swb_and_uwb, ops_swb_2mic_hf, paths);

static const chain_config_t audio_voice_hfp_config_1mic_swb_speaker =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_swb_and_uwb, ops_swb_1mic_speaker, paths);

static const chain_config_t audio_voice_hfp_config_2mic_swb_speaker =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_swb_and_uwb, ops_swb_2mic_speaker, paths);


/* --------------------------------------------------------------*/

static const operator_path_node_t receive_no_cvc[] =
{
    {receive_role, 0, 0},
};

static const operator_path_node_t send_mic1_no_cvc[] =
{
    {send_role, 0, 0}
};

static const operator_path_t paths_no_cvc[] =
{
    {path_receive, path_with_in_and_out, ARRAY_DIM((receive_no_cvc)), receive_no_cvc},
    {path_send_mic1, path_with_in_and_out, ARRAY_DIM((send_mic1_no_cvc)), send_mic1_no_cvc},
};

static const chain_config_t audio_voice_hfp_config_nb_no_cvc =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp, ops_nb_no_cvc, paths_no_cvc);

static const chain_config_t audio_voice_hfp_config_wb_no_cvc =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp, ops_wb_no_cvc, paths_no_cvc);

static const chain_config_t audio_voice_hfp_config_uwb_no_cvc =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_swb_and_uwb, ops_uwb_no_cvc, paths_no_cvc);

static const chain_config_t audio_voice_hfp_config_swb_no_cvc =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_cvc_common, audio_ucid_hfp_swb_and_uwb, ops_swb_no_cvc, paths_no_cvc);


static const chain_config_t * getNoCvcChainConfig(const audio_voice_context_t* ctx)
{
    const chain_config_t * config = NULL;
    switch(AudioVoiceCommonGetPluginType(ctx->variant))
    {
        case narrow_band:
            config = &audio_voice_hfp_config_nb_no_cvc;
            break;
        case wide_band:
            config = &audio_voice_hfp_config_wb_no_cvc;
            break;
        case ultra_wide_band:
            config = &audio_voice_hfp_config_uwb_no_cvc;
            break;
        case super_wide_band:
            config = &audio_voice_hfp_config_swb_no_cvc;
            break;
        default:
            config = &audio_voice_hfp_config_nb_no_cvc;
            break;
    }
    return config;
}

/******************************************************************************/
const chain_config_t* AudioVoiceCommonGetChainConfig(const audio_voice_context_t* ctx)
{
    const chain_config_t * config = NULL;

    if(AudioConfigGetQuality(audio_stream_voice) == audio_quality_low_power)
    {
        return getNoCvcChainConfig(ctx);
    }

    switch(ctx->variant)
    {
        case cvc_1_mic_headset_cvsd:
            config = &audio_voice_hfp_config_1mic_nb_hs;
            break;
        case cvc_1_mic_headset_msbc:
            config = &audio_voice_hfp_config_1mic_wb_hs;
            break;
        case cvc_2_mic_headset_cvsd:
            config = &audio_voice_hfp_config_2mic_nb_hs;
            break;
        case cvc_2_mic_headset_msbc:
            config = &audio_voice_hfp_config_2mic_wb_hs;
            break;
        case cvc_2_mic_headset_binaural_nb:
            config = &audio_voice_hfp_config_2mic_binaural_nb_hs;
            break;
        case cvc_2_mic_headset_binaural_wb:
            config = &audio_voice_hfp_config_2mic_binaural_wb_hs;
            break;
        case cvc_1_mic_handsfree_cvsd:
            config = &audio_voice_hfp_config_1mic_nb_handsfree;
            break;
        case cvc_1_mic_handsfree_msbc:
            config = &audio_voice_hfp_config_1mic_wb_handsfree;
            break;
        case cvc_2_mic_handsfree_cvsd:
            config = &audio_voice_hfp_config_2mic_nb_handsfree;
            break;
        case cvc_2_mic_handsfree_msbc:
            config = &audio_voice_hfp_config_2mic_wb_handsfree;
            break;
        case cvc_1_mic_speaker_cvsd:
            config = &audio_voice_hfp_config_1mic_nb_speaker;
            break;
        case cvc_1_mic_speaker_msbc:
            config = &audio_voice_hfp_config_1mic_wb_speaker;
            break;
        case cvc_2_mic_speaker_cvsd:
            config = &audio_voice_hfp_config_2mic_nb_speaker;
            break;
        case cvc_2_mic_speaker_msbc:
            config = &audio_voice_hfp_config_2mic_wb_speaker;
            break;
        case cvc_1_mic_headset_uwb:
            config = &audio_voice_hfp_config_1mic_uwb_hs;
            break;
        case cvc_2_mic_headset_uwb:
            config = &audio_voice_hfp_config_2mic_uwb_hs;
            break;
        case cvc_2_mic_headset_binaural_uwb:
            config = &audio_voice_hfp_config_2mic_binaural_uwb_hs;
            break;
        case cvc_1_mic_handsfree_uwb:
            config = &audio_voice_hfp_config_1mic_uwb_handsfree;
            break;
        case cvc_2_mic_handsfree_uwb:
            config = &audio_voice_hfp_config_2mic_uwb_handsfree;
            break;
        case cvc_1_mic_speaker_uwb:
            config = &audio_voice_hfp_config_1mic_uwb_speaker;
            break;
        case cvc_2_mic_speaker_uwb:
            config = &audio_voice_hfp_config_2mic_uwb_speaker;
            break;
        case cvc_1_mic_headset_swb:
            config = &audio_voice_hfp_config_1mic_swb_hs;
            break;
        case cvc_2_mic_headset_swb:
            config = &audio_voice_hfp_config_2mic_swb_hs;
            break;
        case cvc_2_mic_headset_binaural_swb:
            config = &audio_voice_hfp_config_2mic_binaural_swb_hs;
            break;
        case cvc_1_mic_handsfree_swb:
            config = &audio_voice_hfp_config_1mic_swb_handsfree;
            break;
        case cvc_2_mic_handsfree_swb:
            config = &audio_voice_hfp_config_2mic_swb_handsfree;
            break;
        case cvc_1_mic_speaker_swb:
            config = &audio_voice_hfp_config_1mic_swb_speaker;
            break;
        case cvc_2_mic_speaker_swb:
            config = &audio_voice_hfp_config_2mic_swb_speaker;
            break;

        default:
            return NULL;
    }
    return config;
}

operator_config_t* AudioVoiceCommonGetRAFilter(const audio_voice_context_t* ctx)
{
    operator_config_t * config = ra_nb;
    switch(AudioVoiceCommonGetPluginType(ctx->variant))
    {
        case narrow_band:
            config = ra_nb;
            break;
        case wide_band:
            config = ra_wb;
            break;
        case ultra_wide_band:
            config = ra_uwb;
            break;
        case super_wide_band:
            config = ra_swb;
            break;
        default:
            config = ra_nb;
            break;
    }
    return config;
}

bool AudioVoiceCommonIsWideband(cvc_plugin_type_t cvc_plugin_type)
{
    bool is_wideband = FALSE;
    switch(cvc_plugin_type)
    {
        case cvc_2_mic_headset_msbc:
        case cvc_2_mic_handsfree_msbc:
        case cvc_2_mic_headset_binaural_wb:
        case cvc_2_mic_speaker_msbc:
            is_wideband = TRUE;
            break;
        default:
            is_wideband = FALSE;
            break;
    }
    return is_wideband;
}

bool AudioVoiceCommonIs2Mic(cvc_plugin_type_t cvc_plugin_type)
{
    bool is_2_mic = FALSE;
    switch(cvc_plugin_type)
    {
        case cvc_2_mic_headset_cvsd:
        case cvc_2_mic_headset_cvsd_bex:
        case cvc_2_mic_handsfree_cvsd:
        case cvc_2_mic_handsfree_cvsd_bex:
        case cvc_2_mic_headset_cvsd_asr:
        case cvc_2_mic_handsfree_cvsd_asr:
        case cvc_2_mic_headset_binaural_nb:
        case cvc_2_mic_speaker_cvsd:
        case cvc_2_mic_headset_msbc:
        case cvc_2_mic_handsfree_msbc:
        case cvc_2_mic_headset_binaural_wb:
        case cvc_2_mic_speaker_msbc:
        case cvc_2_mic_headset_uwb:
        case cvc_2_mic_headset_binaural_uwb:
        case cvc_2_mic_handsfree_uwb:
        case cvc_2_mic_speaker_uwb:
        case cvc_2_mic_headset_swb:
        case cvc_2_mic_headset_binaural_swb:
        case cvc_2_mic_handsfree_swb:
        case cvc_2_mic_speaker_swb:
            is_2_mic = TRUE;
            break;
        default:
            is_2_mic = FALSE;
            break;
    }
    return is_2_mic;
}

plugin_type_t AudioVoiceCommonGetPluginType(cvc_plugin_type_t cvc_plugin_type)
{
    plugin_type_t type = narrow_band;
    switch(cvc_plugin_type)
    {
        case cvc_1_mic_headset_cvsd:
        case cvc_1_mic_headset_cvsd_bex:
        case cvc_2_mic_headset_cvsd:
        case cvc_2_mic_headset_cvsd_bex:
        case cvc_1_mic_handsfree_cvsd:
        case cvc_1_mic_handsfree_cvsd_bex:
        case cvc_2_mic_handsfree_cvsd:
        case cvc_2_mic_handsfree_cvsd_bex:
        case cvc_1_mic_headset_cvsd_asr:
        case cvc_2_mic_headset_cvsd_asr:
        case cvc_1_mic_handsfree_cvsd_asr:
        case cvc_2_mic_handsfree_cvsd_asr:
        case cvc_2_mic_headset_binaural_nb:
        case cvc_1_mic_speaker_cvsd:
        case cvc_2_mic_speaker_cvsd:
            type = narrow_band;
            break;
        case cvc_1_mic_headset_msbc:
        case cvc_2_mic_headset_msbc:
        case cvc_1_mic_handsfree_msbc:
        case cvc_2_mic_handsfree_msbc:
        case cvc_2_mic_headset_binaural_wb:
        case cvc_1_mic_speaker_msbc:
        case cvc_2_mic_speaker_msbc:
            type = wide_band;
            break;
        case cvc_1_mic_headset_uwb:
        case cvc_2_mic_headset_uwb:
        case cvc_2_mic_headset_binaural_uwb:
        case cvc_1_mic_handsfree_uwb:
        case cvc_2_mic_handsfree_uwb:
        case cvc_1_mic_speaker_uwb:
        case cvc_2_mic_speaker_uwb:
            type = ultra_wide_band;
            break;
        case cvc_1_mic_headset_swb:
        case cvc_2_mic_headset_swb:
        case cvc_2_mic_headset_binaural_swb:
        case cvc_1_mic_handsfree_swb:
        case cvc_2_mic_handsfree_swb:
        case cvc_1_mic_speaker_swb:
        case cvc_2_mic_speaker_swb:
            type = super_wide_band;
            break;
        default:
            type = narrow_band;
            break;
    }
    return type;
}
