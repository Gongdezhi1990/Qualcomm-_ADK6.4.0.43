/****************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_input_broadcast.c

DESCRIPTION
    Broadcast audio plugin when the device is in the receiver role.
*/

#include <stdlib.h>

#include <audio.h>
#include <audio_plugin_if.h>
#include <audio_plugin_music_variants.h>
#include <audio_input_common.h>
#include <broadcast_cmd.h>
#include <broadcast_context.h>
#include <broadcast_packetiser.h>
#include <broadcast_stream_service_record.h>
#include <message.h>
#include <print.h>
#include <scm.h>
#include <vmtypes.h>

#include "audio_input_broadcast.h"
#include "audio_input_broadcast_connect.h"
#include "audio_input_broadcast_private.h"
#include "audio_input_broadcast_packetiser.h"

static void dummyHandler(Task task, Message message, audio_input_context_t* ctx);

static ba_receiver_plugin_context_t *ba_ctx;

/* The externally available plugin object. */
const A2dpPluginTaskdata csr_ba_receiver_decoder_plugin = {{AudioInputBroadcastMessageHandler}, BA_CELT_DECODER, BITFIELD_CAST(8, 0)};

static const audio_input_state_table_t ba_state_table =
{
    {
        [audio_input_idle] = {[audio_input_connect_req] = audioInputBroadcastConnectHandler},
        [audio_input_connecting] = {[audio_input_connect_complete] = AudioInputCommonFadeInCompleteHandler, [audio_input_error] = AudioInputCommonFadeOutCompleteHandler},
        [audio_input_connected] = {[audio_input_disconnect_req] = AudioInputCommonDisconnectHandler},
        [audio_input_error_state] = {[audio_input_disconnect_req] = dummyHandler}
    }
};

static void createBroadcastReceiverContext(Message message)
{
    const AUDIO_PLUGIN_CONNECT_MSG_T *msg = (const AUDIO_PLUGIN_CONNECT_MSG_T *)message;

    ba_ctx = calloc(1, sizeof(*ba_ctx));
    PanicNull(ba_ctx);
    ba_ctx->app_task = msg->app_task;
    ba_ctx->received_sample_rate = rtime_sample_rate_unknown;
}

static void cleanContextData(void)
{
    free(ba_ctx);
    ba_ctx = NULL;
}

ba_receiver_plugin_context_t *audioInputBroadcastGetContext(void)
{
    return ba_ctx;
}

static void disconnected(Task task, Message message, audio_input_context_t* ctx)
{
    UNUSED(task);
    UNUSED(message);
    UNUSED(ctx);

    BroadcastCmdScmDisable();
}

static void dummyHandler(Task task, Message message, audio_input_context_t* ctx)
{
    UNUSED(task);
    UNUSED(message);
    UNUSED(ctx);
}

static void audioInputBroadcastScmDisableCfmHandler(Task task, Message message, audio_input_context_t *ctx)
{
    /* Confirmation that the scm library has been disabled.
       It is now ok to destroy the broadcast_packetiser. */
    audioInputBroadcastFadeOutCompleteHandler(task, message, ctx);

    cleanContextData();
}

static void setUpStateHandlers(void)
{
    AudioInputCommonSetStateTable(&ba_state_table, &ba_ctx->audio_ctx);
}

static void assertWeAreInThisState(audio_input_state_t current_state)
{
    PanicFalse(ba_ctx && ba_ctx->audio_ctx.state == current_state);
}

void AudioInputBroadcastMessageHandler(Task task, MessageId id, Message message)
{
    switch(id)
    {
        case AUDIO_PLUGIN_CONNECT_MSG:
            PRINT(("BA RX: AUDIO_PLUGIN_CONNECT_MSG\n"));
            PanicNotNull(ba_ctx);
            createBroadcastReceiverContext(message);
            setUpStateHandlers();
            break;

        case AUDIO_PLUGIN_DISCONNECT_MSG:
            PRINT(("BA RX: AUDIO_PLUGIN_DISCONNECT_MSG\n"));
            break;

        case AUDIO_PLUGIN_SET_GROUP_VOLUME_MSG:
            /* Volume message should be used as volume offset on the broadcast receiver.
               It is not implemented yet. */
            break;

        #ifdef HOSTED_TEST_ENVIRONMENT
        case AUDIO_PLUGIN_TEST_RESET_MSG:
            audioInputBroadcastTestReset();
            cleanContextData();
            return;
        #endif

        case AUDIO_MIXER_FADE_OUT_CFM:
            PRINT(("BA RX: AUDIO_MIXER_FADE_OUT_CFM\n"));
            assertWeAreInThisState(audio_input_disconnecting);
            disconnected(task, message, &ba_ctx->audio_ctx);
            return;

        case BROADCAST_CMD_DISABLE_CFM:
            PRINT(("BROADCAST_CMD_DISABLE_CFM\n"));
            assertWeAreInThisState(audio_input_disconnecting);
            /* ba_ctx will not be valid after this is called. */
            audioInputBroadcastScmDisableCfmHandler(task, message, &ba_ctx->audio_ctx);
            return;

        default:
            if (audioInputBroadcastPacketiserIsBroadcastPacketiserMessageId(id))
            {
                audioInputBroadcastPacketiserHandleMessage(task, id, message, &ba_ctx->audio_ctx);
                return;
            }
            break;
    }

    PanicNull(ba_ctx);
    AudioInputCommonMessageHandler(task, id, message, &ba_ctx->audio_ctx);

    /* Must only free context after transition to state audio_input_idle */
    if (ba_ctx && ba_ctx->audio_ctx.state == audio_input_idle)
        cleanContextData();
}
