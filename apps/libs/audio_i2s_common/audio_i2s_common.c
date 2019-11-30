/****************************************************************************
Copyright (c) 2013 - 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_i2s_common.c
DESCRIPTION
    Base support for external I2S audio devices
NOTES
*/

#include <audio.h>
#include <gain_utils.h>
#include <stdlib.h>
#include <panic.h>
#include <print.h>
#include <file.h>
#include <stream.h> 
#include <sink.h>
#include <source.h>
#include <kalimba.h>
#include <kalimba_standard_messages.h>
#include <message.h>
#include <transform.h>
#include <string.h>
#include <i2c.h>
#include <pio_common.h>

#include "audio_i2s_common.h"
#include "audio_i2s_mclk.h"
#include "audio_i2s_SSM2518.h"
#include "audio_i2s_CSRA6620.h"

typedef enum
{
    standby = 0, /* Can be used during external amplifier in low power down mode */
    active,      /* Used when external amplifier is in running/active state */
    power_off    /* Used to denote external amplifier is powered off */
}external_amp_state;

typedef struct
{
    external_amp_state state;
}external_amp_state_t;

/* External Amplifier state */
static external_amp_state_t external_amp = { power_off };

/* I2S configuration data */
I2SConfiguration * I2S_config;

#define POLARITY_LEFT_WHEN_WS_HIGH  0
#define POLARITY_RIGHT_WHEN_WS_HIGH 1

static void EnableI2sDevice(bool enable);
static void SetExternalAmpState(external_amp_state state);
static external_amp_state GetExternalAmpCurrentState(void);

/****************************************************************************
DESCRIPTION: AudioI2SInitialise :

    This function gets a pointer to the application malloc'd slot
    containing the i2s configuration data

PARAMETERS:

    pointer to malloc'd slot

RETURNS:
    none
*/
void AudioI2SInitialise(I2SConfiguration * config)
{
    /* keep pointer to I2S pskey configuration in ram, not possible to read from
       ps on the fly everytime as interrupts audio */
    I2S_config = config;
}


/****************************************************************************
DESCRIPTION: AudioI2SConfigureSink :

    This function configures the I2S interface ready for a connection.

PARAMETERS:

    Sink sink   - The Sink to configure
    uint32 rate - Sample rate of the data coming from the DSP

RETURNS:
    Nothing.
*/
void AudioI2SConfigureSink(Sink sink, uint32 rate)
{
    /* configure the I2S interface operating mode, run in master or slave mode */
    PanicFalse(SinkConfigure(sink, STREAM_I2S_MASTER_MODE, I2S_config->i2s_init_config.master_operation));

    /* set the bit clock (BCLK) rate for master mode */
    PanicFalse(SinkConfigure(sink, STREAM_I2S_MASTER_CLOCK_RATE, (rate * I2S_config->i2s_init_config.bit_clock_scaling_factor)));

    /* set the word clock (SYNC) rate of the dsp audio data */
    PanicFalse(SinkConfigure(sink, STREAM_I2S_SYNC_RATE, rate));

    /* left justified or i2s data */
    PanicFalse(SinkConfigure(sink, STREAM_I2S_JSTFY_FORMAT, I2S_config->i2s_init_config.left_or_right_justified));

    /* MSB of data occurs on the second CLK */
    PanicFalse(SinkConfigure(sink, STREAM_I2S_LFT_JSTFY_DLY, I2S_config->i2s_init_config.justified_bit_delay));

    /* AUDIO_CHANNEL_SLOT_0 = left, AUDIO_CHANNEL_SLOT_1 = right */
    PanicFalse(SinkConfigure(sink, STREAM_I2S_CHNL_PLRTY, POLARITY_RIGHT_WHEN_WS_HIGH));

    /* number of data bits per sample, 16 or 24 */
    PanicFalse(SinkConfigure(sink, STREAM_I2S_BITS_PER_SAMPLE, I2S_config->i2s_init_config.bits_per_sample));

    /* configure MCLK for the interface */
    AudioI2SConfigureMasterClockIfRequired(sink, rate);

    PRINT(("I2S: AudioI2SConfigureSink A&B, %luHz\n", (unsigned long) rate));
}

