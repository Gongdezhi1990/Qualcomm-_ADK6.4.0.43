/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Interrupt manager
*/
#ifndef __INT_H__
#define __INT_H__

#include "hydra/hydra_types.h"
#define IO_DEFS_MODULE_K32_INTERRUPT
#define IO_DEFS_MODULE_K32_MISC
#include "io/io.h"
#include "hal/hal.h"
#include "hal/halint.h"

/**
 * Indicates pending background interrupts/messages.
 * The dorm code checks it to decide whether to do a sleep.
 */
extern volatile bool background_work_pending;

/** Type used for specifying the interrupt priority level.
 * These values shouldn't be used directly. Instead the abstractions
 * below such as \c INT_LEVEL_FG should be used.
 */
typedef enum int_priority
{
    INT_PRIORITY_LOW  = 1,
    INT_PRIORITY_MED  = 2,
    INT_PRIORITY_HIGH = 3
} int_level;

/** Priority level to use for handling different interrupt sources */
#define INT_LEVEL_FG          (INT_PRIORITY_MED)
#define INT_LEVEL_HOSTIO      (INT_PRIORITY_MED)
#define INT_LEVEL_EXCEPTION   (INT_PRIORITY_HIGH)

/** (from BC)
 * Keyword "interrupt" to allow declarations of the form:
 * \code
 *     void interrupt fn( void );
 * \endcode
 *
 * We need to declare this here to avoid nested \#include problems.
 */
#define interrupt

/**
 * Initialise the interrupt subsystem
 *
 * Initialise the mechanism used to support the block_...() and
 * unblock_...() calls.
 *
 * This must be called before any use is made of these calls.
 */
extern void init_int(void);

/**
 * Set handler and priority for an interrupt source
 *
 * Configure interrupt "source" to call "handler" at priority "level", and
 * enable or disable the source as appropriate.
 *
 * \param source One of the enums from \c int_source_enum_enum
 * \param level The priority of the interrupt
 *
 * \param handler If NULL then a default interrupt handler will be set that
 * does nothing and the interrupt source will be disabled. Otherwise, the
 * specified handler will be installed and the interrupt source will be
 * enabled.
 *
 * This function temporarily disables all interrupts to ensure that the
 * change is made atomically. Hence, it should not be called
 * frequently.
 */
extern void configure_interrupt(int_source source, int_level level,
                                void (* handler)(void));

/**
 * Enable or disable an interrupt source
 *
 * If "enable" is TRUE then interrupt requests from "source" will be
 * latched, resulting in an interrupt service routine being called when
 * interrupts are unblocked. If "enable" is FALSE then interrupts from that
 * source are disabled.
 *
 * This function is not re-entrant; the caller must ensure that interrupts
 * are disabled if there is any possibility of the configuration being
 * changed at multiple priority levels.
 */
extern void enable_interrupt(int_source source, bool enable);

/**
 * Is the given interrupt source enabled?
 */
extern bool interrupt_is_enabled(int_source source);

/**
 * Block interrupts
 *
 * Block all interrupts. The caller is expected to call
 * unblock_interrupts() (very!) shortly after calling this function.
 *
 * These two function co-operate to ensure that nested pairs of calls
 * behave as expected; only the final call to unblock_interrupts()
 * actually unblocks interrupts.
 *
 * These functions are intended for ensuring atomic access to data that
 * is shared between code running at multiple interrupt priority levels.
 */
extern void block_interrupts(void);

/**
 * Unblock interrupts
 *
 * Unblock all interrupts. The caller is expected to call
 * this (very!) shortly after a call to block_interrupts().
 */
extern void unblock_interrupts(void);

/**
 * Block all interrupts
 *
 * Block all interrupts. The caller is expected to call
 * unblock_all_interrupts() (very!) shortly after calling this function.
 *
 * These two function co-operate to ensure that nested pairs of calls
 * behave as expected; only the final call to unblock_all_interrupts()
 * actually unblocks interrupts.
 *
 * These functions should should only be used when essential, e.g.
 */
extern void block_all_interrupts(void);

/**
 * Unblock all interrupts
 *
 * Unblock all interrupts. The caller is expected to call this (very!)
 * shortly after a call to block_all_interrupts().
 */
extern void unblock_all_interrupts(void);

#ifdef __KALIMBA__
/**
 * Blocking to levels isn't supported by the Kalimba so we translate the
 * calls for different levels, default levels and all levels to the normal
 * blocking/unblocking calls that block everything.
 */
