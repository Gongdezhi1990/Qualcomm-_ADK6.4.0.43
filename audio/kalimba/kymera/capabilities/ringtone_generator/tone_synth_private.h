/**
 * Copyright (c) 2017 Qualcomm Technologies International, Ltd.
 * \defgroup tone_synth
 * \file  tone_synth.h
 * \ingroup capabilities
 *
 * Tone synthesizer private header file. <br>
 *
 */

#ifndef TONE_SYNTH_TONE_SYNTH_PRIVATE_H
#define TONE_SYNTH_TONE_SYNTH_PRIVATE_H

/*****************************************************************************
Include Files
*/

#include "tone_synth.h"

/****************************************************************************
Module Private Macros
*/

/* n/a */

/****************************************************************************
Module Private Constant Definitions
*/

/* n/a */

/****************************************************************************
Module Private Type Definitions
*/

/* n/a */

/****************************************************************************
Module Private Function Declarations
*/

/**
* \brief    Start a tone: reset phase accumulator and sample counter,
*           set initial amplitude
*
* \param    synth_data  The tone synthesizer context
*/
extern void tone_synth_tone_start(TONE_SYNTH_DATA *synth_data);

/**
* \brief    Calculate a sine, with 16 bit phase and 8 bit value resolution
*
* \param    x 16 bit phase
* \return   Sample value
*/
extern int16 tone_synth_sin_8bit(uint16 x);



#endif /* TONE_SYNTH_TONE_SYNTH_PRIVATE_H */

