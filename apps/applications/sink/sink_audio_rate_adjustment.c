/*
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
*/
/**
\file
\ingroup sink_app
\brief   Set the time-to-play parameters for A2DP and TWS.
*/

#ifdef ENABLE_STANDALONE_RATE_ADJUSTMENT

#include <stdio.h>

#include <audio_config.h>
#include <config_store.h>

#include "sink_configmanager.h"
#include "sink_audio_rate_adjustment.h"
#include "sink_audio_ra_config_def.h"

/**************************************************************************/
void sinkAudioRateAdjustmentInit(void)
{
    audio_ra_readonly_config_def_t *audio_ra_config_data = NULL;

    if(configManagerGetReadOnlyConfig(AUDIO_RA_READONLY_CONFIG_BLK_ID, (const void **)&audio_ra_config_data))
    {
        AudioConfigSetStandaloneRateAdjustment(audio_ra_config_data->enable_standalone_rate_adjustment);
        configManagerReleaseConfig(AUDIO_RA_READONLY_CONFIG_BLK_ID);
    }
}
#endif
