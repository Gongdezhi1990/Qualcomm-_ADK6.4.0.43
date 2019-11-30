/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
Copyright (c) 2008 - 2016 Qualcomm Technologies International, Ltd.
  
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 ************************************************************************//**
 * \file
 * Interface for timer functions
*/

#if !defined(PL_TIMERS_H)
#define PL_TIMERS_H

/****************************************************************************
Include Files
*/
#include "hydra/hydra_types.h"
#include "timed_event/rtime.h"

/****************************************************************************
Public Macro Declarations
*/
/* This is derived from BlueCore rtime */

/** Constants for use in setting timers, in terms of timer ticks.  */
#define US_PER_MS         ((INTERVAL) 1000)
#define MS_PER_SEC        ((INTERVAL) 1000)
#define US_PER_SEC        (US_PER_MS * MS_PER_SEC)

#define HAL_SET_REG_TIMER1_TRIGGER(time) \
                                      hal_set_reg_timer1_trigger((unsigned)(time))

/* 32-bit time in 32-bit number */
#define INT_TIME(t) ((int32)(t))

/* Time manipulation macros
 *
 * There are two different data types at work here.
 *
 * One is a "system time" -  this is something we could program
 * into the timer hardware, or read back from hal_get_time().
 * This is constrained by the hardware to be a 32-bit number, and is unsigned.
 * Numbers outside the current 70-minute-ish epoch are meaningless.
 * Adding two system times, or multiplying a system time by a constant,
 * doesn't produce a sensible result.
 *
 * The other is a "time interval". This is a number of microseconds
 * between two events, and (depending on ordering) can be negative.
 * Time intervals are just numbers - they can be added, multiplied, whatever.
 *
 * time_add takes a system time and a time interval, and returns a system time.
 *
 * time_sub takes two system times, and returns a time interval.
 * This is just a number, so it can be compared with other numbers
 * (thresholds etc) directly.
 */

/****************************************************************************
Public Type Declarations
*/
/**
 * tTimerId - A timer identifier.
 *
 * Note: we want this to have the most efficient type that is at least 24 bits
 * long, but we also need to know exactly how long that is.
 */
#if UINT_MAX >= 0xFFFFFF
typedef unsigned int tTimerId;
#define TIMERID_BIT UINT_BIT
#elif ULONG_MAX >= 0xFFFFFF
typedef unsigned long tTimerId;
#define TIMERID_BIT ULONG_BIT
#else
#error "unsigned long is too short for timerIDs!"
#endif

/**
 * Invalid timer id value
 */
#define TIMER_ID_INVALID   (tTimerId)0

/**
 * Function type for timed event handler
 * The event handler accepts a data pointer,
 * provided when the timer is scheduled.
 */
typedef void (*tTimerEventFunction)(void *task_data);

/**
 * Alternative function type for timed event handler.  This interface is to
 * accommodate Hydra-generic-orientated clients
 * @param iarg Integer argument supplied when the timer was scheduled
 * @param task_data Data pointer provided when the timer was scheduled
 */
typedef void (*tTimerEventFunctionAlt)(uint16 iarg, void *task_data);

/****************************************************************************
Global Variable Definitions
*/

#ifdef SCHED_NO_TIMER_PREEMPTION
/** In order to avoid timed events pre-empting their associated tasks, we
 * set an effective priority level at which they are run.  Tasks at lower
 * priority levels cannot post timed events  */
#define TIMED_EVENT_PRIORITY DEFAULT_PRIORITY
#endif

/****************************************************************************
Public Function Prototypes
*/

/**
 * Initialise the timer ISR.  It is (and must remain) safe to call this
 * function more than once.
 *
 * pl_timers on the Apps uses the low-priority timer1.  Had to pick something.
 */
extern void init_pl_timers(void);


