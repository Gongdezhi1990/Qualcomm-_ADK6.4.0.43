/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
Copyright (c) 2008 - 2016 Qualcomm Technologies International, Ltd.
  
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
#if !defined(DORM_MODULE_PRESENT) && !defined(DISABLE_SHALLOW_SLEEP)
extern void enter_shallow_sleep(void);
#endif

/* Make all the task_id_<task name>_queues enums visible in the DWARF */
/*lint -e750 */
#define SCHED_TASK_START_EXPAND_PRESERVE_QUEUE_DEBUG(tsk, init_fn, tsk_fn, lvl) \
    SCHED_TASK_START_PRIORITY_EXPAND_PRESERVE_QUEUE_DEBUG\
                                (tsk, init_fn, tsk_fn, lvl, DEFAULT_PRIORITY)

#define SCHED_TASK_START_PRIORITY_EXPAND_PRESERVE_QUEUE_DEBUG(tsk, init_fn, tsk_fn, lvl, pri)\
        PRESERVE_ENUM_FOR_DEBUGGING(task_id_##tsk##_queues)

#define SCHED_TASK_QUEUE_EXPAND_PRESERVE_QUEUE_DEBUG(tsk, q)

/** No end-of-per-queue handling */
#define SCHED_TASK_END_EXPAND_PRESERVE_QUEUE_DEBUG(tsk)

SCHED_TASK_LIST(PRESERVE_QUEUE_DEBUG)

PRESERVE_ENUM_FOR_DEBUGGING(bg_int_ids_enum)

/*
 * Count how many tasks there are
 */
#define SCHED_TASK_START_EXPAND_COUNT(task, init_fn, tsk_fn, lvl) \
    SCHED_TASK_START_PRIORITY_EXPAND_COUNT(task, init_fn, tsk_fn, lvl, \
                                           DEFAULT_PRIORITY)
/*lint --e{973} */
#define SCHED_TASK_START_PRIORITY_EXPAND_COUNT(task, init_fn, tsk_fn, lvl, pri) \
    + 1
#define SCHED_TASK_QUEUE_EXPAND_COUNT(tsk, q)
#define SCHED_TASK_END_EXPAND_COUNT(tsk)

const uint16 N_TASKS = (0 SCHED_TASK_LIST(COUNT));

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
 * Maximum possible task index within a priority level.  We split the number
 * space between P0 and P1
 */
#define MAX_TASK_INDEX    TASK_ID_END_LOCAL

/**
 * \brief Macro to find the priority level of the highest priority task
 * from a priority mask.
 *
 * Assumes Mask is an unsigned int and is greater than 0.
 */
#define MAP_PRIORITY_MASK_TO_HIGHEST_LEVEL(Mask) BIT_GET_TOP_SET_POS(Mask)

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
#define SCHED_TASK_START_PRIORITY_EXPAND_ARRAYS(tsk, init_fn, tsk_fn, lvl, pri)\
    static MSGQ queue_ ## tsk [task_id_ ## tsk ## _n_queues];
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
/** No per-queue handling */
#define SCHED_TASK_QUEUE_EXPAND_TABLE(tsk, q)
/** No end-of-per-queue handling */
#define SCHED_TASK_END_EXPAND_TABLE(tsk)

TASK tasks [] =
{
    SCHED_TASK_LIST(TABLE)
};

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
 #define BG_INT_PRIORITY_EXPAND_WRAPPER(a, b, p)         \
 static void b ## _wrap(void **ppriv)                    \
 {                                                       \
     uint16f raised;                                        \
     (void)get_highest_bg_int(BG_INT_ID_AS_UINT(a), &raised);    \
     UNUSED(ppriv);                                      \
     UNUSED(raised);                                     \
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

/****************************************************************************
Private Type Declarations
*/

/****************************************************************************
Private Constant Declarations
*/

/****************************************************************************
Private Variable Definitions
*/

/** Useful to have these "logically private" variables global, to allow 
  * debug-visibility at wider scope */

/**
 * The identifier of the current task or bg_int.  Bit 22 indicates which this
 * is; we use this to check that \c get_message / \c get_highest_bg_int are
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
 * next call to put_message(). Avoids repeated calls to malloc and free as
 * messages are created and released
 */
static MSG *pCachedMessage = (MSG *) NULL;

TASKQ tasks_in_priority[NUM_PRIORITIES];

BG_INTQ bg_ints_in_priority[NUM_PRIORITIES];

int SchedInterruptActive = 0;

#ifndef SCHEDULER_WITHOUT_RUNLEVELS
/** Flags used internally within the scheduler*/
static struct {
    /** Run level at which scheduler is currently operating - initialised to
     * illegal value (in init_sched) to simplify initialisation routine */
    uint16f current_runlevel;
} sched_flags;
#endif

/** A flag to allow us to exit a runlevel. Set on calling \c sched() */
static bool run = FALSE;

/** A flag to indicate whether we have any task housekeeping to do, such as
 * deletion of tasks that have been marked as ready to be deleted. This
 * housekeeping will be done only when the scheduler has not been called
 * reentrantly but will be checked at least once per scheduler loop at that
 * level.
 */
static volatile bool housekeeping = FALSE;

/****************************************************************************
Private Function Prototypes
*/
static void check_context_switch(uint16f inputPriorityMask);

/****************************************************************************
Private Function Definitions
*/

/**
 * Find the record for a background interrupt.
 *
 * @param bgint_id The ID of the background interrupt to find
 *
 * \return A pointer to the BGINT structure for the background interrupt.
 */
/* The similar sched_find_task() is in the public function section. */
static BGINT *sched_find_bgint(taskid bgint_id)
{
    BGINT *b;
    uint16f priority;
    const tskid index = QID_TO_TSKID(bgint_id);

    /*
     * If this is a static bgint then the bottom bits of its ID is the index
     * into the bg_ints[] array. The same bottom bits may appear at multiple
     * priorities. So, let's look at that slot in the array and read back the
     * full ID. If we have a match we've avoided a full search.
     *
     * Note that our caller was relying on us to validate the priority so that
     * it could safely do a lookup into bg_ints_in_priority[]. If it's a
     * static bg_int then we did this implicitly by checking it was one of
     * the bg_ints we defined at compile time.
     */
    if (index < N_BG_INTS)
    {
        b = &bg_ints[index];
        if (b->id == bgint_id)
            return b;
    }

    /* Dynamic task: we'll have to brute force it. */

    priority = GET_TASK_PRIORITY(bgint_id);

    if (priority >= NUM_PRIORITIES)
    {
        return NULL;
    }

    for (b = bg_ints_in_priority[priority].first; b != NULL; b = b->next)
    {
        if (bgint_id == b->id)
        {
            return b;
        }
    }

    return NULL;
}

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
    while (0 != p_bgint->raised)
    {
        if (p_bgint->raised&1)
        {
            /* If we've found a message, sched_n_messages should be > 0 */
            if ((TotalNumMessages == 0) ||
                (bg_ints_in_priority[priority_index].num_raised <= 0))
            {
                panic(PANIC_OXYGOS_INVALID_MESSAGE_COUNT);
            }
            TotalNumMessages--;
            background_work_pending = (TotalNumMessages != 0);
            bg_ints_in_priority[priority_index].num_raised--;
        }
        /* Keep right shifting */
        p_bgint->raised >>= 1;
    } /* until all bg interrupts are cleared */

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
    uint16f queue_index;
    uint16f priority_index = GET_TASK_PRIORITY(p_task->id);

    /* Go through each message queue and remove the messages */
    for (queue_index = 0; queue_index < p_task->nqueues; queue_index++)
    {
        MSGQ *pQueue = &p_task->mqueues[queue_index];
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
            background_work_pending = (TotalNumMessages != 0);
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

            /*
             * We could put this message into pCachedMessage instead of freeing
             * it but deleting tasks is so rare it's a waste of code space.
             */
            pfree(pMessage);

        } /* as long as there is a message in the queue */
    } /* loop through queues */
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

        if (HighestPriorityLevel < TIMED_EVENT_PRIORITY)
        {
            if (tasks_in_priority[TIMED_EVENT_PRIORITY].num_msgs == 0 &&
                bg_ints_in_priority[TIMED_EVENT_PRIORITY].num_raised == 0)
            {
                /* If the TIMED_EVENT_PRIORITY bit was only set
                 * because of timed events and nothing else came in at that
                 * priority while they were running, clear the bit again */
                CurrentPriorityMask &= ~(1 << TIMED_EVENT_PRIORITY);
            }
            else
            {
                /* There is pending higher priority BG int / task,
                 * do context switch. */
                check_context_switch(GET_PRIORITY_MASK(HighestPriorityLevel));
            }
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

    background_work_pending = (TotalNumMessages != 0);

    unblock_interrupts();
}
#else /* SCHED_NO_TIMER_PREEMPTION */
#define scheduler_service_expired_events(HighestPriorityLevel)\
        timers_service_expired_casual_events()

#define scheduler_service_expired_events_after_sleep() \
        timers_service_expired_casual_events()
#endif /* SCHED_NO_TIMER_PREEMPTION */


/**
 * \brief Strict timed event handler used to wake up the background when it
 * goes to sleep or starts a low-priority task with future casual timed events
 * to be attended to.
 */
#ifdef SCHED_NO_TIMER_PREEMPTION
void sched_wakeup_from_timers(void)
{
    /* Set the CurrentPriorityMask to enable servicing of timed events.  Block
     * higher-level interrupts for the read-modify-write. */
    block_interrupts();
    CurrentPriorityMask |= (1 << TIMED_EVENT_PRIORITY);
    background_work_pending = TRUE;
    dorm_wake();
    unblock_interrupts();
}
#endif /* SCHED_NO_TIMER_PREEMPTION */

/**
 * Physically delete bg ints that have been marked for deletion.
 * \param priority The priority level of the queue to prune
 *
 * \note This function must be called with interrupts blocked.
 */
static void prune_bg_ints(uint16f priority)
{
    BGINT *b, **bposition;

    /*Prevent unchecked array overflow*/
    assert(priority < NUM_PRIORITIES);

    for (bposition = &bg_ints_in_priority[priority].first;
         (b = *bposition) != NULL;
        )
    {
        if (b->prunable)
        {
            flush_bg_ints(b);
            *bposition = b->next;
            PL_PRINT_P1(TR_PL_SCHED_TASK_DELETE,
                        "Bg int 0x%06x deleted\n", b->id);
            pfree(b);
        }
        else
        {
            bposition = &b->next;
        }
    }
}

/**
 * Physically delete tasks that have been marked for deletion.
 * \param priority The priority level of the queue to prune
 *
 * \note This function is must be called with interrupts blocked.
 */
static void prune_tasks(uint16f priority)
{
    TASK *t, **tposition;

    /*Prevent unchecked array overflow*/
    assert(priority < NUM_PRIORITIES);

    for (tposition = &tasks_in_priority[priority].first;
         (t = *tposition) != NULL;
        )
    {
        if (t->prunable)
        {
            flush_task_messages(t);
            pfree(t->mqueues);
            *tposition = t->next;
            PL_PRINT_P1(TR_PL_SCHED_TASK_DELETE,
                        "Task 0x%06x deleted\n", t->id);
            pfree(t);
        }
        else
        {
            tposition = &t->next;
        }
    }
}

static void do_housekeeping(void)
{
    uint16f n;

    /*
     * No need to block interrupts. If someone set this to true after
     * the previous test but before this set then going to find all the
     * work they wanted us to do.
     */
    housekeeping = FALSE;

    /*
     * Deleting tasks and background interrupts is rare, so the cost of looping
     * over all tasks and background interrupts whenever something happens
     * is small.
     *
     * Since we're now at the lowest priority in the system, we know that no
     * one will be sending messages to or setting background intrerupts on
     * the things we're about to delete as we can't be interrupting them.
     *
     * At the moment, we block interrupts over this whole activity. That's
     * not a great strategy. We realy need to push the interrupt blocking
     * into the individual prune functions so we can limits its scope.
     */
    for (n = 0; n < NUM_PRIORITIES; ++n)
    {
        block_interrupts();
        prune_bg_ints(n);
        prune_tasks(n);
        unblock_interrupts();
    }
}

static void service_bg_int(BGINT *b, uint16f HighestPriorityLevel)
{
    if (0 != b->raised)
    {
        if (b->prunable)
        {
            /*
             * Since we're considering calling the background interrupts
             * for this task, this must be a safe time to clear any
             * background interrupts that were pending for this now deleted
             * task.
             */
            flush_bg_ints(b);
            return;
        }

        if (NULL == b->handler)
        {
            panic(PANIC_OXYGOS_NULL_HANDLER);
        }

        /* Switch to the bg_int */
        current_id = &b->id;

        /* Need to write the appropriate thing for dynamic BG ints */
        PL_PRINT_P1(TR_PL_SCHED_TASK_RUN,
                    "Running bg int 0x%06x\n", b->id);
        /* Unlock IRQs and call the handler function for this task */
        unblock_interrupts();

        b->handler(b->ppriv);

        block_interrupts();

        PL_PRINT_P1(TR_PL_SCHED_TASK_RUN,
                    "Bg int 0x%06x completed\n", b->id);

        /* check for any casual timer expiry before checking any more
         * messages and background interrupts. Timers have highest
         * priority.
         */
        scheduler_service_expired_events(HighestPriorityLevel);
    }
}

static bool service_bg_ints(BG_INTQ *bg_ints_queue,
                            uint16f HighestPriorityLevel)
{
    BGINT *b;

    /* Run through all bg ints of this priority.  If any are raised,
     * run their handlers */
    if(0 == bg_ints_queue->num_raised)
    {
        return FALSE;
    }

    /* If we have a link to the first message, use that rather than
     * searching the queue. */
    if(bg_ints_queue->first_message)
    {
        b = bg_ints_queue->first_message;
        bg_ints_queue->first_message = NULL;
        service_bg_int(b, HighestPriorityLevel);
        b = b->next;
    }
    else
    {
        b = bg_ints_queue->first;
    }

    for (; b != NULL && bg_ints_queue->num_raised; b = b->next)
    {
        service_bg_int(b, HighestPriorityLevel);
    }

    return TRUE;
}

static void service_task(TASK *t, uint16f HighestPriorityLevel)
{
    if (t->prunable)
    {
        /*
         * Since we're considering processing messages for this task, this
         * must be a safe time to clear any messages that were pending for
         * this now deleted task.
         */
        flush_task_messages(t);
        return;
    }

#ifndef SCHEDULER_WITHOUT_RUNLEVELS
    if (t->runlevel <= sched_flags.current_runlevel)
#endif
    {
        uint16f numQsRemaining;
        MSGQ *q;

        /*
         * If any of the task's queues holds a message then call the
         * task's handler.
         */
        for (numQsRemaining = t->nqueues, q = t->mqueues;
             numQsRemaining != 0; numQsRemaining--, q++)
        {
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

                /* check for any casual timer expiry before checking
                 * any more messages and background interrupts. Timers
                 * have highest priority.
                 */
                scheduler_service_expired_events(HighestPriorityLevel);
                /*
                 * Once we've run the handler for this task once, for
                 * any of its Qs, move on to the next task of this
                 * priority. If there are still any messages left in
                 * any of the Qs for this task we will run the handler
                 * for this task again, after other tasks of this
                 * priority have run
                 */
                break;
            }
        } /* end of for loop round Qs for this task */

        PL_PRINT_P1(TR_PL_SCHED_TASK_RUN,
                    "Task 0x%06x completed\n", t->id);
    }
}

