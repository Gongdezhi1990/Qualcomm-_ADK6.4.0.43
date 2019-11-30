/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_voice_hfp_connect.c

DESCRIPTION
    Audio voice HFP connect implementation.
*/

#include "audio_voice_hfp_connect.h"

#include <stdlib.h>

#include <audio.h>
#include <print.h>
#include <message.h>
#include <audio_voice_common.h>
#include <audio_plugin_voice_variants.h>
#include <chain.h>

#include "audio_voice_hfp.h"

#define MSBC_SCO_FRAME_LENGTH   (60)
#define SCO_METADATA_ENABLE     (1)

#define OPERATORS_SETUP_SWB_ENCODE_CODEC_MODE(mode) \
{ \
    .key = operators_setup_swb_encode_codec_mode, \
    .value = {.codec_mode = mode } \
}

#define OPERATORS_SETUP_SWB_DECODE_CODEC_MODE(mode) \
{ \
    .key = operators_setup_swb_decode_codec_mode, \
    .value = {.codec_mode = mode } \
}

static const operator_setup_item_t uwb_decode_setup[] =
{
    OPERATORS_SETUP_SWB_DECODE_CODEC_MODE(swb_codec_mode_uwb)
};

static const operator_setup_item_t uwb_encode_setup[] =
{
    OPERATORS_SETUP_SWB_ENCODE_CODEC_MODE(swb_codec_mode_uwb)
};

static operator_config_t hfp_nb[] =
{
    MAKE_OPERATOR_CONFIG_PRIORITY_MEDIUM(capability_id_sco_receive, audio_voice_receive_role), \
    MAKE_OPERATOR_CONFIG_PRIORITY_HIGH(capability_id_sco_send, audio_voice_send_role)
};

static operator_config_t hfp_wb[] =
{
    MAKE_OPERATOR_CONFIG_PRIORITY_MEDIUM(capability_id_wbs_receive, audio_voice_receive_role), \
    MAKE_OPERATOR_CONFIG_PRIORITY_HIGH(capability_id_wbs_send, audio_voice_send_role)
};

static operator_config_t hfp_uwb[] =
{
    MAKE_OPERATOR_CONFIG_PRIORITY_MEDIUM_WITH_SETUP(capability_id_swbs_decode, audio_voice_receive_role, uwb_decode_setup), \
    MAKE_OPERATOR_CONFIG_PRIORITY_HIGH_WITH_SETUP(capability_id_swbs_encode, audio_voice_send_role, uwb_encode_setup)
};

static operator_config_t hfp_swb[] =
{
    MAKE_OPERATOR_CONFIG_PRIORITY_MEDIUM(capability_id_swbs_decode, audio_voice_receive_role), \
    MAKE_OPERATOR_CONFIG_PRIORITY_HIGH(capability_id_swbs_encode, audio_voice_send_role)
};
/******************************************************************************/

static operator_config_t* getOperatorFilter(cvc_plugin_type_t variant)
{
    operator_config_t * config = hfp_nb;
    switch(AudioVoiceCommonGetPluginType(variant))
    {
        case narrow_band:
            config = hfp_nb;
            break;
        case wide_band:
            config = hfp_wb;
            break;
        case ultra_wide_band:
            config = hfp_uwb;
            break;
        case super_wide_band:
            config = hfp_swb;
            break;
        default:
            config = hfp_nb;
            break;
    }
    return config;
}

static bool isMsbc(audio_voice_context_t* ctx)
{
    return (AudioVoiceCommonGetPluginType(ctx->variant) == wide_band);
}

static operator_filters_t* getFilters(cvc_plugin_type_t variant)
{
    operator_filters_t* filters = (operator_filters_t*)calloc(1,sizeof(operator_filters_t));

    filters->num_operator_filters = 2;
    filters->operator_filters = getOperatorFilter(variant);

    return filters;
}

static void configureSourceSink( Message msg, audio_voice_context_t* ctx)
{
    AUDIO_PLUGIN_CONNECT_MSG_T* connect_msg =  (AUDIO_PLUGIN_CONNECT_MSG_T*) msg;
    if(isMsbc(ctx))
    {
        SinkConfigure(connect_msg->audio_sink, VM_SINK_SCO_SET_FRAME_LENGTH, MSBC_SCO_FRAME_LENGTH);
    }

    SourceConfigure(StreamSourceFromSink(connect_msg->audio_sink), VM_SOURCE_SCO_METADATA_ENABLE, SCO_METADATA_ENABLE);
}

static uint16 getRxTtpLatency(cvc_plugin_type_t variant)
{
    uint16 rx_ttp_latency = 0;

    switch(AudioVoiceCommonGetPluginType(variant))
    {
        case ultra_wide_band:
        case super_wide_band:
            rx_ttp_latency = AUDIO_SWB_AND_UWB_RX_TTP_LATENCY;
            break;
        default:
            rx_ttp_latency = AUDIO_RX_TTP_LATENCY;
            break;
    }
    return rx_ttp_latency;
}

/******************************************************************************/
void AudioVoiceHfpConnectAndFadeIn(Task task, Message msg, audio_voice_context_t* ctx)
{
    CvcPluginTaskdata* plugin = (CvcPluginTaskdata*)task;
    operator_filters_t* filters = getFilters(plugin->cvc_plugin_variant);

    ctx->variant = plugin->cvc_plugin_variant;
    ctx->ttp_latency.target_in_ms = getRxTtpLatency(plugin->cvc_plugin_variant);
    ctx->ttp_latency.min_in_ms = 0;
    ctx->ttp_latency.max_in_ms = 0;

    /* Need to enable dsp before sending messages to sink/source */
    AudioVoiceCommonDspPowerOn();
    configureSourceSink(msg, ctx);
    AudioVoiceCommonConnectAndFadeIn(task, msg, ctx, filters);
    free(filters);
}

