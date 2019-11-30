/*****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    ama_command_handlers.c

DESCRIPTION
    Handlers for ama commands
*/

#include "ama.h"
#include "ama_state.h"
#include "ama_speech.h"
#include "ama_command_handlers.h"
#include "speech.pb-c.h"
#include "ama_debug.h"
#include "ama_send_envelope.h"
#include "ama_transport.h"
#include "ama_send_command.h"
#include "ama_private.h"
#include <string.h>

#define MAKE_DEFAULT_RESPONSE_ENVELOPE(envelope_name, command_id) \
Response response = RESPONSE__INIT;\
response.error_code = ERROR_CODE__SUCCESS;\
ControlEnvelope  envelope_name = CONTROL_ENVELOPE__INIT;\
elementPaster(&envelope_name, command_id , &response);

static void elementPaster(ControlEnvelope* envelope_name,Command command_id, Response* response)
{
    envelope_name->command = command_id;
    envelope_name->u.response = response;
    envelope_name->payload_case = CONTROL_ENVELOPE__PAYLOAD_RESPONSE;
}
static void sendDefaultResponse(Command command);

void amaHandleCommandNotifySpeechState(ControlEnvelope *control_envelope_in)
{
    NotifySpeechState *notify_speech_state = control_envelope_in->u.notify_speech_state;

    AMA_DEBUG(("AMA COMMAND__NOTIFY_SPEECH_STATE received State=%d\n", notify_speech_state->state));

    amaSpeechNotifyStateMsg(notify_speech_state->state);
}


void amaHandleCommandStopSpeech(ControlEnvelope *control_envelope_in)
{
    AMA_DEBUG(("AMA COMMAND__STOP_SPEECH received Error=%d\n",
                    control_envelope_in->u.stop_speech->error_code));

    Dialog *dialog = control_envelope_in->u.stop_speech->dialog;

    if(dialog->id == amaSpeechGetCurrentDialogId())
    {
        amaSpeechMessage(ama_speech_msg_stop, NULL);
    }

    sendDefaultResponse(control_envelope_in->command);
}


void amaHandleCommandGetDeviceInformation(ControlEnvelope *control_envelope_in)
{
    MAKE_DEFAULT_RESPONSE_ENVELOPE(control_envelope_out, control_envelope_in->command);

    Transport supported_transports[NUMBER_OF_SUPPORTED_TRANSPORTS];
    DeviceInformation device_information = DEVICE_INFORMATION__INIT;

    /* Get the AMA device configuration. */
    ama_device_config_t *device_config = amaGetDeviceConfiguration();

    AMA_DEBUG(("AMA COMMAND__GET_DEVICE_INFORMATION received\n"));

    device_information.n_supported_transports = amaTransportGetNumberOfTranports();

    device_information.name = device_config->name;
    device_information.device_type = device_config->device_type;
    device_information.serial_number = device_config->serial_number;

    supported_transports[TRANSPORT__BLUETOOTH_LOW_ENERGY] = TRANSPORT__BLUETOOTH_LOW_ENERGY;
    supported_transports[TRANSPORT__BLUETOOTH_RFCOMM] = TRANSPORT__BLUETOOTH_RFCOMM;
    supported_transports[TRANSPORT__BLUETOOTH_IAP] = TRANSPORT__BLUETOOTH_IAP;

    device_information.supported_transports = &supported_transports[0];

    /* assign resposne union type */
    response.u.device_information = &device_information;

    response.payload_case = RESPONSE__PAYLOAD_DEVICE_INFORMATION;

    amaSendEnvelope(&control_envelope_out);

}


void amaHandleCommandGetDeviceConfiguration(ControlEnvelope *control_envelope_in)
{
    MAKE_DEFAULT_RESPONSE_ENVELOPE(control_envelope_out, control_envelope_in->command);

    AMA_DEBUG(("AMA COMMAND__GET_DEVICE_CONFIGURATION received\n"));

    DeviceConfiguration device_config = DEVICE_CONFIGURATION__INIT;

    device_config.needs_assistant_override = FALSE;
    device_config.needs_setup = TRUE;

    /* assign response union type */
    response.u.device_configuration = &device_config;

    response.payload_case = RESPONSE__PAYLOAD_DEVICE_CONFIGURATION;
    amaSendEnvelope(&control_envelope_out);

}