static bool service_tasks(TASKQ *tasks_queue, uint16f HighestPriorityLevel)
{
    TASK *t;

   /*
    * Run through all tasks of this priority. For each task check all
    * its Qs to see if there are any messages. If so, run the task
    * handler
    */
    if(0 == tasks_queue->num_msgs)
    {
        return FALSE;
    }

    /* If we have a link to the first message, use that rather than
     * searching the queue. */
    if(NULL != tasks_queue->first_message)
    {
        t = tasks_queue->first_message;
        tasks_queue->first_message = NULL;
        service_task(t, HighestPriorityLevel);
        t = t->next;
    }
    else
    {
        t = tasks_queue->first;
    }

    for (; t != NULL && tasks_queue->num_msgs; t = t->next)
    {
        service_task(t, HighestPriorityLevel);
    }

    return TRUE;
}

#ifdef SCHED_NO_TIMER_PREEMPTION
/**
 * When running low priority tasks, timer interrupt needs to be configured
 * to trigger handling of pending timed events.
 */
static void update_timed_events_trigger(void)
{
    /* If we're now running a low or lowest priority level and a
     * timed event is pending we need to ensure we break out of the
     * lower priority task at the right time to handle it.  Otherwise the
     * timed event could be delayed for ages, since we plan to use the
     * lower priority levels for particularly slow tasks */
    if (CurrentPriorityMask < (1 << TIMED_EVENT_PRIORITY))
    {
        timers_scheduler_needs_wakeup();
    }
    else
    {
         /* Check and cancel timer if some other interrupt has kicked us
          * into non-low-priority context */
        timers_scheduler_has_awoken();
    }
}
#endif /* SCHED_NO_TIMER_PREEMPTION */

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
 *   It will unblock them if, and only if, it finds anything to do.
 *
 * \note This function must only unblock interrupts if there is work to do and
 * the current priority level for the scheduler has been updated. Otherwise,
 * if an interrupt of the same level occurs, this function will be called with
 * the same input priority which allows the stack depth to grow indefinitely.
 *
 * \param[in] inputPriorityMask bit field with each bit representing  a priority
 * level, LSB representing LOWEST_PRIORITY, etc.
 *
 */
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
        TASKQ *tasks_queue;
        BG_INTQ *bg_ints_queue;
        uint16f HighestPriorityLevel;

        /*
         * Lookup highest priority that is ready to run, and range of tasks to
         * check Qs on. Initially there must be at least one message for this
         * priority level.
         */
        HighestPriorityLevel =
            MAP_PRIORITY_MASK_TO_HIGHEST_LEVEL(CurrentPriorityMask);

        /*Prevent unchecked array overflow*/
        assert(HighestPriorityLevel < NUM_PRIORITIES);

        /* This priority level must correspond to at least one real task or
         * bg int */
        if (NULL == tasks_in_priority[HighestPriorityLevel].first &&
            NULL == bg_ints_in_priority[HighestPriorityLevel].first
#ifdef SCHED_NO_TIMER_PREEMPTION
            /* The one exception is if we've been specially kicked out of a
             * low-level task to service a timed event */
            && HighestPriorityLevel != TIMED_EVENT_PRIORITY
#endif /* SCHED_NO_TIMER_PREEMPTION */
            )
        {
            panic(PANIC_OXYGOS_INVALID_TASK_ID);
        }

