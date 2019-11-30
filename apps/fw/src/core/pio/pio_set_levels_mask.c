/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Write the state of multiple PIOs
 */

#include "pio/pio_private.h"


void pio_set_levels_mask(uint16 bank,
                         pio_size_bits pios_mask,
                         pio_size_bits levels_mask)
{
    ATOMIC_BLOCK_START {
        /* Perform a read-modify-write to only change the specified PIOs */
        hal_set_output_pios(bank,
                            BIT_WRITE_MASK(hal_get_output_pios(bank),
                                           pios_mask,
                                           levels_mask));
    } ATOMIC_BLOCK_END;
}
