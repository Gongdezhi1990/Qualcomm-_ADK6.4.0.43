/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 *
 * Main public header for the \c timed_event module.
 *
 * Note that basic time-related definitions are currently in \c rtime.h
 * in this module.  Usually this header should be included in
 * preference.
 *
 */

#ifndef TIMED_EVENT_H
#define TIMED_EVENT_H

#include "hydra/hydra_types.h"
#include "hydra/hydra_macros.h"
#include "timed_event/rtime.h"
#include "pl_timers/pl_timers.h"




/**
 * The maximum number of timed events.
 * Allow this to be overridden by a configuration file.
 */
#ifndef MAX_TIMED_EVENTS
#define MAX_TIMED_EVENTS (100)
#endif /* MAX_TIMED_EVENTS */


/** Callback function for timed events */
typedef void (*timed_event_fn_t)(uint16 fniarg, void *fnvarg);

typedef tTimerId tid;
/** Active tids are non-zero, so zero signals "not a timer id". */
#define NO_TID   ((tid)(0))

/**
 * Call function at a particular time
 *
 * Causes the void function "fn" to be called with the arguments
 * "fniarg" and "fnvarg" at time "when".
 *
 * The period to "when" must be less than half the range of a time.
 *
 * timed_event_at() does nothing with "fniarg" and "fnvarg" except
 * deliver them via a call to "fn()".   (Unless cancel_timed_event() is
 * used to prevent delivery.)
 *
 * Returns a timed event identifier, which can be used cancel the timed event
 * with a call to cancel_timed_event().
 *
 * \param when Time at which timed event should be called
 * \param fn Pointer to function to call at this time
 * \param fniarg Integer argument to pass to the handler
 * \param fnvarg Opaque pointer to pass to the handler
 * \return tid The ID of the created timed event

 * \note
 *
 * The function will be called at or after "when"; the actual delay will
 * depend on the timing behaviour of the scheduler's tasks.
 */
extern tid timed_event_at(TIME when, timed_event_fn_t fn,
                          uint16 fniarg, void *fnvarg);
#define timed_event_at(when, fn, fniarg, fnvarg) \
        timed_event_at_between(when, when, fn, fniarg, fnvarg)


/**
 * Call function after delay
 *
 * Causes the void function "fn" to be called with arguments "fniarg"
 * and "fnvarg" after "delay" has elapsed.
 *
 * Note that "delay" is signed.  Negative numbers represent times in the
 * past.
 *
 * timed_event_in() does nothing with "fniarg" and "fnvarg" except
 * deliver them via a call to "fn()".   (Unless cancel_timed_event()
 * is used to prevent delivery.)
 *
 * Returns a timed event identifier, which can be used cancel the timed
 * event with a call to cancel_timed_event().
 *
 * \param delay Interval to wait until timed event should be called
 * \param fn Pointer to function to call at this time
 * \param fniarg Integer argument to pass to the handler
 * \param fnvarg Opaque pointer to pass to the handler
 * \return tid The ID of the created timed event

 * \note
 *
 * The function will be called at or after "delay" has passed; the
 * actual delay will depend on the timing behaviour of the scheduler's
 * tasks.
 */
extern tid timed_event_in(INTERVAL delay, timed_event_fn_t fn,
                          uint16 fniarg, void *fnvarg);
#define timed_event_in(delay, fn, fniarg, fnvarg) \
                timed_event_in_between(delay, delay, fn, fniarg, fnvarg)

/**
 * Cancel a timed event
 *
 * Attempts to prevent the timed event with identifier "eventid" from
 * occurring.
 *
 * If the function returns TRUE, and if "pmi" and/or "pmv" are non-null,
 * then these pointers are used to return the corresponding arguments
 * provided to the put_message() call.
 * \param eventid The ID of the event
 * \param pmi Pointer to space to return integer arg in (if desired)
 * \param pmv Pointer to space to return pointer arg in (if desired) *
 * \return TRUE if the event was cancelled.   FALSE if the event has already
 * occurred or if the "eventid" was invalid.
 *
 * \note
 *
 * If the "pmv" to put_message() carried a pmalloc()ed chunk of memory
 * then the caller of cancel_timed_event() must obtain and pfree() the
 * memory to prevent a memory leak.
 */
extern bool cancel_timed_event(tid eventid, uint16 *pmi, void **pmv);
#define cancel_timed_event(eventid, pmi, pmv) \
                        timer_cancel_event_ret(eventid, pmi, pmv)
/**
 * Cancel all timed events, including casual timed events.
 *
 * This is present because the test harness requires it, however is
 * available for use generally.
 *
 * (Note that in earlier versions of the test harness it was called
 * \c cancel_all_timers().  It has been renamed to make it clear that
 * it refers to timed events as managed by this module.)
 */
extern void cancel_all_timed_events(void);
#define cancel_all_timed_events() timer_cancel_all_bg()

