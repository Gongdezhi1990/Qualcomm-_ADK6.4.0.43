/*
Copyright (c) 2004 - 2016 Qualcomm Technologies International, Ltd.
*/
/**
\file
\ingroup sink_app
\brief
    header file which defines the interface between the audio (voice) prompts and the application

*/

#ifndef SINK_AUDIO_PROMPTS_H
#define SINK_AUDIO_PROMPTS_H

#include <csrtypes.h>
#include <vmtypes.h>

#include "sink_events.h"

/****************************************************************************
NAME	
	SinkAudioPromptsIsEnabled
    
DESCRIPTION
    Get whether Audio prompt is enabled
    
PARAMS
    void
    
RETURNS
	bool
*/
bool SinkAudioPromptsIsEnabled(void);

/****************************************************************************
NAME	
	SinkAudioPromptsSetEnabled
    
DESCRIPTION
    Set the Audio prompt enabled
    
PARAMS
    enabled  State to set for audio prompts
    
RETURNS
	void
*/
void SinkAudioPromptsSetEnabled(bool enabled);


/****************************************************************************
NAME	
	bool SinkAudioPromptsGetSessionData(void)

    
DESCRIPTION
    Get the Audio prompts session data

RETURNS
    void
*/

 void SinkAudioPromptsGetSessionData(void);
/****************************************************************************
NAME	
	bool SinkAudioPromptsSetSessionData(void)

    
DESCRIPTION
    Set the Audio session data
    
RETURNS
    void
*/
void SinkAudioPromptsSetSessionData(void);

/****************************************************************************
NAME	
	SinkAudioPromptsGetLanguage
    
DESCRIPTION
    Get the audio prompt language
    
RETURNS
	uint8
*/
uint8 SinkAudioPromptsGetLanguage(void);

/****************************************************************************
NAME	
	SinkAudioPromptsSetLanguage
    
DESCRIPTION
    Set the audio prompt language
    
PARAMS
    language  Language identifier
    
RETURNS
	void
*/
void SinkAudioPromptsSetLanguage(uint8 language);

/****************************************************************************
NAME	
	SinkAudioPromptInit
    
DESCRIPTION
    Initialization of Audio prompt module

PARAMS
    full_init  Flag specifying full initialization or not.
    
RETURNS
	void
*/
void SinkAudioPromptsInit(bool full_init);

/****************************************************************************
NAME	
	SinkAudioPromptsGetNumAudioPromptFiles
    
DESCRIPTION
    Get the total number of audio prompt files.

    This should always be the number of event->prompt mappings multiplied
    by the number of audio prompt sets/languages.
    
RETURNS
	uint16
*/
uint16 SinkAudioPromptsGetNumAudioPromptFiles(void);

/****************************************************************************
NAME	
	SinkAudioPromptsSetNumAudioPromptFiles
    
DESCRIPTION
    Sets the total number of audio prompt files and saves it in the
    relevant config storage.

    This should always be the number of event->prompt mappings multiplied
    by the number of audio prompt sets/languages.

PARAMS
    num_files  Total number of audio prompt files.
    
RETURNS
	void
*/
void SinkAudioPromptsSetNumAudioPromptFiles(uint16 num_files);


typedef struct
{
    uint16   event;
	BITFIELD unused:8;
	BITFIELD prompt_id:8;
    BITFIELD cancel_queue_play_immediate:1;
    BITFIELD sco_block:1;
    BITFIELD state_mask:14 ;
} audio_prompt_config_t;

/****************************************************************************
NAME	
    SinkAudioPromptsGetPromptConfig
    
DESCRIPTION
    Get the audio prompt config for an event.

PARAMS
    event   Event to get the audio prompt config for.
    prompt  Pointer to a audio_prompt_config_t
    
RETURNS
    bool: TRUE if success else FALSE
*/
bool SinkAudioPromptsGetPromptConfig(uint16 event, audio_prompt_config_t *prompt);