#define block_interrupt_levels(levels)          block_interrupts()
#define unblock_interrupt_levels(levels)        unblock_interrupts()
#define block_all_interrupts()                  block_interrupts()
#define unblock_all_interrupts()                unblock_interrupts()
#define block_background()                      block_interrupts()
#define unblock_background()                    unblock_interrupts()
#endif /* __KALIMBA__ */

/**
 * Flag used in other modules (such as submsg) to indicate that the audio
 * style of interrupt enable/configure is needed.
 */
#define KALIMBA_INTERRUPTS

/**
 * Register an interrupt handler
 * \param name The name of the interrupt in the form INT_SOURCE_BUS_MSG_EVENT.
 * Adding INT_SOURCE_LOW_PRI_ should give the name of an element in the
 * \c int_source_enum_enum.
 * \param pri Interrupt priority. Should be one of \c LOW, \c MED, \c HIGH
 * \param fn The handler function to call
 *
 */
#define INTERRUPT_REGISTER(name,pri,fn)     \
    configure_interrupt((int_source)INT_SOURCE_ ## name, \
                                                        INT_PRIORITY_## pri, fn)

/**
 * Disable an interrupt
 * \param name The name of the interrupt in the form INT_SOURCE_BUS_MSG_EVENT.
 * Adding INT_SOURCE_LOW_PRI_ should give the name of an element in the
 * \c int_source_enum_enum.
 * \param pri Ignored. Present only for compatibility with audio code.
 *
 */
#define INTERRUPT_SOURCE_DISABLE(name,pri)  \
    enable_interrupt((int_source)INT_SOURCE_ ## name, FALSE)

/**
 * Indicates whether interrupts are blocked.
 *
 * Used in assert code to detect coding errors where it would be fatal
 * if interrupts were disabled.
 */
#define interrupts_are_blocked()                (!hal_get_reg_int_unblock())

/** The atomic block macros can be used round sections of code that satisfy
 *  the following criteria:
 *  - They do not call (un)block_all_interrupts().
 *  - They do not adjust INT_UNBLOCK.
 *  - They do not exit the block by any means other than falling out of
 *  the bottom (no break, continue, goto or return), unless they use
 *  a specific ATOMIC_BLOCK macro.
 *
 *  For safety it's advisable not to call any functions (otherwise one of
 *  the above criteria could be violated accidentally. This applies doubly
 *  to dereferencing function pointer to functions with a wide remit.
 *
 *  Inside the atomic block, if there are groups of instructions that need
 *  not be executed atomically *but which still satisfy the critera above*
 *  then the commands ATOMIC_BLOCK_NON_ATOMIC and ATOMIC_BLOCK_ATOMIC may be
 *  used to temporarily switch atomicity off and on. Note that
 *  ATOMIC_BLOCK_NON_ATOMIC does not necessarily enable interrupts, it merely
 *  returns them to the state they were in at the start of the atomic block.
 *  If the non atomic code satisfies the criteria for atomic code then the use
 *  of the ATOMIC_BLOCK_NON_ATOMIC and ATOMIC_BLOCK_ATOMIC macros is likely to
 *  be marginally faster than exiting the old block and starting another.
 *
 *  Example of use:
 *
 *  ATOMIC_BLOCK_START {
 *  atomic code;
 *  ATOMIC_BLOCK_NON_ATOMIC;
 *  non atomic code;
 *  ATOMIC_BLOCK_ATOMIC;
 *  atomic code;
 *  if (...)
 *  ATOMIC_BLOCK_RETURN(...);
 *  atomic code;
 *  } ATOMIC_BLOCK_END;
 *
 *  Avoid using these macros unless absolutely essential for efficiency; they
 *  block all interrupts.
 */
#define ATOMIC_BLOCK_START \
    do { \
        block_all_interrupts();

#define ATOMIC_BLOCK_NON_ATOMIC                 \
        unblock_all_interrupts()

#define ATOMIC_BLOCK_ATOMIC                     \
        block_all_interrupts()

/** Common macro expansions for different types of exit from atomic blocks */

/*lint -emacro(725, ATOMIC_BLOCK_END) Expected positive indentation - some hope! */
#define ATOMIC_BLOCK_END \
        ATOMIC_BLOCK_NON_ATOMIC; \
    } while (0)

#define ATOMIC_BLOCK_RETURN(retval)             \
    do {                                        \
        ATOMIC_BLOCK_NON_ATOMIC;                \
        return retval;                          \
    } while (0)

#endif /* __INT_H__ */
