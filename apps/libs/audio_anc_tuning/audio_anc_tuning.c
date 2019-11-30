/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_anc_tuning.c

DESCRIPTION
    Message dispatcher.
*/

#include "audio_anc_tuning.h"
#include "audio_anc_tuning_chain_config.h"

#include <stdlib.h>
#include <audio.h>
#include <audio_plugin_if.h>
#include <audio_plugin_common.h>
#include <anc.h>
#include <panic.h>
#include <operators.h>
#include <vmal.h>

#define MONITOR_MIC_GAIN                0
#define ZERO_RAW_GAIN                   0x8020

#define NUMBER_OF_AUDIO_INSTANCES       3
#define MONITOR_MIC_ANALOGUE_INSTANCE   AUDIO_INSTANCE_0
#define MONITOR_MIC_DIGITAL_INSTANCE    AUDIO_INSTANCE_1

#define SETUCID_UCID                    0

#define TUNING_CAPABILITY_MESSAGE_ID	0x0001

typedef struct
{
    uint16 id;
    uint16 is_stereo;
    uint16 is_two_mic;
} anc_tuning_set_config_msg_t;

static void AudioAncTuningMessageHandler(Task task, MessageId id, Message message);

const A2dpPluginTaskdata anc_tuning_plugin = {{AudioAncTuningMessageHandler}, 0, BITFIELD_CAST(8, 0)};

static kymera_chain_handle_t tuning_chain = NULL;
static anc_mic_params_t anc_mic_params;

static bool isAncConfigStereo(void)
{
    return (anc_mic_params.enabled_mics == hybrid_mode || anc_mic_params.enabled_mics == feed_back_mode
                || anc_mic_params.enabled_mics == feed_forward_mode);
}

static bool isAncConfigLeftMicEnabled(void)
{
    return ((anc_mic_params.enabled_mics & feed_forward_left) || (anc_mic_params.enabled_mics & feed_back_left));
}

static bool isAncConfigRightMicEnabled(void)
{
    return ((anc_mic_params.enabled_mics & feed_forward_right) || (anc_mic_params.enabled_mics & feed_back_right));
}

static bool isAncConfigTwoMic(void)
{
    return (anc_mic_params.enabled_mics == hybrid_mode || anc_mic_params.enabled_mics == hybrid_mode_right_only
            || anc_mic_params.enabled_mics == hybrid_mode_left_only);
}

static void connectMic(audio_mic_params * mic_params, tuning_operator_input_terminals_t terminal)
{
    Operator tuning_operator = ChainGetOperatorByRole(tuning_chain, tuning_role);
    Source mic_source = AudioPluginMicSetup(mic_params->channel, *mic_params, ANC_TUNING_MODE_SAMPLE_RATE);
    Sink terminal_sink = StreamSinkFromOperatorTerminal(tuning_operator, terminal);

    PanicNull(StreamConnect(mic_source, terminal_sink));
}

static void connectAncMicrophones(void)
{
    switch(anc_mic_params.enabled_mics)
    {
        case hybrid_mode:
        case hybrid_mode_left_only:
        case hybrid_mode_right_only:
            if (isAncConfigLeftMicEnabled())
            {
                connectMic(&anc_mic_params.feed_back_left, mic_1_left_terminal);
                connectMic(&anc_mic_params.feed_forward_left, mic_2_left_terminal);
            }
            if (isAncConfigRightMicEnabled())
            {
                connectMic(&anc_mic_params.feed_back_right, mic_1_right_terminal);
                connectMic(&anc_mic_params.feed_forward_right, mic_2_right_terminal);
            }
            break;

        case feed_back_mode:
        case feed_back_mode_left_only:
        case feed_back_mode_right_only:
            if (isAncConfigLeftMicEnabled())
                connectMic(&anc_mic_params.feed_back_left, mic_1_left_terminal);
            if (isAncConfigRightMicEnabled())
                connectMic(&anc_mic_params.feed_back_right, mic_1_right_terminal);
            break;

        case feed_forward_mode:
        case feed_forward_mode_left_only:
        case feed_forward_mode_right_only:
            if (isAncConfigLeftMicEnabled())
                connectMic(&anc_mic_params.feed_forward_left, mic_1_left_terminal);
            if (isAncConfigRightMicEnabled())
                connectMic(&anc_mic_params.feed_forward_right, mic_1_right_terminal);
            break;

        default:
            Panic();
            break;
    }
}

