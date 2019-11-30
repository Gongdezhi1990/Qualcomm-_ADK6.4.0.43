/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_plugin_voice_variants.h

DESCRIPTION
    Definitions of voice plug-in variants.
*/

#ifndef AUDIO_PLUGIN_VOICE_VARIANTS_H_
#define AUDIO_PLUGIN_VOICE_VARIANTS_H_

#include <message.h>
#include <hfp.h>

/*******************************************************************************
* CVC plug-in types - Values for the selecting the plug-in variant in the
* CvcPluginTaskdata structure.
*/

/* hfp_wbs_codec_mask is defined in hfp.h */

typedef enum
{
    narrow_band,
    wide_band,
    ultra_wide_band,
    super_wide_band,
    max_number_plugin_types
} plugin_type_t;

typedef enum{
    plugin_no_dsp = 0,
        
    plugin_cvc_hs_1mic = 1,
    plugin_cvc_hs_1mic_bex = 2,
    plugin_cvc_hs_2mic = 3,
    plugin_cvc_hs_2mic_bex = 4,
    plugin_cvc_hs_2mic_binaural = 5,

    plugin_cvc_hf_1mic = 6,
    plugin_cvc_hf_1mic_bex = 7,
    plugin_cvc_hf_2mic = 8,
    plugin_cvc_hf_2mic_bex = 9,
    
    plugin_cvc_spkr_1mic = 10,
    plugin_cvc_spkr_2mic = 11,
    
    plugin_cvc_spkr_3mic_farfield = 12,
    
    max_num_voice_variants,
    plugin_cvc_1mic_example = max_num_voice_variants,
    plugin_cvc_2mic_example = 14,
    max_num_voice_variants_inc_examples
} plugin_variant_t;


typedef enum
{
    cvc_1_mic_headset_cvsd,
    cvc_1_mic_headset_cvsd_bex,
    cvc_1_mic_headset_msbc,

    cvc_2_mic_headset_cvsd,
    cvc_2_mic_headset_cvsd_bex,
    cvc_2_mic_headset_msbc,

    cvc_1_mic_handsfree_cvsd,
    cvc_1_mic_handsfree_cvsd_bex,
    cvc_1_mic_handsfree_msbc,

    cvc_2_mic_handsfree_cvsd,
    cvc_2_mic_handsfree_cvsd_bex,
    cvc_2_mic_handsfree_msbc,

    cvc_disabled,

    cvc_1_mic_headset_cvsd_asr,
    cvc_2_mic_headset_cvsd_asr,

    cvc_1_mic_handsfree_cvsd_asr,
    cvc_2_mic_handsfree_cvsd_asr,

    cvc_2_mic_headset_binaural_nb,
    cvc_2_mic_headset_binaural_wb,

    cvc_1_mic_speaker_cvsd,
    cvc_1_mic_speaker_msbc,

    cvc_2_mic_speaker_cvsd,
    cvc_2_mic_speaker_msbc,

    cvc_1_mic_headset_uwb,
    cvc_2_mic_headset_uwb,
    cvc_2_mic_headset_binaural_uwb,
    cvc_1_mic_handsfree_uwb,
    cvc_2_mic_handsfree_uwb,
    cvc_1_mic_speaker_uwb,
    cvc_2_mic_speaker_uwb,

    cvc_1_mic_headset_swb,
    cvc_2_mic_headset_swb,
    cvc_2_mic_headset_binaural_swb,
    cvc_1_mic_handsfree_swb,
    cvc_2_mic_handsfree_swb,
    cvc_1_mic_speaker_swb,
    cvc_2_mic_speaker_swb

} cvc_plugin_type_t;

typedef enum
{
    link_encoding_cvsd,
    link_encoding_msbc,
    link_encoding_usb_pcm
} link_encoding_t;

typedef enum
{
    one_mic,
    two_mic
} voice_mics_t;

typedef enum
{
    mic_rate_8khz,
    mic_rate_16khz,
    mic_rate_24khz,
    mic_rate_32khz
} mic_input_rate_t;

typedef struct
{
    TaskData          data;
    cvc_plugin_type_t cvc_plugin_variant:6;
    link_encoding_t   encoder:3;
    voice_mics_t      voice_mics:1;
    mic_input_rate_t  mic_input_rate:2;
    unsigned          reserved:4;
} CvcPluginTaskdata;

#define SWBS_SAMPLE_RATE        ((uint32)32000)
#define UWBS_SAMPLE_RATE        ((uint32)24000)
#define WBS_SAMPLE_RATE         ((uint32)16000)
#define NBS_SAMPLE_RATE         ((uint32)8000)

/****************************************************************************
DESCRIPTION
    Returns HFP plugin based on codec and index
*/
Task AudioPluginVoiceVariantsGetHfpPlugin(plugin_type_t type, plugin_variant_t variant);

extern const TaskData aov_plugin;

/****************************************************************************
DESCRIPTION
    Returns USB plugin based on codec and index
*/
Task AudioPluginVoiceVariantsGetUsbPlugin(plugin_type_t type, plugin_variant_t variant);

/****************************************************************************
DESCRIPTION
    Returns HFP ASR plugin based on codec and index
*/
Task AudioPluginVoiceVariantsGetAsrPlugin(plugin_type_t type, plugin_variant_t variant);


#endif /* AUDIO_PLUGIN_VOICE_VARIANTS_H_ */
