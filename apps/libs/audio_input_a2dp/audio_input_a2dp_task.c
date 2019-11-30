/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_a2dp_task.c

DESCRIPTION
    Helper to get data from the plugin task
*/
#include <vmtypes.h>
#include <panic.h>
#include <audio_plugin_music_variants.h>
#include <audio_input_a2dp_task.h>
#include <audio_input_a2dp.h>

const A2dpPluginTaskdata csr_sbc_decoder_plugin = {{AudioPluginA2dpMessageHandler}, SBC_DECODER, BITFIELD_CAST(8, 0)};
const A2dpPluginTaskdata csr_aptx_decoder_plugin = {{AudioPluginA2dpMessageHandler}, APTX_DECODER, BITFIELD_CAST(8, 0)};
const A2dpPluginTaskdata csr_aptx_acl_sprint_decoder_plugin = {{AudioPluginA2dpMessageHandler}, APTX_ACL_SPRINT_DECODER, BITFIELD_CAST(8, 0)};
const A2dpPluginTaskdata csr_aac_decoder_plugin = {{AudioPluginA2dpMessageHandler}, AAC_DECODER, BITFIELD_CAST(8, 0)};
const A2dpPluginTaskdata csr_aptxhd_decoder_plugin = {{AudioPluginA2dpMessageHandler}, APTXHD_DECODER, BITFIELD_CAST(8, 0)};
const A2dpPluginTaskdata csr_aptx_ad_decoder_plugin = {{AudioPluginA2dpMessageHandler}, APTX_AD_DECODER, BITFIELD_CAST(8, 0)};

const A2dpPluginTaskdata csr_tws_master_sbc_decoder_plugin = {{AudioPluginA2dpMessageHandler}, TWS_SBC_DECODER, BITFIELD_CAST(8, 0)};
const A2dpPluginTaskdata csr_tws_master_aptx_decoder_plugin = {{AudioPluginA2dpMessageHandler}, TWS_APTX_DECODER, BITFIELD_CAST(8, 0)};
const A2dpPluginTaskdata csr_tws_master_aac_decoder_plugin = {{AudioPluginA2dpMessageHandler}, TWS_AAC_DECODER, BITFIELD_CAST(8, 0)};
const A2dpPluginTaskdata csr_tws_master_aptx_ad_decoder_plugin = {{AudioPluginA2dpMessageHandler}, TWS_APTX_AD_DECODER, BITFIELD_CAST(8, 0)};

const A2dpPluginTaskdata csr_ba_sbc_decoder_plugin = {{AudioPluginA2dpMessageHandler}, BA_SBC_DECODER, BITFIELD_CAST(8, 0)};
const A2dpPluginTaskdata csr_ba_aac_decoder_plugin = {{AudioPluginA2dpMessageHandler}, BA_AAC_DECODER, BITFIELD_CAST(8, 0)};

/****************************************************************************
DESCRIPTION
    Converts global decoder type to local one.
    Result is limited to actually supported decoders.
*/
static audio_codec_t convertDecoderType(A2DP_DECODER_PLUGIN_TYPE_T decoder)
{
    audio_codec_t supported_decoder = audio_codec_sbc;

    switch(decoder)
    {
        case SBC_DECODER:
        case TWS_SBC_DECODER:
        case BA_SBC_DECODER:
            supported_decoder = audio_codec_sbc;
            break;
        case APTX_DECODER:
        case TWS_APTX_DECODER:
            supported_decoder = audio_codec_aptx;
            break;
        case APTX_ACL_SPRINT_DECODER:
            supported_decoder = audio_codec_aptx_ll;
            break;
        case AAC_DECODER:
        case TWS_AAC_DECODER:
        case BA_AAC_DECODER:
            supported_decoder = audio_codec_aac;
            break;
        case APTXHD_DECODER:
            supported_decoder = audio_codec_aptx_hd;
            break;
        case APTX_AD_DECODER:
        case TWS_APTX_AD_DECODER:
            supported_decoder = audio_codec_aptx_adaptive;
            break;
        default:
            /* Decoder is not supported */
            Panic();
    }

    return supported_decoder;
}

audio_codec_t AudioPluginA2dpTaskGetCodec(Task task)
{
    A2DP_DECODER_PLUGIN_TYPE_T decoder = ((A2dpPluginTaskdata*)task)->a2dp_plugin_variant;
    return convertDecoderType(decoder);
}

bool AudioPluginA2dpTaskIsBroadcaster(Task task)
{
    A2DP_DECODER_PLUGIN_TYPE_T decoder = ((A2dpPluginTaskdata*)task)->a2dp_plugin_variant;

    if(decoder == BA_SBC_DECODER || decoder == BA_AAC_DECODER)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
