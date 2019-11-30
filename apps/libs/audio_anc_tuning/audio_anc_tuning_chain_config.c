/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_anc_tuning_chain_config.c

DESCRIPTION

*/

#include <operators.h>
#include "audio_anc_tuning_chain_config.h"

#define USB_SAMPLE_SIZE_16_BIT              (2)

#define USB_SAMPLE_SIZE                     USB_SAMPLE_SIZE_16_BIT
#define USB_CHANNELS                        (2)
#define USB_SAMPLE_RATE                     ANC_TUNING_MODE_SAMPLE_RATE

#define UNCONNECTED                         (255) // put this in common file

#define OPERATORS_SETUP_USB_CONFIG(sample_rate, sample_size, num_channels) \
{ \
    .key = operators_setup_usb_config, \
    .value = {.usb_config = {sample_rate, sample_size, num_channels}} \
}

static const operator_setup_item_t usb_setup[] =
{
    OPERATORS_SETUP_USB_CONFIG(USB_SAMPLE_RATE, USB_SAMPLE_SIZE, USB_CHANNELS)
};

static operator_config_t tuning_mode_operators[] =
{
    MAKE_OPERATOR_CONFIG_WITH_SETUP(capability_id_usb_audio_rx, usb_rx_role, usb_setup),
    MAKE_OPERATOR_CONFIG(capability_id_anc_tuning, tuning_role),
    MAKE_OPERATOR_CONFIG(capability_id_passthrough, pre_usb_tx_passthrough_role),
    MAKE_OPERATOR_CONFIG_WITH_SETUP(capability_id_usb_audio_tx, usb_tx_role, usb_setup)
};

static const operator_path_node_t usb_left[] =
{
    {usb_rx_role, usb_rx_input_pcm, usb_rx_left_terminal},
    {tuning_role, usb_input_left_terminal, usb_output_left_terminal},
    {pre_usb_tx_passthrough_role, passthrough_input_left, passthrough_output_left},
    {usb_tx_role, usb_tx_left_terminal, usb_tx_output_pcm}
};

static const operator_path_node_t usb_right[] =
{
    {usb_rx_role, UNCONNECTED, usb_rx_right_terminal},
    {tuning_role, usb_input_right_terminal, usb_output_right_terminal},
    {pre_usb_tx_passthrough_role, passthrough_input_right, passthrough_output_right},
    {usb_tx_role, usb_tx_right_terminal, UNCONNECTED}
};

static const operator_path_node_t speaker_left[] =
{
    {tuning_role, UNCONNECTED, speaker_left_terminal},
};

static const operator_path_node_t speaker_right[] =
{
    {tuning_role, UNCONNECTED, speaker_right_terminal},
};

static const operator_path_node_t monitor_left[] =
{
    {tuning_role, monitor_left_terminal, UNCONNECTED}
};

static const operator_path_node_t monitor_right[] =
{
    {tuning_role, monitor_right_terminal, UNCONNECTED}
};

static const operator_path_node_t mic_1_left[] =
{
    {tuning_role, mic_1_left_terminal, UNCONNECTED}
};

static const operator_path_node_t mic_1_right[] =
{
    {tuning_role, mic_1_right_terminal, UNCONNECTED}
};

static const operator_path_node_t mic_2_left[] =
{
    {tuning_role, mic_2_left_terminal, UNCONNECTED}
};

static const operator_path_node_t mic_2_right[] =
{
    {tuning_role, mic_2_right_terminal, UNCONNECTED}
};

static const operator_path_t paths[] =
{
    {path_usb, path_with_in_and_out, ARRAY_DIM((usb_left)), usb_left},
    {path_usb_internal, path_with_no_in_or_out, ARRAY_DIM((usb_right)), usb_right},

    {path_monitor_left, path_with_input, ARRAY_DIM((monitor_left)), monitor_left},
    {path_monitor_right, path_with_input, ARRAY_DIM((monitor_right)), monitor_right},

    {path_dac_left, path_with_input, ARRAY_DIM((speaker_left)), speaker_left},
    {path_dac_right, path_with_input, ARRAY_DIM((speaker_right)), speaker_right},

    {path_mic_1_left, path_with_input, ARRAY_DIM((mic_1_left)), mic_1_left},
    {path_mic_1_right, path_with_input, ARRAY_DIM((mic_1_right)), mic_1_right},
    {path_mic_2_left, path_with_input, ARRAY_DIM((mic_2_left)), mic_2_left},
    {path_mic_2_right, path_with_input, ARRAY_DIM((mic_2_right)), mic_2_right}
};

static const chain_config_t tuning_mode_config =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_tuning_mode, audio_ucid_anc_tuning_mode, tuning_mode_operators, paths);

const chain_config_t* AudioAncTuningGetChainConfig(void)
{
    return &tuning_mode_config;
}
