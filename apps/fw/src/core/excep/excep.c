/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

#include "excep/excep_private.h"

#define HAL_GET_REG_APPS_BANKED_EXCEPTION_STATUS()       \
                    hal_get_reg_apps_banked_exceptions_p1_status()



/**
 * ISR for Kalimba memory exceptions. Panics unless overridden in
 * which case it increments a counter.
 */
static void memory_exception_handler(void)
{
    exception_type excep_type = hal_get_reg_exception_type();

    if(excep_type == EXCEPTION_TYPE_OTHER)
    {
        uint32 banked_excep_type =
                HAL_GET_REG_APPS_BANKED_EXCEPTION_STATUS();
        panic_diatribe(PANIC_KALIMBA_EXCEPTION, banked_excep_type);
    }
    panic_diatribe(PANIC_KALIMBA_MEMORY_EXCEPTION, excep_type);
}

void excep_enable(void)
{
    configure_interrupt(INT_SOURCE_SW_ERROR, INT_LEVEL_EXCEPTION,
                        memory_exception_handler);
    enable_interrupt(INT_SOURCE_SW_ERROR, TRUE);

    hal_set_reg_exception_en(EXCEPTION_EN_IRQ_MASK);

}

void excep_branch_through_zero(void)
{
    /* The initialisation code is being re-executed without a hardware
       reset, so panic to log the failure and ensure a clean recovery. */
    panic(PANIC_HYDRA_BRANCH_THROUGH_ZERO);
}

void excep_disable(void)
{
    hal_set_reg_exception_en(0);
    enable_interrupt(INT_SOURCE_SW_ERROR, FALSE);
}
