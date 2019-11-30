/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    audio.c

DESCRIPTION
     The main audio manager file

*/
#include "audio.h"

#include <stdlib.h>
#include <string.h>
#include <panic.h>
#include <print.h>
#include <csrtypes.h>

#include <stream.h>
#include <audio_config.h>
#include <audio_plugin_if.h>
#include <audio_plugin_music_params.h>
#include <audio_plugin_voice_prompts_variants.h>
#include <audio_plugin_voice_assistant_variants.h>
#include <gain_utils.h>


#if defined(__KALIMBA__) && !defined(__GNUC__)
# pragma unitsuppress OuterDefinition    /* MAKE_...MESSAGE macros used at different levels in same function */
#endif

typedef struct audio_lib_Tag
{
    Task control_plugin;                    /* plugin where all the messages are sent, not necessarily active yet */
    Task relay_plugin;                      /* plugin used for managing stream relay operation */
    Task AUDIO_BUSY;                        /* audio routing currently in progress */
    Task audio_prompt_playing_plugin;
    Task output_plugin;
    AUDIO_PLUGIN_CONNECT_MSG_T message;     /* store of current connection params */
    DSP_STATUS_INFO_T dsp_status;           /* current dsp status */
    Sink forwarding_sink;
    Sink left_tone_mixing_sink;                  /* to be set by non VP plugins, so VP plugin can connect and mix together */
    Sink right_tone_mixing_sink;
    unsigned number_of_audio_connections:3;
    unsigned audio_in_use:1;                /* audio currently being routed */
    unsigned asr_running:1;                 /* asr is currently running/listening */
    unsigned content_protection:1;
    unsigned unused:10;
    Task audio_voice_assistant_plugin; /* Voice assistant plugin task*/
}AUDIO_t ;


static Task audioGetControlPlugin(void);
static void audioSetControlPlugin(Task plugin);
static bool CallbackCheckRequired(TaskData *oldtask, TaskData *newtask);
static void MakeBusyCallback(TaskData *task);

static AUDIO_t * AUDIO;


void AudioLibraryInit (  void )
{
    if (AUDIO == NULL)
    {
        /* malloc one allocation for audio and plugin use */
        AUDIO = PanicUnlessMalloc(sizeof(AUDIO_t));

        /* initialise to 0 */
        memset(AUDIO,0,sizeof(AUDIO_t));
    }
}


/****************************************************************************
NAME
    audioCanConnect

DESCRIPTION
    This function returns a boolean to indicate whether resources are available to allow a connect
    request to succeed

RETURNS
    void
*/
static bool audioCanConnect(void)
{
    unsigned max_connections = AudioConfigGetMaximumConcurrentInputs();
    return (AUDIO->number_of_audio_connections < max_connections);
}


/****************************************************************************
NAME
    audioCancelMessage

DESCRIPTION
    This function sends cancel message requests to the active plugin, along with the plugin passed as a parameter

RETURNS
    void
*/
static void audioCancelMessage(Task plugin, audio_plugin_interface_message_type_t msg)
{
    Task audio_plugin = audioGetControlPlugin();

    if(audio_plugin)
    {
        MessageCancelAll(audio_plugin, msg);
    }
    if (audio_plugin != plugin)
        MessageCancelAll(plugin, msg);
}


audio_instance_t AudioConnect ( Task audio_plugin,
                     Sink audio_sink,
                     AUDIO_SINK_T sink_type,
                     int16 volume,
                     uint32 rate,
                     AudioPluginFeatures features,
                     AUDIO_MODE_T mode,
                     AUDIO_ROUTE_T route,
                     AUDIO_POWER_T power,
                     void * params,
                     Task app_task )
{
    PanicNull(AUDIO);
    if (audioCanConnect() || (sink_type == AUDIO_SINK_SIDE_GRAPH))
    {
        MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_CONNECT_MSG, message );
        PanicNull(audio_plugin);

        message->audio_sink = audio_sink ;
        message->sink_type  = sink_type ;
        message->volume     = volume ;
        message->rate       = rate;
        message->mode       = mode ;
        message->route      = route;
        message->features   = features ;
        message->params     = params ;
        message->app_task   = app_task ;
        message->power      = power ;

        if(sink_type != AUDIO_SINK_SIDE_GRAPH)
        {
            AUDIO->number_of_audio_connections++;
            AUDIO->message = *message;
            audioSetControlPlugin(audio_plugin);
        }

        PRINT(("AUD: AudioConnect pl[%p] sk[%p] bsy[%p]\n", (void*)audio_plugin, (void*)audio_sink, (void*)AUDIO->AUDIO_BUSY ));
        MessageSendConditionallyOnTask ( audio_plugin, AUDIO_PLUGIN_CONNECT_MSG, message, AudioBusyPtr() );

        return PanicNull(AudioInstanceCreate(audio_plugin, audio_sink, sink_type));
    }
    return (audio_instance_t)NULL;
}

