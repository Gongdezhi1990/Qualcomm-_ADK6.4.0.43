/****************************************************************************
Copyright (c) 2016-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_usb.c

DESCRIPTION
    Message dispatcher.
*/

#include <stdlib.h>
#include <audio.h>
#include <audio_plugin_if.h>
#include <audio_input_common.h>
#include <audio_plugin_music_variants.h>
#include <vmtypes.h>
#include <print.h>

#include "audio_input_usb.h"
#include "audio_input_usb_broadcast.h"
#include "audio_input_usb_connect.h"

static void handleForwardingCreate(Task task, Message message, audio_input_context_t* ctx);
static void handleForwardingStart(Task task, Message message, audio_input_context_t* ctx);
static void handleForwardingDestroy(Task task, Message message, audio_input_context_t* ctx);
static void handleDisconnectReq(Task task, Message message, audio_input_context_t* ctx);
static void disconnected(Task task, Message message, audio_input_context_t* ctx);
static void dummyHandler(Task task, Message message, audio_input_context_t* ctx);

static audio_input_context_t* usb_ctx = NULL;

const A2dpPluginTaskdata csr_usb_decoder_plugin = {{AudioPluginUsbMessageHandler}, USB_DECODER, BITFIELD_CAST(8, 0)};
const A2dpPluginTaskdata csr_ba_usb_decoder_plugin = {{AudioPluginUsbMessageHandler}, BA_USB_DECODER, BITFIELD_CAST(8, 0)};

static const audio_input_state_table_t usb_state_table =
{
    {
        [audio_input_idle] = {[audio_input_connect_req] = AudioInputUsbConnectHandler},
        [audio_input_connecting] = {[audio_input_connect_complete] = AudioInputCommonFadeInCompleteHandler, [audio_input_forward_created] = handleForwardingCreate, [audio_input_forward_started] = handleForwardingStart},
        [audio_input_connected] = {[audio_input_disconnect_req] = handleDisconnectReq, [audio_input_forward_req] = AudioInputCommonForwardStart},
        [audio_input_disconnecting] = {[audio_input_disconnect_complete] = disconnected, [audio_input_forward_stopped] = handleForwardingDestroy, [audio_input_forward_destroyed] = disconnected},
        [audio_input_forwarding_setup] = {[audio_input_forward_created] = handleForwardingCreate, [audio_input_forward_started] = handleForwardingStart, [audio_input_connect_complete] = AudioInputCommonFadeInCompleteHandler, [audio_input_error] = AudioInputCommonFadeOutCompleteHandler},
        [audio_input_forwarding] = {[audio_input_forward_stop_req] = AudioInputCommonForwardStop, [audio_input_disconnect_req] = handleDisconnectReq},
        [audio_input_forwarding_tear_down] = {[audio_input_forward_stopped] = handleForwardingDestroy, [audio_input_forward_destroyed] = audioInputUsbConnectToMixer, [audio_input_connect_complete] = AudioInputCommonFadeInCompleteHandler},
        [audio_input_forwarding_disconnect] = {[audio_input_disconnect_complete] = AudioInputCommonForwardStop, [audio_input_forward_stopped] = handleForwardingDestroy, [audio_input_forward_destroyed] = disconnected},
        [audio_input_error_state] = {[audio_input_disconnect_req] = dummyHandler}
    }
};

static void dummyHandler(Task task, Message message, audio_input_context_t* ctx)
{
    UNUSED(task);
    UNUSED(message);
    UNUSED(ctx);
}

static void started(Task task, Message message, audio_input_context_t* ctx)
{
    ChainStart(ctx->chain);
    AudioInputCommonForwardHandleStartCfm(task, message, ctx);
    audioInputUsbEnableForwardingMonoOutput(ctx);
}

static void disconnected(Task task, Message message, audio_input_context_t* ctx)
{
    AudioInputCommonFadeOutCompleteHandler(task, message, ctx);
}

static void handleForwardingCreate(Task task, Message message, audio_input_context_t* ctx)
{
    if (AudioInputCommonTaskIsBroadcaster(task))
    {
        audioInputUsbBroadcastStart(task, message, ctx);
    }
    else
    {
        AudioInputCommonForwardHandleCreateCfm(task, message, ctx);
    }
}

static void handleForwardingStart(Task task, Message message, audio_input_context_t* ctx)
{
    if (AudioInputCommonTaskIsBroadcaster(task))
    {
        audioInputUsbBroadcastStartChain(task, message, ctx);
    }
    else
    {
        started(task, message, ctx);
    }
}

static void handleForwardingDestroy(Task task, Message message, audio_input_context_t* ctx)
{
    if (AudioInputCommonTaskIsBroadcaster(task))
    {
        audioInputUsbBroadcastDestroy(task, ctx->ba.plugin, ctx);
    }
    else
    {
        AudioInputCommonForwardDestroy(task, message, ctx);
        audioInputUsbDisableForwardingMonoOutput(ctx);
    }
}

static void handleDisconnectReq(Task task, Message message, audio_input_context_t* ctx)
{
    if (AudioInputCommonTaskIsBroadcaster(task))
    {
        audioInputUsbBroadcastStop(task, ctx);
    }
    else
    {
        AudioInputCommonDisconnectHandler(task, message, ctx);
    }
}

static void setUpStateHandlers(void)
{
    usb_ctx = calloc(1, sizeof(audio_input_context_t));
    AudioInputCommonSetStateTable(&usb_state_table, usb_ctx);
}

static void cleanContextData(void)
{
    free(usb_ctx);
    usb_ctx = NULL;
}

void AudioPluginUsbMessageHandler(Task task, MessageId id, Message message)
{
    switch(id)
    {
        case AUDIO_PLUGIN_CONNECT_MSG:
            PanicNotNull(usb_ctx);
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
                AudioPluginForwardingVolumeChangeInd(usb_ctx->ba.plugin);
            }
            break;

        default:
            break;
    }

    AudioInputCommonMessageHandler(task, id, message, usb_ctx);

    /* Must only free context after transition to state audio_input_idle */
    if (usb_ctx && usb_ctx->state == audio_input_idle)
        cleanContextData();
}

uint32 AudioGetUsbSampleRate(void)
{
    if(usb_ctx)
        return usb_ctx->sample_rate;

    return 0;
}
