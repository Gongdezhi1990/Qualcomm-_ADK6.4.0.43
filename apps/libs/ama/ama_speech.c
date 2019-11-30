#include "speech.pb-c.h"

#include "ama_speech.h"
#include "ama_send_command.h"
#include <panic.h>
#include "ama_debug.h"
#include "voice_assistant_audio_manager.h"
#include "message.h"

typedef enum{
    speech_state_uninitialised,
    speech_state_initialised,
    speech_state_idle,
    speech_state_sending,
    speech_state_top
} speech_state_t;

#ifdef DEBUG_AMA_LIB
const char *state_strings[] = {
    "speech_state_uninitialised",
    "speech_state_initialised",
    "speech_state_idle",
    "speech_state_sending"
};

static const char *message_strings[] = {
    "ama_speech_msg_initialise",
    "ama_speech_msg_start",
    "ama_speech_msg_start_ack",
    "ama_speech_msg_stop",
    "ama_speech_msg_ready_to_sync_settings",
    "ama_speech_msg_response_timeout",
    "ama_speech_msg_provide_speech",
    "ama_speech_msg_suspend",
    "ama_speech_msg_resume",
    "ama_speech_msg_end"
};
#endif /* DEBUG_AMA_LIB */

typedef struct{
    SpeechInitiator__Type speech_initiator;
    AudioProfile audio_profile;
    AudioFormat audio_format;
    AudioSource audio_source;
    speech_state_t speech_state;
    bool    audio_enabled;
    uint32 new_dialog_id;
    uint32 current_dialog_id;
}speech_settings_t;

#define MAX_START_DIALOG_ID 0x7fffffff

#define AMA_SPEECH_INITIATOR_DEFAULT        ama_speech_initiator_tap
#define AMA_SPEECH_AUDIO_PROFILE_DEFAULT    ama_audio_profile_near_field
#define AMA_SPEECH_AUDIO_FORMAT_DEFAULT     ama_audio_format_msbc
#define AMA_SPEECH_AUDIO_SOURCE_DEFAULT     ama_audio_source_stream


static speech_settings_t speech_settings = {
    AMA_SPEECH_INITIATOR_DEFAULT,
    AMA_SPEECH_AUDIO_PROFILE_DEFAULT,
    AMA_SPEECH_AUDIO_FORMAT_DEFAULT,
    AMA_SPEECH_AUDIO_SOURCE_DEFAULT,
    speech_state_uninitialised,
    FALSE,
    MAX_START_DIALOG_ID,
    0
};

typedef enum{
    SPEECH_MESSAGE_TIMEOUT,
    SPEECH_MESSAGE_TOP
}SPEECH_MESSAGE_T;


static void setSpeechState(speech_state_t state);
static speech_state_t getSpeechState(void);
static bool handleSpeechMsgStateUnInit(ama_speech_msg_t msg_id, void *msg);
static bool handleSpeechMsgStateInit(ama_speech_msg_t msg_id, void *msg);
static bool handleSpeechMsgStateIdle(ama_speech_msg_t msg_id, void *msg);
static bool handleSpeechMsgStateSending(ama_speech_msg_t msg_id, void *msg);
static void initialiseSpeech(void);
static bool handleSpeechStart(ama_speech_msg_start_t *msg);
static bool handleSpeechProvide(ama_speech_msg_provide_speech_t *msg);
static bool handleSpeechStop(ama_speech_msg_stop_t *msg);
static void setSpeechInitiator(ama_speech_initiator_t initiator);
static void speechMessageHandler(Task task, MessageId id, Message message);
static void handleSpeechMessageTimeout(void);
static void setupResponseTimeoutMsg(void);
static void updateProvidedDialogId(uint32 provided_id);
static void newDialogId(void);
static void handleSpeechStartAck(void);



static const TaskData speechTaskData = {speechMessageHandler};


void amaSpeechSetAudioSource(ama_audio_source_t source)
{
    switch(source)
    {
        case ama_audio_source_stream:
            speech_settings.audio_source = AUDIO_SOURCE__STREAM;
            break;

        case ama_audio_source_bluetooth_sco:
            speech_settings.audio_source = AUDIO_SOURCE__BLUETOOTH_SCO;
            break;

        default:
            Panic();
            break;
    }
}