static unsigned getInstanceForMonitorMics(void)
{
    unsigned instance = 0;

    for(instance = 0; instance < NUMBER_OF_AUDIO_INSTANCES; instance++)
    {
        if(((anc_mic_params.enabled_mics & feed_forward_left) && anc_mic_params.feed_forward_left.instance == instance)
                || ((anc_mic_params.enabled_mics & feed_back_left) && anc_mic_params.feed_back_left.instance == instance)
                || ((anc_mic_params.enabled_mics & feed_forward_right) && anc_mic_params.feed_forward_right.instance == instance)
                || ((anc_mic_params.enabled_mics & feed_back_right) && anc_mic_params.feed_back_right.instance == instance))
        {
            continue;
        }
        break;
    }

    PanicFalse(instance == MONITOR_MIC_DIGITAL_INSTANCE || instance == MONITOR_MIC_ANALOGUE_INSTANCE);

    return instance;
}

static bool isMonitorMicDigital(audio_instance instance)
{
    if(instance != MONITOR_MIC_DIGITAL_INSTANCE && instance != MONITOR_MIC_ANALOGUE_INSTANCE)
    {
        Panic();
    }
    return (instance == MONITOR_MIC_ANALOGUE_INSTANCE ? FALSE : TRUE);
}

static audio_mic_params getMonitorMic(audio_channel channel)
{
    audio_mic_params monitor_mic = { .bias_config = BIAS_CONFIG_DISABLE, .pre_amp = 0,
            .pio = 0, .gain = MONITOR_MIC_GAIN, .is_digital = FALSE, .instance = 0, .channel = channel };
    monitor_mic.instance = getInstanceForMonitorMics();
    monitor_mic.is_digital = isMonitorMicDigital(monitor_mic.instance);
    return monitor_mic;
}

static void setMonitorMicGain(audio_mic_params * mic_params)
{
    if(isMonitorMicDigital(mic_params->instance) == FALSE)
    {
        Source mic_source = AudioPluginGetMicSource(*mic_params, mic_params->channel);
        SourceConfigure(mic_source, STREAM_CODEC_RAW_INPUT_GAIN, ZERO_RAW_GAIN);
    }
}

static void connectMonitorMicrophones(void)
{
    audio_mic_params monitor_mic;

    if (isAncConfigLeftMicEnabled())
    {
        monitor_mic = getMonitorMic(AUDIO_CHANNEL_A);
        connectMic(&monitor_mic, monitor_left_terminal);
        setMonitorMicGain(&monitor_mic);
    }

    if (isAncConfigRightMicEnabled())
    {
        monitor_mic = getMonitorMic(AUDIO_CHANNEL_B);
        connectMic(&monitor_mic, monitor_right_terminal);
        setMonitorMicGain(&monitor_mic);
    }
}

static Source getMonitorMicToSynchroniseWith(void)
{
    Source sync_source = 0;
    audio_mic_params monitor_mic;

    if(isAncConfigLeftMicEnabled())
    {
        monitor_mic = getMonitorMic(AUDIO_CHANNEL_A);
        sync_source = AudioPluginGetMicSource(monitor_mic, monitor_mic.channel);
    }
    else
    {
        monitor_mic = getMonitorMic(AUDIO_CHANNEL_B);
        sync_source = AudioPluginGetMicSource(monitor_mic, monitor_mic.channel);
    }

    return sync_source;
}

static void synchroniseMicrophones(void)
{
    Source sync_source = getMonitorMicToSynchroniseWith();

    if (isAncConfigStereo())
    {
        audio_mic_params monitor_mic = getMonitorMic(AUDIO_CHANNEL_B);
        SourceSynchronise(sync_source, AudioPluginGetMicSource(monitor_mic, monitor_mic.channel));
    }

    if(anc_mic_params.enabled_mics & feed_forward_left)
    {
        SourceSynchronise(sync_source, AudioPluginGetMicSource(anc_mic_params.feed_forward_left, anc_mic_params.feed_forward_left.channel));
    }

    if(anc_mic_params.enabled_mics & feed_forward_right)
    {
        SourceSynchronise(sync_source, AudioPluginGetMicSource(anc_mic_params.feed_forward_right, anc_mic_params.feed_forward_right.channel));
    }

    if(anc_mic_params.enabled_mics & feed_back_left)
    {
        SourceSynchronise(sync_source, AudioPluginGetMicSource(anc_mic_params.feed_back_left, anc_mic_params.feed_back_left.channel));
    }

    if(anc_mic_params.enabled_mics & feed_back_right)
    {
        SourceSynchronise(sync_source, AudioPluginGetMicSource(anc_mic_params.feed_back_right, anc_mic_params.feed_back_right.channel));
    }
}

static void connectMicrophones(void)
{
    synchroniseMicrophones();
    connectAncMicrophones();
    connectMonitorMicrophones();
}


static void connectUsbInterface(void)
{
    StreamConnect(StreamUsbEndPointSource(end_point_iso_in), ChainGetInput(tuning_chain, path_usb));

    StreamConnect(ChainGetOutput(tuning_chain, path_usb), StreamUsbEndPointSink(end_point_iso_out));
}

