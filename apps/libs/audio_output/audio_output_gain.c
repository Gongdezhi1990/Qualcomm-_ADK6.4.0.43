/******************************************************************************
Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd.

 
FILE NAME
    audio_output_gain.c
 
DESCRIPTION
    Contains functions related to setting/getting of audio output gains.
*/

#include "audio_output.h"
#include "audio_output_private.h"

#include "audio_i2s_common.h"
#include <print.h>
#include <stdlib.h>
#include <gain_utils.h>
#include <sink.h>
#include <panic.h>
#include <audio_config.h>

static uint16 system_gain_from_master(int16 master)
{
    /* Convert dB master volume to DAC based system volume */
    int16 system = (CODEC_STEPS + (master / DB_TO_DAC));
    
    /* DAC gain only goes down to -45dB, DSP volume control goes to -60dB */
    if(system < 0)
        system = 0;
    
    return (uint16)system;
}

static i2s_out_t get_i2s_chan(audio_output_t audio_out)
{
    audio_output_hardware_instance_t inst = config->mapping[audio_out].endpoint.instance;
    audio_output_channel_t chan = config->mapping[audio_out].endpoint.channel;
    
    if(inst == audio_output_hardware_instance_0)
    {
        if(chan == audio_output_channel_a)
            return i2s_out_1_left;
        if(chan == audio_output_channel_b)
            return i2s_out_1_right;
    }
    else if(inst == audio_output_hardware_instance_1)
    {
        if(chan == audio_output_channel_a)
            return i2s_out_2_left;
        if(chan == audio_output_channel_b)
            return i2s_out_2_right;
    }
    
    /* Catch invalid configuration with panic */
    Panic();
    return i2s_out_1_left;
}

static int16 get_trim(audio_output_t audio_out, bool skip_i2s_trim)
{
    audio_output_hardware_type_t type = config->mapping[audio_out].endpoint.type;
    
    if(skip_i2s_trim && type == audio_output_type_i2s)
        return 0;
    
    return config->mapping[audio_out].volume_trim;
}

static void get_all_trims(const audio_output_group_t group, bool skip_i2s_trim,
                          audio_output_gain_t* gain_info)
{
    if(group == audio_output_group_main)
    {
        gain_info->trim.main.primary_left    = get_trim(audio_output_primary_left, skip_i2s_trim);
        gain_info->trim.main.primary_right   = get_trim(audio_output_primary_right, skip_i2s_trim);
        gain_info->trim.main.secondary_left  = get_trim(audio_output_secondary_left, skip_i2s_trim);
        gain_info->trim.main.secondary_right = get_trim(audio_output_secondary_right, skip_i2s_trim);
        gain_info->trim.main.wired_sub       = get_trim(audio_output_wired_sub, skip_i2s_trim);
    }
    else if(group == audio_output_group_aux)
    {
        gain_info->trim.aux.aux_left  = get_trim(audio_output_aux_left, skip_i2s_trim);
        gain_info->trim.aux.aux_right = get_trim(audio_output_aux_right, skip_i2s_trim);
    }
}

static void overrideDigitalVolumeWithMute(const audio_output_group_t group,
                                                        audio_output_gain_t * const gain_info)
{
    gain_info->common.master = DIGITAL_VOLUME_MUTE;

    if(group == audio_output_group_main)
    {
        gain_info->trim.main.primary_left = 0;
        gain_info->trim.main.primary_right = 0;
        gain_info->trim.main.secondary_left = 0;
        gain_info->trim.main.secondary_right = 0;
        gain_info->trim.main.wired_sub = 0;
    }
    else
    {
        gain_info->trim.aux.aux_left = 0;
        gain_info->trim.aux.aux_right = 0;
    }
}