static void audioDisconnectTask(Task audio_plugin, AUDIO_SINK_T sink_type)
{

    PanicNull(AUDIO);

    if(audio_plugin
        && (AUDIO->number_of_audio_connections || (sink_type == AUDIO_SINK_SIDE_GRAPH)))
    {
        /*  Cancel all volume related messages immediately.
            This stops stale volume messages being sent to a new audio connection
            that uses the same plugin. */
        MessageCancelAll(audio_plugin, AUDIO_PLUGIN_SET_VOLUME_MSG);
        MessageCancelAll(audio_plugin, AUDIO_PLUGIN_SET_GROUP_VOLUME_MSG);
        MessageCancelAll(audio_plugin, AUDIO_PLUGIN_RESET_VOLUME_MSG);
        MessageCancelAll(audio_plugin, AUDIO_PLUGIN_DELAY_VOLUME_SET_MSG);
        MessageCancelAll(audio_plugin, AUDIO_PLUGIN_SET_SOFT_MUTE_MSG);
        MessageCancelAll(audio_plugin, AUDIO_PLUGIN_ALLOW_VOLUME_CHANGES_MSG);

        /* Cancel the Power message immediately. 
           May result in receiving set power message after ba_ctx is freed after disconnection */
        MessageCancelAll(audio_plugin, AUDIO_PLUGIN_SET_POWER_MSG );

        MessageSendConditionallyOnTask (audio_plugin, AUDIO_PLUGIN_DISCONNECT_MSG, 0, AudioBusyPtr()) ;

        if(sink_type != AUDIO_SINK_SIDE_GRAPH)
        {
            AUDIO->number_of_audio_connections--;
        }
    }

    if (audio_plugin == audioGetControlPlugin())
    {
        audioSetControlPlugin(NULL);
        AUDIO->relay_plugin = NULL;
    }
}

void AudioDisconnect(void)
{
    Task audio_plugin = audioGetControlPlugin();

    PanicFalse(AudioConfigGetMaximumConcurrentInputs() == 1);
    if ( audio_plugin )
    {
        audio_instance_t instance =
            PanicNull(AudioInstanceGetInstanceFromTask(audio_plugin));
        audioDisconnectTask(audio_plugin,
                            AudioInstanceGetSinkType(instance));
        AudioInstanceDestroy(instance);
    }
    else
    {
        PRINT(("AUD: AudioDisconnect ignored, not connected\n"));
    }
}

void AudioDisconnectInstance(audio_instance_t instance)
{
    audioDisconnectTask(AudioInstanceGetPluginTask(instance),
                        AudioInstanceGetSinkType(instance));

    AudioInstanceDestroy(instance);
}

void AudioSetVolume( int16 volume , int16 tone_volume)
{
    Task plugin = audioGetControlPlugin();

    if ( plugin )
    {
        MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_SET_VOLUME_MSG, message ) ;

        message->volume     = volume ;
        message->tone_volume = tone_volume;

        MessageSendConditionallyOnTask ( plugin, AUDIO_PLUGIN_SET_VOLUME_MSG, message, AudioBusyPtr() ) ;

        AUDIO->message.volume = volume;
    }

    AudioConfigSetMasterVolume(VolumeConvertDACGainToDB(volume));
}


bool AudioSetMode ( AUDIO_MODE_T mode, void * params )
{
    bool lResult = FALSE ;
    Task plugin = audioGetControlPlugin();

    if (plugin)
    {
        MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_SET_MODE_MSG, message ) ;

        message->mode = mode ;
        message->params = params ;

        MessageSendConditionallyOnTask ( plugin, AUDIO_PLUGIN_SET_MODE_MSG, message, AudioBusyPtr() ) ;

        lResult = TRUE ;
        AUDIO->message.mode   = mode;
    }

    return lResult ;
}


bool AudioSetMusicProcessingEnhancements (A2DP_MUSIC_PROCESSING_T music_processing_mode, uint16 music_processing_enhancements)
{
    bool lResult = FALSE ;
    Task plugin = audioGetControlPlugin();

    if (plugin)
    {
        MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_SET_MUSIC_ENHANCEMENTS_MSG, message );
        memset(message, 0, sizeof(AUDIO_PLUGIN_SET_MUSIC_ENHANCEMENTS_MSG_T));

        message->music_processing_mode = music_processing_mode;
        message->music_processing_enhancements = music_processing_enhancements ;

        MessageSendConditionallyOnTask ( plugin, AUDIO_PLUGIN_SET_MUSIC_ENHANCEMENTS_MSG, message, AudioBusyPtr() ) ;

        lResult = TRUE ;
    }

    return lResult ;
}


