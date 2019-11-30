/****************************************************************************
Copyright (c) 2016-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_voice_prompts_chain.c

DESCRIPTION
    Module talking directly to the Kymera.
*/
#include <operator.h>
#include <stream.h>
#include <panic.h>
#include <vmal.h>
#include <stdlib.h>

#include <print.h>
#include <operators.h>
#include <chain.h>
#include <audio.h>
#include <audio_mixer.h>
#include <audio_plugin_if.h>

#include "audio_input_voice_prompts_defs.h"
#include "audio_input_voice_prompts_chain.h"
#include "audio_input_voice_prompts_play.h"
#include "audio_input_voice_prompts_utils.h"
#include "audio_input_voice_prompts_chain_config.h"


typedef struct __vp_data_t
{
    vp_chain_state_t state;
    kymera_chain_handle_t chain;
    audio_mixer_input_t mixer_input;
} vp_data_t;

extern const TaskData csr_voice_prompts_plugin;

/****************************************************************************
DESCRIPTION
    Determine if a decoder is required to decode the voice prompt
*/
static bool isDecoderRequired(voice_prompts_codec_t codec_type)
{
    bool decoderRequired = FALSE;

    switch(codec_type)
    {
        case voice_prompts_codec_pcm:
        case voice_prompts_codec_pcm_8khz:
            decoderRequired = FALSE;
        break;

        case voice_prompts_codec_sbc:
            decoderRequired = TRUE;
        break;

        case voice_prompts_codec_ima_adpcm:
        case voice_prompts_codec_mp3:
        case voice_prompts_codec_aac:
        case voice_prompts_codec_tone:
        default:
            Panic();
    }

    return decoderRequired;
}

/****************************************************************************
DESCRIPTION
    Configure the chain required to play tones
*/
static void configureToneChain(kymera_chain_handle_t chain, const ringtone_note *tone)
{
    Operator tone_op = ChainGetOperatorByRole(chain, tone_role);

    OperatorsConfigureToneGenerator(tone_op, tone, (Task)&csr_voice_prompts_plugin);
}

/****************************************************************************
DESCRIPTION
    Connect supplied sources to audio mixer
*/
static audio_mixer_input_t connectToMixer(vp_data_t* vp_data, unsigned input_sample_rate, bool stereo)
{
    audio_mixer_connect_t connect_data;
    audio_mixer_input_t input;

    connect_data.left_src = ChainGetOutput(vp_data->chain, left_output_path_role);
    connect_data.right_src = stereo ? ChainGetOutput(vp_data->chain, right_output_path_role) : NULL;
    connect_data.connection_type = CONNECTION_TYPE_TONES;
    connect_data.sample_rate = input_sample_rate;
    connect_data.channel_mode = CHANNEL_MODE_STEREO;
    connect_data.variable_rate = FALSE;

    input = AudioMixerConnect(&connect_data);

    PanicFalse(input != audio_mixer_input_error_none);

    return input;
}

/****************************************************************************
DESCRIPTION
    Create, setup and make internal connections for tone playing chain.
    Task is where tone end notification will be sent.
*/
vp_handle_t VoicePromptsChainConnectTone(vp_context_t *context)
{
    vp_data_t* vp_data = PanicUnlessNew(vp_data_t);

    vp_data->chain = audioInputVoicePromptsCreateToneChain();
    configureToneChain(vp_data->chain, context->tone);

    return (vp_handle_t)vp_data;
}

/****************************************************************************
DESCRIPTION
    Create the required voice prompt chain
*/
static void voicePromptsChainCreate(vp_data_t* vp_data, voice_prompts_codec_t codec_type)
{
    if (isDecoderRequired(codec_type))
    {
        vp_data->chain = audioInputVoicePromptsCreatePromptDecoderChain();
    }
    else
    {
        /* Need a dummy chain to register for message for when prompt completes */
        /* Can we register for message from file source instead? Then passthrough operator
         * will not be required. */
        vp_data->chain = audioInputVoicePromptsCreatePromptDummyChain();
    }
}

/****************************************************************************
DESCRIPTION
    Connect the input sink to the voice prompt chain.
*/
static void voicePromptsChainPromptConnectInput(vp_data_t* vp_data, vp_context_t *context)
{
    Sink sink;

    sink = ChainGetInput(vp_data->chain, input_path_role);

    VoicePromptsRegisterForMessagesFromSink(sink);

    PanicNull(StreamConnect(context->source, sink));
}

/****************************************************************************
DESCRIPTION
    Create, setup and make internal connections for voice prompt playing chain.

    @todo: Split this up into multiple steps to minimise how long the vm is blocked for.
*/
vp_handle_t VoicePromptsChainConnectPrompt(vp_context_t *context)
{
    vp_data_t* vp_data = PanicUnlessNew(vp_data_t);

    PRINT(("VoicePromptsChainInstantiateForType 0x%x, %u\n", context->codec_type, context->playback_rate));

    voicePromptsChainCreate(vp_data, context->codec_type);

    return (vp_handle_t)vp_data;
}

