/****************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_voice_prompts_chain_config.h

DESCRIPTION
    Voice prompts audio input chain configuration.
*/

#ifndef AUDIO_INPUT_VOICE_PROMPTS_CHAIN_CONFIG_H_
#define AUDIO_INPUT_VOICE_PROMPTS_CHAIN_CONFIG_H_

#include <chain.h>

typedef enum
{
    tone_role,
    decoder_role,
    dummy_role
} usb_operator_roles_t;

typedef enum
{
    input_path_role,
    left_output_path_role,
    right_output_path_role
} chain_endpoint_path_roles_t;

kymera_chain_handle_t audioInputVoicePromptsCreateToneChain(void);
kymera_chain_handle_t audioInputVoicePromptsCreatePromptDecoderChain(void);
kymera_chain_handle_t audioInputVoicePromptsCreatePromptDummyChain(void);

#endif /* AUDIO_INPUT_VOICE_PROMPTS_CHAIN_CONFIG_H_ */
