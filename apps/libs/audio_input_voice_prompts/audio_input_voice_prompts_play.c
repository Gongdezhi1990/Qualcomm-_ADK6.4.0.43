/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_audio_prompts_play.c

DESCRIPTION
    Handler functions for voice prompts and tones source messages.
*/
#include <string.h>
#include <stdlib.h>
#include <operators.h>
#include <audio_mixer.h>
#include <audio.h>
#include <audio_plugin_music_params.h>
#include <audio_plugin_music_variants.h>
#include <panic.h>
#include <print.h>


#include "audio_input_voice_prompts_defs.h"
#include "audio_input_voice_prompts_play.h"
#include "audio_input_voice_prompts_file.h"
#include "audio_plugin_voice_prompts_variants.h"

static vp_context_t    *phrase_data = NULL;

static void setupContextBasedOnVPFile(void)
{
    Source lSource;
    voice_prompt_t prompt;

    /* Get the prompt data */
    if(!phrase_data)
    {
        return;
    }

    /* determine if this is a tone */
    if(phrase_data->tone)
    {
        /* Setting up prompt structure for tone. */
        prompt.codec_type = voice_prompts_codec_tone;
        prompt.playback_rate = TONE_SAMPLE_RATE;
        prompt.stereo = FALSE;

        /* audio source is always NULL in case of Kymera*/
        lSource = NULL;
    }
    else
    {
        lSource = VoicePromptsFileGetPrompt(phrase_data, &prompt);
        if(lSource)
        {
            MessageStreamTaskFromSource(lSource,(Task)&csr_voice_prompts_plugin);
        }
        else
        {
            Panic();
        }
    }

    /* Stash the source */
    phrase_data->source = lSource;
    phrase_data->codec_type = prompt.codec_type;
    phrase_data->stereo = prompt.stereo;
    phrase_data->playback_rate =  (prompt.playback_rate ? prompt.playback_rate : 8000);

    PRINT(("VP: %X rate 0x%x \n", phrase_data->codec_type,phrase_data->playback_rate));

    return;
}

static void DisconnectAndCloseSource(Source source)
{
    if(SourceIsValid(source))
    {
        StreamDisconnect(source, NULL);
        SourceClose(source);
    }
}

/****************************************************************************
DESCRIPTION
    Returns the pointer to Voice Prompt Context Data Structure
*/
vp_context_t *VoicePromptsGetContext(void)
{
    return phrase_data;
}


/****************************************************************************
DESCRIPTION
    plays a voice prompt using the audio plugin
*/
void AudioInputVoicePromptsPluginPlayPhrase (FILE_INDEX prompt_index, FILE_INDEX prompt_header_index, int16 no_dsp_prompt_volume , AudioPluginFeatures features, Task app_task)
{
    if(phrase_data != NULL)
        Panic();

    PRINT(("VP: Play Phrase:\n"));

    /* Allocate the memory */
    phrase_data = (vp_context_t *) PanicUnlessMalloc(sizeof(vp_context_t));
    memset(phrase_data,0,sizeof(vp_context_t));

    /* Set up params */
    phrase_data->prompt_index        = prompt_index;
    phrase_data->prompt_header_index = prompt_header_index;
    phrase_data->prompt_volume       = no_dsp_prompt_volume;
    phrase_data->features            = features;
    phrase_data->tone                = NULL;  /* not a tone */
    phrase_data->app_task            = app_task;

    setupContextBasedOnVPFile();

    SetAudioBusy((TaskData*) &(csr_voice_prompts_plugin));
    AudioSetAudioPromptPlayingTask((Task)&csr_voice_prompts_plugin);

    VoicePromptsDspPlay(phrase_data);
}

/****************************************************************************
DESCRIPTION
    plays a tone using the audio plugin
*/
void AudioInputVoicePromptsPluginPlayTone (const ringtone_note * tone, int16 tone_volume, AudioPluginFeatures features)
{
    if(tone == NULL)
        Panic();

    PRINT(("VP: Play tone:\n"));

    MessageCancelAll((TaskData*) &csr_voice_prompts_plugin, MESSAGE_STREAM_DISCONNECT );
    MessageCancelAll((TaskData*) &csr_voice_prompts_plugin, MESSAGE_FROM_KALIMBA);

    /* Allocate the memory */
    phrase_data = (vp_context_t *) PanicUnlessMalloc(sizeof(vp_context_t));
    memset(phrase_data,0,sizeof(vp_context_t));

    /* Set up params */
    phrase_data->prompt_index        = 0;
    phrase_data->prompt_header_index = 0; /* not a prompt */
    phrase_data->prompt_volume       = tone_volume;
    phrase_data->features            = features;
    phrase_data->tone                = tone;

    setupContextBasedOnVPFile();

    SetAudioBusy((TaskData*) &(csr_voice_prompts_plugin));
    AudioSetAudioPromptPlayingTask((TaskData*)&(csr_voice_prompts_plugin));

    VoicePromptsDspPlay(phrase_data);
}

/****************************************************************************
DESCRIPTION
    Act on a MESSAGE_STREAM_DISCONNECT being received by the plugin.

    If phrase_data has already been freed then there is no further work
    required (because we caused the stream disconnect ourselves during
    clean-up).
    If the prompt was being mixed, we must wait for the MUSIC_TONE_COMPLETE
    message to be sure that the prompt has finished playing and so we ignore
    the stream disconnect.  If it wasn't being mixed, the
    MESSAGE_STREAM_DISCONNECT signals that the prompt has finished.
*/
void AudioInputVoicePromptsPluginHandleStreamDisconnect(void)
{
    PRINT(("VP: Handle stream disconnect\n"));

    if(VoicePromptsDspShallHandleStreamDisconnect(phrase_data))
    {
        /* Prompt has finished so tidy up */
        AudioInputVoicePromptsPluginStopPhrase();
    }
    /* else
     * do nothing either because we have tidied up already or because
     * CsrVoicePromptsPluginCleanup() will be called when
     * AudioBusy is cleared on receipt of a MUSIC_TONE_COMPLETE
     */
}

/****************************************************************************
DESCRIPTION
    Stop a prompt from currently playing, start by fading out
*/
void AudioInputVoicePromptsPluginStopPhrase(void)
{
    if(AudioBusyTask() == &csr_voice_prompts_plugin)
    {
        VoicePromptsDspStop();
    }
}

/****************************************************************************
DESCRIPTION
    Callback called when the voice prompt plugin loses its audio busy lock
    either when CsrVoicePromptsPluginStopPhrase() voluntarily frees it or
    when some other ill-mannered task grabs or frees the lock itself.
*/
void CsrVoicePromptsPluginCleanup (void)
{
    if(!phrase_data)
        Panic();

    PRINT(("VP: Terminated\n"));

    VoicePromptsDspPrepareForClose(phrase_data);

    DisconnectAndCloseSource(phrase_data->source);

    VoicePromptsDspCleanup(phrase_data);

    /* Notify the application that the prompt has completed */
    if(phrase_data->app_task)
    {
        MessageSend(phrase_data->app_task, AUDIO_PROMPT_DONE, NULL);
    }

    freePhraseData();

    AudioSetAudioPromptPlayingTask((Task)NULL);

    MessageCancelAll ((TaskData *) &csr_voice_prompts_plugin,
            AUDIO_PLUGIN_STOP_TONE_AND_PROMPT_MSG) ;
}

/****************************************************************************
DESCRIPTION
    Free memory allocated with phrase_data
*/
void freePhraseData(void)
{
    free(phrase_data);
    phrase_data = NULL;
}
