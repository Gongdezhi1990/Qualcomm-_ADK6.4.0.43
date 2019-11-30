/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

#include "panic/panic_private.h"
#include "hal/hal.h"

/**
 * A static function to preserve data at panic.
 * Kalimba needs a different approach.
 * \param diatribe Panic argument.
 * \param panic_time Timestamp.
 * \param deathbed_confession Panic ID.
 */
static void panic_preserve_data(DIATRIBE_TYPE diatribe,
                                uint32 panic_time,
                                panicid deathbed_confession)
{
#ifndef SUBSYSTEM_AUDIO
#ifdef PRESERVED_MODULE_PRESENT
    preserved.panic.last_arg = diatribe;
    /* Note when it all went wrong */
    preserved.panic.last_time = panic_time;
    /* Store the reason code in a preserved variable so that it can be
       reported after the chip has been reset (this also makes it easy to
       access from a debugger) */
    preserved.panic.last_id = deathbed_confession;
#else
    UNUSED(diatribe);
    UNUSED(panic_time);
    UNUSED(deathbed_confession);
#endif /* PRESERVED_MODULE_PRESENT */
#else
    preserve_panic_diatribe = diatribe;
    preserve_panic_id = deathbed_confession;
    preserve_panic_time_ls = (uint24)panic_time;
    preserve_panic_time_ms = (uint24)(panic_time >> 24);
#endif
}

/**
 * Protest volubly and die
 *
 * IMPLEMENTATION NOTE
 *
 * Ideally this function would only perform trivial operations that do not
 * rely on any memory contents or other state. This is because it is likely
 * to be called when something has gone disasterously wrong, in which case
 * the state cannot be assumed to be valid. Functions from the HAL can be
 * considered reasonably safe, but other functions should generally be
 * avoided.
 *
 * Unfortunately compromises need to be made, such as signing the preserved
 * memory to ensure that the panic can be correctly reported after the chip
 * has been reset. These exceptions should be kept to a minimum.
 */
void panic_diatribe(panicid deathbed_confession, DIATRIBE_TYPE diatribe)
{
    TIME panic_time = hal_get_time();

    /* Block all interrupts to ensure that the state is accurately preserved */
#ifndef OS_OXYGOS
    hal_block_interrupts();
#else
    block_interrupts();
    patch_fn(panic_diatribe);
#endif
    L0_DBG_MSG2("PANIC 0x%x %d", deathbed_confession, diatribe);
    panic_preserve_data(diatribe, panic_time, deathbed_confession);

    panic_report(deathbed_confession, panic_time, diatribe, THIS_PROCESSOR);

#ifdef RESET_MODULE_PRESENT
    /* Ask any other processors to halt (but do not wait for acknowledgement
       in case they are unresponsive) and prepare this processor's preserved
       memory */
    reset_panic_prepare();
#endif

    /* Perform a stack backtrace (using the address of the function parameter
       as a rough starting point). The implementation of this function is
       (very) target specific, so is provided as part of the CRT. */

#ifdef PIO_DEBUG_MODULE_PRESENT
    /* Indicate the reason code on the LEDs and/or PIOs if required */
    PIO_TRACE_POINT(PANIC);
#endif

#if !defined(DESKTOP_TEST_BUILD) && !defined(PRODUCTION_BUILD)
    /* We want to end up in an infinite loop here rather than in the CRT (which
     * is how exit is implemented) so that we remain in DWARF-instrumented code
     * and can get the all-important panic backtrace. */
    for (;;)
    {
    }
#else /* !DESKTOP_TEST_BUILD && !PRODUCTION_BUILD */

    /*
     * On a standalone chip we would reset or halt the processor as
     * appropriate.  Here, we wait for the Curator to take action
     * based on the subreport message.
     */

    /* On an embedded platform this enters an infinite loop (at least until
       the chip is reset, e.g. by the watchdog if enabled) */
    /*NOTREACHED*/
#ifdef DESKTOP_TEST_BUILD
    #include <stdlib.h>
#endif
    exit((int) deathbed_confession);
#endif /* !DESKTOP_TEST_BUILD && !PRODUCTION_BUILD */
}

/**
 * Protest pathetically and die
 */
void panic(panicid deathbed_confession)
{
    panic_diatribe(deathbed_confession, 0U);
}