#ifdef SCHED_NO_TIMER_PREEMPTION
        /* Check if we're about to enter priority level < TIMED_EVENT_PRIORITY. */
        update_timed_events_trigger();
#endif

        /* check for any casual timer expiry before checking messages and
         * background interrupts. Timers have highest priority.
         *
         * But for CarlOS-orientated code we need to be sure that a timed event
         * isn't going to pre-empt the task it's associated with, so only
         * service timed events if we're at a low enough priority level
         */
        scheduler_service_expired_events(HighestPriorityLevel);

        bg_ints_queue = &bg_ints_in_priority[HighestPriorityLevel];
        tasks_queue = &tasks_in_priority[HighestPriorityLevel];

        /* Loop until there's no bg_ints or tasks to service. */
        for(;;)
        {
            /* Give equal priority to bgints and tasks by alternating between
               them rather than servicing all of one type before the next. */
            bool more = service_bg_ints(bg_ints_queue, HighestPriorityLevel);
            more |= service_tasks(tasks_queue, HighestPriorityLevel);
            /*
             * We don't want task deletion to be locked out completely. The
             * only requirement is that we can't be threaded in scheduler at
             * a lower priority level.
             *
             * There is a risk that if we've got, say, a long running level 0
             * task that would yield periodically and it gets interrupted by
             * a burst of processing at level 2 which also yields fairly
             * often so the watchdog gets kicked, then the housekeeping will
             * be delayed until level 2 finishes completely and level 0
             * continues and yields.
             *
             * This is probably an acceptable restriction.
             */
            if (housekeeping && inputPriorityMask == 0)
            {
                do_housekeeping();
            }
            if(!more)
            {
                break;
            }
        }

        /* We've run all tasks of this priority - clear the bit
         * indicating there are tasks to run */
        CurrentPriorityMask &= (uint16f) (~(((uint16f) 1 << HighestPriorityLevel)));

    }/*End of while loop checking all priorities greater than inputPriorityMask */

