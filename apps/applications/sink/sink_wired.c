/*
Copyright (c) 2005 - 2018 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief
    Application level implementation of Wired Sink features

NOTES
    - Conditional on ENABLE_WIRED define
*/

#include "sink_main_task.h"
#include "sink_debug.h"
#include "sink_wired.h"
#include "sink_audio.h"
#include "sink_pio.h"
#include "sink_powermanager.h"
#include "sink_statemanager.h"
#include "sink_tones.h"
#include "sink_display.h"
#include "sink_audio_routing.h"
#include "sink_volume.h"
#include "sink_config.h"
#include "sink_ba_broadcaster.h"
#include "sink_ba.h"
#include "audio_plugin_if.h"
#include "sink_malloc_debug.h"

#ifdef ENABLE_SUBWOOFER
#include "sink_swat.h"
#endif

#include <panic.h>
#include <stream.h>
#include <source.h>
#include <sink.h>
#include <string.h>
#include <audio_output.h>
#include <ps.h>
#include <config_store.h>
#include <audio_config.h>
#include <audio_i2s_common.h>
#include <pio_common.h>

#ifdef ENABLE_WIRED


#include <audio_plugin_music_params.h>
#include <audio_plugin_music_variants.h>
#include <audio_plugin_output_variants.h>

#include "sink_wired_config_def.h"
#ifdef DEBUG_WIRED
    #define WIRED_DEBUG(x) DEBUG(x)
#else
    #define WIRED_DEBUG(x)
#endif

/* Wired PIOs */
#define PIO_ANALOG_DETECT (SinkWiredGetAnalogInput())
#define PIO_SPDIF_DETECT (SinkWiredGetSpdifDetect())
#define PIO_I2S_DETECT (SinkWiredGetI2sDetect())
#define PIO_WIRED_SELECT    (PIO_POWER_ON)
#define PIO_SPDIF_INPUT (SinkWiredGetSpdifInput())
#define PIO_SPDIF_OUTPUT (SinkWiredGetSpdifOutput())
#define PIN_WIRED_ALWAYSON  (PIN_INVALID)     /* Input PIO is disabled, always assumed on */
#define PIN_WIRED_DISABLED  (0xFE)            /* Entire input type is disabled */

/* Wired audio params */
#ifdef FORCE_ANALOGUE_INPUT
    #define ANALOG_CONNECTED  (TRUE)
    #define ANALOG_READY      (TRUE)
    #define SPDIF_CONNECTED   (FALSE)
    #define SPDIF_READY       (FALSE)
    #define I2S_CONNECTED     (FALSE)
    #define I2S_READY         (FALSE)
#else
    /* If the Detect PIO for S/PDIF or analog wired input is set to N/A (0xff), assume always connected */
    #define ANALOG_CONNECTED     (PIO_ANALOG_DETECT == PIN_WIRED_ALWAYSON?TRUE:!PioGetPio(PIO_ANALOG_DETECT))
    #define ANALOG_READY         (PIO_ANALOG_DETECT !=PIN_WIRED_DISABLED)
    #define SPDIF_CONNECTED      (PIO_SPDIF_DETECT == PIN_WIRED_ALWAYSON?TRUE:!PioGetPio(PIO_SPDIF_DETECT))
    #define SPDIF_READY          (PIO_SPDIF_DETECT !=PIN_WIRED_DISABLED)
    #define I2S_CONNECTED        (PIO_I2S_DETECT == PIN_WIRED_ALWAYSON?TRUE:!PioGetPio(PIO_I2S_DETECT))
    #define I2S_READY            (PIO_I2S_DETECT !=PIN_WIRED_DISABLED)
#endif

/* These values must never represent a "real" Sink.
 * Hydracore may have a wider range of possible Sink values. Although a clash is
 * considered unlikely, Hydracore values (pointers in most case) would be divisble
 * by 4. */
#define ANALOG_SINK      ((Sink)0xFFFF)
#define SPDIF_SINK       ((Sink)0xFFFE)
#define I2S_SINK         ((Sink)0xFFFD)

#define WIRED_RATE       (48000)

/*Sink Wired global data */
typedef struct __sink_wired_global_data_t
{
    spdif_ac3_configuration   spdif_ac3_config_data;
    spdif_configuration         spdif_config_data;
    volume_info                    analog_volume;
    volume_info                    spdif_volume;
    volume_info             i2s_volume;
    I2SConfiguration*            i2s_config_data;

    unsigned spdif_detect:8;
    unsigned analog_input:8;
    unsigned spdif_input:8;
    unsigned i2s_detect:8;
    unsigned spdif_output:8;

}sink_wired_global_data;

static sink_wired_global_data wired_global_data;

#define GWIRED (wired_global_data)

static uint32 getWiredRate(WIRED_SAMPLING_RATE_T wired_rate);

