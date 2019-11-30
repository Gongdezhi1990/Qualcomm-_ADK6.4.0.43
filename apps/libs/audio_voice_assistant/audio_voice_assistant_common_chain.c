/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_voice_assistant_common_chain.c

DESCRIPTION
    Handles creation and control of the common chain of the
    VA graph
*/
#include <stdlib.h>
#include <vmtypes.h>
#include <panic.h>
#include <audio_plugin_ucid.h>

#include "audio_voice_assistant_common_chain.h"
#include "audio_voice_assistant_private.h"

#include "audio_voice_assistant_common_chain_config_novtt.h"
#include "audio_voice_assistant_common_chain_config_vtt.h"

#define VA_SPLITTER_BUFFER_SIZE 32000 /*TODO: Tune this */

static kymera_chain_handle_t va_common_chain = NULL;

/****************************************************************************
DESCRIPTION
    Get the chain config for a given cVc variant
*/
static const chain_config_t* getChainConfig(plugin_variant_t variant)
{
    return (audioVaGetContext()->support_voice_trigger) 
                      ? getChainConfigVtt(variant) : getChainConfigNoVtt(variant);
}

/****************************************************************************
DESCRIPTION
    Get the operator config based on matching role
*/
static const operator_config_t * getOperatorConfigBasedOnRole(unsigned role)
{
    return (audioVaGetContext()->support_voice_trigger) 
                      ? getOperatorConfigBasedOnRoleVtt(role) : getOperatorConfigBasedOnRoleNoVtt(role);

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
    Get the capability Id based on CVC plugin variant.
*/
static capability_id_t getCvcSendCapabilityId(plugin_variant_t variant)
{
    switch(variant)
    {
        case plugin_cvc_hs_1mic:
            return capability_id_cvc_hs_1mic_wb;

        case plugin_cvc_hs_2mic:
            return capability_id_cvc_hs_2mic_90deg_wb;

        case plugin_cvc_hs_2mic_binaural:
            return capability_id_cvc_hs_2mic_0deg_wb;

        case plugin_cvc_spkr_1mic:
            return capability_id_cvc_spk_1mic_wb;

        case plugin_cvc_spkr_2mic:
            return capability_id_cvc_spk_2mic_0deg_wb;

        case plugin_cvc_spkr_3mic_farfield:
            return capability_id_none; /* TODO: Update 3mic cvc cap id */

        default:
            Panic();
            return capability_id_none;
    }
}

/****************************************************************************
DESCRIPTION
    Get the operator filters config
*/
static operator_filters_t* getFilterConfig(plugin_variant_t variant)
{
    operator_filters_t* filters = (operator_filters_t*)calloc(1,sizeof(operator_filters_t));
    operator_config_t* filter_config = (operator_config_t*)calloc(1,sizeof(operator_config_t));

    *filter_config = getChainConfigFilter(getOperatorConfigBasedOnRole(va_common_cvc_role), getCvcSendCapabilityId(variant));

    filters->num_operator_filters = 1;
    filters->operator_filters = filter_config;

    return filters;
}

/****************************************************************************
DESCRIPTION
    Get n-mic cVc send capability usecase Id
*/
static ucid_cvc_send_t getNMicCvcSendUcid(bool cvc_omni_mode)
{
    return (cvc_omni_mode)?(ucid_cvc_send_va_farfield_omni_mode):(ucid_cvc_send_va_farfield);
}

/****************************************************************************
DESCRIPTION
    Configure VAD capability
*/
static void configureVad(void)
{
    Operator vad_operator = ChainGetOperatorByRole(va_common_chain, va_common_vad_role);

    if(vad_operator) /* Only for voice trigger (VTT). */
    {
        OperatorsStandardSetSampleRate(vad_operator, VA_AUDIO_SAMPLE_RATE);

    }
}

/****************************************************************************
DESCRIPTION
    Configure cVc send capability
*/
static void configureCvc(bool cvc_omni_mode)
{
    Operator op_cv = ChainGetOperatorByRole(va_common_chain, va_common_cvc_role);

    if(audioVaIsFFVEnabled())
    {
        OperatorsStandardSetUCID(op_cv, getNMicCvcSendUcid(cvc_omni_mode));
        if(cvc_omni_mode)
            OperatorsCvcSendEnableOmniMode(op_cv);
    }
    else
	{
        OperatorsStandardSetUCID(op_cv, ucid_cvc_send_va);
    }
}

/****************************************************************************
DESCRIPTION
    Configure splitter buffer capability
*/
static void configureSplitter(void)
{
    Operator op_splitter = ChainGetOperatorByRole(va_common_chain,va_common_splitter_role);

    if(op_splitter) /* Only for voice trigger (VTT). */
    {
        /*Configure Splitter */
        /* SET_WORKING_MODE(MSG ID: 0x6), 1-> Buffer data internally */
        OperatorsSplitterSetWorkingMode(op_splitter,splitter_mode_buffer_input);
        /* SET_LOCATION(MSG ID: 0x7), 1-> Buffer location SRAM */
        OperatorsSplitterSetBufferLocation(op_splitter,splitter_buffer_location_sram);
        /* SET_PACKING(MSG ID: 0x8), 1-> packed */
        OperatorsSplitterSetPacking(op_splitter,splitter_packing_packed);
        /* SET_DATA_FORMAT(MSG ID: 0x8),  1-> audio_with_metadata */
        OperatorsSplitterSetDataFormat(op_splitter,operator_data_format_pcm);
        /* SET_BUFFER_SIZE(MSG ID: 0x200C), : <samples to buffer > */
        OperatorsStandardSetBufferSize(op_splitter,VA_SPLITTER_BUFFER_SIZE);
        /* setting the sample rate and metadata reframing */
        OperatorsStandardSetSampleRate(op_splitter, 16000);
        OperatorsSplitterSetMetadataReframing(op_splitter, splitter_reframing_enable, 384);
    }
}

/****************************************************************************
DESCRIPTION
    Configure common chain
*/
static void configureChain(bool cvc_omni_mode)
{
    configureVad();
    configureCvc(cvc_omni_mode);
    configureSplitter();
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
    Brings up and connects the cvc and splitter. Enables and connects
    the mics and the chain to AEC ref.
*/
void audioVaCreateCommonChain(plugin_variant_t variant,bool cvc_omni_mode)
{
    operator_filters_t* filter_config = getFilterConfig(variant);

    PanicFalse(va_common_chain == NULL);
    va_common_chain = PanicNull(ChainCreateWithFilter(getChainConfig(variant), filter_config));
    freeFilters(filter_config);
    configureChain(cvc_omni_mode);
}

/****************************************************************************
DESCRIPTION
    Start the common chain
*/
void audioVaStartCommonChain(void)
{
    ChainStart(va_common_chain);
}

/****************************************************************************
DESCRIPTION
    Destroy the common chain
*/
void audioVaDestroyCommonChain(void)
{
    if(va_common_chain)
    {
        ChainDestroy(va_common_chain);
        va_common_chain = NULL;
    }
}

/****************************************************************************
DESCRIPTION
    Stop the common chain
*/
void audioVaStopCommonChain(void)
{
    if(va_common_chain)
        ChainStop(va_common_chain);
}

/****************************************************************************
DESCRIPTION
    Connect the common chain
*/
void audioVaConnectCommonChain(void)
{
    ChainConnect(va_common_chain);
}

/****************************************************************************
DESCRIPTION
    Get the trigger output/source of common chain based on trigger output number
    (0- Output number0/ 1-Output number1/ 2-Output number2/ 3-Output number3)
    Trigger chain shall try to connect to this output
*/
Source audioVaGetCommonChainTriggerOutput(uint16 output_num)
{
    return ChainGetOutput(va_common_chain, (va_common_trigger_base_path + output_num));
}
/****************************************************************************
DESCRIPTION
    Get the capture output of common chain based on capture output number
    (0- Output number0/ 1-Output number1/ 2-Output number2/ 3-Output number3)
    Capture chain shall try to connect to this output
*/
Source audioVaGetCommonChainCaptureOutput(uint16 output_num)
{
    return ChainGetOutput(va_common_chain, (va_common_capture_base_path + output_num));
}

/****************************************************************************
DESCRIPTION
    Interface used to configure common chain to get captured data after timestamp
*/
void audioVaConfigCommonChainForStartCapture(uint32 start_timestamp,bool trigger_chain_active)
{
    Operator op_splitter = ChainGetOperatorByRole(va_common_chain,va_common_splitter_role);

    if(op_splitter) /* Only for voice trigger (VTT). */
    {
        if(start_timestamp)
            /* ACTIVATE_STREAMS_AFTER_TIMESTAMP(MSG ID: 0x3), 2-> output stream 1 */
            OperatorsSplitterActivateOutputStreamAfterTimestamp(op_splitter, start_timestamp, splitter_output_stream_1);
        else
        {
            if(trigger_chain_active)
            {
                /* first try to deactivate the output which is on hold */
                OperatorsSplitterDeactivateOutputStream(op_splitter,splitter_output_stream_1);
            }
            OperatorsSplitterActivateOutputStream(op_splitter,splitter_output_stream_1);
        }
    }
}

/****************************************************************************
DESCRIPTION
    Interface used to configure common chain to stop capture
*/
void audioVaConfigCommonChainForStopCapture(void)
{
    Operator op_splitter = ChainGetOperatorByRole(va_common_chain,va_common_splitter_role);

    if(op_splitter) /* Only for voice trigger (VTT). */
    {
        /* DEACTIVATE_STREAMS (MSG ID: 0x4),2-> stream 1 */
        OperatorsSplitterDeactivateOutputStream(op_splitter,splitter_output_stream_1);
    }
}

/****************************************************************************
DESCRIPTION
    Interface used to configure common chain for trigger detection
*/
void audioVaConfigCommonChainForStartTrigger(void)
{
    Operator op_splitter = INVALID_OPERATOR;
    op_splitter = ChainGetOperatorByRole(va_common_chain,va_common_splitter_role);

    /* BUFFER_STREAMS(MSG ID: 0x5), 2-> output stream 1 */
    OperatorsSplitterBufferOutputStream(op_splitter,splitter_output_stream_1);
    /* ACTIVATE_STREAMS(MSG ID: 0x2), 1-> output stream 0 */
    OperatorsSplitterActivateOutputStream(op_splitter,splitter_output_stream_0);
}

/****************************************************************************
DESCRIPTION
    Interface used to configure common chain for Stop Trigger
*/
void audioVaConfigCommonChainForStopTrigger(void)
{
    Operator op_splitter = INVALID_OPERATOR;
    op_splitter = ChainGetOperatorByRole(va_common_chain,va_common_splitter_role);

    /* DEACTIVATE_STREAMS (MSG ID: 0x4),1-> stream 0 */
    OperatorsSplitterDeactivateOutputStream(op_splitter,splitter_output_stream_0);
}


/****************************************************************************
DESCRIPTION
    Get the input to the common chain for given mic
*/
Sink audioVaGetCommonChainMicInput(uint8 mic)
{
    va_common_path_role_t input_id = 0;

    switch(mic)
    {
        case 0:
            input_id = va_common_mic1_path;
            break;
        case 1:
            input_id = va_common_mic2_path;
            break;
        case 2:
            input_id = va_common_mic3_path;
            break;
        default:
            Panic();
            break;
    }
    return ChainGetInput(va_common_chain, input_id);
}

/****************************************************************************
DESCRIPTION
    Get cVc reference input for common chain
*/
Sink audioVaGetCommonChainCvcRefInput(void)
{
    return ChainGetInput(va_common_chain, va_common_aec_ref_path);
}

/****************************************************************************
DESCRIPTION
    Check Common chain is already created or not.
*/
bool audioVaIsCommonChainCreated(void)
{
    return (va_common_chain == NULL)? FALSE:TRUE;
}

/****************************************************************************
DESCRIPTION
    Apply user params to cVc n-Mic operator
*/
void audioVaApplyCommonChainNMicCvcUserParams(bool cvc_omni_mode)
{
    Operator op_cv = ChainGetOperatorByRole(va_common_chain, va_common_cvc_role);

    if (op_cv)
    {
        OperatorsStandardSetUCID(op_cv, getNMicCvcSendUcid(cvc_omni_mode));
    }
}

/****************************************************************************
DESCRIPTION
    Enable omni mode in Get cVc n-Mic usecase Id
*/
void audioVaCommonChainEnableNMicCvcOmniMode(void)
{
    Operator op_cv = ChainGetOperatorByRole(va_common_chain, va_common_cvc_role);

    if(op_cv)
        OperatorsCvcSendEnableOmniMode(op_cv);
}

/****************************************************************************
DESCRIPTION
    Disable omni mode in Get cVc n-Mic usecase Id
*/
void audioVaCommonChainDisableNMicCvcOmniMode(void)
{
    Operator op_cv = ChainGetOperatorByRole(va_common_chain, va_common_cvc_role);

    if(op_cv)
        OperatorsCvcSendDisableOmniMode(op_cv);
}

/****************************************************************************
DESCRIPTION
    Get VAD operator
*/
Operator audioVaGetCommonChainOperator(va_common_operator_role_t role)
{
    Operator op;
      
    op = ChainGetOperatorByRole(va_common_chain,role);
    
    return op;
}
