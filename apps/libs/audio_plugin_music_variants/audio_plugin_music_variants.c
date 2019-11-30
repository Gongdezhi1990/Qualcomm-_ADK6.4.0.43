/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

 
FILE NAME
    audio_plugin_music_variants.c
 
DESCRIPTION
    Definitions of not used music plugin variants.
*/

#include <message.h>
#include <csrtypes.h>
#include <vmtypes.h>
#include <panic.h>

#include "audio_plugin_music_variants.h"

static void dummyMessageHandler(Task task, MessageId id, Message message);

const A2dpPluginTaskdata csr_aptx_ad_decoder_plugin = {{dummyMessageHandler}, APTX_AD_DECODER, BITFIELD_CAST(8, 0)};
const A2dpPluginTaskdata csr_tws_master_aptx_ad_decoder_plugin = {{dummyMessageHandler}, TWS_APTX_AD_DECODER, BITFIELD_CAST(8, 0)};
const A2dpPluginTaskdata csr_tws_slave_aptx_ad_decoder_plugin = {{dummyMessageHandler}, TWS_APTX_AD_DECODER, BITFIELD_CAST(8, 0)};

static void dummyMessageHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(id);
    UNUSED(message);

    Panic();
}

bool AudioPluginMusicVariantsCodecDeterminesTwsEncoding(void)
{
    return TRUE;
}
