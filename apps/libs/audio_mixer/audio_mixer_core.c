/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_mixer_core.c

DESCRIPTION
       Implementation of core mixer components chain management.

       The mixer capability has 3 inputs. These are configured as follows:
       1. Audio (Mono/Stereo as required)
       2. Voice (Mono)
       3. Downmixing (Stereo or no connection)

       To facilitate channel mode switching (This does not apply to voice only operation) the source/sync and mixer are used in
       various configurations as described below.

       There are 5 permanent connections between the source/sync and mixer. It is not possible to dynamically change this without
       stopping the entire input chain.

       There are 3 input groups to the source/sync. Audio (stereo), Voice (Mono), Dummy (Stereo).
       The dummy input group always has no connection, and is used to ensure silence flows through the source/sync.

       When in left/right channel mode, the source/sync capability of used to dispose of the unwanted input.
       i.e. The unwanted input does not have its data routed to an output.

       When downmixing, we require to mix the left and right channels together. This is achieved by routing
       the source sync left input to the right channel of the downmix inputs on the mixer, and similarly for the
       right channel input. The mixer is then used to combine the left and right channels together, providing
       downmixed signals on both the left and right channels.

*/

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <audio_music_processing.h>
#include <audio_plugin_ucid.h>
#include <audio_ports.h>
#include <operators.h>
#include <chain.h>
#include <panic.h>
#include <rtime.h>
#include <audio_config.h>

#include "audio_mixer_core.h"
#include "audio_mixer_core_chain_config.h"
#include "audio_mixer_gain.h"
#include "audio_mixer_input.h"
#include "audio_mixer_connection.h"

#define NUM_VOICE_CHANNELS 1
#define NUM_UNUSED_CHANNELS 1
#define DEFAULT_FIRST_CHANNEL 0
#define DEFAULT_PRIMARY_STREAM 1

#define SOURCE_SINK_DOWNMIX_LEFT_CHANNEL (3)
#define SOURCE_SINK_DOWNMIX_RIGHT_CHANNEL (4)

#define MIXER_DOWNMIX_LEFT_CHANNEL (3)
#define MIXER_DOWNMIX_RIGHT_CHANNEL (4)


typedef struct mixer_core_data
{
    kymera_chain_handle_t      chain;
    AUDIO_MUSIC_CHANNEL_MODE_T channel_mode;
} mixer_core_data_t;

static mixer_core_data_t core_data =
{
    .chain = NULL,
    .channel_mode = CHANNEL_MODE_STEREO
};

/****************************************************************************
DESCRIPTION
    Configures the source sync capability
*/
static void mixerCoreConfigureSourceSync(connection_type_t connection_type, uint32 output_sample_rate, bool mono_path_enabled)
{
    uint16 num_sink_groups;
    uint16 num_source_groups;
    uint16 num_routes;
    Operator source_sync_op;
    static const source_sync_sink_group_t* sink_groups;
    static const source_sync_source_group_t* source_groups;
    source_sync_route_t* routes;

    source_sync_op = ChainGetOperatorByRole(mixerCoreGetChain(), sync_role);

    if(mono_path_enabled)
    {
        sink_groups = mixerCoreGetMonoPathSourceSyncSinkGroup(connection_type, &num_sink_groups);
        source_groups = mixerCoreGetMonoPathSourceSyncSourceGroup(connection_type, &num_source_groups);
        routes = mixerCoreGetMonoPathSourceSyncRoutes(connection_type, &num_routes, output_sample_rate);
    }
    else
    {
        sink_groups = mixerCoreGetSourceSyncSinkGroup(connection_type, &num_sink_groups);
        source_groups = mixerCoreGetSourceSyncSourceGroup(connection_type, &num_source_groups);
        routes = mixerCoreGetSourceSyncRoutes(connection_type, core_data.channel_mode, &num_routes, output_sample_rate);
    }

    if(source_sync_op)
    {
        OperatorsStandardSetUCID(source_sync_op, ucid_source_sync_core);
        OperatorsSourceSyncSetSinkGroups(source_sync_op, num_sink_groups, sink_groups);
        OperatorsSourceSyncSetSourceGroups(source_sync_op, num_source_groups, source_groups);
        OperatorsSourceSyncSetRoutes(source_sync_op, num_routes, routes);
    }
    
    free(routes);
}