/****************************************************************************
NAME
    read_pio_config

DESCRIPTION
    Cache the read-only PIO configuration

RETURNS
    void
*/
static void read_pio_config(void)
{
    sink_wired_readonly_config_def_t *ro_data;

    (void) configManagerGetReadOnlyConfig(SINK_WIRED_READONLY_CONFIG_BLK_ID, (const void **)&ro_data);

    GWIRED.spdif_detect = ro_data->spdif_detect;
    GWIRED.analog_input = ro_data->analog_input;
    GWIRED.spdif_input = ro_data->spdif_input;
    GWIRED.i2s_detect = ro_data->i2s_detect;
    GWIRED.spdif_output = ro_data->spdif_output;

    configManagerReleaseConfig(SINK_WIRED_READONLY_CONFIG_BLK_ID);
}


/****************************************************************************
NAME
     SinkWiredReadI2SAndSpdifConfiguration

    DESCRIPTION
    Gets I2S and spdif pskey config and any associated I2S data packets

RETURNS
     void

*/
void SinkWiredReadI2SAndSpdifConfiguration(void)
{
    uint16 count;
    uint16 size;
    spdif_settings_config_def_t *ro_data;
    i2s_data_config_def_t *i2s_ro_data;

    memset(&GWIRED.spdif_config_data, 0, sizeof(GWIRED.spdif_config_data));
    memset(&GWIRED.i2s_config_data, 0, sizeof(GWIRED.i2s_config_data));


    /* Retrieve the SPDIF configuration & I2S Configuration data and update it in Wired Global data structure. */
    if (configManagerGetReadOnlyConfig(SPDIF_SETTINGS_CONFIG_BLK_ID, (const void**)&ro_data))
    {

        GWIRED.spdif_config_data.target_latency = ro_data->SpdifConfig.target_latency;
        GWIRED.spdif_config_data.enable_24_bit_audio_input = ro_data->SpdifConfig.enable_24_bit_audio;
        configManagerReleaseConfig(SPDIF_SETTINGS_CONFIG_BLK_ID);
    }

    size = configManagerGetReadOnlyConfig(I2S_DATA_CONFIG_BLK_ID, (const void**)&i2s_ro_data);
    if (size)
    {
        /* Convert the config data size (in uint16) to words */
        size *= sizeof(uint16);
        
        GWIRED.i2s_config_data = mallocPanic(size);
        
        memset(GWIRED.i2s_config_data, 0, size);
        
        GWIRED.i2s_config_data->i2s_init_config.volume_no_of_bits = i2s_ro_data->i2s_init_config.volume_no_of_bits;
        GWIRED.i2s_config_data->i2s_init_config.volume_range_max = i2s_ro_data->i2s_init_config.volume_range_max;
        GWIRED.i2s_config_data->i2s_init_config.volume_range_min = i2s_ro_data->i2s_init_config.volume_range_min;
        GWIRED.i2s_config_data->i2s_init_config.music_resampling_frequency = i2s_ro_data->i2s_init_config.music_resampling_frequency;
        GWIRED.i2s_config_data->i2s_init_config.voice_resampling_frequency = i2s_ro_data->i2s_init_config.voice_resampling_frequency;
        GWIRED.i2s_config_data->i2s_init_config.bit_clock_scaling_factor = i2s_ro_data->i2s_init_config.bit_clock_scaling_factor;
        GWIRED.i2s_config_data->i2s_init_config.master_clock_scaling_factor = i2s_ro_data->i2s_init_config.master_clock_scaling_factor;
        GWIRED.i2s_config_data->i2s_init_config.plugin_type = i2s_ro_data->i2s_init_config.plugin_type;
        GWIRED.i2s_config_data->i2s_init_config.master_operation = i2s_ro_data->i2s_init_config.master_operation;
        GWIRED.i2s_config_data->i2s_init_config.bits_per_sample = i2s_ro_data->i2s_init_config.bits_per_sample;
        GWIRED.i2s_config_data->i2s_init_config.i2s_configuration_command_pskey_length = size - sizeof(i2s_init_config_t_t);
        GWIRED.i2s_config_data->i2s_init_config.left_or_right_justified = i2s_ro_data->i2s_init_config.left_or_right_justified;
        GWIRED.i2s_config_data->i2s_init_config.justified_bit_delay = i2s_ro_data->i2s_init_config.justified_bit_delay;
        GWIRED.i2s_config_data->i2s_init_config.enable_24_bit_audio_input = i2s_ro_data->i2s_init_config.enable_24_bit_audio_input;
        GWIRED.i2s_config_data->i2s_init_config.enable_pio = i2s_ro_data->i2s_init_config.enable_pio;
        
        if(i2s_ro_data->i2s_init_config.plugin_type == i2s_plugin_none_use_pskey)	 
        {
            GWIRED.i2s_config_data->i2s_init_config.number_of_initialisation_cmds = i2s_ro_data->i2s_init_config.number_of_initialisation_cmds;
            GWIRED.i2s_config_data->i2s_init_config.number_of_volume_cmds = i2s_ro_data->i2s_init_config.number_of_volume_cmds;
            GWIRED.i2s_config_data->i2s_init_config.volume_cmds_offset = i2s_ro_data->i2s_init_config.volume_cmds_offset;
            GWIRED.i2s_config_data->i2s_init_config.number_of_shutdown_cmds = i2s_ro_data->i2s_init_config.number_of_shutdown_cmds;
            GWIRED.i2s_config_data->i2s_init_config.shutdown_cmds_offset = i2s_ro_data->i2s_init_config.shutdown_cmds_offset;
        }

        /* i2s_data_config_def_t contains a variable length array that may not contain any data */
        if (size >= sizeof(i2s_data_config_def_t))
        {
            for(count =0; count< (size - sizeof(i2s_data_config_def_t)); count++)
            {
                GWIRED.i2s_config_data->i2s_data_config.data[count] = (uint8) i2s_ro_data->data[count].word;
            }
        }
        configManagerReleaseConfig(I2S_DATA_CONFIG_BLK_ID);
    }

    /* Pass data to I2S library for future use */
    AudioI2SInitialise(GWIRED.i2s_config_data);
}