void amaHandleCommandStartSetup(ControlEnvelope *control_envelope_in)
{
    AMA_DEBUG(("AMA COMMAND__START_SETUP received\n"));
    sendDefaultResponse(control_envelope_in->command);
}

void amaHandleCommandCompleteSetup(ControlEnvelope *control_envelope_in)
{
    AMA_DEBUG(("AMA COMMAND__COMPLETE_SETUP received\n"));

    sendDefaultResponse(control_envelope_in->command);
}

static void bdaddrToArray(uint8 *array, bdaddr *bdaddr_in)
{
    array[1] = (uint8)(bdaddr_in->nap & 0xff);
    array[0] = (uint8)((bdaddr_in->nap>>8) & 0xff);
    array[2] = bdaddr_in->uap;
    array[5] = (uint8)(bdaddr_in->lap) & 0xff;
    array[4] = (uint8)(bdaddr_in->lap>>8) & 0xff ;
    array[3] = (uint8)(bdaddr_in->lap>>16) & 0xff ;
}

void amaHandleCommandUpgradeTransport(ControlEnvelope *control_envelope_in)
{
    uint8 bdaddr_array[6];
    MAKE_DEFAULT_RESPONSE_ENVELOPE(control_envelope_out, control_envelope_in->command);
    ConnectionDetails connection_details = CONNECTION_DETAILS__INIT;
    UpgradeTransport *upgrade_transport = control_envelope_in->u.upgrade_transport;

    AMA_DEBUG(("AMA COMMAND__UPGRADE_TRANSPORT received. Transport=%d\n", upgrade_transport->transport));

    bdaddrToArray(bdaddr_array, amaGetLocalAddress());

    amaSendSinkMessage(AMA_ENABLE_CLASSIC_PAIRING_IND, NULL);

    connection_details.identifier.len = 6;
    connection_details.identifier.data = bdaddr_array;

    AmaTransportUpgrade((ama_transport_t)(upgrade_transport->transport));

    response.payload_case = RESPONSE__PAYLOAD_CONNECTION_DETAILS;

    response.u.connection_details = &connection_details;

    amaSendSinkMessage(AMA_UPGRADE_TRANSPORT_IND, NULL);

    amaSendEnvelope(&control_envelope_out);
}

void amaHandleCommandSwitchTransport(ControlEnvelope *control_envelope_in)
{

    MAKE_DEFAULT_RESPONSE_ENVELOPE(control_envelope_out, control_envelope_in->command);

    SwitchTransport* switch_transport = control_envelope_in->u.switch_transport;

    AMA_DEBUG(("AMA COMMAND__SWITCH_TRANSPORT received Transport=%d\n", switch_transport->new_transport));

    amaSendEnvelope(&control_envelope_out);

    /* AMA TODO ... Revisit for future improvement */
    amaSendCommandGetCentralInformation();

    AmaTransportSwitch((ama_transport_t)switch_transport->new_transport);
}


void amaHandleCommandSynchronizeSettings(ControlEnvelope *control_envelope_in)
{
    MAKE_DEFAULT_RESPONSE_ENVELOPE(control_envelope_out, control_envelope_in->command);

    AMA_DEBUG(("AMA COMMAND__SYNCHRONIZE_SETTINGS received\n"));

    amaSendEnvelope(&control_envelope_out);

    amaSendCommandGetCentralInformation();

    amaSpeechMessage(ama_speech_msg_ready_to_sync_settings, NULL);
}

void amaHandleCommandGetState(ControlEnvelope *control_envelope_in)
{
    MAKE_DEFAULT_RESPONSE_ENVELOPE(control_envelope_out, control_envelope_in->command);

    GetState *get_state = control_envelope_in->u.get_state;
    uint32_t feature = get_state->feature;
    uint32 state_value;
    State__ValueCase value_case;

    AMA_DEBUG(("AMA COMMAND__GET_STATE feature %x\n", feature));

    State state = STATE__INIT;

    response.error_code = (ErrorCode) amaLibGetState(feature, &state_value, (ama_state_value_case_t*)&value_case);

    state.value_case = value_case;
    state.feature = feature;

    if(state.value_case == STATE__VALUE_BOOLEAN)
    {
        state.u.boolean = (protobuf_c_boolean)state_value;
    }
    else if(state.value_case == STATE__VALUE_INTEGER)
    {
        state.u.integer = (uint32_t)state_value;
    }

    response.payload_case = RESPONSE__PAYLOAD_STATE;
    response.u.state = &state;

    amaSendEnvelope(&control_envelope_out);
}


