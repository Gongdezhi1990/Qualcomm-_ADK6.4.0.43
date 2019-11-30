/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 *
 * Pause for at least a given number of micro seconds
 */
/*lint -e750 -e962*/ #define IO_DEFS_MODULE_K32_TIMERS
#include "hal/haltime.h"
#include "hydra/hydra_macros.h" /* for RUN_FROM_RAM */


/**
 * Pause for at least a given number of micro seconds
 *
 * NOTES
 *
 * This function assumes that TIMER_TIME has been setup to generate a
 * 1MHz clock.  If it hasn't then this will have problems.
 */
RUN_FROM_RAM
void hal_delay_us(uint16 n)
{
    /* add one to the delay to make sure we don't return early */

    uint32 done = hal_get_time() + n + 1;

    /* This is safe when the time wraps.  We could just use the low 16
       bits though. */

    while (((int32)(hal_get_time() - done)) < 0)
        /*EMPTY*/;
}
