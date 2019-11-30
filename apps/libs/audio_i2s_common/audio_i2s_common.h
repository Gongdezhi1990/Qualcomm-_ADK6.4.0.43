/****************************************************************************
Copyright (c) 2013 - 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_i2s_common.h

DESCRIPTION

*/

/*!
@file   audio_i2s_common.h
@brief  Base functionality for I2S audio input and output devices.
     
*/


#ifndef _AUDIO_I2S_COMMON_H_
#define _AUDIO_I2S_COMMON_H_

#include <library.h>
#include <power.h>
#include <stream.h>

typedef enum
{
    i2s_plugin_none_use_pskey = 0,
    i2s_plugin_ssm2518,
    i2s_plugin_csra6620,
    i2s_plugin_future_expansion
    
}i2s_plugin_type;

typedef enum
{
    i2s_out_1_left = 0,
    i2s_out_1_right,
    i2s_out_2_left,
    i2s_out_2_right
} i2s_out_t;

/* PSKEY_USR35 I2S pskey configuration*/
typedef struct
{
    /* 0: pskey config, 1: SSM2518 device, 2: CSRA6620 device  */
    unsigned plugin_type:8;
    /* master or slave operation */
    unsigned master_operation:8;
    /* left justified or i2s data */
    unsigned left_or_right_justified:4;
    /* justified delay by 1 bit */
    unsigned justified_bit_delay:4;   
    /* bits per sample */
    unsigned bits_per_sample:8;
    /* length of i2s configuration data pskey */
    unsigned i2s_configuration_command_pskey_length:8;
    /* number of individual i2c initialistion commands sent to i2s device */
    unsigned number_of_initialisation_cmds:8;   
    /* number of individual i2c volume commands sent to i2s device */
    unsigned number_of_volume_cmds:8;   
    /* offset in data of start of first volume command */
    unsigned volume_cmds_offset:8;
    /* number of individual i2c shutdown commands sent to an i2s device */
    unsigned number_of_shutdown_cmds:8;   
    /* offset in data of start of first shutdown command */
    unsigned shutdown_cmds_offset:8;
    /* specifies the number of bits of the volume setting in the i2c command, e.g. 8 or 16 */
    unsigned volume_no_of_bits:16;
    /* specifies the volume range maximum, volume sent from sink will be scaled to this range */
    unsigned volume_range_max:16;
    /* specifies the volume range minimum, volume sent from sink will be scaled to this range */
    unsigned volume_range_min:16;
    /* specifies whether to resample music rates for i2s output 0 = no resampling */
    unsigned music_resampling_frequency:16;
    /* specifies whether to resample voice rates for i2s output 0 = no resampling */
    unsigned voice_resampling_frequency:16;
    /* i2s bit clock frequency scaling factor */
    unsigned bit_clock_scaling_factor:16;
    /* i2s master clock frequency scaling factor */
    unsigned master_clock_scaling_factor:16;
    /* i2s input audio bit resolution */
    unsigned enable_24_bit_audio_input:1;
     /* external amplifier enable pio */
    unsigned enable_pio:8;
    /* Unused */
    unsigned unused:7;
    
  
} i2s_init_config_t;

#define PACKET_LENGTH 0
#define PACKET_VOLUME_OFFSET 1
#define PACKET_I2C_ADDR 2
#define PACKET_DATA 3

#define I2S_MINIMUM_SUPPORTED_OUTPUT_SAMPLE_RATE    44100
#define I2S_FALLBACK_OUTPUT_SAMPLE_RATE             48000
#define I2S_NO_RESAMPLE                             0

/* PSKEY_USR36 I2S configuration data */
typedef struct
{ 
    /* data takes the form of:
       {
           uint8 packet_length;
           uint8 volume_level_offset_within_packet;   ***if applicable***
           uint8 packet_data[packet_length];
       }
    */  
    uint8 data[1];
} i2s_data_config_t;

/* configuration in pskey format */
typedef struct
{   
    /* plugin type and lenght of data definitions */
    i2s_init_config_t i2s_init_config;
    /* i2s pskey configuration data */
    i2s_data_config_t i2s_data_config;
    /* don't add anything here, it will get overwritten */    
}I2SConfiguration;
    


/****************************************************************************
DESCRIPTION: AudioI2SInitialisePlugin :

    This function gets a pointer to the application malloc'd slot
    containing the i2s configuration data

PARAMETERS:

    pointer to malloc'd slot

RETURNS:
    none
*/
void AudioI2SInitialise(I2SConfiguration * config);


/****************************************************************************
DESCRIPTION: AudioI2SAudioOutputConfigureSink :

    This function configures the I2S interface ready for a connection.

PARAMETERS:

    Sink sink   - The Sink to configure
    uint32 rate - Sample rate of the data coming from the DSP

RETURNS:
    Nothing.
*/
void AudioI2SConfigureSink(Sink sink, uint32 rate);

/****************************************************************************
DESCRIPTION: AudioI2SCommonEnableMasterClockIfRequired :

    This function enables or disables the MCLK signal for an I2S interface,
    if required. It is separate from sink configuration as it must happen
    after all channels for a given interface have been configured, or after
    all channels of the interface have been disconnected, if disabling MCLK.

PARAMETERS:
    Sink sink   - The Sink to enable/disable MCLK for
    Bool enable - TRUE to enable MCLK output, FALSE to disable

RETURNS:
    none
*/
void AudioI2SEnableMasterClockIfRequired(Sink sink, bool enable);

