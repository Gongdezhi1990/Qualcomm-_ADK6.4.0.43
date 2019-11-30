/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_input_usb_chain_config.c

DESCRIPTION
    Implementation of function to return the appropriate chain configuration data.
*/

#include <operators.h>
#include <panic.h>
#include "audio_input_usb_chain_config.h"
#include "audio_config.h"

static const operator_config_t op_config[] =
{
    MAKE_OPERATOR_CONFIG(capability_id_usb_audio_rx, usb_rx_role)
};

static const operator_path_node_t encoded_channel[] =
{
    {usb_rx_role, 0, 0},
};

static const operator_path_node_t pcm_left_channel[] =
{
    {usb_rx_role, 0, 0}
};

static const operator_path_node_t pcm_right_channel[] =
{
    {usb_rx_role, 0, 1}
};

static const operator_path_t paths[] =
{
    {path_encoded, path_with_input, ARRAY_DIM((encoded_channel)), encoded_channel},
    {path_pcm_left, path_with_output, ARRAY_DIM((pcm_left_channel)), pcm_left_channel},
    {path_pcm_right, path_with_output, ARRAY_DIM((pcm_right_channel)), pcm_right_channel},
};

#define OPERATORS_SETUP_SWITCHED_PASSTHROUGH_FORMAT(format) \
{ \
    .key = operators_setup_switched_passthrough_set_format, \
    .value = {.spc_format = (format)} \
}

static const operator_setup_item_t switched_passthrough_pcm_setup[] =
{
    OPERATORS_SETUP_SWITCHED_PASSTHROUGH_FORMAT(spc_op_format_pcm)
};

/* Mono chains */
static const operator_config_t op_config_mono[] =
{
    MAKE_OPERATOR_CONFIG(capability_id_usb_audio_rx, usb_rx_role),
    MAKE_OPERATOR_CONFIG_WITH_SETUP(capability_id_switched_passthrough_consumer,switched_passthrough_role, switched_passthrough_pcm_setup),
};

static const operator_path_node_t left_forwarding_channel[] =
{
    {usb_rx_role, 0, 0},
    {switched_passthrough_role, 0, 0}

};

static const operator_path_node_t right_forwarding_channel[] =
{
    {usb_rx_role, 0, 1},
    {switched_passthrough_role, 0, 0}
};

static const operator_path_t paths_mono_left[] =
{
    {path_encoded, path_with_input, ARRAY_DIM((encoded_channel)), encoded_channel},
    {path_pcm_forwarding, path_with_output, ARRAY_DIM((right_forwarding_channel)), right_forwarding_channel},
    {path_pcm_left, path_with_output, ARRAY_DIM((pcm_left_channel)), pcm_left_channel},
};

static const operator_path_t paths_mono_right[] =
{
    {path_encoded, path_with_input, ARRAY_DIM((encoded_channel)), encoded_channel},
    {path_pcm_left, path_with_output, ARRAY_DIM((pcm_right_channel)), pcm_right_channel},
    {path_pcm_forwarding, path_with_output, ARRAY_DIM((left_forwarding_channel)), left_forwarding_channel},
};

static const chain_config_t usb_chain_config_mono_left =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_input_usb, audio_ucid_input_digital, op_config_mono, paths_mono_left);

static const chain_config_t usb_chain_config_mono_right =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_input_usb, audio_ucid_input_digital, op_config_mono, paths_mono_right);


static const chain_config_t usb_chain_config =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_input_usb, audio_ucid_input_digital, op_config, paths);


static bool isLeftChannel(void)
{
    return (AudioConfigGetTwsChannelModeLocal()== CHANNEL_MODE_LEFT);
}

bool isMonoMode(void)
{
    return (AudioConfigGetRenderingMode()==single_channel_rendering);
}

const chain_config_t* AudioInputUsbGetChainConfig(void)
{
    if (isMonoMode())
    {
	    return isLeftChannel() ? &usb_chain_config_mono_left : &usb_chain_config_mono_right;
    }
    else
    {
        return &usb_chain_config;
    }
}
