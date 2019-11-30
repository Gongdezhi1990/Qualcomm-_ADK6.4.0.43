/****************************************************************************
 * Copyright (c) 2010 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file panic.c
 * \ingroup panic
 *
 * Protest and die
 *
 * \section panic DESCRIPTION
 * This file contains all of the functions dealing with panicking the chip
 *
 */

#include "panic/panic_private.h"
#include "hal/hal.h"
#ifndef KALIMBA_BUILD
#include "hal/halmgt.h"
#endif

/*
 * A static function to preserve data at panic
 * Kalimba needs a different approach
 */
static void panic_preserve_data(DIATRIBE_TYPE diatribe,
                                uint32 panic_time,
                                panicid deathbed_confession)
{
    volatile preserved_struct *this_preserved = (volatile preserved_struct *)PRESERVED_ADDR;

    this_preserved->panic.last_arg = diatribe;
    /* Note when it all went wrong */
    this_preserved->panic.last_time = panic_time;
    /* Store the reason code in a preserved variable so that it can be
       reported after the chip has been reset (this also makes it easy to
       access from a debugger) */
    this_preserved->panic.last_id = deathbed_confession;
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
#ifndef KALIMBA_BUILD
    hal_block_interrupts();
#else
    block_interrupts();
    patch_fn(panic_diatribe);
#endif
    L0_DBG_MSG2("PANIC 0x%x %d", deathbed_confession, diatribe);
    panic_preserve_data(diatribe, panic_time, deathbed_confession);

#ifdef INSTALL_HYDRA
    volatile preserved_struct *this_preserved = (volatile preserved_struct *)PRESERVED_ADDR;

#ifndef AUDIO_SECOND_CORE
    subreport_panic(deathbed_confession, this_preserved->panic.last_arg);
#else
    if (hal_get_reg_processor_id() == 0)
    {
        subreport_panic(deathbed_confession, this_preserved->panic.last_arg);
    }
#endif // !AUDIO_SECOND_CORE

#endif

#ifdef CHIP_BASE_A7DA_KAS
    kascmd_send_panic_message();
#endif

#ifdef CHIP_BASE_NAPIER
    comms_send_panic();
#endif

#ifndef KALIMBA_BUILD
    /* Ask any other processors to halt (but do not wait for acknowledgement
       in case they are unresponsive) and prepare this processor's preserved
       memory */
    reset_panic_prepare();

    /* Perform a stack backtrace (using the address of the function parameter
       as a rough starting point). The implementation of this function is
       (very) target specific, so is provided as part of the CRT. */

    /* Indicate the reason code on the LEDs and/or PIOs if required */
    PIO_TRACE_POINT(PANIC);
#endif

#if !defined(DESKTOP_TEST_BUILD) && !defined(PRODUCTION_BUILD)
    /* The watchdog may have been enabled, so kick it continuously to ensure
       that it never causes a reset */
    for (;;)
    {
#ifndef KALIMBA_BUILD
        hal_kick_watchdog();
#endif
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