void amaSpeechSetAudioFormat(ama_audio_format_t format)
{
    switch(format)
    {
        case ama_audio_format_pcm_l16_16khz_mono:
            speech_settings.audio_format = AUDIO_FORMAT__PCM_L16_16KHZ_MONO;
            break;

        case ama_audio_format_opus_16khz_32kbps_cbr_0_20ms:
            speech_settings.audio_format = AUDIO_FORMAT__OPUS_16KHZ_32KBPS_CBR_0_20MS;
            break;

        case ama_audio_format_opus_16khz_16kbps_cbr_0_20ms:
            speech_settings.audio_format = AUDIO_FORMAT__OPUS_16KHZ_16KBPS_CBR_0_20MS;
            break;

        case ama_audio_format_msbc:
            speech_settings.audio_format = AUDIO_FORMAT__MSBC;
            break;

        default:
            Panic();
            break;
    }
}

void amaSpeechSetAudioProfile(ama_audio_profile_t profile)
{
    switch(profile)
    {
        case ama_audio_profile_close_talk:
            speech_settings.audio_profile = AUDIO_PROFILE__CLOSE_TALK;
            break;

        case ama_audio_profile_near_field:
            speech_settings.audio_profile = AUDIO_PROFILE__NEAR_FIELD;
            break;

        case ama_audio_profile_far_field:
            speech_settings.audio_profile = AUDIO_PROFILE__FAR_FIELD;
            break;

        default:
            Panic();
            break;
    }
}



static void setSpeechInitiator(ama_speech_initiator_t initiator)
{
    switch(initiator)
    {
        case ama_speech_initiator_none:
            speech_settings.speech_initiator = SPEECH_INITIATOR__TYPE__NONE;
            break;

        case ama_speech_initiator_press_and_hold:
            speech_settings.speech_initiator = SPEECH_INITIATOR__TYPE__PRESS_AND_HOLD;
            break;

        case ama_speech_initiator_tap:
            speech_settings.speech_initiator = SPEECH_INITIATOR__TYPE__TAP;
            break;

        case ama_speech_initiator_wakeword:
            speech_settings.speech_initiator = SPEECH_INITIATOR__TYPE__WAKEWORD;
            break;

        default:
            break;
    }
}

AudioSource amaSpeechGetAudioSource(void)
{
    return speech_settings.audio_source;
}

AudioFormat amaSpeechGetAudioFormat(void)
{
    return speech_settings.audio_format;
}

AudioProfile amaSpeechGetAudioProfile(void)
{
    return speech_settings.audio_profile;
}

bool amaSpeechMessage(ama_speech_msg_t msg_id, void *msg)
{
    AMA_DEBUG(("AMA Speech Message=%s State=%s\n", message_strings[msg_id], state_strings[getSpeechState()]));

    switch(getSpeechState())
    {
        case speech_state_uninitialised:
            return handleSpeechMsgStateUnInit(msg_id, msg);

        case speech_state_initialised:
            return handleSpeechMsgStateInit(msg_id, msg);

        case speech_state_idle:
            return handleSpeechMsgStateIdle(msg_id, msg);

        case speech_state_sending:
            return handleSpeechMsgStateSending(msg_id, msg);

        default:
            break;
    }
    return FALSE;
}

bool amaValidStateToSendSpeech(void)
{
   return (getSpeechState()== speech_state_sending);
}

void amaSpeechNotifyStateMsg(SpeechState state)
{
    ama_speech_state_t speech_state = ama_speech_state_err;

    switch(state)
    {
        case SPEECH_STATE__IDLE:
            speech_state = ama_speech_state_idle;
            break;

        case SPEECH_STATE__LISTENING:
            speech_state = ama_speech_state_listening;
            break;

        case SPEECH_STATE__PROCESSING:
            speech_state = ama_speech_state_processing;
            break;

        case SPEECH_STATE__SPEAKING:
            speech_state = ama_speech_state_speaking;
            break;

        default:
            AMA_DEBUG(("AMA Unknown speech state indicated%d\n", state));
            break;
    }

    if(speech_state != ama_speech_state_err)
    {
        MAKE_AMA_MESSAGE(AMA_SPEECH_STATE_IND);
        message->speech_state = speech_state;
        amaSendSinkMessage(AMA_SPEECH_STATE_IND, message);
    }
}
static void amaEnableAudio(void)
{
    if(!speech_settings.audio_enabled)
    {
        VaAudioMgrEnable();
        VaAudioMgrActivateTrigger();
        speech_settings.audio_enabled = TRUE;
    }
}
static void amaDisableAudio(bool deactivate_trigger)
{
    if(speech_settings.audio_enabled)
    {
        if(deactivate_trigger)
        {
            VaAudioMgrDeactivateTrigger();
        }
        VaAudioMgrDisable();
        speech_settings.audio_enabled  = FALSE;
     }
}

