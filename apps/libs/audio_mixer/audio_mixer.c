/****************************************************************************
Copyright (c) 2016 - 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_mixer.c
DESCRIPTION
    Library to implement a mixer chain for use with the audio plugins.
NOTES
*/

#include <stdlib.h>
#include <string.h>
#include <print.h>
#include <panic.h>
#include <stream.h>
#include <operator.h>
#include <library.h>

#include "audio_mixer.h"
#include "audio_mixer_input.h"
#include "audio_mixer_resampler.h"
#include "audio_mixer_processing.h"
#include "audio_mixer_speaker.h"
#include "audio_mixer_core.h"
#include "audio_mixer_chain_interconnections.h"
#include "audio_hardware.h"
#include "audio_mixer_channel_roles.h"

#include <audio_music_processing.h>
#include <chain.h>
#include <audio_output.h>
#include <audio_ports.h>
#include <audio_config.h>
#include <audio.h>
#include "audio_i2s_common.h"

/*
 * The value to be used for the mixerInputSetIsHd threshold, which must be
 * exceeded in mixer_data->output_sample_rate or connect_data->sample_rate for
 * mixerInputSetIsHd to be called with TRUE, else it is called with FALSE.
 */
#define HIGH_SAMPLE_RATE_THRESHOLD        48000

typedef struct mixer_data_t
{
    uint32                      output_sample_rate;
    unsigned                    number_of_mics;
} mixer_data_t;

static mixer_data_t * mixer_data = NULL;

/****************************************************************************
DESCRIPTION
    This allocates and initialises resources for the mixer.
*/
static void mixerInit(uint32 sample_rate, unsigned number_of_mics)
{
    if (mixer_data == NULL)
    {
        mixer_data = PanicUnlessNew(mixer_data_t);
        memset(mixer_data, 0, sizeof(mixer_data_t));

        mixer_data->output_sample_rate = sample_rate;
        mixer_data->number_of_mics = number_of_mics;

        mixerInputInit();
    }
}

/****************************************************************************
DESCRIPTION
    This function destroys resources allocated for the mixer.
*/
static void mixerDeInit(void)
{
    free(mixer_data);
    mixer_data = NULL;
}

/****************************************************************************
RETURNS
    The input to use for the given connection type
*/
static audio_mixer_input_t mixerGetInputForConnection(connection_type_t connection_type)
{
    audio_mixer_input_t input;
    PanicNull(mixer_data);

    switch(connection_type)
    {
        case CONNECTION_TYPE_MUSIC:
        case CONNECTION_TYPE_MUSIC_A2DP:
        case CONNECTION_TYPE_MUSIC_BA_TX:
        case CONNECTION_TYPE_MUSIC_BA_RX:
            input = audio_mixer_input_music;
        break;

        case CONNECTION_TYPE_VOICE:
            input = audio_mixer_input_voice;
        break;

        case CONNECTION_TYPE_TONES:
            input = audio_mixer_input_tones;
        break;

        default:
            input = audio_mixer_input_error_none;
            Panic();
        break;
    }

    if(mixerInputIsAvailable(input))
        return input;

    return audio_mixer_input_error_none;
}


/****************************************************************************
DESCRIPTION
    Returns output sample rate which depends on connection type.
*/
static uint32 mixerGetOutputSampleRate(connection_type_t connection_type, uint32 input_sample_rate)
{
    audio_output_params_t mch_params;
    mch_params.disable_resample = FALSE;

    if (AudioConfigGetMaximumConcurrentInputs() > 1)
    {
        mch_params.sample_rate = AudioConfigGetOutputSampleRate();
    }
    else if(connection_type == CONNECTION_TYPE_MUSIC)
    {
        mch_params.sample_rate = AudioConfigGetWiredAudioOutputSampleRate();
    }
    else if(connection_type == CONNECTION_TYPE_VOICE)
    {
        uint16 i2s_voice_resampling_frequency = AudioI2SVoiceResamplingFrequency();

        if(i2s_voice_resampling_frequency == I2S_NO_RESAMPLE)
            mch_params.sample_rate = input_sample_rate;
        else
        {
            mch_params.sample_rate = i2s_voice_resampling_frequency;
            mch_params.disable_resample = TRUE;
        }
    }
    else
    {
        mch_params.sample_rate = input_sample_rate;
    }

    return AudioOutputGetSampleRate(&mch_params, 0);
}

/****************************************************************************
DESCRIPTION
    Create the mixer chain
*/
static void mixerCreateMixerChain(connection_type_t connection_type, AUDIO_MUSIC_CHANNEL_MODE_T channel_mode)
{
    mixerCoreCreate(connection_type, channel_mode, mixer_data->output_sample_rate);
    mixerSpeakerCreate(connection_type, mixer_data->output_sample_rate);
}

