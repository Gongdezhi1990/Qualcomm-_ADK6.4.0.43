/*****************************************************************************
*
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
*
*****************************************************************************/
/**
 * \file
 * Hydra Transaction Bus (Interface)
 */

#ifndef HYDRA_TXBUS_H
#define HYDRA_TXBUS_H

/*****************************************************************************
 * Interface Helpers
 ****************************************************************************/

/**
 * Integer divide (rounded up).
 */
#define HYDRA_DIV_UP(n,d)           (((n) + ((d) - 1)) / (d))

/**
 * 16 bit, uSecond resolution, wrapping counter suitable for
 * code section timing.
 *
 * \note Behaviour during sleep explicitly undefined.
 */
#ifdef __KALIMBA32__
#define HYDRA_USEC_COUNT() hal_get_time()
#else
#define HYDRA_USEC_COUNT() hal_get_time_lsw()
#endif

/**
 * Timed block of code.
 *
 * Time to uSec precision, [-1, +1) uSec tolerance, 16 bit range.
 *
 * This block does _not_ disable interrupts. Do that yourself if necessary
 * to have more accurate times.
 *
 * Example use:-
\verbatim

    uint16 elapsed;
    HYDRA_TIMED_BLOCK_BEGIN(&elapsed)
    {
       while (!some_condition) { do_something(); }
    }
    HYDRA_TIMED_BLOCK_END;
    if (elapsed > 1000) ...

\endverbatim
 */
#define HYDRA_TIMED_BLOCK_BEGIN(p_elapsed) \
do { \
    unsigned *p_timed_block_elapsed = (p_elapsed); \
    unsigned timed_block_start, timed_block_finish; \
    timed_block_start = HYDRA_USEC_COUNT(); \
    {

#define HYDRA_TIMED_BLOCK_END \
    } \
    timed_block_finish = HYDRA_USEC_COUNT(); \
    *p_timed_block_elapsed = timed_block_finish - timed_block_start; \
} while (0)

/*****************************************************************************
 * Interface.
 ****************************************************************************/

/*****************************************************************************
 * Hydra Transaction Bus Status Code
 ****************************************************************************/

/**
 * Hydra TXBus status code.
 *
 * \todo    Can we get type from some h/w header?
 */
typedef uint8 hydra_txbus_status;

/**
 * Hydra TXBus OK status code.
 *
 * \public \memberof hydra_txbus_status
 *
 * \todo    Can we get value/enum from some h/w header?
 */
#define HYDRA_TXBUS_STATUS_OK ((hydra_txbus_status) 0)

/**
 * Is this the Hydra TXBus OK status code?
 *
 * \public \memberof hydra_txbus_status
 */
#define hydra_txbus_status_is_ok(s) (HYDRA_TXBUS_STATUS_OK ==(s))

/*****************************************************************************
 * Hydra Transaction Bus
 ****************************************************************************/

/**
 * Storage for **** OCTET RESOLUTION **** Hydra transaction bus address.
 *
 * Anyone storing a 16 or 32 bit resolution address in one of these
 * will be shot (a hard-ware fact).
 */
typedef uint32 HYDRA_TXBUS_ADDR;

/**
 * Default/Design limit on time (microseconds) a cpu may be blocked for on a
 * transaction involving TXBUS read. This is a round trip time from
 * a worst case observed in digital simulations.
 */
#define HYDRA_TXBUS_MAX_RD_CYCLES   200

/**
 * Minimum effective clock speed when all parties are awake.
 */
#define HYDRA_TXBUS_MIN_MHZ         20

/**
 * Transaction bridge clocks taken for a transaction to get out
 * of the bridge in one bit mode including header and start/stop
 * bits.
 */
#define HYDRA_TBRIDGE_MAX_CLOCKS_PER_TRANSACTION    110

/**
 * Minimum likely clock speed of the transaction bridge
 */
#define HYDRA_TBRIDGE_MINIMUM_CLOCK_SPEED_MHZ       10

/**
 * Design limit of read transaction.
 *
 * Used by HYDRA_TXBUS_RD_BLOCK to detect timing violations.
 */
#define HYDRA_TXBUS_RD_LIMIT_USECS  \
    (HYDRA_DIV_UP(HYDRA_TXBUS_MAX_RD_CYCLES, HYDRA_TXBUS_MIN_MHZ) + \
     HYDRA_DIV_UP(HYDRA_TBRIDGE_MAX_CLOCKS_PER_TRANSACTION,         \
                  HYDRA_TBRIDGE_MINIMUM_CLOCK_SPEED_MHZ))

/**
 * Upper limit of read transaction.
 *
 * Used by HYDRA_TXBUS_RD_BLOCK to detect lockup.
 */
#define HYDRA_TXBUS_RD_TIMEOUT_USECS 1000

/**
 * Maximum time in micro-seconds we expect it to take to get an arbitrary
 * transaction sent on the bus. This may be going off-chip through the bridge.
 * If we exceed this we will panic.
 */
#define HYDRA_TXBUS_TRANSACTION_SEND_TIMEOUT_USECS      1000

/**
 * Time to allow a bus interrupt transaction to get onto the bus so
 * the hardware can be re-used. The transaction may be queued behind
 * one that is going off-chip so we use the same timeout as for that case.
 */