/**
 * \brief Create a new strict timed event and add to queue
 *
 * \param[in] event_time Absolute time for the event
 * \param[in] event_fn Event expiry handler
 * \param[in] data_ptr Value passed as input to event handler
 *
 * \return returns the timer ID
 */
tTimerId create_add_strict_event(
        TIME event_time, tTimerEventFunction event_fn, void *data_ptr);

/**
 * \brief Create a new casual timed event and add to queue
 *
 * \param[in] earliest Absolute earliest time for the event
 * \param[in] latest Absolute latest time for the event
 * \param[in] event_fn Event expiry handler
 * \param[in] data_ptr Value passed as input to event handler
 *
 * \return returns the timer ID
 */
tTimerId create_add_casual_event(
        TIME earliest, TIME latest, tTimerEventFunction event_fn, void *data_ptr);

/**
 * \brief Create a new casual timed event with the "alt"-style handler and add
 * to queue
 *
 * \param[in] earliest Absolute earliest time for the event
 * \param[in] latest Absolute latest time for the event
 * \param[in] event_fn Event expiry handler
 * \param[in] iarg Integer value to be passed to handler
 * \param[in] data_ptr Value passed as input to event handler
 *
 * \return returns the timer ID
 *
 * \ingroup alt_interface
 */
tTimerId create_add_casual_event_alt(
        TIME earliest, TIME latest, tTimerEventFunctionAlt event_fn, uint16 iarg,
        void *data_ptr);

/**
 * \brief Schedule a timed event to occur at a particular absolute time value.
 * When the event occurs the selected handler function will be called.
 *
 * \param[in] event_time Absolute time value at which to trigger an event
 *
 * \param[in] TimerEventFunction function to call at requested time
 *
 * \param[in] data_pointer Value passed as input to TimerEventFunction
 *
 * \return Return an ID that could be used to cancel the event.
 *
 * \note These events are scheduled in hardware and the event handlers will be
 * invoked at interrupt level. The maximum latency will be interrupt latency +
 * handling time for any other events scheduled at same time. The event handler
 * function should aspire to complete at the earliest and should not block.
 *
 * \par If the timed event has a soft deadline, consider using
 * timer_schedule_event_at_between *
 */
extern tTimerId timer_schedule_event_at(
        TIME event_time,
        tTimerEventFunction TimerEventFunction,
        void *data_pointer);

/**
 * \brief Schedule a timed event to occur at a particular relative time from the
 * current time. When the event occurs the selected handler function will be called.
 *
 * \param[in] time_in a relative time value (in us) at which to trigger an event
 *
 * \param[in] TimerEventFunction function to call at requested time
 *
 * \param[in] data_pointer Value passed as input to TimerEventFunction
 *
 * \return Return an ID that could be used to cancel the event.
 *
 * \note These events are scheduled in hardware and the event handlers will be
 * invoked at interrupt level. The maximum latency will be interrupt latency +
 * handling time for any other events scheduled at same time. The event handler
 * function should aspire to complete at the earliest and should not block.
 *
 * \par If the timed event has a soft deadline, consider using
 * timer_schedule_event_in_between
 */
extern tTimerId timer_schedule_event_in(
        INTERVAL time_in,
        tTimerEventFunction TimerEventFunction,
        void *data_pointer);

/**
 * \brief Schedules a background timed event to occur at or after a particular absolute
 * time value. When the event occurs the selected handler function will be called.
 *
 * \param[in] time_absolute Absolute time value at which to trigger an event
 * \param[in] TimerEventFunction function to call at requested time
 * \param[in] data_pointer Value passed as input to TimerEventFunction
 *
 * \return Returns an ID that could be used to cancel the event.
 *
 * \note These events are scheduled in scheduler/background context and the
 * event handlers will be invoked whenever scheduler has free time after the
 * time_absolute has passed.
 */
extern tTimerId timer_schedule_bg_event_at(
        TIME time_absolute,
        tTimerEventFunction TimerEventFunction,
        void *data_pointer);


