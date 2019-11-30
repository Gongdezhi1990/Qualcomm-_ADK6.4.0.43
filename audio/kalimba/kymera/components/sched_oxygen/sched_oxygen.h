/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 ************************************************************************//**
 *
 * \defgroup sched_oxygen sched oxygen
 *
 * \file sched_oxygen.h
 * \ingroup sched_oxygen
 *
 * Header file for the kalimba scheduler
 *
 * \section basic Basic concepts
 * The system scheduler gives structure to the bulk of the code in the
 * system. It organises the code as a set of tasks, and provides means
 * for communication between the tasks.
 *
 * Each task has two main elements: a set of message queues and a
 * message handler function. The message queues accumulate messages to
 * be consumed by the message handler. The message handler consumes
 * messages from its queues and acts upon them.
 *
 * Each message has two elements: a uint16 and a void* pointer. Much
 * of the time the uint16 carries some form of signal ID and the
 * void* points to the message body. If a message body is not
 * required, the void * pointer can be NULL. The scheduler does not examine
 * or alter either of these elements of each message.
 *
 * The core of the scheduler inspects each task's set of message queues
 * and calls the task's handler if there is any message in any
 * of the task's queues.
 *
 * A task's message handler can post to any tasks' queues, but it can
 * consume messages only from its own queues.
 *
 * \section preemption Pre-emption
 *
 * So far, the same as CarlOS. However this scheduler also supports
 * priorities for each task, and "simple pre-emption".
 *
 * "Simple pre-emption" means that at any of a defined set of
 * potential context switch points, the scheduler examines if any task(s)
 * of a higher priority than the current task has been made ready to run.
 * If so, a call will be made to the higher priority task(s) before we
 * return to the currrent task.
 *
 * The only "potential context switch points" are currently:
 *  1) In put_message() after the message has been added to the queue
 *  2) On return from an interrupt, to check if the ISR has signalled
 *     a higher priority task.
 *
 * KEY POINT: Every task handler function must run to completion, else
 * no lower priority task will ever run. Note this means no task should
 * block, as if it does no lower priority task will run till the block
 * is released. Hence mutexes and semaphores are not implemented in
 * this scheduler.
 *
 * The main gain of this approach, rather than a more conventional
 * RTOS, is that the system only requires one stack. This should
 * help constrain the RAM consumption of this embedded application.
 *
 * \section task_creation Task and Background interrupt creation
 *
 * This scheduler supports both statically and dynamically created tasks. Static
 * task generation must somehow arrange for two arrays, \c tasks and \c bg_ints
 * to be populated and for corresponding enumerations of task, bg int and queue
 * IDs to be created.  If task/bg int coupling is not required, CarlOS-style
 * autogeneration is available provided the build system provides suitable
 * support.  This requires modules wishing to register tasks and/or background
 * interrupts to define special macros containing the requisite details.
 * Dynamic task generation uses the \c create_task, \c delete_task,
 * \c get_msg_qid_for_task interface to dynamically handle coupled and uncoupled
 * tasks and background interrupts.
 *
 * The scheduler stores a void* pointer for the internal use of each
 * task.   The scheduler makes no use of the pointer except to pass it
 * as an argument to each invocation of the task's message handler (or the
 * bg int handler). This allows the system to maintain data between invocations
 * of the handler rather than in static data.  In coupled cases, the bg int
 * stores a pointer to the task's pointer, meaning not only that the memory
 * is shared but that either can safely change its allocation.
 *
 * Each task can have an initialisation function which is called exactly
 * once before any task's main function is called. The initialisation
 * function is passed the same void* pointer that is passed to the main
 * task function.
 *
 ****************************************************************************/

#ifndef SCHED_OXYGEN_H
#define SCHED_OXYGEN_H

/****************************************************************************
Include Files
*/

#include "pl_timers/pl_timers.h" /* For tTimerId */
#include "hydra/hydra_types.h"
#include "timed_event/timed_event.h"
#include "sched/runlevels.h"
#include "sched_oxygen/sched_subsystem.h"
#include "sched_oxygen/bg_int_subsystem.h"
#include "limits.h"


/****************************************************************************
Public Macro Declarations
*/
#ifdef UNIT_TEST_BUILD
#define verify_queue_is_empty(q) pl_verify_queue(q,TRUE)
#define verify_queue_is_not_empty(q) pl_verify_queue(q,FALSE)
#endif

