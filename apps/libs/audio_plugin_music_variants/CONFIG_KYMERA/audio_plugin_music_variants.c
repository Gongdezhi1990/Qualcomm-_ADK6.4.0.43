/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_plugin_music_variants.c

DESCRIPTION
    Definitions of not used music plugin variants.
*/

#include <message.h>
#include <vmtypes.h>
#include <panic.h>

#include "audio_plugin_music_variants.h"

static void dummyMessageHandler(Task task, MessageId id, Message message);

static void dummyMessageHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(id);
    UNUSED(message);

    Panic();
}

const A2dpPluginTaskdata csr_mp3_decoder_plugin = {{dummyMessageHandler}, MP3_DECODER, BITFIELD_CAST(8, 0)};
const A2dpPluginTaskdata csr_faststream_sink_plugin = {{dummyMessageHandler}, FASTSTREAM_SINK, BITFIELD_CAST(8, 0)};
const A2dpPluginTaskdata csr_tws_master_mp3_decoder_plugin = {{dummyMessageHandler}, TWS_MP3_DECODER, BITFIELD_CAST(8, 0)};
const A2dpPluginTaskdata csr_tws_slave_mp3_decoder_plugin = {{dummyMessageHandler}, TWS_MP3_DECODER, BITFIELD_CAST(8, 0)};
const A2dpPluginTaskdata csr_fm_decoder_plugin = {{dummyMessageHandler}, FM_DECODER, BITFIELD_CAST(8, 0)};

bool AudioPluginMusicVariantsCodecDeterminesTwsEncoding(void)
{
    return FALSE;
}
