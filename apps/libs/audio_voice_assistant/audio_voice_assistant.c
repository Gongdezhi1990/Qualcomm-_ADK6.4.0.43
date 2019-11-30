/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_voice_assistant.c

DESCRIPTION
    Message dispatcher.
*/
#include <stdlib.h>

#include <message.h>
#include <audio.h>
#include <audio_input_common.h>
#include <vmtypes.h>
#include <print.h>


#include "audio_voice_assistant.h"
#include "audio_voice_assistant_handlers.h"
#include "audio_plugin_voice_assistant_variants.h"
#include "audio_voice_assistant_private.h"

static va_plugin_context_t *va_context = NULL;

/* Plugin Task */
const TaskData voice_assistant_plugin = { audioVaMessageHandler };


/****************************************************************************
DESCRIPTION
    Function to Clean up Voice Assistant Plugin Context
*/
void audioVaCleanUpContext(void)
{
    PRINT(("VA_Plugin: Destroy context\n"));
    if(va_context)
    {
        free(va_context);
        va_context = NULL;
    }
}

/****************************************************************************
DESCRIPTION
    Function to Create UP Voice Assistant Plugin Context
*/
void audioVaCreateContext(Task app_task, voice_mic_params_t  *mic, bool support_voice_trigger, plugin_variant_t cvc, bool omni_mode)
{
    PRINT(("VA_Plugin: Create context\n"));
    PanicNotNull(va_context);
    PanicNull(app_task);
    va_context = PanicNull(calloc(1, sizeof(*va_context)));
    va_context->app_task = app_task;
    va_context->mic_params = mic;
    va_context->support_voice_trigger = support_voice_trigger;
    va_context->cvc = cvc;
    va_context->cvc_omni_mode = omni_mode;
    va_context->trigger_detected = FALSE;
}

/****************************************************************************
DESCRIPTION
    Function to returns Voice Assistant Plugin Context
*/
va_plugin_context_t * audioVaGetContext(void)
{
    return va_context;
}

/****************************************************************************
DESCRIPTION
    Check context is enabled for Far Filed Voice or not.
*/
bool audioVaIsFFVEnabled(void)
{
    return ((audioVaGetContext()->cvc)== plugin_cvc_spkr_3mic_farfield) ? TRUE:FALSE;
}

/*****************************************************************************
DESCRIPTION
    Function for Handling VA audio messages
*/
void audioVaMessageHandler (Task task, MessageId id, Message message)
{
    UNUSED(task);
    switch(id)
    {
        case AUDIO_PLUGIN_CONNECT_MSG:
            PRINT(("VA_Plugin: AUDIO_PLUGIN_CONNECT_MSG received\n"));
            audioVaConnectMsgHandler((AUDIO_PLUGIN_CONNECT_MSG_T*)message);
            break;

        case AUDIO_PLUGIN_DISCONNECT_MSG:
            PRINT(("VA_Plugin: AUDIO_PLUGIN_DISCONNECT_MSG received\n"));
            PanicNull(audioVaGetContext());
            audioVaDisconnectMsgHandler();
            break;

        case AUDIO_PLUGIN_START_TRIGGER_DETECT_MSG:
            PRINT(("VA_Plugin: AUDIO_PLUGIN_START_TRIGGER_DETECT_MSG received\n"));
            PanicNull(audioVaGetContext());
            audioVaStartTriggerDetectMsgHandler((AUDIO_PLUGIN_START_TRIGGER_DETECT_MSG_T*)message);
            break;

        case AUDIO_PLUGIN_STOP_TRIGGER_DETECT_MSG:
            PRINT(("VA_Plugin: AUDIO_PLUGIN_STOP_TRIGGER_DETECT_MSG received\n"));
            PanicNull(audioVaGetContext());
            audioVaStopTriggerDetectMsgHandler();
            break;

        case AUDIO_PLUGIN_START_VOICE_CAPTURE_MSG:
            PRINT(("VA_Plugin: AUDIO_PLUGIN_START_VOICE_CAPTURE_MSG received \n"));
            PanicNull(audioVaGetContext());
            audioVaStartCaptureMsgHandler((AUDIO_PLUGIN_START_VOICE_CAPTURE_MSG_T*)message);
            break;

        case AUDIO_PLUGIN_STOP_VOICE_CAPTURE_MSG:
            PRINT(("VA_Plugin: AUDIO_PLUGIN_STOP_VOICE_CAPTURE_MSG received \n"));
            PanicNull(audioVaGetContext());
            audioVaStopCaptureMsgHandler();
            break;

        case AUDIO_PLUGIN_TEST_RESET_MSG:
            PRINT(("VA_Plugin: AUDIO_PLUGIN_TEST_RESET_MSG received \n"));
            audioVaCleanUpContext();
            break;

        default:
            PRINT(("VA_Plugin: Unknown message in audioVaMessageHandler()\n"));
            Panic();
            break;
    }
}

