/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
#include "int/int_private.h"

void (*isr_call_table[NUM_ISR_CALL_TABLE_ENTRIES])(void);

#ifdef LOG_IRQS
uint32 irq_counts[INT_SOURCE_LAST_POSN+1];
#endif /* LOG_IRQS */

volatile bool background_work_pending = FALSE;

#if defined(ENABLE_SHALLOW_SLEEP_TIMING) && defined(TEST_TUNNEL_FW_MODULE_PRESENT)
/**
 * Variable declared in core/int/interrupt.asm that counts the microseconds
 * spent in shallow sleep.
 */
extern uint32 total_shallow_sleep_time;

static void test_tunnel_pdu_handler(uint16 src, const uint32 * pdu,
                                                uint32 param_len_words);
#endif /* ENABLE_SHALLOW_SLEEP_TIMING */

static void interrupt_unregistered(void);

/**
 * Assembly function to switch to user mode as a step within the interrupt
 * module initialisation.
 */
/*
 * Lint has no hope of coping with this
 * It would be better moved into an assembler file
 * as it's only called during initialisation
 */
#ifndef DESKTOP_TEST_BUILD
asm void interrupt_switch_to_user_mode(void)
{
#if !defined(_lint)
    rFlags = rFlags OR $UM_FLAG_MASK;
#endif
}
#else
/* This supports coverity */
static void interrupt_switch_to_user_mode(void)
{
}
#endif

void init_int(void)
{
    uint8 entry;

    for(entry = 0; entry < NUM_ISR_CALL_TABLE_ENTRIES; ++entry)
    {
        isr_call_table[entry] = /*lint --e{546}*/&interrupt_unregistered;
    }
    (void)isr_call_table; /* Shut lint up */

    hal_set_reg_int_sources_en(0);

    /* Point the interrupt vector to the assembly code ISR that will call
     * the appropriate handler. */
    hal_set_reg_int_addr((uint32)interrupt_handler);

    /* enable switching to the maximum clock frequency during an interrupt */
    hal_set_reg_int_clock_divide_rate(K32_INTERRUPT__CLOCK_RATE_MAX);
    hal_set_reg_int_clk_switch_en(1);

    /* Enable interrupts */
    hal_set_reg_int_gbl_enable(1);
    hal_set_reg_int_unblock(1);

    /*lint --e{718} --e{746}*/interrupt_switch_to_user_mode();
#if defined(ENABLE_SHALLOW_SLEEP_TIMING) && defined(TEST_TUNNEL_FW_MODULE_PRESENT)
    test_tunnel_fw_early_register_handler(
                                        TEST_TUNNEL_APPS_FW_MODULE_ID_INTERRUPT,
                                        test_tunnel_pdu_handler);
#endif /* ENABLE_SHALLOW_SLEEP_TIMING */
}

static void interrupt_unregistered(void)
{
    panic_diatribe(PANIC_UNREGISTERED_INTERRUPT, hal_get_reg_int_source());
}

#if defined(ENABLE_SHALLOW_SLEEP_TIMING) && defined(TEST_TUNNEL_FW_MODULE_PRESENT)
static void test_tunnel_pdu_handler(uint16 src, const uint32 * pdu,
                                                uint32 param_len_words)
{
    uint32 * response;
    bool ret;

    assert(pdu[0] == TEST_TUNNEL_APPS_FW_INTERRUPT_ID_GET_CPU_USAGE_REQ);
    assert(param_len_words == 1);

    ret = test_tunnel_fw_get_response_ptr(&response, src, 3);
    assert(ret); /* To-host flow control will cause this assert */
    response[0] = TEST_TUNNEL_APPS_FW_INTERRUPT_ID_GET_CPU_USAGE_RSP;
    response[1] = hal_get_reg_timer_time();
    response[2] = total_shallow_sleep_time;
    (void)test_tunnel_fw_send_response(src,
                                TEST_TUNNEL_APPS_FW_MODULE_ID_INTERRUPT,
                                response, 3);
}
#endif /* ENABLE_SHALLOW_SLEEP_TIMING */
