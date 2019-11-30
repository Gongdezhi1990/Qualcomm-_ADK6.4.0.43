/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */


#ifndef ITIME_KAL_H_
#define ITIME_KAL_H_

#include "itime_kal/itime_subsystems.h"
#include "timed_event/rtime.h" /* For INTERVAL */

/** Type of pointer to callback functions */
typedef void (*itime_callback_fn)(void);

/**
 * Helper macro to forward an expansion of the event list to the appropriate
 * formatting macro */
#define ITIME_EVENT_HI(m,n) \
    ITIME_EVENT_EXPAND_HI_##m n
#define ITIME_EVENT_LO(m,n) \
    ITIME_EVENT_EXPAND_LO_##m n


/** Allocate a unique identifier to each itimed event client */
typedef enum itid
{
    /** Generate a simple (comma separated) list of itimed event
     * identifiers, but make sure that the LO come before the HI. */
#define ITIME_EVENT_EXPAND_HI_IDS_LO(t, f)
#define ITIME_EVENT_EXPAND_LO_IDS_LO(t, f) t,
    ITIME_EVENT_LIST(IDS_LO)
#define ITIME_EVENT_EXPAND_HI_IDS_HI(t, f) t,
#define ITIME_EVENT_EXPAND_LO_IDS_HI(t, f)
    ITIME_EVENT_LIST(IDS_HI)

    /** End of identifiers marker - do not use for a real identifier */
    itids
} itid;

/**
 * The following expressions use tricks to count the number of
 * definitions.  This causes flexelint to throw one of its more serious
 * wobblies.  After over an hour of fighting with it and attempting to
 * use variants of the --emacro definition that should be appropriate
 * here, the following suppression was the first to work.  973 is a
 * warning about the use of a unary operator; in the expansion the
 * operator is actually binary.
 */

#ifndef DOXYGEN_IGNORE /* Doxygen can't cope with macros used here */
/** Count low IDs: 0 for every high ID, 1 for every low ID */
#define /*lint --e{973} */ ITIME_EVENT_EXPAND_HI_COUNT_LO(i,f) +0
#define /*lint --e{973} */ ITIME_EVENT_EXPAND_LO_COUNT_LO(i,f) +1
#define ITIME_LO_IDS (0 ITIME_EVENT_LIST(COUNT_LO))
#else
#define ITIME_LO_IDS (1)
#endif /*DOXYGEN_IGNORE */

#ifndef DOXYGEN_IGNORE /* Doxygen can't cope with macros used here */
/** Count high IDs: 1 for every high ID, 0 for every low ID */
#define /*lint --e{973} */ ITIME_EVENT_EXPAND_HI_COUNT_HI(i,f) +1
#define /*lint --e{973} */ ITIME_EVENT_EXPAND_LO_COUNT_HI(i,f) +0
#define ITIME_HI_IDS (0 ITIME_EVENT_LIST(COUNT_HI))
#else
#define ITIME_HI_IDS (1)
#endif /* DOXYGEN_IGNORE */

/**
 * Initialise the module
 *
 * Causes pl_timers initialisation
 */
extern void init_itime(void);

/**
 * Schedule timed interrupt event
 *
 * Causes the handler function declared together with the itid "id" to
 * be called at or after period "delay".
 *
 * "delay" must be less than half the range of type "INTERVAL".
 *
 * This function can be called from all interrupt levels on the machine.
 *
 *  The identifier "id" can be used by cancel_itimed_event().  However,
 *  the function can be called even if the itimed for "id" is already
 *  running.  This replaces the function reset_itimed_event_in.
 */
extern void itimed_event_in(INTERVAL delay, itid id);


/**
 * Schedule timed interrupt event
 *
 * This is identical to itimed_event_in, except that "when"
 * is an absolute time on the microsecond clock.
 *
 *  This can be used to replace a previous itimed event, in place
 *  of the function reset_itimed_event_at.
 */
extern void itimed_event_at(TIME when, itid id);


/**
 * Cancel a timed interrupt event
 *
 * Attempts to prevent the timed interrupt event with identifier "id"
 * scheduled to run at interrupt level "level" from occurring.
 *
 * See the notes with itimed_event_at() on the selection of event
 * identifiers.
 *
 * Returns TRUE if the event was cancelled, else FALSE.
 */
extern bool cancel_itimed_event(itid id);


#ifdef NEED_NEXT_ITIMED_EVENT
/* To implement this would take an extension to pl_timers, so we'll leave
 * it out until we find we need it */
/**
 * When is the next timed interrupt event
 *
 * Writes into "when" the time of the next timed interrupt, if any is pending.
 *
 * This function is intended to be called from the background. (That's the
 * real background - level 0 - not the "levels 0 and 1" background used in
 * some of the IEEE 802 code descriptions.)
 *
 * Returns TRUE if a value was written into "when", else FALSE. The function
 * returns FALSE if no timed interrupt events are pending.
 *
 * NOTE
 *
 * The event may already have occurred by the time the caller reads the
 * value in "when".
 */
extern bool next_itimed_event(TIME *when);
#endif /* NEED_NEXT_ITIMED_EVENT */

#endif /* ITIME_KAL_H_ */
