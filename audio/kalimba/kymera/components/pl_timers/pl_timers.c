/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 ************************************************************************//**
 * \file pl_timers.c
 * \ingroup pl_timers
 *
 * Implementation of platform timer functions.
 *
 ****************************************************************************/

#include "pl_timers.h"
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
/****************************************************************************
Private Constant Declarations
*/

/****************************************************************************
Private Function Prototypes
*/

/* Strict event queue functions */
static TIME get_strict_expiry_time(tTimerStruct *event);
static bool is_strict_event_earlier_than(tTimerStruct *event, TIME event_time);

/* Casual event queue functions */
static TIME get_casual_expiry_time(tTimerStruct *event);
static TIME get_casual_latest_time(tTimerStruct *event);
static bool is_casual_event_earlier_than(tTimerStruct *event, TIME event_time);

/****************************************************************************
Global Variable Definitions
*/
/**
 * Event queue holding casual timed events
 */
tEventsQueue casual_events_queue = { NULL, /**< first event */
        0, /**< last fired time */
        get_casual_expiry_time,
        get_casual_latest_time,
        is_casual_event_earlier_than };

/**
 * Event queue holding strict, interrupt based timed events
 */
tEventsQueue strict_events_queue = { NULL, /**< first event */
        0, /**< last fired time */
        get_strict_expiry_time,
        get_strict_expiry_time,
        is_strict_event_earlier_than };

/****************************************************************************
Private Variable Definitions
*/
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

/** Some cached memory which can be used for a new timer to save the effort of
 * doing a malloc. */

static tTimerStruct *cached_timer = NULL;
pmalloc_cached_report_handler pmalloc_cached_handler = NULL;

/****************************************************************************
Private Function Definitions
*/

/**
 * \brief Determine if the input argument is actually a tAltCasualTimerStruct.
 *        This relies on the fact that memory allocated by malloc has a header
 *        so 2 allocations can never be contiguous in memory.
 *
 * \return TRUE
 */
static inline bool is_tAltCasualTimerStruct(tTimerStruct *timer)
{
    if (timer->data_pointer == NULL)
    {
        return FALSE;
    }
    if (timer->data_pointer == ((void *)(timer + 1)))
    {
        return TRUE;
    }
    return FALSE;
}

/**
 * \brief Removes and returns the first strict event if it has expired
 *
 * \return Pointer to the expired timed event handle.
 */
static inline tTimerStruct *get_next_expired_strict_event(void)
{
    tTimerStruct *event = strict_events_queue.first_event;
    if (NULL != event && !is_current_time_earlier_than(event->variant.event_time))
    {
            /* the head of queue has expired. remove and return, noting the
             * expiry time */
            //strict_events_queue.first_event = event->base.next;
            strict_events_queue.first_event = event->next;
            strict_events_queue.last_fired = event->variant.event_time;
            return (tTimerStruct *)event;
    }
    return NULL;
}


/**
 * \brief Get the expiry time for given strict timed event
 *
 * \param[in] event pointer to event handle
 *
 * \return Expiry time.
 */
static TIME get_strict_expiry_time(tTimerStruct *event)
{
    return event->variant.event_time;
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
    return time_le( event->variant.event_time,event_time);
}

/**
 * \brief Removes and returns the first casual event that has expired
 *
 * \return Pointer to the expired timed event handle.
 */
static inline tTimerStruct *get_next_expired_casual_event(void)
{
    tTimerStruct *event;
    tTimerStruct **event_pos = &(casual_events_queue.first_event);

    while (NULL != (event = *event_pos))
    {
        if (!is_current_time_earlier_than(event->variant.casual.earliest_time))
        {
            /* the event has expired. remove and return, noting the expiry time */
            *event_pos = event->next;
            casual_events_queue.last_fired = event->variant.casual.earliest_time;
            return event;
        }
        event_pos = &(event->next);
    }
    /* None of the events in the queue has expired */
    return NULL;
}

/**
 * \brief Get the expiry time for given casual timed event
 *
 * \param[in] event pointer to event handle
 *
 * \return Expiry time.
 */
static TIME get_casual_expiry_time(tTimerStruct *event)
{
    return event->variant.casual.earliest_time;
}

/**
 * \brief Get the latest expiry time for given casual timed event
 *
 * \param[in] event pointer to event handle
 *
 * \return Expiry time.
 */
