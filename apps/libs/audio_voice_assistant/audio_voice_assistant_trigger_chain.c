/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_voice_assistant_trigger_chain.c

DESCRIPTION
    Handle Vocie Assistant trigger requirements. Enabling/Disbaling QVA operator
    and creating a chain as Splitter->QVA and managing the same
*/

#include <operators.h>
#include <audio_config.h>
#include <print.h>
#include <audio_hardware.h>
#include <audio_ports.h>
#include <audio_plugin_ucid.h>

#include "audio_voice_assistant_trigger_chain.h"
#include "audio_voice_assistant_handlers.h"

#define UNUSED_TERMINAL       (0xff)
#define VA_SAMPLE_RATE          (16000)

static kymera_chain_handle_t va_trigger_chain = NULL;
static DataFileID va_trigger_data = DATA_FILE_ID_INVALID;
static const operator_config_t va_trigger_op_config[] =
{
    MAKE_OPERATOR_CONFIG(capability_id_qva, va_qva_role)
};

static const operator_path_node_t va_path_node_trigger1[] =
{
    {va_qva_role, 0, UNUSED_TERMINAL}
};

static const operator_path_node_t va_path_node_trigger2[] =
{
    {va_qva_role, 1, UNUSED_TERMINAL}
};

static const operator_path_node_t va_path_node_trigger3[] =
{
    {va_qva_role, 2, UNUSED_TERMINAL}
};

static const operator_path_node_t va_path_node_trigger4[] =
{
    {va_qva_role, 3, UNUSED_TERMINAL}
};

static const operator_path_t single_channel_path[] =
{
    {va_trigger1_path, path_with_input, ARRAY_DIM((va_path_node_trigger1)), va_path_node_trigger1},
};

static const operator_path_t multi_channel_paths[] =
{
    {va_trigger1_path, path_with_input, ARRAY_DIM((va_path_node_trigger1)), va_path_node_trigger1},
    {va_trigger2_path, path_with_input, ARRAY_DIM((va_path_node_trigger2)), va_path_node_trigger2},
    {va_trigger3_path, path_with_input, ARRAY_DIM((va_path_node_trigger3)), va_path_node_trigger3},
    {va_trigger4_path, path_with_input, ARRAY_DIM((va_path_node_trigger4)), va_path_node_trigger4},
};

static const chain_config_t va_trigger_chain_single_channel_config =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_va, audio_ucid_va, va_trigger_op_config, single_channel_path);

static const chain_config_t va_trigger_chain_multi_channel_config =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_va, audio_ucid_va, va_trigger_op_config, multi_channel_paths);

/****************************************************************************
DESCRIPTION
    Function to Configure VA QVA operators for trigger phrase detection
*/
static void configureChain(FILE_INDEX trigger_file)
{
    Operator qva_operator;

    if(va_trigger_data == DATA_FILE_ID_INVALID)
        va_trigger_data = PanicZero(OperatorDataLoadEx(trigger_file, DATAFILE_BIN, STORAGE_INTERNAL, FALSE));

    qva_operator = ChainGetOperatorByRole(va_trigger_chain,va_qva_role);

    OperatorsStandardSetSampleRate(qva_operator, VA_SAMPLE_RATE);
    OperatorsStandardSetUCID(qva_operator, ucid_qva);
    OperatorsConfigureQvaTriggerPhrase(qva_operator, va_trigger_data);
}

/****************************************************************************
DESCRIPTION
    Get the chain config for capture chain
*/
static const chain_config_t* getChainConfig(void)
{
    return audioVaIsFFVEnabled()? &va_trigger_chain_multi_channel_config: &va_trigger_chain_single_channel_config;
}

/****************************************************************************
DESCRIPTION
    Creates and connects the trigger chain
*/
bool audioVaCreateTriggerChain(FILE_INDEX trigger_file)
{
   if(va_trigger_chain == NULL)
   {
    	va_trigger_chain = PanicNull(ChainCreate(getChainConfig()));
    	configureChain(trigger_file);
	return TRUE;
   }
   return FALSE;
}

/****************************************************************************
DESCRIPTION
    Starts the trigger chain
*/
void audioVaStartTriggerChain(void)
{
    ChainStart(va_trigger_chain);
}

/****************************************************************************
DESCRIPTION
    Stops the trigger chain
*/
void audioVaStopTriggerChain(void)
{
    if(va_trigger_chain)
        ChainStop(va_trigger_chain);
}

/****************************************************************************
DESCRIPTION
*/
void audioVaDestroyTriggerChain(void)
{
    if(va_trigger_chain)
    {
       if(va_trigger_data != DATA_FILE_ID_INVALID)
       {
		PanicFalse(OperatorDataUnloadEx(va_trigger_data));
		va_trigger_data = DATA_FILE_ID_INVALID;
      }

       ChainDestroy(va_trigger_chain);
       va_trigger_chain = NULL;
    }
}

/****************************************************************************
DESCRIPTION
    Get the input/sink of trigger chain based on input trigger number
    (VA_TRIGGER1/ VA_TRIGGER2/ VA_TRIGGER3/ VA_TRIGGER4)
*/
Sink audioVaGetTriggerChainInput(uint16 input_num)
{
    return ChainGetInput(va_trigger_chain, (va_trigger_path_base + input_num));
}

/****************************************************************************
DESCRIPTION
    Get QVA operator
*/
Operator audioVaGetTriggerChainQvaOperator(void)
{
    Operator qva_operator;
      
    qva_operator = ChainGetOperatorByRole(va_trigger_chain,va_qva_role);
	
	return qva_operator;
}
