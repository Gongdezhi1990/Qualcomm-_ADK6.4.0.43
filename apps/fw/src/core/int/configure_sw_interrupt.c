/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Configure a software interrupt source
 *
 */

#include "int/swint_private.h"


#if SWINT_SW_SOURCES
/** Information for multiplexing hardware interrupts in software */
swint_hw_source swint_hw_sources[SWINT_HW_SOURCES];
swint_sw_source swint_sw_sources[SWINT_SW_SOURCES];
#endif

/** Mask of currently used software interrupt sources */
uint16f swint_configured = 0;


/**
 * Configure a software interrupt source
 */
swint_id configure_sw_interrupt(int_level level, void (* handler)(void))
{
    swint_id id;

    /* Software multiplexed events require special handling */
#if SWINT_SW_SOURCES
    if ((SWINT_MUX_LEVEL_MIN <= level) && (level <= SWINT_MUX_LEVEL_MAX))
    {
        swint_id hw_id;
        swint_sw_source *source;

        /* Determine or allocate a hardware event for this interrupt level */
        for (hw_id = 0; hw_id < SWINT_HW_SOURCES; ++hw_id)
            if (swint_hw_sources[hw_id].mux_count
                && (swint_hw_sources[hw_id].level == level))
                break;
        if (hw_id == SWINT_HW_SOURCES)
        {
            /* Configure a new hardware event for this interrupt level
               (messy because the identifier is not known until afterwards) */
            static void(* const swint_demux_n[])(void) =
                { swint_demux_0,
#if (SWINT_HW_SOURCES > 1)
                        swint_demux_1,
#endif
#if (SWINT_HW_SOURCES > 2)
                        swint_demux_2, swint_demux_3
#endif
#if (SWINT_HW_SOURCES > 4)
                  , swint_demux_4, swint_demux_5
#endif
#if (SWINT_HW_SOURCES > 6)
                  , swint_demux_6
#endif
                };
            hw_id = configure_sw_interrupt_raw(level, NULL);
            configure_interrupt(SWINT_TO_INT_SOURCE(hw_id),
                                level, swint_demux_n[hw_id]);
            swint_hw_sources[hw_id].level = level;
        }

        /* Increment the hardware event reference count */
        ++swint_hw_sources[hw_id].mux_count;

        /* Attempt to find an unused software interrupt source */
        source = swint_sw_sources;
        while (source->handler)
            if (++source == &swint_sw_sources[SWINT_SW_SOURCES])
                panic(PANIC_HYDRA_TOO_MANY_SW_INTERRUPTS);
        id = (swint_id) ((source - swint_sw_sources) + SWINT_HW_SOURCES);

        /* Configure this client */
        source->hw_id = hw_id;
        source->req = FALSE;
        source->handler = handler;
    }
    else
#endif /* SWINT_SW_SOURCES */
        id = configure_sw_interrupt_raw(level, handler);

    /* Return the software interrupt identifier */
    return id;
}
