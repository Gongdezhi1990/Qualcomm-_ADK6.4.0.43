/*************************************************************************
 * Copyright (c) 2018 Qualcomm Technologies International, Ltd.
 * All Rights Reserved.
 * Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*************************************************************************/
/**
 * \defgroup extmem external memory manager
 *
 * \file audio_pios.h
 * \ingroup audio_pios
 *
 * Public definitions for audio_pios
 */
#ifndef _AUDIO_PIOS_H_
#define _AUDIO_PIOS_H_

/****************************************************************************
Include Files
*/
#include "types.h"

/****************************************************************************
Macro Declarations
*/

/* All known platforms that use Kymera have number of PIOs that can be represented
 * as a bit field using 3 32-bit words */
#define NUM_PIO_WORDS 3

/****************************************************************************
Type Declarations
*/

/**
 * Enumeration to represent PIO direction
 */
typedef enum
{
    INPUT_PIO = 0,
    OUTPUT_PIO = 1
}PIO_DIRECTION;

/**
 * Structure to contain Audio_PIOs as a bitfield.
 */
typedef struct{
    /* pio_words[0] contains PIOs 0-31(msb), pio_words[1] contains PIOS 32-63 &
     * pio_words[2] contains PIOs 64-95. The higher bits of pio_words[2] will
     * be ignored if the platform does not support as many PIOs. */
    uint32 pio_words[NUM_PIO_WORDS];
} audio_pios;


/****************************************************************************
Public Function Definitions
*/

/**
 *
 * \brief Set the required direction for the specified PIOs represented as bitfields
 *
 * \note The access functions assume that the Audio Subsystem already has the
 * ownership of the PIOs allocated from the Curator.
 *
 * \param direction The required PIO direction.
 * \param pio_mask PIOs to be set, represented as bit field.
 */
extern void audio_pios_set_direction(PIO_DIRECTION direction, audio_pios pio_mask);

/**
 *
 * \brief Set the output PIOs specified in bitmask to the specified value.
 *
 * \note The access functions assume that the Audio Subsystem already has the
 * ownership of the PIOs allocated from the Curator.
 *
 * \param pio_mask Each bit in the mask corresponds to a PIO line. Bits set to 1
 * in this mask will be modified. Bits set to 0 in this mask will not be modified.
 * \param pio_value Each bit in the "pio_value"  corresponds to a PIO line. Bits
 * set to 1 in this value will result in that PIO line being driven high. Bits set
 * to 0 in this value will result in that PIO line being driven low.
 */
extern void audio_pios_set_outputs(audio_pios pio_mask, audio_pios pio_value);

/**
 * \brief  Read the current pio status. The function will return the current
 * PIO status of all PIOs. The caller should mask out the required PIOs from
 * the returned bit fields.
 *
 * \note The access functions assume that the Audio Subsystem already has the
 * ownership of the PIOs allocated from the Curator.
 *
 * \param pios Pointer to return current PIO status as a bit field.
 *
 */
extern void audio_pios_get_inputs(audio_pios *pios);

#endif /*_AUDIO_PIOS_H_*/
