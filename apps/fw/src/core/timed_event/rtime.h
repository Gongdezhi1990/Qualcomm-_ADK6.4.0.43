/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

#ifndef RTIME_H
#define RTIME_H

#include "hydra/hydra_types.h"


/** @{ */
/*lint -e750 -e962*/ #define IO_DEFS_MODULE_K32_TIMERS
#include "hal/hal.h"
#include "hal/haltime.h"

#include "timed_event/rtime_types.h"

/**
 * Get the current system time
 *
 * Returns the current system time.
 */
#define get_time()        hal_get_time()

/**
 * Get the current system time to 16-bit accuracy
 *
 * Returns the lowest 16 bits of the system time.  This can be optimised
 * to a single register read.
 */
#define get_time16()     hal_get_reg_timer_time_lsw()

/** @} */

/**
 * \name time constants.
 *
 * CUR-2276 Moved MILLISECOND etc to haltime.h so that we can have only one
 * set of defines, without hal relying on this higher layer timed_event.
 */

/**
 * Add two time values
 *
 * Returns the sum of "t1 and "t2".  See \ref rtime_clock for notes on
 * what this means.
 *
 * \note NOTES
 *
 * Implemented as a macro, because it's trivial.
 *
 * Typically, this will be used to add an \c INTERVAL to a \c TIME,
 * returning a new \c TIME.  Lint will complain about this if the result
 * needs to be used as a \c TIME because at that point the result has
 * become signed and it doesn't know it's OK to treat the same bits as
 * an unsigned number to get a \c TIME.  Because we are happy that the
 * result always needs to be treated as a \c TIME whatever bits it contains
 * (or CSR would have turned into dust long ago) we'll cast both
 * arguments to \c TIME to shut it up.
 *
 * Actually, adding a \c TIME to a \c TIME is exactly the one
 * operation that \e doesn't make sense here!  I don't know of a
 * way to convince lint of that.  When the new C++ library comes along
 * in 2031 this will be fixed.  Logically, we would then have
 *\verbatim
TIME TIME::operator+(INTERVAL t2);
TIME INTERVAL::operator+(TIME t2);
INTERVAL INTERVAL::operator+(INTERVAL t2);
\endverbatim
 */
#define time_add(t1, t2) ((TIME)(t1) + (TIME)(t2))

/**
 * Subtract two time values
 *
 * Returns "t1 - t2".  See \ref rtime_clock for notes on what this means.
 *
 * \note NOTES
 *
 * Implemented as a macro, because it's trivial.
 *
 * This returns a signed number for correct use in comparisons.
 */
#define time_sub(t1, t2) ((INTERVAL) (t1) - (INTERVAL) (t2))

/**
 * Compare two time values
 *
 * Compares the two time values "t1" and "t2".  See \ref rtime_clock for
 * notes on what this means.
 *
 * Returns TRUE if "t1" equals "t2", else FALSE.
 *
 * NOTE
 *
 * Implemented as a macro, because it's trivial.
 */
#define time_eq(t1, t2)  ((t1) == (t2))

/**
 * Compare two time values
 *
 * Compares the time values "t1" and "t2".  See \ref rtime_clock for
 * notes on what this means.
 *
 * Returns TRUE if "t1" is greater than "t2", else FALSE.
 *
 * NOTE
 *
 * Implemented as a macro, because it's trivial.
 */
#define time_gt(t1, t2) (time_sub((t1), (t2)) > 0)

/**
 * Compare two time values
 *
 * Compares the time values "t1" and "t2".  See \ref rtime_clock for
 * notes on what this means.
 *
 * Returns TRUE if "t1" is greater than, or equal to, "t2", else FALSE.
 *
 * NOTE
 *
 * Implemented as a macro, because it's trivial.
 */
#define time_ge(t1, t2) (time_sub((t1), (t2)) >= 0)

/**
 * Compare two time values
 *
 * Compares the time values "t1" and "t2".  See \ref rtime_clock for
 * notes on what this means.
 *
 * Returns TRUE if "t1" is less than "t2", else FALSE.
 *
 * NOTE
 *
 * Implemented as a macro, because it's trivial.
 */
#define time_lt(t1, t2) (time_sub((t1), (t2)) < 0)

/**
 * Compare two time values
 *
 * Compares the time values "t1" and "t2".  See \ref rtime_clock for
 * notes on what this means.
 *
 * Returns TRUE if "t1" is less than, or equal to, "t2", else FALSE.
 *
 * NOTE
 *
 * Implemented as a macro, because it's trivial.
 */
#define time_le(t1, t2) (time_sub((t1), (t2)) <= 0)

#endif /* RTIME_H */
