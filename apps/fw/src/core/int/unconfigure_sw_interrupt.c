/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Unconfigure a software interrupt source
 *
 */

#include "int/swint_private.h"

/**
 * Unconfigure a software interrupt source
 */
void unconfigure_sw_interrupt(swint_id id)
{
    /* Software multiplexed events require special handling */
    if (id < SWINT_HW_SOURCES)
    {
        uint16f mask = 1U << id;
        int_source source;

        /* Check that the interrupt source is currently configured */
        if (!(swint_configured & mask))
        {
            panic(PANIC_HYDRA_INVALID_SW_INTERRUPT);
        }
        /* Disable this interrupt source */
        source = SWINT_TO_INT_SOURCE(id);
        enable_interrupt(source, FALSE);

        /* Mark this source as unused */
        swint_configured &= ~mask;
    }
#if SWINT_SW_SOURCES
    else if (id < SWINT_SOURCES)
    {
        /* Check that the interrupt source is currently configured */
        swint_sw_source *source = &swint_sw_sources[id - SWINT_HW_SOURCES];
        if (!source->handler)
        {
            panic(PANIC_HYDRA_INVALID_SW_INTERRUPT);
        }
        /* Unconfigure this client */
        source->handler = NULL;

        /* Decrement the hardware event reference count */
        if (!--swint_hw_sources[source->hw_id].mux_count)
        {
            unconfigure_sw_interrupt(source->hw_id);
        }
    }
#endif
    else
    {
        panic(PANIC_HYDRA_INVALID_SW_INTERRUPT);
    }
}
