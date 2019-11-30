/*!
\copyright  Copyright (c) 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_config.c
\brief	    Config data
*/

#include <panic.h>

#include "av_headset_config.h"
#include "av_headset_log.h"

#if defined(INCLUDE_PROXIMITY)
#if   defined(HAVE_VNCL3020)

#include "peripherals/vncl3020.h"
const struct __proximity_config proximity_config = {
    .threshold_low = 3000,
    .threshold_high = 3500,
    .threshold_counts = vncl3020_threshold_count_4,
    .rate = vncl3020_proximity_rate_7p8125_per_second,
    .i2c_clock_khz = 100,
    .pios = {
        /* The PROXIMITY_PIO definitions are defined in the platform x2p file */
        .on = PROXIMITY_PIO_ON,
        .i2c_scl = PROXIMITY_PIO_I2C_SCL,
        .i2c_sda = PROXIMITY_PIO_I2C_SDA,
        .interrupt = PROXIMITY_PIO_INT,
    },
};

#else
#error INCLUDE_PROXIMITY was defined, but no proximity sensor type was defined.
#endif   /* HAVE_VNCL3020 */
#endif /* INCLUDE_PROXIMITY */

#if defined(INCLUDE_ACCELEROMETER)
#if   defined(HAVE_ADXL362)

#include "peripherals/adxl362.h"
const struct __accelerometer_config accelerometer_config = {
    /* 250mg activity threshold, magic value from datasheet */
    .activity_threshold = 0x00FA,
    /* 150mg activity threshold, magic value from datasheet */
    .inactivity_threshold = 0x0096,
    /* Inactivity timer is about 5 seconds */
    .inactivity_timer = 30,
    .spi_clock_khz = 400,
    .pios = {
        /* The ACCELEROMETER_PIO definitions are defined in the platform x2p file */
        .on = ACCELEROMETER_PIO_ON,
        .spi_clk = ACCELEROMETER_PIO_SPI_CLK,
        .spi_cs = ACCELEROMETER_PIO_SPI_CS,
        .spi_mosi = ACCELEROMETER_PIO_SPI_MOSI,
        .spi_miso = ACCELEROMETER_PIO_SPI_MISO,
        .interrupt = ACCELEROMETER_PIO_INT,
    },
};
#else
#error INCLUDE_ACCELEROMETER was defined, but no accelerometer type was defined.
#endif   /* HAVE_ADXL362*/
#endif /* INCLUDE_ACCELEROMETER */

#if defined(INCLUDE_TEMPERATURE)
#if   defined(HAVE_THERMISTOR)

#include "peripherals/thermistor.h"
/* Requrired for THERMISTOR_ADC */
#include <app/adc/adc_if.h>
const thermistorConfig thermistor_config = {
    .on = THERMISTOR_ON,
    .adc = THERMISTOR_ADC,
};
#else
#error INCLUDE_TEMPERATURE was defined, but no temperature sensor was defined.
#endif   /* HAVE_THERMISTOR */
#endif /* INCLUDE_TEMPERATURE */

/*! Configuration of all audio prompts - a configuration  must be defined for
each of the NUMBER_OF_PROMPTS in the system. */
const promptConfig prompt_config[] =
{
    [PROMPT_POWER_ON] = {
        .filename = "power_on.sbc",
        .rate = 48000,
        .format = PROMPT_FORMAT_SBC,
    },
    [PROMPT_POWER_OFF] = {
        .filename = "power_off.sbc",
        .rate = 48000,
        .format = PROMPT_FORMAT_SBC,
    },
    [PROMPT_PAIRING] = {
        .filename = "pairing.sbc",
        .rate = 48000,
        .format = PROMPT_FORMAT_SBC,
    },
    [PROMPT_PAIRING_SUCCESSFUL] = {
        .filename = "pairing_successful.sbc",
        .rate = 48000,
        .format = PROMPT_FORMAT_SBC,
    },
    [PROMPT_PAIRING_FAILED] = {
        .filename = "pairing_failed.sbc",
        .rate = 48000,
        .format = PROMPT_FORMAT_SBC,
    },
    [PROMPT_CONNECTED] = {
        .filename = "connected.sbc",
        .rate = 48000,
        .format = PROMPT_FORMAT_SBC,
    },
    [PROMPT_DISCONNECTED] = {
        .filename = "disconnected.sbc",
        .rate = 48000,
        .format = PROMPT_FORMAT_SBC,
    },
};

/*! A prompt configuration must be defined for each prompt. */
COMPILE_TIME_ASSERT(ARRAY_DIM(prompt_config) == NUMBER_OF_PROMPTS, missing_prompt_configurations);


bool appConfigBleGetAdvertisingRate(appConfigBleAdvertisingMode mode, uint16 *min_rate, uint16 *max_rate)
{
    if (min_rate && max_rate)
    {
        switch (mode)
        {
            case APP_ADVERT_RATE_SLOW:
                *min_rate = appConfigBleSlowAdvertisingRateMin();
                *max_rate = appConfigBleSlowAdvertisingRateMax();
                break;

            case APP_ADVERT_RATE_FAST:
                *min_rate = appConfigBleFastAdvertisingRateMin();
                *max_rate = appConfigBleFastAdvertisingRateMax();
                break;

            default:
                DEBUG_LOG("appConfigBleGetAdvertisingRate Unsupported mode:%d requested",mode);
                Panic();
        }

        DEBUG_LOG("appConfigBleGetAdvertisingRate. Mode:%d requested. Adv rate %d-%d",
                                            mode, *min_rate, *max_rate);
        return TRUE;
    }
    return FALSE;
}

