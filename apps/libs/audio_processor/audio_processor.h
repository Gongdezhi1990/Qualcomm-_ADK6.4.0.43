/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_processor.h

DESCRIPTION
    Use lowest possible processing speed for audio chains and other audio
    use-cases.
*/


/*!
@file    audio_processor.h
@brief   The audio_processor provides the capability to use the lowest audio DSP
         processing speed according to the use-case(s) being used.

         This file provides documentation for the audio_processor API.
*/


#ifndef AUDIO_PROCESSOR_
#define AUDIO_PROCESSOR_

#include <vmtypes.h>
#include <audio_ucid.h>

/* An enumeration defining the set of clock rates supported. */
typedef enum
{
    audio_processor_clock_rate_not_set,
    audio_processor_clock_rate_slow,
    audio_processor_clock_rate_medium,
    audio_processor_clock_rate_high,
    FORCE_ENUM_TO_MIN_16BIT(audio_processor_clock_rate_t)
} audio_processor_clock_rate_t;

/* 
 * Map a combination of audio use cases to a clock rate setting. Only an exact
 * match of the bit_map against the active use case will result in the clock
 * rate being applied, otherwise the default clock rate will be used. 
 */
typedef struct
{
    audio_ucid_bit_map_t            bit_map;
    audio_processor_clock_rate_t    clock_rate;
} audio_processor_use_case_mapping_t;

/* Configuration structure to hold multiple use case mappings */
typedef struct
{
    unsigned num_mappings;
    audio_processor_use_case_mapping_t* mappings;
    audio_processor_clock_rate_t default_rate;
} audio_processor_config_t;

/*!
    @brief Configure use cases and associated clock rates

    @param config The configuration table
*/
void AudioProcessorConfigure(audio_processor_config_t* config);

/*!
    @brief Function to add a use-case to the current state

    @param usecase The use-case being added.
*/
void AudioProcessorAddUseCase(audio_ucid_t usecase);

/*!
    @brief Function to set the sample rate

    @param usecase The use-case being removed.
*/
void AudioProcessorRemoveUseCase(audio_ucid_t usecase);

/*!
    @brief Function to get whether a use-case is in use or not

    @param None.

    @return audio_processor_clock_rate_t value of the current_clock_rate variable
*/
bool AudioProcessorUseCaseIsInUse(audio_ucid_t use_case);

#ifdef HOSTED_TEST_ENVIRONMENT
/*!
    @brief Function to reset module static variables

    This is only intended for unit test and will panic if called in a release build.

    @param usecase The use-case being checked.

    @return bool TRUE if the use-case was recognised and is in use, otherwise FALSE
*/
void AudioProcessorTestReset(void);
#endif

#endif /* AUDIO_PROCESSOR_ */
