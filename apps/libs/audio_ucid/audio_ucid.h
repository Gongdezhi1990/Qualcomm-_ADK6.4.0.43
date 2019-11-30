/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_ucid.h

DESCRIPTION
    Enumeration of audio use-cases.
*/


/*!
@file    audio_ucid.h
@brief   The audio_processor provides the capability to use the lowest audio DSP
         processing speed according to the use-case(s) being used.

         This file provides an enumeration of the audio use-cases.
*/


#ifndef AUDIO_UCID_
#define AUDIO_UCID_

#include <vmtypes.h>

/* Convert an audio_ucid_t to audio_ucid_bit_map_t */
#define USE_CASE_TO_BITMAP(ucid) ((audio_ucid_bit_map_t) 1 << ucid)

/*
 * An enumeration of the valid use-cases that have been identified.
 * Keep this enum starting with audio_ucid_not_defined at zero and with
 * continuous values, with audio_ucid_number_of_ucids after the last
 * user-case value.
 */
typedef enum
{
    audio_ucid_not_defined,                 /* Keep as the first item */
    audio_ucid_input_a2dp,
    audio_ucid_input_a2dp_hq_codec,
    audio_ucid_input_broadcast,
    audio_ucid_input_analogue,
    audio_ucid_input_digital,
    audio_ucid_input_tone,
    audio_ucid_input_voice_prompts_decoder,
    audio_ucid_input_voice_prompts_pcm,
    audio_ucid_hardware_aec,
    audio_ucid_mixer_core,
    audio_ucid_mixer_core_voice,
    audio_ucid_mixer_core_low_power,
    audio_ucid_mixer_voice_resampler,
    audio_ucid_mixer_music_resampler,
    audio_ucid_mixer_prompt_resampler,
    audio_ucid_mixer_processing_mono_voice,
    audio_ucid_mixer_processing_prompt,
    audio_ucid_mixer_processing_stereo_music,
    audio_ucid_mixer_speaker_mono,
    audio_ucid_mixer_speaker_stereo,
    audio_ucid_mixer_speaker_stereo_bass,
    audio_ucid_mixer_speaker_stereo_xover,
    audio_ucid_mixer_speaker_voice_only,
    audio_ucid_hfp,
    audio_ucid_hfp_cvc_headset,
    audio_ucid_hfp_cvc_speaker,
    audio_ucid_hfp_cvc_handsfree,
    audio_ucid_output_broadcast,
    audio_ucid_output_tws,
    audio_ucid_output_digital,
    audio_ucid_hd_audio,
    audio_ucid_aov,
    audio_ucid_anc_tuning_mode,
    audio_ucid_va,
    audio_ucid_hfp_swb_and_uwb,
    audio_ucid_number_of_ucids,             /* Keep as the penultimate item */
    FORCE_ENUM_TO_MIN_16BIT(audio_ucid_t)   /* Keep as the last item */
} audio_ucid_t;

/* 
 * Bitmap of audio use cases above. This must be large enough to hold (1 <<
 * (audio_ucid_number_of_ucids - 1) 
 */
typedef unsigned long long audio_ucid_bit_map_t;

#endif /* AUDIO_UCID_ */
