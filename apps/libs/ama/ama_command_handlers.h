/*****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    ama_command_handlers.h

DESCRIPTION
    Handlers for ama commands
*/

#ifndef _AMA_COMMAND_HANDLERS_H
#define _AMA_COMMAND_HANDLERS_H

#include "accessories.pb-c.h"

/***************************************************************************
DESCRIPTION
    Handles COMMAND__NOTIFY_SPEECH_STATE message 
 
PARAMS
    control_envelope The message from the phone
*/
void amaHandleCommandNotifySpeechState(ControlEnvelope *control_envelope_in);

/***************************************************************************
DESCRIPTION
    Handles COMMAND__STOP_SPEECH message 
 
PARAMS
    control_envelope The message from the phone
*/
void amaHandleCommandStopSpeech(ControlEnvelope *control_envelope_in);

/***************************************************************************
DESCRIPTION
    Handles COMMAND__GET_DEVICE_INFORMATION message 
 
PARAMS
    control_envelope The message from the phone
*/
void amaHandleCommandGetDeviceInformation(ControlEnvelope *control_envelope_in);

/***************************************************************************
DESCRIPTION
    Handles COMMAND__GET_DEVICE_CONFIGURATION message 
 
PARAMS
    control_envelope The message from the phone
*/
void amaHandleCommandGetDeviceConfiguration(ControlEnvelope *control_envelope_in);

/***************************************************************************
DESCRIPTION
    Handles COMMAND__START_SETUP message 
 
PARAMS
    control_envelope The message from the phone
*/
void amaHandleCommandStartSetup(ControlEnvelope * control_envelope_in);

/***************************************************************************
DESCRIPTION
    Handles COMMAND__COMPLETE_SETUP message 
 
PARAMS
    control_envelope The message from the phone
 
*/
void amaHandleCommandCompleteSetup(ControlEnvelope *control_envelope_in);

/***************************************************************************
DESCRIPTION
    Handles COMMAND__UPGRADE_TRANSPORT message 
 
PARAMS
    control_envelope The message from the phone
*/
void amaHandleCommandUpgradeTransport(ControlEnvelope *control_envelope_in);

/***************************************************************************
DESCRIPTION
    Handles COMMAND__SWITCH_TRANSPORT message 
 
PARAMS
    control_envelope The message from the phone
*/
void amaHandleCommandSwitchTransport(ControlEnvelope *control_envelope_in);

/***************************************************************************
DESCRIPTION
    Handles COMMAND__NOTIFY_SPEECH_STATE message 
 
PARAMS
    control_envelope The message from the phone
*/
void amaHandleCommandSynchronizeSettings(ControlEnvelope *control_envelope_in);

/***************************************************************************
DESCRIPTION
    Handles COMMAND__GET_STATE message 
 
PARAMS
    control_envelope The message from the phone
*/
void amaHandleCommandGetState(ControlEnvelope *control_envelope_in);

/***************************************************************************
DESCRIPTION
    Handles COMMAND__SET_STATE message 
 
PARAMS
    control_envelope The message from the phone
*/
void amaHandleCommandSetState(ControlEnvelope *control_envelope_in);

/***************************************************************************
DESCRIPTION
    Handles COMMAND__ISSUE_MEDIA_CONTROL message 
 
PARAMS
    control_envelope The message from the phone
*/
void amaHandleCommandMediaControl(ControlEnvelope *control_envelope_in);

/***************************************************************************
DESCRIPTION
    Handles COMMAND__OVERRIDE_ASSISTANT message 
 
PARAMS
    control_envelope The message from the phone
*/
void amaHandleCommandOverrideAssistant(ControlEnvelope *control_envelope_in);

/***************************************************************************
DESCRIPTION
    Handles COMMAND__PROVIDE_SPEECH message 
 
PARAMS
    control_envelope The message from the phone
*/
void amaHandleCommandProvideSpeech(ControlEnvelope *control_envelope_in);

/***************************************************************************
DESCRIPTION
    Handles COMMAND__ENDPOINT_SPEECH message 
 
PARAMS
    control_envelope The message from the phone
*/
void amaHandleCommandEndpointSpeech(ControlEnvelope *control_envelope_in);

/***************************************************************************
DESCRIPTION
    Handles COMMAND__FORWARD_AT_COMMAND message 
 
PARAMS
    control_envelope The message from the phone
*/
void amaHandleCommandForwardATCommand(ControlEnvelope *control_envelope_in);

/***************************************************************************
DESCRIPTION
    Handles when the message id is unknown
 
PARAMS
    control_envelope The message from the phone
*/
void amaHandleCommandNotHandled(ControlEnvelope *control_envelope_in);

/***************************************************************************
DESCRIPTION
    Handles COMMAND__KEEP_ALIVE message 
 
PARAMS
    control_envelope The message from the phone
*/
void amaHandleCommandKeepAlive(ControlEnvelope *control_envelope_in);


#endif /* _AMA_COMMAND_HANDLERS_H */
