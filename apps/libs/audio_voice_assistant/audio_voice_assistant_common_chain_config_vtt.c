/****************************************************************************
Copyright (c) 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_voice_assistant_common_chain_config_vtt.c

DESCRIPTION
    Handles creation and control of the common chain of the
    VA graph - VTT case
*/

#include "audio_voice_assistant_common_chain.h"
#include "audio_voice_assistant_common_chain_config_vtt.h"

#define UNUSED_TERMINAL (0xff)

static const operator_config_t ops_va_common_vtt[] =
{
    MAKE_OPERATOR_CONFIG_PRIORITY_MEDIUM(capability_id_vad, va_common_vad_role),
    MAKE_OPERATOR_CONFIG(capability_id_none, va_common_cvc_role),
    MAKE_OPERATOR_CONFIG(capability_id_splitter, va_common_splitter_role)
};

static const operator_path_node_t va_path_node_mic1_vtt[] =
{
    {va_common_vad_role, 0, 0},
    {va_common_cvc_role, 1, UNUSED_TERMINAL}
};

static const operator_path_node_t va_path_node_mic2_vtt[] =
{
    {va_common_cvc_role, 2, UNUSED_TERMINAL}
};

static const operator_path_node_t va_path_node_mic3_vtt[] =
{
    {va_common_cvc_role, 3, UNUSED_TERMINAL}
};

static const operator_path_node_t va_path_node_aec_ref_vtt[] =
{
    {va_common_cvc_role, 0, UNUSED_TERMINAL}
};

static const operator_path_node_t va_path_node_trigger1_vtt[] =
{
    {va_common_cvc_role, UNUSED_TERMINAL, 0},
    {va_common_splitter_role, 0, 0}
};

static const operator_path_node_t va_path_node_trigger2_vtt[] =
{
    {va_common_cvc_role, UNUSED_TERMINAL, 1},
    {va_common_splitter_role, 1, 2}
};

static const operator_path_node_t va_path_node_trigger3_vtt[] =
{
    {va_common_cvc_role, UNUSED_TERMINAL, 2},
    {va_common_splitter_role, 2, 4}
};

static const operator_path_node_t va_path_node_trigger4_vtt[] =
{
    {va_common_cvc_role, UNUSED_TERMINAL, 3},
    {va_common_splitter_role, 3, 6}
};

static const operator_path_node_t va_path_node_capture1_vtt[] =
{
    {va_common_splitter_role, UNUSED_TERMINAL, 1}
};

static const operator_path_node_t va_path_node_capture2_vtt[] =
{
    {va_common_splitter_role, UNUSED_TERMINAL, 3}
};

static const operator_path_node_t va_path_node_capture3_vtt[] =
{
    {va_common_splitter_role, UNUSED_TERMINAL, 5}
};

static const operator_path_node_t va_path_node_capture4_vtt[] =
{
    {va_common_splitter_role, UNUSED_TERMINAL, 7}
};

/* --------------------------------------------------------------*/
static const operator_path_t va_1_mic_paths_vtt[] =
{
    {va_common_mic1_path, path_with_input, ARRAY_DIM((va_path_node_mic1_vtt)), va_path_node_mic1_vtt},
    {va_common_aec_ref_path, path_with_input, ARRAY_DIM((va_path_node_aec_ref_vtt)), va_path_node_aec_ref_vtt},
    {va_common_trigger1_path, path_with_output, ARRAY_DIM((va_path_node_trigger1_vtt)), va_path_node_trigger1_vtt},
    {va_common_capture1_path, path_with_output, ARRAY_DIM((va_path_node_capture1_vtt)), va_path_node_capture1_vtt},
};

