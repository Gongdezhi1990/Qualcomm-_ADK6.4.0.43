/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Handler for a multiplexed software interrupt
 * 
 */

#include "int/swint_private.h"

/**
 * Handler for a multiplexed software interrupt
 */
#if SWINT_SW_SOURCES
void swint_demux(swint_id id)
{
    swint_sw_source *source;

    /* Call all pending handlers for this interrupt level */
    for (source = swint_sw_sources;
         source != &swint_sw_sources[SWINT_SW_SOURCES];
         ++source)
    {
        if (source->handler && (source->hw_id == id) && source->req)
        {
            source->req = FALSE;
            (*(source->handler))();
        }
    }
}

/**
 * Handler for a multiplexed software interrupt
 */
void swint_demux_0(void) { swint_demux((swint_id) 0); }
#if (SWINT_HW_SOURCES > 1)
void swint_demux_1(void) { swint_demux((swint_id) 1); }
#endif
#if (SWINT_HW_SOURCES > 2)
void swint_demux_2(void) { swint_demux((swint_id) 2); }
void swint_demux_3(void) { swint_demux((swint_id) 3); }
#endif
#if (SWINT_HW_SOURCES > 4)
void swint_demux_4(void) { swint_demux((swint_id) 4); }
void swint_demux_5(void) { swint_demux((swint_id) 5); }
#endif
#if (SWINT_HW_SOURCES > 6)
void swint_demux_6(void) { swint_demux((swint_id) 6); }
#endif

#endif  /* SWINT_SW_SOURCES */