void AudioSetInputAudioMute (bool enable)
{
    Task plugin = audioGetControlPlugin();
    if (plugin)
    {
        MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_SET_INPUT_AUDIO_MUTE_MSG, message ) ;
        memset(message, 0, sizeof(AUDIO_PLUGIN_SET_INPUT_AUDIO_MUTE_MSG_T));

        message->input_audio_port_mute_active = BITFIELD_CAST(1, enable) ;

        MessageSendConditionallyOnTask ( plugin, AUDIO_PLUGIN_SET_INPUT_AUDIO_MUTE_MSG, message, AudioBusyPtr() );
    }
}


bool AudioConfigureSubWoofer(AUDIO_SUB_TYPE_T  sub_woofer_type, Sink sub_sink )
{
    bool lResult = FALSE ;
    Task plugin = audioGetControlPlugin();

    if (plugin)
    {
        MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_SET_SUB_WOOFER_MSG, message ) ;

        message->sub_woofer_type = sub_woofer_type ;
        message->sub_sink = sub_sink ;

        MessageSendConditionallyOnTask ( plugin, AUDIO_PLUGIN_SET_SUB_WOOFER_MSG, message, AudioBusyPtr() ) ;

        lResult = TRUE ;
    }

    return lResult ;
}

static AUDIO_MUTE_STATE_T getMuteStateFromMessage(AUDIO_PLUGIN_SET_SOFT_MUTE_MSG_T * message)
{
    return ((message->mute_states & AUDIO_MUTE_MASK(audio_mute_group_main)) ? AUDIO_MUTE_ENABLE : AUDIO_MUTE_DISABLE);
}

bool AudioSetSoftMute(AUDIO_PLUGIN_SET_SOFT_MUTE_MSG_T* mute_message)
{
    bool lResult = FALSE;
    Task plugin = audioGetControlPlugin();

    if (plugin)
    {
        MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_SET_SOFT_MUTE_MSG, message );

        memcpy(message, mute_message, sizeof(AUDIO_PLUGIN_SET_SOFT_MUTE_MSG_T));

        AudioConfigSetMasterMuteState(getMuteStateFromMessage(message));

        MessageSendConditionallyOnTask ( plugin, AUDIO_PLUGIN_SET_SOFT_MUTE_MSG, message, AudioBusyPtr() );

        lResult = TRUE ;
    }

    return lResult ;
}


bool AudioStartForwarding(Task relay_plugin, Sink forwarding_sink, bool content_protection, peer_buffer_level buffer_level, Task output_plugin)
{
    bool lResult = FALSE ;
    Task plugin = audioGetControlPlugin();

    PanicNull(AUDIO);

    if ( plugin && ((AUDIO->relay_plugin == NULL) || (AUDIO->relay_plugin == relay_plugin)))
    {
        MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_START_FORWARDING_MSG, message ) ;

        message->forwarding_sink = forwarding_sink ;
        message->content_protection = content_protection ;
        message->peer_dsp_required_buffering_level = buffer_level ;
        message->output_plugin = output_plugin;

        AUDIO->relay_plugin = relay_plugin;
        AUDIO->forwarding_sink = forwarding_sink;
        AUDIO->content_protection = BITFIELD_CAST(1, content_protection);
        AUDIO->output_plugin = output_plugin;

        MessageSendConditionallyOnTask ( AUDIO->relay_plugin, AUDIO_PLUGIN_START_FORWARDING_MSG, message, AudioBusyPtr() ) ;

        lResult = TRUE ;
    }

    return lResult ;
}


bool IsAudioRelaying(void)
{
    PanicNull(AUDIO);

    if(AUDIO->relay_plugin != NULL)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


void AudioStopForwarding(void)
{
    Task plugin = audioGetControlPlugin();

    PanicNull(AUDIO);

    if ( plugin && AUDIO->relay_plugin)
    {
        MessageSendConditionallyOnTask ( AUDIO->relay_plugin, AUDIO_PLUGIN_STOP_FORWARDING_MSG, NULL, AudioBusyPtr() ) ;

        AUDIO->relay_plugin = NULL ;
    }
}


void AudioPlayTone ( const ringtone_note * tone , bool can_queue ,  int16 tone_volume , AudioPluginFeatures features )
{
    if (!IsAudioBusy() || can_queue)
    {
        Task plugin = audioGetControlPlugin();

        MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_PLAY_TONE_MSG, message ) ;

        message->tone        = tone;
        message->can_queue   = can_queue ;
        message->tone_volume = tone_volume  ;
        message->features    = features ;

        if(plugin)
        {
            PRINT(("AUDIO: play tone, plugin = %p \n",(void*)plugin));
            MessageSendConditionallyOnTask ( plugin, AUDIO_PLUGIN_PLAY_TONE_MSG, message, AudioBusyPtr() ) ;
        }
        else
        {
            PRINT(("AUDIO: play tone, no plugin \n"));
            /* Forward message to the Voice Prompts plugin as the DSP is required for multi-channel tones */
            MessageSendConditionallyOnTask( (TaskData*)&csr_voice_prompts_plugin, AUDIO_PLUGIN_PLAY_TONE_MSG, message, AudioBusyPtr() ) ;
        }
    }
    else
    {
        PRINT(("AUDIO: discard tone \n"));
    }
}