static void connectAudioOutputs(void)
{
    Operator tuning_operator = ChainGetOperatorByRole(tuning_chain, tuning_role);
    Source left = StreamSourceFromOperatorTerminal(tuning_operator, speaker_left_terminal);
    Source right = StreamSourceFromOperatorTerminal(tuning_operator, speaker_right_terminal);

    audio_output_params_t output_params = { ANC_TUNING_MODE_SAMPLE_RATE, audio_output_tansform_connect, TRUE };

    if(isAncConfigStereo())
    {
        AudioOutputConnectStereoSource(left, right, &output_params);
    }
    else if(isAncConfigLeftMicEnabled())
    {
        AudioOutputConnectSource(left,
                                audio_output_primary_left,
                                &output_params);
    }
    else
    {
        AudioOutputConnectSource(right,
                                audio_output_primary_left,
                                &output_params);
    }

}

static void configureTuningOperator(void)
{
    Operator tuning_operator = ChainGetOperatorByRole(tuning_chain, tuning_role);

    anc_tuning_set_config_msg_t message;
    message.id = TUNING_CAPABILITY_MESSAGE_ID;
    message.is_stereo = isAncConfigStereo();
    message.is_two_mic = isAncConfigTwoMic();

    PanicFalse(VmalOperatorMessage(tuning_operator, &message, SIZEOF_OPERATOR_MESSAGE(message), NULL, 0));
    OperatorsStandardSetUCID(tuning_operator, SETUCID_UCID);
}

static void connectTuningMode(anc_mic_params_t * mic_params)
{
    tuning_chain = ChainCreate(AudioAncTuningGetChainConfig());
    anc_mic_params = *mic_params;

    configureTuningOperator();

    ChainConnect(tuning_chain);

    connectUsbInterface();
    connectAudioOutputs();

    connectMicrophones();

    ChainStart(tuning_chain);

    SetAudioInUse(TRUE);
}



static void disconnectSource(Source source)
{
    StreamDisconnect(source, 0);
    StreamConnectDispose(source);
    SourceClose(source);
}

static void disconnectSink(Sink sink)
{
    StreamDisconnect(0, sink);
    SinkClose(sink);
}

static void disconnectUsbInterface(void)
{
    disconnectSource(StreamUsbEndPointSource(end_point_iso_in));
    disconnectSink(StreamUsbEndPointSink(end_point_iso_out));
}

static void disconnectAudioOutputs(void)
{
    AudioOutputDisconnect();
}

static void disconnectAncMicrophones(void)
{
    if(anc_mic_params.enabled_mics & feed_forward_left)
    {
        disconnectSource(AudioPluginGetMicSource(anc_mic_params.feed_forward_left, anc_mic_params.feed_forward_left.channel));
    }
    if(anc_mic_params.enabled_mics & feed_forward_right)
    {
        disconnectSource(AudioPluginGetMicSource(anc_mic_params.feed_forward_right, anc_mic_params.feed_forward_right.channel));
    }
    if(anc_mic_params.enabled_mics & feed_back_left)
    {
        disconnectSource(AudioPluginGetMicSource(anc_mic_params.feed_back_left, anc_mic_params.feed_back_left.channel));
    }
    if(anc_mic_params.enabled_mics & feed_back_right)
    {
        disconnectSource(AudioPluginGetMicSource(anc_mic_params.feed_back_right, anc_mic_params.feed_back_right.channel));
    }
}

static void disconnectMonitorMicrophones(void)
{
    audio_mic_params monitor_mic;

    if (isAncConfigLeftMicEnabled())
    {
        monitor_mic = getMonitorMic(AUDIO_CHANNEL_A);
        disconnectSource(AudioPluginGetMicSource(monitor_mic, monitor_mic.channel));
    }    

    if (isAncConfigRightMicEnabled())
    {
        monitor_mic = getMonitorMic(AUDIO_CHANNEL_B);
        disconnectSource(AudioPluginGetMicSource(monitor_mic, monitor_mic.channel));
    }    
}


static void disconnectTuningMode(void)
{
    ChainStop(tuning_chain);

    disconnectUsbInterface();
    disconnectAudioOutputs();
    disconnectAncMicrophones();
    disconnectMonitorMicrophones();

    ChainDestroy(tuning_chain);
    tuning_chain = NULL;

    SetAudioInUse(FALSE);
}


static void AudioAncTuningMessageHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(message);

    switch(id)
    {
        case AUDIO_PLUGIN_CONNECT_MSG:
            {
                anc_mic_params_t * mic_params = (anc_mic_params_t *)((const AUDIO_PLUGIN_CONNECT_MSG_T *)message)->params;
                connectTuningMode(mic_params);
            }
            break;
        
        case AUDIO_PLUGIN_DISCONNECT_MSG:
            disconnectTuningMode();
            break;
        
        default:
            break;
    }
}

/******************************************************************************/
const A2dpPluginTaskdata * AncGetTuningModePlugin(void)
{
    return &anc_tuning_plugin;
}
