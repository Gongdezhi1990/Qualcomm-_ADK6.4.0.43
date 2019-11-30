/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_voice_prompts_chain.h

DESCRIPTION
    Module talking directly to the Kymera.
    Chain is a set of connected the Kymera operators.

    Sequence of events is:
    1. Turn on DSP
    2. Instantiate a chain
    3. Start a chain
    4. Wait for some event
    5. Stop a chain
    6. Destroy a chain
    7. Turn off DSP
*/

#ifndef LIBS_AUDIO_INPUT_VOICE_PROMPTS_AUDIO_INPUT_VOICE_PROMPTS_CHAIN_H_
#define LIBS_AUDIO_INPUT_VOICE_PROMPTS_AUDIO_INPUT_VOICE_PROMPTS_CHAIN_H_

#include <csrtypes.h>
#include <sink.h>
#include <source.h>
#include <audio_plugin_if.h>

#include "audio_input_voice_prompts_defs.h"


/*
   The creation of the tone/prompt audio chain is being broken up into
   multiple steps that are run asynchronously via internal messages.
   The intention is to allow other VM message handlers to run in-between
   steps and not block them for a long time.

   In order to do that, the new messsages and states are defined here. */

/* Prompt/tone audio chain state. */
typedef enum
{
    VP_CHAIN_CREATING = 0,
    VP_CHAIN_CONNECTING_MIXER,
    VP_CHAIN_CONNECTING_INPUT,
    VP_CHAIN_STARTING,
    VP_CHAIN_PLAYING,
} vp_chain_state_t;

#define VP_INTERNAL_MSG_BASE 0x400

/* Prompt/tone chain creation internal msgs. */
typedef enum
{
    VP_CHAIN_CREATED_CFM = VP_INTERNAL_MSG_BASE,
    VP_CHAIN_MIXER_CONNECT_CFM,
    VP_CHAIN_INPUT_CONNECT_CFM,
    VP_CHAIN_START_CFM,

    VP_CHAIN_MSG_LAST
} vp_chain_msg_t;


/****************************************************************************
DESCRIPTION
    Create, setup and make internal connections for tone playing chain.
    Task is where tone end notification will be sent.
*/
vp_handle_t VoicePromptsChainConnectTone(vp_context_t *context);


/****************************************************************************
DESCRIPTION
    Create, setup and make internal connections for voice prompt playing chain.
*/
vp_handle_t VoicePromptsChainConnectPrompt(vp_context_t *context);

/****************************************************************************
DESCRIPTION
    Create and start the vp chain using an asynchronous sequence.

RETURN
    vp_handle_t Handle to the incomplete tone/prompt chain. The chain will
                not be fully created yet because this function uses
                internal messsages to create it asynchronously.
*/
vp_handle_t VoicePromptsChainConnectAndStart(vp_context_t *context);

/****************************************************************************
DESCRIPTION
    Disconnect and destroy chain and associated resources.
*/
void VoicePromptsChainDisconnect(vp_handle_t vp_handle);

/****************************************************************************
DESCRIPTION
    Start the chain
*/
void VoicePromptsChainStart(vp_handle_t vp_handle);

/****************************************************************************
DESCRIPTION
    Stop the chain
*/
void VoicePromptsChainStop(vp_handle_t vp_handle);

/****************************************************************************
DESCRIPTION
    Get the current state of the prompt/tone chain.
*/
vp_chain_state_t VoicePromptsChainGetState(vp_handle_t vp_handle);

/****************************************************************************
DESCRIPTION
    Check if a message is a vp chain internal message.
*/
bool VoicePromptsIsChainMessage(MessageId id);

/****************************************************************************
DESCRIPTION
    Handler for vp chain internal messages
*/
void VoicePromptsChainMessageHandler(Task task, MessageId id, Message message);


#endif /* LIBS_AUDIO_INPUT_VOICE_PROMPTS_AUDIO_INPUT_VOICE_PROMPTS_CHAIN_H_ */
