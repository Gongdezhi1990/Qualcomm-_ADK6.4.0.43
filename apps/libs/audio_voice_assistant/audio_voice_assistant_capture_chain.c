/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_voice_assistant_capture_chain.c

DESCRIPTION
    Handles creation and control of the capture chain of the
    VA graph
*/

#include <operators.h>
#include <panic.h>
#include <audio_config.h>
#include "audio_voice_assistant_capture_chain.h"
#include "audio_voice_assistant_private.h"

#define UNUSED_TERMINAL       (0xff)

static kymera_chain_handle_t va_capture_chain = NULL;

static const operator_config_t va_capture_op_config[] =
{
#ifdef INCLUDE_SPC_OP_IN_CAPTURE_CHAIN
    MAKE_OPERATOR_CONFIG(capability_id_switched_passthrough_consumer, va_common_switched_passthrough_role),
#else
    MAKE_OPERATOR_CONFIG(capability_id_none, va_common_switched_passthrough_role),
#endif
    MAKE_OPERATOR_CONFIG(capability_id_sbc_encoder, va_capture_encoder_role)
};

static const operator_path_node_t va_path_node_capture_input_0[] =
{
    {va_common_switched_passthrough_role, 0, UNUSED_TERMINAL}
};

static const operator_path_node_t va_path_node_capture_input_1[] =
{
    {va_common_switched_passthrough_role, 1, UNUSED_TERMINAL}
};

static const operator_path_node_t va_path_node_capture_input_2[] =
{
    {va_common_switched_passthrough_role, 2, UNUSED_TERMINAL}
};

static const operator_path_node_t va_path_node_capture_input_3[] =
{
    {va_common_switched_passthrough_role, 3, UNUSED_TERMINAL}
};

#ifdef INCLUDE_SPC_OP_IN_CAPTURE_CHAIN
static const operator_path_node_t va_path_node_capture_output[] =
{
    {va_common_switched_passthrough_role, UNUSED_TERMINAL, 0},
    {va_capture_encoder_role, 0, 0}
};

static const operator_path_t single_channel_path[] =
{
    {va_capture_input_0, path_with_input, ARRAY_DIM((va_path_node_capture_input_0)), va_path_node_capture_input_0},
    {va_capture_output, path_with_output, ARRAY_DIM((va_path_node_capture_output)), va_path_node_capture_output},
};
#else

static const operator_path_node_t va_path_node_capture_input[] =
{
    {va_capture_encoder_role, 0, UNUSED_TERMINAL}
};

static const operator_path_node_t va_path_node_capture_output[] =
{
    {va_capture_encoder_role, UNUSED_TERMINAL, 0}
};
static const operator_path_t single_channel_path[] =
{
    {va_capture_input_0, path_with_input, ARRAY_DIM((va_path_node_capture_input)), va_path_node_capture_input},
    {va_capture_output, path_with_output, ARRAY_DIM((va_path_node_capture_output)), va_path_node_capture_output},
};
#endif

static const operator_path_t multi_channel_paths[] =
{
    {va_capture_input_0, path_with_input, ARRAY_DIM((va_path_node_capture_input_0)), va_path_node_capture_input_0},
    {va_capture_input_1, path_with_input, ARRAY_DIM((va_path_node_capture_input_1)), va_path_node_capture_input_1},
    {va_capture_input_2, path_with_input, ARRAY_DIM((va_path_node_capture_input_2)), va_path_node_capture_input_2},
    {va_capture_input_3, path_with_input, ARRAY_DIM((va_path_node_capture_input_3)), va_path_node_capture_input_3},
    {va_capture_output, path_with_output, ARRAY_DIM((va_path_node_capture_output)), va_path_node_capture_output},
};

static const chain_config_t va_capture_chain_single_channel_config =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_va, audio_ucid_va, va_capture_op_config, single_channel_path);

static const chain_config_t va_capture_chain_multi_channel_config =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_va, audio_ucid_va, va_capture_op_config, multi_channel_paths);


/****************************************************************************
DESCRIPTION
    Get the operator config based on matching role
*/
static const operator_config_t * getOperatorConfigBasedOnRole(unsigned role)
{
    unsigned i;

    for(i = 0; i < ARRAY_DIM(va_capture_op_config); ++i)
    {
        if(va_capture_op_config[i].role == role)
            return &va_capture_op_config[i];
    }

    Panic();
    return NULL;
}

/****************************************************************************
DESCRIPTION
    Get the operator filter config with overloaded capability Id.
*/
static operator_config_t getChainConfigFilter(const operator_config_t *base_config, capability_id_t cap_id)
{
    operator_config_t filter;

    /* Copy all fields form base configuration then replace capability ID */
    memcpy(&filter, base_config, sizeof(filter));
    filter.capability_id = cap_id;

    return filter;
}

/****************************************************************************
DESCRIPTION
    Get the capability Id based on CVC plugin type.
*/
static capability_id_t getEncoderCapabilityId(va_capture_codec_t capture_codec)
{
    switch(capture_codec)
    {
        case va_capture_codec_sbc:
            return capability_id_sbc_encoder;

        case va_capture_codec_msbc:
            return capability_id_async_wbs_encoder;

        case va_capture_codec_celt:
        default:
            Panic();
            break;
    }
    return capability_id_none;
}

