/****************************************************************************
Copyright (c) 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_voice_assistant_common_chain_config_novtt.c

DESCRIPTION
    Handles creation and control of the common chain of the
    VA graph - no VTT case
*/

#include "audio_voice_assistant_common_chain.h"
#include "audio_voice_assistant_common_chain_config_novtt.h"

#define UNUSED_TERMINAL (0xff)

static const operator_config_t ops_va_common_novtt[] =
{
    MAKE_OPERATOR_CONFIG(capability_id_none, va_common_cvc_role),
};

static const operator_path_node_t va_path_node_mic1_novtt[] =
{
    {va_common_cvc_role, 1, UNUSED_TERMINAL}
};

static const operator_path_node_t va_path_node_mic2_novtt[] =
{
    {va_common_cvc_role, 2, UNUSED_TERMINAL}
};

static const operator_path_node_t va_path_node_aec_ref_novtt[] =
{
    {va_common_cvc_role, 0, UNUSED_TERMINAL}
};

static const operator_path_node_t va_path_node_capture1_novtt[] =
{
    {va_common_cvc_role, UNUSED_TERMINAL, 0}
};

/* --------------------------------------------------------------*/
static const operator_path_t va_1_mic_paths_novtt[] =
{
    {va_common_mic1_path, path_with_input, ARRAY_DIM((va_path_node_mic1_novtt)), va_path_node_mic1_novtt},
    {va_common_aec_ref_path, path_with_input, ARRAY_DIM((va_path_node_aec_ref_novtt)), va_path_node_aec_ref_novtt},
    {va_common_capture1_path, path_with_output, ARRAY_DIM((va_path_node_capture1_novtt)), va_path_node_capture1_novtt},
};

static const operator_path_t va_2_mic_paths_novtt[] =
{
    {va_common_mic1_path, path_with_input, ARRAY_DIM((va_path_node_mic1_novtt)), va_path_node_mic1_novtt},
    {va_common_mic2_path, path_with_input, ARRAY_DIM((va_path_node_mic2_novtt)), va_path_node_mic2_novtt},
    {va_common_aec_ref_path, path_with_input, ARRAY_DIM((va_path_node_aec_ref_novtt)), va_path_node_aec_ref_novtt},
    {va_common_capture1_path, path_with_output, ARRAY_DIM((va_path_node_capture1_novtt)), va_path_node_capture1_novtt},
};

static const chain_config_t va_common_chain_config_cvc_1mic_novtt =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_va, audio_ucid_va, ops_va_common_novtt, va_1_mic_paths_novtt);

static const chain_config_t va_common_chain_config_cvc_2mic_novtt =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_va, audio_ucid_va, ops_va_common_novtt, va_2_mic_paths_novtt);

/****************************************************************************
DESCRIPTION
    Retrieve the operator for a given role.
*/
const const operator_config_t* getOperatorConfigBasedOnRoleNoVtt(unsigned role)
{
    unsigned index;

    for(index = 0; index < ARRAY_DIM(ops_va_common_novtt); ++index)
    {
        if(ops_va_common_novtt[index].role == role)
            return &ops_va_common_novtt[index];
    }

    Panic();
    return NULL;    
}

/****************************************************************************
DESCRIPTION
    Retrieve a chain configuration that's selected by cVc variant.
*/
const chain_config_t* getChainConfigNoVtt(plugin_variant_t variant)
{
    switch(variant)
    {
        case plugin_cvc_hs_1mic:
        case plugin_cvc_spkr_1mic:
            return &va_common_chain_config_cvc_1mic_novtt;

        case plugin_cvc_hs_2mic:
        case plugin_cvc_hs_2mic_binaural:
        case plugin_cvc_spkr_2mic:
            return &va_common_chain_config_cvc_2mic_novtt;

        case plugin_cvc_spkr_3mic_farfield: /* Not supported for TTT and PTT usage. */
        default:
            Panic();
            return NULL;
    }
}
