/*
Copyright (c) 2004 - 2016 Qualcomm Technologies International, Ltd.
*/
/**
\file
\ingroup sink_app
\brief
    module responsible for audio (voice) prompts

*/

#include "sink_main_task.h"
#include "sink_debug.h"
#include "sink_audio_prompts.h"
#include "sink_events.h"
#include "sink_tones.h"
#include "sink_statemanager.h"
#include "sink_pio.h"
#include "sink_development.h"
#include "sink_hfp_data.h"
#include "vm.h"
#include "sink_config.h"
#include "file.h"
#include "sink_audio_routing.h"
#include "sink_configmanager.h"

#include <stddef.h>
#include <csrtypes.h>
#include <audio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <partition.h>
#include <audio_plugin_voice_prompts_variants.h>
#include <config_store.h>
#include <audio_config.h>

#include "sink_audio_prompts_config_def.h"

#ifdef DEBUG_AUDIO_PROMPTS
    #define PROMPTS_DEBUG(x) DEBUG(x)
#else
    #define PROMPTS_DEBUG(x) 
#endif

#define xstr(s) str(s)
#define str(s) #s
#define NUMERIC_DIGITS_DISPLAY 6

/* Global data for audio prompts */
typedef struct _sink_aduioprompts_global_data_t
{
    unsigned audio_prompts_enabled:1;
    unsigned audio_prompt_language:4;
    unsigned _spare_:11;
}sink_aduioprompts_global_data_t;

/* Global instance for audio prompts */
static sink_aduioprompts_global_data_t gAudioPrompts;
#define GAUDIOPROMPTS gAudioPrompts


/****************************************************************************
NAME    
    void SinkAudioPromptsGetSessionData(void)
    
DESCRIPTION
    Get the Audio Prompts session data
*/
 void SinkAudioPromptsGetSessionData(void)
 {
    sink_audioprompts_writeable_config_def_t *rw_config_data;

    if (configManagerGetReadOnlyConfig(SINK_AUDIOPROMPTS_WRITEABLE_CONFIG_BLK_ID, (const void**)&rw_config_data))
    {
        GAUDIOPROMPTS.audio_prompt_language = rw_config_data->audio_prompt_language;
        GAUDIOPROMPTS.audio_prompts_enabled = rw_config_data->audio_prompts_enabled;

        configManagerReleaseConfig(SINK_AUDIOPROMPTS_WRITEABLE_CONFIG_BLK_ID);
    }
 }
 
/****************************************************************************
NAME    
    void SinkAudioPromptsSetSessionData(void)
    
DESCRIPTION
    Set the Audio Prompts session data
*/
void SinkAudioPromptsSetSessionData(void)
{
    sink_audioprompts_writeable_config_def_t *rw_config_data;

    if (configManagerGetWriteableConfig(SINK_AUDIOPROMPTS_WRITEABLE_CONFIG_BLK_ID, (void**)&rw_config_data, 0))
    {
        rw_config_data->audio_prompt_language = GAUDIOPROMPTS.audio_prompt_language;
        rw_config_data->audio_prompts_enabled = GAUDIOPROMPTS.audio_prompts_enabled;

        configManagerUpdateWriteableConfig(SINK_AUDIOPROMPTS_WRITEABLE_CONFIG_BLK_ID);
    }
}

/****************************************************************************
NAME    
    SinkAudioPromptsIsEnabled
    
DESCRIPTION
    Get whether Audio prompt is enabled
*/
bool SinkAudioPromptsIsEnabled(void)
{
    return GAUDIOPROMPTS.audio_prompts_enabled; 
}

/****************************************************************************
NAME    
    SinkAudioPromptsSetEnabled
    
DESCRIPTION
    Set the Audio prompt enabled
*/
void SinkAudioPromptsSetEnabled(bool enabled)
{
    GAUDIOPROMPTS.audio_prompts_enabled = enabled;
}

/****************************************************************************
NAME    
    SinkAudioPromptsGetLanguage
    
DESCRIPTION
    Get the audio prompt language
*/
uint8 SinkAudioPromptsGetLanguage(void)
{
   return GAUDIOPROMPTS.audio_prompt_language;
}

