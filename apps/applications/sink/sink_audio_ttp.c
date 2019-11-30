/*
Copyright (c) 2017 - 2019 Qualcomm Technologies International, Ltd.
*/
/**
\file
\ingroup sink_app
\brief   Set the time-to-play parameters for A2DP and TWS.
*/

#ifdef ENABLE_AUDIO_TTP

#include <stdio.h>

#include <audio_config.h>
#include <config_store.h>

#include "sink_configmanager.h"
#include "sink_audio_ttp.h"
#include "sink_audio_ttp_config_def.h"

/**************************************************************************/
static void sinkAudioTtpSetTtpLatency(void)
{
    audio_ttp_config_def_t *audio_ttp_config_data = NULL;

    if(configManagerGetReadOnlyConfig(AUDIO_TTP_CONFIG_BLK_ID, (const void **)&audio_ttp_config_data))
    {
        AudioConfigSetA2DPTtpLatency(audio_ttp_config_data->ttp_latency_parameters.a2dp_latency.minimum_ttp_latency_ms,
                                     audio_ttp_config_data->ttp_latency_parameters.a2dp_latency.target_ttp_latency_ms,
                                     audio_ttp_config_data->ttp_latency_parameters.a2dp_latency.maximum_ttp_latency_ms);

        AudioConfigSetTWSTtpLatency(audio_ttp_config_data->ttp_latency_parameters.tws_latency.minimum_ttp_latency_ms,
                                    audio_ttp_config_data->ttp_latency_parameters.tws_latency.target_ttp_latency_ms,
                                    audio_ttp_config_data->ttp_latency_parameters.tws_latency.maximum_ttp_latency_ms);

        AudioConfigSetWiredTtpLatency(audio_ttp_config_data->ttp_latency_parameters.wired_latency.minimum_ttp_latency_ms,
                                    audio_ttp_config_data->ttp_latency_parameters.wired_latency.target_ttp_latency_ms,
                                    audio_ttp_config_data->ttp_latency_parameters.wired_latency.maximum_ttp_latency_ms);

        configManagerReleaseConfig(AUDIO_TTP_CONFIG_BLK_ID);
    }
}

/**************************************************************************/
void sinkAudioTtpLatencyParamsInit(void)
{
    sinkAudioTtpSetTtpLatency();
}

#endif
