/*
Copyright (c) 2017 Qualcomm Technologies International, Ltd.
*/
/**
\file
\ingroup sink_app
\brief   Interface to the audio clock functionality. 
*/

#ifdef ENABLE_AUDIO_CLOCK

/****************************************************************************
    Header files       */
#include <stdio.h>
#include <panic.h>
#include <audio_processor.h>

#include "sink_configmanager.h"
#include "sink_debug.h"
#include "sink_audio_clock.h"
#include "sink_audio_clock_config_def.h"

#ifdef DEBUG_AUDIO_CLOCK
#define AC_DEBUG(x) DEBUG(x)
#else
#define AC_DEBUG(x)
#endif

#define INPUT_USE_CASE_BASE     audio_ucid_input_a2dp
#define VOICE_USE_CASE_BASE     audio_ucid_hfp
#define PROMPT_USE_CASE_BASE    audio_ucid_input_tone
#define FEATURE_USE_CASE_BASE   audio_ucid_hd_audio
#define MIXER_USE_CASE_BASE     audio_ucid_mixer_core
#define OUTPUT_USE_CASE_BASE    audio_ucid_output_broadcast
#define AEC_USE_CASE_BASE       audio_ucid_hardware_aec

static audio_processor_config_t config;

/****************************************************************************/
static audio_ucid_bit_map_t convertUseCaseToBitmap(unsigned short input_use_case, audio_ucid_t base)
{
    audio_ucid_bit_map_t bit_map = (audio_ucid_bit_map_t)input_use_case << base;
    return bit_map;
}

/****************************************************************************/
static void convertPatternToMapping(audio_processor_use_case_mapping_t* mapping, audio_clock_pattern_t* pattern)
{
    mapping->clock_rate = pattern->clock_rate;
    mapping->bit_map = 0;
    mapping->bit_map |= convertUseCaseToBitmap(pattern->input_use_case, INPUT_USE_CASE_BASE);
    mapping->bit_map |= convertUseCaseToBitmap(pattern->voice_use_case, VOICE_USE_CASE_BASE);
    mapping->bit_map |= convertUseCaseToBitmap(pattern->prompt_use_case, PROMPT_USE_CASE_BASE);
    mapping->bit_map |= convertUseCaseToBitmap(pattern->feature_use_case, FEATURE_USE_CASE_BASE);
    mapping->bit_map |= convertUseCaseToBitmap(pattern->mixer_use_case, MIXER_USE_CASE_BASE);
    mapping->bit_map |= convertUseCaseToBitmap(pattern->output_use_case, OUTPUT_USE_CASE_BASE);
    mapping->bit_map |= convertUseCaseToBitmap(pattern->aec_use_case, AEC_USE_CASE_BASE);
}

/****************************************************************************/
static uint16 getNumClockMappingsFromConfigLength(uint16 len_words)
{
    size_t size_clock_array = len_words * sizeof(uint16);
    size_t size_clock_entry = sizeof(audio_clock_pattern_t);
    
    return (uint16)(size_clock_array / size_clock_entry);
}

/****************************************************************************/
static bool readDefaultClockRate(void)
{
    sink_audio_clock_config_def_t* clock_config = NULL;
    
    uint16 len_words = configManagerGetReadOnlyConfig(SINK_AUDIO_CLOCK_CONFIG_BLK_ID, (const void **)&clock_config);
    
    if(len_words)
    {
        config.default_rate = clock_config->default_rate;
        AC_DEBUG(("AC: Default Clock %u\n", config.default_rate));
        configManagerReleaseConfig(SINK_AUDIO_CLOCK_CONFIG_BLK_ID); 
        return TRUE;
    }
    
    return FALSE;
}

/****************************************************************************/
static bool readClockConfigTable(void)
{
    sink_audio_clock_table_config_def_t *clock_table = NULL;
    uint16 len_words = configManagerGetReadOnlyConfig(SINK_AUDIO_CLOCK_TABLE_CONFIG_BLK_ID, (const void **)&clock_table);

    if (len_words)
    {
        unsigned i;
        
        config.num_mappings = getNumClockMappingsFromConfigLength(len_words);
        config.mappings = PanicUnlessMalloc(config.num_mappings * sizeof(config.mappings[0]));
        
        for(i = 0; i < config.num_mappings; i++)
        {
            convertPatternToMapping(&config.mappings[i], &clock_table->audio_clock_array[i]);
            AC_DEBUG(("AC: Mapping %u %016llx - %u\n",i, config.mappings[i].bit_map, config.mappings[i].clock_rate));
        }
        
        configManagerReleaseConfig(SINK_AUDIO_CLOCK_TABLE_CONFIG_BLK_ID); 
        return TRUE;
    }
    
    return FALSE;
}

/****************************************************************************/
void sinkAudioClockInit(void)
{
    bool have_config_table = readClockConfigTable();
    bool have_default_rate = readDefaultClockRate();
    
    if(have_config_table || have_default_rate)
        AudioProcessorConfigure(&config);
}

#endif