#define MAX_TASK_ID ((NUM_PRIORITIES<<16)|0xFF)

/**
 * Find priority from given task/queue id
 */
#define GET_TASK_PRIORITY(x)  ((uint16f)((x)>>16&0x1F))

/**
 * Minimum and maximum values for taskids
 */
#define MIN_TASKID ((taskid) 0)
#define MAX_TASKID ((taskid)0xFFFFFFU)

/**
 * Special value of taskid used to indicate no task
 */
#define NO_TASK ((taskid) MAX_TASKID)

/**
 * Special macro for determining if code execution is taking place at interrupt.
 * This is provided to facilitate optimisations which may be possible if the
 * code can't be interrupted in the current context.
 */
#define is_current_context_interrupt() (SchedInterruptActive != 0)

/****************************************************************************
Public Type Declarations
*/

/**
 * \name Basic types and macros
 */
/*@{*/

/** An identifier issued by the scheduler. */
typedef uint24   scheduler_identifier;

/** Type for storing the task identifier, consisting of the combined tskid
  * (bits 0-7), priority (bits 16-20), coupled flag (bit 21) and bg_int flag
  * (bit 22). */
typedef uint24   taskid;

/** An Opaque pointer type for exposing a BGINT structure externally */
typedef void* BGINT_TASK;


/** Line length reducer. */
#define schid   scheduler_identifier
#define NO_SCHID  ((schid)(0))

/** A queue identifier. */
typedef uint24   qid;

/**
 * A message identifier.
 */
typedef schid msgid;

/** Active msgids are non-zero, so zero signals "not a message id". */
#define NO_MSGID  ((msgid)NO_TID)

/** Time value indicating sched doesn't care, make up your own value. */
#define SCHED_MAX_DELAY  ((TIME)(10 * (MINUTE)))

/** Make a queue identifier (qid) from its components.   This disgusting macro
 * must be visible for public ridicule, even though its value is of interest
 * only to the scheduler's innards.  t is a taskid and must be in the range
 * 0->MAX_ULONG-1.  q must be in the range 1->256. */
#define mkqid(t, q)  (qid)(((((qid)q)-1)<<8) + (t))

/** Use queue 128 of task 128 to indicate 'no queue' -
 * it is highly unlikely we will ever have 128 tasks or 128 queues.
 */
#define NO_QID   (mkqid(128,128))

/**
 * tRoutingInfo - Message routing details
 */

typedef struct
{
    unsigned int src_id;  /**< Source ID for routing. */
    unsigned int dest_id;  /**< Destination ID for routing. */
} tRoutingInfo;

/**
 * MSG - A message
 */
typedef struct msg_tag
{
    struct msg_tag *next; /**< Next in linked list. */
    uint16 mi; /**< The message's uint16. */
    void *mv; /**< The message's void*. */
    msgid id; /**< The message's identifier. */
    tRoutingInfo routing;
} MSG;

/**
 * MSGQ - A message queue.
 */
typedef struct
{
    MSG *first; /**< Pointer to the first message in the Q. Set to NULL if Q is
                 empty */
} MSGQ;


/*@}*/

/**
 * \name Additional OxygOS types.  Ought to be renamed in Hydra sched style
 */
/*@{*/
/**
 * Function type to flush a message received for a task. This is handler will be
 * provided when the task is created.
 */
typedef void (*MSG_FLUSH_HANDLER)(unsigned int msg_int, void* pmsg_body);

/**
 * PRIORITY - A priority level used by the scheduler.
 *
 * Used to decide whether a put_message call from one task to another should
 * interrupt the calling task.
 * NOTE - before adding priority levels, ensure there are at least
 * NUM_PRIORITIES bits in CurrentPriorityMask
 */
typedef enum
{
    LOWEST_PRIORITY = 0,
    LOW_PRIORITY,
    MID_PRIORITY,
    HIGH_PRIORITY,
    HIGHEST_PRIORITY,
    NUM_PRIORITIES
} PRIORITY;

/**
 * Tasks inherited from CarlOS have no notion of priority, so we define a
 * default priority that they will run at if their definition is not altered.
 */
#define DEFAULT_PRIORITY LOWEST_PRIORITY

/*@}*/

/**
 * \name Autogeneration trickery to create tasks and queues.
 */
/*@{*/

