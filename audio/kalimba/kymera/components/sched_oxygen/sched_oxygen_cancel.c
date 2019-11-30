/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file
 *
 * Cancel various things.  Mainly used by tests, but not necessarily.
 */

#include "sched_oxygen/sched_oxygen_private.h"

/**
 * Cancel a message sent to a task
 */
bool cancel_message(qid q, msgid mid, uint16 *pmi, void **pmv)
{
    taskid  tsk = QID_TO_TSKID(q);
    uint16f priority_index = GET_TASK_PRIORITY(q);
#ifdef SCHED_MULTIQ_SUPPORT
    uint16f  queue_number = QID_TO_QINDEX(q);
#endif
    MSG  **pm, *m;
    TASK *t;
    MSGQ  *queue;

    /* The task could be deleted at any point if we don't lock the queue.
     * It doesn't hurt if it gets logically deleted while we're in the process
     * of cancelling the message. */
    LOCK_TASK_LIST(priority_index);

    /* Find the task */
    if (!sched_find_task(QID_TO_TASKID(q), &t))
    {
        UNLOCK_TASK_LIST(priority_index);
        return FALSE;
    }

#ifdef SCHED_MULTIQ_SUPPORT
    /* Sanity check. */
    if (queue_number >= t->nqueues)
    {
        panic(PANIC_HYDRA_UNKNOWN_QUEUE);
    }
    queue = &(tasks[tsk].mqueues[queue_number]);
#else
    queue = &tasks[tsk].mqueue;
#endif

    /* Walk the queue looking for the doomed message. */
    HYDRA_SCHED_ACQUIRE_MUTEX(); /* For firmware this becomes block_interrupts()*/
    for (pm = &(queue->first); *pm != (MSG *)(NULL);
         pm = &((*pm)->next))
    {
        /* Snip the offending message from the list. */
        if (mid == (*pm)->id)
        {
            m = *pm;
            *pm = m->next;
            /* If we've found a message, the "there are things to do" counters
             * should be > 0 */
            if ((TotalNumMessages == 0) ||
                    (tasks_in_priority[priority_index].num_msgs <= 0))
            {
                panic(PANIC_OXYGOS_INVALID_MESSAGE_COUNT);
            }
            --TotalNumMessages;
            tasks_in_priority[priority_index].num_msgs--;

            /* We don't have external messages in sched_oxygen but I'm leaving
             * this here commented out in case we ever do */
#ifndef SCHED_NO_EXTERNAL_MSG_SUPPORT
            if (m->external)
            {
                /* Use of delivered queue covered by mutex */
                if (pmi != (uint16 *)(NULL))
                {
                    *pmi = 0;
                }
                if (pmv != (void **)(NULL))
                {
                    *pmv = m;
                }
                sched_message_done(m);
                HYDRA_SCHED_RELEASE_MUTEX(FALSE);
            }
            else
#endif
            {
                HYDRA_SCHED_RELEASE_MUTEX(FALSE);
                /* Return the message's arguments. */
                if (pmi != (uint16 *)(NULL))
                {
                    *pmi = m->mi;
                }
                if (pmv != (void **)(NULL))
                {
                    *pmv = m->mv;
                }
                /* Discard the message's wrappings. */
                /* release_msgid(m->id); */
                pdelete(m);
            }

#ifdef SCHED_STATS
            if (queue->len != 0)
            {
                --(queue->len);
            }
#endif /* SCHED_STATS */
            UNLOCK_TASK_LIST(priority_index);
            return TRUE;
        }
    }
    HYDRA_SCHED_RELEASE_MUTEX(FALSE);
    UNLOCK_TASK_LIST(priority_index);
    return FALSE;
}

/**
 * Cancel all pending bg_ints
 */
void cancel_all_bg_ints(void)
{
    uint16f i;
    uint16f n_bg_ints = N_BG_INTS; /* Avoid compiler warning */
    HYDRA_SCHED_ACQUIRE_MUTEX();
    for (i = 0; i < n_bg_ints; i++)
    {
        if (bg_ints[i].raised)
        {
            uint16f priority_index = GET_TASK_PRIORITY(bg_ints[i].id);
            L0_DBG_MSG1("bg_int %d pending at test termination", i);

            /* If we've found a raised bg_int, the "there are things to do"
             * counts should be > 0 */
            if ((TotalNumMessages == 0) ||
                    (bg_ints_in_priority[priority_index].num_raised <= 0))
            {
                panic(PANIC_OXYGOS_INVALID_MESSAGE_COUNT);
            }
            TotalNumMessages--;
            bg_ints_in_priority[priority_index].num_raised--;
        }
        bg_ints[i].raised = FALSE;
    }
    HYDRA_SCHED_RELEASE_MUTEX(FALSE);
}

/**
 * Cancel all messages on all queues for all tasks.
 *
 * Does not touch the foreground message queue, however (mainly because there
 * isn't one in the Oxygen scheduler).
 */
void cancel_all_messages(void)
{
    TASK *t;
    MSGQ *queue;

    /*
     * Just grab the mutex for the entire function if needed; no point
     * in subtlety at this stage.
     */
    HYDRA_SCHED_ACQUIRE_MUTEX();

#ifdef HAVE_DYNAMIC_TASKS
    /* TODO */
#error "cancel_all_messages isn't enough when we have dynamic tasks. "
    "Rethink required"
#endif
    for (t = tasks; t < tasks + N_TASKS; t++)
    {
#ifdef SCHED_MULTIQ_SUPPORT
        uint16 qindex;
        for (qindex = 0, queue = t->mqueues;
             qindex < t->nqueues;
             qindex++, queue++)
#else
        queue = &t->mqueue;
#endif
        {
            while (queue->first)
            {
                MSG *m = queue->first;

                queue->first = m->next;

                TotalNumMessages--;
                tasks_in_priority[GET_TASK_PRIORITY(t->id)].num_msgs--;
                pdelete(m);
            }
        }
    }

    if (TotalNumMessages)
    {
        /* Bad counting */
        panic(PANIC_HYDRA_BAD_MESSAGE_COUNT);
    }

    HYDRA_SCHED_RELEASE_MUTEX(FALSE);
}

#ifdef DESKTOP_TEST_BUILD
/**
 * \name Skullduggery only needed in testing.
 *
 * It's not worth exposing bg_int_info to general ridicule just to
 * give this its own file.
 *
 * This works because we always test whether the function pointer
 * is NULL (and have always done so even in the firmware, though
 * I don't know whether that facility has ever been used there).
 */

/*@{*/

/** Copy of bg int information */
static BGINT bg_int_copies[N_BG_INTS + 1]; /* Might be no bg ints */

/** Save and remove bg int functions */
void cancel_bg_ints(void)
{
    uint16 j;
    HYDRA_SCHED_ACQUIRE_MUTEX();
    /* Save the real bg ints, and pretend there aren't any */
    for (j = 0; j < N_BG_INTS; j++)
    {
        bg_int_copies[j].handler = bg_ints[j].handler;
        bg_ints[j].handler = NULL;
    }
    HYDRA_SCHED_RELEASE_MUTEX(FALSE);
}

/** Restore bg int functions */
void restore_bg_ints(void)
{
    uint16f j;
    /* Now put the bg ints back */
    HYDRA_SCHED_ACQUIRE_MUTEX();
    for (j = 0; j < N_BG_INTS; j++)
    {
        bg_ints[j].handler = bg_int_copies[j].handler;
    }
    HYDRA_SCHED_RELEASE_MUTEX(TRUE); /* in principle may trigger something */
}

/*@}*/
#endif
