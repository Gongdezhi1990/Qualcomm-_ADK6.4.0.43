/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Light-Weight Schedule (interface).
 */
#ifndef UTILS_SCHED_H
#define UTILS_SCHED_H

/*****************************************************************************
 * Interface Dependencies
 ****************************************************************************/

#include "utils/utils_sll.h" /* singly linked list */

/*****************************************************************************
 * Type Declarations
 ****************************************************************************/

/**
 * Queue-able Job (Abstract Base)
 *
 * Abstract Base for a Job that can be queued in a Job Queue.
 *
 * A typical specialisation might be a message pumping job for a Task with
 * pending messages.
 *
 * Storage:-
 * - 1 Data ptr.
 * - 1 Fn ptr.
 *
 * Known uses:-
 * - To schedule event delivery to the SSSM on NFC & WIFI BOOT
 *
 * \see utils_JobQ
 */
typedef struct utils_Job utils_Job;

/**
 * Job Queue
 *
 * Simple intrusive/allocation-free queue of Jobs.
 *
 * Storage:-
 * - 1 Data ptr
 *
 * Known uses:-
 * - Patch-safe scheduling of event delivery to the SSSM on NFC & WIFI BOOT
 */
typedef struct utils_JobQ utils_JobQ;

/*****************************************************************************
 * Type Definitions
 ****************************************************************************/

struct utils_Job
{
    /**
     * \private     Intrusive List membership.
     *
     * Enables the Job to be a member of one Job Q.
     */
    utils_SLLMember list_membership;

    /**
     * \protected   Execute this Job (pure virtual)
     *
     * Specialise this to do the job. The job param passed to exec
     * supports retrieval of per-job-instance context.
     */
    void (*exec)(utils_Job *job);
};

struct utils_JobQ
{
    /**
     * \private Queue of runnable Jobs.
     *
     * Optimization. Uses SLL. Could use a queue (SLL + tail ptr) if order(N)
     * is a problem.
     */
    utils_SLL jobs;
};

/*****************************************************************************
 * Macros used within the following implementations
 ****************************************************************************/

#define utils_JobQ_jobs(q) \
    (&(q)->jobs)

/*****************************************************************************
 * Function Declarations
 ****************************************************************************/

/**
 * Initialise this Job.
 *
 * Sets the "exec" function that is called to execute the Job.
 *
 * \protected \memberof utils_Job
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern void utils_Job_init(utils_Job *job, void (*exec)(utils_Job *job));
#else /* DOXYGEN_ONLY */
#define utils_Job_init(j,exec_fn) \
    ((j)->exec = (exec_fn))
#endif /* DOXYGEN_ONLY */

/**
 * Initialise this JobQueue.
 *
 * \public \memberof utils_JobQ
 * \post    Q is empty!
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern bool utils_JobQ_init(utils_JobQ *queue);
#else /* DOXYGEN_ONLY */
#define utils_JobQ_init(q) \
    utils_SLL_init(utils_JobQ_jobs(q))
#endif /* DOXYGEN_ONLY */

/**
 * Enqueue the specified Job to this JobQueue.
 *
 * \public \memberof utils_JobQ
 */
extern void utils_JobQ_enqueue_job(utils_JobQ *queue, utils_Job *job);

/**
 * Service this JobQueue.
 *
 * Removes and executes the first Job in queue (if any).
 *
 * \public \memberof utils_JobQ
 * \returns TRUE if a Job was serviced, FALSE otherwise.
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern bool utils_JobQ_service(utils_JobQ *queue);
#else /* DOXYGEN_ONLY */
#define utils_JobQ_service(q) \
    (utils_SLL_isEmpty(utils_JobQ_jobs(q)) ? \
                        FALSE : (utils_JobQ_exec_next_job(q), TRUE))
#endif /* DOXYGEN_ONLY */

/**
 * Execute first job in this JobQueue.
 *
 * Removes and executes the first Job in queue.
 *
 * \private \memberof utils_JobQ
 * \pre     There is a Job in the queue.
 */
extern void utils_JobQ_exec_next_job(utils_JobQ *queue);

/*****************************************************************************
 * Function Definitions (macros)
 ****************************************************************************/

#ifndef DOXYGEN_ONLY
/** \cond */

#define utils_Job_list_membership(j) \
    (&(j)->list_membership)

#define utils_JobQ_submit_job(q,j) \
    utils_SLL_append(utils_JobQ_jobs(q), utils_Job_list_membership(j))

/** \endcond */
#endif /* DOXYGEN_ONLY */

#endif /* ndef UTILS_SCHED_H */
