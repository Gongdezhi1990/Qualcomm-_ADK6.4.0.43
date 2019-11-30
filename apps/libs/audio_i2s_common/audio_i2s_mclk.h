/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    csr_i2s_audio_plugin_mclk.h
    
DESCRIPTION
    Subroutines related to I2S MCLK
*/


#ifndef _CSR_I2S_AUDIO_PLUGIN_MCLK_H_
#define _CSR_I2S_AUDIO_PLUGIN_MCLK_H_

#include <library.h>
#include <stream.h>

/****************************************************************************
DESCRIPTION:
    This function configures the MCLK signal for an I2S interface, if it is
    required.

PARAMETERS:
    Sink sink           - The Sink to configure MCLK for
    uint32 sample_rate  - Sample rate of the data coming from the DSP
*/
void AudioI2SConfigureMasterClockIfRequired(Sink sink, uint32 sample_rate);

/****************************************************************************
DESCRIPTION:
    This function configures the MCLK signal for an I2S interface, if it is
    required.

PARAMETERS:
    Source source       - The source to configure MCLK for
    uint32 sample_rate  - Sample rate of the data coming from the DSP
*/
void AudioI2SSourceConfigureMasterClockIfRequired(Source source, uint32 sample_rate);
        
#endif /* _CSR_I2S_AUDIO_PLUGIN_MCLK_H_ */
