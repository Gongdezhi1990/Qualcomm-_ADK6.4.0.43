/*
Copyright (c) 2004 - 2017 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief
    Sink_multi_channel is a thin Sink Application Module that configures the
    Audio Output library. The Configuration Block encapsulated by the Module is
    designed to be directly compatible with the Audio Output API.
*/
#include <string.h>

#include <panic.h>
#include <audio_output.h>

#include "sink_configmanager.h"

#include "sink_multi_channel.h"
#include "sink_multi_channel_config_def.h"

/******************************************************************************/
static void memberwiseCopyConfigBlock(const sink_audio_output_readonly_config_def_t* const cfgBlock,
                                            audio_output_config_t* const cfgLibApi)
{
    /* Use a member-wise copy because we cannot guarantee any struct packing consistency between the two types. */
    audio_output_t outputIdx;
    for (outputIdx=audio_output_primary_left; outputIdx < audio_output_max; outputIdx++ )
    {
        cfgLibApi->mapping[outputIdx].endpoint.type =     cfgBlock->route[outputIdx].audio_output_hw_type;
        cfgLibApi->mapping[outputIdx].endpoint.instance = cfgBlock->route[outputIdx].audio_output_hw_instance;
        cfgLibApi->mapping[outputIdx].endpoint.channel =  cfgBlock->route[outputIdx].audio_output_channel;
        cfgLibApi->mapping[outputIdx].volume_trim =       cfgBlock->route[outputIdx].audio_output_channel_volume_trim;
    }

    cfgLibApi->fixed_hw_gain = cfgBlock->fixed_hardware_gain;
    cfgLibApi->output_resolution_mode = cfgBlock->enable_24_bit_resolution;

    cfgLibApi->gain_type[audio_output_group_main] = cfgBlock->volume_scaling_method_main;
    cfgLibApi->gain_type[audio_output_group_aux] = cfgBlock->volume_scaling_method_aux;
}

/******************************************************************************/
static bool populateAudioOutputConfig(audio_output_config_t* const cfgLibApi)
{
    bool ret_val = FALSE;
    sink_audio_output_readonly_config_def_t *cfgBlock = NULL;

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_OUTPUT_READONLY_CONFIG_BLK_ID, (const void **)&cfgBlock))
    {
        memberwiseCopyConfigBlock(cfgBlock, cfgLibApi);
        ret_val = TRUE;
        configManagerReleaseConfig(SINK_AUDIO_OUTPUT_READONLY_CONFIG_BLK_ID);
    }
    return ret_val;
}

/******************************************************************************/
void SinkMultiChannelInit(void)
{
    audio_output_config_t* config = PanicUnlessNew(audio_output_config_t);
    memset(config, 0, sizeof(*config));

    PanicFalse(populateAudioOutputConfig(config));

    PanicFalse(AudioOutputInit(config));
}
