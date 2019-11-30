/****************************************************************************
Copyright (c) 2016-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_a2dp.c

DESCRIPTION
    Message dispatcher.
*/

#include "audio_input_a2dp.h"
#include "audio_input_a2dp_context.h"
#include "audio_input_a2dp_connect.h"
#include "audio_input_a2dp_rtp_config.h"
#include "audio_input_a2dp_chain_config.h"
#include "audio_input_a2dp_forward.h"
#include "audio_input_a2dp_broadcast.h"
#include "audio_input_a2dp_task.h"
#include <audio.h>
#include <audio_plugin_if.h>
#include <audio_input_common.h>
#include <audio_plugin_forwarding.h>
#include <print.h>

#define AUDIO_INPUT_COMMON_CONNECT_COMPLETE     1

#ifdef HOSTED_TEST_ENVIRONMENT
static void testResetLibrary(void)
{
    AudioMixerTestReset();
    audioInputA2dpDeleteContextData();
}
#endif

static void AudioPluginA2dpMixerMessageHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(message);

    audio_input_context_t * ctx = audioInputA2dpGetContextData();

    switch (id)
    {
        case AUDIO_MIXER_FADE_OUT_CFM:
        {
            AudioInputCommonFadeOutCompleteHandler(task, message, ctx);
            audioInputA2dpDeleteContextData();
            break;
        }
        
        case AUDIO_MIXER_FADE_IN_CFM:
        {
            AudioInputCommonFadeInCompleteHandler(task, message, ctx);
            break;
        }
        
        default:
            break;
    }
}

static void AudioPluginA2dpConnectCompleteHandler(void)
{
    SetAudioBusy(NULL);
}

static void AudioPluginA2dpAudioMessageHandler(Task task, MessageId id, Message message)
{
    audio_input_context_t * ctx = audioInputA2dpGetContextData();

    switch(id)
    {
        case AUDIO_PLUGIN_CONNECT_MSG:
        {
            const AUDIO_PLUGIN_CONNECT_MSG_T * connect_message = (const AUDIO_PLUGIN_CONNECT_MSG_T *) message;
            audioInputA2dpCreateContextData();
            ctx = audioInputA2dpGetContextData();
            PRINT(("AUDIO_PLUGIN_CONNECT_MSG \n"));
            audioInputA2dpConnectHandler(ctx, task, connect_message);
            break;
        }

        case AUDIO_PLUGIN_DISCONNECT_MSG:
        {
            PanicNull(ctx);
            ctx->state = audio_input_disconnecting;
            
            PRINT(("AUDIO_PLUGIN_DISCONNECT_MSG \n"));

            if(ctx->tws.plugin)
            {
                audioInputA2dpForwardDestroy(task, ctx);
            }
            else if(AudioPluginA2dpTaskIsBroadcaster(task))
            {
                SetAudioBusy(task);
                AudioPluginForwardingStop(task, ctx->ba.plugin);
            }
            else
            {
                AudioInputCommonDisconnectHandler(task, message, ctx);
            }
            break;
        }

        #ifdef HOSTED_TEST_ENVIRONMENT
        case AUDIO_PLUGIN_TEST_RESET_MSG:
        {
            testResetLibrary();
            break;
        }
        #endif

        case AUDIO_PLUGIN_START_FORWARDING_MSG:
        {
            const AUDIO_PLUGIN_START_FORWARDING_MSG_T* msg = (const AUDIO_PLUGIN_START_FORWARDING_MSG_T *) message;
            audioInputA2dpForwardCreate(task, msg, ctx);
            break;
        }
        
        case AUDIO_PLUGIN_STOP_FORWARDING_MSG:
        {
            audioInputA2dpForwardDestroy(task, ctx);
            break;
        }

        case AUDIO_PLUGIN_SET_GROUP_VOLUME_MSG:
        {
            if(AudioPluginA2dpTaskIsBroadcaster(task))
            {
                AudioPluginForwardingVolumeChangeInd(ctx->ba.plugin);
            }
            AudioInputCommonMessageHandler(task, id, message, ctx);
            break;
        }

        default:
        {
            AudioInputCommonMessageHandler(task, id, message, ctx);
            break;
        }
    }
}

