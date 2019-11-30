/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_hardware_aec_chain_config.c

DESCRIPTION
    Define AEC chain and function to return the AEC chain configuration.
*/

#include <panic.h>

#include "audio_hardware_aec_chain_config.h"
#include "audio_hardware_aec.h"


#define SPEAKER_LEFT_INPUT (0)
#define SPEAKER_RIGHT_INPUT (1)
#define SPEAKER_SECONDARY_LEFT_INPUT (6)
#define SPEAKER_SECONDARY_RIGHT_INPUT (7)
#define MICA_INPUT   (2)
#define MICB_INPUT   (3)
#define MICC_INPUT   (4)

#define REFERENCE_PORT (0)
#define SPEAKER_LEFT_OUTPUT  (1)
#define SPEAKER_RIGHT_OUTPUT  (2)
#define SPEAKER_SECONDARY_LEFT_OUTPUT  (7)
#define SPEAKER_SECONDARY_RIGHT_OUTPUT  (8)
#define MICA_OUTPUT   (3)
#define MICB_OUTPUT   (4)
#define MICC_OUTPUT   (5)

static const operator_config_t aec_ops[] =
{
    MAKE_OPERATOR_CONFIG(capability_id_aec_4mic, aec_role_aec)
};

static const operator_endpoint_t aec_inputs[] =
{
    {aec_role_aec, left_channel,            SPEAKER_LEFT_INPUT},
    {aec_role_aec, right_channel,           SPEAKER_RIGHT_INPUT},
    {aec_role_aec, mic_a_channel,           MICA_INPUT},
    {aec_role_aec, mic_b_channel,           MICB_INPUT},
    {aec_role_aec, mic_c_channel,           MICC_INPUT},
    {aec_role_aec, secondary_left_channel,  SPEAKER_SECONDARY_LEFT_INPUT},
    {aec_role_aec, secondary_right_channel, SPEAKER_SECONDARY_RIGHT_INPUT},
};

static const operator_endpoint_t aec_outputs[] =
{
    {aec_role_aec, mic_ref_channel,                 REFERENCE_PORT},
    {aec_role_aec, left_channel,                    SPEAKER_LEFT_OUTPUT},
    {aec_role_aec, right_channel,                   SPEAKER_RIGHT_OUTPUT},
    {aec_role_aec, mic_a_channel,                   MICA_OUTPUT},
    {aec_role_aec, mic_b_channel,                   MICB_OUTPUT},
    {aec_role_aec, mic_c_channel,                   MICC_OUTPUT},
    {aec_role_aec, secondary_left_channel,          SPEAKER_SECONDARY_LEFT_OUTPUT},
    {aec_role_aec, secondary_right_channel,         SPEAKER_SECONDARY_RIGHT_OUTPUT}
};

static const chain_config_t aec_config =
    MAKE_CHAIN_CONFIG_NO_CONNECTIONS(chain_id_audio_hardware_aec,
        audio_ucid_hardware_aec, aec_ops, aec_inputs, aec_outputs);

/******************************************************************************/
const chain_config_t *hardwareAecGetChainConfig(void)
{
    return &aec_config;
}