/****************************************************************************
DESCRIPTION: AudioI2SAudioOutputSetVolume :

    This function sets the volume level of the I2S external hardware if supported
    by the device being used.

PARAMETERS:
    
    bool   stereo - indicates whether to connect left or left and right channels

RETURNS:
    none
*/
void AudioI2SAudioOutputSetVolume(bool stereo, int16 left_volume, int16 right_volume, bool volume_in_dB)
{

    PRINT(("I2S: AudioI2SAudioOutputSetVolume\n"));

    /* mute the output before disconnecting streams */    
    if(stereo)
        AudioI2SSetDeviceVolume(left_volume, right_volume, volume_in_dB);
    else
        AudioI2SSetDeviceVolume(left_volume, left_volume, volume_in_dB);
}

static void AudioI2SInitialiseDevice_PsKey(void)
{
    uint16 ack = 0;
    /* use the configuration information retrieved from ps or constant if no ps */
    if((I2S_config->i2s_init_config.i2s_configuration_command_pskey_length)&&
       (I2S_config->i2s_init_config.number_of_initialisation_cmds))
    {
        /* configuration data available */
        uint8 i;
        uint8* init_cmd = I2S_config->i2s_data_config.data;
        
        /* cycle through the configuration messages */
        for(i = 0;i < I2S_config->i2s_init_config.number_of_initialisation_cmds; i++)
        {
#ifdef DEBUG_PRINT_ENABLED
            uint8 j;

            PRINT(("I2S: Init Msg Addr=0x%04x,", init_cmd[PACKET_I2C_ADDR]));
            PRINT((" Write Reg 0x%02x =", init_cmd[PACKET_DATA]));
            for(j=1;j<(init_cmd[PACKET_LENGTH]-1);j++)
            {    
                PRINT((" 0x%02x", init_cmd[PACKET_DATA + j]));
            }
            PRINT(("\n"));
#endif

            /* send out packets */
            ack = I2cTransfer(init_cmd[PACKET_I2C_ADDR], &init_cmd[PACKET_DATA], (uint16)(init_cmd[PACKET_LENGTH] - 1), NULL, 0);
            PanicZero(ack);
            /* move to next packet */
            init_cmd += (2 + init_cmd[PACKET_LENGTH]);
        }
    }
    /* no configuration data available so no need to send any i2c commands */
}


/****************************************************************************
DESCRIPTION: EnableI2sDevice :

    This function enables the external amplifier 

PARAMETERS:
    
    enable - enable or disable the external amplifier
	
RETURNS:
    none
*/    
static void EnableI2sDevice(bool enable)
{
    
    uint8 enable_pio = I2S_config->i2s_init_config.enable_pio;
    /*check whther the pio is valid*/
    if(enable_pio != PIN_INVALID)
    {	
        PanicFalse(PioCommonSetPio(enable_pio,pio_drive,enable));	
    }
    
}

/****************************************************************************
DESCRIPTION: AudioI2SInitialiseDevice :

    This function configures the I2S device 

PARAMETERS:
    
    uint32 sample_rate - sample rate of data coming from dsp

RETURNS:
    none
*/    
void AudioI2SInitialiseDevice(uint32 rate)
{   
	/*Enable external i2s amplifier independent of the type of amplifier*/
    SetExternalAmpState(active);
	switch(I2S_config->i2s_init_config.plugin_type)
	{
	    case i2s_plugin_none_use_pskey:
		    AudioI2SInitialiseDevice_PsKey();
		break;
		
		case i2s_plugin_ssm2518:
		    AudioI2SInitialise_SSM2518(rate);
		break;
		
		case i2s_plugin_csra6620:
		    AudioI2SInitialise_CSRA6620(rate);
		break;
		
		default:
		    PRINT(("Can't intialise an unsupported I2S plugin/device\n"));
		break;
	}
}