/**
 * \brief Schedules a background timed event to occur at or after a particular
 * relative time from the current time. When the event occurs the selected
 * handler function will be called.
 *
 * \param[in] time_in a relative time value (in us) at which to trigger an event
 * \param[in] TimerEventFunction function to call at requested time
 * \param[in] data_pointer Value passed as input to TimerEventFunction
 *
 * \return Return an ID that could be used to cancel the event.
 *
 * \note These events are scheduled in scheduler/background context and the
 * event handlers will be invoked whenever scheduler has free time after the
 * expiry time has passed.
 */
extern tTimerId timer_schedule_bg_event_in(
        INTERVAL time_in,
        tTimerEventFunction TimerEventFunction,
        void *data_pointer);

/**
 * \brief Schedules a background event to occur at any time between the specified
 * earliest and latest absolute times. When the event occurs the selected handler
 * function will be called.
 *
 * \param[in] time_earliest Earliest absolute time value at which to trigger an event
 * \param[in] time_latest Latest absolute time value at which to trigger an event
 * \param[in] TimerEventFunction function to call at requested time
 * \param[in] data_pointer Value passed as input to TimerEventFunction
 *
 * \return Returns an ID that could be used to cancel the event.
 *
 * \note These events are scheduled in scheduler/background context and the
 * event handlers will be invoked whenever scheduler has free time after the
 * earliest (and hopefully, before latest time).
 */
extern tTimerId timer_schedule_bg_event_at_between(
        TIME time_earliest,
        TIME time_latest,
        tTimerEventFunction TimerEventFunction,
        void *data_pointer);

/**
 * \brief Schedules a background event to occur at any time between the specified
 * earliest and latest absolute times. When the event occurs the selected handler
 * function will be called.
 *
 * \param[in] time_earliest Earliest absolute time value at which to trigger an event
 * \param[in] time_latest Latest absolute time value at which to trigger an event
 * \param[in] TimerEventFunction function to call at requested time
 * \param[in] iarg Integer value to be passed to handler
 * \param[in] data_pointer Value passed as input to TimerEventFunction
 *
 * \return Returns an ID that could be used to cancel the event.
 *
 * \note These events are scheduled in scheduler/background context and the
 * event handlers will be invoked whenever scheduler has free time after the
 * earliest (and hopefully, before latest time).
 *
 * \ingroup alt_interface
 */
extern tTimerId timer_schedule_bg_event_at_between_alt(
        TIME time_earliest,
        TIME time_latest,
        tTimerEventFunctionAlt TimerEventFunction,
        uint16 iarg,
        void *data_pointer);

/**
 * \brief Schedules a background event to occur at any time between the specified
 * earliest and latest relative time from current time. When the event occurs the
 * selected handler function will be called.
 *
 * \param[in] time_earliest Earliest relative time value (in us) at which to trigger an event
 * \param[in] time_latest Latest relative time value (in us) at which to trigger an event
 * \param[in] TimerEventFunction function to call at requested time
 * \param[in] data_pointer Value passed as input to TimerEventFunction
 *
 * \return Returns an ID that could be used to cancel the event.
 *
 * \note These events are scheduled in scheduler/background context and the
 * event handlers will be invoked whenever scheduler has free time after the
 * earliest (and hopefully, before latest time).
 */
extern tTimerId timer_schedule_bg_event_in_between(
        INTERVAL time_earliest,
        INTERVAL time_latest,
        tTimerEventFunction TimerEventFunction,
        void *data_pointer);

