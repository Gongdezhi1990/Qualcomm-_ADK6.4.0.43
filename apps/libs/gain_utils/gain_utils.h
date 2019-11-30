/****************************************************************************
Copyright (c) 2004 - 2016 Qualcomm Technologies International, Ltd.

*/
/*!
    @file   gain_utils.h

    @brief Header file for the gain utility library. This library implements several utility volume
    functions
 
 */

/*@{*/

#ifndef GAIN_UTILS_H_
#define GAIN_UTILS_H_

#include <csrtypes.h>
#include <audio_data_types.h>

typedef struct
{
    int16 dsp_db_scaled;
    uint8 dac_gain;
}hybrid_gains_t;

/* number of DAC steps */
#define CODEC_STEPS             15
#define MIN_CODEC_GAIN_DB       -45
#define MIN_CODEC_GAIN_STEPS    0

/* DSP gain values are set in dB/60 */ 
#define DB_DSP_SCALING_FACTOR 60

/* Macro to convert dB to dB/60 */
#define GainIn60thdB(gain_db) ((gain_db) * DB_DSP_SCALING_FACTOR)

/* DAC steps of 3 dB * scaling factor of dsp volume control which is 60 */
#define DB_TO_DAC                           GainIn60thdB(3)

/* Max and Min digital volume (Valid for Kalimba only) */
#define MAXIMUM_DIGITAL_VOLUME_0DB          GainIn60thdB(0)
#define MINIMUM_DIGITAL_VOLUME_80dB         GainIn60thdB(-80)

/* voice prompts played through the dsp require 6 dB of attenuation */
#define DSP_VOICE_PROMPTS_LEVEL_ADJUSTMENT  GainIn60thdB(-6) 

/* mute is -120dB */
#define DIGITAL_VOLUME_MUTE                 GainIn60thdB(-120)

/*!
    @brief Defines used to indicate Codec Gain Ranges
*/
#define CODEC_INPUT_GAIN_RANGE      22
#define CODEC_OUTPUT_GAIN_RANGE     22

#define RAW_GAIN_DIGITAL_COMPONENT_0_GAIN    0x8020

/*! 
    @brief The codecs channel being referred to, left, right or both.
*/
typedef enum
{
    /*! The left channel of the codec.*/
    left_ch,                        
    /*! The right channel of the codec.*/
    right_ch,                       
    /*! The left and right channel of the codec. */
    left_and_right_ch               
} codec_channel;


/*!
    @brief Update the codec input gain immediately. 
   
    @param volume The gain level (volume) to set the input channel to.

    @param channel The channel to use.
*/
void CodecSetInputGainNow(uint16 volume, codec_channel channel);


/*!
    @brief Update the codec output gain immediately. 

    @param volume The gain level (volume) to set the output left channel to.

    @param channel The channel to use.
*/
void CodecSetOutputGainNow(uint16 volume, codec_channel channel);
   


/****************************************************************************
NAME
 VolumeConvertDACGainToDB

DESCRIPTION
 Converts dac gain to dB value

RETURNS
 dB value

*/
int16 VolumeConvertDACGainToDB(int16 DAC_Gain);

/****************************************************************************
NAME
 GainDbToDacGain

DESCRIPTION
 Converts dB to dac gain

RETURNS
 dac gain

*/
uint16 GainDbToDacGain(int16 volume);

/****************************************************************************
NAME
 VolumeConvertStepsToDB

DESCRIPTION
 Converts decimal step values into dB values for DSP volume control scheme

RETURNS
 dB value

*/
int16 VolumeConvertStepsToDB(const int16 Volume, const volume_group_config_t * volMappingConfig);

/****************************************************************************
NAME
 codecCalcHybridValues

DESCRIPTION
 Converts decimal step values into dB values for DSP volume control scheme

RETURNS
 dB value

*/
void CodecCalcHybridValues(hybrid_gains_t * gains, int16 volume);

/****************************************************************************
NAME
    GainUtilsConvertVolumeStepToScaledPercentage

DESCRIPTION
    Returns a scaled percentage of a full scale volume corresponding to a volume step.

RETURNS
    Scaled percentage of full scale volume

*/
scaled_volume_percentage_t GainUtilsConvertVolumeStepToScaledPercentage(int16 volume_step,
        volume_group_config_t * volMappingConfig);

/****************************************************************************
NAME
    GainUtilsConvertScaledVolumePercentageToStep

DESCRIPTION
    Returns a volume step corresponding to a scaled percentage of total steps

RETURNS
    Volume step

*/
int16 GainUtilsConvertScaledVolumePercentageToStep(scaled_volume_percentage_t scaled_percentage,
        volume_group_config_t * volMappingConfig);

/*!
    @brief Convert a broadcast volume to dB.
    
    @param volume Broadcast volume in the range 0..31.
    
    @return Volume in dB/60.
*/
int16 ConvertBroadcastVolumeTodB(uint16 volume, int16 *conversion_table);

/*!
    @brief Convert a dB volume to broadcast volume.

    @param volume Volume in dB/60.

    @return Broadcast volume in the range 0..31.
*/
uint16 ConvertdBToBroadcastVolume(int16 volume, int16 *conversion_table);

/*!
    @brief Calculate the analogue component of a raw codec gain value.

    @param dB_60 gain in dB/60.

    @return db_60 expressed as a analogue component of a raw gain value.
*/
uint16 gainUtilsCalculateRawAdcGainAnalogueComponent(uint16 dB_60);


#endif /* GAIN_UTILS_H_ */
/*@}*/