/****************************************************************************
DESCRIPTION: AudioShutdownI2SDevice :

    This function shuts down the I2S device 

PARAMETERS:
    
    none
    
RETURNS:
    none
*/    
void AudioShutdownI2SDevice(void)
{
    uint16 ack = 0;
    /* determine I2S plugin type */
    switch(I2S_config->i2s_init_config.plugin_type)
    {
        case i2s_plugin_none_use_pskey:
            /* use the configuration information retrieved from ps or constant if no ps */
            if((I2S_config->i2s_init_config.i2s_configuration_command_pskey_length)&&
               (I2S_config->i2s_init_config.number_of_shutdown_cmds))
            {
                /* configuration data available */
                uint8 i;                
                uint8* shutdown_cmd = I2S_config->i2s_data_config.data + I2S_config->i2s_init_config.shutdown_cmds_offset;
                
                /* cycle through the configuration messages */
                for(i = 0;i < I2S_config->i2s_init_config.number_of_shutdown_cmds; i++)
                {
#ifdef DEBUG_PRINT_ENABLED
                    uint8 j;

                    PRINT(("I2S: Shutdown Msg Addr=0x%04x",shutdown_cmd[PACKET_I2C_ADDR]));
                    PRINT((" Write Reg=0x%02x,",shutdown_cmd[PACKET_DATA]));
                    for(j=1;j<(shutdown_cmd[PACKET_LENGTH]-1U);j++)
                    {    
                        PRINT((" 0x%02x", shutdown_cmd[PACKET_DATA + j]));
                    }
                    PRINT(("\n"));                         
#endif                    

                    /* send out packets */
                    ack = I2cTransfer(shutdown_cmd[PACKET_I2C_ADDR], &shutdown_cmd[PACKET_DATA], (uint16)(shutdown_cmd[PACKET_LENGTH] - 1), NULL, 0);
                    PanicZero(ack);
                    /* move to next packet */
                    shutdown_cmd += (2 + shutdown_cmd[PACKET_LENGTH]);
                }
            }
            /* no configuration data available so no need to send any i2c commands */
            /* Disable external amplifier */
            SetExternalAmpState(power_off);
        break;
        
        case i2s_plugin_ssm2518:
            /* Disable external amplifier */
            SetExternalAmpState(power_off);
        break;
        case i2s_plugin_csra6620:
            AudioI2SShutdown_CSRA6620();
        break;
        
        default:
            PRINT(("Can't shutdown an unsupported I2S plugin/device\n"));  
        break;
    }
}

