/****************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_processor.c

DESCRIPTION
    Short description about what functions the source file contains.
*/

#include <print.h>
#include <vmtypes.h>
#include <panic.h>
#include <audio/audio_if.h>
#include <audio_clock.h>
#include <string.h>    /* for memset */

#include "audio_processor.h"

/*
 * A bit map indicating the use cases from audio_ucid_t (from
 * audio_ucid_not_defined and less than audio_ucid_number_of_ucids) that
 * have been added and have not been removed.
 */
static audio_ucid_bit_map_t use_case_bit_map = 0;

/*
 * An array of counts for each of the use cases audio_ucid_t.
 * When a use case is added, its use_case_count element is incremented. If it
 * is incremented from zero to one then the bit in the use_case_bit_map is set.
 * When a use case is removed, its its use_case_count element is decremented.
 * If it is decremented to zero then the bit in the use_case_bit_map is cleared.
 */
static uint8 use_case_count[audio_ucid_number_of_ucids];

/*
 * A table of the bitmaps of the special cases against their clock rates.
 * If use_case_bit_map is set to a value in the bit_map field then the audio DSP
 * clock is set according to the value in the clock_rate field, otherwise the
 * high clock rate is used.
 */
audio_processor_config_t* audio_processor_config = NULL;

/* Cached current Audio clock rate value.
 *
 * Use this cached value to avoid calling AudioDspClockConfigure() when the new
 * clock value is the same as the current one.
 *
 * To avoid getting out of sync with the real value used by the audio chip,
 * this cached value is reset to audio_processor_clock_rate_not_set whenever
 * use_case_bit_map becomes 0. use_case_bit_map == 0 is used as a proxy for the
 * audio chip being powered off, i.e. when use_case_bit_map == 0, the app has
 * no active operators (or is about to stop the last one), so as far as it knows,
 * the audio chip will soon be 'off', so the clock rate will be undefined.
 */
static audio_processor_clock_rate_t cached_clock_rate = audio_processor_clock_rate_not_set;


#ifdef DEBUG_PRINT_ENABLED
/* Macro and functions to help with the readability of instrumentation */
#define ADD_CASE(token) case token: return #token

/*
 * Return the string for the audio_ucid_t enum value.
 */
static const char *get_use_case_enum_string(audio_ucid_t use_case)
{
    switch(use_case)
    {
    ADD_CASE(audio_ucid_not_defined);
    ADD_CASE(audio_ucid_input_a2dp);
    ADD_CASE(audio_ucid_input_a2dp_hq_codec);
    ADD_CASE(audio_ucid_input_broadcast);
    ADD_CASE(audio_ucid_input_analogue);
    ADD_CASE(audio_ucid_input_digital);
    ADD_CASE(audio_ucid_input_tone);
    ADD_CASE(audio_ucid_input_voice_prompts_decoder);
    ADD_CASE(audio_ucid_input_voice_prompts_pcm);
    ADD_CASE(audio_ucid_hardware_aec);
    ADD_CASE(audio_ucid_mixer_core);
    ADD_CASE(audio_ucid_mixer_core_voice);
    ADD_CASE(audio_ucid_mixer_core_low_power);
    ADD_CASE(audio_ucid_mixer_voice_resampler);
    ADD_CASE(audio_ucid_mixer_music_resampler);
    ADD_CASE(audio_ucid_mixer_prompt_resampler);
    ADD_CASE(audio_ucid_mixer_processing_mono_voice);
    ADD_CASE(audio_ucid_mixer_processing_prompt);
    ADD_CASE(audio_ucid_mixer_processing_stereo_music);
    ADD_CASE(audio_ucid_mixer_speaker_mono);
    ADD_CASE(audio_ucid_mixer_speaker_stereo);
    ADD_CASE(audio_ucid_mixer_speaker_stereo_bass);
    ADD_CASE(audio_ucid_mixer_speaker_stereo_xover);
    ADD_CASE(audio_ucid_mixer_speaker_voice_only);
    ADD_CASE(audio_ucid_hfp);
    ADD_CASE(audio_ucid_hfp_cvc_headset);
    ADD_CASE(audio_ucid_hfp_cvc_speaker);
    ADD_CASE(audio_ucid_output_broadcast);
    ADD_CASE(audio_ucid_output_tws);
    ADD_CASE(audio_ucid_output_digital);
    ADD_CASE(audio_ucid_hd_audio);
    ADD_CASE(audio_ucid_aov);
    ADD_CASE(audio_ucid_va);
    default: break;
    }
    return "unknown";
}