#define BUS_INTERRUPT_HW_SEND_TIMEOUT_VALUE         HYDRA_TXBUS_RD_LIMIT_USECS

#if defined(INSTALL_HYDRA_TXBUS_RD_TIMING)

/**
 * Hydra TXbus state.
 *
 * Published to implement timing macros.
 */
extern struct hydra_txbus
{
    /**
     * Current txbus read time limit (uSecs).
     *
     * Reads should take _less_ than this amount of time.
     *
     * Initialised to HYDRA_TXBUS_MAX_RD_WAIT_US, but raised,
     * with a warning/fault if the design limit is ever hit.
     */
    uint16 rd_limit_usecs;
} hydra_txbus;

/**
 * Handle trxbus read time limit exceeded.
 *
 * Logs a warning/fault and raises the limit. Called from timing macros only.
 *
 * \private \memberof hydra_txbus
 */
extern void hydra_txbus_rd_time_limit_exceeded(
    /**
     * New limit.
     *
     * Should be 1 usec longer than the offending rd duration so that
     * further reads of the same duration do not trigger another warning.
     *
     * This is convenient for the caller 'cos the elapsed measurement has to
     * exceed the previous limit by 1 usec before certain it has really taken
     * too long because the resolution is only 1 uSec.
     */
    uint16 new_limit_usecs
);

#endif /* defined(INSTALL_HYDRA_TXBUS_RD_TIMING) */

/**
 * Hydra Transaction bus read macro.
 *
 * Supports timeout and optional monitoring and logging of read transactions
 * that take too long.
 *
 * Always Panics if elapsed > HYDRA_TXBUS_RD_TIMEOUT_USECS.
 *
 * If INSTALL_HYDRA_TXBUS_RD_TIMING then also warns if
 * elapsed > HYDRA_TXBUS_MAX_RD_WAIT_US.
 *
 * It is assumed that interrupts will already be disabled around any access to
 * respective hardware. If not the case then suspect timings will be the least
 * of your worries.
 *
 * Example use:-
 \verbatim

    HYDRA_TXBUS_RD(
        hal_set_reg_mmu_buffer_access_do_action(MMU_BUFFER_ACCESS_DO_ACTION_READ),
        !hal_get_mmu_buffer_access_status_active()
    );

 \endverbatim
 */
#if defined(INSTALL_HYDRA_TXBUS_RD_TIMING)

/**
 * Hydra Transaction bus read macro (with timing)
 *
 * Warns if elapsed > HYDRA_TXBUS_MAX_RD_WAIT_US
 * Panics if elapsed > HYDRA_TXBUS_RD_TIMEOUT_USECS
 */
#define HYDRA_TXBUS_RD(start_rd, has_rd_finished) \
do { \
    unsigned txbus_rd_elapsed = 0; \
    unsigned txbus_rd_start = HYDRA_USEC_COUNT(); \
    (start_rd); \
    while (!(has_rd_finished)) \
    { \
        txbus_rd_elapsed = HYDRA_USEC_COUNT() - txbus_rd_start; \
        if (txbus_rd_elapsed > HYDRA_TXBUS_RD_TIMEOUT_USECS) \
        { \
            panic_diatribe(PANIC_HYDRA_TXBUS_WAIT_TIMED_OUT, txbus_rd_elapsed); \
        } \
    } \
    if (txbus_rd_elapsed > hydra_txbus.rd_limit_usecs) \
    { \
        hydra_txbus_rd_time_limit_exceeded(txbus_rd_elapsed); \
    } \
} while (0)

#else /* !defined(INSTALL_HYDRA_TXBUS_RD_TIMING) */

/**
 * Hydra Transaction bus read macro (without timing)
 *
 * Panics if elapsed > HYDRA_TXBUS_RD_TIMEOUT_USECS
 */
#define HYDRA_TXBUS_RD(start_rd, has_rd_finished) \
do { \
    unsigned txbus_rd_start = HYDRA_USEC_COUNT(); \
    (start_rd); \
    while (!(has_rd_finished)) \
    { \
        unsigned txbus_rd_elapsed = HYDRA_USEC_COUNT() - txbus_rd_start; \
        if (txbus_rd_elapsed > HYDRA_TXBUS_RD_TIMEOUT_USECS) \
        { \
            panic_diatribe(PANIC_HYDRA_TXBUS_WAIT_TIMED_OUT, (DIATRIBE_TYPE)txbus_rd_elapsed); \
        } \
    } \
} while (0)

#endif /* !defined(INSTALL_HYDRA_TXBUS_RD_TIMING) */

/**
 * Polls for a maximum of timeout_usec to see if poll_complete becomes
 * TRUE. Panics with panic_id if the poll times out.
 */
#define HYDRA_POLL_WITH_TIMEOUT(poll_complete, timeout_usec, panic_id) \
    do { \
        unsigned rd_start = HYDRA_USEC_COUNT(); \
        while (!(poll_complete)) \
        { \
            unsigned rd_elapsed = HYDRA_USEC_COUNT() - rd_start; \
            if (rd_elapsed > (timeout_usec)) \
            { \
                panic_diatribe((panic_id), (DIATRIBE_TYPE)rd_elapsed); \
            } \
        } \
    } while (0)

#endif /* HYDRA_TXBUS_H */