/******************************************************************************
DESCRIPTION: AudioI2SSetChannelVolume

    This function sets the volume of a single I2S channel on a specific I2S
    device via the I2C interface. The volume can either be passed in as a value
    in 1/60th's of dB (with range -7200 to 0), or as an absolute value in the 
    same format used by the CODEC plugin, for compatibility (range 0x0 - 0xf).

PARAMETERS:
    i2s_out_t channel   The I2S device and channel to set the volume of.
    int16 vol           The volume level required, in dB/60 or CODEC_STEPS.
    bool volume_in_dB   Set to TRUE if volume passed in dB/60, FALSE otherwise.

RETURNS:
    Whether volume was successfully changed for the requested device channel.
*/
bool AudioI2SSetChannelVolume(i2s_out_t channel, int16 vol, bool volume_in_dB)
{
    uint8  i;
    uint32 volume;  /* Working variable to scale and shift volume */
    uint16 range;   /* Working variable to store volume range and remainders */
    uint8* vol_cmd; /* Pointer to I2C command and address information from PS */
    uint8* packet;  /* Data to send over I2C, based on PS info and scaled vol */
    
    bool inverted_range;
    
    if ((I2S_config->i2s_init_config.i2s_configuration_command_pskey_length <= 0) ||
        (I2S_config->i2s_init_config.number_of_volume_cmds <= channel))
    {
        /* There isn't a command for the requested channel. */
        return FALSE;
    }
    else
	{
        /* Ensure I2S device is enabled if CsrSetVolumeI2SChannel called before I2S is initialised. */
        SetExternalAmpState(active);
    }
    
    /* Set vol_cmd to point to first volume command in the command table. */
    vol_cmd = I2S_config->i2s_data_config.data + I2S_config->i2s_init_config.volume_cmds_offset;
    
    /* Adjust to point to the correct command (row) for the channel requested. */
    for (i = 0; i < channel; i++)
    {
        vol_cmd += (2 + vol_cmd[PACKET_LENGTH]);    /* First 2 bytes are unaccounted for in PACKET_LENGTH */
    }
    
    /* Check new command pointed to is valid */
    if (vol_cmd[PACKET_LENGTH] == 0)
    {
        /* Invalid command (zero length) or blank command for this channel. */
        return FALSE;
    }
    
    PRINT(("I2S: SetVol %d", vol));
    
    /*! The input vol needs to be scaled to the I2S amp range, which is a variable calculated from the min/max (uint16) values
        in I2S_INIT_CONFIG. The scale factor is (amp range / input vol range). The amp range can be up to 16-bit, so the first
        step in the calculation (input vol * amp range) is potentially a 32-bit number. However, performing division of 32-bit
        numbers on the XAP is very stack intensive, unless the divisor is a power of 2 (in which case bit shifts can be used in
        place of full division). The input vol range is either 15 or 7200, neither of which are powers of 2, so the scaling is
        instead done in two stages to avoid full division, and thus avoid high stack usage.
        
        First, the range is adjusted to 0-65536, so that the later division by 65536 (2^16) can be performed using a bit shift.
        So long as the scaling and rounding are carried out with reasonable accuracy, no resolution is lost as the min/max amp
        volume values cannot exceed 65535 (2^16 - 1). Since this first scaling can be done with constants (either 65536/15 or
        65536/7200, i.e. 4369.07 or 9.10222 respectively), rather than with variable ranges, the calculation can be optimised
        such that the conversion itself doesn't use full division either (by using an approximately equal fraction and ensuring
        that the denominator is a power of 2). The two possible overall scale factors are then (4369.07 * (amp range / 65536))
        or (9.10222 * (amp range / 65536)), depending on whether the input volume is in dB/60 or not.
    */
    if (volume_in_dB)
    {
        PRINT((" dB/60"));
        
        /* Impose hard limits on input volume */
        if (vol > (int16)MAXIMUM_DIGITAL_VOLUME_0DB)
            vol = (int16)MAXIMUM_DIGITAL_VOLUME_0DB;   /* 0 (0dB) */
        
        if (vol < (int16)DIGITAL_VOLUME_MUTE)
            vol = (int16)DIGITAL_VOLUME_MUTE;          /* -7200 (-120dB) */
        
        /* If using dB scaling, volume needs to be shifted up to positive numbers. */
        vol = (int16)(vol - DIGITAL_VOLUME_MUTE);
        
        /*! Input vol is now in the range 0-7200. To scale to 0-65536 as explained above, this needs to be multiplied by 9.10222.
            Denominator must be a power of 2, so scale by 149131/16384 (= 9.10223), which is plenty accurate enough to ensure no
            no loss of input resolution (with proper rounding).
        */
        #define DB_SCALE_FACTOR_NUMERATOR   149131
        #define DB_SCALE_FACTOR_DENOMINATOR  16384
        
        volume = (uint32)vol * DB_SCALE_FACTOR_NUMERATOR;
        range  = volume % DB_SCALE_FACTOR_DENOMINATOR;  /* Use range as temp variable to store remainder before division */
        volume = volume / DB_SCALE_FACTOR_DENOMINATOR;  /* Rounds down by default */
        
        if (range >= DB_SCALE_FACTOR_DENOMINATOR/2) /* If remainder >= divisor/2, need to round up */
            volume ++;
    }
    else
    {
        PRINT(("/%d", CODEC_STEPS));
        
        /* Impose hard limits on input volume */
        if (vol > CODEC_STEPS)
            vol = CODEC_STEPS;
        
        if (vol < 0)
            vol = 0;
        
        /*! Input vol is now in the range 0-15. To scale to 0-65536 as explained above, this needs to be multiplied by 4369.07.
            Denominator must be a power of 2, so scale by 69905/16 (= 4369.06), which is plenty accurate enough to ensure no
            no loss of input resolution (with proper rounding).
        */
        #define CODEC_SCALE_FACTOR_NUMERATOR    69905
        #define CODEC_SCALE_FACTOR_DENOMINATOR     16
        
        volume = (uint32)(vol * CODEC_SCALE_FACTOR_NUMERATOR);
        range  = volume % CODEC_SCALE_FACTOR_DENOMINATOR;   /* Use range as temp variable to store remainder before division */
        volume = volume / CODEC_SCALE_FACTOR_DENOMINATOR;   /* Rounds down by default */
        
        if (range >= CODEC_SCALE_FACTOR_DENOMINATOR/2)  /* If remainder >= divisor/2, need to round up */
            volume ++;
    }
    
    PRINT(("\n"));
    
    /* Allocate packet (excludes PACKET_I2C_ADDR, so subtract 1 from PACKET_LENGTH) */
    packet = PanicUnlessMalloc((size_t)(vol_cmd[PACKET_LENGTH]-1));
    
    /* Copy command data into packet */
    memcpy(packet, &vol_cmd[PACKET_DATA], (size_t)(vol_cmd[PACKET_LENGTH]-1)); 
    
    /*! Volume is now in the range 0-65536. Need to scale and shift to amp range so multiply by (amp range / 65536) and round appropriately. */
    
    /* First calculate external amplifier range */
    if (I2S_config->i2s_init_config.volume_range_max < I2S_config->i2s_init_config.volume_range_min)
    {
        /* Inverted range */
        range = (uint16)(I2S_config->i2s_init_config.volume_range_min - I2S_config->i2s_init_config.volume_range_max);
        inverted_range = 1;
    }
    else
    {
        /* Non-inverted range */
        range = (uint16)(I2S_config->i2s_init_config.volume_range_max - I2S_config->i2s_init_config.volume_range_min);
        inverted_range = 0;
    }
    
    /* Scale by range/65536 */
    volume = volume * range;
    range  = volume % 65536;    /* Re-use range as temp variable to store remainder before division */
    volume = volume / 65536;    /* Rounds down by default */
    
    /* Round up if (remainder >= divisor/2), or if (remainder > divisor/2) for inverted ranges (by adding 1) */
    if (range >= (65536/2 + inverted_range))
        volume++;
    
    /* Shift result in case of non-zero range start */
    if (inverted_range)
        volume = (I2S_config->i2s_init_config.volume_range_min - volume);
    else
        volume = (I2S_config->i2s_init_config.volume_range_min + volume);
    
    PRINT(("I2S: Scaled vol [%lu] of amp range min [%u] to max [%u]\n",
            (unsigned long) volume,
            I2S_config->i2s_init_config.volume_range_min,
            I2S_config->i2s_init_config.volume_range_max));
    
    /* Insert volume information into packet to complete the command */
    if (I2S_config->i2s_init_config.volume_no_of_bits <= 8)
    {
        /* 8 bit command - Replace a single byte of volume data */
        packet[vol_cmd[PACKET_VOLUME_OFFSET]] = (volume & 0xff);
    }
    else if (I2S_config->i2s_init_config.volume_no_of_bits <= 16)
    {
        /* 16 bit command - Replace two bytes of volume data */
        packet[vol_cmd[PACKET_VOLUME_OFFSET]]     = ((volume >> 8) & 0xff);
        packet[vol_cmd[PACKET_VOLUME_OFFSET] + 1] = ( volume       & 0xff);
    }
    else if (I2S_config->i2s_init_config.volume_no_of_bits <= 24)
    {
        /* 24 bit command - Replace three bytes of volume data */
        packet[vol_cmd[PACKET_VOLUME_OFFSET]]     = ((volume >> 16) & 0xff);    /* Note that due to 16-bit limit on amp min/max volume in PS, this will be 0 */
        packet[vol_cmd[PACKET_VOLUME_OFFSET] + 1] = ((volume >> 8)  & 0xff);
        packet[vol_cmd[PACKET_VOLUME_OFFSET] + 2] = ( volume        & 0xff);
    }
    else if (I2S_config->i2s_init_config.volume_no_of_bits <= 32)
    {
        /* 32 bit command - Replace four bytes of volume data */
        packet[vol_cmd[PACKET_VOLUME_OFFSET]]     = (uint8)((volume >> 24) & 0xff);    /* Note that due to 16-bit limit on amp min/max volume in PS, this will be 0 */
        packet[vol_cmd[PACKET_VOLUME_OFFSET] + 1] = ((volume >> 16) & 0xff);    /* Note that due to 16-bit limit on amp min/max volume in PS, this will be 0 */
        packet[vol_cmd[PACKET_VOLUME_OFFSET] + 2] = ((volume >> 8)  & 0xff);
        packet[vol_cmd[PACKET_VOLUME_OFFSET] + 3] = ( volume        & 0xff);
    }
    else
    {
        PRINT(("I2S: Error - Invalid number of bits for volume command"));
        Panic();
    }
    
#ifdef DEBUG_PRINT_ENABLED
    PRINT(("I2S: Vol Msg 0x%02x", vol_cmd[PACKET_I2C_ADDR]));
    for (i = 0; i < (vol_cmd[PACKET_LENGTH] - 1); i++)
    {
        PRINT((" 0x%02x", packet[i]));
    }
    PRINT(("\n"));
#endif
    
    /* Send completed volume command packet over I2C, PACKET_LENGTH includes ID byte so subtract 1 */
    PanicFalse(I2cTransfer(vol_cmd[PACKET_I2C_ADDR], packet, (uint16)(vol_cmd[PACKET_LENGTH] - 1), NULL, 0));
    
    /* Dispose of temporary malloc'd memory */
    free(packet);
    packet = NULL;
    
    return TRUE;
}

