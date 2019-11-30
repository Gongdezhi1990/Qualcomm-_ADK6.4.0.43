/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    csr_i2s_audio_plugin_mclk.c

DESCRIPTION
    Subroutines related to I2S MCLK
*/

#include <audio.h>
#include <audio_mclk.h>
#include <stdlib.h>
#include <panic.h>
#include <print.h>
#include <stream.h> 
#include <sink.h>
#include <source.h>
#include "../audio_i2s_mclk.h"

#include "audio_i2s_common.h"

/* I2S configuration data */
extern I2SConfiguration * I2S_config;

/******************************************************************************/
void AudioI2SConfigureMasterClockIfRequired(Sink sink, uint32 sample_rate)
{
    if (AudioI2SIsMasterClockRequired())
    {
        /* set the master clock (MCLK) rate for master mode */
        PanicFalse(SinkConfigure(sink, STREAM_I2S_MASTER_MCLK_MULT, I2S_config->i2s_init_config.master_clock_scaling_factor));

        /* set the clock source to MCLK */
        PanicFalse(SinkConfigure(sink, STREAM_I2S_MASTER_CLK_SOURCE, CLK_SOURCE_TYPE_MCLK));
        
        UNUSED(sample_rate);
    }
}

/******************************************************************************/
void AudioI2SSourceConfigureMasterClockIfRequired(Source source, uint32 sample_rate)
{
    if (AudioI2SIsMasterClockRequired())
    {
        /* set the master clock (MCLK) rate for master mode */
        PanicFalse(SourceConfigure(source, STREAM_I2S_MASTER_MCLK_MULT, I2S_config->i2s_init_config.master_clock_scaling_factor));

        /* set the clock source to MCLK */
        PanicFalse(SourceConfigure(source, STREAM_I2S_MASTER_CLK_SOURCE, CLK_SOURCE_TYPE_MCLK));

        if (!AudioI2SMasterIsEnabled())
        {
            /* configure MCLK to come from external source in slave mode */
            PanicFalse(AudioMasterClockConfigure(TRUE, sample_rate * I2S_config->i2s_init_config.master_clock_scaling_factor));
        }
    }
}

/******************************************************************************/
void AudioI2SEnableMasterClockIfRequired(Sink sink, bool enable)
{
    if (AudioI2SIsMasterClockRequired())
    {
        if (enable)
        {
            if (AudioI2SMasterIsEnabled())
            {
                /* I2S Master, enable MCLK output */
                PanicFalse(SinkMasterClockEnable(sink, MCLK_ENABLE_OUTPUT));
            }
        }
        else
        {
            /* Disable MCLK */
            PanicFalse(SinkMasterClockEnable(sink, MCLK_DISABLE));
        }
    }
}

/******************************************************************************/
void AudioI2SSourceEnableMasterClockIfRequired(Source source, bool enable)
{
    if (AudioI2SIsMasterClockRequired())
    {
        if (enable)
        {
            if (AudioI2SMasterIsEnabled())
            {
                /* I2S Master, enable MCLK output */
                PanicFalse(SourceMasterClockEnable(source, MCLK_ENABLE_OUTPUT));
            }
            else
            {
                /* I2S Slave, enable MCLK input from external source */
                PanicFalse(SourceMasterClockEnable(source, MCLK_ENABLE));
            }
        }
        else
        {
            /* Disable MCLK */
            PanicFalse(SourceMasterClockEnable(source, MCLK_DISABLE));
        }
    }
}
