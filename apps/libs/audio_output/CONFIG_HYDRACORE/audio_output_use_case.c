/******************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_output_use_case.c

DESCRIPTION
    Implements use-case specific routines for audio output.
*/

#include "audio_output.h"
#include "audio_output_private.h"
#include <audio_processor.h>

/******************************************************************************/
void audioOutputCheckUseCase(bool active, audio_output_hardware_type_t type)
{
    /* Only currently interested in two of the available types: I2S and S/PDIF */
    if ((type == audio_output_type_i2s) || (type == audio_output_type_spdif))
    {
        if (active)
        {
            AudioProcessorAddUseCase(audio_ucid_output_digital);
        }
        else
        {
            AudioProcessorRemoveUseCase(audio_ucid_output_digital);
        }
    }
}

