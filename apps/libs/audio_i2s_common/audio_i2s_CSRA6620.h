/****************************************************************************
Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    csr_i2s_CSRA6620_plugin.h
    
DESCRIPTION
	
*/

/*!
@file   csr_i2s_CSRA6620_plugin.h
@brief  Header file for the CSRA6620 specific plugin interface.

    The parameters / enums here define the message interface used for the 
    chip specific i2s audio plugin
    
*/

#ifndef _CSR_I2S_CSRA6620_PLUGIN_H_
#define _CSR_I2S_CSRA6620_PLUGIN_H_

#include <library.h>
#include <power.h>
#include <stream.h>


/* Register definitions for the I2C control interface of the CSRA6620 device. */
typedef enum
{
    CH1_VOLUME_0_FA         = 0x702f,
    CH1_VOLUME_1_FA         = 0x7030,
    CH2_VOLUME_0_FA         = 0x7031,   
    CH2_VOLUME_1_FA         = 0x7032,
    EXT_PA_PROTECT_POLARITY = 0x707d,
    TEMP_PROT_BACKOFF       = 0x7082,
    CHIP_STATE_CTRL_FA      = 0x7085,
    PWM_OUTPUT_CONFIG       = 0x7087,
    CHIP_STATE_STATUS_FA    = 0x70a2
    
}CSRA6620_reg_map;

/********** Register Bitfield Enumeration Definitions **********/

/* Register 7085 CHIP_STATE_CTRL_FA */
typedef enum
{
    CHIP_STATE_CTRL_FOR_CONFIG_MODE = 0x0,
    CHIP_STATE_CTRL_ALLOW_RUN       = 0x1,
    CHIP_STATE_CTRL_FORCE_STBY      = 0x2,
    CHIP_STATE_CTRL_DO_BURN         = 0x4,
    CHIP_STATE_CTRL_DO_CAL          = 0x8,
    CHIP_STATE_CTRL_BURN_MANUAL_CAL = 0x10
}chip_state_ctrl_fa_t;

/*  Register 7082 TEMP_PROT_BACKOFF */
typedef enum
{
    TEMP_BACKOFF_DISABLE            = 0x1,
    TEMP_BACKOFF_TYPE               = 0x2,
    TEMP_BACKOFF_INTERVAL_30mS      = 0x4,
    TEMP_BACKOFF_INTERVAL_120mS     = 0x8, 
    TEMP_BACKOFF_INTERVAL_520mS     = 0xC                          
}temp_prot_backoff_t;

/*  Register 707d EXT_PA_PROTECT_POLARITY */
typedef enum
{
    PA_PROTECT_POLARITY_ACT_LOW     = 0x1, 
    PA_THERM_WARN_POLARITY_ACT_LOW  = 0x2                         
}ext_pa_protect_polarity_t;

/*  Register 7087 PWM_OUTPUT_CONFIG */
typedef enum
{
    LOW_SIDE_POLARITY               = 0x1,
    HIGH_SIDE_POLARITY              = 0x2,
    BRIDGE_PROTOCOL                 = 0x4,
    BRIDGE_TYPE_SEL                 = 0x8,
    MODULATION_RATE_800KHZ          = 0x16,
    MODULATION_MODE_BD              = 0x40,
    CLAMP_RESET_SEL                 = 0x80                                  
}pwm_output_config_t;



#define CTRL_VALUE_FOR_CONFIG_MODE              (CHIP_STATE_CTRL_FOR_CONFIG_MODE)

                  /* Temperature protection backoff disable: => Enable
                     Temperature protection backoff mechanism: => FSSD filer, 
                     Minimum interval between subsequent applications of the temperature backoff procedure: => Every 520 mSec */
#define CTRL_VALUE_FOR_TEMP_PROTECTION          (TEMP_BACKOFF_INTERVAL_520mS)

                 /*  Bit 0: (1) PROTECT input is active low  
                     Bit 1: (1) THERM_WARN input is active low */
#define CTRL_VALUE_FOR_PA_POLARITY_PROTECTION   (PA_PROTECT_POLARITY_ACT_LOW | PA_THERM_WARN_POLARITY_ACT_LOW)

                /*   Bit 0:  (0) Non-inverted low-side polarity (NMOS equivalent control)
                     Bit 1:  (1) Inverted high-side polarity (PMOS equivalent control)
                     Bit 2:  (1) Generate high-side and low-side signals separately for each half-bridge
                     Bit 3:  (0) Single-ended (half bridge) signaling and feedback
                     Bit 4:5 (1) Modulation rate set to 800kHz
                     Bit 6:  (0) Modulation Mode is AD
                     Bit 7:  (0) CLAMP pin used as CLAMP (with delay between deassertion of CLAMP and PWM startup) */
#define CTRL_VALUE_FOR_PWM_OUTPUT_CONFIG        (MODULATION_RATE_800KHZ | BRIDGE_PROTOCOL | HIGH_SIDE_POLARITY)

#define CTRL_VALUE_FOR_ALLOW_RUN                (CHIP_STATE_CTRL_ALLOW_RUN)

#define CTRL_VALUE_FOR_SHUTTING_DOWN            (CHIP_STATE_CTRL_FORCE_STBY)

/* the address byte also contains the read/write flag as bit 7 */
#define ADDR_CSRA6620 						0x68

#define ADDR_CSRA6620_7BITS     			(ADDR_CSRA6620 << 1)

/* MAX and MIN volume values */
#define MIN_VOLUME_LIMIT_IN_DB				-9025
#define MAX_VOLUME_LIMIT_IN_DB				+2400

#define MIN_VOLUME_LIMIT_AS_UINT			0
#define MAX_VOLUME_LIMIT_AS_UINT			457

/* Default Volumes values */
#define DEFAULT_LEFT_VOLUME_DB              -1000   /* in db x 100 */
#define DEFAULT_RIGHT_VOLUME_DB             -1000

/* Internal message ids */
#define MESSAGE_CHECK_SW_BOOT_STATUS        0x0081
#define MESSAGE_DELAY_COMPLETE              0x0082 

#define STANDY_MODE_VALUE                   0x02 
#define CONFIG_MODE_VALUE                   0x00 


/****************************************************************************
DESCRIPTION: AudioI2SInitialise_CSRA6620 :

    This function configures the I2S device 

PARAMETERS:
    
    uint32 sample_rate - sample rate of data coming from dsp

RETURNS:
    none
*/    
void AudioI2SInitialise_CSRA6620(uint32 sample_rate);

/****************************************************************************
DESCRIPTION: handleNextConfigStep :,

    This function handles processing of the next configuration step in the 
    setting up of the CSRA6620 device.

PARAMETERS:
    
    none

RETURNS:
    none
*/  
void handleNextConfigStep(void);

/****************************************************************************
DESCRIPTION: AudioI2SSetVolume_CSRA6620

    This function sets the I2S device volume via the I2C

PARAMETERS:
    
    uint16 volume - volume level required

RETURNS:
    none
*/    
void AudioI2SSetVolume_CSRA6620(int16 left_vol, int16 right_vol, bool volume_in_dB);

/****************************************************************************
DESCRIPTION: AudioI2SShutdown_CSRA6620 :

    This function stutdowns the CSRA6620 device 

PARAMETERS:
    
    none

RETURNS:
    none
*/    
void AudioI2SShutdown_CSRA6620(void);


#endif /*_CSR_I2S_CSRA6620_PLUGIN_H_*/
