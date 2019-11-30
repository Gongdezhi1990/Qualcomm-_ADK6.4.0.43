/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 ************************************************************************//**
 * \defgroup pl_timers Platform timers
 * \ingroup platform
 *
 * \file pl_timers.h
 * \ingroup pl_timers
 *
 * Interface for timer functions
 *
 * NOTE - Defines interface to use timer functionality on the platform.
 *
 * \note  There are two interfaces to pl_timers: the standard interface, in
 * which events are registered with a \c tTimerEventFunction and a \c void * to
 * pass in when the event fires, and the "alt" interface, in which events are
 * registered with a \c tTimerEventFunctionAlt and two arguments - an unsigned
 * integer and a \c void *.  The standard interface is to be preferred
 * for new code as it is slightly more efficient.  The alt interface exists to
 * support integration of the Hydra generic codebase with pl_timers.
 *
 *
 ****************************************************************************/

#if !defined(PL_TIMERS_H)
#define PL_TIMERS_H

/****************************************************************************
Include Files
*/
#include "hydra/hydra_types.h"
#include "limits.h"
#include "hal_time.h"
#include "sections.h"
#include "platform/pl_intrinsics.h"

/****************************************************************************
Public Macro Declarations
*/
/* This is derived from BlueCore rtime */

/** Constants for use in setting timers, in terms of timer ticks.  */
#define US_PER_MS         ((INTERVAL) 1000)
#define MS_PER_SEC        ((INTERVAL) 1000)
#define US_PER_SEC        (US_PER_MS * MS_PER_SEC)

#define MILLISECOND   US_PER_MS
#define SECOND        US_PER_SEC
#define MINUTE        (60 * SECOND)

#ifdef HAVE_32BIT_TIMERS
/* This ifdef actually means "have 32-bit timers *and 24-bit words*",
 * hence the shennanigans with setting a second register */
#define HAL_SET_REG_TIMER1_TRIGGER(time)\
    do {\
        hal_set_reg_timer1_trigger((time) & 0xFFFFFFul);\
        hal_set_reg_timer1_trigger_ms((uint24)((time) >> 24)); \
    }\
    while (0)

#define HAL_SET_REG_TIMER2_TRIGGER(time)\
    do {\
        hal_set_reg_timer2_trigger((time) & 0xFFFFFFul);\
        hal_set_reg_timer2_trigger_ms((uint24)((time) >> 24)); \
    }\
    while (0)
#else
/* Setting a 32-bit register for K32 and a 24-bit register for K24
 * looks exactly the same */
#define HAL_SET_REG_TIMER1_TRIGGER(time) \
                                      hal_set_reg_timer1_trigger((unsigned)(time))

#define HAL_SET_REG_TIMER2_TRIGGER(time) \
                                      hal_set_reg_timer2_trigger((unsigned)(time))
#endif

/* This is ugly, but we have 4 possible combinations :
 * 24-bit or 32-bit hardware timer support,
 * with 24-bit or 32-bit CPU architecture.
 * Some cases need sign-extension for time_sub to work properly.
 *
 * There is now a fifth option: *native* 32-bit timers, which is what we have
 * with __KALIMBA32__
 */

#if defined (HAVE_32BIT_TIMERS)
/* See above for the meaning of HAVE_32BIT_TIMERS */
#define MAX_TIME 0xfffffffful
#ifdef __GNUC__
/* 32-bit time in 32-bit number */
#define INT_TIME(t) ((int32)(t))
#else
/* Sign-extend 32-bit to 48-bit */
#define INT_TIME(t) ((int48) ((((uint48)(t) & 0x80000000ul) != 0) ? ((uint48)(t) | 0xffff00000000ul) : ((t) & MAX_TIME)))
#endif
#elif defined (__KALIMBA32__)
/* 32-bit time in 32-bit number */
#define MAX_TIME 0xfffffffful
#define INT_TIME(t) ((int32)(t))
#else
#define MAX_TIME 0xfffffful
#ifdef __GNUC__
/* Sign-extend 24-bit to 32-bit */
#define INT_TIME(t) ((int32) ((((uint32)(t) & 0x800000ul) != 0) ? ((uint32)(t) | 0xff000000ul) : ((t) & MAX_TIME)))
#else
/* 24-bit time in 24-bit number */
#define INT_TIME(t) ((int24)(t))
#endif
#endif


/* Time manipulation macros
 *
 * There are two different data types at work here.
 *
 * One is a "system time" -  this is something we could program
 * into the timer hardware, or read back from hal_get_time().
 * This is constrained by the hardware to be a 32-bit number
 * (24 bits without HAVE_32BIT_TIMERS), and is unsigned.
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

/**
 * \brief add two time values
 *
 * \returns   The sum of "t1 and "t2".
 *
 * \note Implemented as a macro, because it's trivial.
 * \note Adding the numbers can overflow the range of a TIME, so the user must
 * be cautious.
 */
