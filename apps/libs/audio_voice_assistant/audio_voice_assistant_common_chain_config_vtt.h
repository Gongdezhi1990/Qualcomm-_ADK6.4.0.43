/****************************************************************************
Copyright (c) 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_voice_assistant_common_chain_config_vtt.h

DESCRIPTION
    Handles creation and control of the common chain of the
    VA graph - VTT case
*/

#ifndef AUDIO_VOICE_ASSISTANT_COMMON_CHAIN_CONFIG_VTT_H_
#define AUDIO_VOICE_ASSISTANT_COMMON_CHAIN_CONFIG_VTT_H_

/****************************************************************************
DESCRIPTION
    Retrieve the operator for a given role.
*/
extern const operator_config_t* getOperatorConfigBasedOnRoleVtt(unsigned role);

/****************************************************************************
DESCRIPTION
    Retrieve a chain configuration that's selected by cVc variant.
*/
extern const chain_config_t* getChainConfigVtt(plugin_variant_t variant);

#endif /* AUDIO_VOICE_ASSISTANT_COMMON_CHAIN_CONFIG_VTT_H_ */
