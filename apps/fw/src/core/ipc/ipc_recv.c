/* Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd. */
/*    */

#include "ipc/ipc_private.h"

#include "pmalloc/pmalloc.h"  /* Needed because we call pfree explicitly from here*/

/**
 * Leaves the IPC receive buffer pages mapped in if set to TRUE.
 */
static bool leave_pages_mapped = FALSE;

void ipc_leave_recv_buffer_pages_mapped(void)
{
    leave_pages_mapped = TRUE;
}

/**
 * @brief Update the IPC receive buffer behind pointer.
 *
 * Either frees the now unused buffer pages or leaves them mapped in depending
 * on the value of @c leave_pages_mapped.
 *
 * @param [in,out] freed  Points to a bool to track whether the behind has
 *                        already been updated or not.
 */
static void ipc_recv_update_behind(bool *freed)
{
    if( !*freed )
    {
        if( leave_pages_mapped )
        {
            buf_update_behind(ipc_data.recv);
        }
        else
        {
            buf_update_behind_free(ipc_data.recv);
        }
        *freed = TRUE;
    }
}

static void ipc_signal_interproc_event_handler(IPC_SIGNAL_ID id, const void *msg)
{
    UNUSED(msg);
    switch(id)
    {
    case IPC_SIGNAL_ID_SIGNAL_INTERPROC_EVENT:
        hal_set_reg_interproc_event_1(1);
        break;
    default:
        panic_diatribe(PANIC_IPC_UNHANDLED_MESSAGE_ID, id);
    }
}

/**
 * @brief Determines whether ipc_recv() should try to sleep.
 * @return TRUE  If there's no pending work and it's safe to sleep.
 * @return FALSE If ipc_recv() shouldn't sleep.
 */
static bool ipc_recv_should_sleep(void)
{
    bool should_sleep = FALSE;

    if (!ipc_data.pending && !background_work_pending)
    {
        /* The send queue must be clear before sleeping or we may block waiting
           for a response to a message that hasn't been sent yet. */
        block_interrupts();
        should_sleep = ipc_clear_queue();
        unblock_interrupts();
    }

    return should_sleep;
}

/**
 * @brief Put the processor into shallow sleep if possible.
 * @param timeout The timeout to pass to the sleep function.
 */
static void ipc_recv_try_to_sleep(TIME timeout)
{
    while (ipc_recv_should_sleep())
    {
        dorm_shallow_sleep(timeout);
    }
}

void *ipc_recv_atomic(IPC_SIGNAL_ID recv_id, void *blocking_msg)
{
    /* Note: the flag is cleared within ipc_recv_handler when the blocking
     * response is seen */
    ipc_data.atomic_rsp_pending = TRUE;
    return ipc_recv(recv_id, blocking_msg);
}

void *ipc_recv(IPC_SIGNAL_ID recv_id, void *blocking_msg)
{
    bool changed_background_work_pending = FALSE;
    /* Sleep until we see the IPC interrupt fire, and then process the
     * entries.  Keep doing this until we see the recv_id message */

    /*
     * On return payload == blocking_msg unless blocking_msg was NULL, in which
     * case payload will be a pmalloc'd block
     */
    void *payload = NULL;
    /* Purely nominal timeout - it is ignored by dorm_shallow_sleep. */
    TIME latest = time_add(hal_get_time(), SECOND);

    /* Post the blocking ID to the callback queue with a NULL handler.
     * \c call_msg_callback() will interpret this as indicating the message
     * we're blocking on */
    ipc_recv_cb(recv_id, NULL);

    do {
        /* Only bother trying to sleep if we're not in a nested call.  These
         * are supposed to be fast, so there's not much point. */
        if (ipc_data.nest_level == 0)
        {
            ipc_recv_try_to_sleep(latest);
        }

        if (ipc_data.pending)
        {
            payload = ipc_recv_handler(blocking_msg);
        }

        if (ipc_data.nest_level == 0)
        {
            if (background_work_pending && payload == NULL)
            {
                /* We can't service background work anyway until
                 * an expected IPC response is received, so no need to prevent
                 * processor from shallow sleeping. */
                background_work_pending = FALSE;
                /* Remember that we tampered with it */
                changed_background_work_pending = TRUE;
            }
        }
    } while (payload == NULL);

    /* Restore indicator of pending background work.
     * Note: this is save because code running from interrupt handlers
     * only increases TotalNumMessages, so once background_work_pending is set,
     * it doesn't get cleared until the scheduler has a chance to run
     * background work. */
    if (changed_background_work_pending)
    {
        background_work_pending = TRUE;
    }

    return payload;
}