/****************************************************************************
NAME 
     getWiredRoConfig
    
    DESCRIPTION
    Gets the Sink Wired Read Only configuration
 
RETURNS

*/
static bool getWiredRoConfig(sink_wired_readonly_config_def_t* config)
{
    sink_wired_readonly_config_def_t *ro_config = NULL;
    uint16 size;

    size = configManagerGetReadOnlyConfig(SINK_WIRED_READONLY_CONFIG_BLK_ID, (const void **)&ro_config);

    *config = *ro_config;

    configManagerReleaseConfig(SINK_WIRED_READONLY_CONFIG_BLK_ID);

    return (size != 0);
}

/****************************************************************************
NAME 
     sinkWiredReadI2sAudioInstanceConfig
    
    DESCRIPTION
    Gets the Audio Instance for I2S from configuration
 
RETURNS
     audio_instance

*/
static audio_instance sinkWiredReadI2sAudioInstanceConfig(void)
{
    sink_wired_readonly_config_def_t config;
    audio_instance i2s_audio_instance = AUDIO_INSTANCE_1;

    if(getWiredRoConfig(&config))
    {
        i2s_audio_instance = config.i2s_audio_instance;
    }
    return i2s_audio_instance;
}

/****************************************************************************
NAME 
     sinkWiredReadSpdifAudioInstanceConfig
    
    DESCRIPTION
    Gets the Audio Instance for SPDIF from configuration
 
RETURNS
     audio_instance

*/
static audio_instance sinkWiredReadSpdifAudioInstanceConfig(void)
{
    sink_wired_readonly_config_def_t config;
    audio_instance spdif_audio_instance = AUDIO_INSTANCE_0;

    if(getWiredRoConfig(&config))
    {
        spdif_audio_instance = config.spdif_audio_instance;
    }

    return spdif_audio_instance;
}

/****************************************************************************
NAME
     SinkWiredGetAudioConnectedPowerOffTimeout

    DESCRIPTION
    Gets the Sink Wired Audio Connected PowerOff Timeout

RETURNS
     uint16

*/
uint16 SinkWiredGetAudioConnectedPowerOffTimeout(void)
{
    uint16 time_out = 0;
    sink_wired_readonly_config_def_t *ro_data;
    if (configManagerGetReadOnlyConfig(SINK_WIRED_READONLY_CONFIG_BLK_ID, (const void **)&ro_data))
    {
        time_out = ro_data->WiredAudioConnectedPowerOffTimeout_s;
        configManagerReleaseConfig(SINK_WIRED_READONLY_CONFIG_BLK_ID);
    }
    return time_out;
}

