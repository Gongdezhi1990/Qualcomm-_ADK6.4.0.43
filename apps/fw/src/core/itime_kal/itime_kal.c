/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 *
 * Implementation of itime interface in terms of pl_timers, for using Hydra
 * generic code on Kalimbas
 *
 */

#include "itime_kal/itime_kal_private.h"

/**
 * We rely on the fact that the invalid timer ID value is also the value that
 * the CRT initialises statics to.
 */
static tTimerId current_tids[ITIME_LO_IDS + ITIME_HI_IDS + 1];

#define ITIME_EVENT_EXPAND_WRAPPER(t, f) \
static void f ## _wrap(void *task_data)                 \
{                                                       \
    UNUSED(task_data);                                  \
    current_tids[(t)] = 0;                              \
    f();                                                \
}
#ifndef DOXYGEN_IGNORE /* Doxygen just can't cope with this */
#if ITIME_LO_IDS
#define ITIME_EVENT_EXPAND_LO_WRAPPER_LO(t, f) \
        ITIME_EVENT_EXPAND_WRAPPER(t, f)
#define ITIME_EVENT_EXPAND_LO_WRAPPER_HI(t, f)
#endif
#if ITIME_HI_IDS
#define ITIME_EVENT_EXPAND_HI_WRAPPER_LO(t, f)
#define ITIME_EVENT_EXPAND_HI_WRAPPER_HI(t, f) \
        ITIME_EVENT_EXPAND_WRAPPER(t, f)
#endif
ITIME_EVENT_LIST(WRAPPER_LO)
ITIME_EVENT_LIST(WRAPPER_HI)
#endif

/**
 * The callback functions.  Keep these separate from the event
 * structures as they needed to be in initialised data.
 * Make sure these are in the same order as the itids: LO then HI.
 */
static const tTimerEventFunction itfuncs[ITIME_LO_IDS + ITIME_HI_IDS + 1] = {
#ifndef DOXYGEN_IGNORE /* Doxygen just can't cope with this */
#if ITIME_LO_IDS
#define ITIME_EVENT_EXPAND_LO_FUNCDEF_LO(t, f) f ## _wrap,
#define ITIME_EVENT_EXPAND_LO_FUNCDEF_HI(t, f)
#endif
#if ITIME_HI_IDS
#define ITIME_EVENT_EXPAND_HI_FUNCDEF_LO(t, f)
#define ITIME_EVENT_EXPAND_HI_FUNCDEF_HI(t, f) f ## _wrap,
#endif
    ITIME_EVENT_LIST(FUNCDEF_LO)
    ITIME_EVENT_LIST(FUNCDEF_HI)
    /* This last one is just for the annoying business with the comma. */
#endif
    (tTimerEventFunction)NULL
};

void init_itime(void)
{
    /* pl_timers initialisation is safe to call multiple times, so we don't
     * have to worry if someone else who's interested in the raw pl_timers
     * interface has got there before us. */
    init_pl_timers();
}

void itimed_event_in(INTERVAL delay, itid id)
{
    itimed_event_at(time_add(get_time(), (TIME)delay), id);
}

void itimed_event_at(TIME when, itid id)
{

    /* Cancel any currently pending event for this ID */
    (void)cancel_itimed_event(id);
    /* Call into pl_timers to post a timed event, and remember the id in
     * case of cancellation */
    current_tids[id] = timer_schedule_event_at(when, itfuncs[id], (void *)NULL);
}


bool cancel_itimed_event(itid id)
{
    /*
     * Couple of things to note here:
     * 1. There's logic inside timer_cancel_event that also checks the timerID
     * is valid, so this check is redundant except that timer_cancel_event
     * gives no indication of whether anything was pending, so to get something
     * like the right semantics for itime, we need our own check.
     * 2. There's a function in pl_timers to cancel an event based on the
     * handler function, which we could use to avoid storing current_tids.
     * However, that would mean we'd always have to search the full list of
     * pending timers regardless of whether there was a timer for this ID set,
     * so it seems more efficient on balance to store a few extra words instead.
     */
    if (current_tids[id])
    {
        timer_cancel_event(current_tids[id]);
        return TRUE;
    }
    return FALSE;
}

#ifdef NEXT_NEXT_ITIMED_EVENT
bool next_itimed_event(TIME *when)
{
    UNUSED(when);
    return FALSE;
}
#endif
