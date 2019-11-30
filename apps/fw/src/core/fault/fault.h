#ifndef FAULT_H
#define FAULT_H

/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Public header file for the fault module.
*/


#include "hydra/hydra_types.h"
/*#include "itime/itime.h"*/

/** Extra fault codes that should not be published. Really, all of these should
 * become either real (documented faults) or they should be fixed. */
#define FAULTIDS_EXTRA \
    /* Useless "can't be bothered" code. */ \
    fault_mystery = (int) 0x8000, \

/* Include the published fault codes */
#include "faultids.h"

/**
 * Initialises the fault subsystem.
 * This must be called before calls to any other function defined in this file.
 */
extern void init_fault(void);

/**
 * Attempt to produce an error message to the outside world.  This message
 * indicates a non-fatal error of some sort. It is possible that the error will
 * hamper proper operation of the chip, but it is assumed that the host
 * interface and operating system components of the chip are still functioning.
 * The argument can be used to give extra information about the error. The way
 * in which the argument is used is dependent on the fault code.
 * This function should be callable from any processor at any interrupt level.
 * See the description of fault_set_panic_on_fault(), which can modify
 * fault_diatribe()'s normal behaviour.
 * \param confession Fault ID.
 * \param arg Fault argument.
 */
extern void fault_diatribe(faultid confession, DIATRIBE_TYPE arg);
#ifdef FAULT_COY
#define /*lint -emacro(774, fault) */ /*lint -esym(123, fault) */ fault_diatribe(c,a) do { if (((uint16) c & 0x8000) == 0) fault_diatribe(c, a); } while (0)
#endif

/**
 * Sorthand maco for raising a simple fault.
 * \param confession Fault ID
 */
#define fault(confession) fault_diatribe(confession, (DIATRIBE_TYPE)0)

/**
 * Like fault_diatribe(), but the argument and timestamp override those of any
 * pending fault of the given type.
 */
extern void fault_update(faultid confession, DIATRIBE_TYPE arg);

/**
 * A call to fault_diatribe() normally will (attempt to) send a message to the
 * host, informing it of a survivable error event. This function changes this
 * behaviour such that a call to fault_diatribe() can lead directly to a call
 * to panic(). This is intended only to support system debugging.
 * \param enable If "enable" is TRUE and if no call has been made to
 *               fault_set_panic_on_fault_code() then a call to
 *               fault_diatribe() will lead directly to a call to panic(). If
 *               "enable" is TRUE and if the last call to
 *               fault_set_panic_on_fault_code() did not have the argument
 *               "fault_none" then any call to fault_diatribe() will lead
 *               directly to a call to panic() if the fault_diatribe() call's
 *               argument matches the last argument to
 *               fault_set_panic_on_fault_code(). If "enable" is FALSE then
 *               normal fault_diatribe() behaviour is restored.
 */
extern void fault_set_panic_on_fault(bool enable);

/**
 * Obtain panic_on_fault configuration
 * \return The value last written to fault_set_panic_on_fault(), or FALSE if no
 *         such call has been made.
 */
extern bool fault_get_panic_on_fault(void);

/**
 * Refine panic_on_fault configuration. See the description of
 * fault_set_panic_on_fault().
 * \param id Fault ID.
 * \note It would be more natural for "id" to be a faultid. It is of type
 *       uint16 to fit efficiently with mibsetfns.c.
 */
extern void fault_set_panic_on_fault_code(uint16 id);

/**
 * Obtain panic_on_fault refined config.
 * \return The value last written to fault_set_panic_on_fault_code(), or
 *         "fault_none" if no such call has been made.
 * \note It would be more natural for this function to return a faultid. It
 *       returns a uint16 to fit efficiently with mibgetfns.c.
 */
extern uint16 fault_get_panic_on_fault_code(void);

/**
 * Entry point of the IPC fault reports from P1. This is designed to be called
 * only by the IPC module.
 * \param id Fault ID.
 * \param n Number of occurences.
 * \param t Timestamp.
 * \param arg Fault argument.
 */
extern void fault_from_p1(uint16 id, uint16 n, uint32 t, uint32 arg);


#endif /* FAULT_H */