/**
 * Find the first handler for this message ID in the callback queue, remove it
 * from the queue, then either call the handler if it is not NULL, or, if it is
 * NULL, copy the message into a pmalloc block for subsequent return to the
 * caller (this is the blocking message)
 *
 * @note It is worth removing the handler from the queue before
 * execution as the callback may modify the queue (e.g. to repost itself as a
 * handler).
 *
 * @param id  Message ID we're searching for
 * @param msg Message body to pass to callback
 * @param msg_length Byte length of message body
 * @param already_stored Flag to indicate that the message being processed is
 * already stored on the out of order queue, so it shouldn't be stored again
 * @param pblocking_msg Pointer to the pointer to space for the blocking msg
 * to be returned to the caller in.
 * @param got_blocking Flag to be returned to indicate whether \p pblocking_msg
 * has been filled in
 * @return TRUE if a callback was found (and called) else FALSE (indicating a
 * logical error)
 */
static bool call_msg_callback(IPC_SIGNAL_ID id, const void *msg,
                              uint32 msg_length,
                              bool already_stored,
                              void **pblocking_msg,
                              bool *got_blocking)
{
    IPC_RECV_CB_QUEUE **pnext = &ipc_data.recv_cb;
    while(*pnext != NULL)
    {
        if ((*pnext)->recv_id == id)
        {
            IPC_RECV_CB_QUEUE *found = *pnext;
            /* stash the callback */
            IPC_RECV_CB cb = found->cb;
            /* stash the nest level the recv was posted at */
            uint32 posted_nest_level = found->nest_level;
            if (cb == NULL)
            {
                *got_blocking = FALSE; /* unless otherwise stated */
                if (posted_nest_level == ipc_data.nest_level)
                {
                    /* This is the message that is being blocked on, so simply
                     * make a copy of the message and pass the pointer back.
                     * If pblocking_msg points to a non-NULL pointer it means the
                     * caller has already allocated space.  If not, we have to. */
                    if (*pblocking_msg == NULL)
                    {
                        *pblocking_msg = pmalloc(msg_length);
                    }
                    memcpy(*pblocking_msg, msg, msg_length);
                    *got_blocking = TRUE;
                    /* remove this entry from the list */
                    *pnext = (*pnext)->next;
                    pfree(found);
                }
                else if (!already_stored)
                {
                    /* It's impossible to receive a response at a lower nest
                     * level than the recv was posted at, because the inner
                     * call wouldn't have returned without getting its
                     * response */
                    assert(posted_nest_level < ipc_data.nest_level);
                    /* We've received a response we were waiting for, but in an
                     * inner call to ipc_recv.  So we need to save the message
                     * for later */
                    ipc_queue_msg_core(&ipc_data.out_of_order_rsps,
                                       id, msg, (uint16)msg_length);
                    *got_blocking = FALSE;
                }
            }
            else
            {
                /* remove this entry from the list */
                *pnext = (*pnext)->next;
                pfree(found);
                /* Call the callback */
                cb(id, msg);
                *got_blocking = FALSE;
            }
            return TRUE;
        }

        pnext = &((*pnext)->next);
    }
    /* Whoops - there's no callback registered */
    return FALSE;
}

/**
 * Loop through the list of postponed high-priority handlers, calling them
 * one by one.
 * Note: the list may be modified as we go as a result of calls to atomic
 * blocking traps by the high-priority handlers.  That is logically safe as we
 * read the next pointer only after this can have happened.  For it to lead to
 * us staying in the loop for a long time, it would be necessary for further
 * trigger messages to keep arriving at a very high rate, i.e. fast enough to
 * keep arriving during blocking calls the handlers are making.  But this
 * indicates a more fundamental problem in terms of P1 processing data fast
 * enough, so we can assume here that this'll be OK.
 */
static void ipc_recv_process_postponed_hp_handlers(void)
{
    IPC_MSG_QUEUE **pnext = &ipc_data.postponed_hp_triggers;
    while (*pnext)
    {
        IPC_MSG_QUEUE *msg = *pnext;
        switch(msg->msg_id)
        {
        case IPC_SIGNAL_ID_APP_MSG:
        case IPC_SIGNAL_ID_APP_SINK_SOURCE_MSG:
        case IPC_SIGNAL_ID_APP_MSG_TO_HANDLER:
            if (!ipc_trap_api_handler(msg->msg_id, msg->msg, msg->length_bytes))
            {
                /* This is a programming error in the IPC layer */
                L0_DBG_MSG("IPC implementation logic error: trigger handler "
                            "didn't run during explicit processing of "
                            "postponed triggers");
                assert(FALSE);
            }
            break;
        default:
            L0_DBG_MSG1("IPC implementation logic error: unexpected HP trigger "
                        "signal 0x%x", msg->msg_id);
            assert(FALSE);
        }
        *pnext = (*pnext)->next;
        pfree(msg);
    }
}

