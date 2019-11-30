/****************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_output_tws_packetiser.c

DESCRIPTION
    TWS functionality
*/
#include <panic.h>
#include <tws_packetiser.h>
#include <audio_plugin_forwarding.h>
#include <audio_config.h>

#include "audio_output_tws.h"
#include "audio_output_tws_task.h"
#include "audio_output_tws_packetiser.h"

/*The last time before the TTP at which a packet may be transmitted*/
#define TWS_TX_DEADLINE_MILLISECONDS (50)

static tws_packetiser_master_t* packetiser;

static tws_packetiser_codec_t getPacketiserCodecFromAudioCodec(audio_codec_t source_codec)
{
    if(source_codec == audio_codec_aptx)
        return TWS_PACKETISER_CODEC_APTX;

    if(source_codec == audio_codec_aac)
        return TWS_PACKETISER_CODEC_AAC;

    if(source_codec == audio_codec_aptx_adaptive)
        return TWS_PACKETISER_CODEC_APTX_AD;

    return TWS_PACKETISER_CODEC_SBC;
}

static tws_packetiser_codec_t getPacketiserCodec(const audio_plugin_forwarding_params_t* params)
{
    if(params->transcode.codec != audio_codec_none)
        return getPacketiserCodecFromAudioCodec(params->transcode.codec);

    return getPacketiserCodecFromAudioCodec(params->source_codec);
}

static tws_packet_master_modes_t getPacketiserMode(Task output_task, tws_packetiser_codec_t codec, bool content_protection)
{
    if(AudioOutputTwsTaskIsTws(output_task))
        return TWS_PACKETISER_MASTER_MODE_TWS;

    if(AudioOutputTwsTaskIsShareMe(output_task) && codec == TWS_PACKETISER_CODEC_APTX && !content_protection)
        return TWS_PACKETISER_MASTER_MODE_NO_HEADER;

    return TWS_PACKETISER_MASTER_MODE_RTP;
}

bool AudioOutputTwsPacketiserInit(Task output_task, Source forwarding_source, const audio_plugin_forwarding_params_t* params)
{
    tws_packetiser_master_config_t config;
    bool content_protection_enabled = params->content_protection;
    tws_packetiser_codec_t codec = getPacketiserCodec(params);
    config.source = forwarding_source;
    config.codec = codec;
    config.sink = params->forwarding_sink;
    config.client = output_task;
    config.mode = getPacketiserMode(output_task, codec, content_protection_enabled);
    config.cp_header_enable = content_protection_enabled;
    config.scmst = params->content_protection ?
                    packetiser_helper_scmst_copy_prohibited :
                    packetiser_helper_scmst_copy_allowed;
    config.mtu = AudioConfigGetPeerMtu();

    config.time_before_ttp_to_tx = (params->ttp_latency.min_in_ms) * US_PER_MS;
    config.tx_deadline = (TWS_TX_DEADLINE_MILLISECONDS) * US_PER_MS;
    
    packetiser = TwsPacketiserMasterInit(&config);

    return (packetiser != NULL);
}

void AudioOutputTwsPacketiserDestroy(void)
{
    if(packetiser)
    {
        TwsPacketiserMasterDestroy(packetiser);
        packetiser = NULL;
    }
}