static void cancelPendingTones(void)
{
    Task audio_plugin = audioGetControlPlugin();
    if(audio_plugin)
    {
        MessageCancelAll(audio_plugin, AUDIO_PLUGIN_PLAY_TONE_MSG);
    }
    MessageCancelAll(AudioGetTonePlugin(), AUDIO_PLUGIN_PLAY_TONE_MSG);
}

static void cancelPendingVoicePrompts(void)
{
    Task audio_plugin = audioGetControlPlugin();
    if(audio_plugin)
    {
        MessageCancelAll(audio_plugin, AUDIO_PLUGIN_PLAY_AUDIO_PROMPT_MSG);
    }
    MessageCancelAll(AudioGetTonePlugin(), AUDIO_PLUGIN_PLAY_AUDIO_PROMPT_MSG);
}

void AudioStopToneAndPrompt(bool prompt_terminate)
{
    PRINT(("AUDIO: STOP tone, busy = %p\n",(void*)AUDIO->AUDIO_BUSY));

    cancelPendingTones();
    cancelPendingVoicePrompts();

    if(AudioIsAudioPromptPlaying())
    {
        if(prompt_terminate)
        {
            MessageSend(AudioGetAudioPromptPlayingTask(), AUDIO_PLUGIN_STOP_TONE_AND_PROMPT_MSG, 0);
        }
    }
}


void AudioPlayAudioPrompt ( Task plugin, FILE_INDEX prompt_index, FILE_INDEX prompt_header_index, bool can_queue, int16 ap_volume, AudioPluginFeatures features, bool override, Task app_task )
{
    PanicNull(AUDIO);

    if ((uint16*)AUDIO->AUDIO_BUSY == NULL || can_queue)
    {
        Task audio_plugin = audioGetControlPlugin();
        MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_PLAY_AUDIO_PROMPT_MSG, message ) ;

        message->prompt_index          = prompt_index;
        message->prompt_header_index   = prompt_header_index;
        message->can_queue             = can_queue;
        message->app_task              = app_task;
        message->ap_volume             = ap_volume;
        message->features              = features;

        if (audio_plugin)
        {
            /* if this voice prompt is set to play immediately, cancel any queued prompts */
            if(override)
            {
                audioCancelMessage(plugin, AUDIO_PLUGIN_PLAY_AUDIO_PROMPT_MSG);
                MessageCancelAll( plugin, AUDIO_PLUGIN_CONNECT_MSG);
            }

            /* determine whether voice prompts can be mixed with dsp audio, to do this
               voice prompts must be in adpcm or pcm format and the audio plugin must be capable
               of supporting tone mixing via kalimba port 3 (tone port) */
            /* OR if the audio plugin connect message is in the queue but not yet actioned, queue
               the Audio Prompt message until the audio plugin is loaded, then decide what to do with it */
            if((GetCurrentDspStatus() || !IsAudioInUse()) &&
               (CsrVoicePromptsIsMixable(prompt_header_index)))
            {
                PRINT(("AUD play Audio Prompt via DSP mixing\n"));
                MessageSendConditionallyOnTask ( plugin , AUDIO_PLUGIN_PLAY_AUDIO_PROMPT_MSG ,message , AudioBusyPtr() ) ;
            }
            /* audio is connected via audio plugin and not using the DSP or not ADPCM voice prompts
               so need to disconnect current plugin prior to playing voice prompts */
            else
            {

                PRINT(("AUD play -Audio Prompt disconnect audio\n"));
                /*if we have a plugin connected, then perform the disconnect*/
                MessageSendConditionallyOnTask ( audio_plugin, AUDIO_PLUGIN_DISCONNECT_MSG , 0 , AudioBusyPtr() ) ;
                MessageSendConditionallyOnTask ( plugin , AUDIO_PLUGIN_PLAY_AUDIO_PROMPT_MSG , message , AudioBusyPtr() ) ;

                /* Queue message to reconnect plugin */
                {
                    MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_CONNECT_MSG, connect_message ) ;
                    *connect_message = AUDIO->message;
                    MessageSendConditionallyOnTask ( audio_plugin, AUDIO_PLUGIN_CONNECT_MSG , connect_message , AudioBusyPtr() ) ;
                }

                /*  Start the reconnected plugin with all audio groups muted.
                    When the application receives the following AUDIO_REFRESH_VOLUME message it should
                    restore the the correct mute status for all the groups using AudioSetSoftMute().
                    If, for example, the mic was muted in the audio plugin before the audio prompt was played,
                    this ensures the mic remains muted until the application has chance to update the mute status.
                */
                {
                    MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_SET_SOFT_MUTE_MSG, soft_mute_message ) ;
                    soft_mute_message->mute_states = AUDIO_MUTE_ENABLE_ALL;
                    soft_mute_message->unused = 0;
                    MessageSendConditionallyOnTask ( audio_plugin, AUDIO_PLUGIN_SET_SOFT_MUTE_MSG , soft_mute_message , AudioBusyPtr() ) ;
                }

                /* Request that the app refreshes the volume and mute status after reconnect */
                {
                    PRINT(("AUD:Vol refresh\n"));
                    MessageSendConditionallyOnTask(app_task, AUDIO_REFRESH_VOLUME, NULL, AudioBusyPtr());
                }

                if ( NULL != AUDIO->relay_plugin)
                {
                    MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_START_FORWARDING_MSG, start_forwarding_message ) ;

                    start_forwarding_message->forwarding_sink = AUDIO->forwarding_sink ;
                    start_forwarding_message->content_protection = AUDIO->content_protection ;
                    start_forwarding_message->output_plugin = AUDIO->output_plugin;

                    MessageSendConditionallyOnTask ( AUDIO->relay_plugin, AUDIO_PLUGIN_START_FORWARDING_MSG, start_forwarding_message , AudioBusyPtr()) ;
                }
            }
        }
        else
        {
            PRINT(("AUD play Audio Prompt no plugin\n"));
            /* if this voice prompt is set to play immediately, cancel any queued prompts */
            if(override)
                audioCancelMessage(plugin, AUDIO_PLUGIN_PLAY_AUDIO_PROMPT_MSG);

            MessageSendConditionallyOnTask ( plugin , AUDIO_PLUGIN_PLAY_AUDIO_PROMPT_MSG ,message , AudioBusyPtr() ) ;
        }
    }
    else
    {
        PRINT(("AUDIO: discard Audio Prompt\n"));
    }
}