#ifdef SCHED_NO_TIMER_PREEMPTION
    /* Check if we're about to return to priority level < TIMED_EVENT_PRIORITY,
     * but not if about to shallow sleep (CurrentPriorityMask == 0). */
    if (CurrentPriorityMask)
    {
        update_timed_events_trigger();
    }
#endif

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
                tasks[n].init(&( tasks[n].priv));

                /* Is there's an associated bg_int, point it at the same memory
                 * area.  This is harmless if the bg_int is not meant to be
                 * associated with the task because standalone bg ints don't have
                 * a notion of a memory area, so must ignore the pointer their
                 * handler is passed. */
                /*lint -e{661,662} Lint doesn't know that N_BG_INTS is the size of
                 * bg_ints. */
                /* coverity[mixed_enums] */
                if (n < N_BG_INTS)
                {
                    bg_ints[n].ppriv = &tasks[n].priv;
                }
            }
        }
    }

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

static uint16f get_context_switch_priority_mask(void)
{
    uint16f priority;

    /* Check if context switch is needed. Set mask so message for task with
     * HIGHER priority than current task will cause context switch */
    if (NULL == current_id)
    {
        /* The scheduler may be idle at the moment, in which case the context
         * switch should NOT happen now and the message should be handled from
         * scheduler context when the main scheduler loop gets to run. Set
         * priority to max priority;
         * NOTE: This will only happen in the non interrupt context if an
         * init message is sent from main() to get the scheduler rolling,
         * for example in host based tests, before calling the plsched();
         */
         priority = NUM_PRIORITIES;
    }
    else
    {
        priority = GET_TASK_PRIORITY(*current_id);

        /* If a timer is currently being serviced the priority should be at least
         * as high as the timer priority, so we don't interrupt servicing expired
         * timers with other expired timers.
         */
        if (timer_being_serviced && (priority < TIMED_EVENT_PRIORITY))
        {
            priority = TIMED_EVENT_PRIORITY;
        }
    }

    /*
     * Set mask so all equal or lower priority bits to current task priority
     * are set, so a HIGHER priority task causes a context switch to happen
     */
    return GET_PRIORITY_MASK(priority);
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
    uint16f queueIndex = QID_TO_QINDEX(queueId);
    uint16f priority_index = GET_TASK_PRIORITY(queueId);
    MSGQ *pQueue;
    TASK *pTask;
    msgid id;

    pTask = sched_find_task(task_id);

    if (NULL == pTask)
    {
        panic_diatribe(PANIC_OXYGOS_INVALID_TASK_ID, queueId);
    }
    if (queueIndex >= pTask->nqueues)
    {
        panic_diatribe(PANIC_OXYGOS_INVALID_QUEUE_NUMBER, queueId);
    }

    /* if the message pointer is null something has gone wrong so panic */
    if (pMessage == (MSG *) NULL)
    {
        panic(PANIC_OXYGOS_SCHED_MSG_IS_NULL);
    }

    pQueue = &(pTask->mqueues[queueIndex]);

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

        /*
         * We could put this message into pCachedMessage instead of freeing
         * it, but posting messages to a task that's been marked for deletion
         * is so rare, it's a waste of code space.
         */
        unblock_interrupts();
        pfree(pMessage);

        /* There is no such thing as cancel message yet. so return MAX_SCHED_ID */
        return MAX_SCHED_ID;
    }
    else
    {
        TASKQ *tasks_queue = &tasks_in_priority[priority_index];

        /* Store the message on the end of the task's message chain.  */
        MSG **mq = &pQueue->first;
        while (*mq != (MSG *) NULL)
        {
            mq = &(*mq)->next;
        }
        *mq = pMessage;

        /* Increment message counts */
        TotalNumMessages++;
        background_work_pending = TRUE;
        dorm_wake();
        /* Clear first_message if we have more than 1 message. This ensures
           tasks are visited in order of ID. num_msgs == 1 is the common case
           so this restriction doesn't have a large performance impact. */
        tasks_queue->first_message = tasks_queue->num_msgs ? NULL : pTask;
        tasks_queue->num_msgs++;

        /* Set the bit representing that a task of this priority is ready to run */
        CurrentPriorityMask |= (unsigned) (1UL << priority_index);
    }
    unblock_interrupts();

    /*
     * If the put_message was not called from an ISR, check for context switch.
     * For messages sent from an IRQ, context switch is checked on exit from IRQ
     */
    if (!SchedInterruptActive)
    {
        uint16f priorityMask;
        block_interrupts();
        priorityMask = get_context_switch_priority_mask();
        check_context_switch(priorityMask);
        unblock_interrupts();
    }

    return id;
}

