/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

 
FILE NAME
    audio_data_types.h
 
DESCRIPTION
    Header defining audio related data types plus some utility functions.

    Note: This file shouldn't include other libraries.
    E.g. Conversion from type defined here to some HFP type is needed it shouldn't be done here.
*/

#ifndef AUDIO_DATA_TYPES_H_
#define AUDIO_DATA_TYPES_H_

#include <csrtypes.h>

typedef enum
{
    audio_codec_none,
    audio_codec_sbc,
    audio_codec_aptx,
    audio_codec_pcm,
    audio_codec_aptx_ll,
    audio_codec_aptx_hd,
    audio_codec_aac,
    audio_codec_celt,
    audio_codec_aptx_adaptive
} audio_codec_t;

typedef struct __volume_group_config_t
{
    int16 no_of_steps;         /* number of steps of volume change permitted */
    int16 volume_knee_value_1;  /* volume point at which curve of dB conversion changes */
    int16 volume_knee_value_2;  /* volume point at which curve of dB conversion changes */
    int16 dB_knee_value_1;      /* dB value for point at which curve of dB conversion changes */
    int16 dB_knee_value_2;      /* dB value for point at which curve of dB conversion changes */
    int16 dB_max;               /* dB value at maximum volume level */
    int16 dB_min;               /* dB value at minimum volume level */
} volume_group_config_t;

/* Integer in range 0 to 100 can't hold enough precision to represent required 500 volume steps.
   This is why percentage representation of the volume must be scaled up
   and stored in big enough variable.
 */
typedef uint16 scaled_volume_percentage_t;

/* Size of scaled_volume_percentage_t in bytes */
#define SCALED_VOLUME_PERCENTAGE_SIZE (sizeof(scaled_volume_percentage_t)/sizeof(uint8))

#define VOLUME_PERCENTAGE_SCALING_FACTOR 5

#define BROADCAST_VOLUME_STEPS (32)

#endif /* AUDIO_DATA_TYPES_H_ */