/****************************************************************************
DESCRIPTION
Connect to audio hardware, this will connect the audio mixer to audio output and connect and start processing
It will include AEC chain, if applicable
*/
static void mixerHardwareConnect(connection_type_t connection_type)
{
    audio_hardware_connect_t hw_connect_data;

    hw_connect_data.connection_type = connection_type;
    hw_connect_data.output_sample_rate= mixer_data->output_sample_rate;
    hw_connect_data.primary_left = ChainGetOutput(mixerSpeakerGetChain(), mixer_left_channel);
    hw_connect_data.primary_right = ChainGetOutput(mixerSpeakerGetChain(), mixer_right_channel);
    hw_connect_data.secondary_left = ChainGetOutput(mixerSpeakerGetChain(), mixer_secondary_left_channel);
    hw_connect_data.secondary_right = ChainGetOutput(mixerSpeakerGetChain(), mixer_secondary_right_channel);
    hw_connect_data.wired_sub =  ChainGetOutput(mixerSpeakerGetChain(), wired_sub_channel);
    hw_connect_data.aux_left = NULL;
    hw_connect_data.aux_right = NULL;

    AudioHardwareConnectOutput(&hw_connect_data);
}

/****************************************************************************
DESCRIPTION
    Create the mixer chain, configure it and establish internal connections
    between operators.
*/
static void mixerCreateAndConnect(const audio_mixer_connect_t* connect_data)
{
    bool mixer_already_created = (mixer_data != NULL);
    bool mono_path_enabled = (AudioConfigGetRenderingMode()==single_channel_rendering);
    unsigned path_count = (mono_path_enabled)? 1 : 2;

    if(!mixer_already_created)
    {
        mixerInit(mixerGetOutputSampleRate(connect_data->connection_type, connect_data->sample_rate), MAX_NUMBER_OF_MICS);
        OperatorsFrameworkEnable();
        mixerCreateMixerChain(connect_data->connection_type, connect_data->channel_mode);
        ChainJoinMatchingRoles(mixerCoreGetChain(), mixerSpeakerGetChain(), path_count);
        mixerHardwareConnect(connect_data->connection_type);
    }
    else
    {
        if (connect_data->connection_type==CONNECTION_TYPE_VOICE)
        {
            AudioHardwareConfigureMicSampleRate(connect_data->sample_rate);
        }
    }
}

/****************************************************************************
DESCRIPTION
    Destroy the mixer chain
*/
static void mixerDestroyMixerChain(void)
{
    mixerCoreDestroy();
    mixerSpeakerDestroy();
    mixerDeInit();
}

/****************************************************************************
DESCRIPTION
    Starts the resampler chain. If this is only connection to the mixer, it
    also starts the mixer chain.
*/
static void mixerStartProcessingInput(audio_mixer_input_t mixer_input, bool mixer_already_active)
{
    if(!mixer_already_active)
    {
        mixerSpeakerStart();
        mixerCoreStart();
    }
    mixerProcessingStart(mixerInputGetProcessing(mixer_input));
    mixerResamplerStart(mixerInputGetResampler(mixer_input));
}

/****************************************************************************
DESCRIPTION
    Stops the resampler chain, and if this is the last connection to the mixer
    also stops the mixer chain.
*/
static void mixerStopProcessingInput(audio_mixer_input_t mixer_input)
{
    mixerResamplerStop(mixerInputGetResampler(mixer_input));
    mixerProcessingStop(mixerInputGetProcessing(mixer_input));
    if (mixerInputIsLastConnected(mixer_input))
    {
        mixerCoreStop();
        mixerSpeakerStop();
    }
}

/****************************************************************************
DESCRIPTION
    Destroys the resampler chain, and if this is the last connection to the mixer
    also destroys the mixer chain.
*/
static void mixerDestroyChain(audio_mixer_input_t mixer_input)
{
    mixerResamplerDestroy(mixerInputGetResampler(mixer_input));

    mixerProcessingDestroy(mixerInputGetProcessing(mixer_input));
    mixerInputSetResampler(mixer_input, NULL);
    mixerInputSetProcessing(mixer_input, NULL);
    if (mixerInputIsLastConnected(mixer_input))
    {
        /* Must disconnect audio_hardware before destroying last capability (it is possible audio_hardware does not instantiate any capabilities),
        as framework must be enabled when disconnecting audio_output */
        AudioHardwareDisconnectOutput();
        mixerDestroyMixerChain();
        OperatorsFrameworkDisable();
    }
    mixerInputSetIsHd(mixer_input, FALSE);
    mixerInputSetConnected(mixer_input, FALSE);
}

/****************************************************************************
DESCRIPTION
    Stops and destroys all operators associated with the supplied mixer input
    If this is the last connection to the mixer operator then the mixer is also
    stopped, destroyed and the DAC connection closed.
*/
static void mixerDisconnectInput(audio_mixer_input_t mixer_input)
{
    mixerStopProcessingInput(mixer_input);
    mixerDestroyChain(mixer_input);
}

