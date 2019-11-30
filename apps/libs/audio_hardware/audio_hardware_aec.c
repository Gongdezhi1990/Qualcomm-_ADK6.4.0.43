/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_hardware_aec.c

DESCRIPTION
       Implementation of AEC management
*/

#include <stddef.h>
#include <string.h>
#include <audio_ports.h>

#include <panic.h>

#include "audio_hardware_aec.h"
#include "audio_hardware_aec_chain_config.h"
#include "audio_hardware_output.h"
#include "audio_plugin_ucid.h"
#include "audio_config.h"

#define AEC_VERSION_TTP_SUPPORTED_MSB 2
#define AEC_VERSION_TTP_SUPPORTED_LSB 4

#define AEC_VERSION_TASK_PERIOD_SUPPORTED_MSB 2
#define AEC_VERSION_TASK_PERIOD_SUPPORTED_LSB 6

typedef struct aec_data
{
    kymera_chain_handle_t chain;
    audio_hw_mic_use_t  aec_mic_use;
    unsigned number_of_mics;
    uint32   output_sample_rate;
    uint32   mic_sample_rate;
} aec_data_t;

typedef enum
{
    aec_use_direction_out,
    aec_use_direction_in,
}aec_use_direction_t;

static uint8 aec_in_use_flags = 0;

static aec_data_t aec_data =
{
    .chain = NULL,
    .aec_mic_use = audio_hw_voice_call,
    .number_of_mics = 0,
    .mic_sample_rate = DEFAULT_MIC_SAMPLE_RATE,
    .output_sample_rate = 0
};


/****************************************************************************
DESCRIPTION
    Register the AEC operator with the ports library
*/
static void hardwareAecRegisterExternalPorts(void)
{
    AudioPortsSetAecReference(ChainGetOutput(aec_data.chain, mic_ref_channel));

    AudioPortsSetAecOutputTerminal(ChainGetOutput(aec_data.chain, mic_a_channel), 0);
    AudioPortsSetAecOutputTerminal(ChainGetOutput(aec_data.chain, mic_b_channel), 1);
    AudioPortsSetAecOutputTerminal(ChainGetOutput(aec_data.chain, mic_c_channel), 2);

    AudioPortsSetAecMicInput(ChainGetInput(aec_data.chain, mic_a_channel), 0);
    AudioPortsSetAecMicInput(ChainGetInput(aec_data.chain, mic_b_channel), 1);
    AudioPortsSetAecMicInput(ChainGetInput(aec_data.chain, mic_c_channel), 2);
}

/****************************************************************************
DESCRIPTION
    Derive AEC UCID configuration from mic sample rate
*/
static ucid_aec_t hardwareAecGetUcid(uint32 mic_sample_rate)
{
    ucid_aec_t ucid_aec = ucid_aec_nb;
    if(aec_data.aec_mic_use == audio_hw_voice_assistant)
    {
        ucid_aec = ucid_aec_wb_va;
    }
    else if(aec_data.aec_mic_use == audio_hw_voice_call)
    {
        switch(mic_sample_rate)
        {
            case 16000:
                ucid_aec = ucid_aec_wb;
                break;
            case 24000:
                ucid_aec = ucid_aec_uwb;
                break;
            case 32000:
                ucid_aec = ucid_aec_swb;
                break;
            case 8000:
            default:
                ucid_aec = ucid_aec_nb;
                break;
        }
    }
    return ucid_aec;
}


/****************************************************************************
DESCRIPTION
    Configure the rate at which to run the microphone/reference AEC outputs
*/
static void hardwareAecSetMicRate(uint32 mic_sample_rate)
{
    aec_data.mic_sample_rate = mic_sample_rate;
}

/****************************************************************************
DESCRIPTION
    Configure the output sample rate
*/
static void hardwareAecSetOutputSampleRate(uint32 output_sample_rate)
{
    aec_data.output_sample_rate = output_sample_rate;
}

/****************************************************************************
DESCRIPTION
    Check wheather configuring task period is supported 
*/
static bool isTaskPeriodConfigsupported(capablity_version_t cap_version)
{

    if(cap_version.version_msb > AEC_VERSION_TASK_PERIOD_SUPPORTED_MSB)
        return TRUE;

    if(cap_version.version_msb == AEC_VERSION_TASK_PERIOD_SUPPORTED_MSB)
    {
        if(cap_version.version_lsb >= AEC_VERSION_TASK_PERIOD_SUPPORTED_LSB)
            return TRUE;
    }

    return FALSE;
}

