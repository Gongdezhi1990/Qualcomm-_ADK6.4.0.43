/*
Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief
    Speech Recognition application interface.

NOTES

    This enables and disables the qtil speech recognition plugin and
    converts the response into sink device user events.

    Speech Recognition can be used for answering / rejecting a call and is
    enabled when the sink device event EventSysSpeechRecognitionStart is received at
    the application event handler. This calls speechRecognitionStart()
    which starts the speech recognition engine and registers a const task for the
    callback message.

    Once the callback is received, this is converted into an applcication message
    (user event) which actions the sink device application.

    No globals used
    1 bit required in sink_private_data.h for storing whther the speech rec is active
    or not - this is in turn used (via speechRecognitionIsActive() ) for the
    sink_audio.c code to decide whether or not to process any audio stream
    connections
*/

/****************************************************************************
    Header files
*/
#include "sink_speech_recognition.h"
#include "sink_configmanager.h"
#include "sink_audio.h"
#include "sink_statemanager.h"
#include "sink_audio_routing.h"
#include "sink_hfp_data.h"

#ifdef ENABLE_SPEECH_RECOGNITION

#include "sink_speech_rec_config_def.h"
#include "sink_powermanager.h"
#include "sink_main_task.h"

#ifdef ENABLE_GAIA
#include "sink_gaia.h"
#endif

#include <hfp.h>
#include <audio_plugin_voice_variants.h>
#include <audio.h>
#include <config_store.h>

#ifdef DEBUG_SPEECH_REC
#define SR_DEBUG(x) DEBUG(x)
#else
#define SR_DEBUG(x)
#endif

/*
    Global data required by Speech recognition module
*/
typedef struct
{
    unsigned    csr_speech_recognition_is_active:1;
    unsigned    csr_speech_recognition_tuning_active:1;
    unsigned    ssr_enabled:1;
    unsigned    unused:13;

} speech_rec_global_data_t;

/* Global data structure element for speech recognition module */
static speech_rec_global_data_t gSRData;
#define GSRDATA  gSRData

static bool speech_rec_call_to_answer(void);

static void speech_rec_handler(Task task, MessageId id, Message message);

/* task for messages to be delivered */
static const TaskData speechRecTask = {speech_rec_handler};


/****************************************************************************
DESCRIPTION
    This function used to retrive repeat timer
*/
uint16 speechRecognitionGetRepeatTime(void)
{
    uint16 sr_repeat_timer = 0;
    speech_rec_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetReadOnlyConfig(SPEECH_REC_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        sr_repeat_timer = read_configdata->speechRecRepeatTime_ms;
        configManagerReleaseConfig(SPEECH_REC_READONLY_CONFIG_BLK_ID);
    }

    return sr_repeat_timer;
}

