/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_voice_hfp.c

DESCRIPTION
    Implementation of HFP audio input plug-in.
*/

#include "audio_voice_hfp.h"

#include <audio.h>
#include <stdlib.h>
#include <audio_voice_common.h>

#include "audio_voice_hfp_connect.h"

static audio_voice_context_t* hfp_ctx = 0;


static const audio_voice_state_table_t hfp_state_table =
{
    {
        [audio_voice_not_loaded] = {
                                        [audio_voice_connect_req] = {AudioVoiceHfpConnectAndFadeIn, audio_voice_connecting}
                                   },
        [audio_voice_connecting] = {
                                        [audio_voice_connect_complete] = {AudioVoiceCommonConnectAndFadeInSuccess, audio_voice_connected},
                                        [audio_voice_error] = {AudioVoiceCommonConnectAndFadeInError, audio_voice_error_state}
                                   },
        [audio_voice_connected] = {
                                       [audio_voice_disconnect_req] = {AudioVoiceCommonDisconnectFadeOut, audio_voice_disconnecting},
                                       [audio_voice_play_tone] = {AudioVoiceCommonTonePlay, audio_voice_connected},
                                       [audio_voice_stop_tone] = {AudioVoiceCommonToneStop, audio_voice_connected},
                                       [audio_voice_set_volume] = {AudioVoiceCommonVolumeSet, audio_voice_connected},
                                       [audio_voice_reset_volume] = {AudioVoiceCommonVolumeSet, audio_voice_connected},
                                       [audio_voice_set_soft_mute] = {AudioVoiceCommonVolumeSetSoftMute, audio_voice_connected}
                                  },
        [audio_voice_disconnecting] = {
                                       [audio_voice_disconnect_complete] = {AudioVoiceCommonDisconnectDestroyChainAndPlugin, audio_voice_not_loaded}
                                      },
        [audio_voice_error_state] = {
                                       [audio_voice_disconnect_req] = {AudioVoiceCommonDestroyPlugin, audio_voice_not_loaded},
                                       [audio_voice_play_tone] = {AudioVoiceCommonDummyStateHander, audio_voice_error_state},
                                       [audio_voice_stop_tone] = {AudioVoiceCommonDummyStateHander, audio_voice_error_state},
                                       [audio_voice_set_volume] = {AudioVoiceCommonDummyStateHander, audio_voice_error_state},
                                       [audio_voice_reset_volume] = {AudioVoiceCommonDummyStateHander, audio_voice_error_state},
                                       [audio_voice_set_soft_mute] = {AudioVoiceCommonDummyStateHander, audio_voice_error_state}
                                    }
    }
};


static void cleanContextData(void)
{
    free(hfp_ctx);
    hfp_ctx = 0;
}

static void setUpStateHandlers(void)
{
    PanicNotNull(hfp_ctx);
    hfp_ctx = AudioVoiceCommonCreateContext();
    AudioVoiceCommonSetStateTable(&hfp_state_table, hfp_ctx);
}

/****************************************************************************
DESCRIPTION
    The main task message handler.
*/
void AudioVoiceHfpMessageHandler(Task task, MessageId id, Message message)
{
    switch(id)
    {
        case AUDIO_PLUGIN_CONNECT_MSG:
        {
            setUpStateHandlers();
            break;
        }

#ifdef HOSTED_TEST_ENVIRONMENT
        case AUDIO_PLUGIN_TEST_RESET_MSG:
        {
            cleanContextData();
            return;
        }
#endif

        default:
        {
            break;
        }
    }

    AudioVoiceCommonMessageHandler(task, id, message, hfp_ctx);

    /* Must only free context after transition to state audio_voice_not_loaded */
    if (hfp_ctx && hfp_ctx->state == audio_voice_not_loaded)
    {
        cleanContextData();
    }
}