/****************************************************************************
DESCRIPTION
    Apply configuration settings to the AEC operator
    Set AEC UCID and Sample Rates
*/
static void hardwareAecConfigureChain(void)
{
#define DEFAULT_OP_SAMPLE_RATE 48000

    Operator aec_op;

    aec_op = ChainGetOperatorByRole(aec_data.chain, aec_role_aec);

    if(aec_data.output_sample_rate == 0)
    {
        hardwareAecSetOutputSampleRate(DEFAULT_OP_SAMPLE_RATE);
    }

    if (aec_op)
    {
        OperatorsStandardSetUCID(aec_op, hardwareAecGetUcid(aec_data.mic_sample_rate));
        OperatorsAecSetSampleRate(aec_op, aec_data.output_sample_rate, aec_data.mic_sample_rate);
    }
}

/****************************************************************************
DESCRIPTION
    Unregister any AEC ports previously registered with audio_ports
*/
static void hardwareAecClearRegisteredPorts(unsigned number_of_mics)
{
    unsigned i;
    AudioPortsSetAecReference(NULL);

    for (i = 0; i < number_of_mics; i++)
    {
        AudioPortsSetAecOutputTerminal(NULL, i);
        AudioPortsSetAecMicInput(NULL, i);
    }
}

/****************************************************************************
DESCRIPTION
    Initialise the AEC object
*/
static void hardwareInitAec(unsigned number_of_mics)
{
    aec_data.number_of_mics = number_of_mics;
    hardwareAecClearRegisteredPorts(number_of_mics);
}

/****************************************************************************
DESCRIPTION
    Create the AEC object
*/
static void hardwareAecCreate(void)
{
    const chain_config_t* config = hardwareAecGetChainConfig();

    aec_data.chain = PanicNull(ChainCreate(config));

    /*AEC sample rates must be set before it is connected */
    hardwareAecConfigureChain();

    ChainConnect(aec_data.chain);
}

/****************************************************************************
DESCRIPTION
    Get the chain handle for the AEC object
*/
static kymera_chain_handle_t hardwareAecGetChain(void)
{
    return aec_data.chain;
}

/****************************************************************************
DESCRIPTION
    Start processing within the AEC object
*/
static void hardwareAecStart(void)
{
    if (aec_data.chain != NULL)
    {
        ChainStart(aec_data.chain);
    }
}

/****************************************************************************
DESCRIPTION
    Stop processing within the AEC object
*/
static void hardwareAecStop(void)
{
    if (aec_data.chain != NULL)
    {
        ChainStop(aec_data.chain);
    }
}

/****************************************************************************
DESCRIPTION
    Destroy the AEC object
*/
static void hardwareAecDestroy(void)
{
    if (aec_data.chain != NULL)
    {
        ChainDestroy(aec_data.chain);
        hardwareAecClearRegisteredPorts(aec_data.number_of_mics);
        
        aec_data.chain = NULL;
        aec_data.aec_mic_use = audio_hw_voice_call;
        aec_data.mic_sample_rate = DEFAULT_MIC_SAMPLE_RATE;
        aec_data.number_of_mics = 0;
        aec_data.output_sample_rate = 0;
    }
}
/****************************************************************************
DESCRIPTION
    Check wheather ttp is supported. 
*/
static bool isttpsupported(capablity_version_t cap_version)
{

    if(cap_version.version_msb > AEC_VERSION_TTP_SUPPORTED_MSB)
    {
        return TRUE;
    }
	
    if(cap_version.version_msb == AEC_VERSION_TTP_SUPPORTED_MSB)
    {
        if(cap_version.version_lsb >= AEC_VERSION_TTP_SUPPORTED_LSB)
        {
            return TRUE;
        }
    }

    return FALSE;
}

/****************************************************************************
DESCRIPTION
    Set the AEC Task Period
*/
static void hardwareAecSetTaskPeriod(void)
{
#define AEC_TASK_PERIOD_1MS 1000
#define AEC_TASK_PERIOD_2MS 2*AEC_TASK_PERIOD_1MS

#define AEC_DECIM_FACTOR_DEFAULT 1

    Operator aec_op = ChainGetOperatorByRole(aec_data.chain, aec_role_aec);

    if(aec_op)
    {
        capablity_version_t cap_version = OperatorGetCapabilityVersion(aec_op);

        /* Set the task period only if AEC Reference supports it */
        if(isTaskPeriodConfigsupported(cap_version))
        {
            /* Update the task period */
            OperatorsAecSetTaskPeriod(aec_op, 
                (uint16)((aec_data.aec_mic_use == audio_hw_voice_assistant) ? AEC_TASK_PERIOD_2MS : AEC_TASK_PERIOD_1MS), 
                AEC_DECIM_FACTOR_DEFAULT);
        }
    }
}

