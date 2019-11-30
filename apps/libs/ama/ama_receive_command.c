/*****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    ama_receive_command.c

DESCRIPTION
    Passes incoming command packet to respective handler function
*/

#include "stdio.h"
#include "stdlib.h"
#include <vm.h>
#include "ama_receive_command.h"
#include "ama_send_command.h"
#include "accessories.pb-c.h"
#include "ama_command_handlers.h"
#include "ama_debug.h"


static void amaHandleCommand(ControlEnvelope *control_envelope_in);
static void amaHandleResponse(ControlEnvelope *control_envelope_in);



void amaReceiveCommand(char* data, uint16 length)
{
    if(data && length)
    {
        ControlEnvelope* control_envelope_in = 
                control_envelope__unpack(NULL, (size_t)length, (const uint8_t*)data);

        if(control_envelope_in)
        {
            if(control_envelope_in->payload_case == CONTROL_ENVELOPE__PAYLOAD_RESPONSE)
            {
                amaHandleResponse(control_envelope_in);
            }
            else
            {
                amaHandleCommand(control_envelope_in);
            }

            control_envelope__free_unpacked(control_envelope_in, NULL);

        }

    }
}


static void amaHandleCommand(ControlEnvelope *control_envelope_in)
{

    switch(control_envelope_in->command)
    {
        case COMMAND__GET_DEVICE_INFORMATION:
            amaHandleCommandGetDeviceInformation(control_envelope_in);
            break;

        case COMMAND__GET_DEVICE_CONFIGURATION:
            amaHandleCommandGetDeviceConfiguration(control_envelope_in);
            break;

        case COMMAND__START_SETUP:
            amaHandleCommandStartSetup(control_envelope_in);
            break;

        case COMMAND__COMPLETE_SETUP:
            amaHandleCommandCompleteSetup(control_envelope_in);
            break;

        case COMMAND__UPGRADE_TRANSPORT:
            amaHandleCommandUpgradeTransport(control_envelope_in);
            break;

        case COMMAND__SWITCH_TRANSPORT:
            amaHandleCommandSwitchTransport(control_envelope_in);
            break;

        case COMMAND__GET_STATE:
            amaHandleCommandGetState(control_envelope_in);
            break;

        case COMMAND__SET_STATE:
            amaHandleCommandSetState(control_envelope_in);
            break;

        case  COMMAND__ISSUE_MEDIA_CONTROL:
            amaHandleCommandMediaControl(control_envelope_in);
            break;

        case COMMAND__OVERRIDE_ASSISTANT:
            amaHandleCommandOverrideAssistant(control_envelope_in);
            break;

        case COMMAND__NOTIFY_SPEECH_STATE:
            amaHandleCommandNotifySpeechState(control_envelope_in);
            break;

        case COMMAND__STOP_SPEECH:
            amaHandleCommandStopSpeech(control_envelope_in);
            break;

        case COMMAND__PROVIDE_SPEECH:
            amaHandleCommandProvideSpeech(control_envelope_in);
            break;

        case COMMAND__ENDPOINT_SPEECH:
            amaHandleCommandEndpointSpeech(control_envelope_in);
            break;

        case COMMAND__FORWARD_AT_COMMAND:
            amaHandleCommandForwardATCommand(control_envelope_in);
            break;

        case COMMAND__SYNCHRONIZE_SETTINGS:
            amaHandleCommandSynchronizeSettings(control_envelope_in);
            break;

        case COMMAND__KEEP_ALIVE:
            amaHandleCommandKeepAlive(control_envelope_in);
            break;

        case COMMAND__NONE:
             break;

        case COMMAND__START_SPEECH:
        case COMMAND__GET_CENTRAL_INFORMATION:
        case COMMAND__NOTIFY_DEVICE_CONFIGURATION:
        case COMMAND__RESET_CONNECTION:
        default:
            amaHandleCommandNotHandled(control_envelope_in);
            break;
    }
}

static void amaHandleResponse(ControlEnvelope *control_envelope_in)
{
    Command command = control_envelope_in->command;

    switch(command)
    {
        case COMMAND__START_SPEECH:
            amaHandleResponseStartSpeech(control_envelope_in);
            break;

        case COMMAND__GET_CENTRAL_INFORMATION:
            amaHandleResponseGetCentralInformation(control_envelope_in);
            break;

        case COMMAND__STOP_SPEECH:
        case COMMAND__ENDPOINT_SPEECH:
        case COMMAND__INCOMING_CALL:
        case COMMAND__RESET_CONNECTION:
        case COMMAND__KEEP_ALIVE:
        case COMMAND__SYNCHRONIZE_STATE:
        case COMMAND__NONE:
        default:
            amaHandleResponseNotHandled(control_envelope_in);
            break;
    }
    return;
}

