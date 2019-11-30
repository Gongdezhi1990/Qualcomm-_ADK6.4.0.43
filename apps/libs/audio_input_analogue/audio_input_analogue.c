/****************************************************************************
Copyright (c) 2016-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_analogue.c

DESCRIPTION
    Message dispatcher.
*/
#include <stdlib.h>

#include <audio_plugin_if.h>
#include <audio_plugin_music_variants.h>
#include <audio_input_common.h>
#include <audio_plugin_forwarding.h>

#include "audio_input_analogue.h"
#include "audio_input_analogue_broadcast.h"
#include "audio_input_analogue_connect.h"

static void dummyHandler(Task task, Message message, audio_input_context_t* ctx);
static void disconnected(Task task, Message message, audio_input_context_t* ctx);
static void start(Task task, Message message, audio_input_context_t* ctx);
static void audioInputAnalogueHandleForwardingCreate(Task task, Message message, audio_input_context_t* ctx);
static void audioInputAnalogueHandleForwardingStart(Task task, Message message, audio_input_context_t* ctx);
static void audioInputAnalogueHandleForwardingDestroy(Task task, Message message, audio_input_context_t* ctx);
static void audioInputAnalogueHandleDisconnectReq(Task task, Message message, audio_input_context_t* ctx);

static audio_input_context_t *ana_ctx = NULL;

const A2dpPluginTaskdata csr_analogue_decoder_plugin = {{AudioInputAnalogueMessageHandler}, ANALOG_DECODER, BITFIELD_CAST(8, 0)};
const A2dpPluginTaskdata csr_ba_analogue_decoder_plugin = {{AudioInputAnalogueMessageHandler}, BA_ANALOGUE_DECODER, BITFIELD_CAST(8, 0)};

static const audio_input_state_table_t ana_state_table =
{
    {
        [audio_input_idle] = {[audio_input_connect_req] = AudioInputAnalogueConnectHandler},
        [audio_input_connecting] = {[audio_input_connect_complete] = AudioInputCommonFadeInCompleteHandler, [audio_input_forward_created] = audioInputAnalogueHandleForwardingCreate, [audio_input_forward_started] = audioInputAnalogueHandleForwardingStart},
        [audio_input_connected] = {[audio_input_disconnect_req] = audioInputAnalogueHandleDisconnectReq, [audio_input_forward_req] = start},
        [audio_input_disconnecting] = {[audio_input_disconnect_complete] = disconnected, [audio_input_forward_stopped] = audioInputAnalogueHandleForwardingDestroy, [audio_input_forward_destroyed] = disconnected},
        [audio_input_forwarding_setup] = {[audio_input_forward_created] = audioInputAnalogueHandleForwardingCreate, [audio_input_forward_started] = audioInputAnalogueHandleForwardingStart, [audio_input_connect_complete] = AudioInputCommonFadeInCompleteHandler, [audio_input_error] = AudioInputCommonFadeOutCompleteHandler},
        [audio_input_forwarding] = {[audio_input_forward_stop_req] = AudioInputCommonForwardStop, [audio_input_disconnect_req] = audioInputAnalogueHandleDisconnectReq},
        [audio_input_forwarding_tear_down] = {[audio_input_forward_stopped] = audioInputAnalogueHandleForwardingDestroy, [audio_input_forward_destroyed] = AudioInputAnalogueConnect, [audio_input_connect_complete] = AudioInputCommonFadeInCompleteHandler},
        [audio_input_forwarding_disconnect] = {[audio_input_disconnect_complete] = AudioInputCommonForwardStop, [audio_input_forward_stopped] = audioInputAnalogueHandleForwardingDestroy, [audio_input_forward_destroyed] = disconnected},
        [audio_input_error_state] = {[audio_input_disconnect_req] = dummyHandler}
    }
};

static void cleanContextData(void)
{
    free(ana_ctx);
    ana_ctx = NULL;
}

static void start(Task task, Message message, audio_input_context_t* ctx)
{
    AudioInputCommonForwardStart(task, message, ctx);
    ChainDestroy(ctx->chain);
    ctx->chain = NULL;
}

static void audioInputAnalogueHandleForwardingCreate(Task task, Message message, audio_input_context_t* ctx)
{
    if (AudioInputCommonTaskIsBroadcaster(task))
    {
        audioInputAnalogueBroadcastStart(task, message, ctx);
    }
    else
    {
        AudioInputCommonForwardHandleCreateCfm(task, message, ctx);
    }
}

static void audioInputAnalogueHandleForwardingStart(Task task, Message message, audio_input_context_t* ctx)
{
    if (AudioInputCommonTaskIsBroadcaster(task))
    {
        audioInputAnalogueBroadcastStartChain(task, message, ctx);
    }
    else
    {
        AudioInputCommonForwardHandleStartCfm(task, message, ctx);
    }
}

static void disconnected(Task task, Message message, audio_input_context_t* ctx)
{
    AudioInputCommonFadeOutCompleteHandler(task, message, ctx);
}

static void dummyHandler(Task task, Message message, audio_input_context_t* ctx)
{
    UNUSED(task);
    UNUSED(message);
    UNUSED(ctx);
}

static void audioInputAnalogueHandleForwardingDestroy(Task task, Message message, audio_input_context_t* ctx)
{
    if (AudioInputCommonTaskIsBroadcaster(task))
    {
        audioInputAnalogueBroadcastDestroy(task, ctx->ba.plugin, ctx);
    }
    else
    {
        AudioInputCommonForwardDestroy(task, message, ctx);
    }
}

static void audioInputAnalogueHandleDisconnectReq(Task task, Message message, audio_input_context_t* ctx)
{
    if (AudioInputCommonTaskIsBroadcaster(task))
    {
        audioInputAnalogueBroadcastStop(task, ctx);
    }
    else
    {
        AudioInputCommonDisconnectHandler(task, message, ctx);
    }
}

static void setUpStateHandlers(void)
{
    ana_ctx = calloc(1, sizeof(audio_input_context_t));
    AudioInputCommonSetStateTable(&ana_state_table, ana_ctx);
}

void AudioInputAnalogueMessageHandler(Task task, MessageId id, Message message)
{
    switch(id)
    {
        case AUDIO_PLUGIN_CONNECT_MSG:
            PanicNotNull(ana_ctx);
            setUpStateHandlers();
            break;

        #ifdef HOSTED_TEST_ENVIRONMENT
        case AUDIO_PLUGIN_TEST_RESET_MSG:
            cleanContextData();
            return;
        #endif

        case AUDIO_PLUGIN_SET_GROUP_VOLUME_MSG:
            if (AudioInputCommonTaskIsBroadcaster(task))
            {
                /* Broadcast the volume change to all receivers. */
                AudioPluginForwardingVolumeChangeInd(ana_ctx->ba.plugin);
            }
            break;

        default:
            break;
    }

    AudioInputCommonMessageHandler(task, id, message, ana_ctx);

    /* Must only free context after transition to state audio_input_idle */
    if (ana_ctx && ana_ctx->state == audio_input_idle)
        cleanContextData();
}