bool SinkWiredSetAudioConnectedPowerOffTimeout(uint16 timeout)
{
    sink_wired_readonly_config_def_t *ro_data;

    if (configManagerGetWriteableConfig(SINK_WIRED_READONLY_CONFIG_BLK_ID, (void **)&ro_data, 0))
    {
        ro_data->WiredAudioConnectedPowerOffTimeout_s = timeout;
        configManagerUpdateWriteableConfig(SINK_WIRED_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/****************************************************************************
NAME
    SinkWiredGetAnalogueDisconnectDebouncePeriod

DESCRIPTION
    Gets the Analogue Audio Disconnection debounce period, the time between
    physical disconection detection and destruction of the audio stream
    
RETURNS
    Debounce period in milliseconds

*/
uint16 SinkWiredGetAnalogueDisconnectDebouncePeriod(void)
{
    uint16 timeout = 0;
    sink_wired_readonly_config_def_t *ro_data;
    
    if (configManagerGetReadOnlyConfig(SINK_WIRED_READONLY_CONFIG_BLK_ID, (const void **)&ro_data))
    {
        timeout = ro_data->AnalogueAudioDisconnectDebounce_ms;
        configManagerReleaseConfig(SINK_WIRED_READONLY_CONFIG_BLK_ID);
    }
    
    return timeout;
}

/****************************************************************************
NAME
     SinkWiredGetSpdifAC3Settings

    DESCRIPTION
    Gets the Sink Wired SpdifAC3Settings

RETURNS
     spdif_ac3_configuration*

*/
spdif_ac3_configuration* SinkWiredGetSpdifAC3Settings(void)
{
    return &GWIRED.spdif_ac3_config_data;
}

/****************************************************************************
NAME
     SinkWiredGetSpdifConfig

    DESCRIPTION
    Gets the Sink Wired SpdifConfig

RETURNS
     spdif_configuration*

*/
spdif_configuration* SinkWiredGetSpdifConfig(void)
{
    return &GWIRED.spdif_config_data;
}

/****************************************************************************
NAME
     SinkWiredIsPowerOffOnWiredAudioConnected()

    DESCRIPTION
    Gets the Sink Wired PowerOff On WiredAudio Connected

RETURNS
     bool

*/
bool SinkWiredIsPowerOffOnWiredAudioConnected(void)
{
    bool result = FALSE;
    sink_wired_readonly_config_def_t *ro_data;
    if (configManagerGetReadOnlyConfig(SINK_WIRED_READONLY_CONFIG_BLK_ID, (const void **)&ro_data))
    {
        result = ro_data->PowerOffOnWiredAudioConnected;
        configManagerReleaseConfig(SINK_WIRED_READONLY_CONFIG_BLK_ID);
    }
    return result;
}

/****************************************************************************
NAME
     SinkWiredPlayUsbAndWiredInLimbo()

    DESCRIPTION
    Gets the Sink Wired Play Usb And Wired In Limbo

RETURNS
     bool

*/
bool SinkWiredPlayUsbAndWiredInLimbo(void)
{
    bool result = FALSE;
    sink_wired_readonly_config_def_t *ro_data;
    if (configManagerGetReadOnlyConfig(SINK_WIRED_READONLY_CONFIG_BLK_ID, (const void **)&ro_data))
    {
        result = ro_data->PlayUsbWiredInLimbo;
        configManagerReleaseConfig(SINK_WIRED_READONLY_CONFIG_BLK_ID);
    }
    return result;
}

/****************************************************************************
NAME
     SinkWiredReadSpdifAudioMaxInputRate()

    DESCRIPTION
    Reads the Sink Audio maximum Input Rate for spdif

RETURNS
     WIRED_SAMPLING_RATE_T

*/
static WIRED_SAMPLING_RATE_T sinkWiredReadSpdifAudioMaxInputRate(void)
{
    WIRED_SAMPLING_RATE_T rate = NON_DEFINED;
    sink_wired_readonly_config_def_t *ro_data;
    if(configManagerGetReadOnlyConfig(SINK_WIRED_READONLY_CONFIG_BLK_ID, (const void **)&ro_data))
    {
        rate = ro_data ->max_sample_rate;
        configManagerReleaseConfig(SINK_WIRED_READONLY_CONFIG_BLK_ID);
    }
    return rate;
 
}

/****************************************************************************
NAME
     SinkWiredGetAudioInputRate()

    DESCRIPTION
    Gets the Sink Wired Audio Input Rate

RETURNS
     WIRED_SAMPLING_RATE_T

*/
WIRED_SAMPLING_RATE_T SinkWiredGetAudioInputRate(void)
{
    WIRED_SAMPLING_RATE_T rate = NON_DEFINED;
    sink_wired_readonly_config_def_t *ro_data;
    if (configManagerGetReadOnlyConfig(SINK_WIRED_READONLY_CONFIG_BLK_ID, (const void **)&ro_data))
    {
        rate = ro_data->wired_audio_input_rate;
        configManagerReleaseConfig(SINK_WIRED_READONLY_CONFIG_BLK_ID);
    }
    return rate;
}

/****************************************************************************
NAME
     SinkWiredGetAudioOutputRate()

    DESCRIPTION
    Gets the Sink Wired Audio Outpur Rate

RETURNS
     WIRED_SAMPLING_RATE_T

*/
WIRED_SAMPLING_RATE_T SinkWiredGetAudioOutputRate(void)
{
    WIRED_SAMPLING_RATE_T rate = NON_DEFINED;
    sink_wired_readonly_config_def_t *ro_data;
    if (configManagerGetReadOnlyConfig(SINK_WIRED_READONLY_CONFIG_BLK_ID, (const void **)&ro_data))
    {
        rate = ro_data->wired_audio_output_rate;
        AudioConfigSetWiredAudioOutputSampleRate(getWiredRate(rate));
        configManagerReleaseConfig(SINK_WIRED_READONLY_CONFIG_BLK_ID);
    }
    return rate;
}

/****************************************************************************
NAME
     SinkWiredGetSpdifDetect()

    DESCRIPTION
    Gets the Sink Wired Audio Spdif Detect

RETURNS
     uint8

*/
uint8 SinkWiredGetSpdifDetect(void)
{
    return GWIRED.spdif_detect;
}

/****************************************************************************
NAME
     SinkWiredGetAnalogInput()

    DESCRIPTION
    Gets the Sink Wired Audio Analog Input

RETURNS
     uint8

*/
uint8 SinkWiredGetAnalogInput(void)
{
    return GWIRED.analog_input;
}

/****************************************************************************
NAME
     SinkWiredGetSpdifInput()

    DESCRIPTION
    Gets the Sink Wired Audio Spdif Input

RETURNS
     uint8

*/
uint8 SinkWiredGetSpdifInput(void)
{
    return GWIRED.spdif_input;
}

/****************************************************************************
NAME
     SinkWiredGetI2sDetect()

    DESCRIPTION
    Gets the Sink Wired Audio I2s Detect

RETURNS
     uint8

*/
uint8 SinkWiredGetI2sDetect(void)
{
    return GWIRED.i2s_detect;
}

/****************************************************************************
NAME 
     SinkWiredGetI2sPluginType()
    
    DESCRIPTION
    Gets the Sink Wired Audio I2s Plugin type (pskey configured or SSM2518)
 
RETURNS
     i2s_plugin_type

*/
uint8 SinkWiredGetI2sPluginType(void)
{
    return GWIRED.i2s_config_data->i2s_init_config.plugin_type;
}

/****************************************************************************
NAME 
     SinkWiredGetSpdifOutput()

    DESCRIPTION
    Gets the Sink Wired Audio Spdif Output

RETURNS
     uint8

*/
uint8 SinkWiredGetSpdifOutput(void)
{
    return GWIRED.spdif_output;
}

/****************************************************************************
NAME
	SinkWiredGetSessionData

DESCRIPTION
    Get Sink Wired Session data from PS Key and update the Sink Wired Global Data
*/
void SinkWiredGetSessionData(void)
{
    sink_wired_writeable_config_def_t *rw_config_data;
    if (configManagerGetReadOnlyConfig(SINK_WIRED_WRITEABLE_CONFIG_BLK_ID, (const void**)&rw_config_data))
    {
        GWIRED.analog_volume.main_volume = rw_config_data->analog_volume.main_volume;
        GWIRED.analog_volume.aux_volume = rw_config_data->analog_volume.aux_volume;
        GWIRED.spdif_volume.main_volume = rw_config_data->spdif_volume.main_volume;
        GWIRED.spdif_volume.aux_volume = rw_config_data->spdif_volume.aux_volume;
        GWIRED.i2s_volume.main_volume = rw_config_data->i2s_volume.main_volume;
        GWIRED.i2s_volume.aux_volume = rw_config_data->i2s_volume.aux_volume;
        configManagerReleaseConfig(SINK_WIRED_WRITEABLE_CONFIG_BLK_ID);
    }
}

/****************************************************************************
NAME
	SinkWiredSetSessionData

DESCRIPTION
    Get Sink Wired Global Data and update Sink Wired Session data in PS Key
*/
void SinkWiredSetSessionData(void)
{
    sink_wired_writeable_config_def_t *rw_config_data;
    if (configManagerGetWriteableConfig(SINK_WIRED_WRITEABLE_CONFIG_BLK_ID, (void**)&rw_config_data, 0))
    {
        rw_config_data->analog_volume.main_volume = GWIRED.analog_volume.main_volume;
        rw_config_data->analog_volume.aux_volume = GWIRED.analog_volume.aux_volume;
        rw_config_data->spdif_volume.main_volume = GWIRED.spdif_volume.main_volume;
        rw_config_data->spdif_volume.aux_volume = GWIRED.spdif_volume.aux_volume;
        rw_config_data->i2s_volume.main_volume = GWIRED.i2s_volume.main_volume;
        rw_config_data->i2s_volume.aux_volume = GWIRED.i2s_volume.aux_volume;
        configManagerUpdateWriteableConfig(SINK_WIRED_WRITEABLE_CONFIG_BLK_ID);
    }
}

/*************************************************************************
NAME
    SinkWiredGetAnalogVolume

DESCRIPTION
    Get the Analog volume.

RETURNS
    void

**************************************************************************/
void SinkWiredGetAnalogVolume(volume_info *analog_volume)
{
    *analog_volume = GWIRED.analog_volume;
}

/*************************************************************************
NAME
    SinkWiredSetAnalogVolume

DESCRIPTION
    Set the Analog volume.

RETURNS
    void

**************************************************************************/
void SinkWiredSetAnalogVolume(const volume_info analog_volume)
{
    GWIRED.analog_volume = analog_volume;
}

/*************************************************************************
NAME
    SinkWiredGetSpdifVolume

DESCRIPTION
    Get the Spdif volume.

RETURNS
    void

**************************************************************************/
void SinkWiredGetSpdifVolume(volume_info *spdif_volume)
{
    *spdif_volume = GWIRED.spdif_volume;
}

/*************************************************************************
NAME
    SinkWiredSetSpdifVolume

DESCRIPTION
    Set the Spdif volume.

RETURNS
    void

**************************************************************************/
void SinkWiredSetSpdifVolume(const volume_info spdif_volume)
{
    GWIRED.spdif_volume = spdif_volume;
}

/*************************************************************************
NAME
    SinkWiredGetI2sVolume

DESCRIPTION
    Get the I2S volume.

RETURNS
    void

**************************************************************************/
void SinkWiredGetI2sVolume(volume_info *i2s_volume)
{
    *i2s_volume = GWIRED.i2s_volume;
}

/*************************************************************************
NAME
    SinkWiredSetI2sVolume

DESCRIPTION
    Set the I2S volume.

RETURNS
    void

**************************************************************************/
void SinkWiredSetI2sVolume(const volume_info i2s_volume)
{
    GWIRED.i2s_volume = i2s_volume;
}

/*************************************************************************
NAME
    SinkWiredGetAnalogMainVolume

DESCRIPTION
    Get the Analog Main volume.

RETURNS
    int16

**************************************************************************/
int16 SinkWiredGetAnalogMainVolume(void)
{
    return GWIRED.analog_volume.main_volume;
}

/*************************************************************************
NAME
    SinkWiredGetSpdifMainVolume

DESCRIPTION
    Get the Spdif Main volume.

RETURNS
    int16

**************************************************************************/
int16 SinkWiredGetSpdifMainVolume(void)
{
    return GWIRED.spdif_volume.main_volume;
}

/*************************************************************************
NAME
    SinkWiredGetI2sMainVolume

DESCRIPTION
    Get the I2S Main volume.

RETURNS
    int16

**************************************************************************/
int16 SinkWiredGetI2sMainVolume(void)
{
    return GWIRED.i2s_volume.main_volume;
}

/****************************************************************************
NAME
    wiredAudioInit

DESCRIPTION
    Set up wired audio PIOs and configuration

RETURNS
    void
*/
void wiredAudioInit(void)
{
    read_pio_config();

    WIRED_DEBUG(("WIRE: Select %d\n", PIO_WIRED_SELECT));

    if(ANALOG_READY)
    {
        WIRED_DEBUG(("WIRE: analog Detect %d\n", PIO_ANALOG_DETECT));

        /* Pull detect high, audio jack will pull it low */
        if(PIO_ANALOG_DETECT != PIN_WIRED_ALWAYSON)
            PioSetPio(PIO_ANALOG_DETECT, pio_pull, TRUE);

        stateManagerAmpPowerControl(POWER_UP);
    }

    if(SPDIF_READY)
    {
        WIRED_DEBUG(("WIRE: spdif Detect %d\n", PIO_SPDIF_DETECT));

        if(PIO_SPDIF_DETECT != PIN_WIRED_ALWAYSON)
        {
            /* Pull detect high, audio jack will pull it low */
            PioSetPio(PIO_SPDIF_DETECT, pio_pull, TRUE);
        }
        stateManagerAmpPowerControl(POWER_UP);

        if((PIO_SPDIF_INPUT != PIN_WIRED_ALWAYSON) && (PIO_SPDIF_INPUT != PIN_WIRED_DISABLED))
        {
            /* configure SPDIF ports, required for CSR8675 */
            PioCommonConfigureSpdifInput(PIO_SPDIF_INPUT);
        }
        if(PIO_SPDIF_OUTPUT != PIN_INVALID)
        {
            /* configure SPDIF output port, required for CSR8675 */
            PioCommonConfigureSpdifOutput(PIO_SPDIF_OUTPUT);
        }
    }

    if(I2S_READY)
    {
        WIRED_DEBUG(("WIRE: I2S Detect %d\n", PIO_I2S_DETECT));

        /* Pull detect high, audio jack will pull it low */
        if(PIO_I2S_DETECT != PIN_WIRED_ALWAYSON)
            PioSetPio(PIO_I2S_DETECT, pio_pull, TRUE);

        stateManagerAmpPowerControl(POWER_UP);
    }
}

static uint32 getWiredRate(WIRED_SAMPLING_RATE_T wired_rate)
{
    uint32 rate = WIRED_RATE;

    switch(wired_rate)
    {
        case WIRED_RATE_44100:
            rate = 44100;
        break;

        case WIRED_RATE_48000:
            rate = 48000;
        break;
#ifdef HI_RES_AUDIO
        /* Hi Res audio rates are only supported on CSR8675 and not on CSR8670 */
        case WIRED_RATE_88200:
            rate = 88200;
        break;

        case WIRED_RATE_96000:
            rate = 96000;
        break;
#endif
        default:
            break;
    }

    return rate;
}

static void populateCommonConnectParams(audio_connect_parameters *connect_parameters)
{
    AudioPluginFeatures PluginFeatures = sinkAudioGetPluginFeatures();
    A2dpPluginConnectParams *audio_connect_params = getAudioPluginConnectParams();
    uint32 input_rate = getWiredRate(SinkWiredGetAudioInputRate());

    /* Make sure we're using correct parameters for Wired audio */
    audio_connect_params->mode_params = getAudioPluginModeParams();
    /* pass in the AC3 configuration */
    audio_connect_params->spdif_ac3_config = SinkWiredGetSpdifAC3Settings();
    /* pass in spdif configuration */
    audio_connect_params->spdif_config = SinkWiredGetSpdifConfig();

    /* Read 24 bit configuration from ps key*/
    audio_connect_params->wired_audio_output_rate = getWiredRate(SinkWiredGetAudioOutputRate());

#ifdef ENABLE_SUBWOOFER
    /* set the sub woofer link type prior to passing to audio connect */
    audio_connect_params->sub_woofer_type  = AUDIO_SUB_WOOFER_NONE;
    audio_connect_params->sub_sink  = NULL;
#endif

    WIRED_DEBUG(("WIRE: populateCommonConnectParams\n"));

    if(peerIsThisDeviceTwsMaster())
    {
        input_rate = WIRED_RATE;
    }

    peerPopulatePluginConnectData(&PluginFeatures, input_rate);

    connect_parameters->volume = TonesGetToneVolumeInDb(audio_output_group_main);
    connect_parameters->rate = input_rate;
    connect_parameters->features = PluginFeatures;
    connect_parameters->mode = AUDIO_MODE_CONNECTED;
    connect_parameters->route = AUDIO_ROUTE_INTERNAL;
    connect_parameters->power = powerManagerGetLBIPM();
    connect_parameters->params = audio_connect_params;
    connect_parameters->app_task = &theSink.task;
}


bool analoguePopulateConnectParameters(audio_connect_parameters *connect_parameters)
{
    WIRED_DEBUG(("ANALOG: Audio "));

    if(ANALOG_READY && ANALOG_CONNECTED)
    {
        WIRED_DEBUG(("Connected [0x%04X]\n", (uint16)ANALOG_SINK));
        populateCommonConnectParams(connect_parameters);

        connect_parameters->sink_type = AUDIO_SINK_ANALOG;
        connect_parameters->audio_sink = ANALOG_SINK;

        if(useBroadcastPlugin())
        {
            connect_parameters->audio_plugin = (Task)&csr_ba_analogue_decoder_plugin;
            connect_parameters->volume = sinkBroadcastAudioGetVolume(audio_source_ANALOG);
            ((A2dpPluginConnectParams *)(connect_parameters->params))->ba_output_plugin = AudioPluginOutputGetBroadcastPluginTask();
        }
        else
        {
            connect_parameters->audio_plugin = (Task)&csr_analogue_decoder_plugin;
            ((A2dpPluginConnectParams *)(connect_parameters->params))->ba_output_plugin = (Task)NULL;
        }

        return TRUE;
    }
    return FALSE;
}


bool spdifPopulateConnectParameters(audio_connect_parameters *connect_parameters)
{
    WIRED_DEBUG(("SPDIF: Audio "));

    if(SPDIF_READY && SPDIF_CONNECTED)
    {
        WIRED_DEBUG(("Connected [0x%04X]\n", (uint16)SPDIF_SINK));
        populateCommonConnectParams(connect_parameters);
        connect_parameters->audio_plugin = (Task)&csr_spdif_decoder_plugin;
        connect_parameters->sink_type = AUDIO_SINK_SPDIF;
        connect_parameters->audio_sink = SPDIF_SINK;
        return TRUE;
    }
    return FALSE;
}


bool i2sPopulateConnectParameters(audio_connect_parameters *connect_parameters)
{
    WIRED_DEBUG(("I2S: Audio "));

    if(I2S_READY && I2S_CONNECTED)
    {
        WIRED_DEBUG(("Connected [0x%04X]\n", (uint16)SPDIF_SINK));
        populateCommonConnectParams(connect_parameters);
        connect_parameters->audio_plugin = (Task)&csr_i2s_decoder_plugin;
        connect_parameters->sink_type = AUDIO_SINK_I2S;
        connect_parameters->audio_sink = I2S_SINK;
        return TRUE;
    }
    return FALSE;
}

/****************************************************************************
NAME
    analogAudioSinkMatch

DESCRIPTION
    Compare sink to analog audio sink.

RETURNS
    TRUE if Sink matches, FALSE otherwise
*/
bool analogAudioSinkMatch(Sink sink)
{
    if(ANALOG_READY)
        return (sink == ANALOG_SINK && sink);
    return FALSE;
}

/****************************************************************************
NAME
    spdifAudioSinkMatch

DESCRIPTION
    Compare sink to spdif audio sink.

RETURNS
    TRUE if Sink matches, FALSE otherwise
*/
bool spdifAudioSinkMatch(Sink sink)
{
    if(SPDIF_READY)
        return (sink == SPDIF_SINK && sink);
    return FALSE;
}

/****************************************************************************
NAME
    i2sAudioSinkMatch

DESCRIPTION
    Compare sink to i2s audio sink.

RETURNS
    TRUE if Sink matches, FALSE otherwise
*/
bool i2sAudioSinkMatch(Sink sink)
{
    if(I2S_READY)
         return (sink == I2S_SINK && sink);
    return FALSE;
}


/****************************************************************************
NAME
    analogAudioConnected

DESCRIPTION
    Determine if analog audio input is connected

RETURNS
    TRUE if connected, otherwise FALSE
*/
bool analogAudioConnected(void)
{
    /* If Wired mode configured and ready check PIO */
    if (ANALOG_READY)
    {
        return ANALOG_CONNECTED;
    }

    /* Wired audio cannot be connected yet */
    return FALSE;
}

/****************************************************************************
NAME
    analogGetAudioSink

DESCRIPTION
    Check analog state and return sink if available

RETURNS
    Sink if available, otherwise NULL
*/
Sink analogGetAudioSink(void)
{
    if (ANALOG_READY)
    {
        return ANALOG_SINK;
    }

    return NULL;
}


/****************************************************************************
NAME
    analogGetAudioRate

DESCRIPTION
    Obtains the current defined sample rate for wired audio

RETURNS
    None
*/
void analogGetAudioRate (uint32 *rate)
{
    if (ANALOG_READY)
    {
        *rate = WIRED_RATE;
    }
    else
    {
        *rate = 0;
    }
}


/****************************************************************************
NAME
    spdifAudioConnected

DESCRIPTION
    Determine if spdif audio input is connected

RETURNS
    TRUE if connected, otherwise FALSE
*/
bool spdifAudioConnected(void)
{
    /* If Wired mode configured and ready check PIO */
    if (SPDIF_READY)
    {
        return SPDIF_CONNECTED;
    }

    /* spdif audio cannot be connected yet */
    return FALSE;
}

/****************************************************************************
NAME
    i2sAudioConnected

DESCRIPTION
    Determine if i2s audio input is connected

RETURNS
    TRUE if connected, otherwise FALSE
*/
bool i2sAudioConnected(void)
{
 /* If Wired mode configured and ready check PIO */
    if (I2S_READY)
    {
        return I2S_CONNECTED;
    }

    /* Wired audio cannot be connected yet */
    return FALSE;
}

bool wiredAnalogAudioIsRouted(void)
{
    if(analogAudioSinkMatch(sinkAudioGetRoutedAudioSink()))
    {
        return TRUE;
    }
    return FALSE;
}

bool wiredSpdifAudioIsRouted(void)
{
    if(spdifAudioSinkMatch(sinkAudioGetRoutedAudioSink()))
    {
        return TRUE;
    }
    return FALSE;
}

bool wiredI2SAudioIsRouted(void)
{
    if(i2sAudioSinkMatch(sinkAudioGetRoutedAudioSink()))
    {
        return TRUE;
    }
    return FALSE;
}

void sinkWiredConfigureI2sInput(void)
{
    pio_common_pin_t pins;
    audio_instance i2s_audio_instance = sinkWiredReadI2sAudioInstanceConfig(); 

    AudioConfigSetI2sAudioInstance(i2s_audio_instance);

    if(i2s_audio_instance == AUDIO_INSTANCE_1)
    {
        pins = pin_pcmb_in | pin_pcmb_sync | pin_pcmb_clk;
    }
    else
    {
        pins = pin_pcma_in | pin_pcma_sync | pin_pcma_clk;
    }
    
    PanicFalse(PioCommonEnableFunctionPins(pins));
}

void sinkWiredConfigureSpdifInput(void)
{
    pio_common_pin_t pins;
    audio_instance spdif_audio_instance = sinkWiredReadSpdifAudioInstanceConfig();
    uint32 max_sample_rate =  getWiredRate(sinkWiredReadSpdifAudioMaxInputRate());

    AudioConfigSetSpdifAudioInstance(spdif_audio_instance);
    AudioConfigSetMaxSampleRate(max_sample_rate);

    if(spdif_audio_instance == AUDIO_INSTANCE_1)
    {
        pins = pin_spdifb_rx;
    }
    else
    {
        pins = pin_spdifa_rx;
    }
    
    PanicFalse(PioCommonEnableFunctionPins(pins));
}
#endif /* ENABLE_WIRED */