void AudioSetPower(AUDIO_POWER_T power)
{
    Task plugin = audioGetControlPlugin();

    if (plugin)
    {
        MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_SET_POWER_MSG, message ) ;

        message->power = power ;
        /* NB. Don't update AUDIO.message or we may end up with no DSP and metadata enabled */
        MessageSendConditionallyOnTask (plugin, AUDIO_PLUGIN_SET_POWER_MSG, message, AudioBusyPtr()) ;
    }

}


static void sendProductionTestCommand(audio_plugin_interface_message_type_t message_type)
{
    Task plugin = audioGetControlPlugin();

    if(plugin)
    {
        MessageSendConditionallyOnTask ( plugin, message_type, 0, AudioBusyPtr() ) ;
    }
}


void AudioMicSwitch ( void )
{
    PRINT(("AUD: MicSwitch\n") );

    sendProductionTestCommand(AUDIO_PLUGIN_MIC_SWITCH_MSG);
}


void AudioOutputSwitch(void)
{
    PRINT(("AUD: OutputSwitch\n"));

    sendProductionTestCommand(AUDIO_PLUGIN_OUTPUT_SWITCH_MSG);
}


bool AudioStartASR ( AUDIO_MODE_T mode )
{
    bool lResult = FALSE ;
    Task plugin = audioGetControlPlugin();

    PRINT(("AUD: AudioStartASR"));

    if (plugin)
    {
        MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_START_ASR, message ) ;

        message->mode = mode ;

        MessageSendConditionallyOnTask ( plugin, AUDIO_PLUGIN_START_ASR, message, AudioBusyPtr() ) ;

        lResult = TRUE ;
    }
    return lResult;
}


bool IsAudioBusy(void)
{
    if(AUDIO == NULL)
        return FALSE;

    PRINT(("AUD: IsAudioBusy = %s \n",AUDIO->AUDIO_BUSY?"TRUE":"FALSE")) ;
    return AUDIO->AUDIO_BUSY == NULL ? FALSE : TRUE;
}


Task AudioBusyTask(void)
{
    PanicNull(AUDIO);

    PRINT(("AUD: AudioBusyTask = 0x%X\n",(unsigned)AUDIO->AUDIO_BUSY));
    return AUDIO->AUDIO_BUSY;

}