/**
 * \brief Schedules a background event to occur at any time between the specified
 * earliest and latest relative time from current time. When the event occurs the
 * selected handler function will be called.
 *
 * \param[in] time_earliest Earliest relative time value (in us) at which to trigger an event
 * \param[in] time_latest Latest relative time value (in us) at which to trigger an event
 * \param[in] TimerEventFunction function to call at requested time
 * \param[in] iarg Integer value to be passed to handler
 * \param[in] data_pointer Value passed as input to TimerEventFunction
 *
 * \return Returns an ID that could be used to cancel the event.
 *
 * \note These events are scheduled in scheduler/background context and the
 * event handlers will be invoked whenever scheduler has free time after the
 * earliest (and hopefully, before latest time).
 *
 * \ingroup alt_interface
 */
extern tTimerId timer_schedule_bg_event_in_between_alt(
        INTERVAL time_earliest,
        INTERVAL time_latest,
        tTimerEventFunctionAlt TimerEventFunction,
        uint16 iarg,
        void *data_pointer);

/**
 * \brief Interrupt based timer service routine that services all strict events that
 * have expired and programs the timer registers for the next strict timed event.
 *
 * \note This function is also used as the timer interrupt service routine.
 */
extern void timer_service_routine(void);


/**
 * \brief Schedules a background timed event to occur at or after a particular
 * relative time from the current time. When the event occurs the selected
 * handler function will be called.
 *
 * \param[in] time_in a relative time value (in us) at which to trigger an event
 * \param[in] TimerEventFunction function to call at requested time
 * \param[in] data_pointer Value passed as input to TimerEventFunction
 *
 * \return Return an ID that could be used to cancel the event.
 *
 * \note These events are scheduled in scheduler/background context and the
 * event handlers will be invoked whenever scheduler has free time after the
 * expiry time has passed.
 */
extern tTimerId asm_timer_schedule_bg_event_in(
    INTERVAL time_in,
    tTimerEventFunction TimerEventFunction,
    void *data_pointer);

/**
 * \brief Cancel scheduled timer event with the selected ID
 *
 * \param[in] timer_id The timer ID of the timer struct to cancel
 * \param[in] piarg Pointer to the message integer
 * \param[in] ptask_data Pointer to the message pointer
 *
 */
bool timer_cancel_event_ret(tTimerId timer_id, uint16 *piarg, void **ptask_data);

void timer_cancel_event(tTimerId timer_id);
#define timer_cancel_event(timer_id) \
                    ((void)timer_cancel_event_ret(timer_id, NULL, NULL))


/**
 * \brief Cancel scheduled timer event(s) with the given handler function
 * and (optionally) data pointer
 *
 * \param[in] TimerEventFunction Function called at timer expiry
 * \param[in] data_pointer data pointer to test against. Not checked if NULL.
 *
 */
void timer_cancel_event_by_function(tTimerEventFunction TimerEventFunction,
                                    void *data_pointer);

/**
 * \brief Cancel scheduled timer event(s) with the given handler function
 * and (optionally) data pointer
 *
 * \param[in] TimerEventFunction Function called at timer expiry
 * \param[in] iarg Integer arg to test against
 * \param[in] data_pointer data pointer to test against. Not checked if NULL.
 *
 * \ingroup alt_interface
 */
void timer_cancel_event_by_function_alt(tTimerEventFunctionAlt TimerEventFunction,
                                    uint16 iarg,
                                    void *data_pointer);

/**
 * Cancel the first event on the strict event queue which
 * matches the supplied \c TimerEventFunction and also matches according to the
 * optional data comparison function.  The arbitrary data pointer of the message
 * is returned in the matched_data parameter
 * @param TimerEventFunction Timer handler function to match for
 * @param data_pointer "Test-against" data for the comparison function
 * @param cmp_fn The comparsion function.  Unused if NULL.
 * @param pmatched_data Pointer to space for returning the arbitrary data
 * pointer inside the queued event, in case this needs to be freed
 * @return TRUE if a match was found else FALSE
 */
bool timer_cancel_strict_event_by_function_cmp(
                                       tTimerEventFunction TimerEventFunction,
                                       void *data_pointer,
                                       bool (*cmp_fn)(const void *timer_data,
                                                      const void *data_pointer),
                                       void **pmatched_data);

