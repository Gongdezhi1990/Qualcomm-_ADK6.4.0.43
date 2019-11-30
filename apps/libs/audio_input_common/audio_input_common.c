/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_common.c

DESCRIPTION
    Default handlers for non input specific messages.
*/
#include <stdlib.h>
#include <string.h>
#include <print.h>


#include <audio.h>
#include <audio_plugin_if.h>
#include <audio_plugin_forwarding.h>
#include <audio_music_processing.h>
#include <audio_plugin_music_variants.h>
#include <audio_config.h>
#include <audio_mixer.h>

#include "audio_input_common.h"
#include "audio_input_common_music_processing.h"
#include "audio_input_common_states.h"

typedef enum
{
    AUDIO_INPUT_COMMON_ERROR,
    AUDIO_INPUT_COMMON_CONNECT_COMPLETE
} audio_input_common_message_t;

static void audioInputCommonControlMessageHandler(Task task, MessageId id, Message message, audio_input_context_t *ctx)
{
    switch(id)
    {
        case AUDIO_PLUGIN_SET_SOFT_MUTE_MSG:
        {
            const AUDIO_PLUGIN_SET_SOFT_MUTE_MSG_T *msg = (const AUDIO_PLUGIN_SET_SOFT_MUTE_MSG_T *)message;
            PRINT(("AUDIO_PLUGIN_SET_SOFT_MUTE_MSG_T mute states: %d\n", msg->mute_states));
            AudioInputCommonSetSoftMuteHandler(msg, ctx);
            break;
        }

        case AUDIO_PLUGIN_SET_INPUT_AUDIO_MUTE_MSG:
        {
#ifdef DEBUG_PRINT_ENABLED
            const AUDIO_PLUGIN_SET_INPUT_AUDIO_MUTE_MSG_T *msg = (const AUDIO_PLUGIN_SET_INPUT_AUDIO_MUTE_MSG_T *)message;
            PRINT(("AUDIO_PLUGIN_SET_INPUT_AUDIO_MUTE_MSG input audio port mute active: %d\n", msg->input_audio_port_mute_active));
#endif
            break;
        }

        case AUDIO_PLUGIN_SET_GROUP_VOLUME_MSG:
        {
            const AUDIO_PLUGIN_SET_GROUP_VOLUME_MSG_T *msg = (const AUDIO_PLUGIN_SET_GROUP_VOLUME_MSG_T *)message;
            PRINT(("AUDIO_PLUGIN_SET_GROUP_VOLUME_MSG \n"));
            AudioInputCommonSetGroupVolumeHandler(msg);
            break;
        }

        case AUDIO_PLUGIN_PLAY_TONE_MSG:
        {
            const AUDIO_PLUGIN_PLAY_TONE_MSG_T *msg = (const AUDIO_PLUGIN_PLAY_TONE_MSG_T *)message;
            PRINT(("AUDIO_PLUGIN_PLAY_TONE_MSG tone: %p, can queue: %ld, tone volume: %d\n", msg->tone, (const long int) msg->can_queue, msg->tone_volume));
            AudioInputCommonPlayToneHandler(msg);
            break;
        }

        case AUDIO_PLUGIN_STOP_TONE_AND_PROMPT_MSG:
        {
            PRINT(("AUDIO_PLUGIN_STOP_TONE_AND_PROMPT_MSG \n"));
            AudioInputCommonStopToneHandler();
            break;
        }

        case AUDIO_PLUGIN_SET_MODE_MSG:
        {
            const AUDIO_PLUGIN_SET_MODE_MSG_T *msg = (const AUDIO_PLUGIN_SET_MODE_MSG_T *)message;
            AudioInputCommonSetMode(msg->mode, msg->params);
            break;
        }

        case AUDIO_PLUGIN_SET_MUSIC_ENHANCEMENTS_MSG:
        {
            const AUDIO_PLUGIN_SET_MUSIC_ENHANCEMENTS_MSG_T *msg = (const AUDIO_PLUGIN_SET_MUSIC_ENHANCEMENTS_MSG_T *)message;
            audioInputCommonSetMusicProcessing(ctx, msg->music_processing_mode, msg->music_processing_enhancements);
            break;
        }

        case AUDIO_PLUGIN_SET_USER_EQ_PARAMETER_MSG:
        {
            const AUDIO_PLUGIN_SET_USER_EQ_PARAMETER_MSG_T* msg = (const AUDIO_PLUGIN_SET_USER_EQ_PARAMETER_MSG_T *)message;
            AudioInputCommonSetUserEqParameter(&msg->param);
            break;
        }

        case AUDIO_PLUGIN_APPLY_USER_EQ_PARAMETERS_MSG:
        {
            AudioInputCommonApplyUserEqParameters();
            break;
        }

        case AUDIO_PLUGIN_CLEAR_USER_EQ_PARAMETERS_MSG:
        {
            AudioInputCommonClearUserEqParameters();
            break;
        }

        case AUDIO_PLUGIN_GET_USER_EQ_PARAMETER_MSG:
        {
            const AUDIO_PLUGIN_GET_USER_EQ_PARAMETER_MSG_T* msg = (const AUDIO_PLUGIN_GET_USER_EQ_PARAMETER_MSG_T *)message;
            AudioInputCommonGetUserEqParameter(&msg->param_id, task, msg->callback_task);
            break;
        }

        case AUDIO_PLUGIN_GET_USER_EQ_PARAMETERS_MSG:
        {
            const AUDIO_PLUGIN_GET_USER_EQ_PARAMETERS_MSG_T* msg = (const AUDIO_PLUGIN_GET_USER_EQ_PARAMETERS_MSG_T *)message;
            AudioInputCommonGetUserEqParameters(msg->number_of_params, msg->param_ids, task, msg->callback_task);
            break;
        }

        case AUDIO_PLUGIN_GET_USER_EQ_PARAMETER_RESPONSE_MSG:
        {
            const AUDIO_PLUGIN_GET_USER_EQ_PARAMETER_RESPONSE_MSG_T* msg = (const AUDIO_PLUGIN_GET_USER_EQ_PARAMETER_RESPONSE_MSG_T *)message;
            AudioInputCommonSendGetUserEqParameterResponse(msg->data_valid, msg->param);
            break;
        }

        case AUDIO_PLUGIN_GET_USER_EQ_PARAMETERS_RESPONSE_MSG:
        {
            const AUDIO_PLUGIN_GET_USER_EQ_PARAMETERS_RESPONSE_MSG_T* msg = (const AUDIO_PLUGIN_GET_USER_EQ_PARAMETERS_RESPONSE_MSG_T *)message;
            AudioInputCommonSendGetUserEqParametersResponse(msg->data_valid, msg->number_of_params, msg->params);
            break;
        }

        case AUDIO_PLUGIN_SET_CHANNEL_MODE_MSG:
        {
            AudioInputCommonSetTwsChannelMode();
            break;
        }

        default:
            break;
    }
}

