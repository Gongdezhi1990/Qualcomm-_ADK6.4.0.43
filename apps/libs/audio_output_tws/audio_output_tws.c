/****************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_output_tws.c

DESCRIPTION
    Message dispatcher.
*/
#include <stdlib.h>

#include <message.h>
#include <audio_plugin_if.h>
#include <audio_mixer.h>
#include <vmtypes.h>
#include <print.h>

#include <audio.h>
#include <audio_plugin_forwarding.h>
#include <audio_plugin_output_variants.h>
#include <tws_packetiser.h>

#include "audio_output_tws.h"
#include "audio_output_tws_connect.h"
#include "audio_output_tws_ctx.h"

static void AudioOutputTwsAudioMessageHandler(Task task, MessageId id, Message message)
{
    UNUSED(message);
    UNUSED(task);
    
    switch(id)
    {
        #ifdef HOSTED_TEST_ENVIRONMENT
        case AUDIO_PLUGIN_TEST_RESET_MSG:
            AudioOutputTwsConnectTestReset();
            break;
        #endif

        default:
            Panic();
            break;
    }
}

static void AudioOutputTwsForwardingMessageHandler(Task task, MessageId id, Message message)
{
    UNUSED(message);
    UNUSED(task);
    
    switch(id)
    {
        case AUDIO_PLUGIN_FORWARDING_CREATE_REQ:
        {
            AudioOutputTwsConnect(task, (const AUDIO_PLUGIN_FORWARDING_CREATE_REQ_T*)message);
            break;
        }
        
        case AUDIO_PLUGIN_FORWARDING_START_REQ:
        {
            AudioOutputTwsStart(task, (const AUDIO_PLUGIN_FORWARDING_START_REQ_T*)message);
            break;
        }
        
        case AUDIO_PLUGIN_FORWARDING_STOP_REQ:
        {
            AudioOutputTwsStop(task, (const AUDIO_PLUGIN_FORWARDING_STOP_REQ_T*)message);
            break;
        }
        
        case AUDIO_PLUGIN_FORWARDING_DESTROY_REQ:
        {
            AudioOutputTwsDestroy(task, (const AUDIO_PLUGIN_FORWARDING_DESTROY_REQ_T*)message);
            break;
        }
        
        default:
            Panic();
            break;
    }
}

static void AudioOutputTwsPacketiserHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(message);

    switch(id)
    {
    case TWS_PACKETISER_FIRST_AUDIO_FRAME_IND:
        AudioPluginForwardingStreamingInd(AudioOutputTwsGetInputTask());
        break;

    default:
        break;
    }
}

void AudioOutputTwsMessageHandler(Task task, MessageId id, Message message)
{
    PRINT(("audio_output_tws handler id 0x%x\n", (unsigned)id));
    
    if(id >= AUDIO_DOWNSTREAM_MESSAGE_BASE && id < AUDIO_DOWNSTREAM_MESSAGE_TOP)
    {
        AudioOutputTwsAudioMessageHandler(task, id, message);
    }
    else if (id >= AUDIO_PLUGIN_FORWARDING_BASE && id < AUDIO_PLUGIN_FORWARDING_TOP)
    {
        AudioOutputTwsForwardingMessageHandler(task, id, message);
    }
    else if(id >= TWS_PACKETISER_MESSAGE_BASE && id < TWS_PACKETISER_MESSAGE_TOP)
    {
        AudioOutputTwsPacketiserHandler(task, id, message);
    }
    else
    {
        Panic();
    }
}