static TIME get_casual_latest_time(tTimerStruct *event)
{
    return event->variant.casual.latest_time;
}

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
    return time_le(event->variant.casual.latest_time, event_time);
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
 * \brief Allocates a new unique timer ID. This function MUST be called
 * with interrupts BLOCKED.
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
    /* Form the new id. - This is the critical bit and
     * requires interrupts to be blocked, for streamlining
     * of code the whole function requires this, as typically
     * the caller needs to block interrupts for other purposes
     * around this call. */
    new_id |= (*p_counter)++;
    if (*p_counter >= MAX_UNIQUE_TIMER_ID_COUNT)
    {
        *p_counter = MIN_UNIQUE_TIMER_ID_COUNT;
    }
    return new_id;
}

/**
 * \brief Configures and enables the next hardware timer. WARNING! This function
 * expects to be called with interrupts locked otherwise unexpected behaviour of
 * the timer hardware may result.
 */
static void set_next_hardware_timer(void)
{
    TIME fire_time;
    if (timers_get_next_event_time_int(&strict_events_queue, &fire_time))
    {
        /* Set timer registers and enable the timer hardware */
        HAL_SET_REG_TIMER1_TRIGGER(fire_time);
        hal_set_reg_timer1_en(1);
    }
}

/**
 * \brief Configures and enables the wakeup timer to ensure the chip is woken
 * from sleep to service a casual timer. WARNING! This function expects to be
 * called with interrupts locked otherwise unexpected behaviour of the timer
 * hardware may result.
 */
static void set_casual_timer_wakeup(void)
{
    TIME fire_time;
    if (timers_get_next_event_time_int(&casual_events_queue, &fire_time))
    {
        /* Set timer registers and enable the timer hardware */
        HAL_SET_REG_TIMER2_TRIGGER(fire_time);
        hal_set_reg_timer2_en(1);
    }
}

/*
 * create_add_strict_event
 */
tTimerId create_add_strict_event(
        TIME event_time, tTimerEventFunction event_fn, void *data_ptr)
{
    tTimerStruct *new_event;
    tTimerId timer_id;

    patch_fn_shared(timers_create);

#ifdef DEBUG_KICK_TIMERS
    {
        TIME now = hal_get_time();
        if (time_sub(now, event_time) > MAX_EVENT_TIME_PAST)
        {
            panic_diatribe(PANIC_AUDIO_TIMER_TOO_OLD, (DIATRIBE_TYPE)event_time);
        }
    }
#endif

    /* Interrupts need to be blocked whilst taking the cached timer if we can.
     * Then again while getting the new_timer_id. And again whilst the event
     * is added to the queue. To save effort, interrupts are blocked for the
     * whole sequence. We avoid doing this if the current context is interrupt
     * in the first place.
     */
    if (!is_current_context_interrupt())
    {
        block_interrupts();
    }
    if (cached_timer)
    {
        new_event = cached_timer;
        cached_timer = NULL;
    }
    else
    {
        new_event = pnew(tTimerStruct);
    }
    new_event->next = NULL;
    new_event->data_pointer = data_ptr;
    new_event->TimedEventFunction = event_fn;
    new_event->variant.event_time = event_time;

    timer_id = get_new_timer_id(STRICT_EVENT, event_time);
    new_event->timer_id = timer_id;

    /* If this changes the next timer to fire set it before re-enabling
     * the timer hardware */
    if (add_event(&strict_events_queue, (tTimerStruct *)new_event))
    {
        /* Disable timer enable */
        hal_set_reg_timer1_en(0);
        /* Set timer registers and enable the timer hardware */
        HAL_SET_REG_TIMER1_TRIGGER(event_time);
        hal_set_reg_timer1_en(1);
    }
    /* Unblock interrupts if we blocked them in the first place */
    if (!is_current_context_interrupt())
    {
        unblock_interrupts();
    }

    return timer_id;
}

/*
 * Casual-event wakeup timer2 interrupt handler
 * This doesn't do much, but the interrupt will wake from shallow sleep
 * and cause the timer expiry handler to run from the scheduler background
 */
void casual_kick_event(void)
{
    /* Wakeup timer expiry
     * We can't reschedule it here, because the casual timer
     * will still be in the queue until handled.
     */

    /* Let the scheduler know that it has something to do. This function expects
     * to be called from interrupt context */
    sched_background_kick_event();
}

