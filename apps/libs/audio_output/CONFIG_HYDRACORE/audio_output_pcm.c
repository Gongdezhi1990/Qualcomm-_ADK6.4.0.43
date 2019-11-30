/******************************************************************************
Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_output_pcm.c

DESCRIPTION
    Implements hardware specific routines for audio output via PCM.
*/

#include <hydra_macros.h>

#include "audio_output.h"
#include "audio_output_private.h"

/******************************************************************************/
bool audioOutputPcmInitHardware(const audio_output_config_t* config)
{
    UNUSED(config);
    /* Nothing to do. */
    return TRUE;
}
