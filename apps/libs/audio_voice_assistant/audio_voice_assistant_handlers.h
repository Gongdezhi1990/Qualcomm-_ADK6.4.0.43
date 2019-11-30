/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_voice_assistant_handlers.h

DESCRIPTION
        Message handler functions..
*/

#ifndef AUDIO_VOICE_ASSISTANT_HANDLERS_H_
#define AUDIO_VOICE_ASSISTANT_HANDLERS_H_

#include <audio_plugin_if.h>

/****************************************************************************
DESCRIPTION
    Handle AUDIO_PLUGIN_CONNECT_MSG.
*/
void audioVaConnectMsgHandler(AUDIO_PLUGIN_CONNECT_MSG_T *message);

/****************************************************************************
DESCRIPTION
    Handle AUDIO_PLUGIN_DISCONNECT_MSG.
*/
void audioVaDisconnectMsgHandler(void);

/****************************************************************************
DESCRIPTION
    Handle AUDIO_PLUGIN_START_VOICE_CAPTURE_MSG.
*/
void audioVaStartCaptureMsgHandler(AUDIO_PLUGIN_START_VOICE_CAPTURE_MSG_T *message);

/****************************************************************************
DESCRIPTION
    Handle AUDIO_PLUGIN_STOP_VOICE_CAPTURE_MSG.
*/
void audioVaStopCaptureMsgHandler(void);

/****************************************************************************
DESCRIPTION
    Handle AUDIO_PLUGIN_START_TRIGGER_DETECT_MSG.
*/
void audioVaStartTriggerDetectMsgHandler(AUDIO_PLUGIN_START_TRIGGER_DETECT_MSG_T *message);

/****************************************************************************
DESCRIPTION
    Handle AUDIO_PLUGIN_STOP_TRIGGER_DETECT_MSG.
*/
void audioVaStopTriggerDetectMsgHandler(void);


/****************************************************************************
DESCRIPTION
    QVA trigger phrase indication message handler.
    Function send the trigger phrase detection to registered application task
*/
void audioVaQvaMessageHandler(Task task, MessageId id, Message message);

#endif /* AUDIO_VOICE_ASSISTANT_HANDLERS_H_ */

