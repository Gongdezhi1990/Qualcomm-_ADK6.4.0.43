/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_plugin_voice_variants.c

DESCRIPTION
    Definitions of voice plug-in variants.
*/

#include <stdlib.h>
#include <stdio.h>
#include <print.h>

#include "audio_plugin_voice_variants.h"

extern CvcPluginTaskdata hfp_nb_cvc_1_mic_headset_plugin;
extern CvcPluginTaskdata hfp_nb_cvc_2_mic_headset_plugin;
extern CvcPluginTaskdata hfp_nb_cvc_2_mic_headset_binaural_plugin;
extern CvcPluginTaskdata hfp_nb_cvc_1_mic_speaker_plugin;
extern CvcPluginTaskdata hfp_nb_cvc_2_mic_speaker_plugin;

extern CvcPluginTaskdata hfp_wb_cvc_1_mic_headset_plugin;
extern CvcPluginTaskdata hfp_wb_cvc_2_mic_headset_plugin;
extern CvcPluginTaskdata hfp_wb_cvc_2_mic_headset_binaural_plugin;
extern CvcPluginTaskdata hfp_wb_cvc_1_mic_speaker_plugin;
extern CvcPluginTaskdata hfp_wb_cvc_2_mic_speaker_plugin;

extern CvcPluginTaskdata hfp_uwb_cvc_1_mic_headset_plugin;
extern CvcPluginTaskdata hfp_uwb_cvc_2_mic_headset_plugin;
extern CvcPluginTaskdata hfp_uwb_cvc_2_mic_headset_binaural_plugin;
extern CvcPluginTaskdata hfp_uwb_cvc_1_mic_handsfree_plugin;
extern CvcPluginTaskdata hfp_uwb_cvc_2_mic_handsfree_plugin;
extern CvcPluginTaskdata hfp_uwb_cvc_1_mic_speaker_plugin;
extern CvcPluginTaskdata hfp_uwb_cvc_2_mic_speaker_plugin;

extern CvcPluginTaskdata hfp_swb_cvc_1_mic_headset_plugin;
extern CvcPluginTaskdata hfp_swb_cvc_2_mic_headset_plugin;
extern CvcPluginTaskdata hfp_swb_cvc_2_mic_headset_binaural_plugin;
extern CvcPluginTaskdata hfp_swb_cvc_1_mic_handsfree_plugin;
extern CvcPluginTaskdata hfp_swb_cvc_2_mic_handsfree_plugin;
extern CvcPluginTaskdata hfp_swb_cvc_1_mic_speaker_plugin;
extern CvcPluginTaskdata hfp_swb_cvc_2_mic_speaker_plugin;


extern CvcPluginTaskdata usb_nb_cvc_1_mic_headset_plugin;
extern CvcPluginTaskdata usb_nb_cvc_2_mic_headset_plugin;
extern CvcPluginTaskdata usb_nb_cvc_2_mic_headset_binaural_plugin;
extern CvcPluginTaskdata usb_nb_cvc_1_mic_speaker_plugin;
extern CvcPluginTaskdata usb_nb_cvc_2_mic_speaker_plugin;

extern CvcPluginTaskdata usb_wb_cvc_1_mic_headset_plugin;
extern CvcPluginTaskdata usb_wb_cvc_2_mic_headset_plugin;
extern CvcPluginTaskdata usb_wb_cvc_2_mic_headset_binaural_plugin;
extern CvcPluginTaskdata usb_wb_cvc_1_mic_speaker_plugin;
extern CvcPluginTaskdata usb_wb_cvc_2_mic_speaker_plugin;

extern CvcPluginTaskdata usb_uwb_cvc_1_mic_headset_plugin;
extern CvcPluginTaskdata usb_uwb_cvc_2_mic_headset_plugin;
extern CvcPluginTaskdata usb_uwb_cvc_2_mic_headset_binaural_plugin;
extern CvcPluginTaskdata usb_uwb_cvc_1_mic_handsfree_plugin;
extern CvcPluginTaskdata usb_uwb_cvc_2_mic_handsfree_plugin;
extern CvcPluginTaskdata usb_uwb_cvc_1_mic_speaker_plugin;
extern CvcPluginTaskdata usb_uwb_cvc_2_mic_speaker_plugin;

