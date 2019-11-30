/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_a2dp_chain_config_common.c

DESCRIPTION
    Common functions used in audio_input_a2dp chain configs
*/

#include "audio_input_a2dp_chain_config_common.h"
#include <audio_config.h>


bool audioInputA2dpIsSinkConfiguredToUseLeftChannel(void)
{
    return (AudioConfigGetTwsChannelModeLocal() == CHANNEL_MODE_LEFT);
}

bool audioInputA2dpIsSinkInMonoMode(void)
{
    return (AudioConfigGetRenderingMode() == single_channel_rendering);
}

const operator_config_t * audioInputA2dpGetOpConfigWithThisRole(unsigned role, const operator_config_t *op_config_array, unsigned length_of_array)
{
    unsigned i;

    for (i = 0; i < length_of_array; i++)
    {
        if (op_config_array[i].role == role)
            return &op_config_array[i];
    }

    Panic();
    return NULL;
}

operator_config_t audioInputA2dpGetOpConfigFilter(capability_id_t cap_id, const operator_config_t *op_config)
{
    operator_config_t filter;

    /* Copy all fields form base configuration then replace capability ID */
    memcpy(&filter, op_config, sizeof(filter));
    filter.capability_id = cap_id;

    return filter;
}