void AudioInputCommonMessageHandler(Task task, MessageId id, Message message, audio_input_context_t *ctx)
{
    PRINT(("Audio Input Common Handler: id 0x%x\n", (unsigned)id));

    switch(id)
    {
        case AUDIO_INPUT_COMMON_ERROR:
            AudioInputCommonHandleEvent(task, audio_input_error, message, ctx);
        break;

        case AUDIO_PLUGIN_CONNECT_MSG:
            AudioInputCommonHandleEvent(task, audio_input_connect_req, message, ctx);
        break;

        case AUDIO_MIXER_FADE_IN_CFM:
        case AUDIO_INPUT_COMMON_CONNECT_COMPLETE:
            AudioInputCommonHandleEvent(task, audio_input_connect_complete, message, ctx);
        break;

        case AUDIO_PLUGIN_DISCONNECT_MSG:
            AudioInputCommonHandleEvent(task, audio_input_disconnect_req, message, ctx);
        break;

        case AUDIO_MIXER_FADE_OUT_CFM:
            AudioInputCommonHandleEvent(task, audio_input_disconnect_complete, message, ctx);
        break;

        case AUDIO_PLUGIN_START_FORWARDING_MSG:
            AudioInputCommonHandleEvent(task, audio_input_forward_req, message, ctx);
        break;

        case AUDIO_PLUGIN_FORWARDING_CREATE_CFM:
            AudioInputCommonHandleEvent(task, audio_input_forward_created, message, ctx);
        break;

        case AUDIO_PLUGIN_FORWARDING_START_CFM:
            AudioInputCommonHandleEvent(task, audio_input_forward_started, message, ctx);
        break;

        case AUDIO_PLUGIN_STOP_FORWARDING_MSG:
            AudioInputCommonHandleEvent(task, audio_input_forward_stop_req, message, ctx);
        break;

        case AUDIO_PLUGIN_FORWARDING_STOP_CFM:
            AudioInputCommonHandleEvent(task, audio_input_forward_stopped, message, ctx);
        break;

        case AUDIO_PLUGIN_FORWARDING_DESTROY_CFM:
            AudioInputCommonHandleEvent(task, audio_input_forward_destroyed, message, ctx);
        break;

        default:
            if(ctx->state != audio_input_error_state)
                audioInputCommonControlMessageHandler(task, id, message, ctx);
            break;
    }
}