static bool handleSpeechMsgStateUnInit(ama_speech_msg_t msg_id, void *msg)
{
    UNUSED(msg);
    switch(msg_id)
    {
        case ama_speech_msg_initialise:
            initialiseSpeech();
            return TRUE;

        default:
            AMA_DEBUG(("AMA Speech Message Unhandled!!\n"));
            break;
    }
    return FALSE;
}

static bool handleSpeechMsgStateInit(ama_speech_msg_t msg_id, void *msg)
{
    UNUSED(msg);
    switch(msg_id)
    {
        case ama_speech_msg_ready_to_sync_settings:
            setSpeechState(speech_state_idle);
            amaEnableAudio();
            return TRUE;

        case ama_speech_msg_initialise:
            initialiseSpeech();
            return TRUE;

        case ama_speech_msg_start:
        case ama_speech_msg_stop:
        default:
            AMA_DEBUG(("AMA Speech Message Unhandled!!\n"));
            break;
    }
    return FALSE;
}


static bool handleSpeechMsgStateIdle(ama_speech_msg_t msg_id, void *msg)
{
    switch(msg_id)
    {
        case ama_speech_msg_start:
            return handleSpeechStart((ama_speech_msg_start_t*)msg);

        case ama_speech_msg_initialise:
            amaDisableAudio(TRUE);
            initialiseSpeech();
            return TRUE;

        case ama_speech_msg_resume:
            amaEnableAudio();
            return TRUE;

        case ama_speech_msg_provide_speech:
            return handleSpeechProvide((ama_speech_msg_provide_speech_t*)msg);

        case ama_speech_msg_suspend:
            amaSendCommandStopSpeech(ERROR_CODE__USER_CANCELLED);
            amaDisableAudio(TRUE);
            return TRUE;

        case ama_speech_msg_stop:
            amaSendCommandStopSpeech(ERROR_CODE__USER_CANCELLED);
            return TRUE;

        case ama_speech_msg_end:
        case ama_speech_msg_ready_to_sync_settings:
        default:
            AMA_DEBUG(("AMA Speech Message Unhandled!!\n"));
            break;
    }
    return FALSE;
}

static bool handleSpeechMsgStateSending(ama_speech_msg_t msg_id, void *msg)
{
    switch(msg_id)
    {
        case ama_speech_msg_stop:
        case ama_speech_msg_response_timeout:
            setSpeechState(speech_state_idle);
            return handleSpeechStop(msg);

       case ama_speech_msg_start_ack:
            handleSpeechStartAck();
            return TRUE;

       case ama_speech_msg_initialise:
           VaAudioMgrStopCapture();
           amaDisableAudio(FALSE);
           initialiseSpeech();
           return TRUE;

        case ama_speech_msg_end:
            amaSendCommandEndSpeech();
            VaAudioMgrStopCapture();
            VaAudioMgrActivateTrigger();
            setSpeechState(speech_state_idle);
            return TRUE;

        case ama_speech_msg_suspend:
            VaAudioMgrStopCapture();
            amaSendCommandStopSpeech(ERROR_CODE__USER_CANCELLED);
            amaDisableAudio(FALSE);
            setSpeechState(speech_state_idle);
            return TRUE;


       case ama_speech_msg_start:
       case ama_speech_msg_ready_to_sync_settings:
       default:
            AMA_DEBUG(("AMA Speech Message Unhandled!!\n"));
            break;
    }
    return FALSE;
}

static void setSpeechState(speech_state_t state)
{
    AMA_DEBUG(("AMA Set Speech State old=%s new=%s\n ", state_strings[speech_settings.speech_state], state_strings[state]));

    speech_settings.speech_state = state;
}

static speech_state_t getSpeechState(void)
{
    return  speech_settings.speech_state;
}