static tTimerId populate_add_casual_event(tTimerStruct *new_event,
                                          TIME earliest, TIME latest,
                                          tTimerEventFunction event_fn,
                                          void *data_ptr)
{
#ifdef SCHED_NO_TIMER_PREEMPTION
    /* Assert that we're not creating a timed_event that could pre-empt its
     * creating task */
    assert(current_task_priority() >= TIMED_EVENT_PRIORITY);
#endif

    new_event->next = NULL;
    new_event->data_pointer = data_ptr;
    new_event->TimedEventFunction = event_fn;
    new_event->variant.casual.earliest_time = earliest;
    new_event->variant.casual.latest_time = latest;

    /* get_new_timer_id and adding this event to the queue need interrupts to be blocked  */
    block_interrupts();
    new_event->timer_id = get_new_timer_id(
                                 CASUAL_EVENT, earliest);

    /* If this changes the next timer to fire, set a new wakeup timer */
    if (add_event(&casual_events_queue, (tTimerStruct *)new_event))
    {
        /* Disable timer enable */
        hal_set_reg_timer2_en(0);
        /* Set timer registers and enable the timer hardware */
        HAL_SET_REG_TIMER2_TRIGGER(latest);
        hal_set_reg_timer2_en(1);
    }
    unblock_interrupts();
    return new_event->timer_id;
}

/*
 * create_add_casual_event
 */
tTimerId create_add_casual_event(
        TIME earliest, TIME latest, tTimerEventFunction event_fn, void *data_ptr)
{
    tTimerStruct *new_event;

    patch_fn_shared(timers_create);

    block_interrupts();
    if (cached_timer)
    {
        new_event = cached_timer;
        cached_timer = NULL;
        unblock_interrupts();
    }
    else
    {
        unblock_interrupts();
        new_event = pnew(tTimerStruct);
    }
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
    void *data; /**< Data pointer argument for the alt-style handler */
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
    tTimerStruct event;
    alt_handler_data alt; /**< Alt handler plus arguments */
} tAltCasualTimerStruct;

/**
 * Boilerplate wrapper registered as the handler for all "alt"-style events.
 * It simply interprets the supplied data pointer as an \c alt_handler_data and
 * calls the real handler inside it.
 * @param data
 */
static void alt_handler_wrapper(void *data)
{
    /* Unpack the real handler and its arguments from passed-in pointer and make
     * the call */
    alt_handler_data *hdl_data = (alt_handler_data *)data;
    hdl_data->fn(hdl_data->iarg, hdl_data->data);
}

tTimerId create_add_casual_event_alt(
        TIME earliest, TIME latest, tTimerEventFunctionAlt event_fn, uint16 iarg,
        void *data_ptr)
{
    tAltCasualTimerStruct *event = pnew(tAltCasualTimerStruct);
    /* Assert that the event member is the same thing you get by casting the
     * overall struct to a tCasualTimerStruct */
    assert(&event->event == (tTimerStruct *)event);
    event->alt.iarg = iarg;
    event->alt.data = data_ptr;
    event->alt.fn = event_fn;
    return populate_add_casual_event((tTimerStruct *)event,
                                     earliest, latest, alt_handler_wrapper,
                                     (void *)&event->alt);
}

static unsigned cached_timer_size(void)
{
    unsigned size = psizeof(cached_timer);
    if (pmalloc_cached_handler != NULL)
    {
        /* Call the next function in the chain of handlers,
         * and add that result to the cached timer size 
         */
        size += pmalloc_cached_handler();
    }
    return size;
}

/****************************************************************************
Public Function Definitions
*/

/*
 * init_pl_timers
 * 
 * Initialise timer module. 
 * This installs a handler to allow the pmalloc code to count the 
 * "cached" timer as free memory.
 */