/****************************************************************************
NAME    
    SinkAudioPromptsSetLanguage
    
DESCRIPTION
    Set the audio prompt language
*/
void SinkAudioPromptsSetLanguage(uint8 language)
{
    GAUDIOPROMPTS.audio_prompt_language = language;
}

/****************************************************************************
NAME    
    SinkAudioPromptsGetNumAudioPromptFiles
    
DESCRIPTION
    Get the total number of audio prompt files.

    This should always be the number of event->prompt mappings multiplied
    by the number of audio prompt sets/languages.

PARAMS
    void
    
RETURNS
    uint16
*/
uint16 SinkAudioPromptsGetNumAudioPromptFiles(void)
{
    uint16 ret_val = 0;
    sink_audioprompts_writeable_config_def_t *rw_config_data;

    /* Read entire config block */
    if (configManagerGetReadOnlyConfig(SINK_AUDIOPROMPTS_WRITEABLE_CONFIG_BLK_ID, (const void **)&rw_config_data))
    {
        ret_val = rw_config_data->num_audio_prompts;
        /* Release config Block and dont write back */
        configManagerReleaseConfig(SINK_AUDIOPROMPTS_WRITEABLE_CONFIG_BLK_ID);
    }
    return ret_val;
}

/****************************************************************************
NAME    
    SinkAudioPromptsSetNumAudioPromptFiles
    
DESCRIPTION
    Sets the total number of audio prompt files and saves it in the
    relevant config storage.

    This should always be the number of event->prompt mappings multiplied
    by the number of audio prompt sets/languages.

PARAMS
    num_files Total number of audio prompt files.
    
RETURNS
    void
*/
void SinkAudioPromptsSetNumAudioPromptFiles(uint16 num_files)
{
    sink_audioprompts_writeable_config_def_t *rw_config_data;
    
    /* Read entire config block */
    if (configManagerGetWriteableConfig(SINK_AUDIOPROMPTS_WRITEABLE_CONFIG_BLK_ID, (void**)&rw_config_data, 0))
    {
        rw_config_data->num_audio_prompts = num_files;
        /* Write the data back and release config block*/
        configManagerUpdateWriteableConfig(SINK_AUDIOPROMPTS_WRITEABLE_CONFIG_BLK_ID);
    }
}

/****************************************************************************
NAME    
    SinkAudioPromptsGetNumAudioPromptLanguages
    
DESCRIPTION
    Get the number of audio prompt languages
*/
uint8 SinkAudioPromptsGetNumAudioPromptLanguages(void)
{
    sink_audioprompts_readonly_config_def_t *ro_data = NULL;
    uint8 num_languages = 0;

    /* Read the existing Config data */
    if (configManagerGetReadOnlyConfig(SINK_AUDIOPROMPTS_READONLY_CONFIG_BLK_ID, (const void **)&ro_data))
    {
        num_languages = ro_data->num_audio_prompt_languages;
        configManagerReleaseConfig(SINK_AUDIOPROMPTS_READONLY_CONFIG_BLK_ID);
    }
    return num_languages;
}

/****************************************************************************
NAME    
    SinkAudioPromptsGetDisableAudioPromptTerminate
    
DESCRIPTION
    Get the Disable AudioPrompt Terminate
*/
bool SinkAudioPromptsGetDisableAudioPromptTerminate(void)
{
    sink_audioprompts_readonly_config_def_t *ro_data = NULL;
    bool result = FALSE;

    /* Read the existing Config data */   
    if (configManagerGetReadOnlyConfig(SINK_AUDIOPROMPTS_READONLY_CONFIG_BLK_ID, (const void **)&ro_data))
    {
        result = ro_data->DisableAudioPromptTerminate;
        configManagerReleaseConfig(SINK_AUDIOPROMPTS_READONLY_CONFIG_BLK_ID);
    }
    return result;
}