const Task * AudioBusyPtr(void)
{
    PanicNull(AUDIO);

    PRINT(("AUD: AudioBusyPtr = %p\n", (void*)&AUDIO->AUDIO_BUSY));
    return &AUDIO->AUDIO_BUSY;
}


void SetAudioBusy(TaskData* newtask)
{
    TaskData *oldtask;

    PanicNull(AUDIO);
    oldtask = (TaskData *)AUDIO->AUDIO_BUSY;

    PRINT(("AUD: SetAudioBusy = %p -> %p \n",
            (void *) oldtask, (void *) newtask));

    if (CallbackCheckRequired(oldtask, newtask))
    {
        MakeBusyCallback(oldtask);
    }

    AUDIO->AUDIO_BUSY = newtask;
}


/****************************************************************************
NAME
    CallbackCheckRequired

DESCRIPTION
    Looks to see whether a task no longer has an audio lock

RETURNS
        TRUE if a task used to have the lock but doesn't any more
        FALSE otherwise

*/
static bool CallbackCheckRequired(TaskData *oldtask, TaskData *newtask)
{
    return (oldtask && (newtask != oldtask)) ? TRUE : FALSE ;
}


/****************************************************************************
NAME
    MakeBusyCallback

DESCRIPTION
    Makes a callback to the task losing the audio lock.

        This is useful because we can't always rely on all tasks to play nicely
        with the audio lock.  This mechanism provides a means for a task to
        reset its state when another task either grabs the busy lock or frees
        it when it shouldn't have.

        Hard coded callback functions have to be used instead of more elegant
        options involving either a table of function pointers or getting tasks
        to register their own callback.  The reason is that the ADK stack-size
        calculator assumes that any function pointer can hold the value of
        _any_ function whose address has ever been taken, which inevitably
        leads to the assumed possibility of recursion and a failure in the
        stack size calculation.

RETURNS

*/
static void MakeBusyCallback(TaskData *oldtask)
{
    UNUSED(oldtask);

    if (oldtask == &csr_voice_prompts_plugin)
    {
        PRINT(("AUD: Calling CsrVoicePromptsPluginCleanup()\n"));
        CsrVoicePromptsPluginCleanup();
    }
}


DSP_STATUS_INFO_T GetCurrentDspStatus(void)
{
    PanicNull(AUDIO);
    return AUDIO->dsp_status;
}


void SetCurrentDspStatus(DSP_STATUS_INFO_T status)
{
    PanicNull(AUDIO);
    AUDIO->dsp_status = status;
}


bool IsAudioInUse(void)
{
    PanicNull(AUDIO);

    PRINT(("AUD: IsAudioInUse = %s\n", AUDIO->audio_in_use ? "TRUE":"FALSE"));
    return AUDIO->audio_in_use;
}


void SetAudioInUse(bool status)
{
    PanicNull(AUDIO);

    AUDIO->audio_in_use = BITFIELD_CAST(1, status);
    PRINT(("AUD: SetAudioInUse = %s \n", AUDIO->audio_in_use ? "TRUE":"FALSE"));
}

void AudioSetRelayPlugin(Task plugin)
{
    if (AUDIO)
        AUDIO->relay_plugin = plugin;
}

void AudioSetAudioPromptPlayingTask(Task plugin)
{
    if(AUDIO)
    {
        AUDIO->audio_prompt_playing_plugin = plugin;
    }
}

Task AudioGetAudioPromptPlayingTask(void)
{
    Task prompt_playing_task = (Task)NULL;
    if(AUDIO)
    {
        prompt_playing_task = AUDIO->audio_prompt_playing_plugin;
    }
    return prompt_playing_task;
}

bool AudioIsAudioPromptPlaying(void)
{
    return (AudioGetAudioPromptPlayingTask() ? TRUE : FALSE);
}

bool IsAsrPlaying(void)
{
    PanicNull(AUDIO);
    return AUDIO->asr_running;
}


void SetAsrPlaying(bool status)
{
    PanicNull(AUDIO);
    AUDIO->asr_running = BITFIELD_CAST(1, status);
}

/****************************************************************************
NAME
    audioGetControlPlugin

DESCRIPTION
    This method returns the audio plugin associated with the most recent AudioConnect message.
    The message has not necessarily been processed, so it may be that it does not point to a
    currently active plugin. It is intentionally made static as only the audio library should
    be accessing this data.
    The primary use of this call is to get the plugin to be used for sending messages to.

RETURNS
    Task  associated with the most recent AudioConnect message
*/
static Task audioGetControlPlugin(void)
{
    PanicNull(AUDIO);
    return AUDIO->control_plugin;
}