/** Trick to allow multiple use of macro. */
#define SCHED_TASK_START(m, n)                  \
    SCHED_TASK_START_EXPAND_ ## m n

#define SCHED_TASK_START_PRIORITY(m, n)         \
    SCHED_TASK_START_PRIORITY_EXPAND_ ## m n

/** Trick to allow multiple use of macro. */
#define SCHED_TASK_QUEUE(m, n)                  \
    SCHED_TASK_QUEUE_EXPAND_ ## m n
/** Trick to allow multiple use of macro. */
#define SCHED_TASK_END(m, n)                    \
    SCHED_TASK_END_EXPAND_ ## m n

/** Create prototypes for the init and task functions for each subsystem
 * e.g. extern void test_task(void ** task_data);
 *      extern void init_test_task(void ** task_data); */
/* Default-priority version */
#define SCHED_TASK_START_EXPAND_PROTO(tsk, init_fn, tsk_fn, lvl) \
    SCHED_TASK_START_PRIORITY_EXPAND_PROTO(tsk, init_fn, tsk_fn, lvl, \
                                           DEFAULT_PRIORITY)

#define SCHED_TASK_START_PRIORITY_EXPAND_PROTO(tsk, init_fn, tsk_fn, lvl, pri) \
    extern void init_fn(void ** task_data);                           \
    extern void tsk_fn(void ** task_data);
#define SCHED_TASK_QUEUE_EXPAND_PROTO(tsk, q)
#define SCHED_TASK_END_EXPAND_PROTO(tsk)

SCHED_TASK_LIST(PROTO)

/**
 * Create enum of tasks themselves
 */
#define SCHED_TASK_START_EXPAND_ID(tsk, init_fn, tsk_fn, lvl) \
    SCHED_TASK_START_PRIORITY_EXPAND_ID(tsk, init_fn, tsk_fn, lvl, \
                                        DEFAULT_PRIORITY)
#define SCHED_TASK_START_PRIORITY_EXPAND_ID(tsk, init_fn, tsk_fn, lvl, pri) \
    task_id_ ## tsk,
#define SCHED_TASK_QUEUE_EXPAND_ID(tsk, q)
#define SCHED_TASK_END_EXPAND_ID(tsk)

/**
 * The task IDs.
 */
enum task_id_t
{
    SCHED_TASK_LIST(ID)
    N_TASKS
};

/**
 * Create enum of tasks with their priorities encoded in
 */
#define SCHED_TASK_START_EXPAND_PRIORITY_ID(tsk, init_fn, tsk_fn, lvl) \
    SCHED_TASK_START_PRIORITY_EXPAND_PRIORITY_ID(tsk, init_fn, tsk_fn, lvl, \
                                                 DEFAULT_PRIORITY)
#define SCHED_TASK_START_PRIORITY_EXPAND_PRIORITY_ID(tsk, init_fn, tsk_fn, lvl,\
                                                     pri)\
    priority_task_id_ ## tsk = task_id_ ## tsk + (pri << 16),
#define SCHED_TASK_QUEUE_EXPAND_PRIORITY_ID(tsk, q)
#define SCHED_TASK_END_EXPAND_PRIORITY_ID(tsk)

/**
 * The priority-encoded task IDs.
 */
enum priority_task_id_t
{
    /*lint --e{726}*/SCHED_TASK_LIST(PRIORITY_ID)
    NO_PRIORITY_TASK_ID = 0
};

/**
 * Create an enum of tasks based on the task end macros.
 * This is just to ensure that the names are unique; we don't use the result.
 */
#define SCHED_TASK_START_EXPAND_END_ID(tsk, init_fn, tsk_fn, lvl)
#define SCHED_TASK_START_PRIORITY_EXPAND_END_ID(tsk, init_fn, tsk_fn, lvl, pri)
#define SCHED_TASK_QUEUE_EXPAND_END_ID(tsk, q)
#define SCHED_TASK_END_EXPAND_END_ID(tsk) task_end_id_ ## tsk,

enum task_end_id_t
{
    SCHED_TASK_LIST(END_ID)
    N_ALL_END_TASKS
};

/**
 * Create enums of IDs for queues, one enum per task.
 * This is not the value used in the code --- that's the queue
 * itself --- so disguise the name.
 *
 * For historical reasons, the queue IDs are 1-based.  Rather than
 * create confusion by changing this, we'll put in a dummy
 * zero entry here.  To count the queues, we'll first get the
 * number plus one for free, then define the actual number as
 * that value minus one.  We do that in a later enum because
 * otherwise lint whines on about MISRA rules.
 */
