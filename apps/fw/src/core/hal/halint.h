/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Hardware abstraction layer for interrupt hardware
 */

#ifndef HALINT_H
#define HALINT_H

#include "hydra/hydra_types.h"
#include "hal/hal_macros.h"


#define SW_INT_ID_APPCMD    1 /**< used by APPCMD module */
#define SW_INT_SOURCE_APPCMD    INT_SOURCE_SW1

/**
 * Generate a software interrupt
 *
 * Generate a software interrupt with identifier "id" (0 to 3). This
 * can be used for a process running at one interrupt level to provoke
 * a process at a different interrupt level.
 *
 * If the interrupt source for the specified identifier has been
 * configured at a higher priority than the current interrupt level, and
 * interrupts are currently enabled, then  the interrupt will occur prior
 * to control returning to the calling software.
 *
 * However, if it has been configured at a lower priority than the current
 * interrupt level then the interrupt will not occur until the current
 * interrupt service routine has completed and any other pending interrupts
 * with a higher priority have also been serviced. The order of servicing
 * interrupts of a given level is indeterminate.
 */
extern void hal_generate_sw_interrupt(uint16 id);

/* We define number of available software interrupts == 1 because
 * INT_SW1_EVENT is reserved for APPCMD and thus can't be used */
#define CHIP_SW_INT_COUNT 1

#define hal_generate_sw_interrupt(id)                                   \
    do { \
        volatile uint32 *reg = &INT_SW0_EVENT; \
        if (id != 0) \
        { \
            panic(PANIC_HYDRA_INVALID_SW_INTERRUPT); \
        } \
        *reg = 0; \
        *reg = 1; \
        *reg = 0; \
    } while(0)

#endif /* HALINT_H */