/****************************************************************************
DESCRIPTION
    Configures the mixer capability
*/
static void mixerCoreConfigure(connection_type_t connection_type, uint32 output_sample_rate, bool mono_path_enabled)
{
    Operator mixer_op;

    mixer_op = ChainGetOperatorByRole(core_data.chain, mixer_role);
    if(mixer_op)
    {
        unsigned num_music_channels = mixerCoreGetNumberOfMusicChannels(connection_type, mono_path_enabled);
        unsigned num_downmix_channels = mixerCoreGetNumberOfDownmixChannels(connection_type, mono_path_enabled);
        unsigned number_of_samples_to_ramp = (output_sample_rate * FADE_OUT_DELAY_MS)/ MS_PER_SEC;

        OperatorsMixerSetChannelsPerStream(mixer_op, num_music_channels, NUM_VOICE_CHANNELS, num_downmix_channels);
        OperatorsMixerSetPrimaryStream(mixer_op, DEFAULT_PRIMARY_STREAM);
        OperatorsMixerSetNumberOfSamplesToRamp(mixer_op, number_of_samples_to_ramp);
    }

    ChainConfigureSampleRate(core_data.chain, output_sample_rate, NULL, 0);
}

/******************************************************************************/
void mixerCoreCreate(connection_type_t connection_type, AUDIO_MUSIC_CHANNEL_MODE_T channel_mode, uint32 output_sample_rate)
{
    bool mono_path_enabled = (AudioConfigGetRenderingMode()==single_channel_rendering);
    const chain_config_t* config = mixerCoreGetChainConfig(connection_type, mono_path_enabled);

    core_data.chain = PanicNull(ChainCreate(config));
    core_data.channel_mode = channel_mode;

    mixerCoreConfigure(connection_type, output_sample_rate, mono_path_enabled);
    mixerCoreConfigureSourceSync(connection_type, output_sample_rate, mono_path_enabled);
    mixerCoreSetInputGains();

    ChainConnect(mixerCoreGetChain());
}

/******************************************************************************/
void mixerCoreDestroy(void)
{
    if (core_data.chain != NULL)
    {
        ChainDestroy(core_data.chain);
        core_data.chain = NULL;
    }
}

/******************************************************************************/
kymera_chain_handle_t mixerCoreGetChain(void)
{
    return core_data.chain;
}

/******************************************************************************/
void mixerCoreStart(void)
{
    ChainStart(core_data.chain);
}

/******************************************************************************/
void mixerCoreStop(void)
{
    ChainStop(core_data.chain);
}

/****************************************************************************
DESCRIPTION
    Return a bool indicating if channel mode is downmixing
*/
static bool mixerCoreChannelModeIsDownMix(void)
{
    return (core_data.channel_mode == CHANNEL_MODE_DOWNMIX) ? TRUE : FALSE;
}

/******************************************************************************/
void mixerCoreSetInputGains(void)
{
    Operator mixer_op = ChainGetOperatorByRole(mixerCoreGetChain(), mixer_role);

    if(mixer_op)
    {
        bool music_muted = mixerInputIsMuted(audio_mixer_input_music);
        bool voice_muted = mixerInputIsMuted(audio_mixer_input_voice);
        bool downmixing = mixerCoreChannelModeIsDownMix();

        int music_gain = music_muted ? GAIN_MIN : GAIN_UNITY;
        int voice_gain = voice_muted ? GAIN_MIN : GAIN_UNITY;
        int downmix_gain;

        if(!music_muted && !voice_muted)
        {
            voice_gain = GAIN_FOREGROUND;
            music_gain = GAIN_BACKGROUND;
        }

        downmix_gain = downmixing ? music_gain : GAIN_MIN;

        OperatorsMixerSetGains(mixer_op, music_gain, voice_gain, downmix_gain);
    }
}

/******************************************************************************/
void mixerCoreConfigureChannelMode(AUDIO_MUSIC_CHANNEL_MODE_T channel_mode, uint32 sample_rate)
{
    bool mono_path_enabled = (AudioConfigGetRenderingMode()==single_channel_rendering);

    if(!mono_path_enabled)
    {
        source_sync_route_t* routes;
        uint16 num_routes;
        Operator source_sync_op;

        core_data.channel_mode = channel_mode;
        routes = mixerCoreGetSourceSyncChannelModeRoutes(channel_mode, &num_routes, sample_rate);

        source_sync_op = ChainGetOperatorByRole(mixerCoreGetChain(), sync_role);
        if(source_sync_op)
        {
            OperatorsSourceSyncSetRoutes(source_sync_op, num_routes, routes);
        }

        free(routes);
    }
}