#define SCHED_TASK_START_EXPAND_QUEUE_IDS(tsk, init_fn, tsk_fn, lvl) \
    SCHED_TASK_START_PRIORITY_EXPAND_QUEUE_IDS(tsk, init_fn, tsk_fn, lvl, \
                                               DEFAULT_PRIORITY)
#define SCHED_TASK_START_PRIORITY_EXPAND_QUEUE_IDS(tsk, init_fn, tsk_fn, lvl, \
                                                   pri) \
    enum task_id_ ## tsk ## _queue_id_with_tasks {                   \
        task_id_ ## tsk ## _queue_zero_entry,
#define SCHED_TASK_QUEUE_EXPAND_QUEUE_IDS(tsk, q)       \
        task_queue_id_within_task_ ## q,
#define SCHED_TASK_END_EXPAND_QUEUE_IDS(tsk)                  \
        task_id_ ## tsk ## _n_queue_ids_plus_one              \
    };
SCHED_TASK_LIST(QUEUE_IDS)

/**
 * This is the aforementioned enum to stop lint whining.
 * It doesn't like enums that have assignments to be mixed
 * with those that don't.  Personally, I don't care.
 */
#define SCHED_TASK_START_EXPAND_QUEUE_COUNT(tsk, init_fn, tsk_fn, lvl) \
    SCHED_TASK_START_PRIORITY_EXPAND_QUEUE_COUNT(tsk, init_fn, tsk_fn, lvl, \
                                                 DEFAULT_PRIORITY)
#define SCHED_TASK_START_PRIORITY_EXPAND_QUEUE_COUNT(tsk, init_fn, tsk_fn, lvl,\
                                                     pri) \
    enum task_id_ ## tsk ## _queue_id_for_count {                     \
        task_id_ ## tsk ## _n_queues =                                \
            task_id_ ## tsk ## _n_queue_ids_plus_one - 1              \
    };
#define SCHED_TASK_QUEUE_EXPAND_QUEUE_COUNT(tsk, q)
#define SCHED_TASK_END_EXPAND_QUEUE_COUNT(tsk)
SCHED_TASK_LIST(QUEUE_COUNT)

/**
 * Create enums defining the queues themselves.
 * This uses the previous set of enums to get the numbers within
 * the tasks, then this number and the task ID to get the
 * actual qid, which we assign to q, which is supposed to be globally
 * unique (we'll get an error if there's a duplicate, which is a
 * feature).   I hope you're paying attention.
 */
#define SCHED_TASK_START_EXPAND_QUEUES(tsk, init_fn, tsk_fn, lvl) \
    SCHED_TASK_START_PRIORITY_EXPAND_QUEUES(tsk, init_fn, tsk_fn, lvl, \
                                            DEFAULT_PRIORITY)
#define SCHED_TASK_START_PRIORITY_EXPAND_QUEUES(tsk, init_fn, tsk_fn, lvl, \
                                                pri) \
    enum task_id_ ## tsk ## _queues {
#define SCHED_TASK_QUEUE_EXPAND_QUEUES(tsk, q)  \
    q = mkqid(priority_task_id_ ## tsk, task_queue_id_within_task_ ## q),
#define SCHED_TASK_END_EXPAND_QUEUES(tsk) \
    task_id_ ## tsk ## _invalid_qid = NO_QID \
    };
SCHED_TASK_LIST(QUEUES)

/*@}*/

/**
 * \name Autogeneration trickery to create bg_ints.
 */
/*@{*/

#define BG_INT(m, n)                            \
    BG_INT_EXPAND_ ## m n
#define BG_INT_PRIORITY(m, n)                   \
    BG_INT_PRIORITY_EXPAND_ ## m n


/** Indices for bg_ints.  Values index into bg_ints[]. */
#define BG_INT_EXPAND_INDEX(a, b) \
    BG_INT_PRIORITY_EXPAND_INDEX(a, b, DEFAULT_PRIORITY)
#define BG_INT_PRIORITY_EXPAND_INDEX(a, b, p) a ## _bg_int_index,

enum bg_int_indices_enum {
    BG_INT_LIST(INDEX)
    N_BG_INTS
};

