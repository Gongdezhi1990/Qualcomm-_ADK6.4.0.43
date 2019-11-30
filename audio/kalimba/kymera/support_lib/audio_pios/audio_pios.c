/***************************************************************
 * Copyright (c) 2018 Qualcomm Technologies International, Ltd.
 **************************************************************/

#include "audio_pios/audio_pios.h"
/* Dummy functions to resolve kalsim builds */
void audio_pios_set_direction(PIO_DIRECTION direction, audio_pios pio_mask)
{
}

void audio_pios_set_outputs(audio_pios pio_mask, audio_pios pio_value)
{
}

void audio_pios_get_inputs(audio_pios *pios)
{
}
