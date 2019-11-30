/*****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    ama_send_command.c

DESCRIPTION
    Functions to send commands to the phone with functions to handle responses to
    the commands.
*/

#include "accessories.pb-c.h"
#include "ama_send_envelope.h"
#include "ama_send_command.h"
#include "ama_debug.h"
#include "ama_speech.h"

void amaSendCommandStartSpeech(SpeechInitiator__Type speech_initiator,
                                             AudioProfile audio_profile,
                                             AudioFormat audio_format,
                                             AudioSource audio_source,
                                             uint32 start_sample,
                                             uint32 end_sample
)
{
    ControlEnvelope  control_envelope_out = CONTROL_ENVELOPE__INIT;
    control_envelope_out.command = COMMAND__START_SPEECH;
    control_envelope_out.payload_case = CONTROL_ENVELOPE__PAYLOAD_START_SPEECH;
    StartSpeech start_speech = START_SPEECH__INIT;
    SpeechSettings settings = SPEECH_SETTINGS__INIT;

    settings.audio_profile = audio_profile;
    settings.audio_format = audio_format;
    settings.audio_source = audio_source;

    start_speech.settings = &settings;

    SpeechInitiator  initiator = SPEECH_INITIATOR__INIT;

    initiator.type = speech_initiator;

    SpeechInitiator__WakeWord wakeWord = SPEECH_INITIATOR__WAKE_WORD__INIT;

    wakeWord.start_index_in_samples = start_sample;
    wakeWord.end_index_in_samples = end_sample;

    initiator.wake_word = &wakeWord;

    start_speech.initiator = &initiator;

    Dialog dialog = DIALOG__INIT;
    dialog.id = amaSpeechGetCurrentDialogId();

    start_speech.dialog = &dialog;

    control_envelope_out.u.start_speech = &start_speech;

    AMA_DEBUG(("AMA Send COMMAND__START_SPEECH Dialog Id=%d\n", dialog.id));

    amaSendEnvelope(&control_envelope_out);
}


void amaHandleResponseStartSpeech(ControlEnvelope *control_envelope_in)
{
    Response* response = control_envelope_in->u.response;
    Response__PayloadCase response_case = response->payload_case;

    AMA_DEBUG(("AMA COMMAND__START_SPEECH Response "));

    if( response->error_code == ERROR_CODE__SUCCESS)
    {
        switch(response_case)
        {
            case RESPONSE__PAYLOAD__NOT_SET:
                AMA_DEBUG(("No payload\n"));
                break;

            case RESPONSE__PAYLOAD_DIALOG:
                {   /* AMA TO-DO cg11 should we check this against the one we sent ? */
                    AMA_DEBUG(("Dialog ID %d\n", response->u.dialog->id));
                }
                break;

            default:
                AMA_DEBUG(("Unhandled response_case %d\n", response_case));
                AMA_DEBUG(("Dialog ID %d\n", response->u.dialog->id));
                break;
        }
        amaSpeechMessage(ama_speech_msg_start_ack, NULL);

    }
    else
    {
        AMA_DEBUG((" Error!! Code=%d\n", response->error_code));
        amaSpeechMessage(ama_speech_msg_stop, NULL);
    }
}

void amaSendCommandStopSpeech(ErrorCode reason)
{
    ControlEnvelope  controlEnvelopeOut = CONTROL_ENVELOPE__INIT;
    controlEnvelopeOut.command = COMMAND__STOP_SPEECH;
    controlEnvelopeOut.payload_case =  CONTROL_ENVELOPE__PAYLOAD_STOP_SPEECH;

    Dialog dialog = DIALOG__INIT;

    dialog.id = amaSpeechGetCurrentDialogId();

    AMA_DEBUG(("AMA Send COMMAND__STOP_SPEECH Dialog Id=%d\n", dialog.id));

    StopSpeech stop_speech = STOP_SPEECH__INIT;

    stop_speech.dialog = &dialog;
    stop_speech.error_code = reason;
    controlEnvelopeOut.u.stop_speech = &stop_speech;

    amaSendEnvelope(&controlEnvelopeOut);
}

void amaSendCommandEndSpeech(void)
{
    ControlEnvelope  controlEnvelopeOut = CONTROL_ENVELOPE__INIT;
    controlEnvelopeOut.command = COMMAND__ENDPOINT_SPEECH;
    controlEnvelopeOut.payload_case =  CONTROL_ENVELOPE__PAYLOAD_ENDPOINT_SPEECH;

    Dialog dialog = DIALOG__INIT;

    dialog.id = amaSpeechGetCurrentDialogId();

    AMA_DEBUG(("AMA Send COMMAND__ENDPOINT_SPEECH Dialog Id=%d\n", dialog.id));

    EndpointSpeech endpoint_speech = ENDPOINT_SPEECH__INIT;

    endpoint_speech.dialog = &dialog;

    controlEnvelopeOut.u.endpoint_speech = &endpoint_speech;

    amaSendEnvelope(&controlEnvelopeOut);
}

void amaSendCommandIncomingCall(char* caller_number)
{
    ControlEnvelope  controlEnvelopeOut = CONTROL_ENVELOPE__INIT;

    AMA_DEBUG(("AMA Send COMMAND__INCOMING_CALL\n"));

    controlEnvelopeOut.command = COMMAND__INCOMING_CALL;
    controlEnvelopeOut.payload_case =  CONTROL_ENVELOPE__PAYLOAD_INCOMING_CALL;

    IncomingCall incoming_call = INCOMING_CALL__INIT;

    incoming_call.caller_number = caller_number;  /* AMA TODO Revisit for future improvement */

    controlEnvelopeOut.u.incoming_call = &incoming_call;

    amaSendEnvelope(&controlEnvelopeOut);
}