/** BG int IDs are distinguished from task IDs by having the uppermost bit of
 * the third octet set (meaning that the priority flag is reduced to 7 bits,
 * but that's more than enough).
 */
#define BG_INT_FLAG_BIT (1U << 22)
#define ID_IS_BG_INT_ID(id) ((id) & BG_INT_FLAG_BIT)

/**
 * For convenience we record whether a task/bg int is coupled with a bg int/
 * task by setting this bit
 */
#define BG_INT_TASK_IS_COUPLED_FLAG_BIT (1 << 21)
#define IS_COUPLED(id) ((id) & BG_INT_TASK_IS_COUPLED_FLAG_BIT)

/** IDs for bg_ints.  Values are identical to indices in LSO.  We'll need to
 * provide a way of specifying priority levels directly, though */
#define BG_INT_EXPAND_ID(a, b) \
    BG_INT_PRIORITY_EXPAND_ID(a, b, DEFAULT_PRIORITY)
#define BG_INT_PRIORITY_EXPAND_ID(a, b, p) a ## _bg_int_id = \
                                                           a ## _bg_int_index \
                                                        | (((p) & 0x1f) << 16) \
                                                        | BG_INT_FLAG_BIT,

typedef enum bg_int_ids_enum {
    /*lint --e{726}*/BG_INT_LIST(ID)
    NO_BG_INT_ID = 0
} bg_int_ids;


/** Function prototypes */
#define BG_INT_EXPAND_PROTO(a, b) \
    BG_INT_PRIORITY_EXPAND_PROTO(a, b, DEFAULT_PRIORITY)
#define BG_INT_PRIORITY_EXPAND_PROTO(a, b, p) extern void b(void);

BG_INT_LIST(PROTO)

/** return numerical bg_int id */
#define BG_INT_ID(a) (a ## _bg_int_id)
/** return the bg_int id as a uint24 (otherwise it is considered signed, which
 * causes a problem in 24-bit builds as theoretically bit 23 could be set,
 * although we've now set things up to avoid ever setting bit 23.) */
#define BG_INT_ID_AS_UINT(a) ((uint24)BG_INT_ID(a))

/**
 * Generate a background interrupt from an ID.
 *
 * On the host this needs to be atomic.
 * Wake dorm if sleeping.
 */
#ifdef __KALIMBA__
#define GEN_BG_INT_FROM_ID(x)                   \
    do {                                        \
        raise_bg_int((taskid) (x));          \
    } while (0)
#else /* __KALIMBA__ */
#define GEN_BG_INT_FROM_ID(x)                   \
    do {                                        \
        raise_bg_int((taskid) (x));          \
        dorm_wake();                            \
    } while (0)
#endif /* __KALIMBA__ */

/**
 * Generate a background interrupt.
 *
 * Standard way of calling the above.
 */