static const operator_path_t va_2_mic_paths_vtt[] =
{
    {va_common_mic1_path, path_with_input, ARRAY_DIM((va_path_node_mic1_vtt)), va_path_node_mic1_vtt},
    {va_common_mic2_path, path_with_input, ARRAY_DIM((va_path_node_mic2_vtt)), va_path_node_mic2_vtt},
    {va_common_aec_ref_path, path_with_input, ARRAY_DIM((va_path_node_aec_ref_vtt)), va_path_node_aec_ref_vtt},
    {va_common_trigger1_path, path_with_output, ARRAY_DIM((va_path_node_trigger1_vtt)), va_path_node_trigger1_vtt},
    {va_common_capture1_path, path_with_output, ARRAY_DIM((va_path_node_capture1_vtt)), va_path_node_capture1_vtt},
};

static const operator_path_t va_3_mic_paths_vtt[] =
{
    {va_common_mic1_path, path_with_input, ARRAY_DIM((va_path_node_mic1_vtt)), va_path_node_mic1_vtt},
    {va_common_mic2_path, path_with_input, ARRAY_DIM((va_path_node_mic2_vtt)), va_path_node_mic2_vtt},
    {va_common_mic3_path, path_with_input, ARRAY_DIM((va_path_node_mic3_vtt)), va_path_node_mic3_vtt},
    {va_common_aec_ref_path, path_with_input, ARRAY_DIM((va_path_node_aec_ref_vtt)), va_path_node_aec_ref_vtt},
    {va_common_trigger1_path, path_with_output, ARRAY_DIM((va_path_node_trigger1_vtt)), va_path_node_trigger1_vtt},
    {va_common_trigger2_path, path_with_output, ARRAY_DIM((va_path_node_trigger2_vtt)), va_path_node_trigger2_vtt},
    {va_common_trigger3_path, path_with_output, ARRAY_DIM((va_path_node_trigger3_vtt)), va_path_node_trigger3_vtt},
    {va_common_trigger4_path, path_with_output, ARRAY_DIM((va_path_node_trigger4_vtt)), va_path_node_trigger4_vtt},
    {va_common_capture1_path, path_with_output, ARRAY_DIM((va_path_node_capture1_vtt)), va_path_node_capture1_vtt},
    {va_common_capture2_path, path_with_output, ARRAY_DIM((va_path_node_capture2_vtt)), va_path_node_capture2_vtt},
    {va_common_capture3_path, path_with_output, ARRAY_DIM((va_path_node_capture3_vtt)), va_path_node_capture3_vtt},
    {va_common_capture4_path, path_with_output, ARRAY_DIM((va_path_node_capture4_vtt)), va_path_node_capture4_vtt},
};

static const chain_config_t va_common_chain_config_cvc_1mic_vtt =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_va, audio_ucid_va, ops_va_common_vtt, va_1_mic_paths_vtt);

static const chain_config_t va_common_chain_config_cvc_2mic_vtt =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_va, audio_ucid_va, ops_va_common_vtt, va_2_mic_paths_vtt);

static const chain_config_t va_common_chain_config_cvc_3mic_vtt =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_va, audio_ucid_va, ops_va_common_vtt, va_3_mic_paths_vtt);

/****************************************************************************
DESCRIPTION
    Retrieve the operator for a given role.
*/
const const operator_config_t* getOperatorConfigBasedOnRoleVtt(unsigned role)
{
    unsigned index;

    for(index = 0; index < ARRAY_DIM(ops_va_common_vtt); ++index)
    {
        if(ops_va_common_vtt[index].role == role)
            return &ops_va_common_vtt[index];
    }

    Panic();
    return NULL;    
}

/****************************************************************************
DESCRIPTION
    Retrieve a chain configuration that's selected by cVc variant.
*/
const chain_config_t* getChainConfigVtt(plugin_variant_t variant)
{
    switch(variant)
    {
        case plugin_cvc_hs_1mic:
        case plugin_cvc_spkr_1mic:
            return &va_common_chain_config_cvc_1mic_vtt;

        case plugin_cvc_hs_2mic:
        case plugin_cvc_hs_2mic_binaural:
        case plugin_cvc_spkr_2mic:
            return &va_common_chain_config_cvc_2mic_vtt;

        case plugin_cvc_spkr_3mic_farfield:
            return &va_common_chain_config_cvc_3mic_vtt;

        default:
            Panic();
            return NULL;
    }
}
