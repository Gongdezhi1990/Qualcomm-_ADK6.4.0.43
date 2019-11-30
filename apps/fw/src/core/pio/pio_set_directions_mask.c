/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Write the direction of multiple PIOs
 */

#include "pio/pio_private.h"


void pio_set_directions_mask(uint16 bank,
                             pio_size_bits pios_mask,
                             pio_size_bits output_mask)
{
    ATOMIC_BLOCK_START {
        /* Perform a read-modify-write to only change the specified PIOs */
        hal_set_pio_directions(bank,
                               BIT_WRITE_MASK(hal_get_pio_directions(bank),
                                              pios_mask,
                                              output_mask));
    } ATOMIC_BLOCK_END; 
}

