/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

#ifndef INT_LOCAL_H_
#define INT_LOCAL_H_

#define IO_DEFS_MODULE_K32_TIMERS
#include "assert.h"
#include "io/io.h"
#include "hal/hal.h"
#include "int/int.h"
#include "panic/panic.h"
#ifdef TEST_TUNNEL_FW_MODULE_PRESENT
#include "test_tunnel_fw/test_tunnel_fw.h"
#endif

#define NUM_ISR_CALL_TABLE_ENTRIES  (INT_SOURCE_LAST+1)

/**
 * Maximum supported by the hardware
 * Refer to bit width of INT_PRIORITY register
 */
#define MAX_INT_PRIORITY    (3)

extern void (* isr_call_table[NUM_ISR_CALL_TABLE_ENTRIES])(void);

/**
 * The interrupt handler routine in the interrupt.asm code that saves
 * and restores state and calls the registered handler.
 */
void interrupt_handler(void);

#endif /* INT_LOCAL_H_ */