/******************************************************************************
DESCRIPTION: AudioI2SSetDeviceVolume

    This function sets the primary I2S device volume via the I2C interface, for
    use in stereo mode.

PARAMETERS:
    int16 left_vol    - Volume level for primary left channel.
    int16 right_vol   - Volume level for primary left channel.
    bool volume_in_dB - Whether the volume is passed in in dB or CODEC_STEPS.

RETURNS:
    none
*/
void AudioI2SSetDeviceVolume(int16 left_vol, int16 right_vol, bool volume_in_dB)
{
    /* determine i2s plugin type in use */
    switch(I2S_config->i2s_init_config.plugin_type)
    {
        case i2s_plugin_none_use_pskey:
        {
            AudioI2SSetChannelVolume(i2s_out_1_left, left_vol, volume_in_dB);
            AudioI2SSetChannelVolume(i2s_out_1_right, right_vol, volume_in_dB);
        }
        break;
        
        case i2s_plugin_ssm2518:
        {
            /* If using dB scaling, SSM2518 plugin expects volumes to be scaled to a base
               of 2 (originally to reduce stack usage). Specifically, a range of 0-1024.
               Volume is converted from -1/60dB to 1/10dB * 0.853 which makes full scale
               (-120dB) equivalent to 1024 counts, which simplifies the maths required to
               generate scaled volume levels to be sent over the I2C interface */
            if(volume_in_dB)
            {
                PRINT(("I2S: SetVol dB[%x] ", left_vol));
                
                /* reduce resolution of dB values from 1/60th to 1/10th */
                left_vol /= 6;
                right_vol /= 6;
                /* multiply by 0.8533 to reduce to the range of 0 to 1024 */
                left_vol = (int16)(((0 - left_vol) * 17)/20);
                right_vol = (int16)(((0 - right_vol) * 17)/20);
                
                PRINT(("scaled[%x]\n", left_vol));
            }
            
            AudioI2SSetVolume_SSM2518(left_vol, right_vol, volume_in_dB);
        }
        
        case i2s_plugin_csra6620:
        {
            AudioI2SSetVolume_CSRA6620(left_vol, right_vol, volume_in_dB);
        }
        break;
        
        default:
            PRINT(("Can't Set volume on unknown I2S plugin/device\n"));
        break;
    }
}

