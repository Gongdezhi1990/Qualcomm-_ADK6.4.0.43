/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 ************************************************************************//**
 * \file
 * Kalimba scheduler - see header file for full description
 *
 * \ingroup platform
 *
 ****************************************************************************/
#include "sched_oxygen/sched_oxygen_private.h"

/****************************************************************************
Private Macro Declarations
*/

/**
 * We use a common source of "unique" identifier numbers.
 */
#define GET_MSGID() (msgid) get_scheduler_identifier()

/**
 * Construct a basic 24-bit taskid from task/bgint index (LS octet) and task
 * priority (lower 6 bits of MS octet).  The middle octet is always zero for
 * task IDs. The MS two bits are used to flag bg ints and coupled tasks.
 */
#define SET_PRIORITY_TASK_ID(task_priority,index) (task_priority << 16 | index)

/**
 * Set the bit indicating that the taskid refers to a bg int
 */
#define MARK_AS_BG_INT(id) ((id) |= BG_INT_FLAG_BIT)
/**
 * Set the bit indicating that the taskid refers to a task/bg int that is
 * "coupled" to a bg int/task.
 */
#define MARK_AS_COUPLED(id) ((id) |= BG_INT_TASK_IS_COUPLED_FLAG_BIT)

/**
 * Test whether two IDs are equal modulo the BG_INT_FLAG bit - i.e. either they
 * are the same ID or they are a coupled task/bg_int pair
 */
#define IDS_SAME_OR_COUPLED(id1, id2) \
                    (((id1) & ~BG_INT_FLAG_BIT) == ((id2) & ~BG_INT_FLAG_BIT))

/**
 * Maximum possible task index within a priority level
 */
#define MAX_TASK_INDEX    255

/**
 * \brief Macro to find the priority level of the highest priority task
 * from a priority mask.
 *
 * Assumes Mask is an unsigned int and is greater than 0.
 * Also due to how signdet works Mask cannot have its MSB set. This
 * restriction is fine as long as the number of bits in the mask is greater
 * than NUM_PRIORITIES, which is checked with an assertion in init_sched.
 *
 */
#ifndef DESKTOP_TEST_BUILD
#define MAP_PRIORITY_MASK_TO_HIGHEST_LEVEL(Mask) \
    ((uint16f)((UINT_BIT - 2) - (unsigned)PL_SIGNDET(((int)(Mask)))))
#else
#define MAP_PRIORITY_MASK_TO_HIGHEST_LEVEL(Mask) \
                                    map_priority_mask_to_highest_level(Mask)
static uint16f map_priority_mask_to_highest_level(unsigned int mask)
{
    uint16f highest = UINT_BIT - 1;
    while (! ((1 << highest) & mask))
    {
        highest--;
    }
    return highest;
}

#endif /* DESKTOP_TEST_BUILD */

/**
 * \brief Macro to calculate a priority mask from a priority level.
 * The mask calculated has the bits set for the priority supplied and all
 * lower priorities.
 *
 * Again assumes the number of bits in a unit24 is greater than NUM_PRIORITIES
 * (checked in init_sched)
 *
 */
 #define GET_PRIORITY_MASK(priority) \
                                ((uint16f) ((((uint16f) 1) << (priority + 1)) - 1))

/**
 * \brief Minimum delay for put_message_in() -- if less than this, just
 * PutMessage()
 */
#define MIN_PUT_MESSAGE_IN_DELAY ((INTERVAL) 10)


/****************************************************************************
Global Variable Definitions
*/

/* The following code relates to CarlOS-style task/bgint autogeneration */

/**
 * Create the array of task variables./
 */
/*@{*/

/** A variable for each task */
/*lint -e750 */
#define SCHED_TASK_START_EXPAND_ARRAYS(tsk, init_fn, tsk_fn, lvl) \
    SCHED_TASK_START_PRIORITY_EXPAND_ARRAYS(tsk, init_fn, tsk_fn, lvl, \
                                                            DEFAULT_PRIORITY)
#ifdef SCHED_MULTIQ_SUPPORT
#define SCHED_TASK_START_PRIORITY_EXPAND_ARRAYS(tsk, init_fn, tsk_fn, lvl, pri)\
    static MSGQ queue_ ## tsk [task_id_ ## tsk ## _n_queues];
#else
#define SCHED_TASK_START_PRIORITY_EXPAND_ARRAYS(tsk, init_fn, tsk_fn, lvl, pri)
#endif

/** No per-queue handling */
#define SCHED_TASK_QUEUE_EXPAND_ARRAYS(tsk, q)
/** No end-of-per-queue handling */
#define SCHED_TASK_END_EXPAND_ARRAYS(tsk)

SCHED_TASK_LIST(ARRAYS)

/*@}*/

/**
 * \name Create a table of tasks. The length is known (N_ALL_TASKS - 1)
 */
/*@{*/

#ifdef SCHEDULER_WITHOUT_RUNLEVELS
/** Run level argument is ignored */
#define TASK_RUNLEVEL(lvl)
#else /* SCHEDULER_WITHOUT_RUNLEVELS */
/** Run level becomes last element */
#define TASK_RUNLEVEL(lvl)                      \
    lvl,                        /* runlevel */
#endif /* SCHEDULER_WITHOUT_RUNLEVELS */

/** Generate element of TASK */
/*lint -e750 */
#define SCHED_TASK_START_EXPAND_TABLE(tsk, init_fn, tsk_fn, lvl) \
    SCHED_TASK_START_PRIORITY_EXPAND_TABLE(tsk, init_fn, tsk_fn, lvl, \
                                                             DEFAULT_PRIORITY)