#define time_add(t1, t2)    (((t1) + (t2)) & MAX_TIME)

/**
 * \brief subtract two time values
 *
 * \note Implemented as a macro, because it's trivial.
 *
 * \par Subtracting the numbers can provoke an underflow. This returns a signed number
 * for correct use in comparisons.
 */
#define time_sub(t1, t2)    INT_TIME(((TIME_INTERVAL)(t1) - (TIME_INTERVAL)(t2)))

 /**
 * \brief compare two time values "t1" and "t2" for equality.
 *
 * \returns   TRUE if "t1" equals "t2", else FALSE.
 *
 * \note Implemented as a macro, because it's trivial.
 */
#define time_eq(t1, t2)     ((t1) == (t2))

/**
 * \brief compare two time values "t1" and "t2" for inequality
 *
 * \returns   FALSE if "t1" equals "t2", else TRUE.
 *
 * \note Implemented as a macro, because it's trivial.
 */
#define time_ne(t1, t2)     ((t1) != (t2))

/**
 * \brief compare two time values "t1" and "t2"
 *
 * \returns   TRUE if "t1" is greater than "t2", else FALSE.
 *
 * \note Implemented as a macro, because it's trivial.
 */
#define time_gt(t1, t2) (time_sub((t1), (t2)) > 0)

/**
 * \brief compare two time values "t1" and "t2"
 *
 * \returns   TRUE if "t1" is greater than, or equal to, "t2", else FALSE.
 *
 * \note Implemented as a macro, because it's trivial.
 */
#define time_ge(t1, t2) (time_sub((t1), (t2)) >= 0)

/**
 * \brief compare two time values "t1" and "t2"
 *
 * \returns   TRUE if "t1" is less than "t2", else FALSE.
 *
 * \note Implemented as a macro, because it's trivial.
 */
#define time_lt(t1, t2) (time_sub((t1), (t2)) < 0)

/**
 * \brief compare two time values "t1" and "t2"
 *
 * \returns  TRUE if "t1" is less than, or equal to, "t2", else FALSE.
 *
 * \note Implemented as a macro, because it's trivial.
 */
#define time_le(t1, t2) (time_sub((t1), (t2)) <= 0)

/**
 * \brief set a timer atomically
 *
 * \param[in] event_time Absolute time value at which to trigger an event
 *
 * \param[in] TimerEventFunction function to call at requested time
 *
 * \param[in] data_pointer Value passed as input to TimerEventFunction
 *
 * \param[out] id Location to store the timer ID
 *
 * \note The timer_schedule_event_in/_at functions suffer from a 
 * potential race hazard, in that the timer can expire before the function 
 * returns. If the expiry function resets the timer, the ID returned is no 
 * longer valid, and can't be used to subsequently cancel the timer.
 * These macros avoid the problem by blocking interrupts until the timer ID is assigned.
 */

#define timer_schedule_event_at_atomic(event_time, TimerEventFunction, data_pointer, id) \
    do \
    { \
        LOCK_INTERRUPTS; \
        *id = timer_schedule_event_at(event_time, TimerEventFunction, data_pointer); \
        UNLOCK_INTERRUPTS; \
     } \
     while (0)

/**
 * \brief set a timer atomically
 *
 * \param[in] time_in Relative time value at which to trigger an event
 *
 * \param[in] TimerEventFunction function to call at requested time
 *
 * \param[in] data_pointer Value passed as input to TimerEventFunction
 *
 * \param[out] id Location to store the timer ID
 *
 * \note The timer_schedule_event_in/_at functions suffer from a 
 * potential race hazard, in that the timer can expire before the function 
 * returns. If the expiry function resets the timer, the ID returned is no 
 * longer valid, and can't be used to subsequently cancel the timer.
 * These macros avoid the problem by blocking interrupts until the timer ID is assigned.
 */

#define timer_schedule_event_in_atomic(time_in, TimerEventFunction, data_pointer, id) \
    do \
    { \
        LOCK_INTERRUPTS; \
        *id = timer_schedule_event_in(time_in, TimerEventFunction, data_pointer); \
        UNLOCK_INTERRUPTS; \
     } \
     while (0)



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
typedef void (*tTimerEventFunction)(void *data);

/**
 * Alternative function type for timed event handler.  This interface is to
 * accommodate Hydra-generic-orientated clients
 * @param iarg Integer argument supplied when the timer was scheduled
 * @param data Data pointer provided when the timer was scheduled
 */
typedef void (*tTimerEventFunctionAlt)(uint16 iarg, void *data);

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
INLINE_SECTION static inline tTimerId timer_schedule_event_at(
       TIME event_time,
       tTimerEventFunction TimerEventFunction,
       void *data_pointer)
{
    return create_add_strict_event(event_time, TimerEventFunction, data_pointer);
}

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
INLINE_SECTION static inline tTimerId timer_schedule_event_in(
        INTERVAL time_in,
        tTimerEventFunction TimerEventFunction,
        void *data_pointer)
{
    return create_add_strict_event(
            time_add(hal_get_time(), time_in), TimerEventFunction, data_pointer);
}