extern CvcPluginTaskdata usb_swb_cvc_1_mic_headset_plugin;
extern CvcPluginTaskdata usb_swb_cvc_2_mic_headset_plugin;
extern CvcPluginTaskdata usb_swb_cvc_2_mic_headset_binaural_plugin;
extern CvcPluginTaskdata usb_swb_cvc_1_mic_handsfree_plugin;
extern CvcPluginTaskdata usb_swb_cvc_2_mic_handsfree_plugin;
extern CvcPluginTaskdata usb_swb_cvc_1_mic_speaker_plugin;
extern CvcPluginTaskdata usb_swb_cvc_2_mic_speaker_plugin;

#define MAX_NUM_VOICE_PLUGIN_VARIANTS (max_num_voice_variants)

const Task hfp_cvc_plugins[][MAX_NUM_VOICE_PLUGIN_VARIANTS] =
{
    [narrow_band][plugin_cvc_hs_1mic] = (Task)&hfp_nb_cvc_1_mic_headset_plugin,
    [narrow_band][plugin_cvc_hs_2mic] = (Task)&hfp_nb_cvc_2_mic_headset_plugin,
    [narrow_band][plugin_cvc_hs_2mic_binaural] = (Task)&hfp_nb_cvc_2_mic_headset_binaural_plugin,
    [narrow_band][plugin_cvc_spkr_1mic] = (Task)&hfp_nb_cvc_1_mic_speaker_plugin,
    [narrow_band][plugin_cvc_spkr_2mic] = (Task)&hfp_nb_cvc_2_mic_speaker_plugin,

    [wide_band][plugin_cvc_hs_1mic] = (Task)&hfp_wb_cvc_1_mic_headset_plugin,
    [wide_band][plugin_cvc_hs_2mic] = (Task)&hfp_wb_cvc_2_mic_headset_plugin,
    [wide_band][plugin_cvc_hs_2mic_binaural] = (Task)&hfp_wb_cvc_2_mic_headset_binaural_plugin,
    [wide_band][plugin_cvc_spkr_1mic] = (Task)&hfp_wb_cvc_1_mic_speaker_plugin,
    [wide_band][plugin_cvc_spkr_2mic] = (Task)&hfp_wb_cvc_2_mic_speaker_plugin,

    [ultra_wide_band][plugin_cvc_hs_1mic] = (Task)&hfp_uwb_cvc_1_mic_headset_plugin,
    [ultra_wide_band][plugin_cvc_hs_2mic] = (Task)&hfp_uwb_cvc_2_mic_headset_plugin,
    [ultra_wide_band][plugin_cvc_hs_2mic_binaural] = (Task)&hfp_uwb_cvc_2_mic_headset_binaural_plugin,
    [ultra_wide_band][plugin_cvc_hf_1mic] = (Task)&hfp_uwb_cvc_1_mic_handsfree_plugin,
    [ultra_wide_band][plugin_cvc_hf_2mic] = (Task)&hfp_uwb_cvc_2_mic_handsfree_plugin,
    [ultra_wide_band][plugin_cvc_spkr_1mic] = (Task)&hfp_uwb_cvc_1_mic_speaker_plugin,
    [ultra_wide_band][plugin_cvc_spkr_2mic] = (Task)&hfp_uwb_cvc_2_mic_speaker_plugin,

    [super_wide_band][plugin_cvc_hs_1mic] = (Task)&hfp_swb_cvc_1_mic_headset_plugin,
    [super_wide_band][plugin_cvc_hs_2mic] = (Task)&hfp_swb_cvc_2_mic_headset_plugin,
    [super_wide_band][plugin_cvc_hs_2mic_binaural] = (Task)&hfp_swb_cvc_2_mic_headset_binaural_plugin,
    [super_wide_band][plugin_cvc_hf_1mic] = (Task)&hfp_swb_cvc_1_mic_handsfree_plugin,
    [super_wide_band][plugin_cvc_hf_2mic] = (Task)&hfp_swb_cvc_2_mic_handsfree_plugin,
    [super_wide_band][plugin_cvc_spkr_1mic] = (Task)&hfp_swb_cvc_1_mic_speaker_plugin,
    [super_wide_band][plugin_cvc_spkr_2mic] = (Task)&hfp_swb_cvc_2_mic_speaker_plugin
};