#define GEN_BG_INT(x)                           \
    GEN_BG_INT_FROM_ID(x ## _bg_int_id)

/*@}*/

/****************************************************************************
Global Variable Definitions
*/

/**
 * Set by interrupt handler to indicate IRQ is active. Used by scheduler to
 * disable context switch when put_message is called from within IRQ
 */
extern int SchedInterruptActive;

/****************************************************************************
Public Function Prototypes
*/

/**
 * \brief initialise the scheduler
 */
extern void init_sched( void );

/**
 * This function terminates the current runlevel of sched
 * Terimantion occurs next time sched restarts its outer loop
 * No test is made to ensure that all pending messages are consumed,
 * timers expired, bg_ints serviced or such.
 * It is assumed that thjis function will only be called
 * to terminate limited runlevels during boot
 */
extern void terminate_runlevel(void);

#ifdef SCHEDULER_WITHOUT_RUNLEVELS
/**
 * If there are no runlevels, terminating a runlevel stops the scheduler.
 *
 * This is for compatibility with the host code, but it also makes
 * the intention clear.
 */
#define sched_stop() terminate_runlevel()
#endif

/**
 * \brief The main function of the background task scheduler. This invokes
 * tasks as messages become available for them.
 *
 * \return Doesn't return, except for DESKTOP_TEST_BUILD builds
 */
#ifdef SCHEDULER_WITHOUT_RUNLEVELS
extern void sched_norunlevels(void);
#define sched(runlevel) sched_norunlevels()
#else /* SCHEDULER_WITHOUT_RUNLEVELS */
extern void sched(uint16f runlevel);
#endif /* SCHEDULER_WITHOUT_RUNLEVELS */

#ifndef SCHEDULER_WITHOUT_RUNLEVELS
/**
 * Set current run level
 *
 * The scheduler internally holds a "run level" value; this determines
 * the set of tasks that the scheduler services.  Each task also has a
 * defined "run level" value.  The scheduler normally handles tasks
 * with defined run levels up to, and including, the scheduler's run
 * level value.
 *
 * The set_runlevel() call increases the scheduler's run level value
 * to "level".
 *
 * set_runlevel() first runs any initialisation functions for tasks with
 * defined run levels between one more than the scheduler's existing
 * run level and "level".
 *
 * When set_runlevel() returns the scheduler performs its normal
 * operations for all tasks with defined run levels upto, and
 * including, "level".
 *
 * This function must not be called from any task's initialisation
 * function, nor can it be called until the scheduler has been started.
 *
 * This function panic()s if the scheduler is not configured to
 * handle a run level as high as "runlevel".
 *
 * Returns TRUE if the new run level is set; FALSE if the scheduler is
 * already running at run level "level" or higher.
 */
extern bool set_runlevel(uint16f level);
#endif /* ndef SCHEDULER_WITHOUT_RUNLEVELS */

/**
 * \brief Sends a message consisting of the integer mi and the void* pointer
 * mv to the message queue queueId owned by a task. mi and
 * mv are neither inspected nor changed by the scheduler - the task that owns
 * queueId is expected to make sense of the values.
 *
 * \param[in] queueId      ID of the queue to send the message to
 *
 * \param[in] mi   Int associated with the message
 *
 * \param[in] mv Pointer associated with the message. May be NULL
 *
 * \return A message identifier.   This can be used with cancel_message().
 *
 * \note If mv is not null then it will typically be a chunk of malloc()ed
 * memory, though there is no need for it to be so.   Tasks should normally obey the
 * convention that when a message built with malloc()ed memory is given to
 * put_message() then ownership of the memory is ceded to the scheduler - and
 * eventually to the recipient task.   I.e., the receiver of the message will be
 * expected to free() the message storage.
 *
 * \note Note also that this function must be re-entrant - a call to this function
 * may cause another task to run which itself may call this function.
 */

extern msgid put_message_with_routing(qid queueId, 
                                    uint16 messageInt, 
                                    void *pMessageBody,
                                    tRoutingInfo *routing);

#define put_message(q, mi, mb) put_message_with_routing(q, mi, mb, NULL)

/**
 * put_message can be used from the foreground with sched_oxygen, unlike CarlOS
 * sched
 */
#define fg_put_message(queueId, mi, mv) put_message(queueId, mi, mv)

/**
 * \brief Send a message at a specific time in the future.
 * This basically wraps put_message, so refer there for information.
 *
 * \param[in] queueId       ID of the queue to send the message to
 * \param[in] mi    Int associated with the message
 * \param[in] mv  Pointer associated with the message. May be NULL.
 * \param[in] deadline      Time at which to send the message.
 *
 * \return  A timer ID. This can be used with timer_cancel_event if necessary.
 */
extern tTimerId put_message_at(TIME deadline,
                               qid queueId,
                               uint16 mi,
                               void *mv);

/**
 * \brief Send a message after a delay.
 * This basically wraps put_message, so refer there for information.
 *
 * \param[in] queueId       ID of the queue to send the message to
 * \param[in] mi    Int associated with the message
 * \param[in] mv  Pointer associated with the message. May be NULL.
 * \param[in] delay         Send the message after this amount of time.
 *
 * \return  A timer ID, or TIMER_ID_INVALID if the delay was so short that
 * the message was put onto the queue immediately.
 */
extern tTimerId put_message_in(INTERVAL delay,
                               qid queueId,
                               uint16 mi,
                               void *mv);

/**
 * Cancel a message that has been scheduled for sending later
 * @param q UNUSED (included for compatibility with sched_carlos)
 * @param timer_id ID returned from put_message_at/put_message_in
 * @param pmi Pointer to space for mi to be returned in (NULL if not required)
 * @param pmv Pointer to space for mv to be returned in (NULL if not required)
 * @return TRUE if a message was cancelled, else FALSE.  If FALSE, *pmi and *pmv
 * are invalid.
 */
extern bool cancel_timed_message(qid q,
                                 tTimerId timer_id,
                                 uint16 *pmi, void **pmv);

/**
 * \brief Raise a background interrupt to required bg interrupt if not already
 * raised.
 *
 * \param[in] task_id      ID of the bg_int to send the message to.  If the
 * task generation mechanism supports pairing tasks and bg_ints, it is legal to
 * pass the paired task's taskid here instead.
 *
 * \return Nothing.
 *
 * \note
 * This function is usually called within an interrupt handler to signal an
 * event to a background task.
 */
extern void raise_bg_int(taskid task_id);

/**
 * \brief Raise a background interrupt to required bg interrupt if not already
 * raised.
 *
 * \param[in] bg_int      The background interrupt to raise a bg interrupt on.
 *
 * \return Nothing.
 *
 * \note
 * This function is usually called to kick an operator in a very efficient way.
 * This function performs no safety checks and the caller is responsible for
 * managing the validity of bg_int over the lifetime of the operator task.
 */
extern void raise_bg_int_with_bgint(BGINT_TASK bg_int);

/**
 * \brief Obtains a message from the message queue ID queue_id if one is
 * available. The calling task must own this Q. The message consists of one or
 * both of a int and a void*.
 *
 * \param[in]     queue_id        ID of the queue from which to get a message
 *
 * \param[in,out] *pmi   Integer from the message, if pointer is not NULL
 *
 * \param[in,out] *pmv Pointer associated with the message, if
 * pmv is not NULL.
 *
 * \return TRUE if a message has been obtained from the queue, else FALSE.
 *
 * \note
 * See the note on malloc()ed memory ownership in the description of \ref
 * put_message .
 *
 * \par
 * If a message is taken from the queue, then *pmi and *pmv
 * are set to the mi and mv passed to put_message().
 *
 * \par
 * pmi and/or pmv can be null, in which case the corresponding value from the
 * message is discarded.
 *
 * \par
 * If the function returns TRUE, and if pmv is null, then there is a
 * danger of a memory leak.  The scheduler does not know what is stored
 * in the void*, so it cannot reclaim a chunk of malloc()ed memory
 * there.   In most circumstances pmv should not be null; however,
 * it may be that the application knows that all messages sent on "q" will use
 * the "MessageInt" only, so it may be acceptable for pmv to be null.
 *
 * \par
 * MUST NOT BE CALLED DIRECTLY FROM INTERRUPT CODE - can only be called by the
 * task that owns the queue given by queueId.
 */
extern bool get_message_with_routing(qid queue_id,    
                         uint16 *pmi,
                                     void **pmv,
                                     tRoutingInfo *routing);

#define get_message(q, mi, mb) get_message_with_routing(q, mi, mb, NULL)

/**
 * \brief Creates a new task and/or bg int with the given parameters,
 * initialises the new task and returns the task id.
 *
 * If \c bg_int_handler is \c NULL, creates a standalone (uncoupled) task. If
 * \c msg_handler is NULL, \c num_of_queues should be 0 and the function creates
 * an uncoupled bg int.  If both handlers are supplied, the function creates a
 * coupled bg int/task pair.  In this case the id returned is the task's; the
 * bg int's differs from this only in bit 23.
 *
 * \param[in]     task_priority  Priority of the new task
 * \param[in]     num_of_queues  Number of message queues associated with the
 * task
 * \param[in]     task_data      Task-specific data (see below)
 * \param[in]     msg_handler    Message handler function to handle messages
 * received in the message queues associated with the task
 * \param[in]     bg_int_handler Background interrupts handling function to
 * handle background interrupts
 * \param[in]     flush_msg      Function to flush any queued messages to the
 * task when the task is deleted. Could be NULL
 * \param[in,out] created_task_id   Pointer to return the task id in if the 
 * creation is successful
 *
 * \return TRUE if the creation succeeded, FALSE if there was insufficient 
 * memory.to create the task.
 *
 * \note The task id of the newly created task is used to reference the 
 * task to send messages or background interrupts.  Note that in the coupled
 * case, while the bg int has its own ID, it is possible to raise bg ints using
 * the task's ID.
 *
 * \par \c task_data is specific to the task and is passed to all the handler
 * functions for the task. The scheduler does not do anything with it. The task
 * user should allocate memory if it is required and free it when the task is
 * deleted.
 *
 * \par If \c flush_msg routine is not provided and the scheduler tries to
 * reclaim \c pmv for any messages that are queued to the task that is being
 * deleted, then there is a danger of a memory leak.  The scheduler does not
 * know what is stored in the \c void*, so it cannot reclaim a chunk of
 * \c malloc()ed memory there. It may be that the application knows that all
 * messages sent on "q" will use the "MessageInt" only, or that there will be
 * no messages in the queue when the task delete is called, so it may be
 * acceptable for \c flush_msg to be \c NULL.
 */
extern bool create_task(
        PRIORITY task_priority, uint16f num_of_queues,
        void *task_data, void (*msg_handler)(void **task_task_data),
        void (*bg_int_handler)(void **bg_task_data), MSG_FLUSH_HANDLER flush_msg,
        taskid *created_task_id);

/**
 * Helper macro that automatically sets redundant \c create_task arguments for
 * creating an uncoupled bg int
 */
#define create_uncoupled_bgint(task_priority, task_data, bg_int_handler, id) \
    create_task(task_priority, 0, task_data, NULL, bg_int_handler, NULL, id)

/**
 * Helper macro that automatically sets redundant \c create_task arguments for
 * creating an uncoupled task
 */
#define create_uncoupled_task(task_priority, num_of_queues, task_data,\
                              msg_handler, flush_msg, id) \
    create_task(task_priority, num_of_queues, task_data, msg_handler, NULL,\
                flush_msg, id)