/****************************************************************************
DESCRIPTION: AudioI2SMusicResamplingFrequency

    This function returns the current resampling frequency for music apps,

PARAMETERS:
    
    none

RETURNS:
    frequency or 0 indicating no resampling required
*/    
uint16 AudioI2SMusicResamplingFrequency(void)
{
    if (I2S_config)
    {
        return I2S_config->i2s_init_config.music_resampling_frequency;
    }
    return I2S_NO_RESAMPLE;
}

uint32 AudioI2SGetOutputResamplingFrequencyForI2s(const uint32 requested_rate)
{
    uint32 rate = AudioI2SMusicResamplingFrequency();
    if(rate == I2S_NO_RESAMPLE)
    {
        rate = requested_rate;

        if(rate < I2S_MINIMUM_SUPPORTED_OUTPUT_SAMPLE_RATE)
        {
            rate = I2S_FALLBACK_OUTPUT_SAMPLE_RATE;
        }
    }
    return rate;
}

/****************************************************************************
DESCRIPTION: AudioI2SVoiceResamplingFrequency

    This function returns the current resampling frequency for voice apps,

PARAMETERS:
    
    none

RETURNS:
    frequency or 0 indicating no resampling required
*/    
uint16 AudioI2SVoiceResamplingFrequency(void)
{
    if (I2S_config)
    {
        return I2S_config->i2s_init_config.voice_resampling_frequency;
    }
    return I2S_NO_RESAMPLE;
}

