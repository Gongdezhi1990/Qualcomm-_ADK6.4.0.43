/*****************************************************************************
*
* Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
*
*****************************************************************************/
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
/**
 * \defgroup timed_event_oxygen timed_event_oxygen
 * \ingroup core
 *
 * This module provides "scheduler" timed events primarily for use within
 * firmware.  These events are executed from the scheduler's main loop, but
 * note that unlike CarlOS, the Oxygen scheduler may execute its main loop
 * pre-emptively.  Hence these timed events are not strictly "background"
 * events as in the CarlOS world and without due care it is possible in
 * principle for any timed event handler to pre-empt any other since there is
 * no notion of priority for timed events.  However, with suitable limitations
 * on when timed events can be run and what priority level they can be set
 * from it is possible to exclude pre-emption.  This is enabled by the
 * \c SCHED_NO_TIMER_PREEMPTION definition.
 *
 * Note that this module is a thin interface to pl_timers, a module inherited
 * from and shared with the Audio firmware.
 *
 * Functions here divide roughly into two parts.
 * - Functions for manipulating timed events (\ref
 *   timed_event_main_api).  This interface is available throughout the
 *   firmware.  Timed events are allocated from the \c pmalloc module;
 *   this is the only significant resource limitation on their use.
 * - Functions for integrating timed events into the firmware scheduler
 *   (\ref timed_event_scheduler_api).
 *
 * \section timed_event_history Brief historical notes
 *
 * (This section could also be labelled "Excuses".)
 *
 * Timed events were orignally a core part of the original BlueCore
 * scheduler and its descendants, referred to as \c CarlOS.  Because
 * this was considered a core part of the operating system no
 * attempt was made to impose a modular naming convention on functions,
 * other than the fact that most functions contain the string
 * \c timed_event.
 *
 * The present module has been separated from \c sched in order
 * to allow it to be integrated into other schedulers, but the
 * function names have mostly been retained except where this
 * would cause notable inconvenience or confusion.
 *
 * \section timed_event_main_api Main API for timed events.
 *
 * This section outlines the features available for clients of the timed
 * event module.
 *
 * \subsection timed_event_setting_api Setting the two types of timed event
 *
 * There are two types of timed event: normal and casual.
 *
 * A normal timed event has a single time at which the event goes off.
 * (For restrictions and other notes on the use of time within the
 * firmware, see \ref rtime_clock.)  It is important to realise that the
 * mechanism is non-preemptive; the scheduler may execute this timed
 * event at any point after the time has been reached, depending on
 * other tasks.  Normal timed events are run in time order, i.e. if
 * event A is scheduled before event B, event A will run before event B.
 *
 * Normal timed events are set by means of the functions \c
 * timed_event_at() and \c timed_event_in().  The former is slightly
 * more efficient if a the target time is already known.
 *
 * Casual timed events have two times, an earliest and a latest.  The
 * timed event mechanism may cause the event to be called at any point
 * from the earliest time; the latest time is treated similarly to the
 * single time recorded for normal timed events (i.e. it is \e not the
 * latest point at which a timed event will actually be started, just
 * the latest point at it which the scheduler will consider it ready for
 * use).  On Hydra systems the main use of casual timed events is to
 * allow the Curator's deep sleep manager to coalesce events to allow
 * the maximum deep sleep period with the minimum number of wakes.  When
 * multiple subsystems are active, this is the principal mechanism for
 * ensuring scheduling efficiency, so should be used whenever possible.
 * As a general rule of thumb, the earliest time should be no later than
 * half the time into the future of the latest time at the point where
 * the event is set; any less leeway is likely to cause the scheduler
 * difficulties in ensuring efficiency.  (For events set further than a
 * few seconds into the future, allowing a leeway of at least a couple of
 * seconds between the earliest and latest times should be sufficient.)
 *
 * Casual timed events are set by means of the functions \c
 * timed_event_at_between() and \c timed_event_in_between().
 *
 * pl_timers orders casual events by their \e latest time
 *
 * \subsection timed_event_tid Timed event identifer (tid)
 *
 * The functions that set both types of timed event return a \c tid,
 * an unsigned integer that can be used to refer to the timed event.
 * The definition \c NO_TID is guaranteed never to refer to a valid
 * timer.
 *
 * A \c tid is not guaranteed to be unique, though the code makes
 * sufficient effort that it is unlikely a duplicate will be used, and
 * long experience suggests this is good enough in firmware usage.
 * See the implementation of \c timed_event_get_id().
 *
 * It is not necessary to know whether the underlying event is
 * a normal or casual timed event to use the \c tid.  In fact, internally,
 * normal timed events are simply casual events with earliest = latest.
 *
 * \subsection timed_event_args Arguments to timed events
 *
 * The handler passed when setting a future timed event is recorded
 * only for that occurrence of the timed event.  This is rather
 * different from the interrupt timer, \c itime, where handlers
 * are restricted to a predefined set.
 *
 * When setting future timed events, a \c uint16 and a \c void *
 * argument are passed; the same arguments will be passed to the timed
 * event handler when it is run.  The arguments are not examined by the
 * timed event code and are entirely for the convenience of the timed
 * event itself, to prevent it needing extra long-term storage of its
 * own.
 *
 * The arguments may also be used to find the \c tid of a timed
 * event; see \ref timed_event_info_api.
 *
 * \subsection timed_event_cancelling_api Cancelling timed events
 *
 * A timed event can be cancelled, if its \c tid is known, by
 * a call to cancel_timed_event().  See \ref timed_event_info_api
 * for how to find a \c tid.
 *
 * \subsubsection not_implemented_cancelling_fns Unimplemented timed event
 * cancelling functions
 * \e {These functions are not implemented in timed_event_oxygen but were
 * present in the original CarlOS timed_event interface}
 * \c destroy_timed_event() passes a pointer to a
 * \c tid which will be set to \c NO_TID if the event is found and cancelled.
 * This is a minor enhancement on top of \c cancel_timed_event().
 *
 * \c cancel_all_timed_events() can be used to cancel every known timed
 * event.  This is designed for use in unit tests, but may be used in
 * firmware if desired.
 *
 * \subsection timed_event_info_api Information about timed events
 *
 * \c time_of_timed_event() returns the time of a timed event that has
 * not yet been run, by means of its \c tid.
 *
 * \c find_timed_event() allows a timed event to be located
 * by means of its handler.  Optionally, pointers to the \c uint16 and
 * \c void * arguments may also be passed.  If the caller happens
 * to know that all timed events using that handler have a unique
 * value for the \c uint16 argument, this serves as unique way
 * of identifying the timed event (though less efficiently than
 * by the \c tid).

 * \subsubsection not_implemented_info_fns Unimplemented timed event
 * info functions
 * \e {These functions are not implemented in timed_event_oxygen but were
 * present in the original CarlOS timed_event interface}
 * \c time_of_current_timed_event() and \c tid_of_current_timed_event()
 * give information about the timed event that caused the current code
 * to be run; they return \c FALSE or \c NO_TID if the current code
 * is not running within a normal or casual timed event.
 *
 */

#ifndef TIMED_EVENT_H
#define TIMED_EVENT_H

#include "hydra/hydra_types.h"
#include "hydra/hydra_macros.h"
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