/**
 * \brief Gets the message queue Id for the given task
 *
 * \param[in] task_id       task Id which owns the message queue
 * \param[in] queue_index   message queue index within the task, numbered 0-N-1
 *                          where N is the number of queues for the task
 * \param[out] pQueue_id    Pointer to return requested queue Id
 *
 * \return TRUE if queue exists.
 */
extern bool get_msg_qid_for_task(taskid task_id, uint16f queue_index,
                                 qid *pQueue_id );

/**
 * \brief Deletes a scheduler task that was previously created
 *
 * \param[in] id       task Id which is to be deleted
 */
extern void delete_task(taskid id);

#ifdef UNIT_TEST_BUILD
extern void pl_verify_queue(qid queue_id, bool is_empty);
#endif

/**
 * Cancel all pending messages.
 *
 * Here for the host code, but available wherever needed.
 */
void cancel_all_messages(void);

/**
 * Cancel a single message on a given queue
 * \param q The queue
 * \param mid The message to cancel
 * \param pmi Pointer to the message integer
 * \param pmv Pointer to the message pointer
 * \return TRUE if the specified message was found on the queue (and deleted);
 * FALSE if not.
 */
extern bool cancel_message(qid q, msgid mid, uint16 *pmi, void **pmv);

/**
 * Cancel all pending bg_ints.
 *
 * This function is usually only used by the test code, but there
 * is nothing test-specific in it.
 */