#ifdef __KALIMBA__
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
extern tTimerId asm_timer_schedule_event_in(
        INTERVAL time_in,
        tTimerEventFunction TimerEventFunction,
        void *data_pointer);
#endif
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
INLINE_SECTION static inline tTimerId timer_schedule_bg_event_at(
        TIME time_absolute,
        tTimerEventFunction TimerEventFunction,
        void *data_pointer)
{
    /* earliest and latest time is the same */
    return create_add_casual_event(
                time_absolute, time_absolute, TimerEventFunction, data_pointer);
}
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
INLINE_SECTION static inline tTimerId timer_schedule_bg_event_in(
        INTERVAL time_in,
        tTimerEventFunction TimerEventFunction,
        void *data_pointer)
{
    /* Convert time_in to be absolute time */
    TIME event_time = time_add(hal_get_time(), time_in);

    /* earliest and latest time is the same */
    return create_add_casual_event(
                event_time, event_time, TimerEventFunction, data_pointer);
}
/**
 * \brief Same function as timer_schedule_bg_event_in. The only difference is that thi
 * is not inline function which makes it callable for assembly language.
 *
 * \param[in] time_in a relative time value (in us) at which to trigger an event
 * \param[in] TimerEventFunction function to call at requested time
 * \param[in] data_pointer Value passed as input to TimerEventFunction
 *
 * \return Return an ID that could be used to cancel the event.
 *
 * \note This is a wrapper function which can be called from assembly code.
 * The C functions may be inlined and are hence not guaranteed to be visible
 * to anything that doesn't include this header.
 *
 */
extern tTimerId asm_timer_schedule_bg_event_in(
        TIME_INTERVAL time_in,
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
INLINE_SECTION static inline tTimerId timer_schedule_bg_event_at_between(
        TIME time_earliest,
        TIME time_latest,
        tTimerEventFunction TimerEventFunction,
        void *data_pointer)
{
    return create_add_casual_event(
                time_earliest, time_latest, TimerEventFunction, data_pointer);
}

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
INLINE_SECTION static inline tTimerId timer_schedule_bg_event_at_between_alt(
        TIME time_earliest,
        TIME time_latest,
        tTimerEventFunctionAlt TimerEventFunction,
        uint16 iarg,
        void *data_pointer)
{
    return create_add_casual_event_alt(
                time_earliest, time_latest, TimerEventFunction,
                iarg, data_pointer);
}
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
INLINE_SECTION static inline tTimerId timer_schedule_bg_event_in_between(
        INTERVAL time_earliest,
        INTERVAL time_latest,
        tTimerEventFunction TimerEventFunction,
        void *data_pointer)
{
    TIME now = hal_get_time();
    return create_add_casual_event(
            time_add(now, time_earliest),
            time_add(now, time_latest),
            TimerEventFunction, data_pointer);
}
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
INLINE_SECTION static inline tTimerId timer_schedule_bg_event_in_between_alt(
        INTERVAL time_earliest,
        INTERVAL time_latest,
        tTimerEventFunctionAlt TimerEventFunction,
        uint16 iarg,
        void *data_pointer)
{
    TIME now = hal_get_time();
    return create_add_casual_event_alt(
                time_add(now, time_earliest),
                time_add(now, time_latest),
                TimerEventFunction,
                iarg, data_pointer);
}
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
bool timer_cancel_event_ret(tTimerId timer_id, uint16 *piarg, void **pdata);

void timer_cancel_event(tTimerId timer_id);
#define timer_cancel_event(timer_id) \
                    ((void)timer_cancel_event_ret(timer_id, NULL, NULL))


INLINE_SECTION static inline void timer_cancel_event_atomic(tTimerId *timer_id)
{
    interrupt_block();
    if (*timer_id != TIMER_ID_INVALID)
    {
        (void) timer_cancel_event_ret(*timer_id, NULL, NULL);
        *timer_id = TIMER_ID_INVALID;
    }
    interrupt_unblock();
}


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
 * @param pdata Pointer to data value to search for. Ignored if NULL.
 * @return
 *
 * \ingroup alt_interface
 */
extern tTimerId timer_find_first_bg_event_by_fn_alt(tTimerEventFunctionAlt fn,
                                                    uint16 *piarg,
                                                    void **pdata);

/**
 * Delete the casual_events_queue.  This function is used by desktop test code,
 * but it is available for firmware use too.
 */
extern void timer_cancel_all_bg(void);

/**
 * Reschedule the casual_events wakeup timer.
 */
extern void timers_reschedule_wakeup(void);

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
