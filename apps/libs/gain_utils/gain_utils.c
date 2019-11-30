/****************************************************************************
Copyright (c) 2004 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    gain_utils.c
    
DESCRIPTION

*/

#include <stdlib.h>
#include <panic.h>

#include "gain_utils.h"

typedef struct
{
    int16 volMax;
    int16 volMin;
    int16 x0;
    int16 y0;
    int16 x1;
    int16 y1;
    int16 dx;
    int16 dy;
    int16 e;
    int16 dB;
}convert_volume_db_t;

/****************************************************************************
NAME
 VolumeConvertDACGainToDB

DESCRIPTION
 Converts dac gain to dB value

RETURNS
 dB value

*/
int16 VolumeConvertDACGainToDB(int16 DAC_Gain)
{
    int32 Result;    
    
    /* convert from 0 to 15 DAC gain to -45 to 0dB */  
    Result = (DAC_Gain * (0 - MIN_CODEC_GAIN_DB)) / CODEC_STEPS;
    Result += MIN_CODEC_GAIN_DB;
    Result *= DB_DSP_SCALING_FACTOR;
    
    return (int16)Result;
}

/****************************************************************************
NAME
 GainDbToDacGain

DESCRIPTION
 Converts dB to dac gain

RETURNS
 dac gain

*/
uint16 GainDbToDacGain(int16 volume)
{
    int16 dac_gain = (CODEC_STEPS + (volume / DB_TO_DAC));

    /* DAC gain only goes down to -45dB */
    if(dac_gain < 0)
        dac_gain = 0;

    return (uint16)dac_gain;
}

/****************************************************************************
NAME
 VolumeConvertStepsToDB

DESCRIPTION
 Converts decimal step values into dB values for DSP volume control scheme

RETURNS
 dB value

*/
int16 VolumeConvertStepsToDB(const int16 Volume, const volume_group_config_t * volMappingConfig)
{
    /* Volume - volume number from 1 to no_of_steps (it is assumed that vol 0 will be mute)
     dbMin - minimum volume in integer dB returned when Volume is 1
     dbMax - maximum volume in integer dB returned when Volume is number of steps - 1
     Knee(2) - Volume at which volume curve changes
     dbKnee(2) - integer dB of volume at Knee changes

     if a single segment line is required, then set avrcpKnee to 0 and dbKnee to dbMin */

    int16 result;
    convert_volume_db_t * convert = PanicUnlessMalloc(sizeof(convert_volume_db_t));
    
    convert->volMax = (volMappingConfig->no_of_steps - 1);
    convert->volMin = 1;
    
    convert->x0 = volMappingConfig->volume_knee_value_1;
    convert->y0 = volMappingConfig->dB_knee_value_1;
    convert->x1 = volMappingConfig->volume_knee_value_2;
    convert->y1 = volMappingConfig->dB_knee_value_2;

    if (Volume < volMappingConfig->volume_knee_value_1)
    {
        convert->x0 = convert->volMin;
        convert->x1 = volMappingConfig->volume_knee_value_1;
        convert->y0 = volMappingConfig->dB_min;
        convert->y1 = volMappingConfig->dB_knee_value_1;
    }

    if (Volume > volMappingConfig->volume_knee_value_2)
    {
        convert->x0 = volMappingConfig->volume_knee_value_2;
        convert->x1 = convert->volMax;
        convert->y0 = volMappingConfig->dB_knee_value_2;
        convert->y1 = volMappingConfig->dB_max;
    }
    convert->dx = convert->x1 - convert->x0;
    convert->dy = convert->y1 - convert->y0;

    convert->e = (Volume-convert->x0)*convert->dy;
    convert->dB = ((1+2*convert->e/convert->dx)/2);
    convert->e -= convert->dB*convert->dx;
    convert->dB = (convert->y0+convert->dB)*DB_DSP_SCALING_FACTOR + ((1+2*DB_DSP_SCALING_FACTOR*convert->e/convert->dx)/2);

    result = convert->dB;
    
    /* free temporary memory */
    free (convert);
    
    return result;
}

