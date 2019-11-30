/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_anc_tuning_chain_config.h

DESCRIPTION
    ANC tuning mode chain configuration.
*/

#ifndef AUDIO_ANC_TUNING_CHAIN_CONFIG_H_
#define AUDIO_ANC_TUNING_CHAIN_CONFIG_H_

#include <chain.h>

#define ANC_TUNING_MODE_SAMPLE_RATE     (48000)

typedef enum
{
    usb_rx_role,
    tuning_role,
    pre_usb_tx_passthrough_role,
    usb_tx_role
} chain_roles_t;

typedef enum
{
    usb_rx_input_pcm    = 0
} usb_rx_operator_input_terminals_t;

typedef enum
{
    usb_rx_left_terminal    = 0,
    usb_rx_right_terminal   = 1,
} usb_rx_operator_output_terminals_t;

typedef enum
{

    usb_input_left_terminal     = 0,
    usb_input_right_terminal    = 1,
    monitor_left_terminal       = 2,
    monitor_right_terminal      = 3,
    mic_1_left_terminal         = 4,
    mic_1_right_terminal        = 5,
    mic_2_left_terminal         = 6,
    mic_2_right_terminal        = 7
} tuning_operator_input_terminals_t;

typedef enum
{
    usb_output_left_terminal    = 0,
    usb_output_right_terminal   = 1,
    speaker_left_terminal       = 2,
    speaker_right_terminal      = 3
} tuning_operator_output_terminals_t;

typedef enum
{
    passthrough_input_left      = 0,
    passthrough_input_right     = 1
} pre_usb_tx_passthrough_operator_input_terminals_t;

typedef enum
{
    passthrough_output_left      = 0,
    passthrough_output_right     = 1
} pre_usb_tx_passthrough_operator_output_terminals_t;

typedef enum
{
    usb_tx_left_terminal    = 0,
    usb_tx_right_terminal   = 1,
} usb_tx_operator_input_terminals_t;

typedef enum
{
    usb_tx_output_pcm   = 0
} usb_tx_operator_output_terminals_t;

typedef enum
{
    path_usb,
    path_usb_internal,
    path_monitor_left,
    path_monitor_right,
    path_mic_1_left,
    path_mic_1_right,
    path_mic_2_left,
    path_mic_2_right,
    path_dac_left,
    path_dac_right
} path_channel_t;

/****************************************************************************
DESCRIPTION
    Return the chain configuration.
*/
const chain_config_t* AudioAncTuningGetChainConfig(void);

#endif /* AUDIO_ANC_TUNING_CHAIN_CONFIG_H_ */