/****************************************************************************
DESCRIPTION
    Helper function to disconnect a Source, flush buffers then close
*/
static void sourceDisconnectFlushClose(Source source)
{
    if (source)
    {
        /* The calling order of these functions is critical */
        StreamDisconnect(source, 0);

        /* flush buffer */
        StreamConnectDispose(source);

        /* disconnect and close */
        SourceClose(source);
    }
}

/****************************************************************************
DESCRIPTION
    Handler for AUDIO_PLUGIN_DISCONNECT_MSG message.
*/
void AudioInputCommonDisconnectHandler(Task task, Message message, audio_input_context_t* ctx)
{
    UNUSED(message);
    PRINT(("AudioInputCommonDisconnectHandler()\n"));
    SetAudioBusy(task);
    PanicNull(ctx);
    AudioMixerFadeOut(ctx->mixer_input, task);
}

void AudioInputCommonDisconnectChainFromMixer(Task task, Message message, audio_input_context_t *ctx)
{
    UNUSED(task);
    UNUSED(message);
    PRINT(("AudioInputCommonDisconnectChain()\n"));
    PanicNull(ctx);

    if(ctx->chain)
    {
        ChainStop(ctx->chain);
    }

    if (ctx->unpacketiser)
    {
        TransformStop(ctx->unpacketiser);
        ctx->unpacketiser = NULL;
    }

    if(ctx->mixer_input != audio_mixer_input_error_none)
    {
        AudioMixerDisconnect(ctx->mixer_input);
        ctx->mixer_input = audio_mixer_input_error_none;
    }
}

void AudioInputCommonFadeOutCompleteHandler(Task task, Message message, audio_input_context_t *ctx)
{
    PRINT(("AudioInputCommonFadeOutCompleteHandler()\n"));
    PanicNull(ctx);

    AudioInputCommonDisconnectChainFromMixer(task, message, ctx);

    if(ctx->codec_source)
    {
        sourceDisconnectFlushClose(ctx->codec_source);
        ctx->codec_source = 0;
    }
    else
    {
        sourceDisconnectFlushClose(ctx->left_source);
        sourceDisconnectFlushClose(ctx->right_source);
        ctx->left_source = 0;
        ctx->right_source = 0;
    }
    if(ctx->chain)
    {
        ChainDestroy(ctx->chain);
        ctx->chain = 0;
    }
    /*Restore default kick period before disabling framework*/
    OperatorsFrameworkSetKickPeriod(DEFAULT_KICK_PERIOD);
    OperatorsFrameworkDisable();

    SetAudioInUse(FALSE);
    SetCurrentDspStatus(DSP_NOT_LOADED);
    SetAudioBusy(NULL);
}

/****************************************************************************
DESCRIPTION
    Helper function to work out mute state.
*/
static AUDIO_MUTE_STATE_T getMuteState(audio_output_group_t group, uint16 mute_mask)
{
    AUDIO_MUTE_STATE_T state = AUDIO_MUTE_DISABLE;

    PanicFalse(group != audio_output_group_all);

    if(mute_mask & AUDIO_MUTE_MASK(group))
        state = AUDIO_MUTE_ENABLE;

    return state;
}

void AudioInputCommonSetSoftMuteHandler(const AUDIO_PLUGIN_SET_SOFT_MUTE_MSG_T *msg, audio_input_context_t *ctx)
{
    uint16 mute_mask = msg->mute_states;
    AUDIO_MUTE_STATE_T mute_state = getMuteState(audio_output_group_main, mute_mask);
    bool muted = (mute_state == AUDIO_MUTE_ENABLE) ? TRUE : FALSE;

    PRINT(("AudioInputCommonSetSoftMuteHandler() mute %s\n",
            muted ? "enable" : "disable"));

    AudioMixerMuteOutput(muted);

    /* Note: Broadcast mode only - forward the mute to the output plugin */ 
    if (ctx->ba.plugin)
    {
        AudioPluginForwardingSoftMuteInd(ctx->ba.plugin, muted);
    }
}

