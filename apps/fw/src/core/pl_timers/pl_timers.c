/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
Copyright (c) 2008 - 2016 Qualcomm Technologies International, Ltd.
  
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 ************************************************************************//**
 * \file
 * Implementation of platform timer functions.
*/

#include "pl_timers/pl_timers_private.h"
#include "assert.h"

/****************************************************************************
Private Macro Declarations
*/

/**
 * Maximum positive time difference or delay allowed
 */
#define MAX_DELAY  ((INTERVAL)(UINT_MAX>>1)) /* MAX_UINT/2 */

/**
 * The following comment dates back to the 24-bit Audio origins of this file.
 *
 * The timerID is a 24-bit number, with bits b0 to b19 formed by a counter from
 * MIN_UNIQUE_TIMER_ID_COUNT to MAX_UNIQUE_TIMER_ID_COUNT with 0 being illegal value.
 *
 * Assume a timer is created/firing every 625usec per direction per SCO link. Assuming 3
 * active SCO links, that is 6 timers every 625usec or 9600 timers every second. A 20 bit
 * counter will roll over every 109 (approximate) seconds.
 *
 * However, most timers will be shorter than 109 seconds, usually shorter than a second
 * even. If the longer timers (longer than say 5 seconds) and the periodic ones use a
 * separate counter for forming the timerID, then the risk of duplicate timerID is
 * reduced; only the longer/periodic timers risk having duplicate timerID and only if
 * there are more than a million such timers created in the system.
 */
#define MIN_UNIQUE_TIMER_ID_COUNT    1
#define MAX_UNIQUE_TIMER_ID_COUNT    ((1 << (TIMERID_BIT - 4)) - 1)

/**
 * Timer ID is a combination of timer id count, strict/casual event flag and
 * long-periodic/short event flag.
 * The timer id count gives the timer ID its uniqueness, where as the flag is used
 * to determine which event queue the timer belongs to internally.
 */
#define STRICT_EVENT  (1UL<< (TIMERID_BIT - 1))
#define CASUAL_EVENT  0

#define EVENT_IS_STRICT(id)  ((id)&STRICT_EVENT)
/*lint --e{750}*/#define EVENT_IS_CASUAL(id)  (!EVENT_IS_STRICT(id)) /* defined for completeness */

/** Timers with expiry time more than LONG_TIMER_THRESHOLD are long timers */
#define LONG_TIMER_THRESHOLD (INTERVAL)(5*SECOND)
/**
 * Flag in timer ID indicating if the timer is longer than LONG_TIMER_THRESHOLD or
 * periodic
 */
#define LONG_EVENT  (1<<(TIMERID_BIT - 2))

#ifdef DEBUG_KICK_TIMERS
/* If we set a timer in the past, it immediately expires.
 * Limit how far back it can be.
 */
#define MAX_EVENT_TIME_PAST (50*MILLISECOND)
#endif

/****************************************************************************
Private Type Declarations
*/

typedef struct tEventsQueueTag
{
    tTimerStruct *first_event; /**< Head of event queue */

    /** The last time a timer on this message queue fired. This is provided so
     * that a user can request this if they want to schedule a periodic timer
     * during the timer handler. */
    TIME last_fired;

    /* functions that act on queue elements */
    TIME (*get_latest_time)(tTimerStruct *t); /**< Get the latest expiry time */
    /**
     * comparison function, which returns TRUE if event expires earlier than
     * given time
     */
    bool (*is_event_time_earlier_than)(tTimerStruct *t, TIME time);
} tEventsQueue;

/****************************************************************************
Private Constant Declarations
*/

/****************************************************************************
Private Function Prototypes
*/

/* Strict event queue functions */
static TIME get_strict_latest_time(tTimerStruct *event);
static bool is_strict_event_earlier_than(tTimerStruct *event, TIME event_time);

/* Casual event queue functions */
static TIME get_casual_latest_time(tTimerStruct *event);
static bool is_casual_event_earlier_than(tTimerStruct *event, TIME event_time);

/****************************************************************************
Global Variable Definitions
*/

/**
 * Indicates whether this module is currently servicing expired timers.
 *
 * This information is shared with the scheduler. Timers do not have a taskid
 * associated with them so an extra flag is needed when we're currently servicing
 * expired timers. Without this knowledge it's possible that an interrupt can
 * occur whilst servicing the timer and the scheduler will decide to service them
 * again based on the priority derived from its current_id.
 */
bool timer_being_serviced = FALSE;

/****************************************************************************
Private Variable Definitions
*/

/**
 * Event queue holding casual timed events
 */
static tEventsQueue casual_events_queue = { NULL, /**< first event */
        0, /**< last fired time */
        get_casual_latest_time,
        is_casual_event_earlier_than };

/**
 * Event queue holding strict, interrupt based timed events
 */
static tEventsQueue strict_events_queue = { NULL, /**< first event */
        0, /**< last fired time */
        get_strict_latest_time,
        is_strict_event_earlier_than };

