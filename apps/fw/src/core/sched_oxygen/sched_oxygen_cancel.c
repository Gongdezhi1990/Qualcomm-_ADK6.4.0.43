/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 *
 * Cancel various things.  Mainly used by tests, but not necessarily.
 *
 * There are two main "flavours" of cancellation.  In the first, original, case,
 * you supply a message ID and the queue is trawled for that message.  In the
 * more general case, you supply a message int and a comparison function that
 * takes a message int and body and another int and void * for comparison (these
 * are nominally a message int and body for comparison, but the handler function
 * can interpret them as it likes)
 */

#include "sched_oxygen/sched_oxygen_private.h"

static bool cancel_message_impl(qid q, msgid mid, uint16 *pmi, void **pmv,
                        bool (*msg_cmp)(uint16 mi, void *mv,
                                        uint16 cmp_mi, void *cmp_mv),
                        uint16 cmp_mi, void *cmp_mv)
{
    uint16f priority_index = GET_TASK_PRIORITY(q);
    uint16f  queue_number = QID_TO_QINDEX(q);
    MSG  **pm, *m;
    TASK *t;
    MSGQ  *queue;

    /* Find the task */
    t = sched_find_task(QID_TO_TASKID(q));
    if (t == NULL)
    {
        return FALSE;
    }

    /* Sanity check. */
    if (queue_number >= t->nqueues)
    {
        panic(PANIC_HYDRA_UNKNOWN_QUEUE);
    }
    queue = &(t->mqueues[queue_number]);

    /* Walk the queue looking for the doomed message. */
    HYDRA_SCHED_ACQUIRE_MUTEX(); /* For firmware this becomes block_interrupts()*/
    for (pm = &(queue->first); *pm != (MSG *)(NULL);
         pm = &((*pm)->next))
    {
        /* Snip the offending message from the list. */
            /* Mode 1: just check the msgid */
        if ((msg_cmp == NULL && mid == (*pm)->id) ||
            /* Mode 2: msg_cmp returns TRUE */
            (msg_cmp != NULL && msg_cmp((*pm)->mi, (*pm)->mv, cmp_mi, cmp_mv)))
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
            background_work_pending = (TotalNumMessages != 0);
            tasks_in_priority[priority_index].num_msgs--;
            tasks_in_priority[priority_index].first_message = NULL;
            
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
            

#ifdef SCHED_STATS
            if (queue->len != 0)
            {
                --(queue->len);
            }
#endif /* SCHED_STATS */
            return TRUE;
        }
    }
    HYDRA_SCHED_RELEASE_MUTEX(FALSE);
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
            background_work_pending = (TotalNumMessages != 0);
            bg_ints_in_priority[priority_index].num_raised--;
            bg_ints_in_priority[priority_index].first_message = NULL;
        }
        bg_ints[i].raised = 0;
    }
    HYDRA_SCHED_RELEASE_MUTEX(FALSE);
}

/**
 * Cancel a message sent to a task
 */
bool cancel_message(qid q, msgid mid, uint16 *pmi, void **pmv)
{
    return cancel_message_impl(q, mid, pmi, pmv, NULL, 0, NULL);
}

bool cancel_message_cmp(qid q, uint16 *pmi, void **pmv,
                        bool (*msg_cmp)(uint16 mi, void *mv,
                                               uint16 cmp_mi, void *cmp_mv),
                        uint16 cmp_mi, void *cmp_mv)
{
    return cancel_message_impl(q, NO_MSGID, pmi, pmv, msg_cmp, cmp_mi, cmp_mv);
}



#ifdef DESKTOP_TEST_BUILD
/**
 * Cancel all messages on all queues for all tasks.
 *
 * Does not touch the foreground message queue, however (mainly because there
 * isn't one in the Oxygen scheduler).
 *
 * WARNING: DO NOT CALL THIS FUNCTION IN PRODUCTION. This function does not
 * and cannot clean up any custodial pointers within messages. It's for use
 * only by the test harness.
 */
void cancel_all_messages(void)
{
    uint16 qindex;
    const TASK *t;
    MSGQ *queue;

    /*
     * Just grab the mutex for the entire function if needed; no point
     * in subtlety at this stage.
     */
    HYDRA_SCHED_ACQUIRE_MUTEX();

    for (t = tasks; t < tasks + N_TASKS; t++)
    {
        tasks_in_priority[GET_TASK_PRIORITY(t->id)].first_message = NULL;
        for (qindex = 0, queue = t->mqueues;
             qindex < t->nqueues;
             qindex++, queue++)
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
    background_work_pending = FALSE;

    HYDRA_SCHED_RELEASE_MUTEX(FALSE);
}
#endif

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