void AudioInputCommonSetGroupVolumeHandler(const AUDIO_PLUGIN_SET_GROUP_VOLUME_MSG_T *msg)
{
    if(msg->group == audio_output_group_main || msg->group == audio_output_group_all)
    {
        PRINT(("AudioInputCommonSetGroupVolumeHandler main\n"));
        AudioMixerSetVolume();
    }
}

void AudioInputCommonPlayToneHandler(const AUDIO_PLUGIN_PLAY_TONE_MSG_T *msg)
{
    MAKE_AUDIO_MESSAGE(AUDIO_PLUGIN_PLAY_TONE_MSG, new_message);

    memcpy(new_message, msg, sizeof(AUDIO_PLUGIN_PLAY_TONE_MSG_T));

    MessageSend(AudioGetTonePlugin(), AUDIO_PLUGIN_PLAY_TONE_MSG, new_message);

    /* Set audio busy here otherwise other pending tone messages will be sent */
    SetAudioBusy(AudioGetTonePlugin());
}

void AudioInputCommonStopToneHandler(void)
{
    MessageSend(AudioGetTonePlugin(), AUDIO_PLUGIN_STOP_TONE_AND_PROMPT_MSG, NULL);
}

static bool remainMutedAfterConnect(audio_input_context_t *ctx)
{
    return (ctx->tws.mute_until_start_forwarding);
}

void AudioInputCommonConnect(audio_input_context_t *ctx, Task task)
{
    SetAudioInUse(TRUE);
    SetCurrentDspStatus(DSP_RUNNING);

    if(remainMutedAfterConnect(ctx))
    {
        AudioInputCommmonSendConnectComplete(task);
    }
    else
    {
    AudioMixerFadeIn(ctx->mixer_input, task);
}

    SetAudioBusy(task);
    audioInputCommonSetMusicProcessing(ctx, ctx->music_processing, ctx->music_enhancements);
}

void AudioInputCommonFadeInCompleteHandler(Task task, Message message, audio_input_context_t* ctx)
{
    UNUSED(task);
    UNUSED(message);
    UNUSED(ctx);
    SetAudioBusy(NULL);
}

void AudioInputCommonSetMode (const AUDIO_MODE_T mode, const void * params)
{
    UNUSED(mode);
    UNUSED(params);
#ifdef DEBUG_PRINT_ENABLED
     PRINT(("AUDIO_PLUGIN_SET_MODE_MSG mode: %d\n", mode));
#endif
}

void AudioInputCommonSetTwsChannelMode(void)
{
    AudioMixerUpdateChannelMode(AudioConfigGetTwsChannelModeLocal());
}

void AudioInputCommonIgnoreEvent(Task task, Message message, audio_input_context_t* ctx)
{
    UNUSED(task);
    UNUSED(message);
    UNUSED(ctx);
}

void AudioInputCommmonSendError(Task task)
{
    MessageSend(task, AUDIO_INPUT_COMMON_ERROR, NULL);
    SetAudioBusy(task);
}

void AudioInputCommmonSendConnectComplete(Task task)
{
    MessageSend(task, AUDIO_INPUT_COMMON_CONNECT_COMPLETE, NULL);
}

bool AudioInputCommonTaskIsBroadcaster(Task task)
{
    A2DP_DECODER_PLUGIN_TYPE_T decoder = ((A2dpPluginTaskdata*)task)->a2dp_plugin_variant;

    switch (decoder)
    {
    case BA_SBC_DECODER:
    case BA_AAC_DECODER:
    case BA_ANALOGUE_DECODER:
    case BA_USB_DECODER:
        return TRUE;
    default:
        return FALSE;
    }
}


void AudioInputCommonDspPowerOn(void)
{
    OperatorsFrameworkEnable();
    SetCurrentDspStatus(DSP_LOADED_IDLE);
}


unsigned AudioInputCommonGetKickPeriod(void)
{
    bool is_music_low_power = AudioConfigIsMusicLowPower();

    if (is_music_low_power)
        return LOW_POWER_GRAPH_KICK_PERIOD;
    return DEFAULT_KICK_PERIOD;
}

unsigned AudioInputCommonGetKickPeriodFromCodec(audio_codec_t codec)
{
    bool is_music_low_power = AudioConfigIsMusicLowPower();

    switch(codec)
    {
        case audio_codec_sbc:
        case audio_codec_aac:
        case audio_codec_aptx:
            if (is_music_low_power)
                return LOW_POWER_GRAPH_KICK_PERIOD;

            return DEFAULT_KICK_PERIOD;
        default:
            return DEFAULT_KICK_PERIOD;
    }
}