/**
 * Does the scheduler need an interrupt to wake it up.
 */
static volatile bool scheduler_needs_wakeup = FALSE;
/**
 * The time at which the scheduler needs to be woken up to process background
 * tasks.
 */
static volatile TIME scheduler_wakeup_time;

#ifdef UNIT_TEST_BUILD
/**
 * Current event, is a global pointer to the event currently being serviced
 * This is only provided in unit tests so that it is possible to verify the
 * timer behaviour.
 */
static tTimerStruct *current_event = NULL;
#endif /* UNIT_TEST_BUILD */

/** timer id seed counter for short events */
static unsigned int short_event_count = MIN_UNIQUE_TIMER_ID_COUNT;
/** timer id seed counter for longer events */
static unsigned int long_event_count = MIN_UNIQUE_TIMER_ID_COUNT;

/****************************************************************************
Private Function Definitions
*/

/*
 * Downcasts from the tTimerStruct base class to the subclasses cause
 * complaints from lint that tStrictTimerStruct and tCasualTimerStruct are
 * larger than the structure being cast into them.
 *
 * There are ways of avoiding this using unions but they're not great. In
 * C++ we could have a proper class heirarchy. GreenWiki has a discussion
 * on the page PointerCastingIsEvil.
 *
 * Lint can be told that casts between particular types are safe. For the
 * moment we'll suppress in individual locations when we've reviewed each
 * suspicious cast.
 */

/*
 * Macro version for using in code that needs to be quick.
 */ 
#define fast_get_strict_expiry_time(event) \
    ((/*lint -e(1939)*/(tStrictTimerStruct *)(event))->event_time)

#define fast_get_strict_latest_time(event) \
    fast_get_strict_expiry_time(event)

/**
 * \brief Get the latest expiry time for given strict timed event
 *
 * \param[in] event pointer to event handle
 *
 * \return Expiry time.
 */
static TIME get_strict_latest_time(tTimerStruct *event)
{
    return fast_get_strict_latest_time(event);
}

/**
 * \brief Comparison function to sort strict events list
 *
 * \param[in] event pointer to event handle
 * \param[in] event_time Time to compare against
 *
 * \return TRUE if event expires later than given time, else FALSE
 */
static bool is_strict_event_earlier_than(tTimerStruct *event, TIME event_time)
{
    return time_le((/*lint -e(1939)*/(tStrictTimerStruct *)event)->event_time,event_time);
}

/**
 * \brief Get the expiry time for given casual timed event
 *
 * \param[in] event pointer to event handle
 *
 * \return Expiry time.
 */
#define fast_get_casual_expiry_time(event) \
    ((/*lint -e(1939)*/(tCasualTimerStruct *)(event))->earliest_time)

/*
 * Macro version for using in code that needs to be quick.
 */ 
#define fast_get_casual_latest_time(event) \
    ((/*lint -e(1939)*/(tCasualTimerStruct *)(event))->latest_time)

/**
 * \brief Get the latest expiry time for given casual timed event
 *
 * \param[in] event pointer to event handle
 *
 * \return Expiry time.
 */
static TIME get_casual_latest_time(tTimerStruct *event)
{
    return fast_get_casual_latest_time(event);
}

/*
 * Since casual events have two times, the definition of the "earliest" time
 * is a bit ambiguous.
 *
 * When we're going to sleep we want to sleep as long as possible, so we want
 * the earliest latest time (that is, the time after which at least one
 * casual timed event will be firing later than the latest it wanted to fire).
 *
 * When we're awake, we want to run as many events as possible now so that
 * we can avoid deep sleep-wake overheads. That means finding all timers whose
 * earliest time is due.
 *
 * The list can be sorted only one way. So, either we sort by latest time and
 * make it easy to find the time we want to wake up or we sort by earliest
 * time and make it easy to find timers that we could run.
 *
 * This function controls the sort order.
 *
 * We can always put in another variable for the other time and then maintain
 * it separately.
 */

/**
 * \brief Comparison function to sort casual events list
 *
 * \param[in] event pointer to event handle
 * \param[in] event_time Time to compare against
 *
 * \return TRUE if event expires later than given time, else FALSE
 */
static bool is_casual_event_earlier_than(tTimerStruct *event, TIME event_time)
{
    return time_le((/*lint -e(1939)*/(tCasualTimerStruct *)event)->latest_time, event_time);
}

/**
 * \brief Function to add a timed event to a queue. WARNING! Interrupts must be
 * locked around a call to this function.
 *
 * \param[in] event_queue pointer to event queue
 * \param[in] event New event to be added to the queue
 *
 * \return TRUE if the new timer is the earliest in the queue
 */
