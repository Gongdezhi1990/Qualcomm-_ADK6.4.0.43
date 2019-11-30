/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
#include "int/int_private.h"
#include "assert.h"

/**
 * Empty function to be called when a NULL value is passed as the handler
 * for an interrupt
 */
static void null_handler(void)
{
}

void configure_interrupt(int_source source,
                        int_level level,
                        void (* handler)(void))
{
    assert(source < NUM_ISR_CALL_TABLE_ENTRIES);
    assert(level <= MAX_INT_PRIORITY);
    block_interrupts();
    if(handler)
    {
        isr_call_table[source] = handler;
    }
    else
    {
        isr_call_table[source] = null_handler;
    }
    enable_interrupt(source, TRUE);
    hal_set_reg_int_select(source);
    hal_set_reg_int_priority(level);
    unblock_interrupts();
}

void enable_interrupt(int_source source, bool enable)
{
    assert(source < NUM_ISR_CALL_TABLE_ENTRIES);
    if(enable)
    {
        hal_set_reg_int_sources_en(hal_get_reg_int_sources_en() | (1<<source));
    }
    else
    {
        hal_set_reg_int_sources_en(hal_get_reg_int_sources_en() & ~(1<<source));
    }
}

bool interrupt_is_enabled(int_source source)
{
    return (hal_get_reg_int_sources_en() & (1 << source)) != 0;
}