extern void cancel_all_bg_ints(void);

/**
 * Return the priority of the current task
 */
extern uint16f current_task_priority(void);

/**
 * Return the ID of the current task
 */
extern taskid get_current_task(void);

/**
 * Tell the scheduler the casual-wakeup timer expired 
 * and there is (probably) a timer to service. WARNING! This function expects
 * to be called from interrupt (i.e. with interrupts blocked).
 */
extern void sched_background_kick_event(void);

/**
 * \brief Allows a user to get the Background interrupt structure
 * for a task. This is for use with raise_bg_int_with_bgint. The
 * returned bgint becomes invalid when the related task is deleted
 * and should be no longer used.
 * 
 * \param task_id ID of the bg_int to send the message to.  If the
 * task generation mechanism supports pairing tasks and bg_ints, it is legal to
 * pass the paired task's taskid here instead.
 * 
 * \param bgint Location to return the bg structure.
 * 
 * \return TRUE if found. FALSE if not found.
 */
extern bool sched_find_bgint(taskid task_id, BGINT_TASK *bgint);

/**
 * \brief Cleanup cached message(s). Can be called on any processor,
 * but useful when using leak finder on aux (secondary) 
 * processorto not report these memories.
 */
extern void sched_clear_message_cache(void);

#ifdef DESKTOP_TEST_BUILD
/*
 * Temporarily set all bg_int handlers to NULL, for the benefit of
 * bgint_none test
 */
extern void cancel_bg_ints(void);

/*
 * Restore the bg_int handlers to their original values
 */
extern void restore_bg_ints(void);
#endif

#endif   /* SCHED_OXYGEN_H */