static bool add_event(tEventsQueue *event_queue, tTimerStruct *event)
{
    tTimerStruct **ppCurrentEvent;
    TIME event_time;

    event_time = event_queue->get_latest_time(event);

    /* loop through the timer handles and find out where to insert the new timer
     * Use the comparison function for the queue to find the position in the list */
    /*lint -e{722}*/for (ppCurrentEvent = &(event_queue->first_event);
         (*ppCurrentEvent != NULL) &&
         event_queue->is_event_time_earlier_than((*ppCurrentEvent), event_time);
         ppCurrentEvent = &(*ppCurrentEvent)->next);

    /* Add the event to the queue at the position */
    event->next = *ppCurrentEvent;
    *ppCurrentEvent = event;

    /* Was the new event earliest and now at the head of the queue? */
    if (event == event_queue->first_event)
    {
        return TRUE;
    }
    return FALSE;
}

/**
 * \brief Allocates a new unique timer ID
 *
 * \param[in] queue_flag strict/casual flag to be set in the timerID.
 * \param[in] event_time Timer expiry time
 *
 * \return returns the new timer ID
 */
static tTimerId get_new_timer_id(unsigned int queue_flag, TIME event_time)
{
    tTimerId new_id = queue_flag;
    unsigned int *p_counter = &short_event_count;
    /* Calculate the TIMER_TIME value that we will consider to be a long timer.
     *
     * NOTE: We do this outside of the time_le macro to ensure hal_get_time()
     * is only called once.
     */
    TIME curr_long_threshold = time_add(hal_get_time(),LONG_TIMER_THRESHOLD);

    if(time_le(curr_long_threshold, event_time))
    {
        new_id |= LONG_EVENT;
        p_counter = &long_event_count;
    }
    block_interrupts();
    /* Form the new id */
    new_id |= (*p_counter)++;
    if (*p_counter >= MAX_UNIQUE_TIMER_ID_COUNT)
    {
        *p_counter = MIN_UNIQUE_TIMER_ID_COUNT;
    }
    unblock_interrupts();
    return new_id;
}

/*
 * These are used in locations where they need to be blindingly fast.
 */