/****************************************************************************
NAME
    audioSetControlPlugin

DESCRIPTION
    This method sets the audio plugin associated with the most recent AudioConnect message.
    The message has not necessarily been processed, so it may be that it does not point to a
    currently active plugin. It is intentionally made static as only the audio library should
    be accessing this data.

RETURNS

*/
static void audioSetControlPlugin(Task plugin)
{
    PanicNull(AUDIO);
    AUDIO->control_plugin = plugin;
}


bool AudioIsControlPlugin(Task plugin_to_check)
{
    PanicNull(AUDIO);

    if(audioGetControlPlugin() == plugin_to_check)
    {
        return TRUE;
    }

    return FALSE;
}

void AudioOverrideControlPlugin(Task plugin)
{
    audioSetControlPlugin(plugin);
}


Task AudioGetTonePlugin(void)
{
    return (Task)&csr_voice_prompts_plugin;
}

/****************************************************************************
NAME
    AudioGetVaPlugin

DESCRIPTION
    This function returns voice assistant plugin

RETURNS
    void
*/
static Task AudioGetVaPlugin(void)
{
    /* Some of the operator traps which are defined under TRAPSET_WAKE_ON_AUDIO 
    in operator.h file are not available for crescendo.
    So this is a temporary fix to avoid crescendo build issues. This should remove once 
    supported traps available for crescendo. */
#ifdef CRESCENDO_OPERATOR_TRAP_TEMP_FIX
    return NULL;
#else
    return(Task)&voice_assistant_plugin;
#endif
}

void AudioSetGroupVolume(AUDIO_PLUGIN_SET_GROUP_VOLUME_MSG_T *msg)
{
    Task plugin = audioGetControlPlugin();
    if(plugin)
    {
        MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_SET_GROUP_VOLUME_MSG, message ) ;
        memcpy(message, msg, sizeof(AUDIO_PLUGIN_SET_GROUP_VOLUME_MSG_T));
        MessageSendConditionallyOnTask ( plugin, AUDIO_PLUGIN_SET_GROUP_VOLUME_MSG, message, AudioBusyPtr() ) ;

        AUDIO->message.volume = msg->main.master;
    }

    if(msg->group == audio_output_group_main || msg->group == audio_output_group_all)
    {
        AudioConfigSetMasterVolume(msg->main.master);
    }
}


void AudioSetMaximumConcurrentAudioConnections(unsigned connections)
{
    AudioConfigSetMaximumConcurrentInputs(connections);
}


bool AudioSetUserEqParameter(const Task task, const audio_plugin_user_eq_param_t* param)
{
    if (task)
    {
        MAKE_AUDIO_MESSAGE(AUDIO_PLUGIN_SET_USER_EQ_PARAMETER_MSG, message);
        message->param = param[0];
        MessageSendConditionallyOnTask (task, AUDIO_PLUGIN_SET_USER_EQ_PARAMETER_MSG, message, AudioBusyPtr()) ;

        return TRUE;
    }
    return FALSE;
}

bool AudioApplyUserEqParameters(const Task task, bool recalculate_coefficients)
{
    if (task)
    {
        MAKE_AUDIO_MESSAGE(AUDIO_PLUGIN_APPLY_USER_EQ_PARAMETERS_MSG, message);
        message->recalculate_coefficients = recalculate_coefficients;
        MessageSendConditionallyOnTask (task, AUDIO_PLUGIN_APPLY_USER_EQ_PARAMETERS_MSG, message, AudioBusyPtr()) ;
        return TRUE;
    }
    return FALSE;
}

bool AudioClearUserEqParameters(const Task task)
{
    if (task)
    {
        MessageSendConditionallyOnTask (task, AUDIO_PLUGIN_CLEAR_USER_EQ_PARAMETERS_MSG, NULL, AudioBusyPtr()) ;
        return TRUE;
    }
    return FALSE;
}

bool AudioGetUserEqParameter(const Task task, const audio_plugin_user_eq_param_id_t* param, Task callback_task)
{
    if (task)
    {
        MAKE_AUDIO_MESSAGE(AUDIO_PLUGIN_GET_USER_EQ_PARAMETER_MSG, message);
        message->param_id = param[0];
        message->callback_task = callback_task;
        MessageSendConditionallyOnTask (task, AUDIO_PLUGIN_GET_USER_EQ_PARAMETER_MSG, message, AudioBusyPtr()) ;

        return TRUE;
    }
    return FALSE;
}

