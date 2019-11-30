/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

 
FILE NAME
    audio_anc_tuning.h
 
DESCRIPTION
    Audio plugin for the ANC tuning mode.
*/

#ifndef AUDIO_ANC_TUNING_H_
#define AUDIO_ANC_TUNING_H_

#include <message.h>
#include <audio_plugin_music_variants.h>

/*!
    @brief Get the anc tuning mode plugin data.

    @return Pointer to the anc tuning mode plugin data
*/
const A2dpPluginTaskdata * AncGetTuningModePlugin(void);

#endif