/**
 * \brief Cause a delay of n microseconds in a busy wait, which can be interrupted
 *
 * \param[in] delay_duration_us delay in microseconds
 *
 * \note This is a busy wait at full power. Consider using a state machine instead
 * as busy wait will block all other tasks at same or lower priorities
 *
 */
void timer_n_us_delay(INTERVAL delay_duration_us);


/**
 * \brief Compare the current time with a threshold time value.
 *
 * \param[in] t Threshold time to compare against
 *
 * \return TRUE if the current time is later than the threshold
 *
 * \note This function exists because the time comparison macros have
 * multiple references to their parameters. This is horribly inefficient
 * if one of them is hal_get_time()
 */
bool is_current_time_later_than(TIME t);

/**
 * \brief Compare the current time with a threshold time value.
 *
 * \param[in] t Threshold time to compare against
 *
 * \return TRUE if the current time is earlier than the threshold
 *
 * \note This function exists because the time comparison macros have
 * multiple references to their parameters. This is horribly inefficient
 * if one of them is hal_get_time()
 */
bool is_current_time_earlier_than(TIME t);

/**
 * \brief Get the expiry time of the last strict timer. This is provided
 * to facilitate the scheduling of periodic and periodic like timers. It
 * should only be called from within the timer handler context
 * otherwise the accuracy of the value returned cannot be gauranteed.
 *
 * \return Time the last strict timer was schedule.
 */
TIME get_last_fire_time(void);

/**
 * \brief Get the expiry time of the last casual timer. This is provided
 * to facilitate the scheduling of periodic and periodic like timers. It
 * should only be called from within the timer handler context
 * otherwise the accuracy of the value returned cannot be gauranteed.
 *
 * \return The earliest time the last casual timer was scheduled for.
 */
TIME get_last_casual_fire_time(void);

/**
 * Get the scheduled time of the specified casual event, if it exists
 * @param timer_id ID of event
 * @param[out] event_time Pointer to the scheduled time of the event, if found
 * @return TRUE if event found, else FALSE
 */
extern bool timer_get_time_of_bg_event(tTimerId timer_id, TIME *event_time);

/**
 * Return the first casual event which matches the given handler function and,
 * optionally, function arguments
 * @param fn Alt handler function to search for
 * @param piarg Pointer to iarg value to search for. Ignored if NULL.
 * @param ptask_data Pointer to task_data value to search for. Ignored if NULL.
 * @return
 *
 * \ingroup alt_interface
 */
extern tTimerId timer_find_first_bg_event_by_fn_alt(tTimerEventFunctionAlt fn,
                                                    uint16 *piarg,
                                                    void **ptask_data);

/**
 * Tell the timers block that the scheduler is going to sleep so will no
 * longer check whether any background timers have expired. The timers module
 * will need to wake up the scheduler when there are background timers that
 * need servicing.
 *
 * \note The wakeup is performed by calling sched_wakeup_from_timers().
 */
void timers_scheduler_needs_wakeup(void);

/**
 * Tell the timers block that the scheduler is now active so there's no need
 * to inform the scheduler of background timers. The scheduler will check for
 * them at appropriate times.
 *
 * \note It's safe to call this even if the timers block already thinks the
 *       scheduler is awake.
 */
void timers_scheduler_has_awoken(void);

/**
 * Delete the causal_events_queue.  This function is used by desktop test code,
 * but it is available for firmware use too.
 */
extern void timer_cancel_all_bg(void);


#ifdef DESKTOP_TEST_BUILD
/**
 * \brief Sets system time to curr_time, then runs any expired timers
 *
 * \param[in] curr_time Current time
 *
 */
extern void PlRunTimers(TIME curr_time);

/**
 * Call to allow any expired strict timed events to be run
 */
extern void timer_run_expired_strict_events(void);

#endif

#endif /* PL_TIMERS_H */
