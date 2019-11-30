/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 ************************************************************************//**
 * \file
 * header for the internals of the scheduler
 *
 * This file contains bits of the scheduler that we don't want the
 * scheduler's users to see.
 *
 * \section ident Task and background interrupt identifiers
 *
 * The scheduler has a static array of TASKs and a static array of BG_INTs. Both
 * \c TASK and \c BGINT (background interrupts) have a field "pstk_id", which
 * is a compound identifier, consisting of:
 *  - Bits 0-7: the task/bg_int's "tskid", which is its index
 *  into the static array
 *  - Bits 8-15: 0 (this contains the queue ID in queues'
 *  identifiers
 *  - Bits 16-20: the task/bg_int's priority
 *  - Bit 21: set if the task/bg int is coupled to a bg int/task
 *  - Bit 22: set if a bg int, unset if a task
 *  - Higher bits (if any) ignored
 *
 * \section bg_ints Background interrupt behaviour
 *
 * For historical reasons, background interrupts are slightly over-engineered.
 * There are basically two modes of operation, the first originating in the
 * Audio subsystem firmware, the second reflecting the behaviour of CarlOS-style
 * bg ints.
 *
 * \subsection audio_bg_ints Audio-style background interrupts
 * In this approach, there is one bg int handler per task, but it is passed a
 * bitmap which allows up to 16 logical bg ints
 * to be multiplexed to the same "physical" bg int.  Previously, this approach
 * was hard-coded into the scheduler's implementation, with the result that
 * bg ints were always raised on a \e task ID.  Now, \c raise_bg_int() logically
 * takes a \e bg \e int ID, but so long as the convention is followed that the
 * bg int coupled to a task has the same ID as the task apart from bit 22
 * (\c BG_INT_FLAG_BIT), it is possible still to pass a task ID to this
 * function and the right thing happen.  Clearly, it is up to the task geneation
 * mechanism to ensure that this "coupling" correspondence holds, if desired.
 * The dynamic task creation functionality does this based on whether both a
 * message and bg int handler are supplied or just one or the other.
 *
 * \subsection carlos_bg_ints CarlOS-style background interrupts
 * In the CarlOS scheme, there is no distinction between logical and physical
 * bg ints; in other words, each logical bg int is explicitly named and has its
 * own handler.  The relationship between bg ints and tasks is purely
 * conventional, and is usually communicated by suitable naming (e.g. the
 * "submsg" task has two associated bg ints - "submsg_rx" and "submsg_tx").
 *
 * \subsection bg_int_handler BG INT handler
 * In any case, there is no constraint on the behaviour of the bg int handler in
 * terms of its ability to pass messages: it can post to any queue, just as
 * any other function can (since in fact it is just another function).
 *
 * \section tasks Task behaviour
 *
 * Each task owns an array of message queues - also statically
 * defined. This array is not null-terminated - its length is held in
 * the TASK structure. Each task must have between 1 and 256 message
 * queues.
 *
 * Any task may send messages to any queue. Only the task which owns
 * a queue may consume messages from it.
 *
 * The values of qid (queue identifiers) match those of their owning task in
 * bits 0-7 and 16-23.  Bits 8-15 contain the queue index.  So, the first queue
 * of the first task (tskid == 0) with priority (say) 2 has a qid of value
 * 0x00020000. The second queue of the first task has qid 0x00020100. The second
 * queue of the second task with priority (say) 4 has qid of
 * 0x00040101, etc. Use the QID_TO_TSKID and QID_TO_QINDEX macros to extract
 * queue indices and tskids from the qid
 *
 */

#ifndef SCHED_OXYGEN_PRIVATE_H
#define SCHED_OXYGEN_PRIVATE_H

/****************************************************************************
Include Files
*/
#include "assert.h"
#include "limits.h"
#include "sched_oxygen/sched_oxygen.h"
#include "panic/panic.h"
#include "pmalloc/pmalloc.h"
#include "hydra_log/hydra_log.h"
#include "pl_timers/pl_timers_private.h"
#ifdef DORM_MODULE_PRESENT
#include "dorm/dorm.h"
#endif
#include "hal.h"
#include "platform/pl_intrinsics.h"
#include "patch/patch.h"
#include "platform/pl_trace.h"
#include "platform/pl_assert.h"
#include "platform/profiler_c.h"

#ifdef CHIP_BASE_CRESCENDO
#ifndef TODO_CRESCENDO
/* Remove log trace macros.  We'll come up with a better solution for Crescendo
 * when we have to deal with a similar thing in Synergy */
#define PL_PRINT_P1(tr, string, arg)
#define PL_PRINT_P2(tr, string, arg1, arg2)
#define PL_PRINT_P3(tr, string, a1, a2, a3)
#endif /* TODO_CRESCENDO */
#endif /* CHIP_BASE_CRESCENDO */

/****************************************************************************
Public Macro Declarations
*/
/**
 * These macros appeared in the CarlOS code from which some ofthis was derived;
 * we currently have no need for mutexes in desktop builds, but it's worth
 * leaving the calls in place in case they need to be re-inserted in future
 */

#ifdef DESKTOP_TEST_BUILD
/** Don't need to acquire a mutex. */
#define HYDRA_SCHED_ACQUIRE_MUTEX()
/** Don't need to release a mutex, either. */
#define HYDRA_SCHED_RELEASE_MUTEX(not_idle)
#else
/* In the firmware, the places where we acquire a mutex in the desktop
 * case are places we need to block interrupts (but this only currently affects
 * code which isn't expected to be called by the firmware)
 */
#define HYDRA_SCHED_ACQUIRE_MUTEX() block_interrupts();
#define HYDRA_SCHED_RELEASE_MUTEX(not_idle) unblock_interrupts();
#endif /* DESKTOP_TEST_BUILD */
/** Don't need to redirect msgs */
#define HYDRA_SCHED_REDIRECT_MSG(q)

#define LOCK_TASK_LIST_INTS_BLOCKED(priority) (tasks_in_priority[(priority)].locked++)

#define LOCK_TASK_LIST(priority) \
    do {\
        block_interrupts();\
        LOCK_TASK_LIST_INTS_BLOCKED(priority); \
        unblock_interrupts();\
    } while (0)

#define UNLOCK_TASK_LIST_INTS_BLOCKED(priority) \
    do {\
        if (!TASK_LIST_WAS_ALREADY_LOCKED(priority))\
        {\
            prune_tasks(priority);\
        }\
        tasks_in_priority[(priority)].locked--; \
    } while (0)

#define UNLOCK_TASK_LIST(priority) \
    do {\
        block_interrupts();\
        UNLOCK_TASK_LIST_INTS_BLOCKED(priority); \
        unblock_interrupts();\
    } while (0)



#define TASK_LIST_IS_LOCKED(priority) \
    (tasks_in_priority[(priority)].locked > 0)
#define TASK_LIST_WAS_ALREADY_LOCKED(priority) \
    (tasks_in_priority[(priority)].locked > 1)


#define LOCK_BGINT_LIST_INTS_BLOCKED(priority) (bg_ints_in_priority[(priority)].locked++)

#define LOCK_BGINT_LIST(priority) \
    do {\
        block_interrupts();\
        LOCK_BGINT_LIST_INTS_BLOCKED(priority); \
        unblock_interrupts();\
    } while (0)

#define UNLOCK_BGINT_LIST_INTS_BLOCKED(priority) \
    do {\
        if (!BGINT_LIST_WAS_ALREADY_LOCKED(priority))\
        {\
            prune_bg_ints(priority);\
        }\
        bg_ints_in_priority[(priority)].locked--; \
    } while (0)

#define UNLOCK_BGINT_LIST(priority) \
    do {\
        block_interrupts();\
        UNLOCK_BGINT_LIST_INTS_BLOCKED(priority); \
        unblock_interrupts();\
    } while (0)


#define BGINT_LIST_IS_LOCKED(priority) \
    (bg_ints_in_priority[(priority)].locked > 0)
#define BGINT_LIST_WAS_ALREADY_LOCKED(priority) \
    (bg_ints_in_priority[(priority)].locked > 1)

/**
 * The maximum number of messages.
 */
#define MAX_NUM_MESSAGES (100)

/* Use the natural word size for the tskid, even though it's only allowed to
 * range up to 127, because it's more efficient in the processor */
typedef uint16f tskid;

/**
 * Find a message queue index from its qid.
 */
#define QINDEX_MASK  0xff00UL
#define QID_TO_QINDEX(q) ((uint16f)(((q) & QINDEX_MASK) >> 8))

/**
 * Find a tskid from its qid.
 */
#define TSKID_MASK 0xffU
#define QID_TO_TSKID(q) (tskid)((q) & TSKID_MASK)

/**
 * Find a taskid from its qid.
 */
#define TASKID_MASK 0xff00ffU
#define QID_TO_TASKID(q) (taskid)((q) & TASKID_MASK)


/** The maxium number of messages queued from the foreground */
#define MAX_FG_MESSAGES         (10)

/****************************************************************************
Public Type Declarations
*/

/**
 * TASK - The information for a system task.
 *
 * TASKs defined at compile time (static tasks) are held in an array tasks[]
 * created by autogeneration magic.
 * The scheduler holds all defined TASKs (static and dynamic) in a linked list
 * corresponding to the task priority.
 */
typedef struct _TASK
{
    taskid id; /**< priority + tskid of the task. Used as key to search
                          and identify task */
    bool prunable; /**< State if this is a dynamic task and can be
                                deleted*/
    void (*init)(void**); /**< Initialisation function. May be NULL */
    void (*handler)(void**); /**< Message handler. May be NULL */
#ifdef SCHED_MULTIQ_SUPPORT
    MSGQ *mqueues;   /**< Array of task's message queues. */
    unsigned int nqueues;   /**< length of mqueues[]. */
#else
    MSGQ mqueue;   /**< Task's message queue. */
#endif
    /** Function to flush a message received for the task when the task is up
     * for deletion. If this function is not provided, the scheduler will try
     * to reclaim the message body (if not NULL) of any queued messaged when
     * deleting the task. This could potentially result in a memory leak if
     * there are other allocated memory chunks in the message body. */
    MSG_FLUSH_HANDLER flush_msg;
    void *priv; /**< Private data for the task handler */
#ifndef SCHEDULER_WITHOUT_RUNLEVELS
    uint16f runlevel; /**< Runlevel of this task */
#endif
    struct _TASK  *next; /**< Pointer to the next task in a linked list */
} TASK;

/**
 * Task queue
 */
typedef struct
{
    /** Pointer to first task in the Queue; Set to NULL if Q is empty */
    TASK  *first;
    /** Number of messages in priority */
    volatile int num_msgs;
    /** The number of tasks marked for delete */
    volatile uint16f prunable;
    /** Deletion lock. If this is non-zero, tasks can't be deleted, only marked
     * for deletion */
    uint16f locked;
} TASKQ;

/**
 * Background interrupt handler type.
 *
 * @param priv Pointer to the partner task's private memory area.  This *must*
 * be ignored if the bg int does not have a designated partner task.
 *
 * Note that CarlOS-style background interrupt handlers differ slightly in form
 * and function from Audio-style handlers.  Firstly, they don't take the
 * private memory pointer, since by definition they are not partnered with a
 * task; secondly, they have no notion of multiplexing of logical bg ints - they
 * are either raised or not - and as a result, thirdly, they expect the
 * scheduler to clear their flag and hence don't do it themselves.  Hence the
 * CarlOS-style autogeneration macros create a wrapper of type \c bg_int_fn
 * which handles these interface differences and then calls the real handler.
 */
typedef void (*bg_int_fn)(void **priv);

/** Information about a background interrupt. */
typedef struct _BGINT {
    /** BG int ID. Same as a task ID, except that bit 22 is set */
    taskid                  id;
    bool                    prunable; /**< Indicates if this is a dynamic task
                                            that can be deleted*/
    /** bg_int service function. */
    bg_int_fn               handler;
    /** Boolean indicating the BG int has been raised */
    bool                    raised;
    /** Pointer to either the associated task's or the local private memory
     * pointer. */
    void                    **ppriv;
    struct _BGINT           *next;
} BGINT;

typedef struct _UNCOUPLED_BGINT {
    BGINT base; /* The real BGINT struct.  MUST BE THE FIRST ELEMENT IN THIS
    STRUCTURE */
    void *priv; /* Local pointer to private memory */
} UNCOUPLED_BGINT;

typedef struct
{
    BGINT *first;
    volatile int num_raised;
    /** The number of tasks marked for delete */
    volatile uint16f prunable;
    /** Whether the queue is currently locked or not */
    uint16f locked;
} BG_INTQ;



/****************************************************************************
Global Variable Definitions
*/

/**
 * The scheduler's task list. Defined by autogeneration magic. We modify
 * this at runtime, so it's held in RAM.
 */
extern TASK tasks[];

/**
 * The scheduler's background interrupt list.  Defined by autogeneration magic.
 * We modify this at runtime, so it's held in RAM.
 */
extern BGINT bg_ints[];

extern TASKQ tasks_in_priority[NUM_PRIORITIES];
extern BG_INTQ bg_ints_in_priority[NUM_PRIORITIES];
/****************************************************************************
Public Function Prototypes
*/

/* Function called on exit from IRQ in interrupt.asm
 * to check for a possible context switch
 */
extern void exit_irq_check_context_switch(void);

/**
 * Search for the given task.
 *
 * \note If this is called without interrupts blocked, the task may
 * have been deleted by the time the function returns, in which case
 * \c *the_task will be an invalid pointer.
 *
 * \param task_id Task ID to search for
 * \param the_task Pointer to return a pointer to the task in if found,
 * and if not NULL.
 * \return TRUE if the task was found, FALSE if not.
 */
extern bool sched_find_task(taskid task_id, TASK **the_task);

/**
 * Physically delete bg ints that have been logically deleted while
 * their list was locked
 * \param priority The priority level of the queue to prune
 *
 * \note This function is intended to be called only from the macro
 * \c UNLOCK_BGINT_LIST.  It must be called with interrupts blocked.
 */
extern void prune_bg_ints(uint16f priority);

/**
 * Physically delete tasks that have been logically deleted while
 * their list was locked
 * \param priority The priority level of the queue to prune
 *
 * \note This function is intended to be called only from the macro
 * \c UNLOCK_TASK_LIST.  It must be called with interrupts blocked.
 */
extern void prune_tasks(uint16f priority);

/**
 * Wake up the background, if special action is needed to do
 * so.
 */
#if defined(DORM_MODULE_PRESENT) && !defined(UNIT_TEST_BUILD)
#define sched_wake_up_background() dorm_wake()
#elif defined(CHIP_HAS_SHALLOW_SLEEP_REGISTERS) && !defined(UNIT_TEST_BUILD)
/* Set allow_goto_shallow_sleep register for non-hydra (no dorm module) platforms,
 * NAPIER is the only one at the moment. */
#define sched_wake_up_background() hal_set_reg_allow_goto_shallow_sleep(0)
#else
#define sched_wake_up_background() ((void)0)
#endif /* DORM_MODULE_PRESENT && !UNIT_TEST_BUILD */

#endif   /* SCHED_OXYGEN_PRIVATE_H */
