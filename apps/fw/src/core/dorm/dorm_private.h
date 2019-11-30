/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 *
 * Private header for the Hydra dorm module.
 */

#ifndef DORM_PRIVATE_H
#define DORM_PRIVATE_H

#define IO_DEFS_MODULE_CHIP /* An again */
#include "dorm/dorm.h"
#include "hydra/hydra_macros.h"
#include "hal/hal.h"
#include "hydra/hydra_patch.h"
#include "hydra_log/hydra_log.h"

/** Special wake up times used to request sleep with no deadline.
 * Wake time has type uint32 and contains system microsecond clock
 * time. The 32 bit microsecond timer wraps every 4294 seconds or
 * 71.58 minutes. Below values have been carefully selected by
 * taking into consideration that wake up times with deadline will
 * never have (2^32-1) difference between earliest and latest.
 * Note: These values are meant to be used by dorm module for IPC
 * messaging between P1 and P0. Other modules requesting sleep with
 * no deadline should use \c dorm_sleep_no_deadline() instead of
 * using these values.
 */
#define DORM_EARLIEST_WAKEUP_TIME_NO_DEADLINE (1)
#define DORM_LATEST_WAKEUP_TIME_NO_DEADLINE (0)

/**
 * The minimum time for which it's worth doing deep sleep signalling.
 *
 * This is supposed to be short enough that we won't be prevented
 * from deep sleep any time it's going to make a significant
 * difference to power saving, and long enough that we won't waste
 * time with a lot of pointless signalling on short periods.  However,
 * opinions may differ.
 */
#define DEEP_SLEEP_MIN_TIME     (10 * MILLISECOND)

/**
 * Value stored in shared.dorm[].min_mmu_rate to indicated deep sleep.
 */
#define MMU_RATE_DEEP_SLEEP (0xffff)
/**
 * Other values of min_mmu_rate are used directly in the hardware.
 * They indicate the number of cycles gated off in every full clock cycle.
 * Hence the corresponding clock rate is full clock / (1 + min_mmu_rate).
 * If bit 7 is set, the remaining value is multiplied by 16 for extra
 * slowness.
 */
#define MIN_MMU_RATE_DEFAULT (255U /* MMU_FULL / ((127 << 4) + 1) MHz */)

/**
 * Special rate used when full shallow sleep is not appropriate.
 */
#define REDUCED_MMU_RATE_DEFAULT (5U)

/** Full ungated MMU clock rate as supplied to the subsystem */
#define FULL_MMU_RATE_DEFAULT (0U)

/**
 * Maximum divider value for register CLKGEN_CORE_CLK_RATE for reducing
 * the subsystem clock to save power
 */
#define SHALLOW_SLEEP_CLKGEN_CORE_CLK_RATE_VALUE     15

#ifdef DORM_DEBUG_VARIABLES
extern uint16 MIN_MMU_RATE;
extern uint16 REDUCED_MMU_RATE;
extern uint16 FULL_MMU_RATE;
#else
#define MIN_MMU_RATE  MIN_MMU_RATE_DEFAULT
#define REDUCED_MMU_RATE REDUCED_MMU_RATE_DEFAULT
#define FULL_MMU_RATE  FULL_MMU_RATE_DEFAULT
#endif

/** Local configuration */
typedef struct dorm_cfg_t {
    /**
     * Until we're initialised, we won't sleep.
     */
    bool initialised;
    /**
     * The minimum rate we will allow for the MMU clock on
     * this processor.  It is in the units required by the
     * hardware:  (clock_rate / MHz) = 60 / (1 + min_mmu_rate).
     * 0 is therefore a sensible default.
     *
     * Note that this value may be set to 0 in the foreground
     * to ensure that the background does not subsequently
     * reduce the clock rate before exiting the sleep code,
     * hence tests for this are intrinsically race prone.
     */
    volatile uint16 min_mmu_rate;
} dorm_cfg_t;

extern dorm_cfg_t dorm_cfg;

/**
 * Bits in dorm_wake_requested.
 */
typedef enum dorm_wake_t
{
    /** Someone outside dorm wants us to wake. */
    DORM_WAKE_EXTERNAL = 0x0001,
    /** Dorm's own timer went off. */
    DORM_WAKE_TIMER    = 0x0002,
    /**
     * An interrupt caused us to wake the background.
     * We don't necessarily want to leave dorm, however
     * (depending whether DORM_WAKE_EXTERNAL is also set).
     */
    DORM_WAKE_INTERRUPT = 0x0004
} dorm_wake_t;