#define timers_get_next_event_time_int(queue, next_time)                \
    (                                                                   \
        NULL == queue ## _events_queue.first_event ? FALSE :            \
        (                                                               \
            (*(next_time) =                                             \
             fast_get_ ## queue ## _latest_time(                        \
                 queue ## _events_queue.first_event)),                  \
            TRUE                                                        \
        )                                                               \
    )

#define timers_get_next_strict_event_time_int(next_time) \
    timers_get_next_event_time_int(strict, (next_time))

#define timers_get_next_casual_event_time_int(next_time) \
    timers_get_next_event_time_int(casual, (next_time))

/**
 * \brief Configures and enables the next hardware timer. WARNING! This function
 * expects to be called with interrupts locked otherwise unexpected behaviour of
 * the timer hardware may result.
 */
static void set_next_hardware_timer(void)
{
    TIME fire_time;
    const bool has_strict =
        timers_get_next_strict_event_time_int(&fire_time);

    /*lint -save -esym(644, fire_time) if has_strict is true then fire_time
      will have been set. We check has_strict is true on all paths where we
      look at fire_time. In theory we could use -save and -restore directives
      to restrict the allowance just to the right areas of code. but in
      practice, this doesn't work. Even with -save and -restore, the
      suppression affects the rest of the function. */
    if (scheduler_needs_wakeup)
    {
        if (has_strict)
        {
            if (time_lt(fire_time, scheduler_wakeup_time))
            {
                HAL_SET_REG_TIMER1_TRIGGER(fire_time);
                hal_set_reg_timer1_en(1);
                return;
            }
        }
        HAL_SET_REG_TIMER1_TRIGGER(scheduler_wakeup_time);
        hal_set_reg_timer1_en(1);
    }
    else if (has_strict)
    {
        HAL_SET_REG_TIMER1_TRIGGER(fire_time);
        hal_set_reg_timer1_en(1);
    }
}

/*
 * create_add_strict_event
 */
tTimerId create_add_strict_event(
        TIME event_time, tTimerEventFunction event_fn, void *data_ptr)
{
    tStrictTimerStruct *new_event;
    tTimerId timer_id;

    patch_fn_shared(timers_create);

#ifdef DEBUG_KICK_TIMERS
    {
        TIME now = hal_get_time();
        if (time_sub(now, event_time) > MAX_EVENT_TIME_PAST)
        {
            panic_diatribe(PANIC_PL_TIMER_TOO_OLD, (uint16)event_time);
        }
    }
#endif

    new_event = pnew(tStrictTimerStruct);
    new_event->base.next = NULL;
    new_event->base.data_pointer = data_ptr;
    new_event->base.TimedEventFunction = event_fn;
    new_event->event_time = event_time;

    timer_id = get_new_timer_id(STRICT_EVENT, event_time);
    new_event->base.timer_id = timer_id;


    block_interrupts();
    /* If this changes the next timer to fire set it before re-enabling
     * the timer hardware */
    if (add_event(&strict_events_queue, &new_event->base))
    {
        /* Disable timer enable */
        hal_set_reg_timer1_en(0);
        /* Set timer registers and enable the timer hardware */
        HAL_SET_REG_TIMER1_TRIGGER(event_time);
        hal_set_reg_timer1_en(1);
    }
    unblock_interrupts();

    return timer_id;
}

static tTimerId populate_add_casual_event(tCasualTimerStruct *new_event,
                                          TIME earliest, TIME latest,
                                          tTimerEventFunction event_fn,
                                          void *data_ptr)
{
#ifdef SCHED_NO_TIMER_PREEMPTION
    /* Assert that we're not creating a timed_event that could pre-empt its
     * creating task */
    assert(!sched_is_running() ||
            current_task_priority() >= TIMED_EVENT_PRIORITY ||
            sched_in_interrupt());
#endif

    new_event->base.next = NULL;
    new_event->base.data_pointer = data_ptr;
    new_event->base.TimedEventFunction = event_fn;
    new_event->earliest_time = earliest;
    new_event->latest_time = latest;
    new_event->base.timer_id = get_new_timer_id(
                                 CASUAL_EVENT, earliest);

    block_interrupts();
    (void)add_event(&casual_events_queue, &new_event->base);
    unblock_interrupts();
    return new_event->base.timer_id;
}

/*
 * create_add_casual_event
 */
tTimerId create_add_casual_event(
        TIME earliest, TIME latest, tTimerEventFunction event_fn, void *data_ptr)
{
    tCasualTimerStruct *new_event;

    patch_fn_shared(timers_create);

    new_event = pnew(tCasualTimerStruct);
    return populate_add_casual_event(new_event, earliest, latest, event_fn,
                                     data_ptr);

}

/**
 * Structure containing the details of "alt" handler call.  Instances are
 * pointed to by \c tCasualTimerStruct's data_pointer and the wrapper handler
 * for "alt" events, \c alt_handler_wrapper, pulls the information out.
 */
typedef struct
{
    uint16 iarg; /**< Integer argument for the alt-style handler */
    void *task_data; /**< Data pointer argument for the alt-style handler */
    tTimerEventFunctionAlt fn; /**< Alt-style handler function */
} alt_handler_data;

/**
 * This structure exists purely to allow a \c tCasualTimerStruct and a \c
 * alt_handler_data to be allocated and freed in one call.  It is vital that
 * the tCasualTimerStruct be the first element in the structure because the
 * instance is always deleted through a pointer to that member, rather than
 * an explicit pointer to the whole structure.
 */
typedef struct
{
    /* IMPORTANT: tCasualTimerStruct event *must* be the first member of
     * this structure. */
    tCasualTimerStruct event;
    alt_handler_data alt; /**< Alt handler plus arguments */
} tAltCasualTimerStruct;

/**
 * Boilerplate wrapper registered as the handler for all "alt"-style events.
 * It simply interprets the supplied data pointer as an \c alt_handler_data and
 * calls the real handler inside it.
 * @param task_data
 */
static void alt_handler_wrapper(void *task_data)
{
    /* Unpack the real handler and its arguments from passed-in pointer and make
     * the call */
    alt_handler_data *hdl_data = (alt_handler_data *)task_data;
    hdl_data->fn(hdl_data->iarg, hdl_data->task_data);
}

tTimerId create_add_casual_event_alt(
        TIME earliest, TIME latest, tTimerEventFunctionAlt event_fn, uint16 iarg,
        void *data_ptr)
{
    tAltCasualTimerStruct *event = pnew(tAltCasualTimerStruct);
    /* Assert that the event member is the same thing you get by casting the
     * overall struct to a tCasualTimerStruct */
    assert(&event->event == (tCasualTimerStruct *)event);
    event->alt.iarg = iarg;
    event->alt.task_data = data_ptr;
    event->alt.fn = event_fn;
    return populate_add_casual_event((tCasualTimerStruct *)event,
                                     earliest, latest, alt_handler_wrapper,
                                     (void *)&event->alt);
}

/****************************************************************************
Public Function Definitions
*/

void init_pl_timers(void)
{
#if !defined(DESKTOP_TEST_BUILD)
    configure_interrupt(INT_SOURCE_TIMER1, INT_LEVEL_FG,
                                                        timer_service_routine);
#endif
}

#ifdef __KALIMBA__
tTimerId asm_timer_schedule_bg_event_in(
        INTERVAL time_in,
        tTimerEventFunction TimerEventFunction,
        void *data_pointer)
{
    return timer_schedule_bg_event_in(time_in, TimerEventFunction, data_pointer);
}
#endif

/*
 * NAME
 *   timer_cancel_event
 */
bool timer_cancel_event_ret(tTimerId timer_id, uint16 *piarg, void **ptask_data)
{
    tTimerStruct *cancel_event;
    tTimerStruct **ppCurrentEvent;
    tEventsQueue *event_queue = NULL;
    bool event_found = FALSE;

    patch_fn_shared(timers_cancel);

    if (TIMER_ID_INVALID == timer_id)
    {
        /* invalid timer id. Just return */
        return FALSE;
    }

    block_interrupts();
    if (EVENT_IS_STRICT(timer_id))
    {
        event_queue = &strict_events_queue;
        /* About to cancel a strict event. Disable timer */
        hal_set_reg_timer1_en(0);
    }
    else
    {
        event_queue = &casual_events_queue;
    }

    ppCurrentEvent = &(event_queue->first_event);

    while (NULL != (cancel_event = *ppCurrentEvent))
    {
        if (cancel_event->timer_id == timer_id)
        {
            /* Update the list */
            *ppCurrentEvent = cancel_event->next;

            /* Free the timer and return */

            /* If it's an alt event we need to grab the arguments from the alt
             * storage area */
            if (cancel_event->TimedEventFunction == alt_handler_wrapper)
            {
                if (piarg != NULL)
                {
                    *piarg = ((alt_handler_data *)(cancel_event->data_pointer))->iarg;
                }
                if (ptask_data != NULL)
                {
                    *ptask_data = ((alt_handler_data *)(cancel_event->data_pointer))->task_data;
                }
            }
            else if (ptask_data != NULL)
            {
                *ptask_data = cancel_event->data_pointer;
            }

            /* Note: in the case of alt events we rely on the fact that the
             * tCasualTimerStruct being deleted here is the first element in
             * the tAltCasualTimerStruct which was allocated to ensure that
             * the pfree matches the pnew. */
            pfree(cancel_event);
            event_found = TRUE;
            break;
        }
        ppCurrentEvent = &(*ppCurrentEvent)->next;
    }

    /* If there are still strict events then re-enable timers. The hardware will
     * fire if they are in the past so don't need to do a paranoid check. */
    if (event_queue == &strict_events_queue)
    {
        set_next_hardware_timer();
    }
    unblock_interrupts();
    return event_found;
}

/*
 * NAME
 *  timer_cancel_event_by_function
 */
void timer_cancel_event_by_function(tTimerEventFunction TimerEventFunction,
                                    void *data_pointer)
{
    tTimerStruct **ppCurrentEvent;
    tTimerStruct *event;

    patch_fn_shared(timers_cancel);

    /* Disable timers until the cancel routine completes */
    block_interrupts();
    hal_set_reg_timer1_en(0);

    ppCurrentEvent = &(strict_events_queue.first_event);

    /* Loop through the list and cancel all events with given event handler */
    while (NULL != (event = *ppCurrentEvent))
    {
        if ((TimerEventFunction == event->TimedEventFunction) &&
           ((data_pointer == NULL) || (data_pointer == event->data_pointer)))
        {
            /* Found an event with the given function.
             * Delete and continue search */
            *ppCurrentEvent = event->next;
            pfree(event);
            continue;
        }
        /* Go to the next event */
        ppCurrentEvent = &(*ppCurrentEvent)->next;
    }

    /* Next, search through the casual event queue and remove all casual events with
     * given event handler */
    ppCurrentEvent = &(casual_events_queue.first_event);

    /* Loop through the list and cancel all events with given event handler */
    while (NULL != (event = *ppCurrentEvent))
    {
        if ((TimerEventFunction == event->TimedEventFunction) &&
           ((data_pointer == NULL) || (data_pointer == event->data_pointer)))
        {
            /* Found an event with the given function.
             * Delete and continue search */
            *ppCurrentEvent = event->next;
            pfree(event);
            continue;
        }
        /* Go to the next event */
        ppCurrentEvent = &(*ppCurrentEvent)->next;
    }

    /* If there are still strict events then re-enable timers. The hardware will
     * fire if they are in the past so don't need to do a paranoid check. */
    set_next_hardware_timer();
    unblock_interrupts();
}

/* Note: the alt interface is only used for casual timers, so we only search
 * the casual timer queue here */
void timer_cancel_event_by_function_alt(tTimerEventFunctionAlt TimerEventFunction,
                                    uint16 iarg,
                                    void *data_pointer)
{
    tTimerStruct **ppCurrentEvent;
    tTimerStruct *event;

    block_interrupts();
    /* Search through the casual event queue and remove all casual events with
     * given event handler */
    ppCurrentEvent = &(casual_events_queue.first_event);

    /* Loop through the list and cancel all events with given event handler */
    while (NULL != (event = *ppCurrentEvent))
    {
        if (alt_handler_wrapper == event->TimedEventFunction)
        {
            alt_handler_data *hdl_data = (alt_handler_data *)event->data_pointer;
            if (hdl_data->fn == TimerEventFunction &&
                iarg == hdl_data->iarg &&
                ((data_pointer == NULL) || (data_pointer == hdl_data->task_data)))
            {
                /* Found an event with the given function.
                 * Delete and continue search */
                *ppCurrentEvent = event->next;
                pfree(event);
                continue;
            }
        }
        /* Go to the next event */
        ppCurrentEvent = &(*ppCurrentEvent)->next;
    }
    unblock_interrupts();
}

bool timer_cancel_strict_event_by_function_cmp(
            tTimerEventFunction TimerEventFunction,
            void *cmp_data,
            bool (*cmp_fn)(const void *timer_data,
                           const void *cmp_data),
            void **pmatched_data)
{
    tTimerStruct **ppCurrentEvent;
    tTimerStruct *event;

    block_interrupts();
    /* Search through the strict event queue and remove the first event with
     * given event handler that also matches according to the supplied
     * comparison function */
    ppCurrentEvent = &(strict_events_queue.first_event);

    while (NULL != (event = *ppCurrentEvent))
    {
        if ((TimerEventFunction == event->TimedEventFunction) &&
           ((cmp_fn == NULL) || cmp_fn(event->data_pointer, cmp_data)))
        {
            /* Found an event with the given function.
             * Delete and return */
            *ppCurrentEvent = event->next;
            unblock_interrupts();
            if (pmatched_data)
            {
                *pmatched_data = event->data_pointer;
            }
            pfree(event);
            return TRUE;
        }
        /* Go to the next event */
        ppCurrentEvent = &(*ppCurrentEvent)->next;
    }

    /* Didn't find one */
    unblock_interrupts();
    return FALSE;
}

/*
 * NAME
 *   timer_service_routine
 */
void timer_service_routine(void)
{
    /* N.B. THIS FUNCTION IS ONLY CALLED FROM THE INTERRUPT HANDLER. Normally it
     * would be necessary to lock and unlock interrupts around setting the new timer
     */

    /* Disable timer enable, we only want an interrupt to fire if we find a next
     * timer to set after servicing the interrupt.
     * timers_service_expired_strict_events will service any timers that have
     * expired. */
    hal_set_reg_timer1_en(0);
    block_interrupts();
    timers_service_expired_strict_events();
    unblock_interrupts();

    /* We can't guarantee that timer interrupts haven't been enabled again in
     * the interrupt so write disable again to be sure.
     */
    hal_set_reg_timer1_en(0);
    set_next_hardware_timer();

    /* Wake from shallow sleep to check if there is any work to do
     * (e.g. process casual events)
     */
    dorm_wake();
}

static void service_event(tTimerStruct *event)
{
    /* Call the handler function for this event */
    if (event->TimedEventFunction == NULL)
    {
        panic(PANIC_PL_TIMER_NO_HANDLER);
    }

#ifdef UNIT_TEST_BUILD
    /* Unit tests need access to the current event so we make it accessible */
    current_event = event;
#endif
    /* Unlock interrupts while calling the handler function */
    timer_being_serviced = TRUE;
    unblock_interrupts();
    event->TimedEventFunction(event->data_pointer);
    block_interrupts();
    timer_being_serviced = FALSE;

#ifdef UNIT_TEST_BUILD
    current_event = NULL;
#endif

    /* The event has finished it's useful life delete it */
    pfree(event);
}

/*
 * These needs to be blindingly fast, especially in the case where there are
 * events on the list but none are ready to fire. This function could easily
 * be called frequently.
 *
 * If we have to call an event then efficiency is slightly less important ao
 * a function call is OK.
 */

/*
 * NAME
 *   timers_service_expired_strict_events
 */
void timers_service_expired_strict_events(void)
{
    patch_fn_shared(timers_service);

    for(;;)
    {
        tStrictTimerStruct *event = /*lint -e(1939)*/
            (tStrictTimerStruct *) strict_events_queue.first_event;

        if (NULL == event || is_current_time_earlier_than(event->event_time))
            break;

        /*
         * The head of queue has expired. remove and service, noting the
         * expiry time 
         *
         * If we service an event then the queue may be manipulated so we
         * need to restart our search at the beginning.
         */
        strict_events_queue.first_event = event->base.next;
        strict_events_queue.last_fired = event->event_time;

        service_event(&event->base);
    }

    if (scheduler_needs_wakeup &&
        !is_current_time_earlier_than(scheduler_wakeup_time))
    {
        scheduler_needs_wakeup = FALSE;
        sched_wakeup_from_timers();
    }
}

/*
 * NAME
 *   timers_service_expired_casual_events
 */
void timers_service_expired_casual_events(void)
{
    tCasualTimerStruct *event;
    tTimerStruct **event_pos = &casual_events_queue.first_event;

    patch_fn_shared(timers_service);

    while(NULL != (event = /*lint -e(1939)*/(tCasualTimerStruct *) *event_pos))
    {
        if (!is_current_time_earlier_than(event->earliest_time))
        {
            /*
             * The event has expired. remove and service, noting the expiry
             * time.
             *
             * If we service an event then the queue may be manipulated so we
             * need to restart our search at the beginning.
             */
            *event_pos = event->base.next;
            casual_events_queue.last_fired = event->earliest_time;

            service_event(&event->base);

            event_pos = &casual_events_queue.first_event;
        }
        else
        {
            event_pos = &event->base.next;
        }
    }
}

/*
 * NAME
 *   timers_get_next_event_time
 */
bool timers_get_next_event_time(TIME *next_time)
{
    TIME casual_timer_deadline, strict_timer_deadline;
    bool has_casual_deadline, has_strict_deadline;

    /* This gets the 'latest' time of the next casual event */
    has_casual_deadline = timers_get_next_casual_event_time_int(&casual_timer_deadline);
    has_strict_deadline = timers_get_next_strict_event_time_int(&strict_timer_deadline);

    /*lint -save -esym(644, casual_timer_deadline, strict_timer_deadline) if
      has_x_deadline is true then x_timer_deadline will have been set. We
      check has_x_deadline is true on all paths where we look at
      x_timer_deadline. In theory we could use -save and -restore directives
      to restrict the allowance just to the right areas of code. but in
      practice, this doesn't work. Even with -save and -restore, the
      suppression affects the rest of the function. */

    if (has_casual_deadline && has_strict_deadline)
    {
        /* If the strict timer expires later than the casual one, use the latter. */
        if time_gt(strict_timer_deadline, casual_timer_deadline)
        {
            *next_time = casual_timer_deadline;
        }
        else
        {
            *next_time = strict_timer_deadline;
        }
        return TRUE;
    }
    else if (has_casual_deadline)
    {
        *next_time = casual_timer_deadline;
        return TRUE;
    }
    else if (has_strict_deadline)
    {
        *next_time = strict_timer_deadline;
        return TRUE;
    }

    return FALSE;
}

/*
 * NAME
 *   timer_n_us_delay
 */
void timer_n_us_delay(INTERVAL delay_duration_us)
{
    TIME now = hal_get_time();
    TIME end_time = time_add(now, delay_duration_us);
    patch_fn_shared(timers_service);

    /* do busy wait until timer time crosses the value */
    while (time_le(now, end_time))
    {
        now = hal_get_time();
    }
}

/*
 * NAME
 *   is_current_time_later_than
 */
bool is_current_time_later_than(TIME t)
{
    TIME now = hal_get_time();
    return (time_gt(now, t));
}

/*
 * NAME
 *   is_current_time_earlier_than
 */
bool is_current_time_earlier_than(TIME t)
{
    TIME now = hal_get_time();
    return (time_lt(now, t));
}

/*
 * NAME
 *   get_last_fire_time
 */
TIME get_last_fire_time(void)
{
    return strict_events_queue.last_fired;
}

/*
 * NAME
 *   get_last_casual_fire_time
 */
TIME get_last_casual_fire_time(void)
{
    return casual_events_queue.last_fired;
}

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
tTimerId timer_schedule_event_at(
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
tTimerId timer_schedule_event_in(
        INTERVAL time_in,
        tTimerEventFunction TimerEventFunction,
        void *data_pointer)
{
    return create_add_strict_event(
            time_add(hal_get_time(), time_in), TimerEventFunction, data_pointer);
}

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
tTimerId timer_schedule_bg_event_at(
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
tTimerId timer_schedule_bg_event_in(
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
tTimerId timer_schedule_bg_event_at_between(
        TIME time_earliest,
        TIME time_latest,
        tTimerEventFunction TimerEventFunction,
        void *data_pointer)
{
    return create_add_casual_event(
                time_earliest, time_latest, TimerEventFunction, data_pointer);
}

tTimerId timer_schedule_bg_event_at_between_alt(
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
tTimerId timer_schedule_bg_event_in_between(
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

tTimerId timer_schedule_bg_event_in_between_alt(
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

bool timer_get_time_of_bg_event(tTimerId timer_id, TIME *event_time)
{
    /* Search the casual event queue for the specified timerID.  If it is
     * found, return its event_time field. */

    tTimerStruct **ppCurrentEvent, *event;

    block_interrupts();
    ppCurrentEvent = &(casual_events_queue.first_event);

    while (NULL != (event = *ppCurrentEvent))
    {
        if (event->timer_id == timer_id)
        {
            /*lint -e1939 Yes, indeed, a downcast */
            tCasualTimerStruct *casual_event = (tCasualTimerStruct *)event;
            *event_time = casual_event->earliest_time;
            unblock_interrupts();
            return TRUE;
        }
        ppCurrentEvent = &event->next;
    }
    unblock_interrupts();
    return FALSE;
}

tTimerId timer_find_first_bg_event_by_fn_alt(tTimerEventFunctionAlt fn,
                                             uint16 *piarg,
                                             void **ptask_data)
{
    /* Search the casual event queue for the specified timerID.  If it is
     * found, return its event_time field. */

    tTimerStruct **ppCurrentEvent, *event;
    tTimerId id = TIMER_ID_INVALID;

    block_interrupts();
    ppCurrentEvent = &(casual_events_queue.first_event);

    while (NULL != (event = *ppCurrentEvent))
    {
        if (event->TimedEventFunction == alt_handler_wrapper)
        {
            alt_handler_data *alt_event = (alt_handler_data *)event->data_pointer;
            if (alt_event->fn == fn &&
                    (piarg == NULL || *piarg == alt_event->iarg) &&
                    (ptask_data == NULL || *ptask_data == alt_event->task_data))
            {
                id = event->timer_id;
                break;
            }
        }
        ppCurrentEvent = &event->next;
    }

    unblock_interrupts();
    return id;
}

/*
 * The following pair of functions are called every time the scheduler goes
 * in and out of sleep. This could be thousands of times per second. Keep
 * them blindingly fast.
 */

void timers_scheduler_needs_wakeup(void)
{
    TIME next_time;

    if (timers_get_next_casual_event_time_int(&next_time))
    {
        /*lint -esym(644, next_time) If we get onto this path then we know
          next_time was set. Note that this suppression affects the else clause
          as well, so manual review will be necessary. See comments about
          fire_time earlier. */

        scheduler_wakeup_time = next_time;
        scheduler_needs_wakeup = TRUE; /* Atomic */

        /*
         * If interrupts aren't blocked then the queue could change beneath
         * us and the dereference of first_event would become invalid.
         */
        block_interrupts();
        /*
         * This is morally equivalent to calling set_next_hardware_timer
         * but we know scheduler_needs_wakeup is TRUE so we can save some
         * instructions.
         */
        if (strict_events_queue.first_event == NULL ||
            !is_strict_event_earlier_than(strict_events_queue.first_event,
                                          next_time))
        {
            hal_set_reg_timer1_en(0);
            HAL_SET_REG_TIMER1_TRIGGER(next_time);
            hal_set_reg_timer1_en(1);
        }
        unblock_interrupts();
    }
    else
    {
        /*
         * This may be a non-existent path, but just to be safe, if this
         * function gets called twice without an intervening call to
         * timer_scheduler_has_awoken() and if the last time we were called
         * there was a casual timed event but this time there isn't then we
         * need to reset the interrupt timer.
         */
        timers_scheduler_has_awoken();
    }
}

void timers_scheduler_has_awoken(void)
{
    if (scheduler_needs_wakeup)
    {
        scheduler_needs_wakeup = FALSE;

        block_interrupts();
        hal_set_reg_timer1_en(0);
        set_next_hardware_timer();
        unblock_interrupts();
    }
}

void timer_cancel_all_bg(void)
{
    /* Loop through and cancel all the bg timed events */

    tTimerStruct **ppCurrentEvent, *event;

    block_interrupts();
    ppCurrentEvent = &(casual_events_queue.first_event);

    while (NULL != (event = *ppCurrentEvent))
    {
        ppCurrentEvent = &event->next;
        pfree(event);
    }

    casual_events_queue.first_event = NULL;
    unblock_interrupts();
}

#ifdef UNIT_TEST_BUILD
/*
 * NAME
 *   get_timer_expiry_time
 *
 */
TIME get_timer_expiry_time(tTimerId timer_id)
{
    tTimerStruct *event;
    tEventsQueue *event_queue = NULL;
    if (TIMER_ID_INVALID == timer_id)
    {
        /* invalid timer id. Just return */
        return 0;
    }

    if (EVENT_IS_STRICT(timer_id))
    {
        event_queue = &strict_events_queue;
    }
    else
    {
        event_queue = &casual_events_queue;
    }

    /* search through the events queue to find the event */
    for (event = event_queue->first_event;
         NULL != event && event->timer_id != timer_id; event = event->next);

    if (NULL == event)
    {
        /* the event may be being serviced as we speak check this */
        if(current_event->timer_id == timer_id)
        {
            event = current_event;
        }
        if (NULL == event)
        {
            return 0;
        }
    }

    if (EVENT_IS_STRICT(timer_id))
    {
        return fast_get_strict_expiry_time(event);
    }
    else
    {
        return fast_get_casual_expiry_time(event);
    }
}

/*
 * NAME
 *   get_timer_from_id
 *
 */
tTimerStruct *get_timer_from_id(tTimerId timer_id)
{
    tTimerStruct *event;
    tEventsQueue *event_queue = NULL;
    if (TIMER_ID_INVALID == timer_id)
    {
        /* invalid timer id. Just return */
        return NULL;
    }

    if (EVENT_IS_STRICT(timer_id))
    {
        event_queue = &strict_events_queue;
    }
    else
    {
        event_queue = &casual_events_queue;
    }

    /* search through the events queue to find the event */
    for (event = event_queue->first_event;
         NULL != event && event->timer_id != timer_id; event = event->next);

    if (NULL == event)
    {
        /* the event may be being serviced as we speak check this */
        if(current_event->timer_id == timer_id)
        {
            event = current_event;
        }
    }
    return event;
}

#ifdef DESKTOP_TEST_BUILD
/*
 * NAME
 *   PlRunTimers
 *
 */
void PlRunTimers(TIME curr_time)
{
    test_set_time(curr_time);
    /* call the ISR */
    timer_service_routine();
}

#endif /* DESKTOP_TEST_BUILD */
#endif /* UNIT_TEST_BUILD */

#ifdef DESKTOP_TEST_BUILD
void timer_run_expired_strict_events(void)
{
    block_interrupts();
    timers_service_expired_strict_events();
    unblock_interrupts();
}
#endif