/****************************************************************************
NAME    
    SinkAudioPromptsGetVoicePromptNumbers
    
DESCRIPTION
    Get the VoicePrompt Numbers
*/
bool SinkAudioPromptsGetVoicePromptNumbers(void)
{
    sink_audioprompts_readonly_config_def_t *ro_data = NULL;
    bool result = FALSE;

    /* Read the existing Config data */
    if (configManagerGetReadOnlyConfig(SINK_AUDIOPROMPTS_READONLY_CONFIG_BLK_ID, (const void **)&ro_data))
    {
        result = ro_data->VoicePromptNumbers;
        configManagerReleaseConfig(SINK_AUDIOPROMPTS_READONLY_CONFIG_BLK_ID);
    }
    return result;
}

/****************************************************************************
NAME    
    SinkAudioPromptsGetVoicePromptPairing
    
DESCRIPTION
    Get the VoicePrompt Pairing
*/
bool SinkAudioPromptsGetVoicePromptPairing(void)
{
    sink_audioprompts_readonly_config_def_t *ro_data = NULL;
    bool result = FALSE;

    /* Read the existing Config data */
    if (configManagerGetReadOnlyConfig(SINK_AUDIOPROMPTS_READONLY_CONFIG_BLK_ID, (const void **)&ro_data))
    {
        result = ro_data->VoicePromptPairing;
        configManagerReleaseConfig(SINK_AUDIOPROMPTS_READONLY_CONFIG_BLK_ID);
    }
    return result;
}

static FILE_INDEX VoicePromptsGetFileIndex(unsigned index)
{
    char file_name[24];

    /* Get the prompt file name */
    sprintf(file_name, "prompts/%d.prm", index);
    return FileFind(FILE_ROOT, file_name, (uint16)strlen(file_name));
}

static FILE_INDEX VoicePromptsHeaderGetFileIndex(unsigned index)
{
    char file_name[24];

    /* Get the prompt header file name */
    sprintf(file_name, "headers/%d.idx", index);
    return FileFind(FILE_ROOT, file_name, (uint16)strlen(file_name));
}

static unsigned VoicePromptsFileGetIndex(unsigned id, unsigned language)
{
    unsigned index = id;
    uint16 number_of_prompts = SinkAudioPromptsGetNumAudioPromptFiles();
    uint8 number_of_languages = SinkAudioPromptsGetNumAudioPromptLanguages();

    if(number_of_languages)
	{
        index += language * (number_of_prompts/ number_of_languages);

        /* Sanity checking */
        if(index >= number_of_prompts)
        {
            Panic();
        }

        PROMPTS_DEBUG(("VP: Play prompt %d of %d\n", index+1, number_of_prompts));
    }
    else
    {
        PROMPTS_DEBUG(("VP: Number of Audio prompt languages is 0"));
        Panic();
	}
	
    return index;
}