void amaSendCommandKeepAlive(void)
{
    ControlEnvelope  controlEnvelopeOut = CONTROL_ENVELOPE__INIT;

    AMA_DEBUG(("AMA Send COMMAND__KEEP_ALIVE\n"));

    controlEnvelopeOut.command = COMMAND__KEEP_ALIVE;
    controlEnvelopeOut.payload_case =  CONTROL_ENVELOPE__PAYLOAD_KEEP_ALIVE;

    KeepAlive  keep_alive = KEEP_ALIVE__INIT;

    controlEnvelopeOut.u.keep_alive = &keep_alive;

    amaSendEnvelope(&controlEnvelopeOut);
}


void amaSendCommandSyncState(uint32 feature, ama_state_value_case_t value_case, uint16 integer)
{
    ControlEnvelope  controlEnvelopeOut = CONTROL_ENVELOPE__INIT;
    controlEnvelopeOut.command = COMMAND__SYNCHRONIZE_STATE;
    controlEnvelopeOut.payload_case =  CONTROL_ENVELOPE__PAYLOAD_SYNCHRONIZE_STATE;

    State state = STATE__INIT;

    AMA_DEBUG(("AMA Send COMMAND__SYNCHRONIZE_STATE\n"));

    state.feature = feature;
    state.value_case = (State__ValueCase) value_case;

    if(value_case == STATE__VALUE_BOOLEAN)
    {
        state.u.boolean = (protobuf_c_boolean)integer;
    }
    else  if(value_case == STATE__VALUE_INTEGER)
    {
        state.u.boolean = (uint32_t)integer;
    }
    else
    {
        return;
    }

    SynchronizeState synchronize_state = SYNCHRONIZE_STATE__INIT;
    synchronize_state.state = &state;

    controlEnvelopeOut.u.synchronize_state = &synchronize_state;

    amaSendEnvelope(&controlEnvelopeOut);
}


void amaSendCommandGetState(uint32 feature)
{
    ControlEnvelope  controlEnvelopeOut = CONTROL_ENVELOPE__INIT;
    controlEnvelopeOut.command = COMMAND__GET_STATE;
    controlEnvelopeOut.payload_case =  CONTROL_ENVELOPE__PAYLOAD_GET_STATE;

    GetState get_state = GET_STATE__INIT;

    AMA_DEBUG(("AMA Send COMMAND__GET_STATE\n"));

    get_state.feature = feature;

    controlEnvelopeOut.u.get_state = &get_state;

    amaSendEnvelope(&controlEnvelopeOut);
}

void amaSendCommandResetConnection(uint32 timeout, bool force_disconnect)
{
    ControlEnvelope  controlEnvelopeOut = CONTROL_ENVELOPE__INIT;
    controlEnvelopeOut.command = COMMAND__RESET_CONNECTION;
    controlEnvelopeOut.payload_case =  CONTROL_ENVELOPE__PAYLOAD_RESET_CONNECTION;

    ResetConnection reset_connection = RESET_CONNECTION__INIT;

    reset_connection.force_disconnect = force_disconnect;
    reset_connection.timeout = timeout;

    AMA_DEBUG(("AMA Send COMMAND__RESET_CONNECTION\n"));

    controlEnvelopeOut.u.reset_connection = &reset_connection;

    amaSendEnvelope(&controlEnvelopeOut);
}

void amaSendCommandGetCentralInformation(void)
{
    ControlEnvelope  control_envelope_out = CONTROL_ENVELOPE__INIT;
    control_envelope_out.command = COMMAND__GET_CENTRAL_INFORMATION;
    control_envelope_out.payload_case =  CONTROL_ENVELOPE__PAYLOAD_GET_CENTRAL_INFORMATION;

    GetCentralInformation  get_central_information = GET_CENTRAL_INFORMATION__INIT;

    AMA_DEBUG(("AMA Send COMMAND__GET_CENTRAL_INFORMATION\n"));

    control_envelope_out.u.get_central_information = &get_central_information;

    amaSendEnvelope(&control_envelope_out);
}

void amaHandleResponseGetCentralInformation(ControlEnvelope *control_envelope_in)
{   /* Check this. constants.ph-c.h says only the phone can send COMMAND__GET_CENTRAL_INFORMATION */
    /* and we have nothing to send the command??? */
#ifdef DEBUG_AMA_LIB
    Response* response = control_envelope_in->u.response;

    AMA_DEBUG(("AMA COMMAND__GET_CENTRAL_INFORMATION response "));
    if(NULL != response)
    {
        Response__PayloadCase respCase = response->payload_case;
        CentralInformation * centralInformation = response->u.central_information;
        AMA_DEBUG(("Reponse Case %d, code %d, ", respCase, response->error_code));
        if(NULL != centralInformation)
            AMA_DEBUG(("central name %s, platform == %d\n", centralInformation->name, centralInformation->platform));
    }
    
#else
    UNUSED(control_envelope_in);
#endif /* DEBUG_AMA_LIB */

}

void amaHandleResponseNotHandled(ControlEnvelope *control_envelope_in)
{
#ifdef DEBUG_AMA_LIB
    Command command = control_envelope_in->command;
    Response* response = control_envelope_in->u.response;
    Response__PayloadCase response_case = response->payload_case;
    AMA_DEBUG(("AMA Reponse Not handled command = %d, case %d, error code %d == ", command, response_case, response->error_code));
#else
    UNUSED(control_envelope_in);
#endif /* DEBUG_AMA_LIB */
}