#ifdef SCHED_MULTIQ_SUPPORT
#define SCHED_TASK_START_PRIORITY_EXPAND_TABLE(tsk, init_fn, tsk_fn, lvl, pri)\
    {\
        (taskid)(priority_task_id_ ## tsk),                             \
        FALSE,                                                          \
        init_fn,            /* init: initialisation function */         \
        tsk_fn,             /* handler */                               \
        queue_ ## tsk,      /* mqueues: array of queues */              \
        task_id_ ## tsk ## _n_queues, /* nqueues: number of queues */   \
        NULL,               /* flushmsg      */                         \
        NULL,               /* private memory area */                   \
        TASK_RUNLEVEL(lvl)                                              \
        NULL                /* next */                                  \
    },
#else
#define SCHED_TASK_START_PRIORITY_EXPAND_TABLE(tsk, init_fn, tsk_fn, lvl, pri)\
    {\
        (taskid)(priority_task_id_ ## tsk),                             \
        FALSE,                                                          \
        init_fn,            /* init: initialisation function */         \
        tsk_fn,             /* handler */                               \
        {NULL},             /* task queue */                            \
        NULL,               /* flushmsg      */                         \
        NULL,               /* private memory area */                   \
        TASK_RUNLEVEL(lvl)                                              \
        NULL                /* next */                                  \
    },
#endif

/** No per-queue handling */
#define SCHED_TASK_QUEUE_EXPAND_TABLE(tsk, q)
/** No end-of-per-queue handling */
#define SCHED_TASK_END_EXPAND_TABLE(tsk)

#ifndef SCHED_NO_STATIC_TASKS
TASK tasks [] =
{
    SCHED_TASK_LIST(TABLE)
};
#endif /* SCHED_NO_STATIC_TASKS */

/*@}*/


/**
 * \name BG int structures
 */
 /** Generate bg int wrappers that handle sched_oxygen's way of clearing bg ints
  * so that the CarlOS-style handlers we're working with don't have to notice
  * anything. The wrapper also takes a pointer to the bg int's private memory
  * area (which is shared with the partner task, if there is one).
  */
/*lint -e750 */
 #define BG_INT_EXPAND_WRAPPER(a, b) \
     BG_INT_PRIORITY_EXPAND_WRAPPER(a, b, DEFAULT_PRIORITY)

#define BG_INT_PRIORITY_EXPAND_WRAPPER(a, b, p)          \
 static void b ## _wrap(void **ppriv)                    \
 {                                                       \
     UNUSED(ppriv);                                      \
     b();                                                \
 }

BG_INT_LIST(WRAPPER)

/*@{*/
/** Generate BG_INTs from the list of bg ints */
/*lint -e750 */
#define BG_INT_EXPAND_STRUCT(a, b)\
    BG_INT_PRIORITY_EXPAND_STRUCT(a, b, DEFAULT_PRIORITY)
#define BG_INT_PRIORITY_EXPAND_STRUCT(a, b, p) {BG_INT_ID_AS_UINT(a), FALSE, \
                                                 b ## _wrap, FALSE, NULL, NULL},

/**
 * Array of service functions, indexed by bg_int number.
 * There is a NULL handler at the end so it will assemble with no tasks
 * defined
 */
BGINT bg_ints[] = {
    BG_INT_LIST(STRUCT)
    {(bg_int_ids)0, FALSE, (bg_int_fn)NULL, FALSE, NULL, NULL}
};

/*@}*/

#ifdef INSTALL_DM_ALIAS_DEBUG

#define DM_ALIAS_GUARD_SIZE 8
#define DM_ALIAS_GUARD_WORD 0x987654ul

#ifndef __GNUC__
_Pragma("datasection DM2_DEBUG_ALIAS")
#endif
unsigned dm2_debug_alias_data[DM_ALIAS_GUARD_SIZE] =
{
    DM_ALIAS_GUARD_WORD, DM_ALIAS_GUARD_WORD, DM_ALIAS_GUARD_WORD, DM_ALIAS_GUARD_WORD,
    DM_ALIAS_GUARD_WORD, DM_ALIAS_GUARD_WORD, DM_ALIAS_GUARD_WORD, DM_ALIAS_GUARD_WORD
};

#endif /* INSTALL_DM_ALIAS_DEBUG */


/****************************************************************************
Private Type Declarations
*/

/****************************************************************************
Private Constant Declarations
*/

/****************************************************************************
Private Variable Definitions
*/

/* TODO - make these private variables static. But until Xide can display
 * statics from outside the current scope, its useful having them global */

/**
 * The identifier of the current task or bg_int.  Bit 22 indicates which this
 * is; we use this to check that \c get_message is
 * being called in the right context.
 */
taskid *current_id;

/**
 * Number of messages and background interrupts in the system.
 */
volatile uint16f TotalNumMessages;

/**
 * Bit field representing priority of tasks that are ready to run, but have not
 * yet run. LSB represents LOWEST_PRIORITY from PRIORITY enum.
 * NOTE: The width of the type must be known in order for MAP_PRIORITY_MASK_TO_
 * HIGHEST_LEVEL to work.  But we also want to maximise efficiency.  So we use
 * unsigned int and rely on the macro UINT_BIT (defined in hydra_types.h) to
 * give us the width.
 */
unsigned int CurrentPriorityMask;

/**
 * Source of scheduler identifiers.  Init to 1; value 0 is special (NO_SCHID)
 */
scheduler_identifier NextSchedulerIdentifier = 1;

/**
 * Pointer to a previously relinquished message in order to optimise the
 * next call to put_message(). Avoides repeated calls to malloc and free as
 * messages are created and released
 */
static MSG *pCachedMessage = (MSG *) NULL;

TASKQ tasks_in_priority[NUM_PRIORITIES];

BG_INTQ bg_ints_in_priority[NUM_PRIORITIES];

#ifdef DESKTOP_TEST_BUILD
int SchedInterruptActive = 0;
#endif

#ifndef SCHEDULER_WITHOUT_RUNLEVELS
/** Flags used internally within the scheduler*/
static struct {
    /** Run level at which scheduler is currently operating - initialised to
     * illegal value (in init_sched) to simplify initialisation routine */
    uint16f current_runlevel;
} sched_flags;
#endif

/** A flag to allow us to exit a runlevel */
static bool run = TRUE;

/** Record that a casual wakeup timer fired */
static bool casual_wakeup_pending = FALSE;


#ifndef DISABLE_SHALLOW_SLEEP
#ifdef PROFILER_ON
/* Scheduler specific profiler. Mark the scheduler loop as started because the first
 * call for this (and only this)  profiler is stop. */
static profiler sched_loop = {UNINITIALISED_PROFILER,0,0,1,0,0};
#endif
#endif /* DISABLE_SHALLOW_SLEEP */

/****************************************************************************
Private Function Prototypes
*/

/****************************************************************************
Private Function Definitions
*/

/**
 * Cleanly remove all traces of a bg int from the scheduler's internals before
 * it is deleted.
 *
 * @param p_bgint Pointer to the \c BGINT to be flushed.
 *
 * \note This function should only be called with all the interrupts blocked,
 * ie, the whole operation should be atomic.
 */
static void flush_bg_ints(BGINT *p_bgint)
{
    uint16f priority_index = GET_TASK_PRIORITY(p_bgint->id);

    /* Check if there are any background interrupts raised for the task
     * and reduce the number of messages in system accordingly
     */
    if (p_bgint->raised)
        {
            /* If we've found a message, sched_n_messages should be > 0 */
            if ((TotalNumMessages == 0) ||
                (bg_ints_in_priority[priority_index].num_raised <= 0))
            {
                panic(PANIC_OXYGOS_INVALID_MESSAGE_COUNT);
            }
            TotalNumMessages--;
            bg_ints_in_priority[priority_index].num_raised--;
        p_bgint->raised = FALSE;
        }

    if (0 == tasks_in_priority[priority_index].num_msgs &&
            0 == bg_ints_in_priority[priority_index].num_raised)
    {
        /* if there are no more messages in this priority, ie, clear the current
         * priority masks.
         */
        CurrentPriorityMask &= (uint16f) (~(((uint16f) 1 << priority_index)));
    }
}

/**
 * \brief Clears all messages associated with the given task.  Note that if
 * a coupled task+bg int is being deleted, \c flush_bg_ints must be called too.
 *
 * \param[in] p_task pointer to the task structure
 *
 * \note This function should only be called with all the interrupts blocked,
 * ie, the whole operation should be atomic.
 */
static void flush_task_messages(TASK *p_task)
{
    uint16f priority_index = GET_TASK_PRIORITY(p_task->id);

#ifdef SCHED_MULTIQ_SUPPORT
    uint16f queue_index;
    /* Go through each message queue and remove the messages */
    for (queue_index = 0; queue_index < p_task->nqueues; queue_index++)
    {
        MSGQ *pQueue = &p_task->mqueues[queue_index];
#else
        MSGQ *pQueue = &p_task->mqueue;
#endif
        while (pQueue->first)
        {
            MSG *pMessage = pQueue->first;
            /* If we've found a message, sched_n_messages should be > 0 */
            if ((TotalNumMessages == 0) ||
                (tasks_in_priority[priority_index].num_msgs <= 0))
            {
                panic(PANIC_OXYGOS_INVALID_MESSAGE_COUNT);
            }
            /* remove the message from the queue */
            pQueue->first = pMessage->next;
            TotalNumMessages--;
            tasks_in_priority[priority_index].num_msgs--;
            if (NULL != pMessage->mv)
            {
                if (NULL != p_task->flush_msg)
                {
                    /* Call the registered flush message routine to reclaim
                     * the message body
                     */
                    p_task->flush_msg(pMessage->mi, pMessage->mv);
                }
                else
                {
                    /* Try and reclaim the body as it is. This would at least
                     * reduce the memory leak amount
                     */
                    pfree(pMessage->mv);
                }
            }

            if (pCachedMessage == (MSG *) NULL)
            {
                /* put the message in the cache */
                pCachedMessage = pMessage;
            }
            else
            {
                pfree((void *) pMessage);
            }
        } /* as long as there is a message in the queue */
#ifdef SCHED_MULTIQ_SUPPORT
    } /* loop through queues */
#endif
    if (0 == tasks_in_priority[priority_index].num_msgs &&
            0 == bg_ints_in_priority[priority_index].num_raised)
    {
        /* if there are no more messages in this priority, ie, clear the current
         * priority masks.
         */
        CurrentPriorityMask &= (uint16f) (~(((uint16f) 1 << priority_index)));
    }
}

#ifdef SCHED_NO_TIMER_PREEMPTION
/**
 * Handle timed events if we're at a safe priority level.
 * @param HighestPriorityLevel The priority level of the current context
 *
 * \note This function should be called with interrupts blocked.
 */
static void scheduler_service_expired_events(uint16f HighestPriorityLevel)
{
    if (HighestPriorityLevel <= TIMED_EVENT_PRIORITY)
    {
        /* Set the TIMED_EVENT_PRIORITY bit so that the timed event handlers
         * aren't descheduled */
        CurrentPriorityMask |= (1 << TIMED_EVENT_PRIORITY);

        timers_service_expired_casual_events();

        /* If the TIMED_EVENT_PRIORITY bit was only set because of timed events
         * and nothing else came in at that priority while they were running,
         * clear the bit again */
        if (HighestPriorityLevel < TIMED_EVENT_PRIORITY &&
                tasks_in_priority[TIMED_EVENT_PRIORITY].num_msgs == 0 &&
                bg_ints_in_priority[TIMED_EVENT_PRIORITY].num_raised == 0)
        {
            CurrentPriorityMask &= ~(1 << TIMED_EVENT_PRIORITY);
        }
    }
}
/**
 * Handle timed events after we emerge from sleep.
 *
 * \note This function is designed to be called without interrupts blocked.
 */
static void scheduler_service_expired_events_after_sleep(void)
{
    /* Set the TIMED_EVENT_PRIORITY bit so that the timed event handlers
     * aren't descheduled */
    block_interrupts();
    CurrentPriorityMask |= (1 << TIMED_EVENT_PRIORITY);
    timers_service_expired_casual_events();
    /* If the TIMED_EVENT_PRIORITY bit was only set because of timed events
     * and nothing else came in at that priority while they were running,
     * clear the bit again */
    if (tasks_in_priority[TIMED_EVENT_PRIORITY].num_msgs == 0 &&
            bg_ints_in_priority[TIMED_EVENT_PRIORITY].num_raised == 0)
    {
        CurrentPriorityMask &= ~(1 << TIMED_EVENT_PRIORITY);
    }
    unblock_interrupts();
}
#else /* SCHED_NO_TIMER_PREEMPTION */
#define scheduler_service_expired_events(HighestPriorityLevel)\
        timers_service_expired_casual_events()
#define scheduler_service_expired_events_after_sleep() \
        timers_service_expired_casual_events()
#endif /* SCHED_NO_TIMER_PREEMPTION */

void sched_background_kick_event(void)
{
#ifdef SCHED_NO_TIMER_PREEMPTION
    /* Set the CurrentPriorityMask to enable servicing of timed events.  Block
     * higher-level interrupts for the read-modify-write. */
    CurrentPriorityMask |= (1 << TIMED_EVENT_PRIORITY);
#endif /* SCHED_NO_TIMER_PREEMPTION */
    /* Record the wakeup so we stay awake until it's been handled */
    if (!casual_wakeup_pending)
    {
        casual_wakeup_pending = TRUE;
        TotalNumMessages++;
        sched_wake_up_background();
    }
}

void prune_bg_ints(uint16f priority)
{
    BGINT *b, **bposition;

    for (bposition = &bg_ints_in_priority[priority].first;
         (b=*bposition) != NULL &&
                 bg_ints_in_priority[priority].prunable;)
    {
        if (b->prunable)
        {
            /* Some one tried to delete this while bg int was running.
             * It is safe to delete now.
             */
            flush_bg_ints(b);
            /* update the list */
            *bposition = b->next;
            PL_PRINT_P1(TR_PL_SCHED_TASK_DELETE,
                        "Bg int 0x%06x deleted\n", b->id);
            pfree(b);
            bg_ints_in_priority[priority].prunable--;
        }
        else
        {
            bposition = &(*bposition)->next;
        }
    }
    assert(bg_ints_in_priority[priority].prunable == 0);
}

void prune_tasks(uint16f priority)
{
    TASK *t, **tposition;

    for (tposition = &tasks_in_priority[priority].first;
             (t=*tposition) != NULL &&
                     tasks_in_priority[priority].prunable;)
    {
        if (t->prunable)
        {
            flush_task_messages(t);
#ifdef SCHED_MULTIQ_SUPPORT
            pfree(t->mqueues);
#endif
            /* update the list */
            *tposition = t->next;
            PL_PRINT_P1(TR_PL_SCHED_TASK_DELETE,
                    "Task 0x%06x deleted\n", t->id);
            pfree(t);
            tasks_in_priority[priority].prunable--;
        }
        else
        {
            tposition = &(*tposition)->next;
        }
    }
    assert(tasks_in_priority[priority].prunable == 0);
}

/**
 * \brief Switch context to higher priority tasks if needed
 *
 *   This function is the core of the scheduler. It checks to see if a context
 *   switch is needed, based on the inputPriorityMask. Calls all tasks that
 *   are pending of HIGHER priority than the priorities set in the mask.
 *
 *   Note that this function must be re-entrant - a call to it may cause a
 *   higher priority task to run which itself may end up calling this function.
 *   But in this case each recursive call will have the inputPriorityMask set
 *   higher than the previous call, and eventually the stack will unwind and
 *   we'll always end up back at the original call again.
 *
 *   This should always be called with interrupts blocked.
 *   It will unblock them if it finds anything to do.
 *
 * \param[in] inputPriorityMask bit field with each bit representing  a priority
 * level, LSB representing LOWEST_PRIORITY, etc.
 *
 */
RUN_FROM_PM_RAM
static void check_context_switch(uint16f inputPriorityMask)
{
    /* Store original task/bg int, so we can restore it before returning to
     * original activity */
    taskid *orig_id = current_id;

    patch_fn(pl_context_switch);

    /*
     * While we have any tasks with priority HIGHER than that indicated by the
     * input mask, run these tasks
     */
    while (CurrentPriorityMask > inputPriorityMask)
    {
        uint16f HighestPriorityLevel;
        /*
         * Lookup highest priority that is ready to run, and range of tasks to
         * check Qs on. Initially there must be at least one message for this
         * priority level.
         */
        HighestPriorityLevel =
            MAP_PRIORITY_MASK_TO_HIGHEST_LEVEL(CurrentPriorityMask);

#ifdef SCHED_NO_TIMER_PREEMPTION
        /* If we're now running a low or lowest priority level and a
         * timed event is pending we need to ensure we break out of the
         * lower priority task at the right time to handle it.  Otherwise the
         * timed event could be delayed for ages, since we plan to use the
         * lower priority levels for particularly slow tasks */
        if (HighestPriorityLevel < TIMED_EVENT_PRIORITY)
        {
            (void)schedule_kick();
        }
        else
        {
             /* Check and cancel timer if some other interrupt has kicked us
              * into non-low-priority context */
            timer_cancel_event(wake_up_timer_id);
        }
#endif /* SCHED_NO_TIMER_PREEMPTION */

#ifdef HIGH_PRIO_CASUAL_TIMERS
        /* check for any casual timer expiry before checking messages and
         * background interrupts. Timers have highest priority.
         *
         * But for CarlOS-orientated code we need to be sure that a timed event
         * isn't going to pre-empt the task it's associated with, so only
         * service timed events if we're at a low enough priority level
         */
        scheduler_service_expired_events(HighestPriorityLevel);
#endif /* HIGH_PRIO_CASUAL_TIMERS */

        /* While there is a bg_int or task message at this priority keep
         * servicing them before moving to the next priority.
         */
        while ((bg_ints_in_priority[HighestPriorityLevel].num_raised > 0) ||
               (tasks_in_priority[HighestPriorityLevel].num_msgs > 0))
        {
            /* Run through all bg ints of this priority.  If any are raised,
             * run their handlers */
            if (bg_ints_in_priority[HighestPriorityLevel].num_raised > 0)
            {
                BGINT *b;

                /* Set a flag to prevent pre-empting code from actually removing
                 * bg ints from the list.  It's only strictly needed while
                 * interrupts are unblocked, but this is a more natural place to set
                 * it. */
                LOCK_BGINT_LIST_INTS_BLOCKED(HighestPriorityLevel);

                for (b = bg_ints_in_priority[HighestPriorityLevel].first;
                     b != NULL; b = b->next)
                /* Check if the task has any background interrupt raised */
                {
                    if (b->raised && !b->prunable)
                    {
                        if (NULL == b->handler)
                        {
                            panic(PANIC_OXYGOS_NULL_HANDLER);
                        }

                        /* Switch to the bg_int */
                        current_id = &b->id;

                        /* clear the background interrupt and decrement the Total Messages count.
                         * The bg int count is delayed until later as it indicates whether the
                         * loop can be exited. */
                        b->raised = FALSE;
                        TotalNumMessages--;

                        /* Need to write the appropriate thing for dynamic BG ints */
                        PL_PRINT_P1(TR_PL_SCHED_TASK_RUN,
                                                "Running bg int 0x%06x\n", b->id);
                        /* Unlock IRQs and call the handler function for this task */
                        unblock_interrupts();

                        b->handler(b->ppriv);

                        block_interrupts();

                        PL_PRINT_P1(TR_PL_SCHED_TASK_RUN,
                                              "Bg int 0x%06x completed\n", b->id);

                        /* If this is the last bg_int in this priority break out of the
                         * loop as there are no more interrupts to service. This test is
                         * almost free as we need to decrement
                         * Using a predecrement in the if statement  here to help
                         * the compiler to come up with better code */
                        if (--bg_ints_in_priority[HighestPriorityLevel].num_raised == 0)
                        {
                            /* No more bg ints are raised */
                            break;
                        }

                    }

    #ifdef HIGH_PRIO_CASUAL_TIMERS
                    /* check for any casual timer expiry before checking any more
                     * messages and background interrupts. Timers have highest
                     * priority.
                     */
                    scheduler_service_expired_events(HighestPriorityLevel);
    #endif /* HIGH_PRIO_CASUAL_TIMERS */
                }

                /* The queue is not in use anymore, so it's open for manipulation */
                UNLOCK_BGINT_LIST_INTS_BLOCKED(HighestPriorityLevel);
            }

            /*
             * Run through all tasks of this priority. For each task check all
             * its Qs to see if there are any messages. If so, run the task
             * handler
             */
            if (tasks_in_priority[HighestPriorityLevel].num_msgs > 0)
            {
                TASK *t;

                /* Set a flag to prevent pre-empting code from actually removing
                 * tasks from this list. */
                LOCK_TASK_LIST_INTS_BLOCKED(HighestPriorityLevel);

                for (t = tasks_in_priority[HighestPriorityLevel].first;
                     t != NULL; t = t->next)
                {
                    if (!t->prunable
    #ifndef SCHEDULER_WITHOUT_RUNLEVELS
                    && (t->runlevel <= sched_flags.current_runlevel)
    #endif
                        )
                    {
#ifdef SCHED_MULTIQ_SUPPORT
                        uint16f numQsRemaining;
                        MSGQ *q;

                        /*
                         * If any of the task's queues holds a message then call the
                         * task's handler.
                         */
                        for (numQsRemaining = t->nqueues, q = t->mqueues;
                             numQsRemaining != 0; numQsRemaining--, q++)
                        {
#else
                            MSGQ *q = &t->mqueue;
#endif

                            if (q->first != (MSG *) NULL)
                            {
                                /* Switch to the task */
                                current_id = &t->id;

                                PL_PRINT_P1(TR_PL_SCHED_TASK_RUN,
                                                "Running Task 0x%06x\n", t->id);
                                /* Unlock IRQs and call the handler function for this
                                 * task */
                                unblock_interrupts();

                                t->handler(&t->priv);

                                block_interrupts();

                                PL_PRINT_P1(TR_PL_SCHED_TASK_RUN,
                                            "Task 0x%06x completed\n", t->id);
    #ifdef HIGH_PRIO_CASUAL_TIMERS
                                /* check for any casual timer expiry before checking
                                 * any more messages and background interrupts. Timers
                                 * have highest priority.
                                 */
                                scheduler_service_expired_events(HighestPriorityLevel);
    #endif /* HIGH_PRIO_CASUAL_TIMERS */

#ifdef SCHED_MULTIQ_SUPPORT
                                /*
                                 * Once we've run the handler for this task once, for
                                 * any of its Qs, move on to the next task of this
                                 * priority. If there are still any messages left in
                                 * any of the Qs for this task we will run the handler
                                 * for this task again, after other tasks of this
                                 * priority have run
                                 */
                                break;
#endif
                            }
#ifdef SCHED_MULTIQ_SUPPORT
                        } /* end of for loop round Qs for this task */
#endif
                    }

                } /* end of for loop round tasks of this priority */

                UNLOCK_TASK_LIST_INTS_BLOCKED(HighestPriorityLevel);
            }
        }

        /* We've run all tasks of this priority - clear the bit
         * indicating there are tasks to run */
        CurrentPriorityMask &= (uint16f) (~(((uint16f) 1 << HighestPriorityLevel)));

    }/*End of while loop checking all priorities greater than inputPriorityMask */

    /* Restore the original activity */
    current_id = orig_id;
}


#ifdef SCHEDULER_WITHOUT_RUNLEVELS
/**
 * Call each task's init function (if it exists)
 */
static void init_tasks(void)
#else
/**
 * Call the init function for each task introduced by the change of runlevel (or
 * all tasks below the new runlevel if \c first_time is TRUE) and set the new
 * runlevel.
 * @param first_time TRUE if no task initialisation has been done yet
 * @param runlevel The new runlevel the scheduler is entering.
 */
static void init_tasks(bool first_time, uint16f runlevel)
#endif
{
    int n;

    for (n=0; n < N_TASKS; n++)
    {
        /* If there is an init function for this task, run it */
        if (tasks[n].init != (void (*)(void **))(NULL))
        {
#ifndef SCHEDULER_WITHOUT_RUNLEVELS
            if (tasks[n].runlevel <= runlevel &&
                (first_time || tasks[n].runlevel > sched_flags.current_runlevel))
#endif /* SCHEDULER_WITHOUT_RUNLEVELS */
            {
                current_id = &tasks[n].id;
                tasks[n].init(&( tasks[n].priv));

                /* Is there's an associated bg_int, point it at the same memory
                 * area.  This is harmless if the bg_int is not meant to be
                 * associated with the task because standalone bg ints don't have
                 * a notion of a memory area, so must ignore the pointer their
                 * handler is passed. */
                /*lint -e{661,662} Lint doesn't know that N_BG_INTS is the size of
                 * bg_ints. */
                if (n < N_BG_INTS)
                {
                    bg_ints[n].ppriv = &tasks[n].priv;
                }
            }
        }
    }
    current_id = NULL;

#ifndef SCHEDULER_WITHOUT_RUNLEVELS
    sched_flags.current_runlevel = runlevel;
#endif
}

/**
 * \fn static scheduler_identifier get_scheduler_identifier(void)
 *
 * \brief obtain an identifier from the scheduler
 *
 * \return A scheduler identifier
 *
 * NOTES
 * Actually, this just returns the next value from an incrementing
 * uint24, but it's pretty unlikely that this will cause problems.
 *
 * It's a bit more subtle than that: if this number wraps (pretty
 * unlikely!) then we wrap to a fraction of the range of a uint24. We
 * can vaguely guess that there may be a few elements of the system that
 * get created when the system starts and which exist until the system
 * reboots.  These might have identifiers. The approach of only giving
 * away the lower identifiers once should circumvent such nasties,
 * but it clearly isn't perfect.
 *
 * Setting NextSchedulerIdentifier initially to one prevents
 * this function returning the value zero. An identifier with value
 * zero has special meanings in some circumstances (NO_SCHID).
 */
#define MAX_SCHED_ID ((scheduler_identifier)0xFFFFFFU)
#define RESTART_SCHED_ID_BASE (MAX_SCHED_ID/32)

static scheduler_identifier get_scheduler_identifier(void)
{
    scheduler_identifier i;

    block_interrupts();
    i = NextSchedulerIdentifier;
    if (++NextSchedulerIdentifier == MAX_SCHED_ID)
    {
        NextSchedulerIdentifier = RESTART_SCHED_ID_BASE;
    }
    unblock_interrupts();

    return(i);
}

/**
 * NAME
 *  put_allocated_message
 *
 * \brief queue up an allocated message
 *
 * \param[in] queueId
 * \param[in] *pMessage
 *
 * FUNCTION
 * Add an allocated message to the appropriate queue.
 * The mi and *mv fields should already have been filled in.
 *
 * \return The msgid for the message.
 */
static msgid put_allocated_message(qid queueId, MSG *pMessage)
{
    taskid task_id = QID_TO_TASKID(queueId);
#ifdef SCHED_MULTIQ_SUPPORT
    uint16f queueIndex = QID_TO_QINDEX(queueId);
#endif
    uint16f priority_index = GET_TASK_PRIORITY(queueId);
    MSGQ *pQueue;
    TASK *pTask;
    msgid id;

    /* Check for valid tskid and queue number, and a valid message pointer */
    if (priority_index >= NUM_PRIORITIES ||
        NULL == tasks_in_priority[priority_index].first)
    {
        panic_diatribe(PANIC_OXYGOS_INVALID_TASK_ID, queueId);
    }
    /* Find task within the list */

    LOCK_TASK_LIST(priority_index); /* Prevent physical deletions as we traverse*/
    /*lint -e{722}*/for (pTask = tasks_in_priority[priority_index].first;
         (pTask) && (task_id != pTask->id); pTask = pTask->next);

    if (NULL == pTask)
    {
        panic_diatribe(PANIC_OXYGOS_INVALID_TASK_ID, queueId);
    }

#ifdef SCHED_MULTIQ_SUPPORT
    if (queueIndex >= pTask->nqueues)
    {
        panic_diatribe(PANIC_OXYGOS_INVALID_QUEUE_NUMBER, queueId);
    }
#endif

    /* if the message pointer is null something has gone wrong so panic */
    if (pMessage == (MSG *) NULL)
    {
        panic(PANIC_OXYGOS_SCHED_MSG_IS_NULL);
    }

#ifdef SCHED_MULTIQ_SUPPORT
    pQueue = &(pTask->mqueues[queueIndex]);
#else
    pQueue = &pTask->mqueue;
#endif

    pMessage->id = GET_MSGID();
    /* We have to store the message ID locally because if the message is going
     * to a higher-priority task it will have been freed by the time we return*/
    id = pMessage->id;
    pMessage->next = (MSG *) NULL;

    /*
     * Lock IRQs whilst we fiddle with message Qs.
     * TODO - SHOULD WE STORE LENGTH OF Q TO AVOID WHILE WITH IRQs LOCKED
     */
    block_interrupts();

    if (pTask->prunable)
    {
        /* This task is marked for deletion. Just flush the message */
        if (NULL!=pMessage->mv)
        {
            if (NULL!=pTask->flush_msg)
            {
                pTask->flush_msg(pMessage->mi, pMessage->mv);
            }
            else
            {
                pfree(pMessage->mv);
            }
        }
        /* The list has been updated now so the list can be unlocked */
        UNLOCK_TASK_LIST_INTS_BLOCKED(priority_index);

        if (pCachedMessage == (MSG *) NULL)
        {
            /* put the message in the cache */
            pCachedMessage = pMessage;
            unblock_interrupts();
        }
        else
        {
            unblock_interrupts();
            pfree((void *) pMessage);
        }
        /* There is no such thing as cancel message yet. so return MAX_SCHED_ID */
        return MAX_SCHED_ID;
    }
    else
    {
        /* Store the message on the end of the task's message chain.  */
        MSG **mq = &pQueue->first;
        while (*mq != (MSG *) NULL)
        {
            mq = &(*mq)->next;
        }
        *mq = pMessage;

        /* Increment message counts */
        TotalNumMessages++;
        sched_wake_up_background();
        tasks_in_priority[priority_index].num_msgs++;

        /* Set the bit representing that a task of this priority is ready to run */
        CurrentPriorityMask |= (unsigned) (1UL << priority_index);
    }
    UNLOCK_TASK_LIST_INTS_BLOCKED(priority_index);
    unblock_interrupts();

    /*
     * If the put_message was not called from an ISR, check for context switch.
     * For messages sent from an IRQ, context switch is checked on exit from IRQ
     */
    if (!SchedInterruptActive)
    {
            /* The scheduler may be idle at the moment, in which case the
             * context switch should NOT happen now and the message should be
             * handled from scheduler context when the main scheduler loop gets
             * to run.
             * NOTE: This will only happen in the non interrupt context if an
             * init message is sent from main() to get the scheduler rolling,
             * for example in host based tests, before calling the plsched();
             */
        if (NULL != current_id)
        {
            /* It's only worth doing a context switch if this action is sent
             * to a task higher than the current one. */
            uint16f cur_priority = GET_TASK_PRIORITY(*current_id);
            if (cur_priority < priority_index)
            {
                /*
                 * Set mask so all equal or lower priority bits to current task
                 * priority are set, so a HIGHER priority task causes a context
                 * switch to happen
                 */
                uint16f priorityMask = GET_PRIORITY_MASK(cur_priority);
                block_interrupts();
                check_context_switch(priorityMask);
                unblock_interrupts();
            }
        }
    }

    return id;
}

#ifndef DISABLE_SHALLOW_SLEEP
/*
 * Turn down the clock for "shallow sleep" power saving whilst in
 * idle loop.
 *
 * With COAL-based desktop builds for the Crescendo Apps subsystem,
 * sit in a loop waiting for the next itimed event to expire and then service
 * events.  This enables desktop tests to use the itime module with sched_oxygen.
 */
static void sched_sleep(void)
{
#if defined (DESKTOP_TEST_BUILD) && defined (SUBSYSTEM_APPS)

    TIME next_time;
    if (!timers_get_next_event_time_int(&strict_events_queue, &next_time))
    {
        return;
    }
    while (time_lt(hal_get_time(), next_time));
    timers_service_expired_strict_events();

#else /* APPS DESKTOP BUILD */


#if defined (DORM_MODULE_PRESENT) && !defined (UNIT_TEST_BUILD)
    {
        TIME latest;
        /* OXYGOS support for wakeup time ranges doesn't exist */
        if (timers_get_next_event_time(&latest))
        {
            dorm_sleep(latest, latest);
        }
        else
        {
            TIME throttle_timestamp = time_add(get_time(), SCHED_MAX_DELAY);
            dorm_sleep(throttle_timestamp, throttle_timestamp);
        }
    }
#else /* DORM_MODULE_PRESENT && !UNIT_TEST_BUILD */
    /* Just do shallow sleep if dorm not present */
    safe_enable_shallow_sleep();
#endif /* DORM_MODULE_PRESENT && !UNIT_TEST_BUILD */

#endif /* APPS DESKTOP BUILD */
}

#endif /* DISABLE_SHALLOW_SLEEP */

/****************************************************************************
Public Function Definitions
*/

/**
 * Tell the scheduler to quit the current runlevel.
 *
 * If there is no runlevel, exits the scheduler.
 */
void terminate_runlevel(void)
{
    /* On host, signal to main scheduler thread */
    HYDRA_SCHED_ACQUIRE_MUTEX();
    run = FALSE;
    HYDRA_SCHED_RELEASE_MUTEX(TRUE);
}

/**
 * initialise the scheduler.  Note that task initialisation is not done in this
 * call because this is dependent on the scheduler's current runlevel. It is
 * done at the start of \c sched() instead. (For simplicity of code layout,
 * this is true even if SCHEDULER_WITHOUT_RUNLEVELS is defined).
 */
void init_sched(void)
{
    int n;

    union {
        BGINT **ppb;
        TASK **ppt;
    } pcurrent_tail[NUM_PRIORITIES];

    /* Check that the scheduler is set up correctly */
    /* There must be at least NUM_PRIORITIES+1 bits in an unsigned int for the
     * code that uses PL_SIGNDET to determine the highest priority level to work.
     * Check this is true */
    assert(UINT_BIT > NUM_PRIORITIES);

    block_interrupts();

    /*
     * If current task is NULL, context switch will be called with input
     * priority mask corresponding to maximum priority, which will always be
     * less than current mask and so context switch will never happen from task
     * init functions or from interrupt handlers when scheduler is idle
     */
    current_id = NULL;
    TotalNumMessages = 0;
    CurrentPriorityMask = 0;

    /* Loop over the static bg_ints array putting the bg_ints into
     * bg_ints_in_priority in whatever priority level they should go in. We rely
     * on the CRT to zero-initialise first and num_raised */
    for (n = 0; n < NUM_PRIORITIES; ++n)
    {
        pcurrent_tail[n].ppb = &bg_ints_in_priority[n].first;
    }
    for (n = 0; n < N_BG_INTS; ++n)
    {
        int pri = GET_TASK_PRIORITY(bg_ints[n].id);
        *pcurrent_tail[pri].ppb = &bg_ints[n];
        pcurrent_tail[pri].ppb = &(bg_ints[n].next);
    }

    /* Loop over the static tasks array putting the tasks into
     * tasks_in_priority in whatever priority level they should go in. We rely
     * on the CRT to zero-initialise first and num_msgs */

    for (n = 0; n < NUM_PRIORITIES; ++n)
    {
        pcurrent_tail[n].ppt = &tasks_in_priority[n].first;
    }
    for (n = 0; n < N_TASKS; ++n)
    {
        int pri = GET_TASK_PRIORITY(tasks[n].id);
        *pcurrent_tail[pri].ppt = &tasks[n];
        pcurrent_tail[pri].ppt = &(tasks[n].next);
    }

#ifndef SCHEDULER_WITHOUT_RUNLEVELS
    /* Task initialisation now done in the sched() call on a per-runlevel basis.
      * Setting the run_level flag to this value triggers the task initialistion
      * in "first time" mode the first time through sched(). */
    sched_flags.current_runlevel = N_RUNLEVELS;
#endif

    unblock_interrupts();

}

#ifdef SCHEDULER_WITHOUT_RUNLEVELS
/**
 * The main function of the background task scheduler. This invokes
 * bg ints as they are raised and tasks as messages become available for them.
 */
void sched_norunlevels(void)
#else
/**
 * The main function of the background task scheduler. This invokes
 * bg ints as they are raised and tasks as messages become available for them.
 * It ignores tasks at higher runlevels than it was called at.
 *
 * Note that the scheduler can be halted by a call to \c terminate_runlevel.
 * This will cause it to exit once all bg ints and messages have been consumed.
 */
void sched(uint16f runlevel)
#endif
{
#ifndef SCHEDULER_WITHOUT_RUNLEVELS
    /* Sanity check on runlevel argument */
    if (runlevel >= N_RUNLEVELS)
    {
        panic(PANIC_HYDRA_INVALID_RUNLEVEL);
    }
#endif
    run = TRUE;

    /* Tasks' initialisation functions. */
#ifdef SCHEDULER_WITHOUT_RUNLEVELS
    init_tasks();
#else
    init_tasks(sched_flags.current_runlevel == N_RUNLEVELS, runlevel);
#endif

    /* Main scheduling loop - run until the runlevel terminates */
    while (run)
    {
        /* Do we have any messages pending on any tasks ? */
        if (TotalNumMessages != 0)
        {
            /*
             * Check all priority levels, and run any tasks that are ready
             * starting with highest priority task. InputMask is set so tasks
             * of all priorities are run
             */
            block_interrupts();
            check_context_switch(0);

            /* If the wakeup timer fired, check for any casual timed events
             * if there's nothing better to do
             */
            if ((casual_wakeup_pending) && (TotalNumMessages == 1))
            {
                casual_wakeup_pending = FALSE;
                TotalNumMessages--;
                /* There is probably a casual event to handle
                 * Note that the priority isn't actually used here,
                 * but the function / macro needs a parameter
                 */
                scheduler_service_expired_events(DEFAULT_PRIORITY);
            }
            unblock_interrupts();
        }
        else
        {
#ifndef DISABLE_SHALLOW_SLEEP
            PROFILER_STOP(&sched_loop);
            PROFILER_START(&sleep_time);
            sched_sleep();
            PROFILER_STOP(&sleep_time);
            PROFILER_START(&sched_loop);
#else

#ifdef DESKTOP_TEST_BUILD
            /* Make sure timers get handled in test builds without shallow sleep */
            test_run_timers();
#endif

#endif /* DISABLE_SHALLOW_SLEEP */

            block_interrupts();
            if (casual_wakeup_pending)
            {
                casual_wakeup_pending = FALSE;
                TotalNumMessages--;
            }
            /* Service expired casual events. */
            scheduler_service_expired_events_after_sleep();
            unblock_interrupts();

#ifdef DESKTOP_TEST_BUILD
            {
                TIME next_time;
                /* For these builds exit when we have nothing to do.
                 * Before bailing out, check one last time that we really are idle.
                 * In safe_enable_shallow_sleep we could have handled a timer expiry
                 * which queued up a message or bg_int, or perhaps scheduled
                 * another timer. */
                if (TotalNumMessages == 0 &&
                                        !timers_get_next_event_time(&next_time))
                {
                    return;
                }
            }
#endif
        }
#ifdef INSTALL_DM_ALIAS_DEBUG
        {
            int x;
            for (x = 0; (TotalNumMessages == 0) && (x < DM_ALIAS_GUARD_SIZE); x++)
            {
                if (dm2_debug_alias_data[x] != DM_ALIAS_GUARD_WORD)
                {
                    panic_diatribe(PANIC_AUDIO_DEBUG_MEMORY_CORRUPTION, x);
                }
            }
        }
#endif /* INSTALL_DM_ALIAS_DEBUG */


    } /* end of forever loop */
}

#ifndef SCHEDULER_WITHOUT_RUNLEVELS
/**
 * Set current run level.  This function is used to dynamically change the
 * runlevel while the scheduler is running.
 *
 * \param runlevel The new runlevel to run at.
 */
bool set_runlevel(uint16f runlevel)
{
    /* Sanity check on runlevel argument */
    if (runlevel >= N_RUNLEVELS)
    {
        panic(PANIC_HYDRA_INVALID_RUNLEVEL);
    }
    if (runlevel <= sched_flags.current_runlevel)
    {
        return FALSE;
    }

    init_tasks(FALSE, runlevel);

    return TRUE;
}
#endif /* SCHEDULER_WITHOUT_RUNLEVELS */

/**
 * Helper function that creates and populates a \c TASK object, for dynamic
 * task support.  \c TASKs inherit the current runlevel (if runlevels exist).
 *
 * @param num_of_queues Number of queues the task requires
 * @param task_data Pointer to private data for the task
 * @param handler The task's message handler.  Must not be NULL!
 * @param flush_msg The optional message flusher, invoked when the task is
 * deleted
 * @return Pointer to the allocated task, or NULL if
 * insufficient pool memory is available for the structures to be allocated.
 */
static TASK *new_task(uint16f num_of_queues, void *task_data,
                      void (*handler)(void **),
                      MSG_FLUSH_HANDLER flush_msg)
{
    TASK *pTask;
    /* Create some memory for the task. */
    pTask =xzpnew(TASK);
    if (pTask != NULL)
    {
#ifdef SCHED_MULTIQ_SUPPORT
        MSGQ *msg_queues = xzpmalloc(num_of_queues * sizeof(MSGQ));
        if (msg_queues == NULL)
        {
            /* Failed to allocate queues, so free the task struct */
            pfree(pTask);
            pTask = NULL;
        }
        else
#endif
        {
            /* Initialise elements of the new task as per input params */
            pTask->init = NULL;
            pTask->handler = handler;
            pTask->flush_msg = flush_msg;
            pTask->priv = task_data;

#ifdef SCHED_MULTIQ_SUPPORT
            pTask->mqueues = msg_queues;
            pTask->nqueues = (unsigned int)num_of_queues;
#else
            PL_ASSERT(num_of_queues <= 1);
#endif

#ifndef SCHEDULER_WITHOUT_RUNLEVELS
            pTask->runlevel = sched_flags.current_runlevel;
#endif
        }
    }
    return pTask;
}

/**
 * Helper function that creates and populates a \c BGINT, for dynamic task
 * support.
 *
 * \note There is some complexity involved in the handling of the private
 * data area as a result of the shared ownership of the area when a bg int is
 * coupled with a task. If the \c BGINT is coupled with a \c TASK, \c ptask_data
 * must be non-NULL - it is a pointer to the \c TASK's private data pointer,
 * which is stored in the \c BGINT structure in the \c ppriv field. On the other
 * hand, if the BGINT is not associated with a TASK its \c ppriv member will be
 * set to point to a private data pointer, which itself is set equal to
 * \c plocal_data (which may be NULL).
 *
 * \note: In the latter case, the hidden private data pointer is supplied by
 * allocating an \c UNCOUPLED_BGINT instead of a \c BGINT.  However, only the
 * \c BGINT part is returned; we rely upon the fact that the \c BGINT is the
 * first element in the \c UNCOUPLED_BGINT struct to ensure that the memory is
 * subsequently correctly freed.
 *
 * \param ptask_data Pointer to the associated \c TASK's private data pointer.
 * \c NULL if there is no associated \c TASK.
 * \param handler The bg int handler.  Must be supplied.
 * \param plocal_data Direct pointer to the \c BGINT's private memory.  This
 * must be \c NULL if \c ptask_data is not \c NULL.
 * \return Pointer to the newly-allocated \c BGINT structure, or NULL
 * if insufficient pool memory is available for the structures to be
 * allocated.
 */
static BGINT *new_bg_int(void **ptask_data,  void (*handler)(void **),
                         void *plocal_data)
{
    BGINT *p_bgint;

    if (ptask_data == NULL)
    {
        UNCOUPLED_BGINT *p_uncpldbgint = xzpnew(UNCOUPLED_BGINT);
        if (p_uncpldbgint != NULL)
        {
            assert((BGINT *)p_uncpldbgint == &p_uncpldbgint->base);
            p_bgint = (BGINT *)p_uncpldbgint;
            p_uncpldbgint->priv = plocal_data;
            p_bgint->ppriv = &p_uncpldbgint->priv;
        }
        else
        {
            p_bgint = NULL;
        }
    }
    else
    {
        assert(plocal_data == NULL);
        p_bgint = xzpnew(BGINT);
        if (p_bgint != NULL)
        {
            p_bgint->ppriv = ptask_data;
        }
    }

    if (p_bgint != NULL)
    {
        p_bgint->handler = handler;
        p_bgint->raised = FALSE;
    }
    return p_bgint;
}

/**
 * Creates a new task with the given parameters, initialises the new task and
 * returns the task id.
 */
bool create_task(
    PRIORITY task_priority, uint16f num_of_queues, void *task_data,
    void (*msg_handler)(void **), void (*bg_int_handler)(void **),
    MSG_FLUSH_HANDLER flush_msg, taskid *created_task_id)
{
    TASK **pp_task, *p_task = NULL;
    BGINT **pp_bgint, *p_bgint = NULL;
    uint16f index;
    taskid id = NO_TASK;

    if (NUM_PRIORITIES<=task_priority             ||
        /* msg handler has to be present if task has any message queues
         * Also a task has to have either background interrupt handler or msg
         * handler */
        (NULL==msg_handler && (num_of_queues || NULL==bg_int_handler)))
    {
        /* Basic checks to make sure that caller is asking for valid priority,
         * container to return task id, atleast 1 queue if msg handler is
         * specified and a valid task, which has atleast either of bg_int
         * handler or a msg_handler */
        panic(PANIC_OXYGOS_DYNAMIC_TASK_INVALID_PARAMS);
    }

    if (msg_handler)
    {
        p_task = new_task(num_of_queues, task_data, msg_handler, flush_msg);

        if (p_task == NULL)
        {
            /* Task struct creation failed, so get out */
            return FALSE;
        }
    }

    if (bg_int_handler)
    {
        /* If the bg int is coupled to a task, it must store a pointer to the
         * task's priv pointer, so that they are always looking at the same
         * memory space */
        if (p_task)
        {
            p_bgint = new_bg_int(&p_task->priv, bg_int_handler, NULL);
        }
        else
        {
            p_bgint = new_bg_int(NULL, bg_int_handler, task_data);
        }

        if (p_bgint== NULL)
        {
            /* BG struct creation failed, so get out.
             * If we also created a task structure, tidy that up as well
             */
            if (p_task)
            {
#ifdef SCHED_MULTIQ_SUPPORT
                pfree(p_task->mqueues);
#endif
                pfree(p_task);
            }
            return FALSE;
        }
    }

    assert(p_task || p_bgint);

    block_interrupts();

    /* Aquiring the task ID is a bit involved when there is both a
     * task and a bg int.  They need the same index. */
    /* Note: if, say, there's no task, we set pp_task to point to p_task (which
     * is NULL) because that makes it look like we've got to the end of the
     * task list immediately, and hence the loop just searches for a free bg
     * int index.  The same applies if there's no bgint. */

    pp_task = p_task ? &tasks_in_priority[task_priority].first : &p_task;
    pp_bgint = p_bgint? &bg_ints_in_priority[task_priority].first : &p_bgint;

    /* Loop over one both lists, stopping when there is a simultaneous gap in
     * both.*/
    for (index = 0; index <= MAX_TASK_INDEX; index++)
    {
        bool task_index_available =
                (!*pp_task) || (index != QID_TO_TSKID((*pp_task)->id));
        bool bgint_index_available =
                (!*pp_bgint) || (index != QID_TO_TSKID((*pp_bgint)->id));

        /* If we've found the same available index in all the lists, we're done.
         * Note that if we're only interested in one of the lists, the
         * "available" flag for the other will always be TRUE */
        if(bgint_index_available && task_index_available)
        {
            break;
        }

        /* Only advance along the list if the index variable has "caught up with"
         * the tskid of this element */
        if (!bgint_index_available)
        {
            pp_bgint = &(*pp_bgint)->next;
        }
        if (!task_index_available)
        {
            pp_task = &(*pp_task)->next;
        }
    }

    if (index > MAX_TASK_INDEX)
    {
        /* Too many tasks. Caller is unlikely to be able to recover, so panic
         * here */
        unblock_interrupts();
        panic(PANIC_OXYGOS_TOO_MANY_DYNAMIC_TASKS);
    }

    if (p_task)
    {
        /* Insert the task into the list and set its ID */
        p_task->next = *pp_task;
        *pp_task = p_task;
        p_task->id = SET_PRIORITY_TASK_ID(task_priority,index);
        p_task->prunable = FALSE;
        id = p_task->id;
    }

    if (p_bgint)
    {
        /* Insert the bg int into the list and set its ID */
        p_bgint->next = *pp_bgint;
        *pp_bgint = p_bgint;
        p_bgint->id = SET_PRIORITY_TASK_ID(task_priority,index);
        MARK_AS_BG_INT(p_bgint->id);
        p_bgint->prunable = FALSE;
        id = p_bgint->id;
    }

    unblock_interrupts();

    if (p_bgint && p_task)
    {
        /* If it's a coupled pair, mark as such for quicker deletion */

        MARK_AS_COUPLED(p_bgint->id);
        MARK_AS_COUPLED(p_task->id);
        /* In the coupled case, return the task ID (the bg int ID differs only
         * in one byte) */
        id = p_task->id;
    }

    if (created_task_id != NULL)
    {
        *created_task_id = id;
    }

    return (id != NO_TASK);
}

/**
 * Gets the message queue Id for the given task
 */
bool get_msg_qid_for_task(taskid task_id, uint16f queue_index, qid *pQueue_id)
{
    TASK *pTask;

    patch_fn_shared(pl_dynamic_tasks);

    if (GET_TASK_PRIORITY(task_id) >= NUM_PRIORITIES || NULL == pQueue_id)
    {
        panic(PANIC_OXYGOS_DYNAMIC_TASK_INVALID_PARAMS);
    }
    LOCK_TASK_LIST(GET_TASK_PRIORITY(task_id));
    for (pTask = tasks_in_priority[GET_TASK_PRIORITY(task_id)].first;
         NULL != pTask; pTask = pTask->next)
    {
        if (task_id == pTask->id)
        {
            UNLOCK_TASK_LIST(GET_TASK_PRIORITY(task_id));
#ifdef SCHED_MULTIQ_SUPPORT
            *pQueue_id = (qid)(((queue_index&0xFF)<<8)|task_id);
            return (queue_index < pTask->nqueues)?TRUE:FALSE;
#else
            *pQueue_id = (qid)(task_id);
            return TRUE;
#endif
        }
    }
    UNLOCK_TASK_LIST(GET_TASK_PRIORITY(task_id));
    return FALSE;
}

/**
 *  Deletes a scheduler task that was previously created
 *
 */
void delete_task(taskid id)
{
    TASK **pp_task;
    BGINT **pp_bgint;
    taskid task_id = NO_TASK, bgint_id = NO_TASK;
    uint16f priority = GET_TASK_PRIORITY(id);

    patch_fn_shared(pl_dynamic_tasks);

    if (priority >= NUM_PRIORITIES)
    {
        panic(PANIC_OXYGOS_DYNAMIC_TASK_INVALID_PARAMS);
    }

    /* Sort out whether we're dealing with a task, a bg int or a coupled pair */
    if (ID_IS_BG_INT_ID(id))
    {
        bgint_id = id;
        if (IS_COUPLED(id))
        {
            task_id = id & ~BG_INT_FLAG_BIT;
        }
    }
    else
    {
        task_id = id;
        if (IS_COUPLED(id))
        {
            bgint_id = id | BG_INT_FLAG_BIT;
        }
    }

    if (task_id != NO_TASK)
    {
        /* go through the task in priority list and find the task to be deleted */

        /* Remember if the queue was really running, then set the flag so
         * pre-empting calls don't restructure the list as we're walking it */
        LOCK_TASK_LIST(priority);
        /* Note that this lock doesn't stop tasks being added.  However, the
         * way we traverse the list is safe under additions: at worst our
         * traversal will skip over a newly-added element if we are interrupted
         * in the process of retrieving the next pointer. But in this function
         * we're not interested in newly-added elements, so that's OK.  The only
         * point at which we have to worry is when we do a physical deletion and
         * are resetting the previous element's next pointer.
         */
        for (pp_task = &tasks_in_priority[priority].first;
             NULL != *pp_task; pp_task = &(*pp_task)->next)
        {
            if (task_id == (*pp_task)->id)
            {
                TASK *pTask = *pp_task;
                /* We can't safely delete a task if its queue is in the middle
                 * of being processed by the scheduler.  In this case, just mark
                 * it for deletion and the scheduler will delete it when it's
                 * safe to do so */
                if (TASK_LIST_WAS_ALREADY_LOCKED(priority))
                {
                    if (!pTask->prunable)
                    {
                        block_interrupts();
                        pTask->prunable = TRUE;
                        tasks_in_priority[GET_TASK_PRIORITY(task_id)].prunable++;
                        unblock_interrupts();
                        PL_PRINT_P1(TR_PL_SCHED_TASK_DELETE,
                                "Task 0x%06x is queued for delete\n", pTask->id);
                    }
                    else
                    {
                        PL_PRINT_P1(TR_PL_SCHED_TASK_DELETE,
                                    "Task 0x%06x already queued for delete\n",
                                    pTask->id);
                    }
                }
                else
                {
                    /* We're free to mess with this task list, but mark the
                     * task prunable first so that it's skipped by the main
                     * scheduler loop so we don't end up deleting it under the
                     * scheduler's feet. */
                    pTask->prunable = TRUE;
                    block_interrupts();
                    flush_task_messages(pTask);
#ifdef SCHED_MULTIQ_SUPPORT
                    pfree(pTask->mqueues);
#endif
                    /* update the list */
                    *pp_task = pTask->next;
                    unblock_interrupts();
                    PL_PRINT_P1(TR_PL_SCHED_TASK_DELETE,
                            "Task 0x%06x deleted\n", pTask->id);
                    pfree(pTask);
                }
                break;
            } /* id matches */
        } /*loop through all tasks in priority */

        UNLOCK_TASK_LIST(priority);
    }

    if (bgint_id != NO_TASK)
    {
        /* go through the task in priority list and find the task to be deleted */
        LOCK_BGINT_LIST(priority);
        for (pp_bgint = &bg_ints_in_priority[priority].first;
             NULL != *pp_bgint; pp_bgint = &(*pp_bgint)->next)
        {
            if (bgint_id == (*pp_bgint)->id)
            {
                BGINT *p_bgint = *pp_bgint;
                if (BGINT_LIST_WAS_ALREADY_LOCKED(priority))
                {
                    if (!p_bgint->prunable)
                    {
                        block_interrupts();
                        p_bgint->prunable = TRUE;
                        bg_ints_in_priority[priority].prunable++;
                        unblock_interrupts();
                        PL_PRINT_P1(TR_PL_SCHED_TASK_DELETE,
                                    "Bg int 0x%06x is queued for delete\n",
                                    p_bgint->id);
                    }
                    else
                    {
                        PL_PRINT_P1(TR_PL_SCHED_TASK_DELETE,
                                    "Bg int 0x%06x already queued for delete\n",
                                    p_bgint->id);
                    }
                }
                else
                {
                    /* We're free to mess with this bg int list, but mark the
                     * bg int prunable first so that it's skipped by the main
                     * scheduler loop so we don't end up deleting it under the
                     * scheduler's feet. */
                    p_bgint->prunable = TRUE;
                    block_interrupts();
                    /* flush_bg_ints must be called with interrupts blocked */
                    flush_bg_ints(p_bgint);
                    /* update the list  */
                    *pp_bgint = p_bgint->next;
                    unblock_interrupts();
                    PL_PRINT_P1(TR_PL_SCHED_TASK_DELETE,
                                "Bg int 0x%06x deleted\n", p_bgint->id);
                    pfree(p_bgint);
                }
                break;
            } /* id matches */
        } /*loop through all bg ints in priority */

        UNLOCK_BGINT_LIST(priority);
    }
}

/**
 * Sends a message consisting of the integer mi and the void* pointer
 * mv to the message queue queueId.
 *
 * mi and mv are neither inspected nor changed by the
 * scheduler - the task that owns queueId is expected to make sense of the
 * values.  mv may be NULL
 *
 * If mv is not null then it will typically be a chunk of malloc()ed
 * memory, though there is no need for it to be so.   tasks should
 * normally obey the convention that when a message built with
 * malloc()ed memory is given to put_message() then ownership of the
 * memory is ceded to the scheduler - and eventually to the recipient
 * task.   I.e., the receiver of the message will be expected to free()
 * the message storage.
 *
 * Note also that this function must be re-entrant - a call to this function
 * may cause another task to run which itself may call this function.
 */

msgid put_message_with_routing(qid queueId, uint16 mi, void *mv, tRoutingInfo *routing)
{
    MSG *pMessage;

    patch_fn_shared(pl_msgs);

    /* Check there aren't too many messages */
    if (TotalNumMessages >= MAX_NUM_MESSAGES)
    {
      panic_diatribe(PANIC_OXYGOS_TOO_MANY_MESSAGES, TotalNumMessages);
    }

    PL_PRINT_P3(
        TR_PL_PUT_MESSAGE,
        "PL PutMessage called for Queue ID 0x%06x, message int %i, message "
        "pointer is %s\n", queueId,  mi, (NULL==mv)?"NULL":"Not NULL");

    block_interrupts();

    /* use the cached message if available */
    if (pCachedMessage != (MSG *) NULL)
    {
        pMessage = pCachedMessage;
        pCachedMessage = (MSG *) NULL;
        unblock_interrupts();
    }
    else
    {
        unblock_interrupts();
        /* pnew will either succeed or panic */
        pMessage = pnew(MSG);
    }

    /* Set the message parameters */
    pMessage->mi = mi;
    pMessage->mv = mv;

    if (routing == NULL)
    {
        pMessage->routing.src_id = 0;
        pMessage->routing.dest_id = 0;
    }
    else
    {
        pMessage->routing = *routing;
    }

    return put_allocated_message(queueId, pMessage);
}

/**
 * Raises a background interrupt to required task if not already raised.
 *
 * This function is usually called within an interrupt handler to signal an
 * event to a background task.
 */
void raise_bg_int(taskid task_id)
{
    uint16f priority_index;
    BGINT *p_bg_int;

    /* It is legal to pass a task's taskid on the understanding that there is a
     * directly corresponding bg int.  Hence we ensure the BG_INT_FLAG_BIT is
     * set. */
    task_id |= BG_INT_FLAG_BIT;
    priority_index = GET_TASK_PRIORITY(task_id);

    patch_fn_shared(pl_bgint);

    /* Check there arent too many messages */
    if (TotalNumMessages >= MAX_NUM_MESSAGES)
    {
      panic_diatribe(PANIC_OXYGOS_TOO_MANY_MESSAGES, TotalNumMessages);
    }

    /* Sanity checks that we have a valid tskid */
    if (priority_index >= NUM_PRIORITIES ||
        NULL == bg_ints_in_priority[priority_index].first)
    {
        panic_diatribe(PANIC_OXYGOS_INVALID_TASK_ID, task_id);
    }

    /* Find task within the list */
    /*lint -e722 There isn't supposed to a body to this for loop. */
    LOCK_BGINT_LIST(priority_index);
    for (p_bg_int = bg_ints_in_priority[priority_index].first;
         (p_bg_int) && (task_id != p_bg_int->id);
         p_bg_int = p_bg_int->next);

    if (NULL == p_bg_int)
    {
        panic_diatribe(PANIC_OXYGOS_INVALID_TASK_ID, task_id);
    }

    block_interrupts();
    /* Only raise bg interrupt if task is not up for deletion. It's far more
     * likely that the interrupt is already raised than it is prunable so test
     * that first. If either is true there is nothing to do just unwind */
    if (p_bg_int->raised || p_bg_int->prunable)
    {
        UNLOCK_BGINT_LIST_INTS_BLOCKED(priority_index);
        unblock_interrupts();
        return;
    }
    /* The required background interrupt is not set
     * Make sure background interrupt has a handler */
    if (NULL == p_bg_int->handler)
    {
        panic(PANIC_OXYGOS_NULL_HANDLER);
    }
    p_bg_int->raised = TRUE;

    /* Increment message counts */
    TotalNumMessages++;
    sched_wake_up_background();
    bg_ints_in_priority[priority_index].num_raised++;

    /* Set the bit representing that a task of this priority is ready to
     * run */
    CurrentPriorityMask |= (unsigned) (1UL << priority_index);

    UNLOCK_BGINT_LIST_INTS_BLOCKED(priority_index);
    unblock_interrupts();
    /*
     * If the raise_bg_int was not called from an ISR, check for context switch.
     * For bg_ints raised from an IRQ, context switch is checked on exit from IRQ
     */
    if (!SchedInterruptActive)
    {
            /* The scheduler may be idle at the moment, in which case the
             * context switch should NOT happen now and the bg_int should be
             * handled from scheduler context when the main scheduler loop gets
             * to run.
             * NOTE: This will only happen in the non interrupt context if an
             * init bg_int is sent from main() to get the scheduler rolling,
             * for example in host based tests, before calling the plsched();
             * So this is unlikely to ever be exercised.
             */
        if (NULL != current_id)
        {
            /* It's only worth doing a context switch if this action is sent
             * to a task higher than the current one. */
            uint16f cur_priority = GET_TASK_PRIORITY(*current_id);
            if (cur_priority < priority_index)
            {
                /*
                 * Set mask so all equal or lower priority bits to current task
                 * priority are set, so a HIGHER priority task causes a context
                 * switch to happen
                 */
                uint16f priorityMask = GET_PRIORITY_MASK(cur_priority);
                block_interrupts();
                check_context_switch(priorityMask);
                unblock_interrupts();
            }
        }
    }
}

/**
 * Raises a background interrupt to required task if not already raised.
 *
 * This function is usually called to cause an operator to do some processing.
 */
RUN_FROM_PM_RAM
void raise_bg_int_with_bgint(BGINT_TASK bg_int)
{
    BGINT *p_bg_int = (BGINT *)bg_int;
    patch_fn_shared(pl_bgint);

    /* Check there arent too many messages */
    if (TotalNumMessages >= MAX_NUM_MESSAGES)
    {
      panic_diatribe(PANIC_OXYGOS_TOO_MANY_MESSAGES, TotalNumMessages);
    }

    /* Only raise bg interrupt if one isn't already raised. If something jumps
     * in and services the already raised one before we leave this call then
     * it's been serviced which was the point of calling this function so we
     * can exit safely. */
    if (!p_bg_int->raised)
    {
        unsigned priority_index;
        if (!is_current_context_interrupt())
        {
            block_interrupts();
        }
        /* Something could have gotten in and raised an bg interrupt so check
         * again before doing the deed. If it did then there is nothing to do
         * except unblock any interrupts that were blocked. */
        if (p_bg_int->raised)
        {
            if (!is_current_context_interrupt())
            {
                unblock_interrupts();
            }
            return;
        }
        priority_index = GET_TASK_PRIORITY(p_bg_int->id);
        /* The required background interrupt is not set. */

        p_bg_int->raised = TRUE;
        /* Increment message counts */
        TotalNumMessages++;
        sched_wake_up_background();
        bg_ints_in_priority[priority_index].num_raised++;

        /* Set the bit representing that a task of this priority is ready to
         * run */
        CurrentPriorityMask |= (unsigned) (1UL << priority_index);

        if (!is_current_context_interrupt())
        {
            /* The scheduler may be idle at the moment, in which case the
             * context switch should NOT happen now and the bg_int should be
             * handled from scheduler context when the main scheduler loop gets
             * to run.
             * NOTE: This will only happen in the non interrupt context if an
             * init bg_int is sent from main() to get the scheduler rolling,
             * for example in host based tests, before calling the plsched();
             * So this is unlikely to ever be exercised.
             */
            if (NULL != current_id)
            {
                /* It's only worth doing a context switch if this action is sent
                 * to a task higher than the current one. */
                uint16f cur_priority = GET_TASK_PRIORITY(*current_id);
                if (cur_priority < priority_index)
                {
                    /*
                     * Set mask so all equal or lower priority bits to current task
                     * priority are set, so a HIGHER priority task causes a context
                     * switch to happen
                     */
                    uint16f priorityMask = GET_PRIORITY_MASK(cur_priority);
                    check_context_switch(priorityMask);
                }
            }
            unblock_interrupts();
        }
    }
}


/** A timed (delayed) message */
typedef struct  timed_msg_t {
    uint16   mi;      /**< The message's uint16. */
    void    *mv;     /**< The message's void*. */
    qid     queueId; /**< The message's delivery queue. */
} timed_msg;

static void deliver_timed_message(void *stuff)
{
    timed_msg *msg = (timed_msg *) stuff;
    qid qId;
    uint16 mi;
    void *mv;

    /*
     * By pulling the parameters out of the pmalloc space onto
     * the stack, we can free the pmalloc pool block earlier,
     * increasing the chance that one will be available for
     * put_message().
     */
    qId = msg->queueId;
    mi = msg->mi;
    mv = msg->mv;

    /* Recycle thoughtfully. */
    pdelete(msg);

    PL_PRINT_P2(TR_PL_PUT_MESSAGE,
                "Deliver timed message for qId %d at time %d \n", qId,
                hal_get_time());

    /* Ignore the msgid that gets returned here. We can't really use it
     * for anything anyway. */
    (void) put_message(qId, mi, mv);
}

/**
 * eventually send a message to a task
 */
tTimerId put_message_at(TIME deadline, qid queueId, uint16 mi, void *mv)
{
    timed_msg *tm = pnew(timed_msg);

    patch_fn_shared(pl_msgs);

    /* Package the timed message. */
    tm->queueId = queueId;
    tm->mi = mi;
    tm->mv = mv;

    /* Deliver message via a timed event. */
    return timer_schedule_event_at(deadline, deliver_timed_message,
                                   (void *)tm);
}

/**
 * eventually send a message to a task
 */
tTimerId put_message_in(INTERVAL delay, qid queueId,
                        uint16 mi, void *mv)
{
    patch_fn_shared(pl_msgs);

    /* If the delay is too short, send the message straight away. */
    if (time_lt(delay, MIN_PUT_MESSAGE_IN_DELAY))
    {
        /* Ignore the msgid that gets returned here. We can't really use it
         * for anything anyway. */
        (void) put_message(queueId, mi, mv);
        return TIMER_ID_INVALID; /* No timer was set up */
    }
    else
    {
        return put_message_at(time_add(hal_get_time(), delay), queueId,
                              mi, mv);
    }
}

bool cancel_timed_message(qid q, tTimerId timer_id, uint16 *pmi, void **pmv)
{
    void *tmsg;

    UNUSED(q);

    if (!timer_cancel_event_ret(timer_id, NULL, &tmsg))
    {
        return FALSE;
    }

    if (pmi)
    {
        *pmi = ((timed_msg *)tmsg)->mi;
    }
    if (pmv)
    {
        *pmv = ((timed_msg *)tmsg)->mv;
    }
    pfree(tmsg);
    return TRUE;
}


/**
 * Obtains a message from the message queue ID queueId if one is available.
 * The calling task must own this Q. The message consists of one or both
 * of a int and a void*.
 *
 * If the calling task does not own the Q then the scheduler asserts.
 *
 * If a message is taken from the queue, then *pmi and *pmv
 * are set to the mi and mv passed to put_message().
 *
 * pMessageInt and/or pmv can be null, in which case the
 * corresponding value from the message is discarded.
 *
 * If the function returns TRUE, and if pmv is null, then there is a
 * danger of a memory leak.  The scheduler does not know what is stored
 * in the void*, so it cannot reclaim a chunk of malloc()ed memory
 * there.   In most circumstances pmv should not be null, however,
 * it may be that the application knows that all messages sent on "q" will use
 * the "MessageInt" only, so it may be acceptable for pmv to be null.
 *
 * MUST NOT BE CALLED DIRECTLY FROM INTERRUPT CODE - can only be called by the
 * task that owns the queue given by queueId.
 */
bool get_message_with_routing(qid queue_id, uint16 *pmi,void **pmv, tRoutingInfo *routing)
{
    MSG *pMessage;
    MSGQ *pQueue;
    taskid task_id = QID_TO_TASKID(queue_id);
#ifdef SCHED_MULTIQ_SUPPORT
    uint16f queueNumber = QID_TO_QINDEX(queue_id);
#endif
    uint16f priority_index = GET_TASK_PRIORITY(queue_id);
    TASK *current_task;

    patch_fn_shared(pl_msgs);

    /* Check for valid taskid and make sure a task is only allowed to read from
     * its own queues. */
    if (priority_index >= NUM_PRIORITIES                     ||
        NULL == tasks_in_priority[priority_index].first      ||
        NULL == current_id                                   ||
        task_id != *current_id                               ||
        ID_IS_BG_INT_ID(queue_id))
    {
        panic_diatribe(PANIC_OXYGOS_INVALID_TASK_ID, queue_id);
    }

    /* Obtain the task structure that the current_id belongs to.*/
    current_task = STRUCT_FROM_MEMBER(TASK, id, current_id);

#ifdef SCHED_MULTIQ_SUPPORT
    if (queueNumber >= current_task->nqueues)
    {
        panic_diatribe(PANIC_OXYGOS_INVALID_QUEUE_NUMBER, queue_id);
    }

    /* Look up the message queue pointer for the given qid */
    pQueue = &(current_task->mqueues[queueNumber]);
#else
    pQueue = &current_task->mqueue;
#endif

    pMessage = pQueue->first;

    /* If this message queue pointer isnt null, there is a message. Get the
     * unit and void * pointer from the message if required, and remove the
     * message from the queue */
    if (pMessage != (MSG *)(NULL))
    {
        if (pmi != (uint16 *)(NULL))
        {
            *pmi = pMessage->mi;
        }

        if (pmv != (void **)(NULL))
        {
            *pmv = pMessage->mv;
        }

        if (routing != NULL)
        {
            *routing = pMessage->routing;
        }

        /* If we've found a message, sched_n_messages should be > 0 */
        if ((TotalNumMessages == 0) ||
            (tasks_in_priority[priority_index].num_msgs <= 0))
        {
            panic(PANIC_OXYGOS_INVALID_MESSAGE_COUNT);
        }

        /* Need to lock IRQs while we change message counts and fiddle with
         * message Q */
        block_interrupts();

        pQueue->first = pMessage->next;
        TotalNumMessages--;
        tasks_in_priority[priority_index].num_msgs--;

        PL_PRINT_P3(
            TR_PL_GET_MESSAGE,
            "PL Got a message for queue ID 0x%06x, Message ID %i, Message "
            "pointer is %s\n", queue_id, pMessage->mi,
            (NULL==pMessage->mv)?"NULL":"Not NULL");

        if (pCachedMessage == (MSG *) NULL)
        {
            /* put the message in the cache */
            pCachedMessage = pMessage;
            unblock_interrupts();
        }
        else
        {
            unblock_interrupts();
            pfree((void *) pMessage);
        }

        return(TRUE);
    }

    /* If we get here there was no message */
    PL_PRINT_P1(
        TR_PL_GET_MESSAGE,
        "PL GetMessage called for queue ID 0x%06x, but no message to get\n",
        queue_id);
    return(FALSE);
}

/**
 * \brief Called from interrupt handler when a message has been sent in an ISR
 * Checks if context switch is needed.
 *
 */
void exit_irq_check_context_switch(void)
{
    if (run)
    {
        uint16f priorityMask;
        /* Check if context switch is needed. Set mask so message for task with
         * HIGHER priority than current task will cause context switch */
        if (NULL != current_id)
        {
            /*
             * Set mask so all equal or lower priority bits to current task priority
             * are set, so a HIGHER priority task causes a context switch to happen
             */
            priorityMask = GET_PRIORITY_MASK(GET_TASK_PRIORITY(*current_id));
            /* Don't block / unblock interrupts here
             * It's done in the interrupt handler before calling this function.
             */
            check_context_switch(priorityMask);
        }

    }
}

uint16f current_task_priority(void)
{
    if (!CurrentPriorityMask)
    {
        return 0U;
    }
    return MAP_PRIORITY_MASK_TO_HIGHEST_LEVEL(CurrentPriorityMask);
}

/*
 * NAME
 *  get_current_task
 *
 * FUNCTION
 * Gets the Id for the currently-running task
 */
taskid get_current_task(void)
{
    if (current_id != NULL)
    {
        return (*current_id) & 0xfffff;
    }
    else
    {
        return NO_TASK;
    }
}


#ifdef UNIT_TEST_BUILD
void pl_verify_queue(qid queue_id, bool is_empty)
{
    TASK *pTask;
#ifdef SCHED_MULTIQ_SUPPORT
    uint16f queue_index = QID_TO_QINDEX(queue_id);
#endif
    taskid id =  QID_TO_TASKID(queue_id);
    uint16f priority_index = GET_TASK_PRIORITY(queue_id);

    /* Check for valid priority and there is a task in priority*/
    assert(priority_index < NUM_PRIORITIES
           && NULL != tasks_in_priority[priority_index].first);

    /* Find task within the list */
    LOCK_TASK_LIST(priority_index);
    for (pTask = (TASK*)tasks_in_priority[priority_index].first;
         (pTask) && (id != pTask->id); pTask = pTask->next);
    assert(NULL != pTask);

#ifdef SCHED_MULTIQ_SUPPORT
    assert(queue_index<pTask->nqueues);
    if (is_empty)
    {
        assert(NULL==pTask->mqueues[queue_index].first);
    }
    else
    {
        assert(NULL!=pTask->mqueues[queue_index].first);
    }
#else
    if (is_empty)
    {
        assert(NULL==pTask->mqueue.first);
    }
    else
    {
        assert(NULL!=pTask->mqueue.first);
    }
#endif /* SCHED_MULTIQ_SUPPORT */
    UNLOCK_TASK_LIST(priority_index);
}
#endif /* UNIT_TEST_BUILD */

bool sched_find_task(taskid task_id, TASK **the_task)
{
    TASK **tposition, *t;
    uint16f priority = GET_TASK_PRIORITY(task_id);
    LOCK_TASK_LIST(priority);
    for (tposition = &tasks_in_priority[priority].first;
            (t = *tposition) != NULL; tposition = &(*tposition)->next)
    {
        if (t->id == task_id)
        {
            if (the_task)
            {
                *the_task = t;
            }
            UNLOCK_TASK_LIST(priority);
            return TRUE;
        }
    }
    UNLOCK_TASK_LIST(priority);
    return FALSE;
}

/*
 * sched_find_bg_int   - This function is usually used to get a BGINT structure
 * for passing to the streamlined raise_bg_int_with_bgint routine.s
 */
bool sched_find_bgint(taskid task_id, BGINT_TASK *bgint)
{
    BGINT **bposition, *b, **the_bgint = (BGINT **)bgint;
    uint16f priority = GET_TASK_PRIORITY(task_id);

    /* It is legal to pass a task's taskid on the understanding that there is a
     * directly corresponding bg int.  Hence we ensure the BG_INT_FLAG_BIT is
     * set. */
    task_id |= BG_INT_FLAG_BIT;

    LOCK_BGINT_LIST(priority);
    for (bposition = &bg_ints_in_priority[priority].first;
            (b = *bposition) != NULL; bposition = &(*bposition)->next)
    {
        if (b->id == task_id)
        {
            if (the_bgint)
            {
                *the_bgint = b;
            }
            UNLOCK_BGINT_LIST(priority);
            return TRUE;
        }
    }
    UNLOCK_BGINT_LIST(priority);
    return FALSE;
}

/*
 * sched_clear_message_cache
 * Can be called on any processor,
 * but useful when using leak finder on aux (secondary)
 * processorto not report these memories.
 */
void sched_clear_message_cache(void)
{
    block_interrupts();

    if (pCachedMessage != (MSG *) NULL)
    {
        pfree((void *) pCachedMessage);
        pCachedMessage = NULL;
    }

    unblock_interrupts();
}