/**
 * Destroy a timed event
 *
 * If the timed event identified by "*eventid" is (marked as) running,
 * cancel it, and change "*eventid" to NO_TID.
 *
 *      This does nothing if *eventid is NO_TID.
 *
 * \param eventid Pointer to tid to cancel; on return points to NO_TID if
 * the timed event was cancelled
 */
extern void destroy_timed_event(tid *eventid);

/**
 * Call function between two times
 *
 * This behaves exactly as timed_event_at(), except that the time at
 * which the function is to run is chosen, by the scheduler, to be
 * between "earliest" and "latest".
 *
 * The scheduler chooses to run the function at a time that optimises
 * the chip's power consumption.
 * \param earliest earliest time at which timed event should be called
 * \param latest latest time at which timed event should be called
 * \param fn Pointer to function to call at this time
 * \param fniarg Integer argument to pass to the handler
 * \param fnvarg Opaque pointer to pass to the handler
 * \return tid The ID of the created timed event
 */
extern tid timed_event_at_between(
    TIME earliest,
    TIME latest,
    timed_event_fn_t fn,
    uint16 fniarg,
    void *fnvarg);

#define timed_event_at_between(earliest, latest, fn, fniarg, fnvarg) \
   timer_schedule_bg_event_at_between_alt(earliest, latest, fn, fniarg, fnvarg)

/**
 * Call function between two delays
 *
 * This behaves exactly as timed_event_in(), except that the delay
 * before the function is to run is chosen, by the scheduler, to be
 * between "smallest_delay" and "largest_delay".
 *
 * The scheduler chooses to run the function at a time that optimises
 * the chip's power consumption.
 *
 * \param smallest_delay shortest wait before timed event should be called
 * \param largest_delay longest wait before timed event should be called
 * \param fn Pointer to function to call at this time
 * \param fniarg Integer argument to pass to the handler
 * \param fnvarg Opaque pointer to pass to the handler
 * \return tid The ID of the created timed event
 */
extern tid timed_event_in_between(
    INTERVAL smallest_delay,
    INTERVAL largest_delay,
    timed_event_fn_t fn,
    uint16 fniarg,
    void *fnvarg);

#define timed_event_in_between(smallest_delay, largest_delay, \
                               fn, fniarg, fnvarg) \
   timer_schedule_bg_event_in_between_alt(smallest_delay, largest_delay, \
                                          fn, fniarg, fnvarg)

/**
 * When is an event scheduled to take place?
 *
 * If "tid" corresponds to a real event, set "*event_time" to
 * the time the event is due to take place and return TRUE; else
 * return FALSE.  Note that *event_time may be in the past, in which
 * case the event is due but has not yet taken place.
 *
 * \param id tid of event in question
 * \param event_time pointer to time at which specified event is to run, if
 * the event exists.
 * \return TRUE if the event exists else FALSE
 */
extern bool time_of_timed_event(tid id, TIME *event_time);

#ifdef USE_CURRENT_TIMED_EVENT_INTERFACE
/**
 * When was the current event scheduled?
 *
 * Returns the time of the current timed event to go off in
 * "*sched_time".  If no event is in progress, returns FALSE
 * and doesn't record a time.
 *
 * Note this is the scheduled time, not the actual time.  This
 * is deliberate, to make it easy for timed events to reschedule
 * themselves at well-defined intervals.
 *
 * Returns \c TRUE if a timed event was in progress, else \c FALSE, in which
 * case sched_time is not changed.
 */
extern bool time_of_current_timed_event(TIME *sched_time);

/**
 * What is the tid of the current timed event?
 *
 * Return the tid if an event is in progress, else \c NO_TID.
 */
extern tid tid_of_current_timed_event(void);
#endif /* USE_CURRENT_TIMED_EVENT_INTERFACE */

/**
 * Return tid for event set to call function
 *
 * Given the callback function for a timed event, returns the TID
 * corresponding to the event set to call that function.
 *
 * If fniargp is non-NULL, *fniargp must match the uint16 argument
 * to the timed event.  If fnvargp is non-NULL, *fnvargp must
 * match the void * argument to the timed event.
 *
 * Note this has no handling for multiple events which might
 * call the same function.  It's up to the calling function to
 * decide whether there may be multiple timed events with
 * the same callback.
 *
 * \param fn Handler function of sought-after timed event
 * \param fniargp Pointer to (copy of) the integer argument (can be NULL)
 * \param fnvargp Pointer to the pointer argument (can be NULL)
 * \return TID for first event found, else NO_TID.
 */
extern tid find_timed_event(timed_event_fn_t fn, uint16 *fniargp,
                            void **fnvargp);

#define find_timed_event(fn, fniargp, fnvargp) \
    timer_find_first_bg_event_by_fn_alt(fn, fniargp, fnvargp)

#endif /* TIMED_EVENT_H */
