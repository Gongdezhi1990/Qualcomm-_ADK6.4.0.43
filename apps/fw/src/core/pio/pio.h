/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

#ifndef PIO_H
#define PIO_H

#include "hal/halauxio.h"


/**
 * Initialise the low-level PIO abstraction. This function must be called
 * before any use is made of the other functions defined in this file.
 */
extern void init_pio(void);


/**
 * Which bit of hardware within the subsystem gets to control the PIO.
 * Could be one of the processors (so the firmware wiggles it), a debug CLK
 * signal etc.
 * \param pio PIO number.
 * \param owner The new owner.
 */
/* extern void pio_set_internal_owner(uint16 pio, pio_select owner);*/
#define pio_set_internal_owner(pio, owner) \
    pio_set_internal_owners_mask (PBANK(pio), POFFM(pio), owner)

/**
 * Which bit of hardware within the subsystem gets to control the PIOs.
 * Could be one of the processors (so the firmware wiggles them), a debug CLK
 * signal etc.
 * \param bank PIO bank index.
 * \param pios_mask PIO mask.
 * \param owner The new owner of the selected PIOs.
 */
extern void pio_set_internal_owners_mask(uint16 bank,
                                         pio_size_bits pios_mask,
                                         pio_select owner);

/**
 * Returns the PIOs current owners.
 * \param bank PIO bank index.
 * \param pios_mask PIO mask.
 * \return Bit mask indicating the owners of the selected PIOs.
 */
/* extern pio_size_bits pio_get_internal_owners_mask(uint16 bank,
                                                     pio_size_bits pios_mask); */
#define pio_get_internal_owners_mask(bank, pios_mask) \
    (hal_get_pio_select_mask(bank) & pios_mask)


/**
 * Configure the specified PIO to be used as input or output: TRUE - output,
 * FALSE - input.
 * A particular PIO must be owned by this processor for this function to have
 * any effect. This function should be moderately fast; only protection against
 * higher priority interrupts on the same processor is required.
 * \param pio PIO number.
 * \param output TRUE for output and FALSE for input.
 */
/* extern void pio_set_direction(uint16 pio, bool output); */
#define pio_set_direction(pio, output) \
    pio_set_directions_mask (PBANK(pio), POFFM(pio), (output)?POFFM(pio):0) 

/**
 * Configure the specified PIOs to be used as inputs or outputs. Set direction
 * of masked PIOs based on the corresponding bit of "outputs_mask": 1 - output,
 * 0 - input
 * A particular PIO must be owned by this processor for this function to have
 * any effect. This function should be moderately fast; only protection against
 * higher priority interrupts on the same processor is required.
 * \param bank PIO bank index.
 * \param pios_mask PIO mask.
 * \param outputs_mask Output mask. 
 */
extern void pio_set_directions_mask(uint16 bank,
                                    pio_size_bits pios_mask,
                                    pio_size_bits outputs_mask);

/**
 * Returns the PIOs current set direction.
 * \param bank PIO bank index.
 * \param pios_mask PIO mask.
 * \return Bit mask indicating the directions set on the selected PIOs.
 */
/* extern void pio_get_directions_mask(uint16 bank,
                                       pio_size_bits pios_mask); */
#define pio_get_directions_mask(bank, pios_mask) \
    (hal_get_pio_directions(bank) & pios_mask)      


/**
 * Read the state of a single PIO. This always returns the current state of the
 * pin, regardless of how it is being controlled. For an input this will be the
 * externally applied signal (or internal pull-up/pull-down), and for an output
 * it will be the level being driven.
 * \param pio PIO number.
 * \return The state of the PIO as a boolean value.
 */
/* extern bool pio_get_level(uint16 pio_number); */
#define pio_get_level(pio) \
    ((pio_get_levels_mask(PBANK(pio), POFFM(pio)))?TRUE:FALSE)

/**
 * Read the state of several PIOs. This always returns the current state of the
 * pins, regardless of how they are controlled. For an input this will be the
 * externally applied signal (or internal pull-up/pull-down), and for an output
 * it will be the level being driven.
 * \param bank PIO bank index.
 * \param pios_mask PIO mask.
 * \return The state of the PIOs as a bit mask.
 */ 
extern pio_size_bits pio_get_levels_mask(uint16 bank, pio_size_bits pios_mask);

/**
 * Reads the state of all PIOs in a bank.
 * \param bank PIO bank index.
 * \return The state of the PIOs as a bit mask.
 */
/* extern pio_get_levels_all(uint16 bank); */
#define pio_get_levels_all(bank) (pio_get_levels_mask(bank, ALL_PIOS))


/**
 * Write the state of a single PIO. A particular PIO must be owned by this
 * processor for this function to have any effect. If a PIO is configured
 * as an output then this sets the drive level, otherwise for an input it
 * does nothing. Pull control is done by the Curator via CCP messages.
 * \param pio PIO number.
 * \param level Logic level to dive on the PIO.
 */
/* extern void pio_set_level(uint16 pio, bool level); */
#define pio_set_level(pio, level) \
    pio_set_levels_mask(PBANK(pio), POFFM(pio), (level)?POFFM(pio):0)

/**
 * Write the state of multiple PIOs. A particular PIO must be owned by this
 * processor for this function to have any effect. If a PIO is configured
 * as an output then this sets the drive level, otherwise for an input it
 * does nothing. Pull control is done by the Curator via CCP messages.
 * \param bank PIO bank index.
 * \param pios_mask PIOs mask.
 * \param levels_mask Logic levels to dive on the PIO.
 */ 
extern void pio_set_levels_mask(uint16 bank, pio_size_bits pios_mask, pio_size_bits levels_mask);


#endif /* PIO_H */