bool speechRecognitionSetRepeatTime(uint16 time)
{
    speech_rec_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetWriteableConfig(SPEECH_REC_READONLY_CONFIG_BLK_ID, (void **)&read_configdata, 0))
    {
        read_configdata->speechRecRepeatTime_ms = time;
        configManagerUpdateWriteableConfig(SPEECH_REC_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/****************************************************************************
DESCRIPTION
    This function used to retrive if Speech recognition is enabled or not
*/
bool speechRecognitionFeatureEnabled(void)
{
    bool enabled = 0;
    speech_rec_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetReadOnlyConfig(SPEECH_REC_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        enabled = read_configdata->speech_rec_enabled;
        configManagerReleaseConfig(SPEECH_REC_READONLY_CONFIG_BLK_ID);
    }

    return enabled;
}

static void connectSpeechRecognitionPlugin(void)
{
    AudioConnect (AudioPluginVoiceVariantsGetAsrPlugin(narrow_band, sinkHfpDataGetAudioPlugin()),
                           sinkAudioGetRoutedAudioSink()  ,
                           AUDIO_SINK_SCO ,
                           (int16)sinkHfpDataGetDefaultVolume() ,
                           8000 ,
                           sinkAudioGetPluginFeatures(),
                           AUDIO_MODE_CONNECTED,
                           AUDIO_ROUTE_INTERNAL,
                           powerManagerGetLBIPM(),
                           sinkHfpDataGetHfpPluginParams(),
                           (TaskData*)&speechRecTask ) ;
}

/****************************************************************************
DESCRIPTION
  	This function is called to enable speech recognition mode
*/
void speechRecognitionStart(void)
{
    /* if not already running, start asr */
    if ( !speechRecognitionIsActive())
    {
        SR_DEBUG(("Speech Rec START - AudioConnect\n")) ;

        speechRecSetIsActive(TRUE);

        /*reconnect any audio streams that may need connecting*/
        audioSuspendDisconnectAudioSource();

        connectSpeechRecognitionPlugin();

    }
    /* already running, just restart timeout timer */
    else
    {
       SR_DEBUG(("Speech Rec START - restart detected\n")) ;
    }

    /*post a timeout message to restart the SR if no recognition occurs*/
    if(!speechRecognitionTuningIsActive())
    {
        MessageSendLater((TaskData*)&speechRecTask , CSR_SR_APP_TIMEOUT , 0, speechRecognitionGetRepeatTime()) ;
    }
}

/****************************************************************************
DESCRIPTION
  	This function is called to reenable speech recognition mode
*/
void speechRecognitionReStart(void)
{
    SR_DEBUG(("Speech Rec RESTART - AudioConnect\n")) ;

    speechRecSetIsActive(TRUE);

    connectSpeechRecognitionPlugin();

}
/****************************************************************************
DESCRIPTION
  	This function is called to disable speech recognition mode
*/
void speechRecognitionStop(void)
{
    SR_DEBUG(("Speech Rec STOP\n")) ;

    /*if the SR plugin is attached / disconnect it*/
    if (speechRecognitionIsActive())
    {
        SR_DEBUG(("Disconnect SR Plugin\n")) ;
        audioDisconnectRoutedAudio();
    }

    speechRecSetIsActive(FALSE);

    /*cancel any potential APP timeout message */
    MessageCancelAll( (TaskData*)&speechRecTask , CSR_SR_APP_TIMEOUT ) ;

    /*cancel any potential APP restart message */
    MessageCancelAll( (TaskData*)&speechRecTask , CSR_SR_APP_RESTART) ;

    /* cancel any queued start messages */
    MessageCancelAll (&theSink.task , EventSysSpeechRecognitionStart) ;

    /*reconnect any audio streams that may need connecting*/
    audioUpdateAudioRouting();

}

/****************************************************************************
DESCRIPTION
    This function is called to determine if speech rec is currently running
RETURNS
    True if Speech Rec is active
*/
bool speechRecognitionIsActive(void)
{
    SR_DEBUG(("Speech Rec ACTIVE[%x]\n" , (int)GSRDATA.csr_speech_recognition_is_active  )) ;

    return GSRDATA.csr_speech_recognition_is_active ;
}

/****************************************************************************
NAME
    speechRecSetIsActive

DESCRIPTION
    Set if SR is active

PARAMS
    Bool

RETURNS
    void
*/
void speechRecSetIsActive(bool value)
{
    GSRDATA.csr_speech_recognition_is_active = value;
}

/****************************************************************************
NAME
    speechRecognitionTuningIsActive

DESCRIPTION
    This function is called to determine if speech rec tuning is currently active

RETURNS
    True if Speech Rec tuning is active
*/
bool speechRecognitionTuningIsActive(void)
{
    SR_DEBUG(("Speech Rec TUNING ACTIVE[%x]\n" , (int)GSRDATA.csr_speech_recognition_tuning_active));

    return GSRDATA.csr_speech_recognition_tuning_active;
}

/****************************************************************************
NAME
    speechRecognitionSetTuningActive

DESCRIPTION
    This function sets the speech rec tuning

PARAMS
    Bool

RETURNS
    void
*/
void speechRecognitionSetTuningActive(bool value)
{
    GSRDATA.csr_speech_recognition_tuning_active = value;
}

/****************************************************************************
DESCRIPTION
  	This function is called to determine if speech rec is enabled
RETURNS
    True if Speech Rec is enabled
*/
bool speechRecognitionIsEnabled(void)
{

    /* Check if SR is enabled globally and return accordingly*/
    return (speechRecognitionFeatureEnabled() && speechRecognitionIsSSREnabled());
}

/****************************************************************************
DESCRIPTION
  	This function is used to check if the incoming call needs to be handled
  	(accept/reject/re-start speech recognition)
*/
static bool speech_rec_call_to_answer(void)
{
    switch(stateManagerGetState())
    {
        /* normal operation mode for incoming call answer */
        case deviceIncomingCallEstablish:
            {
                return TRUE;
            }
            break;

        /* in deviceThreeWayCallWaiting state, re-start Speech Recognition only if both AGs have incoming call */
        case deviceThreeWayCallWaiting:
            {
                hfp_call_state CallStateAG1 = hfp_call_state_idle;
                hfp_call_state CallStateAG2 = hfp_call_state_idle;

                HfpLinkGetCallState(hfp_primary_link, &CallStateAG1);
                HfpLinkGetCallState(hfp_secondary_link, &CallStateAG2);

                return ((CallStateAG1 == hfp_call_state_incoming) && (CallStateAG2 == hfp_call_state_incoming));
            }
            break;

        default:
            break;
    }

    return FALSE;
}


/****************************************************************************
DESCRIPTION
  	This function is the message handler which receives the messages from the
    SR library and converts them into suitable application messages
*/
static void speech_rec_handler(Task task, MessageId id, Message message)
{
    SR_DEBUG(("ASR message received \n")) ;

    switch (id)
    {
        case CSR_SR_WORD_RESP_YES:
        {
            SR_DEBUG(("\nSR: YES\n"));

            /* when in tuning mode, restart after a successful match */
            if(speechRecognitionTuningIsActive())
            {
                /* recognition suceeded, restart */
                audioDisconnectRoutedAudio();
                MessageSendLater((TaskData*)&speechRecTask , CSR_SR_APP_RESTART , 0, 100 ) ;
            }
            else if (speech_rec_call_to_answer())
            {
                MessageSend (&theSink.task , EventUsrAnswer , 0) ;
            }

            MessageSend (&theSink.task , EventSysSpeechRecognitionTuningYes , 0) ;
        }
        break;

        case CSR_SR_WORD_RESP_NO:
        {
            hfp_call_state CallStateAG1 = hfp_call_state_idle;
            hfp_call_state CallStateAG2 = hfp_call_state_idle;

            HfpLinkGetCallState(hfp_primary_link, &CallStateAG1);
            HfpLinkGetCallState(hfp_secondary_link, &CallStateAG2);

            SR_DEBUG(("\nSR: NO\n"));

            /* when in tuning mode, restart after a successful match */
            if(speechRecognitionTuningIsActive())
            {
                /* recognition suceeded, restart */
                audioDisconnectRoutedAudio();
                MessageSendLater((TaskData*)&speechRecTask , CSR_SR_APP_RESTART , 0, 100 ) ;
            }
            else if (speech_rec_call_to_answer())
            {
                MessageSend (&theSink.task , EventUsrReject , 0) ;
            }

            MessageSend (&theSink.task , EventSysSpeechRecognitionTuningNo , 0) ;
        }
        break;

        case CSR_SR_WORD_RESP_FAILED_YES:
        case CSR_SR_WORD_RESP_FAILED_NO:
        case CSR_SR_WORD_RESP_UNKNOWN:
        {
            SR_DEBUG(("\nSR: Unrecognized word, reason %x\n",id));

            MessageSend (&theSink.task , EventSysSpeechRecognitionFailed , 0) ;
            /* restart the ASR engine */
            AudioStartASR(AUDIO_MODE_CONNECTED);
        }
        break;

        case CSR_SR_APP_RESTART:
            SR_DEBUG(("SR: Restart\n"));
            /* recognition failed, try again */
            speechRecognitionReStart();
        break;

        case CSR_SR_APP_TIMEOUT:
        {
            hfp_call_state CallStateAG1 = hfp_call_state_idle;
            hfp_call_state CallStateAG2 = hfp_call_state_idle;

            HfpLinkGetCallState(hfp_primary_link, &CallStateAG1);
            HfpLinkGetCallState(hfp_secondary_link, &CallStateAG2);

            SR_DEBUG(("\nSR: TimeOut - Restart\n"));

            /* disable the timeout when in tuning mode */
            if ((!speechRecognitionTuningIsActive()) && (speech_rec_call_to_answer()))
            {
                MessageCancelAll (&theSink.task , EventSysSpeechRecognitionStart) ;
                MessageSend (&theSink.task , EventSysSpeechRecognitionStart , 0) ;
            }
        }
        break ;

        default:
            SR_DEBUG(("SR: Unhandled message 0x%x\n", id));
        /*    panic();*/
        break;
    }

#ifdef ENABLE_GAIA
    if (!(speech_rec_call_to_answer()))
    {
        gaiaReportSpeechRecResult(id);
    }
#endif
}


/****************************************************************************
NAME
    speechRecognitionIsSSREnabled

DESCRIPTION
    Get if Simple Speech Recognition is enabled or not

PARAMS
    void

RETURNS
    Bool TRUE for enabled otherwise FALSE
*/
bool speechRecognitionIsSSREnabled(void)
{
    return GSRDATA.ssr_enabled;
}

/****************************************************************************
NAME
    speechRecognitionSetSSREnabled

DESCRIPTION
    Set Simple Speech Recognition enabled or not

PARAMS
    Bool

RETURNS
    void
*/
void speechRecognitionSetSSREnabled(bool value)
{
    GSRDATA.ssr_enabled = value;
}

/**********************************************************************
    Interface for setting session data for SSR enabled
*/
void speechRecognitionSetSessionSSREnabled(void)
{
    speech_rec_writeable_config_def_t *writeable_data;
    SR_DEBUG(("SR:speechRecognitionSetSessionSSREnabled()\n"));

    if (configManagerGetWriteableConfig(SPEECH_REC_WRITEABLE_CONFIG_BLK_ID, (void **)&writeable_data, 0))
    {
        writeable_data->ssr_enabled = GSRDATA.ssr_enabled;
        configManagerUpdateWriteableConfig(SPEECH_REC_WRITEABLE_CONFIG_BLK_ID);
    }
}

/**********************************************************************
    Interface for getting session data for SSR enabled
*/
bool speechRecognitionGetSessionSSREnabled(void)
{
    bool ssr_enabled = FALSE;
    speech_rec_writeable_config_def_t *writeable_data;
    SR_DEBUG(("SR:speechRecognitionGetSessionSSREnabled()\n"));

    if(configManagerGetReadOnlyConfig(SPEECH_REC_WRITEABLE_CONFIG_BLK_ID, (const void **)&writeable_data))
    {
        ssr_enabled = writeable_data->ssr_enabled;
        configManagerReleaseConfig(SPEECH_REC_WRITEABLE_CONFIG_BLK_ID);
    }
    SR_DEBUG(("SR: ssr_enabled = %d\n", ssr_enabled));
    return (ssr_enabled)?TRUE:FALSE;
}

#endif /* ENABLE_SPEECH_RECOGNITION */
