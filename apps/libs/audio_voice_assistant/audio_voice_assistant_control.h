/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_voice_assistant_control.h

DESCRIPTION
    Code to control voice assistant plugin.
*/

#ifndef AUDIO_VOICE_ASSISTANT_CONTROL_H_
#define AUDIO_VOICE_ASSISTANT_CONTROL_H_

#include <message.h>
#include <audio_plugin_if.h>

#include "audio_voice_assistant_private.h"

/****************************************************************************
DESCRIPTION
    Setup voice capture
*/
void audioVaStartCapture(va_plugin_context_t *va_context,bool use_timestamp,uint32 start_timestamp);

/****************************************************************************
DESCRIPTION
    Stop voice capture
*/
void audioVaStopCapture(va_plugin_context_t *va_context);

/****************************************************************************
DESCRIPTION
    Start Trigger Phrase detection
*/
void audioVaStartTrigger(va_plugin_context_t *va_context, AUDIO_PLUGIN_START_TRIGGER_DETECT_MSG_T *msg);

/****************************************************************************
DESCRIPTION
    Stop Trigger Phrase Detection
*/
void audioVaStopTrigger(va_plugin_context_t *va_context);


/******************************************************************************
DESCRIPTION
    Pause Trigger detection until stop trigger
*/
void audioVaPauseTrigger(va_plugin_context_t *va_context);

/******************************************************************************
DESCRIPTION
    Connect Audio and Common Chain CVC and AEC ref
*/
void audioVaStart(va_plugin_context_t *va_context);

/******************************************************************************
DESCRIPTION
    Disconnect Audio and Common Chain, AEC ref and CVC
*/
void audioVaStop(va_plugin_context_t *va_context);

#endif /* AUDIO_VOICE_ASSISTANT_CONTROL_H_ */

