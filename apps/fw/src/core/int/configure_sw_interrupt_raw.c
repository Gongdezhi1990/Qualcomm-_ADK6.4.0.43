/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Configure a software interrupt source
 *
 */

#include "int/swint_private.h"

/**
 * Configure a software interrupt source
 */
swint_id configure_sw_interrupt_raw(int_level level, void (* handler)(void))
{
    swint_id id;
    int_source source;

    /* Attempt to find an unused software interrupt source */
    id = 0;
    while (swint_configured & (1 << id))
    {
        if (SWINT_HW_SOURCES <= ++id)
        {
            panic(PANIC_HYDRA_TOO_MANY_SW_INTERRUPTS);
        }
    }
    /* Mark this source as used */
    swint_configured |= 1 << id;

    /* Configure this interrupt source */
    source = SWINT_TO_INT_SOURCE(id);
    configure_interrupt(source, level, handler);

    /* Return the software interrupt identifier */
    return id;
}
