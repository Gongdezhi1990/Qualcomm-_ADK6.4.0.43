/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_spdif.c

DESCRIPTION
    Message dispatcher.
*/
#include <stdlib.h>

#include <message.h>
#include <audio_plugin_if.h>
#include <audio_plugin_music_variants.h>
#include <audio_input_common.h>
#include <vmtypes.h>
#include <print.h>

#include "audio_input_spdif_private.h"
#include "audio_input_spdif.h"
#include "audio_input_spdif_connect.h"

static audio_input_context_t *ctx = 0;

const A2dpPluginTaskdata csr_spdif_decoder_plugin = {{AudioPluginSpdifMessageHandler}, SPDIF_DECODER, BITFIELD_CAST(8, 0)};
static void cleanContextData(void);



static void AudioPluginSpdifMixerMessageHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(message);

    switch (id)
    {
        case AUDIO_MIXER_FADE_IN_CFM:
        {
            PRINT(("AUDIO_MIXER_FADE_IN_CFM \n"));
            AudioInputCommonFadeInCompleteHandler(task, message, ctx);
            break;
        }

        case AUDIO_MIXER_FADE_OUT_CFM:
        {
            PRINT(("AUDIO_MIXER_FADE_OUT_CFM \n"));
            AudioInputCommonFadeOutCompleteHandler(task, message, ctx);
            cleanContextData();
            break;
        }

        default:
            break;
    }
}

static void AudioPluginSpdifAudioMessageHandler(Task task, MessageId id, Message message)
{
    PRINT(("audio_input_spdif handler id 0x%x\n", (unsigned)id));

    switch(id)
    {
        case AUDIO_PLUGIN_CONNECT_MSG:
        {
            const AUDIO_PLUGIN_CONNECT_MSG_T * connect_message = (const AUDIO_PLUGIN_CONNECT_MSG_T *)message;
            PRINT(("AUDIO_PLUGIN_CONNECT_MSG \n"));
            PanicNotNull(ctx);
            ctx = (audio_input_context_t *)calloc(1, sizeof(*ctx));
            AudioInputSpdifConnectHandler(ctx, task, connect_message);
            break;
        }

        case AUDIO_PLUGIN_DISCONNECT_MSG:
        {
            PRINT(("AUDIO_PLUGIN_DISCONNECT_MSG \n"));
            PanicNull(ctx);
            AudioInputCommonDisconnectHandler(task, message, ctx);
            break;
        }

        default:
        {
            AudioInputCommonMessageHandler(task, id, message, ctx);
            break;
        }
    }
}

void AudioPluginSpdifMessageHandler(Task task, MessageId id, Message message)
{
    if(id >= AUDIO_DOWNSTREAM_MESSAGE_BASE && id < AUDIO_DOWNSTREAM_MESSAGE_TOP)
    {
        AudioPluginSpdifAudioMessageHandler(task, id, message);
    }
    else if (id >= AUDIO_MIXER_MESSAGE_BASE && id < AUDIO_MIXER_MESSAGE_TOP)
    {
        AudioPluginSpdifMixerMessageHandler(task, id, message);
    }
    else
    {
        Panic();
    }
}

static void cleanContextData(void)
{
    free(ctx);
    ctx = 0;
}

audio_input_context_t* AudioInputSpdifGetContext(void)
{
    return ctx;
}