void init_pl_timers(void)
{
    if (cached_timer == NULL)
    {
        cached_timer = pnew(tTimerStruct);
        /* Reporting functions are statically chained, 
         * so remember the previous one if any
         */
        pmalloc_cached_handler = pmalloc_cached_report(cached_timer_size);
    }
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



#ifdef __KALIMBA__
tTimerId asm_timer_schedule_event_in(
        INTERVAL time_in,
        tTimerEventFunction TimerEventFunction,
        void *data_pointer)
{
    return create_add_strict_event(
            time_add(hal_get_time(), time_in), TimerEventFunction, data_pointer);
}
#endif


/*
 * NAME
 *   timer_cancel_event
 */
bool timer_cancel_event_ret(tTimerId timer_id, uint16 *piarg, void **pdata)
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
        /* About to cancel a strict event. Disable timer1 */
        hal_set_reg_timer1_en(0);
    }
    else
    {
        event_queue = &casual_events_queue;
        /* About to cancel a casual event. Disable timer2 */
        hal_set_reg_timer2_en(0);
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
                if (pdata != NULL)
                {
                    *pdata = ((alt_handler_data *)(cancel_event->data_pointer))->data;
                }
            }
            else if (pdata != NULL)
            {
                *pdata = cancel_event->data_pointer;
            }

            if ((NULL == cached_timer) && !is_tAltCasualTimerStruct(cancel_event))
            {
                cached_timer = cancel_event;
            }
            else
            {
                /* Note: in the case of alt events we rely on the fact that the
                 * tCasualTimerStruct being deleted here is the first element in
                 * the tAltCasualTimerStruct which was allocated to ensure that
                 * the pfree matches the pnew. */
                pfree(cancel_event);
            }
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
    else
    {
        set_casual_timer_wakeup();
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
    hal_set_reg_timer2_en(0);

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
            if (NULL == cached_timer)
            {
                cached_timer = event;
            }
            else
            {
                pfree(event);
            }
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
            if ((NULL == cached_timer) && !is_tAltCasualTimerStruct(event))
            {
                cached_timer = event;
            }
            else
            {
                pfree(event);
            }
            continue;
        }
        /* Go to the next event */
        ppCurrentEvent = &(*ppCurrentEvent)->next;
    }

    /* If there are still strict events then re-enable timers. The hardware will
     * fire if they are in the past so don't need to do a paranoid check. */
    set_next_hardware_timer();
    set_casual_timer_wakeup();
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
    /* Disable timers until the cancel routine completes */
    hal_set_reg_timer2_en(0);

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
                ((data_pointer == NULL) || (data_pointer == hdl_data->data)))
            {
                /* Found an event with the given function.
                 * Delete and continue search */
                *ppCurrentEvent = event->next;
                if ((NULL == cached_timer) && !is_tAltCasualTimerStruct(event))
                {
                    cached_timer = event;
                }
                else
                {
                    pfree(event);
                }
                continue;
            }
        }
        /* Go to the next event */
        ppCurrentEvent = &(*ppCurrentEvent)->next;
    }

    set_casual_timer_wakeup();
    unblock_interrupts();
}

/*
 * NAME
 *   timers_service_expired_strict_events
 */
static inline void timers_service_expired_strict_events(void)
{
    tTimerStruct *event;

    patch_fn_shared(timers_service);

    while (NULL != (event = get_next_expired_strict_event()))
    {
#ifdef UNIT_TEST_BUILD
        /* Unit tests need access to the current event so we make it accessible */
        tTimerStruct local_event = *event;
        current_event = &local_event;
#endif
        tTimerEventFunction event_function = event->TimedEventFunction;
        void *event_data_pointer = event->data_pointer;

        /* Call the handler function for this event */
        if (event_function == NULL)
        {
            panic(PANIC_PL_TIMER_NO_HANDLER);
        }

        /* The event has finished its useful life, cache or delete it */
        if (NULL == cached_timer)
        {
            cached_timer = event;
        }
        else
        {
            pfree(event);
        }

        event_function(event_data_pointer);

#ifdef UNIT_TEST_BUILD
        current_event = NULL;
#endif
    }
}

/*
 * NAME
 *   timer_service_routine
 *
 *   NOTE: this is the timer service routine for the Timer1 hardware.
 */
void timer_service_routine(void)
{
    /* N.B. THIS FUNCTION IS ONLY CALLED FROM THE INTERRUPT HANDLER. Normally it
     * would be necessary to lock and unlock interrupts around setting the new timer
     */

    /* Disable timer enable, we only want an interrupt to fire if we find a next
     * timer to set after servicing the interrupt.
     * timers_service_experired_events will service any timers that have
     * expired. */
    hal_set_reg_timer1_en(0);

    timers_service_expired_strict_events();

    /* We can't guarantee that timer interrupts haven't been enabled again in
     * the interrupt so write disable again to be sure.
     */
    hal_set_reg_timer1_en(0);
    set_next_hardware_timer();
}



