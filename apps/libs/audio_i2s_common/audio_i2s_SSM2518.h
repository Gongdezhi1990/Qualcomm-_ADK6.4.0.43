/****************************************************************************
Copyright (c) 2013 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    csr_i2s_SSM2518_plugin.h
    
DESCRIPTION
	
*/

/*!
@file   csr_i2s_SSM2518_plugin.h
@brief  Header file for the chpi specific plugin interface.

    The parameters / enums here define the message interface used for the 
    chip specific i2s audio plugin
    
     
*/


#ifndef _CSR_I2S_SSM2518_PLUGIN_H_
#define _CSR_I2S_SSM2518_PLUGIN_H_

#include <library.h>
#include <power.h>
#include <stream.h>


/* register definitions for the I2C control interface of the SSM 2518 device */
typedef enum
{
    RESET_POWER_CONTROL = 0x00,
    EDGE_CLOCK_CONTROL,
    SERIAL_INTERFACE_SAMPLE_RATE_CONTROL,
    SERIAL_INTERFACE_CONTROL,
    CHANNEL_MAPPING_CONTROL,
    LEFT_VOLUME_CONTROL,
    RIGHT_VOLUME_CONTROL,
    VOLUME_MUTE_CONTROL,
    FAULT_CONTROL_1,
    POWER_FAULT_CONTROL,
    DRC_CONTROL_1,
    DRC_CONTROL_2,
    DRC_CONTROL_3,
    DRC_CONTROL_4,
    DRC_CONTROL_5,
    DRC_CONTROL_6,
    DRC_CONTROL_7,
    DRC_CONTROL_8,
    DRC_CONTROL_9
}SSM2518_reg_map;

/* the address byte also contains the read/write flag as bit 7 */
#define ADDR_WRITE 0x68 
#define ADDR_READ  0x96 

#define S_RST    0x80
#define SPWDN    0x01
#define ASR      0x01
#define M_MUTE   0x01
#define M_UNMUTE 0x00     


/****************************************************************************
DESCRIPTION: AudioI2SInitialise_SSM2518 :

    This function configures the I2S device 

PARAMETERS:
    
    uint32 sample_rate - sample rate of data coming from dsp

RETURNS:
    none
*/    
void AudioI2SInitialise_SSM2518(uint32 sample_rate);

/****************************************************************************
DESCRIPTION: AudioI2SSetVolume_SSM2518

    This function sets the I2S device volume via the I2C

PARAMETERS:
    
    uint16 volume - volume level required

RETURNS:
    none
*/    
void AudioI2SSetVolume_SSM2518(int16 left_vol, int16 right_vol, bool volume_in_dB);

#endif