/****************************************************************************
DESCRIPTION
    Get the operator filters config
*/
static operator_filters_t* getFilterConfig(va_capture_codec_t capture_codec)
{
    operator_filters_t* filters = (operator_filters_t*)calloc(1,sizeof(operator_filters_t));
    operator_config_t* filter_config = (operator_config_t*)calloc(1,sizeof(operator_config_t));

    *filter_config = getChainConfigFilter(getOperatorConfigBasedOnRole(va_capture_encoder_role), getEncoderCapabilityId(capture_codec));

    filters->num_operator_filters = 1;
    filters->operator_filters = filter_config;

    return filters;
}

/* Free all memory associated with the filter */
static void freeFilters(operator_filters_t* filter_config)
{
    operator_config_t* op_config = (operator_config_t*)filter_config->operator_filters;

    free(op_config);
    free(filter_config);
    filter_config = NULL;
}

/****************************************************************************
DESCRIPTION
    Configure Switched passthrough consumer capability
*/
static void configureSpc(void)
{
    Operator op_spc = ChainGetOperatorByRole(va_capture_chain, va_common_switched_passthrough_role);

    if(op_spc)
    {
        OperatorsSetSwitchedPassthruEncoding(op_spc, spc_op_format_pcm);
        OperatorsSetSwitchedPassthruMode(op_spc, spc_op_mode_passthrough);
        /* SPC input 1 is selected by default so that 1 or 2-Mic cVC usecases will work */
        OperatorsSpcSelectPassthroughInput(op_spc, spc_op_select_passthrough_input_1);
    }
}

/****************************************************************************
DESCRIPTION
    Configure encoder capability
*/
static void configureEncoder(va_capture_codec_t codec)
{
    Operator op_encoder = ChainGetOperatorByRole(va_capture_chain, va_capture_encoder_role);
    sbc_encoder_params_t encoder_params;
    uint16 bitpool_val;

    /* Configure encoder for voice capture */
    if(op_encoder)
    {
        switch(codec)
        {
            case va_capture_codec_sbc:
                encoder_params = AudioConfigGetSbcEncoderParams();
                OperatorsSbcEncoderSetEncodingParams(op_encoder, &encoder_params);
                break;

            case va_capture_codec_msbc:
                bitpool_val = AudioConfigGetMsbcEncoderBitpool();
                OperatorsMsbcEncoderSetBitpool(op_encoder, bitpool_val);
                break;

            case va_capture_codec_celt:
            default:
                Panic();
                break;
        }
    }
}

/****************************************************************************
DESCRIPTION
    Configure the capture chain.
*/
static void configureChain(va_capture_codec_t codec)
{
    configureSpc();
    configureEncoder(codec);
}

/****************************************************************************
DESCRIPTION
    Get the chain config for capture chain
*/
static const chain_config_t* getChainConfig(void)
{
    return audioVaIsFFVEnabled()? &va_capture_chain_multi_channel_config: &va_capture_chain_single_channel_config;
}

/****************************************************************************
DESCRIPTION
    Creates and connects the capture chain
*/
void audioVaCreateCaptureChain(void)
{
    va_capture_codec_t codec = AudioConfigGetVaCaptureCodec();
    operator_filters_t* filter_config = getFilterConfig(codec);

    PanicFalse(va_capture_chain == NULL);
    va_capture_chain = PanicNull(ChainCreateWithFilter(getChainConfig(), filter_config));
    freeFilters(filter_config);
    configureChain(codec);
    ChainConnect(va_capture_chain);
}

/****************************************************************************
DESCRIPTION
    Starts the capture chain
*/
void audioVaStartCaptureChain(void)
{
    ChainStart(va_capture_chain);
}

/****************************************************************************
DESCRIPTION
    Stop the capture chain
*/
void audioVaStopCaptureChain(void)
{
    if(va_capture_chain)
        ChainStop(va_capture_chain);
}

/****************************************************************************
DESCRIPTION
    Destroy the capture chain
*/
void audioVaDestroyCaptureChain(void)
{
    if(va_capture_chain)
    {
        ChainDestroy(va_capture_chain);
        va_capture_chain = NULL;
    }
}

/****************************************************************************
DESCRIPTION
    Get input to the capture chain
*/
Sink audioVaGetCaptureChainInput(uint16 input_num)
{
    return ChainGetInput(va_capture_chain, (va_capture_input_base + input_num));
}

/****************************************************************************
DESCRIPTION
    Get output to capture chain
*/
Source audioVaGetCaptureChainOutput(void)
{
    return ChainGetOutput(va_capture_chain, va_capture_output);
}

/****************************************************************************
DESCRIPTION
    Select capture chain Switched passthrough input
    Range: 0 to 3
*/
void audioVaSelectCaptureChainSPCInput(spc_select_passthrough_input_t input)
{
    Operator op_spc = ChainGetOperatorByRole(va_capture_chain, va_common_switched_passthrough_role);

    if(op_spc)
    {

        OperatorsSpcSelectPassthroughInput(op_spc, input);
        OperatorsSetSwitchedPassthruMode(op_spc, spc_op_mode_passthrough);
    }
}