void amaHandleCommandSetState(ControlEnvelope *control_envelope_in)
{
    MAKE_DEFAULT_RESPONSE_ENVELOPE(control_envelope_out, control_envelope_in->command);

    SetState *set_state = control_envelope_in->u.set_state;
    uint32 feature = (uint32)set_state->state->feature;
    State__ValueCase value_case = set_state->state->value_case;
    uint32 state_value = 0xFFFF;

    if(value_case == STATE__VALUE_BOOLEAN)
    {
        state_value = (uint32)set_state->state->u.boolean;
    }
    else if (value_case == STATE__VALUE_INTEGER)
    {
        state_value = (uint32)set_state->state->u.integer;
    }

    AMA_DEBUG(("AMA COMMAND__SET_STATE received feature %x valcase %d val %d\n", feature, value_case, state_value));

    response.error_code = (ErrorCode) amaLibSetState(feature, state_value, (ama_state_value_case_t)value_case);

    amaSendEnvelope(&control_envelope_out);

}

void amaHandleCommandMediaControl(ControlEnvelope *control_envelope_in)
{

    IssueMediaControl *issue_media_control = control_envelope_in->u.issue_media_control;
    MediaControl control =  issue_media_control->control;

    AMA_DEBUG(("AMA COMMAND__ISSUE_MEDIA_CONTROL received control=%d\n", control));

    amaMediaControl((AMA_MEDIA_CONTROL) control);
    sendDefaultResponse(control_envelope_in->command);

}

void amaHandleCommandOverrideAssistant(ControlEnvelope *control_envelope_in)
{
    MAKE_DEFAULT_RESPONSE_ENVELOPE(control_envelope_out, control_envelope_in->command);

    AMA_DEBUG(("AMA COMMAND__OVERRIDE_ASSISTANT received\n"));

    response.error_code = ERROR_CODE__UNSUPPORTED;
    amaSendEnvelope(&control_envelope_out);
}

void amaHandleCommandProvideSpeech(ControlEnvelope *control_envelope_in)
{
    MAKE_DEFAULT_RESPONSE_ENVELOPE(control_envelope_out, control_envelope_in->command);

    ProvideSpeech *provide_speech =control_envelope_in->u.provide_speech;

    Dialog* dialog = provide_speech->dialog;
    Dialog  dialog_response = DIALOG__INIT;

    ama_speech_msg_provide_speech_t provide_speech_message;
    provide_speech_message.dialog_id = dialog->id;

    AMA_DEBUG(("AMA COMMAND__PROVIDE_SPEECH - dialog id =%d\n", dialog->id));

    SpeechSettings settings = SPEECH_SETTINGS__INIT;
    settings.audio_profile = amaSpeechGetAudioProfile();
    settings.audio_format  = amaSpeechGetAudioFormat();
    settings.audio_source  = amaSpeechGetAudioSource();

    SpeechProvider speech_provider = SPEECH_PROVIDER__INIT;
    speech_provider.speech_settings = &settings;
    speech_provider.dialog = &dialog_response;

    response.payload_case = RESPONSE__PAYLOAD_SPEECH_PROVIDER;
    response.u.speech_provider = &speech_provider;

    if(amaSpeechMessage(ama_speech_msg_provide_speech, &provide_speech_message))
    {
        dialog_response.id = amaSpeechGetCurrentDialogId();
        AMA_DEBUG(("AMA Responding success to dialog id =%d\n", dialog_response.id));
    }
    else
    {
        dialog_response.id = dialog->id;
        response.error_code = ERROR_CODE__BUSY;
        AMA_DEBUG(("AMA Responding busy - dialog id =%d\n", dialog_response.id));
    }


    amaSendEnvelope(&control_envelope_out);

}

