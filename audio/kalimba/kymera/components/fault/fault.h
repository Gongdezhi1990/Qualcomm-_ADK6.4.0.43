/****************************************************************************
 * Copyright (c) 2010 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file fault.h
 * \ingroup fault
 *
 * \brief
 * This is the public header file for the fault module. It should
 * be included in all modules that need to send faults to the host.
 */

#ifndef FAULT_H
#define FAULT_H

#ifndef NOCSR
/**
 * \defgroup fault Fault
 * \ingroup error_handling
 *
 * \brief
 * Attempts to report a run-time error condition.  Faults are normally
 * reported to the host via a CCP message.
 *
 *   It is tempting to think that each fault_diatribe() call sends a message to
 *   the host, but this is unworkable:
 *
 *  (a) When systems misbehave they tend to perform the same erroneous
 *      action repeatedly.  In this context, a faulty condition might be
 *      expected to provoke a burst of calls to fault_diatribe(), each with the
 *      same fault identifier.  The system probably won't have the
 *      bandwidth to send all of these to the host.  Also, a heavy load
 *      on comms to the host may be exactly the wrong thing to do when a
 *      system error condition arises.
 *
 *   (b) Calls to fault_diatribe() can be made from the system's various run
 *      levels (interrupts and background).  Thus the calls must be
 *      thread-safe.  We cannot affort to block interrupts for the long
 *      periods (probably) implied by the need to send each message to
 *      the host.
 *
 *   Hence, the fault subsystem has a different structure...
 *
 *  Calls to fault_diatribe() lodge error reports in a tiny database.  They do not
 *  directly cause a message to be sent to the host, except for the first
 *  occurrence.  The database collects multiple reports of the same fault
 *  (the information associated with that fault, namely the argument and
 *  the timestamp, is taken from the first occurrence of the fault, unless
 *  fault_update() is used).
 *
 *  A separate background entity regularly checks the database.  This
 *  makes a single report from any set of calls, and clears the
 *  corresponding entry from the database.
 *
 *  The database is of finite size.  If a call to fault_diatribe() would overflow
 *  the database, the fault is lost.
 *
 *  Each processor has its own database; this means that the same
 *  fault occurring on both processors will be reported twice.
 */
#endif /* NOCSR */

#include "hydra/hydra_types.h"

/** Extra fault codes that should not be published. Really, all of
 * these should become either real (documented faults) or they should
 * be fixed. */
#define FAULTIDS_EXTRA \
    /* Useless "can't be bothered" code. */ \
    fault_mystery = (int) 0x8000, \

/* Include the published fault codes */
#include "faultids.h"

/**
 * Get ready to groan
 *
 * Initialises the fault subsystem.
 *
 * This must be called before calls to any other function defined in this file.
 */
extern void init_fault(void);

/**
 * Protest and continue
 *
 * Attempt to produce an error message to the outside world.  This
 * message indicates a non-fatal error of some sort.  It is possible
 * that the error will hamper proper operation of the chip, but it is
 * assumed that the host interface and operating system components of
 * the chip are still functioning.
 *
 * The argument can be used to give extra information about the
 * error.  The way in which the argument is used is dependent on the
 * fault code.
 *
 * This function should be callable from any processor at any
 * interrupt level.
 *
 * See the description of fault_set_panic_on_fault(), which can modify
 * fault_diatribe()'s normal behaviour.
*/
extern void fault_diatribe(faultid confession, DIATRIBE_TYPE arg);
#ifdef FAULT_COY
#define /*lint -emacro(774, fault) */ /*lint -esym(123, fault) */ fault_diatribe(c,a) do { if (((uint16) c & 0x8000) == 0) fault_diatribe(c, a); } while (0)
#endif

/**
 * A function to return a 32 bit fault argument to the host.
 * Currently only returns a 16 bit one.
 */
#define fault32(confession, arg) fault_diatribe((confession), (uint16)(arg))

/**
 * Protest more authoritatively and continue
 *
 * Like fault_diatribe(), but the argument and timestamp override those
 * of any pending fault of the given type.
 */
extern void fault_update(faultid confession, DIATRIBE_TYPE arg);

/**
 * configure panic_on_fault
 *
 * A call to fault_diatribe() normally will (attempt to) send a message to the
 *  host, informing it of a survivable error event.  This function changes
 *  this behaviour such that a call to fault_diatribe() can lead directly to a call
 *  to panic().  This is intended only to support system debugging.
 *
 *  If "enable" is TRUE and if no call has been made to
 *  fault_set_panic_on_fault_code() then a call to fault_diatribe() will lead
 *  directly to a call to panic().
 *
 *  If "enable" is TRUE and if the last call
 *  to fault_set_panic_on_fault_code() did not have the argument
 *  "fault_none" then any call to fault_diatribe() will lead directly to a call
 *  to panic() if the fault_diatribe() call's argument matches the last argument
 *  to fault_set_panic_on_fault_code().
 *
 *  If "enable" is FALSE then normal fault_diatribe() behaviour is restored.
 */
extern void fault_set_panic_on_fault(bool enable);

/**
 * Obtain panic_on_fault configuration
 *
 * The value last written to fault_set_panic_on_fault(), or FALSE if no
 * such call has been made.
 */
extern bool fault_get_panic_on_fault(void);

/**
 * Refine panic_on_fault configuration
 *
 * See the description of fault_set_panic_on_fault().
 *
 * It would be more natural for "id" to be a faultid.  It is of type
 * uint16 to fit efficiently with mibsetfns.c.
 */
extern void fault_set_panic_on_fault_code(uint16 id);

/**
 * Obtain panic_on_fault refined config
 *
 * The value last written to fault_set_panic_on_fault_code(), or
 * "fault_none" if no such call has been made.
 *
 * It would be more natural for this function to return a faultid.  It
 * returns a uint16 to fit efficiently with mibgetfns.c.
 */
extern uint16 fault_get_panic_on_fault_code(void);

/* This stuff moved here so that it can appear in shared. */
#ifndef FAULT_TIMESTAMP_WIDTH
#define FAULT_TIMESTAMP_WIDTH 32
#endif
#if FAULT_TIMESTAMP_WIDTH != 0 && FAULT_TIMESTAMP_WIDTH != 16 && FAULT_TIMESTAMP_WIDTH != 32
#error FAULT_TIMESTAMP_WIDTH must be 0, 16 or 32
#endif

/**
 * An element of the fault database.
 */
typedef struct {
    faultid f;                 /** A fault identifier. */
    DIATRIBE_TYPE arg;         /** A fault argument. */
#if FAULT_TIMESTAMP_WIDTH == 32
    uint32 st;                 /** Time of first report. */
#endif
#if FAULT_TIMESTAMP_WIDTH == 16
    uint16 st;                 /** Time of first report (time>>8). */
#endif
    unsigned int n : 15;       /** Number of instances of f since last published. */
#define MAX_FI_REPORTS 32767   /** Limit of number of faults to record. */
    unsigned int h : 1;        /** Initial DebugWord16_indication generated? */
#define MAX_FI_TOTAL 65535     /** Limit of total number of faults to record. */
    unsigned short tn;         /** Total number of instances of f. */
} FAULTINFO;

#define NFI 4                  /** Size of faultinfo[] database. */

#endif /* FAULT_H */
