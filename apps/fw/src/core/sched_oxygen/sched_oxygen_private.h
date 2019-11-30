/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
Copyright (c) 2008 - 2016 Qualcomm Technologies International, Ltd.
  
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
*/

#ifndef SCHED_OXYGEN_PRIVATE_H
#define SCHED_OXYGEN_PRIVATE_H

/****************************************************************************
Include Files
*/
#include "assert.h"

#include "sched_oxygen/sched_oxygen.h"
#include "panic/panic.h"
#include "pmalloc/pmalloc.h"
#include "hydra_log/hydra_log.h"
#include "pl_timers/pl_timers_private.h"
/*#include "hal/kalimba_reg_addresses.h"*/
/*lint -e750 -e962*/ #define IO_DEFS_MODULE_K32_TIMERS
#include "io/io_defs.h"
#include "kal_utils/kal_utils.h"
#include "patch.h"
#ifdef IPC_MODULE_PRESENT
#include "ipc/ipc.h"
#endif
#include "utils/utils_bit.h"

/* Remove log trace macros.  We'll come up with a better solution for CSRA6810x
 * when we have to deal with a similar thing in Synergy */
#define PL_PRINT_P1(tr, string, arg)
#define PL_PRINT_P2(tr, string, arg1, arg2)
#define PL_PRINT_P3(tr, string, a1, a2, a3)


/****************************************************************************
Public Macro Declarations
*/
/**
 * These macros appeared in the CarlOS code from which some of this was derived;
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

/**
 * Maximum number of background interrupts per task. Each background
 * interrupt is represented as a bit on the bgintstatus element of a task.
 */
#define MAX_NUM_OF_BG_INTS 16

/**
 * Set background interrupt status bit
 */
#define SET_BG_INT_POS(pos) (1<<(pos))

/**
 * The maximum number of messages.
 */
#define MAX_NUM_MESSAGES (100)

/* Use the natural word size for the tskid, even though it's only allowed to
 * range up to 127, because it's more efficient in the processor */
typedef uint16f tskid;

/**
 * Minimum and maximum values for taskids
 */
#define MIN_TASKID ((taskid) 0)
#define MAX_TASKID ((taskid)0xFFFFFFU)

/**
 * Special value of taskid used to indicate no task
 */
#define NO_TASK MAX_TASKID

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
  * Does the given QID belong to a remote task?
  */
#define QID_IS_REMOTE(q) (QID_TO_TSKID(q) < TASK_ID_START_P1)

/**
 * Find a taskid from its qid.
 */
#define TASKID_MASK 0xff00ffU
#define QID_TO_TASKID(q) (taskid)((q) & TASKID_MASK)

/**
 * Find the ID of queue index zero for a given taskid
 */
#define TASKID_TO_Q0_QID(t)     (qid)(t)

 /**
  * Find priority from given task/queue id
  */
 #define GET_TASK_PRIORITY(x)  ((uint16f)((x)>>16&0x1F))

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
    MSGQ *mqueues;   /**< Array of task's message queues. */
    unsigned int nqueues;   /**< length of mqueues[]. */
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
    /** Shortcut to the first task that has a message to process. */
    TASK *first_message;
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
    /** Boolean or bitmap indicating the BG int has been raised */
    uint16f                 raised;
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
    /** Shortcut to the first bgint that has a message to process. */
    BGINT *first_message;
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

/**
 * Set by interrupt handler to indicate IRQ is active. Used by scheduler to
 * disable context switch when PlPutMessage is called from within IRQ
 */
extern int SchedInterruptActive;


extern TASKQ tasks_in_priority[NUM_PRIORITIES];
extern BG_INTQ bg_ints_in_priority[NUM_PRIORITIES];

/** Number of background interrupts/messages pending. */
extern volatile uint16f TotalNumMessages;

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
 * \return Pointer to the task if found, NULL if not.
 */
extern TASK *sched_find_task(taskid task_id);

#endif   /* SCHED_OXYGEN_PRIVATE_H */