/* Does anything in the out_of_order blocking responses match a blocking
 * message in the cb queue at the current nest level?  If so, return it
 *
 * \param blocking_msg Pointer to the pointer that the blocking msg gets
 * returned in
 * \return TRUE if a suitable blocking msg was found.  (There should logically
 * be either 0 or 1 of these, but there's no check that that's the case.)
 * */
static bool ipc_recv_process_out_of_order_rsps(void **blocking_msg)
{
    IPC_MSG_QUEUE **pnext = &ipc_data.out_of_order_rsps;
    bool got_blocking = FALSE;
    while (*pnext)
    {
        IPC_MSG_QUEUE *msg = *pnext;
        (void)call_msg_callback(msg->msg_id, msg->msg, msg->length_bytes,
                                TRUE, blocking_msg, &got_blocking);
        if (got_blocking)
        {
            /* Remove the link from the list */
            *pnext = (*pnext)->next;
            /* and throw it away (note: the list entry and the message body are
             * in the same pmalloc block) */
            pfree(msg);
            return TRUE;
        }
        pnext = &(*pnext)->next;
    }
    return FALSE;
}


void *ipc_recv_handler(void *blocking_msg)
{
    uint16f n_processed = 0;
    void *returned_msg = NULL;
    bool got_blocking;

    block_interrupts();
    ipc_data.pending = FALSE;
    unblock_interrupts();

    /* We consume everything there is because IPC is a relatively high-priority
     * task */
    while(BUF_ANY_MSGS_TO_SEND(ipc_data.recv) &&
                                        n_processed < IPC_MAX_RECV_MSGS)
    {
        bool msg_freed = FALSE;
        const void *msg = (const void *)buf_map_back_msg(ipc_data.recv);
        const uint16 msg_length = buf_get_back_msg_len(ipc_data.recv);

        /* The header always comes first so we can always cast to it */
        IPC_SIGNAL_ID id = ((const IPC_HEADER *)msg)->id;

        /* Mark the message as "read" immediately so that any recursive calls
         * see the buffer in a consistent state.
         */
        buf_update_back(ipc_data.recv);

        switch(id)
        {
        /* ************************************************************** */
        /* *                 Messages with fixed handlers                 */
        /* ************************************************************** */
        case IPC_SIGNAL_ID_BLUESTACK_PRIM:
            ipc_bluestack_handler(id, msg);
            break;
        case IPC_SIGNAL_ID_TEST_TUNNEL_PRIM:
            ipc_test_tunnel_handler(id, msg, msg_length);
            break;
        case IPC_SIGNAL_ID_SCHED_MSG_PRIM:
            ipc_sched_handler(id, msg);
            break;
        case IPC_SIGNAL_ID_PFREE:
            ipc_malloc_msg_handler(id, msg);
            break;
        case IPC_SIGNAL_ID_APP_MSG:
        case IPC_SIGNAL_ID_APP_SINK_SOURCE_MSG:
        case IPC_SIGNAL_ID_APP_MSG_TO_HANDLER:
        {
            /* These signals are special: they *may* invoke high-priority
             * processing code.  If they wanted to but couldn't because an
             * atomic response is pending, they will return False, in which case
             * we need to store the message until the atomic response has been
             * seen, at which point we call the handler again.
             * NOTE:
             * The high-priority handler can request ipc atomic receive itself by
             * calling fast traps which can lead to recursion of ipc_recv, and ipc
             * messages not being freed until the handler execution completes. This
             * can fill up ipc message queue and stop ipc messaging.
             * Hence, we copy the message into a pmalloc block, free ipc message
             * entry and pass pmalloc block message to high-priority handler..
             */
            void *app_msg = pmalloc(msg_length);
            memcpy(app_msg, msg, msg_length);
            ipc_recv_update_behind(&msg_freed);

            if (!ipc_trap_api_handler(id, app_msg, msg_length))
            {
                assert(ipc_disallow_high_priority_handler_calls()); /* sanity check */
                ipc_queue_msg_core(&ipc_data.postponed_hp_triggers, id, app_msg,
                                   msg_length);
            }
            pfree(app_msg);
        }
            break;
        case IPC_SIGNAL_ID_IPC_LEAVE_RECV_BUFFER_PAGES_MAPPED:
            ipc_leave_recv_buffer_pages_mapped();
            break;
        case IPC_SIGNAL_ID_SIGNAL_INTERPROC_EVENT:
            ipc_signal_interproc_event_handler(id, msg);
            break;
        case IPC_SIGNAL_ID_STREAM_DESTROYED:
        case IPC_SIGNAL_ID_OPERATORS_DESTROYED:
            ipc_stream_handler(id, msg);
            break;

        case IPC_SIGNAL_ID_MEMORY_ACCESS_FAULT_INFO:
            ipc_memory_access_fault_handler(id, msg);
            break;
        case IPC_SIGNAL_ID_TRAP_API_VERSION:
            ipc_trap_api_version_prim_handler(id, msg);
            break;




        /* ************************************************************** */
        /* *              Messages with dynamic handlers                  */
        /* ************************************************************** */
        default:
            /* Is there a dynamic callback for this message?  (This also
             * handles populating the blocking_msg if appropriate) */
            got_blocking = FALSE;
            if (call_msg_callback(id, msg, msg_length, FALSE,
                                  &blocking_msg, &got_blocking))
            {
                if (got_blocking)
                {
                    /* We saw the blocking message, so we can return it */
                    returned_msg = blocking_msg;
                    ipc_data.atomic_rsp_pending = FALSE;
                }
            }
            else
            {
                /* The autogenerated signals are numbered by the corresponding
                 * trap ID, which contains the trapset index in the upper word,
                 * and trapsets are numbered from 1.  Hence this is the lowest
                 * possible number for an autogenerated signal */
                if (id >= 0x10000)
                {
                    /* This is an autogenerated trap API prim */
                        (void)ipc_trap_api_handler(id, msg, msg_length);
                }
                else
                {
                    panic_diatribe(PANIC_IPC_UNHANDLED_MESSAGE_ID, id);
                }
            }
            break;
        }

        if (!ipc_disallow_high_priority_handler_calls())
        {
            /* Any trigger messages that have been blocked because they were
             * received either while an atomic response was pending or inside
             * a nested call to ipc_recv (or both) can now be run. */
            ipc_recv_process_postponed_hp_handlers();
        }

        /* Free ipc message as it has already been processed in
         * message handler called above */
        ipc_recv_update_behind(&msg_freed);


        /* This only records the number of messages processed at nest_level 0,
         * but that's probably good enough because it'll be relatively rare to
         * process messages at level 1, and this is just for a "get out of the
         * loop after a while" check. */
        n_processed++;
    }

    /* Running handlers may have resulted in a recursive call to ipc_recv,
     * meaning that we may have got the blocking response message via that
     * route.  So we check the out_of_order_rsps here */
    if (ipc_recv_process_out_of_order_rsps(&blocking_msg))
    {
        returned_msg = blocking_msg;
    }

    /* Reschedule ourselves if there's anything left to process */
    if (BUF_ANY_MSGS_TO_SEND(ipc_data.recv))
    {
        ipc_data.pending = TRUE;
        GEN_BG_INT(ipc);
    }

    /* Return either the blocking message body or NULL, depending on whether
     * the blocking messsage was seen or not */
    return returned_msg;
}


void ipc_recv_cb(IPC_SIGNAL_ID recv_id, IPC_RECV_CB cb)
{
    /* Place the callback at the end of the queue */
    IPC_RECV_CB_QUEUE *new_cb = pnew(IPC_RECV_CB_QUEUE);
    IPC_RECV_CB_QUEUE **pnext = &ipc_data.recv_cb;

    new_cb->recv_id = recv_id;
    new_cb->cb = cb;
    new_cb->next = NULL;
    new_cb->nest_level = ipc_data.nest_level;

    while(*pnext != NULL)
    {
        pnext = &((*pnext)->next);
    }
    *pnext = new_cb;
}

bool ipc_disallow_high_priority_handler_calls(void)
{
    return ipc_data.atomic_rsp_pending || ipc_data.nest_level > 0;
}

void ipc_high_priority_handler_running(bool start_not_stop)
{
    if (start_not_stop)
    {
        ipc_data.nest_level += 1;
    }
    else
    {
        ipc_data.nest_level -= 1;
    }
}
