/****************************************************************************
Copyright (c) 2017-2019 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_output_broadcast.c

DESCRIPTION
    Audio output broadcast implementation.
*/
#include <string.h>

#include <message.h>
#include <stream.h>
#include <print.h>
#include <panic.h>

#include <vmtypes.h>
#include <audio_plugin_forwarding.h>
#include <audio_music_processing.h>
#include <broadcast_packetiser.h>
#include <broadcast_context.h>
#include <gain_utils.h>
#include <rtime.h>
#include <audio_config.h>
#include <gain_utils.h>
#include <operators.h>

#include "audio_plugin_output_variants.h"
#include "audio_output_broadcast_chain_config.h"
#include "audio_output_broadcast.h"

typedef struct
{
    Task input_task;
    audio_plugin_forwarding_ports_t fw_ports;
    kymera_chain_handle_t chain;
} ba_output_ctx_t;

static ba_output_ctx_t ctx;

static TaskData broadcaster_plugin = {AudioOutputBroadcastMessageHandler};

static unsigned splitter_buffer_size = SPLITTER_BUFFER_SIZE;
static unsigned broadcast_pass_buffer_size = BROADCAST_BUFFER_SIZE;
static unsigned local_pass_buffer_size = LOCAL_BUFFER_SIZE;
static unsigned pre_processing_pass_buffer_size = PRE_PROCESSING_BUFFER_SIZE;

#ifdef TUNE_BUFFERS

void SetSplitterBufferSize(unsigned size);

void SetSplitterBufferSize(unsigned size)
{
    splitter_buffer_size = size;
}

void SetBroadcastPassBufferSize(unsigned size);

void SetBroadcastPassBufferSize(unsigned size)
{
    broadcast_pass_buffer_size = size;
}

void SetLocalPassBufferSize(unsigned size);

void SetLocalPassBufferSize(unsigned size)
{
    local_pass_buffer_size = size;
}

void SetPreProcessingPassBufferSize(unsigned size);

void SetPreProcessingPassBufferSize(unsigned size)
{
    pre_processing_pass_buffer_size = size;
}

#endif

Task AudioPluginOutputGetBroadcastPluginTask(void)
{
    return &broadcaster_plugin;
}

Task AudioPluginOutputGetTwsPluginTask(audio_plugin_output_mode_t mode)
{
    UNUSED(mode);

    return (Task)NULL;
}

/****************************************************************************
DESCRIPTION
    Converts sample rate from Hz to rtime constants used by the packetiser.
*/
static rtime_sample_rate_t convertSampleRate(unsigned sample_rate)
{
    if(sample_rate == 44100)
    {
        return rtime_sample_rate_44100;
    }
    else if(sample_rate == 48000)
    {
        return rtime_sample_rate_48000;
    }
    else
    {
        Panic();
        return rtime_sample_rate_unknown;
    }
}

/****************************************************************************
DESCRIPTION
    Helper function to set the UCID of the passthrough, compander, mixer,
    vse and dbe
*/
static void musicProcessingSetUcid(kymera_chain_handle_t chain)
{
    Operator pass_op;
    Operator compander_op;
    Operator vse_op;
    Operator dbe_op;
    Operator peq_op;

    pass_op = ChainGetOperatorByRole(chain, pre_processing_role);
    if (pass_op)
    {
        OperatorsStandardSetUCID(pass_op, ucid_passthrough_processing);
    }

    compander_op = ChainGetOperatorByRole(chain, compander_role);
    if (compander_op)
    {
        OperatorsStandardSetUCID(compander_op, ucid_compander_processing);
    }

    vse_op = ChainGetOperatorByRole(chain, vse_role);
    if (vse_op)
    {
        OperatorsStandardSetUCID(vse_op, ucid_vse_processing);
    }

    dbe_op = ChainGetOperatorByRole(chain, dbe_role);
    if (dbe_op)
    {
        OperatorsStandardSetUCID(dbe_op, ucid_dbe_processing);
    }

    peq_op = ChainGetOperatorByRole(chain, peq_role);
    if (peq_op)
    {
        OperatorsStandardSetUCID(peq_op, ucid_peq_resampler_0);
    }
}

