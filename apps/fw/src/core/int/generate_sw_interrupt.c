/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Generate a software interrupt
 *
 */

#include "int/swint_private.h"

/**
 * Generate a software interrupt
 */
void generate_sw_interrupt(swint_id id)
{
    /* Determine whether this is a dedicated or multiplexed event */
    if (id < SWINT_HW_SOURCES)
    {
        /* Dedicated hardware interrupt */
        hal_generate_sw_interrupt(id);
    }
#if SWINT_SW_SOURCES
    else if (id < SWINT_SOURCES)
    {
        /* Hardware interrupt shared between multiple clients */
        swint_sw_source *source = &swint_sw_sources[id - SWINT_HW_SOURCES];
        if (!source->handler)
        {
            panic(PANIC_HYDRA_INVALID_SW_INTERRUPT);
        }
        source->req = TRUE;
        hal_generate_sw_interrupt(source->hw_id);
    }
#endif
    else
        panic(PANIC_HYDRA_INVALID_SW_INTERRUPT);
}
