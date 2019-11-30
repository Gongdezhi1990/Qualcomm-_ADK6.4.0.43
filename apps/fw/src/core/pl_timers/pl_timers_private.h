/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
Copyright (c) 2008 - 2016 Qualcomm Technologies International, Ltd.
  
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 ************************************************************************//**
 * \file
 * header for the internals of the timer, which is shared between timer and
 * scheduler
*/
#ifndef PL_TIMERS_PRIVATE_H
#define PL_TIMERS_PRIVATE_H

/****************************************************************************
Include Files
*/
#include "pl_timers/pl_timers.h"
#include "int/int.h"
#include "hydra/hydra_macros.h"
#include "pmalloc/pmalloc.h"
#include "panic/panic.h"
#include "sched_oxygen/sched_oxygen.h" /* For current_task_priority() */
#include "patch.h"

/****************************************************************************
Public Macro Declarations
*/
/****************************************************************************
Public Type Declarations
*/

/**
 * Defines an abstract timer handle. This is only ever used as a pointer
 */
typedef struct tTimerStuctTag
{
    struct tTimerStuctTag *next; /**<Pointer to the next timer in a linked list */
    /** Timer ID is a unique combination of timer id count and various flags based on
     * event parameters. */
    tTimerId timer_id;
    void *data_pointer; /**< data pointer passed to timer expiry function */
    tTimerEventFunction TimedEventFunction; /**< Timer expiry handler function */
} tTimerStruct;

/**
 * Strict timer time values
 */
typedef struct
{
    tTimerStruct base;
    TIME event_time; /**< Absolute time at which timer expires */
} tStrictTimerStruct;

/**
 * Casual timer time values
 */
typedef struct
{
    tTimerStruct base;
    TIME earliest_time; /**< Earliest time at which timer expires */
    TIME latest_time; /**< Latest time at which timer expires */
} tCasualTimerStruct;



/****************************************************************************
Global Variable Definitions
*/
/*
 * The timer event queues and service flag are shared between the scheduler
 * and the timer module.
 */
extern bool timer_being_serviced;

/****************************************************************************
Public Function Prototypes
*/
/**
 * \brief Timer service routine, which traverses through the strict event list
 * and services expired events.
 *
 * \note This function is internal to platform and is called by scheduler and
 * timer interrupt service routine.
 *
 * \pre Interrupts must be disabled before calling this function.
 */
void timers_service_expired_strict_events(void);

/**
 * \brief Timer service routine, which traverses through the casual event list
 * and services expired events.
 *
 * \note This function is internal to platform and is called by scheduler and
 * timer interrupt service routine.
 *
 * \pre Interrupts must be disabled before calling this function.
 */
void timers_service_expired_casual_events(void);

/**
 * \brief Finds the deadline of the next timer (strict or casual) that is set to expire.
 *
 * \param[out] next_time Holds the expiry time of the first event to expire, if there
 * are any.
 *
 * \return Returns TRUE if a valid timer was found, else FALSE
 */
bool timers_get_next_event_time(TIME *next_time);

/**
 * \brief Gets scheduled expiry time for the given timer id
 *
 * \param[in] timer_id event Id for which expiry time is required
 *
 * \return The scheduled expiry time for the event
 *
 * \note This function is used mainly from timer test applications. In the case of casual
 * events, the expiry time returned is the earliest time.
 */
extern TIME get_timer_expiry_time(tTimerId timer_id);

/**
 * \brief Gets pointer to the event the given timer id
 *
 * \param[in] timer_id event Id for which pointer is required
 *
 * \return Pointer to the timer structure
 *
 * \note This function is used mainly from timer test applications.
 */
extern tTimerStruct *get_timer_from_id(tTimerId timer_id);

#endif   /* PL_TIMERS_PRIVATE_H */