/*
 * NAME
 *   timers_service_expired_casual_events
 *
 *   NOTE: This function expects to be called with interrupts already blocked.
 */
void timers_service_expired_casual_events(void)
{
    tTimerStruct *event;
    TIME next_time;

    patch_fn_shared(timers_service);

    while (NULL != (event = get_next_expired_casual_event()))
    {
#ifdef UNIT_TEST_BUILD
        /* Unit tests need access to the current event so we make it accessible */
        tTimerStruct local_event = *event;
        current_event = &local_event;
#endif
        tTimerEventFunction event_function = event->TimedEventFunction;
        void *event_data_pointer = event->data_pointer;

        /* Call the handler function for this event */
        if (event_function == NULL)
        {
            panic(PANIC_PL_TIMER_NO_HANDLER);
        }

        /* The event has finished it's useful life delete it */
        if ((NULL == cached_timer) && !is_tAltCasualTimerStruct(event))
        {
            cached_timer = event;
        }
        else
        {
            pfree(event);
        }

        /* Unlock interrupts while calling the handler function */
        unblock_interrupts();
        event_function(event_data_pointer);
        block_interrupts();

#ifdef UNIT_TEST_BUILD
        current_event = NULL;
#endif
    }

    /* Disable timer enable */
    hal_set_reg_timer2_en(0);

    if (timers_get_next_event_time_int(&casual_events_queue, &next_time))
    {
        /* If there are any casual timers in the queue,
         * program a new wakeup timer
         */
        /* Set timer registers and enable the timer hardware */
        HAL_SET_REG_TIMER2_TRIGGER(next_time);
        hal_set_reg_timer2_en(1);
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
    has_casual_deadline = timers_get_next_event_time_int(&casual_events_queue, &casual_timer_deadline);
    has_strict_deadline = timers_get_next_event_time_int(&strict_events_queue, &strict_timer_deadline);

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
 *   timers_get_next_event_time_int
 */
bool timers_get_next_event_time_int(tEventsQueue *event_queue, TIME *next_time)
{
    if (NULL != event_queue->first_event)
    {
        *next_time = event_queue->get_latest_time(event_queue->first_event);
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
 *NAME
 *  is_current_time_later_than
 */

bool is_current_time_later_than(TIME t)
{
    TIME now = hal_get_time();
    return (time_gt(now, t));
}

/*
 *NAME
 *  is_current_time_earlier_than
 */

bool is_current_time_earlier_than(TIME t)
{
    TIME now = hal_get_time();
    return (time_lt(now, t));
}
/*
 * NAME
 *   get_last_casual_fire_time
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

            tTimerStruct *casual_event = (tTimerStruct *)event;
            *event_time = casual_event->variant.casual.earliest_time;
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
                                             void **pdata)
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
                    (pdata == NULL || *pdata == alt_event->data))
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
    return event_queue->get_expiry_time(event);
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

/*
 * Register definitions for host based tests.
 */

volatile unsigned  TIMER1_EN = 0;
volatile unsigned  TIMER1_TRIGGER = 0;
volatile unsigned  TIMER2_EN = 0;
volatile unsigned  TIMER2_TRIGGER = 0;
#ifdef HAVE_32BIT_TIMERS
volatile unsigned  TIMER1_TRIGGER_MS = 0;
volatile unsigned  TIMER2_TRIGGER_MS = 0;
#endif

static TIME timer_time;

void timer_run_expired_strict_events(void)
{
    timers_service_expired_strict_events();
}

/**
 * \brief Get current (simulated) time
 *
 * \note Dummy function included in non-kalimba builds 
 */
TIME hal_get_time(void)
{
    return timer_time;
}

/**
 * \brief Set current (simulated) time
 *
 * \note Dummy function included in non-kalimba builds 
 */
void test_set_time(TIME time)
{
    timer_time = time;
    /* Limit to match hardware if required */
    timer_time &= MAX_TIME;
}

/**
 * \brief Add delta to current (simulated) time
 *
 * \note Dummy function included in non-kalimba builds 
 */
void test_add_time(TIME_INTERVAL time_delta)
{
    timer_time += time_delta;
    /* Limit to match hardware if required */
    timer_time &= MAX_TIME;
}

#endif