/****************************************************************************
NAME	
	SinkAudioPromptsSetPromptConfig
    
DESCRIPTION
    Add or update the audio prompt config for an event.

PARAMS
    prompt_config  Audio prompt data for an event.
    
RETURNS
    bool: TRUE if success else FALSE
*/
bool SinkAudioPromptsSetPromptConfig(const audio_prompt_config_t *prompt_config);

/****************************************************************************
NAME	
	SinkAudioPromptsGetNumAudioPromptLanguages
    
DESCRIPTION
    Get the number of audio prompt languages

PARAMS
    void
    
RETURNS
	uint8
*/
uint8 SinkAudioPromptsGetNumAudioPromptLanguages(void);

/****************************************************************************
NAME	
	SinkAudioPromptsGetDisableAudioPromptTerminate
    
DESCRIPTION
    Get the Disable AudioPrompt Terminate

PARAMS
    void
    
RETURNS
	bool
*/
bool SinkAudioPromptsGetDisableAudioPromptTerminate(void);

/****************************************************************************
NAME	
	SinkAudioPromptsGetVoicePromptNumbers
    
DESCRIPTION
    Get the VoicePrompt Numbers

PARAMS
    void
    
RETURNS
	bool
*/
bool SinkAudioPromptsGetVoicePromptNumbers(void);

/****************************************************************************
NAME	
	SinkAudioPromptsGetVoicePromptPairing
    
DESCRIPTION
    Get the VoicePrompt Pairing

PARAMS
    void
    
RETURNS
	bool
*/
bool SinkAudioPromptsGetVoicePromptPairing(void);

/****************************************************************************

*/
void AudioPromptConfigure( uint16 size_index );

/****************************************************************************

*/
void AudioPromptPlay(Task plugin, uint16 id, bool can_queue, bool override);

/****************************************************************************
NAME 
    AudioPromptPlayNumString
DESCRIPTION
    Play a numeric string using the Audio Prompt plugin
RETURNS    
*/
void AudioPromptPlayNumString(uint16 size_num_string, const uint8* num_string);

/****************************************************************************
NAME 
    AudioPromptPlayNumber
DESCRIPTION
    Play a uint32 using the audio prompt plugin
RETURNS    
*/
void AudioPromptPlayNumber(uint32 number);

/* **************************************************************************
   */


bool AudioPromptPlayCallerNumber( const uint16 size_number, const uint8* number );

/****************************************************************************
NAME    
    AudioPromptPlayCallerName
    
DESCRIPTION
  	function to play caller name

PARAMETERS
    size_name   Size of name
    name        Pointer to name
    
RETURNS
    
*/
bool AudioPromptPlayCallerName( const uint16 size_name, const uint8* name );
   
/****************************************************************************
NAME    
    AudioPromptCancelNumString
    
DESCRIPTION
  	function to cancel any pending number string messages.
    
RETURNS
    
*/
void AudioPromptCancelNumString( void );

/****************************************************************************
NAME    
    AudioPromptSelectLanguage
    
DESCRIPTION
  	Move to next language
    
RETURNS
    
*/
void AudioPromptSelectLanguage( void );

/****************************************************************************
NAME    
    IsAudioPromptConfiguredForEvent
    
DESCRIPTION
    Checks if audio prompt is configured for the given sink event.
 
PARAMETERS
    sink_event    - incoming sink event

RETURNS
    TRUE if audio prompt is configured for the sink event
    FALSE if audio proompt is not configured for the sink event
*/
bool IsAudioPromptConfiguredForEvent(sinkEvents_t sink_event);

/****************************************************************************
NAME    
    AudioPromptsPlayEvent
    
DESCRIPTION
    Plyas audio prompt attached to the incoming sink event.
 
PARAMETERS
    sink_event           - incoming sink event
    event_can_be_queued  - Flag which indicate if this event
    					   is to be played imidiately or not
 
RETURNS
    void
*/
void AudioPromptPlayEvent(sinkEvents_t sink_event, bool event_can_be_queued);
#endif

