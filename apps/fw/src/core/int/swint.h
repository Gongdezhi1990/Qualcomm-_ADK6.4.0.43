/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Software interrupt manager
 */

#ifndef SWINT_H
#define SWINT_H

#include "hydra/hydra_types.h"
#include "hal/halint.h"
#include "int/int.h"


/** Indentifier for a software interrupt */
typedef uint16 swint_id;


/**
 * Configure a software interrupt source
 *
 * Configure a software interrupt to call "handler" at priority "level".
 * It is not necessary to enable the source separately.
 *
 * If no software interrupt sources are available then this function will
 * panic.
 *
 * Returns
 *
 * An identifier for this software interrupt. This should be passed to
 * generate_sw_interrupt() to trigger the interrupt, and may be passed to
 * unconfigure_sw_interrupt() when this interrupt is no longer required.
 */
extern swint_id configure_sw_interrupt(int_level level,
                                       void (* handler)(void));


/**
 * Unconfigure a software interrupt source
 *
 * Indicate that interrupt source "id" is no longer required. This makes
 * the source available for reallocation via configure_sw_interrupt().
 *
 * IMPLEMENTATION NOTE
 *
 * If PHY_LEAN_AND_MEAN is defined then disabling interrupts is not
 * supported - see comment for configure_interrupt().
 */
extern void unconfigure_sw_interrupt(swint_id id);


/**
 * Generate a software interrupt
 *
 * Generate a software interrupt with identifier "id". This causes the
 * handler registered with configure_sw_interrupt() to be called.
 */
extern void generate_sw_interrupt(swint_id id);


/**
 * Clear a pending software interrupt
 *
 * Clear any pending interrupt for the software interrupt with identifier
 * "id". This is intended for two purposes.
 *
 * The first use is to clear a background wakeup event (used to termainte
 * a SLEEP instruction). Since no interrupt handler is called in this case
 * it is necessary to explicitly clear the event using this function.
 *
 * The second use is for overcoming the problem of two events occurring on
 * the same interrupt source very close together. If the second event occurs
 * before the first has been acknowledged then the interrupt handler will
 * only be called once, but if the second event occurs after the first has
 * been acknowledged then the interrupt handler will be called twice.
 * Unfortunately, the software has no way to distinguish between these
 * two cases.
 *
 * If the interrupt handler will process all pending events from its source
 * then this function can be used to ensure that those events do not
 * generate an additional interrupt. Unfortunately, hardware restrictions
 * mean that this function may have no effect, so do not rely on the
 * interrupt handler not being called. In other words, it should be
 * considered an optional optimisation rather than guaranteed behaviour.
 */
extern void clear_sw_interrupt(swint_id id);


#endif /* SWINT_H */
