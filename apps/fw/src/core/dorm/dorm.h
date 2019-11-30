/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Public header for the dorm module.
*/

#ifndef DORM_H
#define DORM_H

/*lint -e750 -e962*/ #define IO_DEFS_MODULE_K32_MISC
#include "timed_event/timed_event.h"
#include "hal/hal.h"
#include "io/io.h"

/**
 * @name Whether we allow deep or shallow sleep.
 *
 * Copied from BlueCore, once, long ago.
 */
/*@{*/

/**
 * List of modules with an influence on entering deep or shallow sleep.
 * Since we need code to run separately on each processor, it's
 * most convenient to keep the state separate, too.
 */
typedef enum dorm_ids {
    /**
     * For testing only not to allow P1 to enter deep sleep or shallow sleep
     */
    DORMID_TEST_SLEEP,
    /**
     * Final value, giving the count of valid values
     */
    DORMID_COUNT
} dorm_ids;
/**
 * Bits indicating the state of an individual module from \c dorm_ids.
 */
typedef enum dorm_state {
    DORM_STATE_ANY = 0,          /**< Can do what we like */
    DORM_STATE_RADIO_ACTIVE = 1, /**< Can't do full shallow sleep */
    DORM_STATE_NO_SHALLOW = 2,  /**< Can't enter shallow sleep. */
    DORM_STATE_NO_DEEP = 4
} dorm_state;

/*@}*/

/**
 * Sleep until the scheduler wants to wake up.
 *
 * The function will return early as soon as it detects a pending
 * background interrupt or if the chip detects activities
 * on wake up sources configured via \c dorm_set_sleep_info().
 *
 * This function calls \c sched_get_sleep_deadline() to retrieve earliest
 * and latest wakeup time if it needs to know them.
 *
 * \c The earliest and latest times returned by \c sched_get_sleep_deadline
 * have the following requiremets:
 *
 * \c earliest is the earliest time on the microsecond
 * clock at which \c dorm_sleep should return, unless woken by the
 * foreground.  This corresponds to the earliest time of an
 * event scheduled by \c timed_event_at_between().
 *
 * \c latest is the last time on the microsecond clock at which
 * \c dorm_sleep_sched() should return.  As usual, this value is compared
 * with the current time in a signed fashion:  if the difference has
 * the top bit (bit 31) set, then \c latest is treated as being in
 * the past and the function returns immediately.  This means that minor
 * delays in the background scheduler are benign.
 *
 * \c sched_get_sleep_deadline() also returns a boolean to indicate whether
 * it has a deadline or not.
 */
extern void dorm_sleep_sched(void);

/**
 * Enter shallow sleep unless this has been disabled.
 *
 * @param latest Latest time at which to wake up. This is currently ignored.
 *
 * The function will return early as soon as it detects a pending
 * background interrupt.
 */
void dorm_shallow_sleep(TIME latest);

/**
 * Rouse the background
 *
 * Cause the current (or next) call to dorm_sleep() to return as quickly as
 * possible.
 */
extern void dorm_wake(void);
#if (CHIP_HAS_SHALLOW_SLEEP_REGISTERS) && !defined(DESKTOP_TEST_BUILD)
#define dorm_wake() hal_set_reg_allow_goto_shallow_sleep(0)
#else
#define dorm_wake()
#endif

/**
 * Indicate an interrupt has occurred when deep sleep was allowed.
 *
 * This rouses the background, but does not necessarily cause it
 * to leave dorm.  It's needed to reset the deep sleep enable flag.
 */
extern void dorm_wake_interrupt(void);

/**
 * \brief  This function is called by a component when it is no longer
 * in a state that prevents deep sleep.
 *
 * \param comp  The component that no longer prevents deep sleep.
 */
extern void dorm_allow_deep_sleep(dorm_ids comp);
#define dorm_allow_deep_sleep(comp) dorm_set_sleep_info(comp, DORM_STATE_ANY)

/**
 * \brief  This function is called by a component when it enters a state that
 * prevents deep sleep being entered.
 *
 * \param comp  The component that is preventing deep sleep.
 */
extern void dorm_disallow_deep_sleep(dorm_ids comp);
#define dorm_disallow_deep_sleep(comp) dorm_set_sleep_info(comp, DORM_STATE_NO_DEEP)

/**
 * Initialise the subsystem.
 *
 * This must be called before any use if made of the other functions in
 * this library.  It does not perform software configuration, only
 * sets up some low level values.
 */
extern void init_dorm(void);

/**
 * Array storing the values passed down to dorm_sleep_info.
 * The elements are indexed by dormid, and the values are or's of
 * bits given by dorm_state.  Each bit indicates we can or can't go
 * into shallow sleep.
 *
 * This wastes quite a lot of bits, but the gain, that we can set
 * the value atomically as a single word when called from each module,
 * outweighs this.
 */
extern volatile uint16 kip_table[DORMID_COUNT];

/**
 * Set the shallow/deep sleep info for a module
 * Directly accesses \c kip_table.
 */
#define dorm_set_sleep_info(who, what) (kip_table[who] = (what))

/**
 * Get the shallow/deep sleep info for a module
 * Directly accesses \c kip_table.
 */
#define dorm_get_sleep_info(who) ((const uint16)(kip_table[who]))

/**
 * Get the combined kip_table setting
 * \return the inclusive or of all the components' settings
 */
extern uint16 dorm_get_combined_kip_flags(void);

/**
 * Initialise values from MIB
 *
 * To be called when the MIB has been set and the system fully
 * initialised.
 */
extern void dorm_config(void);

/**
 * Indication from P1 about sleep state
 *
 * It is the handler for the IPC \c IPC_SIGNAL_ID_P1_DEEP_SLEEP_MSG
 * \param deep_sleep  True: P1 requests sleeping, False: P1 doesn't request sleeping
 * \param earliest    Earliest wake up time (in microseconds)
 * \param latest      Latest wake up time (in microseconds)
 */
extern void dorm_set_sleep_info_for_p1 (bool deep_sleep, TIME earliest, TIME latest);

/**
 * Indication to dorm module that P1 is not present. P1 status won't be taken
 * into account to decide if the chip can deep sleep.
 */
extern void dorm_p1_not_present(void);

#endif /* DORM_H */
