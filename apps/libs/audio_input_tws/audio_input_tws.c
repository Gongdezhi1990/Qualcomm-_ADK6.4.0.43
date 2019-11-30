/****************************************************************************
Copyright (c) 2016-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_tws.c

DESCRIPTION
    Message dispatcher.
*/
#include <stdlib.h>

#include <message.h>
#include <audio_plugin_if.h>
#include <audio_plugin_music_variants.h>
#include <audio_input_common.h>
#include <tws_packetiser.h>
#include <vmtypes.h>
#include <print.h>

#include "audio_input_tws.h"
#include "audio_input_tws_connect.h"

static void disconnected(Task task, Message message, audio_input_context_t* ctx);
static void dummyHandler(Task task, Message message, audio_input_context_t* ctx);

static audio_input_context_t *tws_ctx = NULL;

const A2dpPluginTaskdata csr_tws_slave_sbc_decoder_plugin = {{AudioPluginTwsMessageHandler}, TWS_SBC_DECODER, BITFIELD_CAST(8, 0)};
const A2dpPluginTaskdata csr_tws_slave_aac_decoder_plugin = {{AudioPluginTwsMessageHandler}, TWS_AAC_DECODER, BITFIELD_CAST(8, 0)};
const A2dpPluginTaskdata csr_tws_slave_aptx_decoder_plugin = {{AudioPluginTwsMessageHandler}, TWS_APTX_DECODER, BITFIELD_CAST(8, 0)};
const A2dpPluginTaskdata csr_tws_slave_aptx_ad_decoder_plugin = {{AudioPluginTwsMessageHandler}, TWS_APTX_AD_DECODER, BITFIELD_CAST(8, 0)};

static const audio_input_state_table_t tws_state_table = 
{
    {
        [audio_input_idle] = {[audio_input_connect_req] = AudioInputTwsConnectHandler},
        [audio_input_connecting] = {[audio_input_connect_complete] = AudioInputCommonFadeInCompleteHandler, [audio_input_error] = AudioInputCommonFadeOutCompleteHandler},
        [audio_input_connected] = {[audio_input_disconnect_req] = AudioInputCommonDisconnectHandler},
        [audio_input_disconnecting] = {[audio_input_disconnect_complete] = disconnected},
        [audio_input_error_state] = {[audio_input_disconnect_req] = dummyHandler}
    }
};

static void disconnected(Task task, Message message, audio_input_context_t* ctx)
{
    AudioInputTwsFadeOutCompleteHandler(task, message, ctx);
}

static void dummyHandler(Task task, Message message, audio_input_context_t* ctx)
{
    UNUSED(task);
    UNUSED(message);
    UNUSED(ctx);
}

static void setUpStateHandlers(void)
{
    tws_ctx = calloc(1, sizeof(audio_input_context_t));
    AudioInputCommonSetStateTable(&tws_state_table, tws_ctx);
}

static void cleanContextData(void)
{
    free(tws_ctx);
    tws_ctx = NULL;
}

void AudioPluginTwsMessageHandler(Task task, MessageId id, Message message)
{
    PRINT(("audio_input_tws handler id 0x%x\n", (unsigned)id));
    
    switch(id)
    {
        case AUDIO_PLUGIN_CONNECT_MSG:
            PanicNotNull(tws_ctx);
            setUpStateHandlers();
            break;

        case TWS_PACKETISER_FIRST_AUDIO_FRAME_IND:
            AudioMixerFadeIn(tws_ctx->mixer_input, NULL);
            return;

        #ifdef HOSTED_TEST_ENVIRONMENT
        case AUDIO_PLUGIN_TEST_RESET_MSG:
            AudioInputTwsTestReset();
            cleanContextData();
            return;
        #endif

        default:
            break;
    }

    AudioInputCommonMessageHandler(task, id, message, tws_ctx);

    /* Must only free context after transition to state audio_input_idle */
    if (tws_ctx && tws_ctx->state == audio_input_idle)
        cleanContextData();
}
