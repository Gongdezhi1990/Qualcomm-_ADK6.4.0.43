/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_voice_assistant_handlers.c

DESCRIPTION
    Message handler functions.
*/
#include "audio_plugin_if.h"
#include "audio.h"
#include "audio_voice_assistant_handlers.h"
#include "audio_voice_assistant_private.h"
#include "audio_voice_assistant_control.h"
#include "audio_voice_assistant_trigger_chain.h"


/****************************************************************************
DESCRIPTION
    Indicates the registred application task that trigger has been detected.
*/
/* ss78: ToDo, this is temporary untill QVA message format is finalized and implemented */
static void indicateTriggerDetected(va_trigger_message_t *qva_msg)
{
    channel_trigger_details_t *channel;
    MAKE_AUDIO_MESSAGE( AUDIO_VA_MESSAGE_TRIGGERED_IND, message ) ;

    channel = &qva_msg->ch0;

    message->start_timestamp = (((uint32)channel->MSB_start_ttp << 16) | (channel->LSB_start_ttp));
    message->end_timestamp = (((uint32)channel->MSB_end_ttp << 16) | (channel->LSB_end_ttp));        

    MessageSend(audioVaGetContext()->app_task, AUDIO_VA_MESSAGE_TRIGGERED_IND, message);
}

/****************************************************************************
DESCRIPTION
    Handle AUDIO_PLUGIN_CONNECT_MSG.
*/
void audioVaConnectMsgHandler(AUDIO_PLUGIN_CONNECT_MSG_T *message)
{
    audio_va_connect_params_t *params;
    PanicFalse(message);
    PanicNull(message->params);
    params = (audio_va_connect_params_t*)message->params;
    audioVaCreateContext(params->app_task, params->mic_params, params->support_voice_trigger,
                         params->cvc_operator, params->cvc_omni_mode);
    free(message->params);
    
    /* If voice trigger is supported, then create the common chain now , 
     * else this can be done along with start capture sequqnce 
    */
    if(audioVaGetContext()->support_voice_trigger)
    {
        audioVaStart(audioVaGetContext());
    }
}

/****************************************************************************
DESCRIPTION
    Handle AUDIO_PLUGIN_DISCONNECT_MSG.
*/
void audioVaDisconnectMsgHandler(void)
{
   /* If voice trigger is supported, then destroy the common chain now , 
     * else this shall be done along with stop capture sequqnce 
     */
    if(audioVaGetContext()->support_voice_trigger)
    {
        /* Stop complete VA chain  */
        audioVaStop(audioVaGetContext());
    }
    /* Clean up and free va context */
    audioVaCleanUpContext();
}

/****************************************************************************
DESCRIPTION
    Handle AUDIO_PLUGIN_START_VOICE_CAPTURE_MSG.
*/
void audioVaStartCaptureMsgHandler(AUDIO_PLUGIN_START_VOICE_CAPTURE_MSG_T *message)
{   
    PanicNull(message);
    audioVaStartCapture(audioVaGetContext(),message->use_timestamp,message->start_timestamp);
}

/****************************************************************************
DESCRIPTION
    Handle AUDIO_PLUGIN_STOP_VOICE_CAPTURE_MSG.
*/
void audioVaStopCaptureMsgHandler(void)
{
    audioVaStopCapture(audioVaGetContext());
}

/****************************************************************************
DESCRIPTION
    Handle AUDIO_PLUGIN_START_TRIGGER_DETECT_MSG.
*/
void audioVaStartTriggerDetectMsgHandler(AUDIO_PLUGIN_START_TRIGGER_DETECT_MSG_T *message)
{
    audioVaStartTrigger(audioVaGetContext(), message);
}

/****************************************************************************
DESCRIPTION
    Handle AUDIO_PLUGIN_STOP_TRIGGER_DETECT_MSG.
*/
void audioVaStopTriggerDetectMsgHandler(void)
{
    audioVaStopTrigger(audioVaGetContext());
}


/****************************************************************************
DESCRIPTION
    Handle QVA Trigger Messages.
*/
void audioVaQvaMessageHandler(Task task, MessageId id, Message msg)
{

    UNUSED(task);
    UNUSED(id);

#define VA_DATA_PAYLOAD 0x0004

    audioVaPauseTrigger(audioVaGetContext());
    if(msg)
    {
        va_trigger_message_t *qva_msg = (va_trigger_message_t*)((uint16*)msg + VA_DATA_PAYLOAD);
        
        indicateTriggerDetected(qva_msg);
    }
}

