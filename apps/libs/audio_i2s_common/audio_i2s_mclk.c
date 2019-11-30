/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    csr_i2s_audio_plugin_mclk.c

DESCRIPTION
    Subroutines related to I2S MCLK
*/

#include "audio_i2s_mclk.h"

#include <audio.h>
#include <stdlib.h>
#include <panic.h>
#include <print.h>
#include <stream.h> 
#include <sink.h>
#include <source.h>

#include "audio_i2s_common.h"


/******************************************************************************/
void AudioI2SConfigureMasterClockIfRequired(Sink sink, uint32 sample_rate)
{
    UNUSED(sink);
    UNUSED(sample_rate);
}

/******************************************************************************/
void AudioI2SSourceConfigureMasterClockIfRequired(Source source, uint32 sample_rate)
{
    UNUSED(source);
    UNUSED(sample_rate);
}

/******************************************************************************/
void AudioI2SEnableMasterClockIfRequired(Sink sink, bool enable)
{
    UNUSED(sink);
    UNUSED(enable);
}

/******************************************************************************/
void AudioI2SSourceEnableMasterClockIfRequired(Source source, bool enable)
{
    UNUSED(source);
    UNUSED(enable);
}