/****************************************************************************
NAME    
    SinkAudioPromptsGetPromptConfig
    
DESCRIPTION
    Get the audio prompt config for an event.

PARAMS
    event  Event to get the audio prompt config for.
    prompt Pointer to a audio_prompt_config_t that is set.
    
RETURNS
    bool: TRUE if success else FALSE
*/
bool SinkAudioPromptsGetPromptConfig(uint16 event, audio_prompt_config_t *prompt)
{
    sink_audioprompts_config_def_t *audio_prompts_config = NULL;
    uint16 idx;
    uint16 size;
    uint16 num_prompts = 0;
    bool ret = FALSE;

    size = configManagerGetReadOnlyConfig(SINK_AUDIOPROMPTS_CONFIG_BLK_ID, (const void **)&audio_prompts_config);
    if (!size)
        return ret;

    num_prompts = (uint16)((size * sizeof(uint16)) / sizeof(audio_prompts_config->audioPromptEvents[0]));
    for (idx = 0; idx < num_prompts; idx++)
    {
        if (audio_prompts_config->audioPromptEvents[idx].event == event)
            break;
    }
    
    if (idx < num_prompts)
    {
        prompt->event = audio_prompts_config->audioPromptEvents[idx].event;
        prompt->prompt_id = audio_prompts_config->audioPromptEvents[idx].prompt_id;
        prompt->cancel_queue_play_immediate = audio_prompts_config->audioPromptEvents[idx].cancel_queue_play_immediate;
        prompt->sco_block = audio_prompts_config->audioPromptEvents[idx].sco_block;
        prompt->state_mask = audio_prompts_config->audioPromptEvents[idx].state_mask;

        ret = TRUE;
    }

    configManagerReleaseConfig(SINK_AUDIOPROMPTS_CONFIG_BLK_ID);

    return ret;
}

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
bool SinkAudioPromptsSetPromptConfig(const audio_prompt_config_t *prompt_config)
{
    sink_audioprompts_config_def_t *audio_prompts_config = NULL;
    uint16 idx;
    uint16 size;
    uint16 num_prompts = 0;

    /* Open the current config as writeable */
    size = configManagerGetWriteableConfig(SINK_AUDIOPROMPTS_CONFIG_BLK_ID, (void**)&audio_prompts_config, 0);
    if (!size)
        return FALSE;

    /* Try to find the event in the existing config. */
    num_prompts = (uint16)(size / sizeof(audio_prompts_config->audioPromptEvents[0]));
    for (idx = 0; idx < num_prompts; idx++)
    {
        if (audio_prompts_config->audioPromptEvents[idx].event == prompt_config->event)
            break;
    }

    /* If it is not found, re-open the config one element larger */
    if (idx == num_prompts)
    {
        configManagerReleaseConfig(SINK_AUDIOPROMPTS_CONFIG_BLK_ID);

        num_prompts += 1;
        size = (num_prompts * sizeof(audio_prompts_config_type_t));
        if (!configManagerGetWriteableConfig(SINK_AUDIOPROMPTS_CONFIG_BLK_ID, (void**)&audio_prompts_config, size))
            return FALSE;
    }

    /* Set the new values for the event. */
    audio_prompts_config->audioPromptEvents[idx].event = prompt_config->event;
    audio_prompts_config->audioPromptEvents[idx].prompt_id = prompt_config->prompt_id;
    audio_prompts_config->audioPromptEvents[idx].cancel_queue_play_immediate = prompt_config->cancel_queue_play_immediate;
    audio_prompts_config->audioPromptEvents[idx].sco_block = prompt_config->sco_block;
    audio_prompts_config->audioPromptEvents[idx].state_mask = prompt_config->state_mask;

    /* Store the config. */
    configManagerUpdateWriteableConfig(SINK_AUDIOPROMPTS_CONFIG_BLK_ID);
    SinkAudioPromptsSetNumAudioPromptFiles(num_prompts * SinkAudioPromptsGetNumAudioPromptLanguages());

    return TRUE;
}

/****************************************************************************
NAME    
    SinkAudioPromptsInit
    
DESCRIPTION
    Initialization of Audio prompt module
*/
void SinkAudioPromptsInit(bool full_init)
{
    uint16 num_audio_prompt_files = SinkAudioPromptsGetNumAudioPromptFiles();

    /* check the number of events configured and the supported audio prompt languages */
    if(num_audio_prompt_files != 0 && full_init)
    {
        
#ifdef ENABLE_SQIFVP
        /* Initilaise as no partitions currently mounted */
        sinkDataSetSquifPartitionMount(0);
#endif /* ENABLE_SQIFVP */                              

    }
}