void amaHandleCommandEndpointSpeech(ControlEnvelope *control_envelope_in)
{

    AMA_DEBUG(("AMA COMMAND__ENDPOINT_SPEECH "));

    if(control_envelope_in->payload_case == CONTROL_ENVELOPE__PAYLOAD_ENDPOINT_SPEECH)
    {
        EndpointSpeech* endpoint_speech = control_envelope_in->u.endpoint_speech;
        Dialog *dialog = endpoint_speech->dialog;

        AMA_DEBUG(("Rx Dialog ID %d\n", dialog->id));
        if(dialog->id == amaSpeechGetCurrentDialogId())
        {
            amaSpeechMessage(ama_speech_msg_stop, NULL);
        }
        else
        {
            AMA_DEBUG(("AMA Dialog Id incorrect. Received %d, should be %d\n",
                        dialog->id,
                        amaSpeechGetCurrentDialogId()));
        }
    }
    else if(control_envelope_in->payload_case == CONTROL_ENVELOPE__PAYLOAD_NOTIFY_SPEECH_STATE)
    {
        /* probably we get this case if send end speech when there is no speech going on */
        NotifySpeechState * notify_speech_state = control_envelope_in->u.notify_speech_state;
        SpeechState state = notify_speech_state->state;
        AMA_DEBUG(("NOTIFY_SPEECH_STATE %d\n", state));
        amaSpeechNotifyStateMsg(state);
    }
    else
    {
        AMA_DEBUG(("unexpected payload case %d\n", control_envelope_in->payload_case));
    }

    sendDefaultResponse(control_envelope_in->command);

}


static ErrorCode processForwardAtCommand(char* command)
{

typedef struct{
    char* at_string;
    ama_at_cmd_t command;
}at_lookup_t;

static const at_lookup_t  at_lookup[] = {
    {"ATA",           ama_at_cmd_ata_ind},
    {"AT+CHUP",       ama_at_cmd_at_plus_chup_ind},
    {"AT+BLDN",       ama_at_cmd_at_plus_bldn_ind},
    {"AT+CHLD=0",     ama_at_cmd_at_plus_chld_eq_0_ind},
    {"AT+CHLD=1",     ama_at_cmd_at_plus_chld_eq_1_ind},
    {"AT+CHLD=2",     ama_at_cmd_at_plus_chld_eq_2_ind},
    {"AT+CHLD=3",     ama_at_cmd_at_plus_chld_eq_3_ind},
    {"ATD",           ama_at_cmd_atd_ind}
};

    uint8 num_of_commands = sizeof(at_lookup) / sizeof(at_lookup[0]);
    uint8 index;

    for(index = 0; index < num_of_commands; index++)
    {
        if(strcmp(at_lookup[index].at_string, command) == 0)
        {
            MAKE_AMA_MESSAGE(AMA_SEND_AT_COMMAND_IND);

            message->at_command = at_lookup[index].command;

            amaSendSinkMessage(AMA_SEND_AT_COMMAND_IND, message);

            return ERROR_CODE__SUCCESS;
        }
    }

    return ERROR_CODE__UNKNOWN;
}


void amaHandleCommandForwardATCommand(ControlEnvelope *control_envelope_in)
{
    MAKE_DEFAULT_RESPONSE_ENVELOPE(control_envelope_out, control_envelope_in->command);

    ForwardATCommand* forward_at_command = control_envelope_in->u.forward_at_command;

    char* forward_command = forward_at_command->command;

    AMA_DEBUG(("AMA COMMAND__FORWARD_AT_COMMAND received Command %s\n", forward_command));

    response.error_code = processForwardAtCommand(forward_command);

    amaSendEnvelope(&control_envelope_out);
}

void amaHandleCommandNotHandled(ControlEnvelope *control_envelope_in)
{
    MAKE_DEFAULT_RESPONSE_ENVELOPE(control_envelope_out, control_envelope_in->command);
    AMA_DEBUG(("AMA unhandled command!! %d\n", control_envelope_in->command));
    response.error_code = ERROR_CODE__UNSUPPORTED;
    amaSendEnvelope(&control_envelope_out);
}


void amaHandleCommandKeepAlive(ControlEnvelope *control_envelope_in)
{
    AMA_DEBUG(("AMA COMMAND__KEEP_ALIVE received\n"));
    sendDefaultResponse(control_envelope_in->command);
}


static void sendDefaultResponse(Command command)
{
    MAKE_DEFAULT_RESPONSE_ENVELOPE(control_envelope_out, command);
    amaSendEnvelope(&control_envelope_out);
}

