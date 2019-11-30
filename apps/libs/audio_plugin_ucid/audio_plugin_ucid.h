/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_plugin_ucid.h

DESCRIPTION
    This library provides UCIDs for all the operators in the audio plugin system

*/

#ifndef _AUDIO_PLUGIN_UCID_H_
#define _AUDIO_PLUGIN_UCID_H_


typedef enum _ucid_peq
{
    ucid_peq_resampler_0,
    ucid_peq_resampler_1,
    ucid_peq_resampler_2,
    ucid_peq_resampler_3,
    ucid_peq_resampler_4,
    ucid_peq_resampler_5,
    ucid_peq_resampler_6,
    ucid_peq_speaker,
    ucid_peq_3band_mbc_processing,
} ucid_peq_t;

typedef enum _ucid_compander
{
    ucid_compander_processing,
    ucid_compander_speaker,
    ucid_compander_low_freq_2band_mbc_processing,
    ucid_compander_high_freq_2band_mbc_processing,
    ucid_compander_low_freq_3band_mbc_processing,
    ucid_compander_mid_freq_3band_mbc_processing,
    ucid_compander_high_freq_3band_mbc_processing,
} ucid_compander_t;

typedef enum _ucid_passthrough
{
    ucid_passthrough_processing,
    ucid_passthrough_speaker,
} ucid_passthrough_t;

typedef enum _ucid_cvc_send
{
    ucid_cvc_send,
    ucid_cvc_send_va,
    ucid_cvc_send_va_farfield,
    ucid_cvc_send_va_farfield_omni_mode,
} ucid_cvc_send_t;

typedef enum _ucid_cvc_recv
{
    ucid_cvc_recv,
} ucid_cvc_recv_t;

typedef enum _ucid_volume
{
    ucid_volume_core_media,
    ucid_volume_speaker_master,
} ucid_volume_t;

typedef enum _ucid_aec
{
    ucid_aec_nb,
    ucid_aec_wb,
    ucid_aec_wb_va,
    ucid_aec_uwb,
    ucid_aec_swb
} ucid_aec_t;

typedef enum _ucid_crossover
{
    ucid_crossover_speaker_stereo_xover,
    ucid_crossover_speaker_stereo_bass,
    ucid_crossover_2band_mbc_processing,
    ucid_crossover_first_3band_mbc_processing,
    ucid_crossover_second_3band_mbc_processing,
} ucid_crossover_t;

typedef enum _ucid_source_sync
{
    ucid_source_sync_core,
} ucid_source_sync_t;

typedef enum _ucid_dbe
{
    ucid_dbe_processing,
} ucid_dbe_t;

typedef enum _ucid_vse
{
    ucid_vse_processing,
} ucid_vse_t;

typedef enum _ucid_qva
{
    ucid_qva,
} ucid_qva_t;

#endif /* _AUDIO_PLUGIN_UCID_H_ */
