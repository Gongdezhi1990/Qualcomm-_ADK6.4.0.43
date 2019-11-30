/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_voice_hfp_tasks.c

DESCRIPTION
    Definitions of audio voice HFP tasks.
*/

#include <audio_plugin_voice_variants.h>
#include <audio_voice_common.h>
#include "audio_voice_hfp.h"


const CvcPluginTaskdata hfp_nb_cvc_1_mic_headset_plugin = { .data = { AudioVoiceHfpMessageHandler }, .cvc_plugin_variant = cvc_1_mic_headset_cvsd };
const CvcPluginTaskdata hfp_nb_cvc_2_mic_headset_plugin = { .data = { AudioVoiceHfpMessageHandler }, .cvc_plugin_variant = cvc_2_mic_headset_cvsd };
const CvcPluginTaskdata hfp_nb_cvc_2_mic_headset_binaural_plugin = { .data = { AudioVoiceHfpMessageHandler }, .cvc_plugin_variant = cvc_2_mic_headset_binaural_nb };
const CvcPluginTaskdata hfp_nb_cvc_1_mic_speaker_plugin = { .data = { AudioVoiceHfpMessageHandler }, .cvc_plugin_variant = cvc_1_mic_speaker_cvsd };
const CvcPluginTaskdata hfp_nb_cvc_2_mic_speaker_plugin = { .data = { AudioVoiceHfpMessageHandler }, .cvc_plugin_variant = cvc_2_mic_speaker_cvsd };

const CvcPluginTaskdata hfp_wb_cvc_1_mic_headset_plugin = { .data = { AudioVoiceHfpMessageHandler }, .cvc_plugin_variant = cvc_1_mic_headset_msbc };
const CvcPluginTaskdata hfp_wb_cvc_2_mic_headset_plugin = { .data = { AudioVoiceHfpMessageHandler }, .cvc_plugin_variant = cvc_2_mic_headset_msbc };
const CvcPluginTaskdata hfp_wb_cvc_2_mic_headset_binaural_plugin = { .data = { AudioVoiceHfpMessageHandler }, .cvc_plugin_variant = cvc_2_mic_headset_binaural_wb };
const CvcPluginTaskdata hfp_wb_cvc_1_mic_speaker_plugin = { .data = { AudioVoiceHfpMessageHandler }, .cvc_plugin_variant = cvc_1_mic_speaker_msbc };
const CvcPluginTaskdata hfp_wb_cvc_2_mic_speaker_plugin = { .data = { AudioVoiceHfpMessageHandler }, .cvc_plugin_variant = cvc_2_mic_speaker_msbc };

const CvcPluginTaskdata hfp_uwb_cvc_1_mic_headset_plugin = { .data = { AudioVoiceHfpMessageHandler }, .cvc_plugin_variant = cvc_1_mic_headset_uwb };
const CvcPluginTaskdata hfp_uwb_cvc_2_mic_headset_plugin = { .data = { AudioVoiceHfpMessageHandler }, .cvc_plugin_variant = cvc_2_mic_headset_uwb };
const CvcPluginTaskdata hfp_uwb_cvc_2_mic_headset_binaural_plugin = { .data = { AudioVoiceHfpMessageHandler }, .cvc_plugin_variant = cvc_2_mic_headset_binaural_uwb };
const CvcPluginTaskdata hfp_uwb_cvc_1_mic_handsfree_plugin = { .data = { AudioVoiceHfpMessageHandler }, .cvc_plugin_variant = cvc_1_mic_handsfree_uwb };
const CvcPluginTaskdata hfp_uwb_cvc_2_mic_handsfree_plugin = { .data = { AudioVoiceHfpMessageHandler }, .cvc_plugin_variant = cvc_2_mic_handsfree_uwb };
const CvcPluginTaskdata hfp_uwb_cvc_1_mic_speaker_plugin = { .data = { AudioVoiceHfpMessageHandler }, .cvc_plugin_variant = cvc_1_mic_speaker_uwb };
const CvcPluginTaskdata hfp_uwb_cvc_2_mic_speaker_plugin = { .data = { AudioVoiceHfpMessageHandler }, .cvc_plugin_variant = cvc_2_mic_speaker_uwb };

const CvcPluginTaskdata hfp_swb_cvc_1_mic_headset_plugin = { .data = { AudioVoiceHfpMessageHandler }, .cvc_plugin_variant = cvc_1_mic_headset_swb };
const CvcPluginTaskdata hfp_swb_cvc_2_mic_headset_plugin = { .data = { AudioVoiceHfpMessageHandler }, .cvc_plugin_variant = cvc_2_mic_headset_swb };
const CvcPluginTaskdata hfp_swb_cvc_2_mic_headset_binaural_plugin = { .data = { AudioVoiceHfpMessageHandler }, .cvc_plugin_variant = cvc_2_mic_headset_binaural_swb };
const CvcPluginTaskdata hfp_swb_cvc_1_mic_handsfree_plugin = { .data = { AudioVoiceHfpMessageHandler }, .cvc_plugin_variant = cvc_1_mic_handsfree_swb };
const CvcPluginTaskdata hfp_swb_cvc_2_mic_handsfree_plugin = { .data = { AudioVoiceHfpMessageHandler }, .cvc_plugin_variant = cvc_2_mic_handsfree_swb };
const CvcPluginTaskdata hfp_swb_cvc_1_mic_speaker_plugin = { .data = { AudioVoiceHfpMessageHandler }, .cvc_plugin_variant = cvc_1_mic_speaker_swb };
const CvcPluginTaskdata hfp_swb_cvc_2_mic_speaker_plugin = { .data = { AudioVoiceHfpMessageHandler }, .cvc_plugin_variant = cvc_2_mic_speaker_swb };