/****************************************************************************
NAME
    AudioPromptPlay
    
DESCRIPTION
    Conditionally call text-to-speech plugin
*/
void AudioPromptPlay(Task plugin, uint16 id, bool can_queue, bool override)
{
    if (SinkAudioPromptsIsEnabled())
    {
        int16 ap_volume = AudioConfigGetToneVolumeToUse();
        uint8 ap_lang = SinkAudioPromptsGetLanguage();
        uint16 voice_prompts_index;
        FILE_INDEX prompt_index;
        FILE_INDEX prompt_header_index;

        /* turn on audio amp */
        enableAudioActivePio();
        
#ifdef ENABLE_SQIFVP        
        /* If using multiple partitions for the voice prompt langages, mount the relevant partiton if required */

        /* find if the partition for this language is mounted */
        if (!((1<<ap_lang) & sinkDataGetSquifMountedPartitions()))
        {
            /* Mount the partition for this prompt */
            uint8 partition_mount;
            PROMPTS_DEBUG(("AudioPromptPlay mount SQIF partition %u (0x%x)\n", ap_lang,  sinkDataGetSquifMountedPartitions()));
            if(!PartitionMountFilesystem(PARTITION_SERIAL_FLASH, ap_lang, PARTITION_LOWER_PRIORITY))
                Panic();

            partition_mount = sinkDataGetSquifMountedPartitions();
            partition_mount |= (1<<GAUDIOPROMPTS.audio_prompt_language);
            sinkDataSetSquifPartitionMount(partition_mount);

            PROMPTS_DEBUG(("AudioPromptPlay SQIF partitions now mounted 0x%x\n", sinkDataGetSquifMountedPartitions() ));
        }
#endif
        
        voice_prompts_index = VoicePromptsFileGetIndex(id, ap_lang);
        prompt_header_index = VoicePromptsHeaderGetFileIndex(voice_prompts_index);
        prompt_index = VoicePromptsGetFileIndex(voice_prompts_index);

        PROMPTS_DEBUG(("AudioPromptPlay %d  [lang:%u][q:%u][o.r.:%u]\n", id, GAUDIOPROMPTS.audio_prompt_language,can_queue,override));
        AudioPlayAudioPrompt(plugin, prompt_index, prompt_header_index, can_queue,
                               ap_volume,  sinkAudioGetPluginFeatures(), override, &theSink.task);
        /* turn amp off if audio is inactive */
        disableAudioActivePioWhenAudioNotBusy();
    }
}

/****************************************************************************
NAME 
    AudioPromptPlayNumString
DESCRIPTION
    Play a numeric string using the Audio Prompts plugin
RETURNS    
*/

void AudioPromptPlayNumString(uint16 size_num_string, const uint8* num_string)
{
    if(size_num_string)
    {
        const uint8 * pData = num_string;
        uint8 i;        

        /* check each character in the string is a numeric character */
        for(i=0;i<size_num_string;i++)
        {                    
            /* Check for non-numeric characters */
            if(*pData >= 0x30 || *pData <= 0x39)
            {
                PROMPTS_DEBUG(("AP: PlayDigit[%x]\n", pData[i]- 0x30 )) ;
                
                switch (pData[i] - 0x30)
                {
                    /* Send event corresponding to the digit, assign audio prompts to these
                       events in the normal manner */
                    case 0:
                        MessageSend(&theSink.task, EventSysToneDigit0, 0);
                        break;
                    case 1:
                        MessageSend(&theSink.task, EventSysToneDigit1, 0);
                        break;                    
                    case 2:
                        MessageSend(&theSink.task, EventSysToneDigit2, 0);
                        break;
                    case 3:
                        MessageSend(&theSink.task, EventSysToneDigit3, 0);
                        break;
                    case 4:
                        MessageSend(&theSink.task, EventSysToneDigit4, 0);
                        break;
                    case 5:
                        MessageSend(&theSink.task, EventSysToneDigit5, 0);
                        break;
                    case 6:
                        MessageSend(&theSink.task, EventSysToneDigit6, 0);
                        break;
                    case 7:
                        MessageSend(&theSink.task, EventSysToneDigit7, 0);
                        break;
                    case 8:
                        MessageSend(&theSink.task, EventSysToneDigit8, 0);
                        break;
                    case 9:
                        MessageSend(&theSink.task, EventSysToneDigit9, 0);
                        break;   
                    default:
                        ;
                }
            }                    
        }
    }
}

/****************************************************************************
NAME 
    AudioPromptPlayNumber
DESCRIPTION
    Play a uint32 using the Audio prompt plugin
RETURNS    
*/