/****************************************************************************
DESCRIPTION
    Helper function to register music processing operators with music processing lib
*/
static void musicProcessingRegisterOperators(kymera_chain_handle_t chain)
{
    Operator compander_op = ChainGetOperatorByRole(chain, compander_role);
    Operator peq_op = ChainGetOperatorByRole(chain, peq_role);
    Operator vse_op = ChainGetOperatorByRole(chain, vse_role);
    Operator dbe_op = ChainGetOperatorByRole(chain, dbe_role);

    AudioMusicProcessingRegisterRole(audio_music_processing_compander_role, compander_op);
    AudioMusicProcessingRegisterRole(audio_music_processing_user_peq_role, peq_op);
    AudioMusicProcessingRegisterRole(audio_music_processing_volume_spatial_enhancement_role, vse_op);
    AudioMusicProcessingRegisterRole(audio_music_processing_dynamic_bass_enhancement_role, dbe_op);
}

/****************************************************************************
DESCRIPTION
    Unregister music processing operators from music processing lib
*/
static void musicProcessingUnregisterOperators(void)
{
    AudioMusicProcessingUnregisterRole(audio_music_processing_compander_role);
    AudioMusicProcessingUnregisterRole(audio_music_processing_user_peq_role);
    AudioMusicProcessingUnregisterRole(audio_music_processing_volume_spatial_enhancement_role);
    AudioMusicProcessingUnregisterRole(audio_music_processing_dynamic_bass_enhancement_role);
}

/****************************************************************************
DESCRIPTION
    Removes -12dB headroom. Headroom exists to avoid clipping in music processing operators.
    Headroom is cancelled in cancel_headroom_role and
    then restored in local_buffer_role.
*/
static void cancelHeadroomBetweenOperators(kymera_chain_handle_t chain)
{
    Operator op;

    op = ChainGetOperatorByRole(chain, cancel_headroom_role);
    OperatorsStandardSetUCID(op, ucid_passthrough_speaker);

    if(!AudioConfigIsMusicLowPower())
    {
        op = ChainGetOperatorByRole(chain, local_buffer_role);
        OperatorsStandardSetUCID(op, ucid_passthrough_processing);
    }
}

static uint16 getCeltFrameSize(const unsigned sample_rate, const erasure_coding_scheme_t ec_scheme)
{
    uint16 frame_size;

    if(ec_scheme == EC_SCHEME_2_5)
    {
        frame_size = sample_rate == 44100 ? CELT_CODEC_FRAME_SIZE_44100HZ : CELT_CODEC_FRAME_SIZE_48KHZ;
    }
    else
    {
        frame_size = sample_rate == 44100 ? CELT_CODEC_FRAME_SIZE_44100HZ_EC3_9 : CELT_CODEC_FRAME_SIZE_48KHZ_EC3_9;
    }

    return frame_size;
}

/****************************************************************************
DESCRIPTION
    Create Kymera chain.
*/
static kymera_chain_handle_t createChain(unsigned sample_rate)
{
    celt_encoder_params_t celt_params;
    Operator op;
    kymera_chain_handle_t chain = audioOutputBroadcastCreateChain();

    celt_params.sample_rate = sample_rate;
    celt_params.frame_size = getCeltFrameSize(sample_rate, BroadcastContextGetEcScheme());
    PRINT(("CELT frame size %d\n", celt_params.frame_size));

    musicProcessingSetUcid(chain);
    musicProcessingRegisterOperators(chain);

    cancelHeadroomBetweenOperators(chain);

    op = ChainGetOperatorByRole(chain, encoder_role);
    OperatorsCeltEncoderSetEncoderParams(op, &celt_params);

    op = ChainGetOperatorByRole(chain, splitter_role);
    PRINT(("splitter buffer size is %d\n", splitter_buffer_size));

    OperatorsConfigureSplitterWithSram(op, splitter_buffer_size, TRUE, operator_data_format_encoded,FALSE);

    op = ChainGetOperatorByRole(chain, decoder_role);
    OperatorsCeltEncoderSetEncoderParams(op, &celt_params);
    
    op = ChainGetOperatorByRole(chain, broadcast_buffer_role);
    OperatorsSetPassthroughDataFormat(op, operator_data_format_encoded);
    PRINT(("broadcast pass buffer size is %d\n", broadcast_pass_buffer_size));
    OperatorsStandardSetBufferSizeWithFormat(op, broadcast_pass_buffer_size, operator_data_format_encoded);

    op = ChainGetOperatorByRole(chain, local_buffer_role);
    OperatorsSetPassthroughDataFormat(op, operator_data_format_pcm);
    PRINT(("local pass buffer size is %d\n", local_pass_buffer_size));
    OperatorsStandardSetBufferSizeWithFormat(op, local_pass_buffer_size, operator_data_format_pcm);

    op = ChainGetOperatorByRole(chain, pre_processing_role);
    PRINT(("pre-processing pass buffer size is %d\n", pre_processing_pass_buffer_size));
    OperatorsStandardSetBufferSizeWithFormat(op, pre_processing_pass_buffer_size, operator_data_format_pcm);

    op = ChainGetOperatorByRole(chain, peq_role);
    OperatorsStandardSetSampleRate(op, sample_rate);

    ChainConnect(chain);

    return chain;
}

