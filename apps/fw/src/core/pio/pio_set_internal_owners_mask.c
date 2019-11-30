/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Write the owner (within the Apps block) of multiple PIOs
 */

#include "pio/pio_private.h"


void pio_set_internal_owners_mask(uint16 bank,
                                  pio_size_bits pios_mask,
                                  pio_select owner)
{
    ATOMIC_BLOCK_START {
        /* Perform a read-modify-write to only change the specified PIOs */
        hal_set_pio_select_mask(bank,
                                BIT_WRITE_MASK(hal_get_pio_select_mask(bank),
                                               pios_mask,
                                               (owner == PIO_SELECT_P1)?
                                                   pios_mask:0));
    } ATOMIC_BLOCK_END;
}
