/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_voice_common_connect.c

DESCRIPTION
    Audio voice common connect/disconnect implementation.
*/

#include <stdlib.h>

#include <audio.h>
#include <print.h>
#include "../audio_voice_hfp/audio_voice_hfp.h"
#include "audio_voice_common.h"
#include "audio_voice_common_dsp.h"

#define MSBC_SCO_FRAME_LENGTH   (60)
#define SCO_METADATA_ENABLE     (1)

/*******************************************************************************
DESCRIPTION
    Get the correct audio sink from the AUDIO_PLUGIN_CONNECT_MSG_T. This may
    either be the USB sink or the SCO sink, depending on connection type.
*/
static Sink getAudioSinkFromConnectMessage(AUDIO_PLUGIN_CONNECT_MSG_T* connect_msg)
{
    if(connect_msg->sink_type == AUDIO_SINK_USB)
    {
        hfp_common_plugin_params_t* params = connect_msg->params;
        return params->usb_params.usb_sink;
    }

    return connect_msg->audio_sink;
}

/*******************************************************************************
DESCRIPTION
    Determine if codec is 2 mic bandwidth extension type
*/
static bool is2MicBex(audio_voice_context_t* ctx)
{
    bool is_2_mic_bex = FALSE;
    switch(ctx->variant)
    {
        case cvc_2_mic_headset_cvsd_bex:
        case cvc_2_mic_handsfree_cvsd_bex:
            is_2_mic_bex = TRUE;
            break;
        default:
            is_2_mic_bex = FALSE;
            break;
    }
    return is_2_mic_bex;
}

/*******************************************************************************
DESCRIPTION
    Determine the input sample rate.
    This is based off mic_sample_rate_16kHz, except for 2 mic bex variants that use the
    narrow band rate
*/
static uint32 getInputSampleRate(audio_voice_context_t* ctx)
{
    uint32 rate = NBS_SAMPLE_RATE;
    if (is2MicBex(ctx))
    {
        rate = NBS_SAMPLE_RATE;
    }
    else
    {
        switch(AudioVoiceCommonGetPluginType(ctx->variant))
        {
            case wide_band:
                rate = WBS_SAMPLE_RATE;
                break;
            case ultra_wide_band:
                rate = UWBS_SAMPLE_RATE;
                break;
            case super_wide_band:
                rate = SWBS_SAMPLE_RATE;
                break;
            case narrow_band:
            default:
                rate = NBS_SAMPLE_RATE;
                break;
        }
    }
    return rate;
}

/******************************************************************************/
void AudioVoiceCommonConnectAndFadeIn(Task task, Message msg, audio_voice_context_t* ctx, operator_filters_t* filters)
{
    AUDIO_PLUGIN_CONNECT_MSG_T* connect_msg =  (AUDIO_PLUGIN_CONNECT_MSG_T*) msg;

    hfp_common_plugin_params_t* params = (hfp_common_plugin_params_t*)connect_msg->params;

    /* Signal that the audio is busy until the parameters are fully loaded so that no tone messages etc. will arrive */
    SetAudioBusy(task);

    PanicNull(params);

    ctx->mic_params    = params->voice_mic_params;
    ctx->audio_sink    = PanicNull(getAudioSinkFromConnectMessage(connect_msg));
    ctx->audio_source  = StreamSourceFromSink(connect_msg->audio_sink);
    ctx->mode          = connect_msg->mode;
    ctx->mic_muted     = FALSE;
    ctx->speaker_muted = FALSE;

    PRINT(("CVC: Connect, audio sink[%p], audio source[%p]\n", (int *)ctx->audio_sink, (int *)ctx->audio_source));

    ctx->incoming_sample_rate = getInputSampleRate(ctx);

    SetAudioInUse(TRUE);
    SetCurrentDspStatus(DSP_LOADED_IDLE);

    AudioVoiceCommonDspCreateChainAndFadeIn(task, ctx, filters);
}

/******************************************************************************/
void AudioVoiceCommonConnectAndFadeInError(Task task, Message msg, audio_voice_context_t* ctx)
{
    UNUSED(task);
    UNUSED(msg);
    AudioVoiceCommonDspDestroyChain(ctx);
    AudioVoiceCommonDspPowerOff();
    SetAudioBusy(NULL);
    SetCurrentDspStatus(DSP_RUNNING);
}

/******************************************************************************/
void AudioVoiceCommonConnectAndFadeInSuccess(Task task, Message msg, audio_voice_context_t* ctx)
{
    UNUSED(task);
    UNUSED(msg);
    UNUSED(ctx);
    SetAudioBusy(NULL);
    SetCurrentDspStatus(DSP_RUNNING);
}

/******************************************************************************/
void AudioVoiceCommonDisconnectFadeOut(Task task, Message msg, audio_voice_context_t* ctx)
{
    UNUSED(msg);
    UNUSED(ctx);
    SetAudioBusy(task);
    AudioVoiceCommonDspFadeOut(task);
}

/******************************************************************************/
void AudioVoiceCommonDisconnectDestroyChainAndPlugin(Task task, Message msg, audio_voice_context_t* ctx)
{
    UNUSED(msg);

    AudioVoiceCommonDspDestroyChain(ctx);

    PRINT(("CVC: Disconnected\n"));

    /* Cancel any firmware messages */
    MessageCancelAll(task, MESSAGE_STREAM_DISCONNECT);

    /* Wait until microphone bias has been turned off before powering down DSP */
    AudioVoiceCommonDspPowerOff();
    AudioVoiceCommonDestroyPlugin(task, msg, ctx);
}

/******************************************************************************/
void AudioVoiceCommonDestroyPlugin(Task task, Message msg, audio_voice_context_t* ctx)
{
    UNUSED(task);
    UNUSED(msg);
    UNUSED(ctx);

    SetCurrentDspStatus(DSP_NOT_LOADED);
    SetAudioInUse(FALSE);
    SetAudioBusy(NULL);
}