/*
 * Return the string for the audio_processor_clock_rate_t enum value.
 */
static const char * get_clock_rate_enum_string(audio_processor_clock_rate_t clock_rate)
{
    switch(clock_rate)
    {
    ADD_CASE(audio_processor_clock_rate_not_set);
    ADD_CASE(audio_processor_clock_rate_slow);
    ADD_CASE(audio_processor_clock_rate_medium);
    ADD_CASE(audio_processor_clock_rate_high);
    default: break;
    }
    return "unknown";
}

#endif

#define PRINT_USE_CASE(use_case) PRINT(("0x%016llx", use_case))

/*
 * Print the value of use_case_bit_map.
 * Used rather than calling PRINT with a format specifier in numerous places to
 * cater for the situation where the format of the use_case_bit_map changes.
 *
 * Parameter:   A string to use to prefix the output
 * Returns:     Nothing
 */
static void printUseCaseBitMap(const char *prefix)
{
    /* Mark prefix as unused in case PRINT is empty */
    UNUSED(prefix);
    PRINT(("%s 0x%016llx\n", prefix, use_case_bit_map));
}

/*
 * Set the active clock rate
 */
static bool setActiveClock(audio_dsp_clock_type active_clock)
{
#ifdef AUDIO_DSP_SPEED
    audio_dsp_clock_configuration clock_config;
    clock_config.active_mode    = active_clock;
    clock_config.low_power_mode = AUDIO_DSP_CLOCK_NO_CHANGE;
    clock_config.trigger_mode   = AUDIO_DSP_CLOCK_NO_CHANGE;
    
    return AudioDspClockConfigure(&clock_config);
#else
    UNUSED(active_clock);
    return FALSE;
#endif
}

/*
 * Convert clock rate from configuration into value understood by AudioDspClockConfigure
 */
static audio_dsp_clock_type convertClockType(audio_processor_clock_rate_t clock_rate)
{
    switch (clock_rate)
    {
        case audio_processor_clock_rate_slow:
            return AUDIO_DSP_SLOW_CLOCK;

        case audio_processor_clock_rate_medium:
            return AUDIO_DSP_BASE_CLOCK;

        case audio_processor_clock_rate_high:
            return AUDIO_DSP_TURBO_CLOCK;

        default:
            return AUDIO_DSP_CLOCK_NO_CHANGE;
    }
}

/*
 * Set the audio DSP clock rate.
 *
 * Parameter:   The new clock rate to be set
 */
static void setAudioDspClockRate(audio_processor_clock_rate_t clock_rate)
{
    PRINT(("Set clock to %s ", get_clock_rate_enum_string(clock_rate)));
    
    if (clock_rate == audio_processor_clock_rate_not_set)
    {
        PRINT(("Nothing to do\n"));
        return;
    }

    if (clock_rate == cached_clock_rate)
    {
        PRINT(("Unchanged\n"));
        return;
    }

    if(!setActiveClock(convertClockType(clock_rate)))
    {
        PRINT(("Failed\n"));
        return;
    }

    /* Only cache the new value if it was set successfully. */
    cached_clock_rate = clock_rate;
    
    PRINT(("Success\n"));
}

/*
 * Check whether a bit is set in use_case_bit_map.
 *
 * Parameter:   The bit to check
 * Returns:     TRUE if the bit is set, else FALSE
 */
static bool isUseCaseBitSet(audio_ucid_t use_case)
{
    return ((use_case_bit_map & USE_CASE_TO_BITMAP(use_case)) != 0);
}

/*
 * Set a bit in use_case_bit_map.
 *
 * Parameter:   The bit to set
 * Returns:     Nothing
 */