bool AudioGetUserEqParameters(const Task task, const unsigned number_of_params, const audio_plugin_user_eq_param_id_t* param_ids, Task callback_task)
{
    if (task)
    {
        unsigned i;
        size_t message_size = CALC_MESSAGE_LENGTH_WITH_VARIABLE_PARAMS(AUDIO_PLUGIN_GET_USER_EQ_PARAMETERS_MSG_T, number_of_params, audio_plugin_user_eq_param_id_t);
        AUDIO_PLUGIN_GET_USER_EQ_PARAMETERS_MSG_T* message = PanicUnlessMalloc(message_size);
        memset(message, 0, message_size);
        message->callback_task = callback_task;
        message->number_of_params = (uint16)number_of_params;
        for (i = 0; i < number_of_params; i++)
        {
            message->param_ids[i] = param_ids[i];
        }
        MessageSendConditionallyOnTask (task, AUDIO_PLUGIN_GET_USER_EQ_PARAMETERS_MSG, message, AudioBusyPtr());

        return TRUE;
    }
    return FALSE;
}

void AudioSetSpeakerPeqBypass(bool speaker_peq_bypass)
{
    AudioConfigSetSpeakerPeqBypass(speaker_peq_bypass);
}

bool AudioSetTwsChannelModes(const Task task, const AUDIO_MUSIC_CHANNEL_MODE_T channel_mode_master, const AUDIO_MUSIC_CHANNEL_MODE_T channel_mode_slave)
{
    AudioConfigSetTwsChannelModeLocal(channel_mode_master);
    AudioConfigSetTwsChannelModeRemote(channel_mode_slave);

    if (task)
    {
        MessageSendConditionallyOnTask (task, AUDIO_PLUGIN_SET_CHANNEL_MODE_MSG, NULL, AudioBusyPtr());
        return TRUE;
    }
    return FALSE;
}

/*  This function is required as the Bluecore VA sets the audio
    busy pointer when running.
    This means that we can't stop it if the stop message is sent
    conditionally on AudioBusyPtr(). The stop message has to be 
    sent conditionally to prevent a race with the connect message
    sent by AudioConnect
*/
static const Task * AudioBusyWithSomethingNotVA(void)
{
    static const Task pointer_to_null = NULL;

    if(*AudioBusyPtr() != AudioGetVaPlugin())
    {
        return AudioBusyPtr();
    }
    return &pointer_to_null;
}

void AudioVoiceAssistantEnableTrigger(FILE_INDEX trigger_file)
{
    MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_START_TRIGGER_DETECT_MSG, message);
    message->trigger_phrase_data_file = trigger_file;
    MessageSendConditionallyOnTask(AudioGetVaPlugin(), AUDIO_PLUGIN_START_TRIGGER_DETECT_MSG, message, AudioBusyPtr());
}

void AudioVoiceAssistantDisableTrigger(void)
{
    MessageSendConditionallyOnTask(AudioGetVaPlugin(), AUDIO_PLUGIN_STOP_TRIGGER_DETECT_MSG, NULL, AudioBusyPtr());
}

void AudioVoiceAssistantStartSend(bool use_timestamp, uint32 start_timestamp)
{
    MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_START_VOICE_CAPTURE_MSG, message);
    message->use_timestamp = use_timestamp;
    message->start_timestamp = start_timestamp;
    MessageSendConditionallyOnTask(AudioGetVaPlugin(), AUDIO_PLUGIN_START_VOICE_CAPTURE_MSG, message, AudioBusyPtr());
}

void AudioVoiceAssistantStopSend(void)
{
    MessageSendConditionallyOnTask(AudioGetVaPlugin(), AUDIO_PLUGIN_STOP_VOICE_CAPTURE_MSG, NULL, AudioBusyWithSomethingNotVA());
}


#ifdef HOSTED_TEST_ENVIRONMENT
void AudioLibraryTestReset(Task audio_plugin)
{
    if(audio_plugin != NULL)
    {
        PRINT(("Reset Plugin %lx\n", (uint32)audio_plugin));
        MessageFlushTask(audio_plugin);
        MessageSend(audio_plugin, AUDIO_PLUGIN_TEST_RESET_MSG, NULL);
    }

    if(AUDIO)
    {
        PRINT(("Reset Audio\n"));
        free(AUDIO);
        AUDIO = NULL;
    }
    AudioInstanceTestReset();
}
#endif

/****************************************************************************
NAME
	AudioUpdateLatencySettings

DESCRIPTION
	Promp the plugin to send updated latency settings to the DSP app

RETURNS

*/
void AudioUpdateLatencySettings(void)
{
    Task audio_plugin = audioGetControlPlugin();

    if(audio_plugin)
    {
        MessageSendConditionallyOnTask(audio_plugin, AUDIO_PLUGIN_UPDATE_LATENCY_SETTINGS_MSG, NULL , AudioBusyPtr());
    }
}