static uint16 getCeltSamplesPerFrame(const unsigned sample_rate)
{
    uint16 samples_per_frame;

    if(sample_rate == 44100)
    {
        samples_per_frame = CELT_CODEC_FRAME_SAMPLES_44100HZ;
    }
    else
    {
        samples_per_frame = CELT_CODEC_FRAME_SAMPLES_48KHZ;
    }

    return samples_per_frame;
}

/****************************************************************************
DESCRIPTION
    Update packetiser's audio related settings.
*/
static void updatePacketiserAudioSettings(Source audio_source, uint32 sample_rate)
{
    broadcast_packetiser_t *bp = BroadcastContextGetBpTxHandle();

    BroadcastPacketiserSetSampleRate(bp, convertSampleRate(sample_rate));
    BroadcastPacketiserSetCodecParameters(bp,
            getCeltFrameSize(sample_rate, BroadcastContextGetEcScheme()),
            getCeltSamplesPerFrame(sample_rate));

    BroadcastPacketiserSetSource(bp, audio_source);

    BroadcastPacketiserSetVolume(bp, 0);
}


/****************************************************************************
DESCRIPTION
    Create handler.
*/
static void handleCreate(Message message)
{
    const AUDIO_PLUGIN_FORWARDING_CREATE_REQ_T *msg = (const AUDIO_PLUGIN_FORWARDING_CREATE_REQ_T *)message;

    memset(&ctx, 0, sizeof(ctx));

    ctx.chain = createChain(msg->params.sample_rate);

    ctx.fw_ports.left_source = ChainGetOutput(ctx.chain, left_decoded_audio_output);
    ctx.fw_ports.right_source = ChainGetOutput(ctx.chain, right_decoded_audio_ouput);

    ctx.input_task = msg->input_task;

    StreamConnect(msg->params.forwarding.pcm.left_source, ChainGetInput(ctx.chain, left_decoded_audio_input));
    StreamConnect(msg->params.forwarding.pcm.right_source, ChainGetInput(ctx.chain, right_decoded_audio_input));

    PanicFalse(SourceMapInit(ChainGetOutput(ctx.chain, broadcast_encoded_audio_output),
            STREAM_TIMESTAMPED, AUDIO_FRAME_METADATA_LENGTH));

    updatePacketiserAudioSettings(ChainGetOutput(ctx.chain, broadcast_encoded_audio_output),
            msg->params.sample_rate);

    AudioPluginForwardingCreateCfm(AudioPluginOutputGetBroadcastPluginTask(),
                    ctx.input_task, &ctx.fw_ports, audio_output_success);
}


/****************************************************************************
DESCRIPTION
    Start handler.
*/
static void handleStart(Message message)
{
    const AUDIO_PLUGIN_FORWARDING_START_REQ_T *msg = (const AUDIO_PLUGIN_FORWARDING_START_REQ_T *)message;

    ChainStart(ctx.chain);

    AudioPluginForwardingStartCfm(AudioPluginOutputGetBroadcastPluginTask(), msg->input_task);
}