static void initialiseSpeech(void)
{
    amaSpeechSetAudioSource(AMA_SPEECH_AUDIO_SOURCE_DEFAULT);
    amaSpeechSetAudioFormat(AMA_SPEECH_AUDIO_FORMAT_DEFAULT);
    amaSpeechSetAudioProfile(AMA_SPEECH_AUDIO_PROFILE_DEFAULT);
    setSpeechInitiator(AMA_SPEECH_INITIATOR_DEFAULT);
    setSpeechState(speech_state_initialised);
    speech_settings.new_dialog_id = 0;
    speech_settings.current_dialog_id = 0;
    speech_settings.audio_enabled = FALSE;
}


static bool handleSpeechStart(ama_speech_msg_start_t *msg)
{

    if(msg)
    {
        uint32 start_timestamp = 0;
        uint32 start_sample = 0;
        uint32 end_sample = 0;

        setSpeechInitiator(msg->initiator);

        if(msg->initiator == ama_speech_initiator_press_and_hold)
        {
            amaSpeechSetAudioProfile(ama_audio_profile_close_talk);
        }
        else
        {
            amaSpeechSetAudioProfile(AMA_SPEECH_AUDIO_PROFILE_DEFAULT);
        }
        newDialogId();

        if(msg->initiator == ama_speech_initiator_wakeword)
        {
            #define SAMPLES_MS 16

            uint32 tp_len;

            start_timestamp = msg->start_timestamp;
            start_sample = (msg->pre_roll/1000) * SAMPLES_MS;

            if(msg->end_timestamp >= msg->start_timestamp)
            {
                tp_len = msg->end_timestamp - msg->start_timestamp;
            }
            else
            {
                tp_len = msg->end_timestamp + (~(msg->start_timestamp)) + 1;
            }

            end_sample = ((tp_len/1000) * SAMPLES_MS) + start_sample;

        }

        amaSendCommandStartSpeech(speech_settings.speech_initiator,
                                  speech_settings.audio_profile,
                                  speech_settings.audio_format,
                                  speech_settings.audio_source,
                                  start_sample,
                                  end_sample);

        VaAudioMgrDeactivateTrigger();

        VaAudioMgrStartCapture(start_timestamp);

        setSpeechState(speech_state_sending);

        setupResponseTimeoutMsg();

        return TRUE;
    }
    return FALSE;
}

static bool handleSpeechProvide(ama_speech_msg_provide_speech_t *msg)
{
    if(msg)
    {
        updateProvidedDialogId(msg->dialog_id);
        VaAudioMgrDeactivateTrigger();
        VaAudioMgrStartCapture(0);
        setSpeechState(speech_state_sending);
        return TRUE;
    }
    return FALSE;
}

static bool handleSpeechStop(ama_speech_msg_stop_t *msg)
{
    VaAudioMgrStopCapture();

    if(msg && msg->send_end)
    {
        amaSendCommandStopSpeech(ERROR_CODE__USER_CANCELLED);
    }
    VaAudioMgrActivateTrigger();
    return TRUE;
}

static void handleSpeechStartAck(void)
{
    MessageCancelAll((Task)&speechTaskData, SPEECH_MESSAGE_TIMEOUT);
}

static void setupResponseTimeoutMsg(void)
{
#define START_SPEECH_RESPONSE_TIMEOUT D_SEC(2)

    MessageSendLater((Task)&speechTaskData, SPEECH_MESSAGE_TIMEOUT, NULL, START_SPEECH_RESPONSE_TIMEOUT);

}

static void speechMessageHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(id);
    UNUSED(message);

    switch(id)
    {
        case SPEECH_MESSAGE_TIMEOUT:
            handleSpeechMessageTimeout();
            break;

        default:
            AMA_DEBUG(("AMA Unexpected Msg in speechMessageHandler\n"));
            Panic();
            break;
    }
}

static void handleSpeechMessageTimeout(void)
{
    amaSpeechMessage(ama_speech_msg_response_timeout, NULL);
}

uint32 amaSpeechGetCurrentDialogId(void)
{
    return speech_settings.current_dialog_id;
}

static void newDialogId(void)
{
    if(speech_settings.new_dialog_id == MAX_START_DIALOG_ID)
    {
        speech_settings.new_dialog_id = 0;
    }
    else
    {
        speech_settings.new_dialog_id++;
    }
    speech_settings.current_dialog_id = speech_settings.new_dialog_id;
}

static void updateProvidedDialogId(uint32 provided_id)
{
    if(provided_id > MAX_START_DIALOG_ID)
    {
        speech_settings.current_dialog_id = provided_id;
    }
}


