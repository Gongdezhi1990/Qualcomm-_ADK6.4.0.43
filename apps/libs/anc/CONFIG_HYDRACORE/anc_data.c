/*******************************************************************************
Copyright (c) 2015-2017 Qualcomm Technologies International, Ltd.


FILE NAME
    anc_data.c

DESCRIPTION
    Encapsulation of the ANC VM Library data.
*/

#include "CONFIG_HYDRACORE/anc_config_data.h"
#include "anc_data.h"
#include "anc_debug.h"
#include "anc_config_read.h"

#include <stdlib.h>
#include <string.h>
#include <audio_config.h>

typedef struct
{
    anc_state state;
    anc_mode_t mode;
    anc_mic_params_t mic_params;
    anc_config_t config_data;
    anc_fine_tune_gain_step_t fine_tune_gain_step;
} anc_lib_data_t;


static anc_lib_data_t anc_lib_data;

/******************************************************************************/
bool ancDataInitialise(void)
{
    memset(&anc_lib_data, 0, sizeof(anc_lib_data_t));
    return TRUE;
}

/******************************************************************************/
bool ancDataDeinitialise(void)
{
    memset(&anc_lib_data, 0, sizeof(anc_lib_data_t));
    return TRUE;
}

/******************************************************************************/
void ancDataSetState(anc_state state)
{
    anc_lib_data.state = state;

    ancConfigDataUpdateOnStateChange();
}

/******************************************************************************/
anc_state ancDataGetState(void)
{
    return anc_lib_data.state;
}

/******************************************************************************/
void ancDataSetMicParams(anc_mic_params_t *mic_params)
{
    anc_lib_data.mic_params = *mic_params;
}

/******************************************************************************/
anc_mic_params_t* ancDataGetMicParams(void)
{
    return &anc_lib_data.mic_params;
}

/******************************************************************************/
bool ancDataSetMode(anc_mode_t mode)
{
    bool value = FALSE;
    
    if (anc_lib_data.mode  != mode)
    {
        anc_lib_data.mode = mode;
        value  = ancConfigDataUpdateOnModeChange(mode);
    }
    return value;
}

/******************************************************************************/
anc_mode_t ancDataGetMode(void)
{
    return anc_lib_data.mode;
}

anc_config_t * ancDataGetConfigData(void)
{
    return &anc_lib_data.config_data;
}

/******************************************************************************/
void ancDataSetFineTuneGain(anc_fine_tune_gain_step_t step)
{
    anc_lib_data.fine_tune_gain_step = step;
}

anc_fine_tune_gain_step_t ancDataGetFineTuneGain(void)
{
    return anc_lib_data.fine_tune_gain_step;
}

/******************************************************************************/
anc_mode_config_t * ancDataGetCurrentModeConfig(void)
{
   return &ancDataGetConfigData()->mode;
}

/******************************************************************************/
bool ancDataRetrieveAndPopulateTuningData(anc_mode_t set_mode)
{
    return ancConfigReadPopulateAncData(ancDataGetConfigData(), set_mode);
}

/******************************************************************************/
bool ancDataIsLeftChannelConfigurable(void)
{
    return (ancDataGetMicParams()->enabled_mics & (feed_forward_left | feed_back_left));
}

bool ancDataIsRightChannelConfigurable(void)
{
    return (ancDataGetMicParams()->enabled_mics & (feed_forward_right | feed_back_right));
}

audio_mic_params * ancConfigDataGetMicForMicPath(anc_path_enable mic_path)
{
    audio_mic_params * mic_params = &ancDataGetMicParams()->feed_forward_left;
    switch(mic_path)
    {
        case feed_forward_left:
            mic_params = &ancDataGetMicParams()->feed_forward_left;
            break;
        case feed_forward_right:
            mic_params = &ancDataGetMicParams()->feed_forward_right;
            break;
        case feed_back_left:
            mic_params = &ancDataGetMicParams()->feed_back_left;
            break;
        case feed_back_right:
            mic_params = &ancDataGetMicParams()->feed_back_right;
            break;
        default:
            Panic();
            break;
    }
    return mic_params;
}

uint32 ancConfigDataGetHardwareGainForMicPath(anc_path_enable mic_path)
{
    uint32 gain = 0;
    switch(mic_path)
    {
        case feed_forward_left:
            gain = ancDataGetConfigData()->hardware_gains.feed_forward_mic_left;
            break;
        case feed_forward_right:
            gain = ancDataGetConfigData()->hardware_gains.feed_forward_mic_right;
            break;
        case feed_back_left:
            gain = ancDataGetConfigData()->hardware_gains.feed_back_mic_left;
            break;
        case feed_back_right:
            gain = ancDataGetConfigData()->hardware_gains.feed_back_mic_right;
            break;
        default:
            Panic();
            break;
    }
    return gain;
}

