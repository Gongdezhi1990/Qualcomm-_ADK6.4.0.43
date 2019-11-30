/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_tws_chain_config_common.h

DESCRIPTION
    Common definitions used in audio_input_tws chain configs
*/

#ifndef AUDIO_INPUT_TWS_CHAIN_CONFIG_COMMON_H_
#define AUDIO_INPUT_TWS_CHAIN_CONFIG_COMMON_H_

#define MONO_OUTPUT (path_left_output)
#define MONO_CONSUMED_OUTPUT (path_right_output)
#define UNUSED_TERMINAL (0xFF)

typedef enum
{
    passthrough_role,
    decoder_role,
    switched_passthrough_role
} operator_roles_t;

typedef enum
{
    path_input,
    path_left_output,
    path_right_output
} chain_paths_common_t;

typedef enum
{
    audio_input_tws_sbc,
    audio_input_tws_aptx,
    audio_input_tws_aac,
    audio_input_tws_aptx_adaptive
} audio_input_tws_decoder_t;

#endif /* AUDIO_INPUT_TWS_CHAIN_CONFIG_COMMON_H_ */
