/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_voice_assistant_common_chain.h

DESCRIPTION
    Handles creation and control of the common chain of the
    VA graph
*/

#ifndef AUDIO_VOICE_ASSISTANT_COMMON_CHAIN_H_
#define AUDIO_VOICE_ASSISTANT_COMMON_CHAIN_H_

#include <chain.h>
#include <audio_plugin_if.h>
#include <audio_plugin_voice_variants.h>

typedef enum
{
    va_common_vad_role,
    va_common_cvc_role,
    va_common_splitter_role
} va_common_operator_role_t;


typedef enum
{
    va_common_mic1_path         = 0,
    va_common_mic2_path         = 1,
    va_common_mic3_path         = 2,
    va_common_aec_ref_path      = 3,
    va_common_trigger_base_path = 4,
    va_common_trigger1_path     = 4,
    va_common_trigger2_path     = 5,
    va_common_trigger3_path     = 6,
    va_common_trigger4_path     = 7,
    va_common_capture_base_path = 8,
    va_common_capture1_path     = 8,
    va_common_capture2_path     = 9,
    va_common_capture3_path     = 10,
    va_common_capture4_path     = 11
} va_common_path_role_t;

/****************************************************************************
DESCRIPTION
    Brings up and connects the cvc and splitter. Enables and connects
    the mics and the chain to AEC ref.
*/
void audioVaCreateCommonChain(plugin_variant_t variant,bool cvc_omni_mode);

/****************************************************************************
DESCRIPTION
    Start the common chain
*/
void audioVaStartCommonChain(void);

/****************************************************************************
DESCRIPTION
    Get the trigger output/source of common chain based on trigger output number
    (0-VA_TRIGGER1/ 1-VA_TRIGGER2/ 2-VA_TRIGGER3/ 3-VA_TRIGGER4)
    Trigger chain shall try to connect to this output
*/
Source audioVaGetCommonChainTriggerOutput(uint16 output_num);

/****************************************************************************
DESCRIPTION
    Get the output 1 of common chain, this is the out put one of splitter 
    Capture chain shall try to connect to this out put
*/
Source audioVaGetCommonChainCaptureOutput(uint16 output_num);

/****************************************************************************
DESCRIPTION
    Interface used to configure common chain to get captured data after timestamp
*/
void audioVaConfigCommonChainForStartCapture(uint32 start_timestamp, bool trigger_chain_active);


/****************************************************************************
DESCRIPTION
    Interface used to configure common chain to stop capture
*/
void audioVaConfigCommonChainForStopCapture(void);

/****************************************************************************
DESCRIPTION
    Interface used to configure common chain for trigger detection
*/
void audioVaConfigCommonChainForStartTrigger(void);

/****************************************************************************
DESCRIPTION
    Interface used to configure common chain for Stop Trigger
*/
void audioVaConfigCommonChainForStopTrigger(void);

/****************************************************************************
DESCRIPTION
    Get the input to the common chain for given mic
*/
Sink audioVaGetCommonChainMicInput(uint8 mic);

/****************************************************************************
DESCRIPTION
    Get cVc reference input for common chain
*/
Sink audioVaGetCommonChainCvcRefInput(void);

/****************************************************************************
DESCRIPTION
    Check Common chain is already created or not.
*/
bool audioVaIsCommonChainCreated(void);

/****************************************************************************
DESCRIPTION
    Destroy the common chain
*/
void audioVaDestroyCommonChain(void);

/****************************************************************************
DESCRIPTION
    Stop the common chain
*/
void audioVaStopCommonChain(void);

/****************************************************************************
DESCRIPTION
    Connect Common Chain
*/    
void audioVaConnectCommonChain(void);

/****************************************************************************
DESCRIPTION
    Apply user params to cVc n-Mic operator
*/
void audioVaApplyCommonChainNMicCvcUserParams(bool cvc_omni_mode);

/****************************************************************************
DESCRIPTION
    Enable omni mode in Get cVc n-Mic usecase Id
*/
void audioVaCommonChainEnableNMicCvcOmniMode(void);

/****************************************************************************
DESCRIPTION
    Disable omni mode in Get cVc n-Mic usecase Id
*/
void audioVaCommonChainDisableNMicCvcOmniMode(void);

/****************************************************************************
DESCRIPTION
    Get operator from common chain
*/
Operator audioVaGetCommonChainOperator(va_common_operator_role_t role);

#endif /* AUDIO_VOICE_ASSISTANT_COMMON_CHAIN_H_ */

