/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  pl_error.c
 * \ingroup platform
 *
 * Kalimba error handlers
 *
 */

/****************************************************************************
Include Files
*/

#include "pl_error.h"
#include "panic/panic.h"
#include "hal.h"
#include "hal_alias.h"

/****************************************************************************
Public Data Definitions
*/

/****************************************************************************
Public Function Definitions
*/

/* General-purpose error handler for Kalimba libs */
void pl_lib_error_handler(unsigned data)
{
    /*
     * The 'data' argument is the value of rLink at the point of entry to
     * $error.
     */
    panic_diatribe(PANIC_AUDIO_LIBRARY_ERROR, data);
}

/* Handler for un-registered interrupt */
void pl_unknown_interrupt_handler(void)
{
    panic(PANIC_AUDIO_UNHANDLED_INTERRUPT);
}

/* Handler for software exception */
void pl_sw_exception_handler(unsigned code_address)
{
    /*
     * The 'code_address' argument indicates whereabouts the PC was when
     * the exception occurred. How accurate it is is platform-dependent;
     * see $M.error_sw_exception.
     *
     * Note that when reported as a panic argument to Curator on Hydra
     * platforms, this will be truncated to 16 bits, so is likely to be
     * ambiguous. In our preserve block it will retain its full width.
     */
#ifdef HAVE_CACHE_EXECUTE_EXCEPTION
    /* If we have the appropriate hardware, check what the exception type was */
    /* TODO_CRESCENDO - image 140829 - check that this is a correct substitution */
    if (hal_get_reg_exception_type() == EXCEPTION_TYPE_PM_PROG_REGION)
    {
        panic_diatribe(PANIC_AUDIO_CACHE_EXCEPTION, code_address);
    }
    else
#endif
    {
        panic_diatribe(PANIC_AUDIO_SW_EXCEPTION, code_address);
    }
}

/* Handler for stack-overflow exception */
void pl_stack_exception_handler(unsigned data)
{
    /*
     * The 'data' argument is the value of STACK_OVERFLOW_PC after the
     * exception interrupt fired.
     */
    panic_diatribe(PANIC_AUDIO_STACK_EXCEPTION, data);
}

/* Handler for APB SLV error exception */
#ifdef CHIP_BASE_A7DA_KAS
void pl_apb_slv_error(void)
{
    panic_diatribe(PANIC_AUDIO_APB_SLV_ERROR, 0);
}
#endif /* CHIP_BASE_A7DA_KAS */