/**
 * Flag indicating whether there is an outstanding wakeup request.
 *
 * This is set in the foreground, hence is volatile.  Although
 * reads and writes to the variable as a whole are atomic the
 * background needs to be careful handling individual bits.
 */
extern volatile uint16 dorm_wake_requested;

/**
 * In addition to preventing a race with the other processor setting
 * a different minimum CPU rate, we need to be careful in case the
 * foreground has told us not to enter shallow sleep.  Hence
 * we need to lock out the foreground of the local processor, too.
 */
#define DORM_ENTER_CRITICAL_SECTION()                   \
    block_interrupt_levels(INT_LEVELS)
#define DORM_LEAVE_CRITICAL_SECTION()                   \
    unblock_interrupt_levels(INT_LEVELS)

#ifdef DORM_DEBUG_VARIABLES
/*
 * Make key variables external for easier access when debugging.
 */
#define DORM_STATIC
#define DORM_CONST

extern TIME dorm_sleep_latest;
#else
#define DORM_STATIC static
#define DORM_CONST const
#endif

/**
 * Global defined in interrupt.asm that lets us divide down the subsystem
 * clock when entering shallow sleep. This has to be coordinated with other
 * users of the clock such as the other processor and things such as USB that
 * need fast access to Apps memory. The deep sleep entry point coordinates
 * these since it is only reached when both processors have no work and it
 * isn't used when the USB transport is active.
 */
extern uint32 shallow_sleep_clk_div;

/**
 * Pick the shallow or deep sleep mode
 *
 * Chooses the minimum clock rate for the current processor.
 * latest is the background wake up time; this will be combined
 * with the itime wakeup time to get the earliest wakeup for deep
 * sleep.
 */
extern void dorm_pick_clock(TIME *latest);

/**
 * Set the minimum rate for the MMU clock
 *
 * This is as in \c dorm_cfg.min_mmu_rate.
 *
 * The "MMU clock" is the nearest thing we have to a processor clock,
 * although it controls other things and its rate may be boosted
 * by requests from peripherals (see init_dorm).
 *
 * This code requires the foreground to be locked out since that
 * may request a maximum clock and we don't then want to reduce it.
 *
 * We never reduce the maximum clock rate, since that doesn't
 * gain anything either.
 */
extern void dorm_set_reduced_mmu_clock(void);

/**
 * Set the full rate for the MMU clock
 * The "MMU clock" is the nearest thing we have to a processor clock,
 * although it controls other things and its rate may be boosted
 * by requests from peripherals (see \c init_dorm()).
 *
 * When either CPU is in action, we always use the full clock rate.
 * There is typically nothing to be gained by performing any activity
 * more slowly than the full rate since it just takes longer.  There
 * is no race with the other processor when setting full rate as
 * long as we advise the other processor first that we need it.
 *
 * Note this function needs to be called after \c dorm_pick_clock() even if
 * \c dorm_set_reduced_mmu_clock() was never called, in order to restore
 * the current CPU's notion of the clock speed as seen by the other
 * CPU.
 */
extern void dorm_set_full_mmu_clock(void);

/**
 * Common code for waking
 *
 * Used internally inside dorm for waking.  The caller must
 * set appropriate bits in \c dorm_wake_requested indicating the
 * nature of the wakeup.
 */
extern void dorm_wake_common(void);


/** Is deep sleep allowed locally?
 *
 * See if we can enter deep sleep, passing our times to the
 * Curator if necessary.  This function calls \c subsleep_ready(),
 * so on a \c TRUE return we need to call the other subsleep messages
 * in sequence.  The times are passed in as pointers in case the
 * function wishes to change them.
 *
 * *\param earliestp is a pointer to the earliest time we'd like to
 * be woken, not counting asynchronous wakes.
 *
 * *\param latestp is the time by which we require to be woken.  As
 * this is background code, some slop is allowed in waking.
 *
 * \return TRUE if we can enter deep sleep, else FALSE.
 */
extern bool dorm_hydra_check_deep_sleep(TIME *earliestp, TIME *latestp);

/**
 * Enter shallow sleep
 * Implemented in fw in asm
 * No parameters or result
 */
extern void enter_shallow_sleep(void);

/**
 * Enter shallow sleep with the subsystem clock rate set to minimum in order
 * to reduce power. P0 is responsible for the clock divider register so on
 * P1 this is just the normal shallow sleep entry.
 */
#define enter_shallow_sleep_with_reduced_clock()  enter_shallow_sleep()


#endif