/****************************************************************************
DESCRIPTION
    Stop handler.
*/
static void handleStop(Message message)
{
    const AUDIO_PLUGIN_FORWARDING_STOP_REQ_T *msg = (const AUDIO_PLUGIN_FORWARDING_STOP_REQ_T *)message;

    ChainStop(ctx.chain);

    AudioPluginForwardingStopCfm(AudioPluginOutputGetBroadcastPluginTask(), msg->input_task);
}

/****************************************************************************
DESCRIPTION
    Destroy handler.
*/
static void handleDestory(Message message)
{
    const AUDIO_PLUGIN_FORWARDING_DESTROY_REQ_T *msg = (const AUDIO_PLUGIN_FORWARDING_DESTROY_REQ_T *)message;

    if(BroadcastContextGetBpTxHandle())
    {
        BroadcastPacketiserSetSource(BroadcastContextGetBpTxHandle(), NULL);
    }

    SourceUnmap(ChainGetOutput(ctx.chain, broadcast_encoded_audio_output));

    musicProcessingUnregisterOperators();

    if(ctx.chain)
    {
       ChainDestroy(ctx.chain);
       ctx.chain = 0;
    }

    AudioPluginForwardingDestroyCfm(AudioPluginOutputGetBroadcastPluginTask(), msg->input_task);
}

/****************************************************************************
DESCRIPTION
    Volume change indication handler.
*/
static void handleVolumeChange(void)
{
    int16 volume_in_db = (int16)AudioConfigGetMasterVolume();
    uint16 volume_in_ba_steps = ConvertdBToBroadcastVolume(volume_in_db, BroadcastContextGetVolumeTable());
    PRINT(("volume update dB %d, steps %d\n", volume_in_db, volume_in_ba_steps));
    BroadcastPacketiserSetVolume(BroadcastContextGetBpTxHandle(), volume_in_ba_steps);
}

/****************************************************************************
DESCRIPTION
    Returns operator which performs broadcast mute function.
*/
static Operator getMuteOperator(void)
{
    return ChainGetOperatorByRole(ctx.chain, cancel_headroom_role);
}

/****************************************************************************
DESCRIPTION
    Mute status indication handler.
*/
static void handleMute(Message message)
{
    const AUDIO_PLUGIN_FORWARDING_SOFT_MUTE_IND_T *msg = (const AUDIO_PLUGIN_FORWARDING_SOFT_MUTE_IND_T *)message;

    PRINT(("Setting broadcast mute %d\n", msg->muted));

    OperatorsStandardFadeOut(getMuteOperator(), msg->muted);
}

/****************************************************************************
DESCRIPTION
    Returns true when message was sent by an audio input plugin.
*/
static bool isItAudioForwardingMessage(MessageId id)
{
    if(id >= AUDIO_PLUGIN_FORWARDING_BASE && id < AUDIO_PLUGIN_FORWARDING_TOP)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/****************************************************************************
DESCRIPTION
    Handler for messages form an audio input plugin.
*/
static void handleAudioForwardingMessages(MessageId id, Message message)
{
    switch(id)
    {
    case AUDIO_PLUGIN_FORWARDING_CREATE_REQ:
        handleCreate(message);
        break;

    case AUDIO_PLUGIN_FORWARDING_START_REQ:
        handleStart(message);
        break;

    case AUDIO_PLUGIN_FORWARDING_STOP_REQ:
        handleStop(message);
        break;

    case AUDIO_PLUGIN_FORWARDING_DESTROY_REQ:
        handleDestory(message);
        break;

    case AUDIO_PLUGIN_FORWARDING_VOLUME_CHANGE_IND:
        handleVolumeChange();
        break;

    case AUDIO_PLUGIN_FORWARDING_SOFT_MUTE_IND:
        handleMute(message);
        break;

    default:
        PRINT(("Unhandled audio forwarding msg id 0x%x\n", id));
        break;
    }
}

void AudioOutputBroadcastMessageHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);

    if(isItAudioForwardingMessage(id))
    {
        handleAudioForwardingMessages(id, message);
    }

}