/****************************************************************************
DESCRIPTION: AudioI2SSourceEnableMasterClockIfRequired :

    This function enables or disables the MCLK signal for an I2S interface,
    if required. It is separate from source configuration as it must happen
    after all channels for a given interface have been configured, or after
    all channels of the interface have been disconnected, if disabling MCLK.

PARAMETERS:
    Source source   - The Source to enable/disable MCLK for
    Bool enable     - TRUE to enable MCLK output, FALSE to disable

RETURNS:
    none
*/
void AudioI2SSourceEnableMasterClockIfRequired(Source source, bool enable);


/****************************************************************************
DESCRIPTION: AudioI2SAudioOutputSetVolume :

    This function sets the volume level of the I2S external hardware if supported
    by the device being used.

PARAMETERS:
    
    bool   stereo - indicates whether to connect left or left and right channels

RETURNS:
    none
*/
void AudioI2SAudioOutputSetVolume(bool stereo, int16 left_volume, int16 right_volume, bool volume_in_dB);

/****************************************************************************
DESCRIPTION: CsrCommonInitialiseI2SDevice :

    This function configures the I2S device 

PARAMETERS:
    
    uint32 sample_rate - sample rate of data coming from dsp

RETURNS:
    none
*/    
void AudioI2SInitialiseDevice(uint32 sample_rate);

/****************************************************************************
DESCRIPTION: CsrShutdownI2SDevice :

    This function shuts down the I2S device 

PARAMETERS:
    
    none
    
RETURNS:
    none
*/    
void AudioShutdownI2SDevice(void);

/******************************************************************************
DESCRIPTION: CsrSetVolumeI2SChannel

    This function sets the volume of a single I2S channel on a specific I2S
    device via the I2C interface. The volume can either be passed in as a value
    in 1/60th's of dB (with range -7200 to 0), or as an absolute value in the 
    same format used by the CODEC plugin for compatibility (range 0x0 - 0xf).

PARAMETERS:
    i2s_out_t channel   The I2S device and channel to set the volume of.
    int16 volume        The volume level required, in dB/60 or CODEC_STEPS.
    bool volume_in_dB   Set to TRUE if volume passed in dB, FALSE otherwise.

RETURNS:
    Whether volume was successfully changed for the requested device channel.
*/
bool AudioI2SSetChannelVolume(i2s_out_t channel, int16 vol, bool volume_in_dB);

/******************************************************************************
DESCRIPTION: CsrSetVolumeI2SDevice

    This function sets the primary I2S device volume via the I2C interface, for
    use in stereo mode.

PARAMETERS:
    int16 left_vol    - Volume level for primary left channel.
    int16 right_vol   - Volume level for primary left channel.
    bool volume_in_dB - Whether the volume is passed in in dB or CODEC_STEPS.

RETURNS:
    none
*/
void AudioI2SSetDeviceVolume(int16 left_vol, int16 right_vol, bool volume_in_dB);

/****************************************************************************
DESCRIPTION: AudioI2SMusicResamplingFrequency

    This function returns the current resampling frequency for music apps,

PARAMETERS:
    
    none

RETURNS:
    frequency or 0 indicating no resampling required
*/    
uint16 AudioI2SMusicResamplingFrequency(void);

/****************************************************************************
DESCRIPTION: AudioI2SVoiceResamplingFrequency

    This function returns the current resampling frequency for voice apps,

PARAMETERS:
    
    none

RETURNS:
    frequency or 0 indicating no resampling required
*/    
uint16 AudioI2SVoiceResamplingFrequency(void);

/****************************************************************************
DESCRIPTION: AudioI2SConfigureSource

    This function configures the supplied source

PARAMETERS:

    Source

RETURNS:
    none
*/
void AudioI2SConfigureSource(Source source, uint32 rate, uint16 bit_resolution);

/****************************************************************************
DESCRIPTION: AudioI2SMasterIsEnabled

    This function returns the I2S operation mode 

PARAMETERS:
    
    none

RETURNS:
    
    TRUE : Operation mode is I2S Master
    FALSE : Operation mode is I2S Slave 
    
*/    
bool AudioI2SMasterIsEnabled(void);

/****************************************************************************
DESCRIPTION: AudioI2SIs24BitAudioInputEnabled

    This function returns true if I2S input audio is of 24 bit resolution 

PARAMETERS:
    
    none

RETURNS:
    
    TRUE : I2S Audio input is 24 bit enabled
    FALSE : I2S Audio input is 16 bit enabled. 
    
*/
bool AudioI2SIs24BitAudioInputEnabled(void);

/****************************************************************************
DESCRIPTION: AudioI2SIsMclkRequired :

    This function returns true if I2S has been configured to output an MCLK signal

PARAMETERS:

    none

RETURNS:

    TRUE : I2S MCLK required
    FALSE : I2S MCLK not required
*/
bool AudioI2SIsMasterClockRequired(void);

uint32 AudioI2SGetOutputResamplingFrequencyForI2s(const uint32 requested_rate);


#endif