/****************************************************************************
NAME
 codecCalcHybridValues

DESCRIPTION
 Converts dB volume (*60) into separate dsp gain of 0 to -3dB and a dac gain

RETURNS
 dac gain and dB dsp value

*/
void CodecCalcHybridValues(hybrid_gains_t * gains, int16 volume)
{
    /* Ensure volume is within allowed range of between -80dB and 0dB */
    if((volume > MAXIMUM_DIGITAL_VOLUME_0DB))
        volume = MAXIMUM_DIGITAL_VOLUME_0DB;
    
    if(volume < MINIMUM_DIGITAL_VOLUME_80dB)
        volume = MINIMUM_DIGITAL_VOLUME_80dB;
    
    /* integer maths to get the next highest dac gain value in the range 0 to 15 whereby
       0 = -45dB and 15 = 0dB */
    if(volume < (MIN_CODEC_GAIN_DB * DB_DSP_SCALING_FACTOR))
    {
        /* < -45dB which is outside of the DAC gain control, therefore use
           min DAC gain and control the rest via the DSP */
        gains->dac_gain = 0;
    }
    /* volume required is within the range -45 to 0dB which is within the DAC control range */
    else
    {
        gains->dac_gain = (CODEC_STEPS + (volume/DB_TO_DAC));
    }
    
    /* dsp attenuation up to -3dB is now calculated */
    gains->dsp_db_scaled = (volume - ((CODEC_STEPS - gains->dac_gain) * DB_TO_DAC * -1));
}

/****************************************************************************
NAME
    GainUtilsConvertVolumeStepToScaledPercentage

DESCRIPTION
    Returns a scaled percentage of a full scale volume corresponding to a volume step.

RETURNS
    Scaled percentage of full scale volume

*/
scaled_volume_percentage_t GainUtilsConvertVolumeStepToScaledPercentage(int16 volume_step,
        volume_group_config_t * volMappingConfig)
{
    scaled_volume_percentage_t scaled_100_percent = 100 * VOLUME_PERCENTAGE_SCALING_FACTOR;
    return (scaled_volume_percentage_t)((scaled_100_percent * volume_step) / volMappingConfig->no_of_steps);
}

/****************************************************************************
NAME
    GainUtilsConvertScaledVolumePercentageToStep

DESCRIPTION
    Returns a volume step corresponding to a percentage of total steps

RETURNS
    Volume step

*/
int16 GainUtilsConvertScaledVolumePercentageToStep(scaled_volume_percentage_t scaled_percentage,
        volume_group_config_t * volMappingConfig)
{
    scaled_volume_percentage_t scaled_100_percent = 100 * VOLUME_PERCENTAGE_SCALING_FACTOR;
    int32 scaled_fraction_of_steps = (int32)(scaled_percentage * volMappingConfig->no_of_steps);
    int16 step = scaled_fraction_of_steps / scaled_100_percent;

    if(scaled_fraction_of_steps % scaled_100_percent)
    {
        step += 1;
    }
    return step;
}

int16 ConvertBroadcastVolumeTodB(uint16 volume, int16 *conversion_table)
{
    if(volume > (BROADCAST_VOLUME_STEPS - 1))
    {
        volume = BROADCAST_VOLUME_STEPS - 1;
    }
    
    return (int16)conversion_table[volume];
}

uint16 ConvertdBToBroadcastVolume(int16 volume_in_db, int16 *conversion_table)
{
    unsigned  best_index = 0;
    int closest_distance = 10000;
    int distance;
    unsigned i;

    for(i = 0; i < BROADCAST_VOLUME_STEPS; ++i)
    {
        distance = conversion_table[i] - volume_in_db;

        if(distance < 0)
        {
            distance *= -1;
        }

        if(distance < closest_distance)
        {
            closest_distance = distance;
            best_index = i;
        }
    }
    
    return (uint16)best_index;
}