/****************************************************************************
DESCRIPTION
    Disconnect and destroy chain and associated resources.
*/
void VoicePromptsChainDisconnect(vp_handle_t vp_handle)
{
    vp_data_t* vp_data = (vp_data_t*)vp_handle;
    Sink input_sink;

    if(vp_data)
    {
        if(vp_data->chain)
        {
            input_sink = ChainGetInput(vp_data->chain, input_path_role);
            if(input_sink)
            {
                /* Cancel all the messages relating to VP that have been sent */
                VoicePromptsDeregisterForMessagesFromSink(input_sink);
            }
            AudioMixerDisconnect(vp_data->mixer_input);
            ChainDestroy(vp_data->chain);
            vp_data->chain= NULL;
        }
        free(vp_data);
    }

    MessageCancelAll((TaskData*)&csr_voice_prompts_plugin, MESSAGE_STREAM_DISCONNECT);
    MessageCancelAll((TaskData*)&csr_voice_prompts_plugin, VP_CHAIN_CREATED_CFM);
    MessageCancelAll((TaskData*)&csr_voice_prompts_plugin, VP_CHAIN_MIXER_CONNECT_CFM);
    MessageCancelAll((TaskData*)&csr_voice_prompts_plugin, VP_CHAIN_INPUT_CONNECT_CFM);
    MessageCancelAll((TaskData*)&csr_voice_prompts_plugin, VP_CHAIN_START_CFM);
}

/****************************************************************************
DESCRIPTION
    Start the vp chain
*/
void VoicePromptsChainStart(vp_handle_t vp_handle)
{
    vp_data_t* vp_data = (vp_data_t *)vp_handle;

    if (vp_data && vp_data->chain)
        ChainStart(vp_data->chain);
}


/****************************************************************************
DESCRIPTION
    Stop the vp chain
*/
void VoicePromptsChainStop(vp_handle_t vp_handle)
{
    vp_data_t* vp_data = (vp_data_t *)vp_handle;

    if (vp_data && vp_data->chain)
        ChainStop(vp_data->chain);
}

/****************************************************************************
DESCRIPTION
    Get the current state of the prompt/tone chain.
*/
vp_chain_state_t VoicePromptsChainGetState(vp_handle_t vp_handle)
{
    vp_data_t* vp_data = (vp_data_t *)vp_handle;
    return vp_data->state;
}

/****************************************************************************
DESCRIPTION
    Create the vp chain and start it.

    To try and minimise the amount of time the vm message loop is blocked,
    this uses a state-based asynchronous sequence to create the chain(s)
    and start them.
*/
vp_handle_t VoicePromptsChainConnectAndStart(vp_context_t *context)
{
    vp_handle_t vp_data = NULL;

    if(VoicePromptsIsItTone(context->codec_type))
    {
        PRINT(("tone\n"));
        vp_data = VoicePromptsChainConnectTone(context);
    }
    else
    {
        PRINT(("prompt\n"));
        vp_data = VoicePromptsChainConnectPrompt(context);
    }

    context->chain = vp_data;
    context->chain->state = VP_CHAIN_CREATING;
    MessageSend((TaskData*)&csr_voice_prompts_plugin, VP_CHAIN_CREATED_CFM, NULL);

    return vp_data;
}

/****************************************************************************
DESCRIPTION
    Check if a message is a vp chain internal message.
*/
bool VoicePromptsIsChainMessage(MessageId id)
{
    return ((id >= VP_INTERNAL_MSG_BASE) && (id < VP_CHAIN_MSG_LAST));
}

/****************************************************************************
DESCRIPTION
    Handler for vp chain creation internal messages.

    Performs the next step in the chain creation then sends an internal
    message to itself, until the chain is complete and started.
*/
void VoicePromptsChainMessageHandler(Task task, MessageId id, Message message)
{
    vp_context_t *context = VoicePromptsGetContext();
    vp_data_t *vp_data = context->chain;

    UNUSED(task);
    UNUSED(message);

    switch (id)
    {
    case VP_CHAIN_CREATED_CFM:
        {
            vp_data->mixer_input = connectToMixer(vp_data, context->playback_rate, context->stereo);

            vp_data->state = VP_CHAIN_CONNECTING_MIXER;
            MessageSend((TaskData*)&csr_voice_prompts_plugin, VP_CHAIN_MIXER_CONNECT_CFM, NULL);
        }
        break;

    case VP_CHAIN_MIXER_CONNECT_CFM:
        {
            if (VoicePromptsIsItTone(context->codec_type))
            {
                VoicePromptsChainStart(vp_data);

                vp_data->state = VP_CHAIN_STARTING;
                MessageSend((TaskData*)&csr_voice_prompts_plugin, VP_CHAIN_START_CFM, NULL);
            }
            else
            {
                voicePromptsChainPromptConnectInput(vp_data, context);

                vp_data->state = VP_CHAIN_CONNECTING_INPUT;
                MessageSend((TaskData*)&csr_voice_prompts_plugin, VP_CHAIN_INPUT_CONNECT_CFM, NULL);
            }
        }
        break;

    case VP_CHAIN_INPUT_CONNECT_CFM:
        {
            VoicePromptsChainStart(vp_data);

            vp_data->state = VP_CHAIN_STARTING;
            MessageSend((TaskData*)&csr_voice_prompts_plugin, VP_CHAIN_START_CFM, NULL);
        }
        break;

    case VP_CHAIN_START_CFM:
        {
            vp_data->state = VP_CHAIN_PLAYING;
        }
        break;

    default:
        Panic();
    }
}