/******************************************************************************/
audio_mixer_input_t AudioMixerConnect(const audio_mixer_connect_t* connect_data)
{
    audio_mixer_input_t mixer_input;
    bool mixer_already_active;

    mixerCreateAndConnect(connect_data);

    /* Must check active before resampler_context is assigned */
    mixer_already_active = mixerInputAnyConnected();
    mixer_input = mixerGetInputForConnection(connect_data->connection_type);

    if (mixer_input != audio_mixer_input_error_none)
    {
        bool is_mono_input = (connect_data->right_src == NULL);
        bool attenuate_volume_pre_processing = mixerSpeakerIncreasesVolumePostProcessing();

        mixer_resampler_context_t resampler_context = mixerResamplerCreate(connect_data,
                                                                           mixer_data->output_sample_rate);

        mixer_processing_context_t processing_context = mixerProcessingCreate(connect_data->connection_type,
                                                                              mixer_data->output_sample_rate,
                                                                              is_mono_input,
                                                                              attenuate_volume_pre_processing);

        mixerInputSetResampler(mixer_input, resampler_context);
        mixerInputSetProcessing(mixer_input, processing_context);

        if(mixerChainConnectInput(mixer_input, connect_data->left_src, connect_data->right_src))
        {
            bool is_input_hd;

            mixerStartProcessingInput(mixer_input, mixer_already_active);
            mixerInputSetConnected(mixer_input, TRUE);

            /*
             * Determine whether or not the connection is HD audio.
             */
            is_input_hd = ((mixer_data->output_sample_rate > HIGH_SAMPLE_RATE_THRESHOLD) ||
                (connect_data->sample_rate > HIGH_SAMPLE_RATE_THRESHOLD));

            mixerInputSetIsHd(mixer_input, is_input_hd);
        }
        else
        {
            mixerDisconnectInput(mixer_input);
            mixer_input = audio_mixer_input_error_none;
        }
    }

    return mixer_input;
}

/******************************************************************************/
audio_mixer_disconnect_response_t AudioMixerDisconnect(audio_mixer_input_t mixer_input)
{
    audio_mixer_disconnect_response_t mixerDisconnectResponse = AUDIO_MIXER_DISCONNECT_ERROR;

    if (mixer_data && mixerInputCanDisconnect(mixer_input))
    {
        mixerDisconnectInput(mixer_input);
        mixerDisconnectResponse = AUDIO_MIXER_DISCONNECT_OK;
    }

    return mixerDisconnectResponse;
}

/******************************************************************************/
void AudioMixerFadeIn(audio_mixer_input_t fade_input, Task calling_task)
{
    mixerInputMute(fade_input, FALSE);

    mixerCoreSetInputGains();

    if (calling_task)
    {
        MessageSendLater(calling_task, AUDIO_MIXER_FADE_IN_CFM, NULL, FADE_OUT_DELAY_MS);
    }
}

/******************************************************************************/
void AudioMixerFadeOut(audio_mixer_input_t fade_input, Task calling_task)
{
    mixerInputMute(fade_input, TRUE);

    mixerCoreSetInputGains();

    if (calling_task)
    {
        MessageSendLater(calling_task, AUDIO_MIXER_FADE_OUT_CFM, NULL, FADE_OUT_DELAY_MS);
    }
}

/******************************************************************************/
void AudioMixerMuteOutput(bool enable)
{
    mixerSpeakerMuteOutput(enable);
}

/******************************************************************************/
void AudioMixerSetVolume(void)
{
    mixerSpeakerSetVolume();
}

/******************************************************************************/
void AudioMixerSetInputSampleRate(audio_mixer_input_t input, unsigned sample_rate)
{
    PanicNull(mixer_data);

    mixerInputChangeSampleRate(input, sample_rate, mixer_data->output_sample_rate);
}

/******************************************************************************/
void AudioMixerUpdateChannelMode(AUDIO_MUSIC_CHANNEL_MODE_T channel_mode)
{
    mixerCoreConfigureChannelMode(channel_mode, mixer_data->output_sample_rate);
    mixerCoreSetInputGains();
}

/******************************************************************************/
uint32 AudioMixerGetOutputSampleRate(connection_type_t connection_type, uint32 input_sample_rate)
{
    return mixerGetOutputSampleRate(connection_type, input_sample_rate);
}

/******************************************************************************/
#ifdef HOSTED_TEST_ENVIRONMENT
void AudioMixerTestReset(void)
{
    if (mixer_data)
    {
        audio_mixer_input_t i;

        AudioMusicProcessingTestReset();

        if(!mixerInputAnyConnected())
        {
            mixerDeInit();
            return;
        }

        for (i = audio_mixer_input_first; i <= audio_mixer_input_last; i++)
        {
            if (mixerInputIsConnected(i))
            {
                mixerDisconnectInput(i);
            }
        }
    }
}
#endif
