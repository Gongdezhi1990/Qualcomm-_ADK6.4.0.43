/*****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    ama_speech_.h

DESCRIPTION
    Functions for high level control of the speech state and interation
    with the phone application
*/

#ifndef __AMA_SPEECH_H_
#define __AMA_SPEECH_H_

#include "ama.h"
#include "ama_private.h"
#include "speech.pb-c.h"

typedef enum{
    ama_audio_profile_close_talk,
    ama_audio_profile_near_field,
    ama_audio_profile_far_field
}ama_audio_profile_t;

typedef enum{
    ama_audio_format_pcm_l16_16khz_mono,
    ama_audio_format_opus_16khz_32kbps_cbr_0_20ms,
    ama_audio_format_opus_16khz_16kbps_cbr_0_20ms,
    ama_audio_format_msbc
}ama_audio_format_t;

typedef enum{
    ama_audio_source_stream,
    ama_audio_source_bluetooth_sco
} ama_audio_source_t;

typedef enum{
    ama_speech_msg_initialise,
    ama_speech_msg_start,
    ama_speech_msg_start_ack,
    ama_speech_msg_stop,
    ama_speech_msg_ready_to_sync_settings,
    ama_speech_msg_response_timeout,
    ama_speech_msg_provide_speech,
    ama_speech_msg_suspend,
    ama_speech_msg_resume,
    ama_speech_msg_end,
    ama_speech_msg_last
}ama_speech_msg_t;


typedef struct{
    ama_speech_initiator_t initiator;
    uint32 pre_roll;
    /* start timestamp for trigger phrase of buffered data */
    uint32 start_timestamp;
    /* end timestamp for trigger phrase of buffered data */
    uint32 end_timestamp;
}ama_speech_msg_start_t;

typedef struct{
    bool send_end;
}ama_speech_msg_stop_t;

typedef struct{
    uint32 dialog_id;
}ama_speech_msg_provide_speech_t;


/***************************************************************************
DESCRIPTION
    Configures the audio source parameter that will be passed to the phone
    when speech is started.
 
PARAMS
    source The audio source for the voice data
*/
void amaSpeechSetAudioSource(ama_audio_source_t source);

/***************************************************************************
DESCRIPTION
    Configures the audio format parameter that will be passed to the phone
    when speech is started.
 
PARAMS
    format The audio format of the voice data
*/
void amaSpeechSetAudioFormat(ama_audio_format_t format);

/***************************************************************************
DESCRIPTION
    Configures the audio profile parameter that will be passed to the phone
    when speech is started.
 
PARAMS
    format The audio profile of the voice data
*/
void amaSpeechSetAudioProfile(ama_audio_profile_t profile);

/***************************************************************************
DESCRIPTION
    Gets the audio source parameter that will be passed to the phone
    when speech is started.
*/
AudioSource amaSpeechGetAudioSource(void);

/***************************************************************************
DESCRIPTION
    Gets the audio format parameter that will be passed to the phone
    when speech is started.
*/
AudioFormat amaSpeechGetAudioFormat(void);

/***************************************************************************
DESCRIPTION
    Gets the audio profile parameter that will be passed to the phone
    when speech is started.
*/
AudioProfile amaSpeechGetAudioProfile(void);

/***************************************************************************
DESCRIPTION
    Passes a speech message to the speech module for processing
 
PARAMS
    msg_id  The message to act upon
    msg Additional message date where required
*/
bool amaSpeechMessage(ama_speech_msg_t msg_id, void *msg);


/***************************************************************************
DESCRIPTION
    Check if we are in a state where it is valid to send speech.
 
RETURNS
    True if we can send speech
*/
bool amaValidStateToSendSpeech(void);

/***************************************************************************
DESCRIPTION
    Translates speech state notification from AMA into a message
    amaSpeechMessage understands
PARAMS
    state The state passed to the accessory 
*/
void amaSpeechNotifyStateMsg(SpeechState state);

/***************************************************************************
DESCRIPTION
    Returns the current DialogId
*/
uint32 amaSpeechGetCurrentDialogId(void);

#endif /* __AMA_SPEECH_H_ */
