/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_a2dp_chain_config_common.h

DESCRIPTION
    Common definitions used in audio_input_a2dp chain configs
*/

#ifndef AUDIO_INPUT_A2DP_CHAIN_CONFIG_COMMON_H_
#define AUDIO_INPUT_A2DP_CHAIN_CONFIG_COMMON_H_

#include <chain.h>

#define PASSTHROUGH_BUFFER_SIZE_FOR_MONO (8000)
#define MONO_OUTPUT (path_left_output)
#define MONO_CONSUMED_OUTPUT (path_right_output)
#define UNUSED_TERMINAL (0xFF)

typedef enum
{
    rtp_role,
    splitter_role,
    decoder_role,
    passthrough_role,
    switched_passthrough_role,
    demux_role
} operator_roles_t;

typedef enum
{
    path_input,
    path_forwarding_output,
    path_left_output,
    path_right_output
} chain_paths_common_t;

bool audioInputA2dpIsSinkConfiguredToUseLeftChannel(void);
bool audioInputA2dpIsSinkInMonoMode(void);

/****************************************************************************
DESCRIPTION
    Get the operator configuration for specific operator role supplied
*/
const operator_config_t * audioInputA2dpGetOpConfigWithThisRole(unsigned role, const operator_config_t *op_config_array, unsigned length_of_array);

/****************************************************************************
DESCRIPTION
    Create an operator configuration filter with the desired capability ID
*/
operator_config_t audioInputA2dpGetOpConfigFilter(capability_id_t cap_id, const operator_config_t *op_config);

#endif /* AUDIO_INPUT_A2DP_CHAIN_CONFIG_COMMON_H_ */