void AudioOutputGainGetDigital(const audio_output_group_t group, 
                                const int16 master_gain, const int16 tone_gain,
                                audio_output_gain_t* gain_info)
{
    hybrid_gains_t hybrid;
    
    PRINT(("AudioOutputGainGetDigital  "));
    PRINT(("%s: ", (group == audio_output_group_main ? "Main" : " Aux")));
    PRINT(("master gain %d tone %d ", master_gain, tone_gain));
    PRINT(("%s", (gain_info == NULL ? "NULL info" : "")));
    
    if(config == NULL || group >= audio_output_group_all || gain_info == NULL)
    {
        Panic();
        return;
    }
    
    gain_info->common.tone = tone_gain;
    
    switch(config->gain_type[group])
    {
        case audio_output_gain_hardware:
            /* Tell the DSP the current hardware gain */
            gain_info->common.system = system_gain_from_master(master_gain);
            /* Set the digital gain to 0dB */
            gain_info->common.master = MAXIMUM_DIGITAL_VOLUME_0DB;
            /* Get all trims, excluding I2S (as I2S trims are handled in hw) */
            get_all_trims(group, TRUE, gain_info);
        break;
        
        case audio_output_gain_digital:
            /* Ensure DSP is told the correct fixed hardware gain */
            gain_info->common.system = system_gain_from_master(config->fixed_hw_gain);
            /* Apply master gain in DSP */
            gain_info->common.master = master_gain;
            /* Get all trims */
            get_all_trims(group, FALSE, gain_info);
        break;
        
        case audio_output_gain_hybrid:
            /* Use existing codec function to obtain hybrid values */
            CodecCalcHybridValues(&hybrid, master_gain);
            /* Ensure DSP is told the correct hardware gain */
            gain_info->common.system = hybrid.dac_gain;
            /* Apply master gain in DSP */
            gain_info->common.master = hybrid.dsp_db_scaled;
            /* Get all trims */
            get_all_trims(group, FALSE, gain_info);
        break;
        
        default:
            Panic();
            return;
    }
    
    if(master_gain == DIGITAL_VOLUME_MUTE)
    {
        overrideDigitalVolumeWithMute(group, gain_info);
    }

    PRINT(("applied gain %d\n", gain_info->common.master));
}

void AudioOutputGainGetDigitalOnly(const audio_output_group_t group, 
                                    const int16 master_gain, const int16 tone_gain,
                                    audio_output_gain_t* gain_info)
{
    PRINT(("AudioOutputGainGetDigitalOnly  "));
    PRINT(("%s: ", (group == audio_output_group_main ? "Main" : " Aux")));
    PRINT(("master gain %d tone %d ", master_gain, tone_gain));
    PRINT(("%s\n", (gain_info == NULL ? "NULL info" : "")));
    
    if(config == NULL || group >= audio_output_group_all || gain_info == NULL)
    {
        Panic();
        return;
    }
    
    /* Tell DSP the overall system gain, master gain and tone gain - typically 
       this is used to sync volume changes on TWS. The DSP will send this 
       information back to the VM at the same time on both TWS devices*/
    gain_info->common.system = system_gain_from_master(master_gain);
    gain_info->common.master = master_gain;
    gain_info->common.tone   = tone_gain;
    
    /* Tell the DSP all trims, again this information is used for TWS 
       synchronised volume changes*/
    get_all_trims(group, FALSE, gain_info);
}

int16 AudioOutputGainGetFixedHardwareLevel(void)
{
    PanicNull((void*)config);
    return config->fixed_hw_gain;
}

static bool isPrimaryChannel(audio_output_t channel)
{
    return (channel == audio_output_primary_left || channel == audio_output_primary_right);
}

static bool isRawGainSetForChannel(audio_output_t channel)
{
    return (isPrimaryChannel(channel) && AudioConfigGetRawDacGain(channel) != DAC_OUTPUT_RAW_GAIN_NONE);
}

static void setDacOutputGain(audio_output_t channel, int16 dac_gain)
{
    Sink sink = audioOutputGetSink(channel);
    if(sink)
    {
        if(isRawGainSetForChannel(channel))
        {
            PanicFalse(SinkConfigure(sink, STREAM_CODEC_RAW_OUTPUT_GAIN, AudioConfigGetRawDacGain(channel)));
        }
        else
        {
            PanicFalse(SinkConfigure(sink, STREAM_CODEC_OUTPUT_GAIN, system_gain_from_master(dac_gain)));
        }
    }
}

static void set_hardware_gain(audio_output_t audio_out, int16 hardware_gain, int16 trim)
{
    switch(config->mapping[audio_out].endpoint.type)
    {
        case audio_output_type_dac:
        {
            setDacOutputGain(audio_out, hardware_gain);
        }
        break;
        
        case audio_output_type_i2s:
        {
            /* Include trim in hardware setting (only for i2s) */
            hardware_gain += trim;
            AudioI2SSetChannelVolume(get_i2s_chan(audio_out), hardware_gain, TRUE);
        }
        break;
        
        default:
            /* Ignore unrecognised type */
        break;
    }
}

/* If gain_info is NULL get trim from config->mapping, otherwise get it from gain_info */
#define get_trim(gain_info, group, chan) (int16)(gain_info ? gain_info->trim.group.chan : config->mapping[audio_output_##chan].volume_trim)

