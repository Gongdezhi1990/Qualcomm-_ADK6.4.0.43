/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Read the state of multiple PIOs
 */

#include "pio/pio_private.h"

pio_size_bits pio_get_levels_mask(uint16 bank,
                                  pio_size_bits pios_mask)
{
    pio_size_bits ret;

    ATOMIC_BLOCK_START {
        /* Perform multiple read operations on the specified PIOs */
        ret = BIT_WRITE_MASK(hal_get_input_pios(bank),
                             hal_get_pio_directions(bank),
                             hal_get_output_pios(bank));
    } ATOMIC_BLOCK_END;
    return ret & pios_mask;
}
        