void AudioPromptPlayNumber(uint32 number)
{
    /* A 32 bit number can be accommodated in 10 digits */
    char num_string[11];
    /* Convert number to at least a 6 digit string left padded with zeros */
    int16 len = sprintf(num_string, "%0" xstr(NUMERIC_DIGITS_DISPLAY) "lu", number);
    
    /* Play the final 6 digits of the string */
    if(len >= NUMERIC_DIGITS_DISPLAY)
    {
        AudioPromptPlayNumString(NUMERIC_DIGITS_DISPLAY, (uint8*)(num_string + len - NUMERIC_DIGITS_DISPLAY));
    }
    else
    {
        FATAL_ERROR(("Conversion of  number to string for Audio Prompts Play failed!\n"));
    }
}

/****************************************************************************
NAME 
DESCRIPTION
RETURNS    
*/

bool AudioPromptPlayCallerNumber( const uint16 size_number, const uint8* number ) 
{
    if(SinkAudioPromptsGetVoicePromptNumbers())
    {
        if(sinkHfpDataGetRepeatCallerIDFlag() && size_number > 0) 
        { 
            sinkHfpDataSetRepeatCallerIDFlag(FALSE);
            AudioPromptPlayNumString(size_number, number);
            return TRUE;
        }
    }

    return FALSE;
}

/****************************************************************************
NAME 
DESCRIPTION
RETURNS    
*/
bool AudioPromptPlayCallerName( const uint16 size_name, const uint8* name ) 
{
#ifdef TEXT_TO_SPEECH_NAMES
    TaskData * task = (TaskData *) &INSERT_TEXT_TO_SPEECH_PLUGIN_HERE;
    
    if(size_name > 0) 
    {
        /* Hook point for Text to speech synthesizer */
        Panic();
    }
#else
    UNUSED(size_name);
#endif
    UNUSED(name);

    return FALSE;
}

/****************************************************************************
NAME    
    AudioPromptCancelNumString
    
DESCRIPTION
      function to cancel any pending number string messages.
    
RETURNS
    
*/
void AudioPromptCancelNumString( void )
{
    /* Do nothing if Audio Prompt Terminate Disabled */
    if(!SinkAudioPromptsGetDisableAudioPromptTerminate())
    {
        PROMPTS_DEBUG(("PROMPTS: Cancel pending digits \n")) ;
        /* Cancel any digits that maybe pending */
        MessageCancelAll(&theSink.task,EventSysToneDigit0);
        MessageCancelAll(&theSink.task,EventSysToneDigit1);
        MessageCancelAll(&theSink.task,EventSysToneDigit2);
        MessageCancelAll(&theSink.task,EventSysToneDigit3);
        MessageCancelAll(&theSink.task,EventSysToneDigit4);
        MessageCancelAll(&theSink.task,EventSysToneDigit5);
        MessageCancelAll(&theSink.task,EventSysToneDigit6); 
        MessageCancelAll(&theSink.task,EventSysToneDigit7);        
        MessageCancelAll(&theSink.task,EventSysToneDigit8);        
        MessageCancelAll(&theSink.task,EventSysToneDigit9);        
    }
}  
      
/****************************************************************************
NAME    
    AudioPromptSelectLanguage
    
DESCRIPTION
      function to select a audio prompt language.
    
RETURNS
    
*/
void AudioPromptSelectLanguage( void )
{
    uint8 current_lang = SinkAudioPromptsGetLanguage();
    uint8 new_lang = current_lang;

#ifdef ENABLE_SQIFVP  
    /* if using Multiple partitions in SQIF for voice prompts make sure we choose one with prompts in it */
    do
    {
        new_lang++;
        if(new_lang >= SinkAudioPromptsGetNumAudioPromptLanguages())
            new_lang = 0;
        
        PROMPTS_DEBUG(("AP: Select language [%u][%u][%u][0x%x]\n", new_lang, current_lang, SinkAudioPromptsGetNumAudioPromptLanguages(), sinkDataGetSquifFreePartitions()));
    }
    while (((1<<new_lang) & sinkDataGetSquifFreePartitions()) && (new_lang != current_lang));

#else
    new_lang++;
    if(new_lang >= SinkAudioPromptsGetNumAudioPromptLanguages())
        new_lang = 0;

#endif

    SinkAudioPromptsSetLanguage(new_lang);

    PROMPTS_DEBUG(("AP: Select language [%u]\n", SinkAudioPromptsGetLanguage()));

    MessageSend(&theSink.task, EventSysConfirmAudioPromptLanguage, NULL);
}