#ifndef DISABLE_SHALLOW_SLEEP
/*
 * Turn down the clock for "shallow sleep" power saving whilst in
 * idle loop.
 *
 * With COAL-based desktop builds for the CSRA6810x Apps subsystem,
 * sit in a loop waiting for the next itimed event to expire and then service
 * events.  This enables desktop tests to use the itime module with sched_oxygen.
 */
static void sched_sleep(void)
{
#if defined (DESKTOP_TEST_BUILD) && defined (SUBSYSTEM_APPS)

    TIME next_time;
    if (!timers_get_next_event_time(&next_time))
    {
        return;
    }
    while (time_lt(hal_get_time(), next_time));
    block_interrupts();
    timers_service_expired_strict_events();
    unblock_interrupts();

#else /* APPS DESKTOP BUILD */

#ifdef DORM_MODULE_PRESENT
    dorm_sleep_sched();
#else
    enter_shallow_sleep();
#endif /* DORM_MODULE_PRESENT */
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

    /* There must be at least NUM_PRIORITIES bits in an unsigned int so that
     * we can store each priority as one bit. */
    assert(UINT_BIT >= NUM_PRIORITIES);

    block_interrupts();

    /*
     * If current task is NULL, context switch will be called with input
     * priority mask corresponding to maximum priority, which will always be
     * less than current mask and so context switch will never happen from task
     * init functions or from interrupt handlers when scheduler is idle
     */
    current_id = NULL;
    TotalNumMessages = 0;
    background_work_pending = FALSE;
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
        /*
         * When the scheduler is idle, it's a good time to do housekeeping
         * such as deleting tasks that are no longer needed.
         *
         * We can also process any messages.
         *
         * If we have no housekeeping and no messages then we can sleep.
         */
        if (housekeeping)
        {
            /*
             * We can't rely on doing houskeeping only in
             * check_context_switch() as it's possible for a task to be
             * deleted from within an interrupt without posting any mesages.
             */
            do_housekeeping();
        }
        else if (TotalNumMessages != 0)
        {
            /*
             * Check all priority levels, and run any tasks that are ready
             * starting with highest priority task. InputMask is set so tasks
             * of all priorities are run
             */
            block_interrupts();
            check_context_switch(0);
            unblock_interrupts();

        }
        else
        {
            timers_scheduler_needs_wakeup();

#ifndef DISABLE_SHALLOW_SLEEP
            sched_sleep();
#endif /* DISABLE_SHALLOW_SLEEP */

            /* If the code reaches here, we woke up from the shallow sleep.
             * Check and cancel timer if some other interrupt has woken up the
             * background */
            timers_scheduler_has_awoken();

            /* Service expired casual events. */
            scheduler_service_expired_events_after_sleep();

#ifdef DESKTOP_TEST_BUILD
            {
                TIME next_time;

                /* For these builds exit when we have nothing to do. Before
                 * bailing out, check one last time that we really are idle.
                 * In enter_shallow_sleep() we could have handled a timer
                 * expiry which queued up a message or bg_int, or perhaps
                 * scheduled another timer. */
                if (TotalNumMessages == 0 &&
                    !timers_get_next_event_time(&next_time))
                {
                    return;
                }
            }
#endif
        }

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
    MSGQ *msg_queues;

    /* Create some memory for the task. */
    pTask =xzpnew(TASK);
    if (pTask != NULL)
    {
        msg_queues = xzpmalloc(num_of_queues * sizeof(MSGQ));
        if (msg_queues == NULL)
        {
            /* Failed to allocate queues, so free the task struct */
            pfree(pTask);
            pTask = NULL;
        }
        else
        {
            /* Initialise elements of the new task as per input params */
            pTask->init = NULL;
            pTask->handler = handler;
            pTask->flush_msg = flush_msg;
            pTask->priv = task_data;
            pTask->mqueues = msg_queues;
            pTask->nqueues = (unsigned int)num_of_queues;
#ifndef SCHEDULER_WITHOUT_RUNLEVELS
            pTask->runlevel = (sched_flags.current_runlevel == N_RUNLEVELS) ?
                    0 : sched_flags.current_runlevel;
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
        p_bgint->raised = 0;
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
                pfree(p_task->mqueues);
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
    for (index = TASK_ID_START_LOCAL; index <= MAX_TASK_INDEX; index++)
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

    pTask = sched_find_task(task_id);

    if (pTask != NULL)
    {
        *pQueue_id = (qid)(((queue_index&0xFF)<<8)|task_id);
        return queue_index < pTask->nqueues;
    }

    return FALSE;
}

/**
 *  Deletes a scheduler task that was previously created
 *
 */
void delete_task(taskid id)
{
    TASK *pTask;
    BGINT *p_bgint;
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

    /*
     * Go through the task and bg_int in priority lists and find the task
     * and/or bg_int to be deleted.
     *
     * Note that we don't actually delete the task here as lower priority
     * levels might be walking the list. Instead we mark it for deletion and
     * get the housekeeping activity, that always runs at the lowest priority,
     * to do the queue manipulation.
     *
     * Note also that the way we traverse the list is safe if tasks/bg_ints
     * are being added: at worst our traversal will skip over a newly-added
     * element if we are interrupted in the process of retrieving the next
     * pointer. But in this function we're not interested in newly-added
     * elements, so that's OK.
     *
     * Note that we don't need to test to see if the task and bgint ids were
     * set away from NO_TASK earlier as if they weren't then we won't find
     * a record on the list and that's not considered to be an error.
     */

    pTask = sched_find_task(task_id);
    if (pTask != NULL)
    {
        if (!pTask->prunable)
        {
            pTask->prunable = TRUE; /* Atomic */
            PL_PRINT_P1(TR_PL_SCHED_TASK_DELETE,
                        "Task 0x%06x is queued for delete\n",
                        pTask->id);
        }
        else
        {
            PL_PRINT_P1(TR_PL_SCHED_TASK_DELETE,
                        "Task 0x%06x already queued for delete\n",
                        pTask->id);
        }
    }

    p_bgint = sched_find_bgint(bgint_id);
    if (p_bgint != NULL)
    {
        if (!p_bgint->prunable)
        {
            p_bgint->prunable = TRUE; /* Atomic */
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

    housekeeping = TRUE;
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
msgid put_message(qid queueId, uint16 mi, void *mv)
{
    MSG *pMessage;
    msgid msgId;

    patch_fn_shared(pl_msgs);

#ifdef IPC_MODULE_PRESENT
    if (QID_IS_REMOTE(queueId))
    {
        ipc_send_sched(queueId, mi, mv);
        return NO_MSGID;
    }
#endif

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

    msgId = put_allocated_message(queueId, pMessage);

    return (msgId);
}

/**
 * Raises a background interrupt to required task if not already raised and
 * with the record for this background interrupt already found.
 *
 * This is called only from macros which use token pasting to ensure that
 * only valid background interrupts can be called or from raise_bg_int which
 * does all the validation.
 */
static void unsafe_raise_bg_int(taskid task_id, uint16f bgIntStatus,
                                BGINT *p_bg_int)
{
    /*
     * There are no checks on the parameters because the caller is required
     * to have validated them. The caller can validate them in any manner,
     * either at run time or at compile time.
     *
     * There is no check on the total number of messages. Callers can do that
     * if it makes sense.
     */

    const uint16f priority_index = GET_TASK_PRIORITY(task_id);

    block_interrupts();
    /* Only raise bg interrupt if task is not up for deletion 
     * and hasn't already been raised. */
    if (0 == (p_bg_int->raised & bgIntStatus) && !p_bg_int->prunable)
    {
        BG_INTQ *bg_ints_q = &bg_ints_in_priority[priority_index];

        /* The required background interrupt is not set
         * Make sure background interrupt has a handler */
        if (NULL == p_bg_int->handler)
        {
            panic(PANIC_OXYGOS_NULL_HANDLER);
        }
        p_bg_int->raised |= bgIntStatus;

        /* Increment message counts */
        TotalNumMessages++;
        /* Clear first_message if we have more than 1 message. This ensures
           tasks are visited in order of ID. num_msgs == 1 is the common case
           so this restriction doesn't have a large performance impact. */
        bg_ints_q->first_message = bg_ints_q->num_raised ? NULL : p_bg_int;
        bg_ints_q->num_raised++;

        /* Set the bit representing that a task of this priority is ready to
         * run */
        CurrentPriorityMask |= (unsigned) (1UL << priority_index);
    }

    /* Always keep background_work_pending up to date. Code outside of the
     * scheduler may have temporarily disabled it to allow the processor to
     * sleep until an interrupt occurs. */
    background_work_pending = (TotalNumMessages != 0);
    if (background_work_pending)
    {
        dorm_wake();
    }

    unblock_interrupts();
}

/**
 * Raises a background interrupt to required static task if not already raised.
 *
 * This is the same functionality as raise_bg_int() but works only on static
 * tasks and is a little faster as it some tests are effectively being
 * performed at compile time.
 *
 * We may be able to optimise further in future if we can guarantee that
 * static bg_ints only ever use a single interrupt. The auto-generated handlers
 * make this assumption.
 */
void raise_static_bg_int(enum bg_int_indices_enum index)
{
    BGINT * const p_bg_int = &bg_ints[index];

    patch_fn_shared(pl_bgint);

    unsafe_raise_bg_int(p_bg_int->id, 1, p_bg_int);
}

/**
 * Raises a background interrupt to required task if not already raised.
 *
 * This function is usually called within an interrupt handler to signal an
 * event to a background task.
 */
void raise_bg_int(taskid task_id, uint16f bgIntBitPos)
{
    BGINT *p_bg_int;

    /* It is legal to pass a task's taskid on the understanding that there is a
     * directly corresponding bg int.  Hence we ensure the BG_INT_FLAG_BIT is
     * set. */
    task_id |= BG_INT_FLAG_BIT;

    patch_fn_shared(pl_bgint);

    /* Check there arent too many messages */
    if (TotalNumMessages >= MAX_NUM_MESSAGES)
    {
      panic_diatribe(PANIC_OXYGOS_TOO_MANY_MESSAGES, (uint16f)TotalNumMessages);
    }

    /* Check the bit position is valid */
    if (bgIntBitPos >= MAX_NUM_OF_BG_INTS)
    {
        panic_diatribe(PANIC_OXYGOS_INVALID_BACKGROUND_INTERRUPT_BIT_POSITION,
                       bgIntBitPos);
    }

    p_bg_int = sched_find_bgint(task_id);

    if (NULL == p_bg_int)
    {
        panic_diatribe(PANIC_OXYGOS_INVALID_TASK_ID, task_id);
    }

    unsafe_raise_bg_int(task_id, SET_BG_INT_POS(bgIntBitPos), p_bg_int);
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
 * Comparison class to be passed to \c cmp_timed_msg_data by the timer search
 * function
 */
typedef struct
{
    unsigned mi:16;  /**< Message ID */
    unsigned use_mi:1; /**Whether to treat the message ID as significant */
    const void *cmp_data; /**< Arbitrary comparison data to be passed to \c cmp_fn */
    TIMED_MSG_CMP_FN cmp_fn; /**< Arbitrary comparison function for the message
                                  body */
} TIMED_MSG_CMP_DATA;

/**
 * Comparison function for pending scheduler messages on the timer queue.
 * Looks first for a match of message IDs and then calls the comparison function
 * that is passed with the \c cmp_data
 * @param data A \c timed_msg, which is the type stored on the timer queue for
 * timed scheduler messages
 * @param cmp_data A \c TIMED_MSG_CMP_DATA, which is the type passed through to
 * the timer search function by \c cancel_matching_timed_messages
 * @return TRUE if the message IDs match and the comparison function says yes
 * (or is NULL)
 */
static bool cmp_timed_msg_data(const void *data, const void *cmp_data)
{
    const timed_msg *timer_msg = (const timed_msg *)data;
    const TIMED_MSG_CMP_DATA *timed_msg_cmp_data =
                                        (const TIMED_MSG_CMP_DATA *)cmp_data;

    return ((!timed_msg_cmp_data->use_mi ||
                            (timer_msg->mi == timed_msg_cmp_data->mi)) &&
            (timed_msg_cmp_data->cmp_fn == NULL ||
                             timed_msg_cmp_data->cmp_fn(
                                                timer_msg->mv,
                                                timed_msg_cmp_data->cmp_data)));
}

bool cancel_first_matching_timed_message(uint16 mi,
                                         const void *cmp_data,
                                         TIMED_MSG_CMP_FN cmp_fn,
                                         uint16 *pmi,
                                         void **pmv)
{
    TIMED_MSG_CMP_DATA timed_msg_cmp_data;
    void *timer_msg;

    timed_msg_cmp_data.use_mi = (pmi == NULL) ? 1 : 0;
    timed_msg_cmp_data.mi = mi;
    timed_msg_cmp_data.cmp_data = cmp_data;
    timed_msg_cmp_data.cmp_fn = cmp_fn;
    if (timer_cancel_strict_event_by_function_cmp(deliver_timed_message,
                                                  &timed_msg_cmp_data,
                                                  cmp_timed_msg_data,
                                                  &timer_msg))
    {
        if (pmv)
        {
            *pmv = ((timed_msg *)timer_msg)->mv;
        }
        if (pmi)
        {
            *pmi = ((timed_msg *)timer_msg)->mi;
        }
        pfree(timer_msg);
        return TRUE;
    }
    return FALSE;
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
bool get_message(qid queue_id, uint16 *pmi,void **pmv)
{
    MSG *pMessage;
    MSGQ *pQueue;
    taskid task_id = QID_TO_TASKID(queue_id);
    uint16f queueNumber = QID_TO_QINDEX(queue_id);
    uint16f priority_index = GET_TASK_PRIORITY(queue_id);
    TASK *current_task;

    patch_fn_shared(pl_msgs);

    /* Check the call is coming from the currently active task and is for
     * one of its own queues. */
    if (NULL == current_id                                   ||
        task_id != *current_id                               ||
        ID_IS_BG_INT_ID(queue_id))
    {
        panic_diatribe(PANIC_OXYGOS_INVALID_TASK_ID, queue_id);
    }

    /* Obtain the task structure that the current_id belongs to.*/
    current_task = STRUCT_FROM_MEMBER(TASK, id, current_id);

    if (queueNumber >= current_task->nqueues)
    {
        panic_diatribe(PANIC_OXYGOS_INVALID_QUEUE_NUMBER, queue_id);
    }

    /* Look up the message queue pointer for the given qid */
    pQueue = &(current_task->mqueues[queueNumber]);
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
        background_work_pending = (TotalNumMessages != 0);
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
 *
 */
bool get_message_for_current_task(uint16 *pmi,void **pmv)
{
    return get_message(TASKID_TO_Q0_QID(*current_id), pmi, pmv);
}

/**
 * Obtains the highest priority background interrupt for the task if one is
 * raised. The calling task must own this background interrupts.
 *
 * If the calling task does not own then the scheduler asserts.
 * MUST NOT BE CALLED DIRECTLY FROM INTERRUPT CODE - can only be called by the
 * task given by id.
 */
bool get_highest_bg_int(taskid bg_int_id, uint16f *highest_bg_int)
{
    uint16f bgIntStatus;
    uint16f priority_index = GET_TASK_PRIORITY(bg_int_id);
    BGINT *current_bg_int;

    patch_fn_shared(pl_bgint);

    /* Check the call is coming from the currently active task and is for
     * one of its own queues. */
    if (NULL == current_id                                   ||
        !IDS_SAME_OR_COUPLED(bg_int_id, *current_id))
    {
        panic_diatribe(PANIC_OXYGOS_INVALID_TASK_ID, bg_int_id);
    }

    current_bg_int = STRUCT_FROM_MEMBER(BGINT, id, current_id);

    if (NULL == highest_bg_int)
    {
        panic(PANIC_OXYGOS_SCHED_MSG_IS_NULL);
    }

    *highest_bg_int = 0;

    /* Note the background interrupt status */
    bgIntStatus = current_bg_int->raised;

    /* Make sure there is a background interrupt raised */
    if (0 != bgIntStatus)
    {
        /* If we've found an interrupt raised message, sched_n_messages should
         * be > 0 */
        if ((TotalNumMessages == 0)    ||
            (bg_ints_in_priority[priority_index].num_raised <= 0))
        {
            panic(PANIC_OXYGOS_INVALID_MESSAGE_COUNT);
        }

        /* Background interrupt prioritised by the bit position.
         * least significant bit represents highest priority
         * Find the least significant bit that is set */
        bgIntStatus = (bgIntStatus & ~(bgIntStatus-1));

        block_interrupts();

        /* clear the background interrupt and decrement the counts */
        current_bg_int->raised &= ~(bgIntStatus);
        TotalNumMessages--;
        background_work_pending = (TotalNumMessages != 0);
        bg_ints_in_priority[priority_index].num_raised--;

        unblock_interrupts();

        /* right shift to find the bit position */
        while (1 != bgIntStatus)
        {
            bgIntStatus >>= 1;
            (*highest_bg_int)++;
        }
        PL_PRINT_P2(
            TR_PL_GET_MESSAGE,
            "PL Got a interrupt %i for task ID 0x%x\n", *highest_bg_int,
            bg_int_id);
        return(TRUE);
    }

    /* If we get here there was no message */
    PL_PRINT_P1(
        TR_PL_GET_MESSAGE,
        "PL PlGetHighestBgInt called for task ID 0x%x, but no message to get\n",
        bg_int_id);
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
        uint16f priorityMask = get_context_switch_priority_mask();
        check_context_switch(priorityMask);
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

bool sched_is_running(void)
{
    return run;
}

bool sched_in_interrupt(void)
{
    return (SchedInterruptActive != 0);
}

#ifdef UNIT_TEST_BUILD
void pl_verify_queue(qid queue_id, bool is_empty)
{
    TASK *pTask;
    uint16f queue_index = QID_TO_QINDEX(queue_id);
    taskid id =  QID_TO_TASKID(queue_id);
    uint16f priority_index = GET_TASK_PRIORITY(queue_id);

    pTask = sched_find_task(id);

    assert(NULL != pTask);
    assert(queue_index<pTask->nqueues);

    if (is_empty)
    {
        assert(NULL==pTask->mqueues[queue_index].first);
    }
    else
    {
        assert(NULL!=pTask->mqueues[queue_index].first);
    }
}
#endif

/* The similar sched_find_bgint() is in the private function section. */

TASK *sched_find_task(taskid task_id)
{
    TASK *t;
    uint16f priority;
    const tskid index = QID_TO_TSKID(task_id);

    /*
     * If this is a static task then the bottom bits of its ID is the index
     * into the tasks[] array. The same bottom bits may appear at multiple
     * priorities. So, let's look at that slot in the array and read back the
     * full ID. If we have a match we've avoided a full search.
     *
     * Note that our caller was relying on us to validate the priority so that
     * it could safely do a lookup into tasks_in_priority[]. If it's a static
     * task then we did this implicitly by checking it was one of the tasks we
     * defined at compile time.
     */
    if (index > SCHED_TASK_ID_START && index < TASK_ID_LAST)
    {
        t = &tasks[index - (SCHED_TASK_ID_START + 1)];
        if (t->id == task_id)
        {
            return t;
        }
    }

    /* Dynamic task: we'll have to brute force it. */

    priority = GET_TASK_PRIORITY(task_id);

    if (priority >= NUM_PRIORITIES)
    {
        return NULL;
    }

    for (t = tasks_in_priority[priority].first; t != NULL; t = t->next)
    {
        if (t->id == task_id)
        {
            return t;
        }
    }

    return NULL;
}

/**
 * Tell dorm how long we'd like to sleep.
 */
bool sched_get_sleep_deadline(TIME *earliest, TIME *latest)
{
    if (!timers_get_next_event_time(latest))
    {
        return FALSE;
    }

    *earliest = *latest;
    return TRUE;
}