static void AudioPluginA2dpHandleForwardingCreateCfm(Task task, Message message)
{
    audio_input_context_t * ctx = audioInputA2dpGetContextData();

    if(AudioPluginA2dpTaskIsBroadcaster(task))
    {
        audioInputA2dpBroadcastStart(task, ctx, (const AUDIO_PLUGIN_FORWARDING_CREATE_CFM_T *) message);
    }
    else
    {
        audioInputA2dpForwardStart(task, ctx, (const AUDIO_PLUGIN_FORWARDING_CREATE_CFM_T *) message);
    }
}

static void AudioPluginA2dpHandleForwardingStartCfm(Task task)
{
    audio_input_context_t * ctx = audioInputA2dpGetContextData();

    if(AudioPluginA2dpTaskIsBroadcaster(task))
    {
        /* Disconnect A2DP source as close to start as possible */
        StreamDisconnect(ctx->left_source, 0);
        StreamConnect(ctx->left_source, ChainGetInput(ctx->chain, path_input));
        ChainStart(ctx->chain);
        AudioInputCommonConnect(ctx, task);
    }
    else
    {
        AudioPluginA2dpForwardEnableForwardingOutput(task, ctx);
    }
}

static void disconnectWithoutFadeOut(void)
{
    audio_input_context_t * ctx = audioInputA2dpGetContextData();

    PRINT(("audio_input_a2dp: disconnectWithoutFadeOut()\n"));
    AudioInputCommonDisconnectHandler(NULL, NULL, ctx);
    AudioInputCommonFadeOutCompleteHandler(NULL, NULL, ctx);
    audioInputA2dpDeleteContextData();
}

static void AudioPluginA2dpForwardHandler(Task task, MessageId id, Message message)
{
    audio_input_context_t * ctx = audioInputA2dpGetContextData();

    switch(id)
    {
        case AUDIO_PLUGIN_FORWARDING_CREATE_CFM:
            AudioPluginA2dpHandleForwardingCreateCfm(task, message);
            break;
        
        case AUDIO_PLUGIN_FORWARDING_START_CFM:
            AudioPluginA2dpHandleForwardingStartCfm(task);
            break;

        case AUDIO_PLUGIN_FORWARDING_STOP_CFM:
            if(AudioPluginA2dpTaskIsBroadcaster(task))
            {
                audioInputA2dpBroadcastDestroy(task, ctx->ba.plugin, ctx);
            }
            break;

        case AUDIO_PLUGIN_FORWARDING_STREAMING_IND:
            AudioMixerFadeIn(ctx->mixer_input, NULL);
            break;
        
        case AUDIO_PLUGIN_FORWARDING_DESTROY_CFM:
            if(AudioPluginA2dpTaskIsBroadcaster(task))
            {
                if(ctx->state == audio_input_disconnecting)
                {
                    disconnectWithoutFadeOut();
                }
            }
            else
            {
                if(ctx->state == audio_input_disconnecting)
                {
                    AudioInputCommonDisconnectHandler(task, message, ctx);
                }
                else
                {
                    AudioPluginA2dpForwardHandleDestroyCfm(ctx);
                }
            }
            break;
        
        default:
            break;
    }
}

void AudioPluginA2dpMessageHandler(Task task, MessageId id, Message message)
{
    PRINT(("audio_input_a2dp handler id 0x%x\n", (unsigned)id));
    
    if(id >= AUDIO_DOWNSTREAM_MESSAGE_BASE && id < AUDIO_DOWNSTREAM_MESSAGE_TOP)
    {
        AudioPluginA2dpAudioMessageHandler(task, id, message);
    }
    else if (id >= AUDIO_MIXER_MESSAGE_BASE && id < AUDIO_MIXER_MESSAGE_TOP)
    {
        AudioPluginA2dpMixerMessageHandler(task, id, message);
    }
    else if(id >= AUDIO_PLUGIN_FORWARDING_BASE && id < AUDIO_PLUGIN_FORWARDING_TOP)
    {
        AudioPluginA2dpForwardHandler(task, id, message);
    }
    else if(id == AUDIO_INPUT_COMMON_CONNECT_COMPLETE)
    {
        AudioPluginA2dpConnectCompleteHandler();
    }
    else
    {
        Panic();
    }
}

bool AudioGetLatencyInTenthsOfMilliseconds(Task audio_plugin, bool *estimated, uint16 *latency)
{
    audio_input_context_t * ctx = audioInputA2dpGetContextData();

    *latency = (uint16)(audioInputA2dpGetLatency(AudioPluginA2dpTaskGetCodec(audio_plugin), ctx, estimated) * 10);

    return TRUE;
}