/****************************************************************************
NAME    
    IsAudioPromptConfiguredForEvent
    
DESCRIPTION
    Checks if audio prompt is configured for the given sink event.
 
PARAMETERS
    sink_event    - incoming sink event

RETURNS
    TRUE if audio prompt is configured for the sink event
    FALSE if audio prompt is not configured for the sink event
*/
bool IsAudioPromptConfiguredForEvent(sinkEvents_t sink_event)
{
    audio_prompts_config_type_t *promptEvent = NULL;
    sink_audioprompts_config_def_t *audio_prompts_config;
    uint16 size;
    uint16 num_prompts = 0;
    bool configured_for_event = FALSE;

    /* Read the existing configuration */
    size = configManagerGetReadOnlyConfig(SINK_AUDIOPROMPTS_CONFIG_BLK_ID, (const void **)&audio_prompts_config);
    if (size)
    {
        num_prompts = (uint16)((size * sizeof(uint16)) / sizeof(audio_prompts_config->audioPromptEvents[0]));

        promptEvent = audio_prompts_config->audioPromptEvents;
        while (promptEvent < &audio_prompts_config->audioPromptEvents[num_prompts])
        {
            /* Check if this event has a configured audio prompt*/
            if (promptEvent->event == sink_event)
            {
                PROMPTS_DEBUG(("AP: Event conf available\n"));
                configured_for_event = TRUE;
                break;
            }
            promptEvent++;
        }
    }
    /* Ensure that we release config */
    configManagerReleaseConfig(SINK_AUDIOPROMPTS_CONFIG_BLK_ID);

    return configured_for_event;
}

/****************************************************************************
NAME    
    AudioPromptPlayEvent
    
DESCRIPTION
    Plyas audio prompt attached to the incoming sink event.
 
PARAMETERS
    sink_event           - incoming sink event
    event_can_be_queued  - Flag which indicate if this event
                           is to be played imidiately or not

RETURNS
    void
*/
void AudioPromptPlayEvent(sinkEvents_t sink_event, bool event_can_be_queued)
{
    TaskData * task = NULL;
    uint16 state_mask = 1 << stateManagerGetState();
    audio_prompts_config_type_t *promptEvent = NULL;
    sink_audioprompts_config_def_t *audio_prompts_config;
    uint16 size;
    uint16 num_prompts = 0;

    /* Read the existing configuration */
    size = configManagerGetReadOnlyConfig(SINK_AUDIOPROMPTS_CONFIG_BLK_ID, (const void **)&audio_prompts_config);
    if (!size)
        return;

    task = (TaskData *) &csr_voice_prompts_plugin;
    num_prompts = (uint16)((size * sizeof(uint16)) / sizeof(audio_prompts_config->audioPromptEvents[0]));
    promptEvent = audio_prompts_config->audioPromptEvents;

    while (promptEvent < &audio_prompts_config->audioPromptEvents[num_prompts])
    {
        /* Play Audio Prompt if  we're not in a blocked state or in streaming A2DP state */
        if((promptEvent->event == sink_event) && 
           ((promptEvent->state_mask & state_mask) && 
           (!(promptEvent->sco_block && sinkAudioIsAudioRouted())||(state_mask & (1<<deviceA2DPStreaming)))))
        {
            PROMPTS_DEBUG(("AP: EvPl[%x][%x][%x][%x]\n", sink_event, promptEvent->event,\
                                      promptEvent->prompt_id, promptEvent->cancel_queue_play_immediate )) ;

            AudioPromptPlay(task, (uint16) promptEvent->prompt_id, event_can_be_queued, promptEvent->cancel_queue_play_immediate);
        }
        promptEvent++;
    }
    
    /* Ensure that we release config */
    configManagerReleaseConfig(SINK_AUDIOPROMPTS_CONFIG_BLK_ID);
}