/****************************************************************************
DESCRIPTION
    Set the AEC TTP
*/
static void hardwareAecSetTtp(void)
{
#define AEC_TTP_LATENCY 40000

    Operator aec_op = ChainGetOperatorByRole(aec_data.chain, aec_role_aec);

    if(aec_op)
    {
        capablity_version_t cap_version = OperatorGetCapabilityVersion(aec_op);

        /* Set time to play latency if the Aec version greater than equal to 2.4. */
        if(isttpsupported(cap_version) && (aec_data.aec_mic_use == audio_hw_voice_assistant))
        {
            OperatorsStandardSetTimeToPlayLatency(aec_op, AEC_TTP_LATENCY);
        }
    }
}

/****************************************************************************/
static void hardwareAecEnable(aec_use_direction_t direction_enabled)
{
    uint8 old_flags = aec_in_use_flags;

    aec_in_use_flags |=  (1 << direction_enabled);
    
    if((old_flags == 0) && (aec_in_use_flags != 0))
    {
        hardwareInitAec(MAX_NUMBER_OF_MICS);
        hardwareAecCreate();
        hardwareAecSetTaskPeriod();
        hardwareAecSetTtp();
        hardwareAecStart();
    }
}

/****************************************************************************/
static void hardwareAecDisable(aec_use_direction_t direction_disabled)
{
    uint8 old_flags = aec_in_use_flags;

    aec_in_use_flags &=  ~(1 << direction_disabled);

    if((old_flags != 0) && (aec_in_use_flags == 0))
    {
        hardwareAecStop();
        hardwareAecDestroy();
    }
}

/****************************************************************************/
void hardwareAecConfigureMicSampleRate(uint32 mic_sample_rate)
{
    hardwareAecSetMicRate(mic_sample_rate);

    hardwareAecConfigureChain();
}

/****************************************************************************/
static bool hardwareAecIsMusicInput(connection_type_t connection_type)
{
    return ((connection_type == CONNECTION_TYPE_MUSIC)||(connection_type == CONNECTION_TYPE_MUSIC_A2DP));
}

/****************************************************************************/
bool hardwareIsAecRequired(connection_type_t connection_type)
{
    if(connection_type == CONNECTION_TYPE_VOICE)
        return TRUE;

    if(AudioConfigGetAlwaysUseAecRef())
        return TRUE;

    if (hardwareAecIsMusicInput(connection_type))
    {
        if (AudioConfigGetMaximumConcurrentInputs() > 1)
            return TRUE;
    }

    return FALSE;
}

/****************************************************************************/
void hardwareAecCreateAndConnectInput(const uint32 mic_sample_rate)
{
    hardwareAecSetMicRate(mic_sample_rate);
    //Instantiate AEC ref
    hardwareAecEnable(aec_use_direction_in);
    hardwareAecConfigureChain();
    hardwareAecRegisterExternalPorts();
}

/****************************************************************************/
void hardwareAecCreateAndConnectOutput(const audio_hardware_connect_t* hw_connect_data)
{

    hardwareAecSetOutputSampleRate(hw_connect_data->output_sample_rate);

    hardwareAecEnable(aec_use_direction_out);
    hardwareAecConfigureChain();

    hardwareMapAecToAudioOutput(hardwareAecGetChain());

    /*Connection order of AEC is important, connect outputs first */
    hardwareConnectToAudioOutput(hw_connect_data->output_sample_rate);

    hardwareConnectAecInputs(hw_connect_data, hardwareAecGetChain());

}

/****************************************************************************/
void hardwareAecDisconnectInput(void)
{
    hardwareAecDisable(aec_use_direction_in);
}

/****************************************************************************/
void hardwareAecDisconnectOutput(void)
{
    hardwareAecDisable(aec_use_direction_out);
}

/****************************************************************************/
void hardwareAecMuteMicOutput(bool enable)
{
    Operator aec_op;

    aec_op = ChainGetOperatorByRole(aec_data.chain, aec_role_aec);
    if (aec_op)
    {
        OperatorsAecMuteMicOutput(aec_op, enable);
    }
}

/****************************************************************************/
Operator hardwareAecGetOperator(void)
{
    return ChainGetOperatorByRole(aec_data.chain, aec_role_aec);
}

/****************************************************************************/
void hardwareAecSetMicUse(audio_hw_mic_use_t  mic_use)
{
    aec_data.aec_mic_use = mic_use;
}