static void setUseCaseBit(audio_ucid_t use_case)
{
    use_case_bit_map |= USE_CASE_TO_BITMAP(use_case);
    printUseCaseBitMap("use_case_bit_map is now");
}

/*
 * Clear a bit in use_case_bit_map.
 *
 * Parameter:   The bit to clear
 * Returns:     Nothing
 */
static void clearUseCaseBit(audio_ucid_t use_case)
{
    use_case_bit_map &= ~USE_CASE_TO_BITMAP(use_case);
    printUseCaseBitMap("use_case_bit_map is now");
}

/*
 * Check the use_case_bit_map against those in the bit_map fields of the
 * use_case_audio_processor_clock_rate_table table. If found, set the
 * audio DSP clock rate to the value given in the clock_rate field.
 * If not found then assume that the highest clock rate is required.
 */
static void updateClockForCurrentUseCase(void)
{
    unsigned index;

    if(audio_processor_config)
    {
        for (index = 0; index < audio_processor_config->num_mappings; index++)
        {
            audio_processor_use_case_mapping_t* map = &audio_processor_config->mappings[index];
            
            PRINT(("Checking for use case "));
            PRINT_USE_CASE(map->bit_map);
            PRINT(("\n"));
            
            if(use_case_bit_map == map->bit_map)
            {
                setAudioDspClockRate(map->clock_rate);
                return;
            }
        }
        
        setAudioDspClockRate(audio_processor_config->default_rate);
    }
}

/******************************************************************************/
void AudioProcessorConfigure(audio_processor_config_t* config)
{
    audio_processor_config = config;
}

/******************************************************************************/
void AudioProcessorAddUseCase(audio_ucid_t use_case)
{
    PRINT(("AudioProcessorAddUseCase(%u): %s\n", use_case,
           get_use_case_enum_string(use_case)));
    if (audio_ucid_number_of_ucids > 64)
    {
        PRINT(("audio_ucid_number_of_ucids too large for unsigned 64-bit integer %d\n",
            audio_ucid_number_of_ucids));
        Panic();
    }

    if (use_case < audio_ucid_number_of_ucids)
    {
        if(use_case != audio_ucid_not_defined)
        {
            /* If the current use_case is 'none', reset the cached clock rate
               so that the next clock value will always be sent. */
            if (use_case_bit_map == 0)
                cached_clock_rate = audio_processor_clock_rate_not_set;

            use_case_count[use_case]++;
            setUseCaseBit(use_case);
        }
        updateClockForCurrentUseCase();
    }
    else
    {
        PRINT(("%u is not valid\n", use_case));
        Panic();
    }
}

/******************************************************************************/
void AudioProcessorRemoveUseCase(audio_ucid_t use_case)
{
    PRINT(("AudioProcessorRemoveUseCase(%u): %s\n", use_case,
           get_use_case_enum_string(use_case)));
    if (use_case < audio_ucid_number_of_ucids)
    {
        if (isUseCaseBitSet(use_case))
        {
            if  (use_case_count[use_case] > 0)
            {
                use_case_count[use_case]--;
            }
            else
            {
                PRINT(("%u bit set but count already zero!\n", use_case));
            }

            if  (use_case_count[use_case] == 0)
            {
                clearUseCaseBit(use_case);
                updateClockForCurrentUseCase();
            }
            else
            {
                PRINT(("%u count %d\n", use_case, use_case_count[use_case]));
            }
        }
        else
        {
            PRINT(("%u already removed\n", use_case));
        }
    }
    else
    {
        PRINT(("%u is not valid\n", use_case));
        Panic();
    }
}

/******************************************************************************/
bool AudioProcessorUseCaseIsInUse(audio_ucid_t use_case)
{
    if (use_case < audio_ucid_number_of_ucids)
        return isUseCaseBitSet(use_case);

    return FALSE;
}

/******************************************************************************/
#ifdef HOSTED_TEST_ENVIRONMENT
void AudioProcessorTestReset(void)
{
    use_case_bit_map = 0;
    memset(use_case_count, 0, sizeof(use_case_count));
    audio_processor_config = NULL;
    cached_clock_rate = audio_processor_clock_rate_not_set;
}
#endif