/****************************************************************************
DESCRIPTION: AudioI2SConfigureSource

    This function configures the supplied source

PARAMETERS:

    Source

RETURNS:
    none
*/
void AudioI2SConfigureSource(Source source, uint32 rate, uint16 bit_resolution)
{
    uint16 I2S_audio_sample_size = RESOLUTION_MODE_16BIT;

    /* configure the I2S interface operating mode, run in master or slave mode */
    PanicFalse(SourceConfigure(source, STREAM_I2S_MASTER_MODE, I2S_config->i2s_init_config.master_operation));
    
    /* set the sample rate of the dsp audio data */
    PanicFalse(SourceConfigure(source, STREAM_I2S_MASTER_CLOCK_RATE, (rate * I2S_config->i2s_init_config.bit_clock_scaling_factor)));

    /* set the sample rate of the dsp audio data */
    PanicFalse(SourceConfigure(source, STREAM_I2S_SYNC_RATE, rate));
              
    /* left justified or i2s data */
    PanicFalse(SourceConfigure(source, STREAM_I2S_JSTFY_FORMAT, I2S_config->i2s_init_config.left_or_right_justified));
     
    /* MSB of data occurs on the second SCLK */
    PanicFalse(SourceConfigure(source, STREAM_I2S_LFT_JSTFY_DLY, I2S_config->i2s_init_config.justified_bit_delay));

    /* AUDIO_CHANNEL_SLOT_0 = left, AUDIO_CHANNEL_SLOT_1 = right */
    PanicFalse(SourceConfigure(source, STREAM_I2S_CHNL_PLRTY, POLARITY_RIGHT_WHEN_WS_HIGH));
     
    /* number of data bits per sample, 16 */
    PanicFalse(SourceConfigure(source, STREAM_I2S_BITS_PER_SAMPLE, I2S_config->i2s_init_config.bits_per_sample));

    if(I2S_config->i2s_init_config.bits_per_sample >= bit_resolution)
    {
        I2S_audio_sample_size = bit_resolution;
    }

    /* Specify the bits per sample for the audio input */
   PanicFalse(SourceConfigure(source, STREAM_AUDIO_SAMPLE_SIZE, I2S_audio_sample_size));
   
    /* configure MCLK for the interface */
    AudioI2SSourceConfigureMasterClockIfRequired(source, rate);
}


/****************************************************************************
DESCRIPTION: AudioI2SMasterIsEnabled

    This function returns the I2S operation mode 

PARAMETERS:
    
    none

RETURNS:
    
    TRUE : Operation mode is I2S Master
    FALSE : Operation mode is I2S Slave 
    
*/    
bool AudioI2SMasterIsEnabled(void)
{
    if(I2S_config->i2s_init_config.master_operation)
        return TRUE;
    else
        return FALSE;
}

/****************************************************************************
DESCRIPTION: AudioI2SIs24BitAudioInputEnabled

    This function returns true if I2S input audio is of 24 bit resolution 

PARAMETERS:
    
    none

RETURNS:
    
    TRUE : I2S Audio input is 24 bit enabled
    FALSE : I2S Audio input is 16 bit enabled. 
    
*/
bool AudioI2SIs24BitAudioInputEnabled(void)
{
    if(I2S_config->i2s_init_config.enable_24_bit_audio_input)
        return TRUE;
    else
        return FALSE;
}

/****************************************************************************
DESCRIPTION: AudioI2SIsMclkRequired :

    This function returns true if I2S has been configured to output an MCLK signal

PARAMETERS:

    none

RETURNS:

    TRUE : I2S MCLK required
    FALSE : I2S MCLK not required
*/
bool AudioI2SIsMasterClockRequired(void)
{
    if (I2S_config->i2s_init_config.master_clock_scaling_factor > 0)
        return TRUE;
    else
        return FALSE;
}

/****************************************************************************
DESCRIPTION: GetExternalAmpCurrentState

This function returns the state of an external amplifier

PARAMETERS:
	none

RETURNS:
    external_amp_state : State of external amplifier
*/
static external_amp_state GetExternalAmpCurrentState(void)
{
    return external_amp.state;
}

/****************************************************************************
DESCRIPTION: SetExternalAmpState

This function sets the state of an external amplifier

PARAMETERS:
    external_amp_state state: State of external amplifier to be changed

RETURNS:
	none
*/
static void SetExternalAmpState(external_amp_state requested_state)
{
    if(GetExternalAmpCurrentState() != requested_state)
    {
        switch (requested_state)
        {
            case active:
                EnableI2sDevice(TRUE);
                external_amp.state = active;
            break;

            case standby:
            break;

            case power_off:
                EnableI2sDevice(FALSE);
                external_amp.state = power_off;
            break;

            default:
            break;
        }
    }
}
