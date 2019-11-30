/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_hardware.c

DESCRIPTION
    This plugin deals with connecting the audio chain to audio hardware
    Audio mixer would build everything up to the point before AEC reference and then call in to audio_hardware to connect to the output.
    Accoustic Echo Cancellation (AEC) is an optional feature in the audio_hardware - required only in cases of HFP and concurrent audio support.
    */

#include <vmtypes.h>
#include "audio_hardware.h"
#include "audio_ports.h"
#include "audio_hardware_aec.h"
#include "audio_output.h"
#include "audio_i2s_common.h"
#include "audio_hardware_output.h"


static bool hardware_input_connected = FALSE;
static bool hardware_output_connected = FALSE;

/****************************************************************************/
bool AudioHardwareConnectInput(const uint32 mic_sample_rate)
{
    if (!hardware_input_connected)
    {
        hardwareAecCreateAndConnectInput(mic_sample_rate);
        hardware_input_connected = TRUE;
    }
    return hardware_input_connected;
}

/****************************************************************************/
bool AudioHardwareConnectOutput(const audio_hardware_connect_t* hw_connect_data)
{  
    if (!hardware_output_connected)
    {
        /*Create AEC chain if applicable*/
        if (hardwareIsAecRequired(hw_connect_data->connection_type))
        {
            hardwareAecCreateAndConnectOutput(hw_connect_data);
        }        
        else
        {
            hardwareConnectDirectToOutput(hw_connect_data);
        }

        hardware_output_connected = TRUE;
    }
    return hardware_output_connected;
}




/****************************************************************************/
bool AudioHardwareDisconnectOutput(void)
{
    if (hardware_output_connected)
    {
        hardwareAecDisconnectOutput();
        AudioOutputDisconnect();

        hardware_output_connected = FALSE;

        return TRUE;
    }
    return FALSE;
}

/****************************************************************************/
bool AudioHardwareDisconnectInput(void)
{
    if(hardware_input_connected)
    {
        hardwareAecDisconnectInput();

        hardware_input_connected = FALSE;

        return TRUE;
    }
    return FALSE;
}


/****************************************************************************/
audio_hardware_speaker_config_t AudioHardwareGetSpeakerConfig(void)
{
    audio_hardware_speaker_config_t speaker_config = speaker_stereo;

    bool lp = AudioOutputIsOutputMapped(audio_output_primary_left);
    bool rp = AudioOutputIsOutputMapped(audio_output_primary_right);
    bool ls = AudioOutputIsOutputMapped(audio_output_secondary_left);
    bool rs = AudioOutputIsOutputMapped(audio_output_secondary_right);
    bool sw = AudioOutputIsOutputMapped(audio_output_wired_sub);
    
    if (lp && rp && ls && rs)
    {
        speaker_config = speaker_stereo_xover;
    }
    else if (lp && rp && sw)
    {
        speaker_config = speaker_stereo_bass;
    }
    else if (lp && rp && !ls && !rs)
    {
        speaker_config = speaker_stereo;
    }
    else if (lp && !rp && !ls && !rs)
    {
        speaker_config = speaker_mono;
    }
    else
    {
        Panic();
    }

    return speaker_config;
}

void AudioHardwareConfigureMicSampleRate(uint32 mic_sample_rate)
{
    hardwareAecConfigureMicSampleRate(mic_sample_rate);
}

/****************************************************************************/
void AudioHardwareMuteMicOutput(bool enable)
{
    hardwareAecMuteMicOutput(enable);
}

/****************************************************************************/
Operator AudioHardwareGetOperator(void)
{
    return hardwareAecGetOperator();
}

/****************************************************************************/
void AudioHardwareSetMicUse(audio_hw_mic_use_t mic_use)
{
    hardwareAecSetMicUse(mic_use);
}