/* If I2S trim not enabled return 0, otherwise get the trim using get_trim */
#define get_trim_if_enabled(inc_i2s_trim, gain_info, group, chan) (int16)(inc_i2s_trim ? get_trim(gain_info, group, chan) : 0)

static void set_all_hardware_gains(const audio_output_group_t group,
                                   const int16 hardware_gain, 
                                   const bool inc_i2s_trim,
                                   const audio_output_gain_t* gain_info)
{
    PRINT(("applied hw gain %d ", system_gain_from_master(hardware_gain)));
    PRINT(("%s\n", (inc_i2s_trim ? "(+I2S trims)" : "")));
    
    if(group == audio_output_group_main)
    {
        set_hardware_gain(audio_output_primary_left, hardware_gain, get_trim_if_enabled(inc_i2s_trim, gain_info, main, primary_left));
        set_hardware_gain(audio_output_primary_right, hardware_gain, get_trim_if_enabled(inc_i2s_trim, gain_info, main, primary_right));
        set_hardware_gain(audio_output_secondary_left, hardware_gain, get_trim_if_enabled(inc_i2s_trim, gain_info, main, secondary_left));
        set_hardware_gain(audio_output_secondary_right, hardware_gain, get_trim_if_enabled(inc_i2s_trim, gain_info, main, secondary_right));
        set_hardware_gain(audio_output_wired_sub, hardware_gain, get_trim_if_enabled(inc_i2s_trim, gain_info, main, wired_sub));
    }
    else if(group == audio_output_group_aux)
    {
        set_hardware_gain(audio_output_aux_left, hardware_gain, get_trim_if_enabled(inc_i2s_trim, gain_info, aux, aux_left));
        set_hardware_gain(audio_output_aux_right, hardware_gain, get_trim_if_enabled(inc_i2s_trim, gain_info, aux, aux_right));
    }
}

void AudioOutputGainSetHardware(const audio_output_group_t group,
                                 const int16 master_gain,
                                 const audio_output_gain_t* gain_info)
{
    int16 hardware_gain;
    hybrid_gains_t hybrid;
    bool inc_i2s_trim = FALSE;
    
    PRINT(("AudioOutputGainSetHardware "));
    PRINT(("%s: ", (group == audio_output_group_main ? "Main" : " Aux")));
    PRINT(("master gain %d ", master_gain));
    
    if(config == NULL || group >= audio_output_group_all)
    {
        Panic();
        return;
    }
    
    switch(config->gain_type[group])
    {
        case audio_output_gain_hardware:
            /* Apply master gain at hardware level */
            hardware_gain = master_gain;
            
            /* When using hardware gain (and not muting) I2S trims should be adjusted 
               to include the channel trim */
            inc_i2s_trim = (hardware_gain != DIGITAL_VOLUME_MUTE);
        break;
        
        case audio_output_gain_digital:
            /* Master gain applied in DSP, set fixed gain at hardware level */
            hardware_gain = config->fixed_hw_gain;
        break;
        
        case audio_output_gain_hybrid:
            /* Use codec function to obtain hybrid values */
            CodecCalcHybridValues(&hybrid, master_gain);
            /* Convert the DAC gain to dB/60 */
            hardware_gain = VolumeConvertDACGainToDB(hybrid.dac_gain);
        break;
        
        default:
            Panic();
            return;
    }
    
    set_all_hardware_gains(group, hardware_gain, inc_i2s_trim, gain_info);
}

void AudioOutputGainSetHardwareOnly(const audio_output_group_t group,
                                     const int16 master_gain)
{
    /* When not muting I2S trims should be adjusted to include the channel trim */
    bool inc_i2s_trim = (master_gain != DIGITAL_VOLUME_MUTE);

    PRINT(("AudioOutputGainSetHardwareOnly "));
    PRINT(("%s: ", (group == audio_output_group_main ? "Main" : " Aux")));
    PRINT(("master gain %d ", master_gain));
    
    if(config == NULL || group >= audio_output_group_all)
    {
        Panic();
        return;
    }
    
    set_all_hardware_gains(group, master_gain, inc_i2s_trim, NULL);
}

audio_output_gain_type_t AudioOutputGainGetType(const audio_output_group_t group)
{
    if(config == NULL || group >= audio_output_group_all)
    {
        Panic();
        return audio_output_gain_invalid;
    }
    
    return config->gain_type[group];
}