const Task usb_cvc_plugins[][MAX_NUM_VOICE_PLUGIN_VARIANTS] =
{ 
    [narrow_band][plugin_cvc_hs_1mic] = (Task)&usb_nb_cvc_1_mic_headset_plugin,
    [narrow_band][plugin_cvc_hs_2mic] = (Task)&usb_nb_cvc_2_mic_headset_plugin,
    [narrow_band][plugin_cvc_hs_2mic_binaural] = (Task)&usb_nb_cvc_2_mic_headset_binaural_plugin,
    [narrow_band][plugin_cvc_spkr_1mic] = (Task)&usb_nb_cvc_1_mic_speaker_plugin,
    [narrow_band][plugin_cvc_spkr_2mic] = (Task)&usb_nb_cvc_2_mic_speaker_plugin,

    [wide_band][plugin_cvc_hs_1mic] = (Task)&usb_wb_cvc_1_mic_headset_plugin,
    [wide_band][plugin_cvc_hs_2mic] = (Task)&usb_wb_cvc_2_mic_headset_plugin,
    [wide_band][plugin_cvc_hs_2mic_binaural] = (Task)&usb_wb_cvc_2_mic_headset_binaural_plugin,
    [wide_band][plugin_cvc_spkr_1mic] = (Task)&usb_wb_cvc_1_mic_speaker_plugin,
    [wide_band][plugin_cvc_spkr_2mic] = (Task)&usb_wb_cvc_2_mic_speaker_plugin,

    [ultra_wide_band][plugin_cvc_hs_1mic] = (Task)&usb_uwb_cvc_1_mic_headset_plugin,
    [ultra_wide_band][plugin_cvc_hs_2mic] = (Task)&usb_uwb_cvc_2_mic_headset_plugin,
    [ultra_wide_band][plugin_cvc_hs_2mic_binaural] = (Task)&usb_uwb_cvc_2_mic_headset_binaural_plugin,
    [ultra_wide_band][plugin_cvc_hf_1mic] = (Task)&usb_uwb_cvc_1_mic_handsfree_plugin,
    [ultra_wide_band][plugin_cvc_hf_2mic] = (Task)&usb_uwb_cvc_2_mic_handsfree_plugin,
    [ultra_wide_band][plugin_cvc_spkr_1mic] = (Task)&usb_uwb_cvc_1_mic_speaker_plugin,
    [ultra_wide_band][plugin_cvc_spkr_2mic] = (Task)&usb_uwb_cvc_2_mic_speaker_plugin,

    [super_wide_band][plugin_cvc_hs_1mic] = (Task)&usb_swb_cvc_1_mic_headset_plugin,
    [super_wide_band][plugin_cvc_hs_2mic] = (Task)&usb_swb_cvc_2_mic_headset_plugin,
    [super_wide_band][plugin_cvc_hs_2mic_binaural] = (Task)&usb_swb_cvc_2_mic_headset_binaural_plugin,
    [super_wide_band][plugin_cvc_hf_1mic] = (Task)&usb_swb_cvc_1_mic_handsfree_plugin,
    [super_wide_band][plugin_cvc_hf_2mic] = (Task)&usb_swb_cvc_2_mic_handsfree_plugin,
    [super_wide_band][plugin_cvc_spkr_1mic] = (Task)&usb_swb_cvc_1_mic_speaker_plugin,
    [super_wide_band][plugin_cvc_spkr_2mic] = (Task)&usb_swb_cvc_2_mic_speaker_plugin
};

static Task getPlugin(const Task * plugins, plugin_type_t type, plugin_variant_t variant)
{
    if(type < max_number_plugin_types)
    {
        if(variant < MAX_NUM_VOICE_PLUGIN_VARIANTS)
        {
            return *(plugins + (type * MAX_NUM_VOICE_PLUGIN_VARIANTS) + variant);
        }
    }
    return NULL;
}

Task AudioPluginVoiceVariantsGetHfpPlugin(plugin_type_t type, plugin_variant_t variant)
{
    return getPlugin(&hfp_cvc_plugins[0][0], type, variant);
}

Task AudioPluginVoiceVariantsGetUsbPlugin(plugin_type_t type, plugin_variant_t variant)
{
    return getPlugin(&usb_cvc_plugins[0][0], type, variant);
}

Task AudioPluginVoiceVariantsGetAsrPlugin(plugin_type_t type, plugin_variant_t variant)
{
    UNUSED(type);
    UNUSED(variant);
    return NULL;
}
