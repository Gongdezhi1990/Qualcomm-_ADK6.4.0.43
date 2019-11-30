/*****************************************************************************
*
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
*
*****************************************************************************/
/**
 * \file int.h
 *  Interrupt handling interface.
 */
#ifndef INT_H
#define INT_H
#include "platform/pl_interrupt.h"

/* This is currently a stub version for Hydra compatibility only */

#define block_interrupts interrupt_block
#define unblock_interrupts interrupt_unblock

#define block_all_interrupts interrupt_block
#define unblock_all_interrupts interrupt_unblock

#define configure_interrupt interrupt_register

/* The atomic block macros can be used round sections of code that satisfy
 * the following criteria:
 *    * They do not call block_ or unblock_interrupts().
 *    * They do not adjust INT_UNBLOCK.
 *    * They do not exit the block by any means other than falling out of
 *      the bottom (no break, continue, goto or return).
 *
 * For safety it's advisable not to call any functions (otherwise one of
 * the above criteria could be violated accidentally. This applies doubly
 * to dereferencing function pointer to functions with a wide remit (such as
 * the radiosched slopseq functions and itimer functions).
 *
 * Example of use:
 *
 *     ATOMIC_BLOCK_START {
 *         atomic code;
 *     } ATOMIC_BLOCK_END;
 *
 * NOTE: This code is stolen from Bluecore - specifically, int.h.
 * Before making any improvements/optimisations, check their versions
 * to see if we can ride their coat-tails.
 */

#define ATOMIC_BLOCK_START \
    do { \
        interrupt_block();

#define ATOMIC_BLOCK_END \
        interrupt_unblock(); \
    } while(0)

typedef enum int_prio
{
    /* If the values change, make sure to update code related to
     * CHIP_HAS_SEPARATE_INTERRUPT_ENABLE_AND_PRIO_REGS */
    INT_PRI_LOW = 1,
    INT_PRI_MID = 2,
    INT_PRI_HIGH = 3
} interrupt_priority;

typedef enum int_prio int_level;

#endif /* INT_H */
