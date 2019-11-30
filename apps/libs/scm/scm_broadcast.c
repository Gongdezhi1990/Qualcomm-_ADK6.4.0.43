/* Copyright (c) 2015 - 2019 Qualcomm Technologies International, Ltd. */
/*  */

#include "scm_private.h"
#include "scm.h"

#include <panic.h>
#include <connection.h>
#include <message.h>
#include <stdlib.h>

/* This macro is used only for SCM broadcast role, to flag pending reset request */
#define SCM_BROADCAST_PENDING_INTERNAL_RESET SCM_NUM_SEQ_NUMS

/*! @brief Send SCM_BROADCAST_INIT_CFM message to client task, */
static void scm_msg_client_init_cfm(Task client_task, SCMB *lib,
                                    scm_status status)
{
    MESSAGE_MAKE(message, SCM_BROADCAST_INIT_CFM_T);
    message->scm = lib;
    message->status = status;
    MessageSend(client_task, SCM_BROADCAST_INIT_CFM, message);
}

/*! @brief Send SCM_BROADCAST_REGISTER_REQ message to transport task */
static void scm_msg_transport_register_req(Task transport_task, SCMB *lib)
{
    MESSAGE_MAKE(message, SCM_BROADCAST_TRANSPORT_REGISTER_REQ_T);
    message->scm = lib;
    MessageSend(transport_task, SCM_BROADCAST_TRANSPORT_REGISTER_REQ, message);
}

/*! @brief Send SCM_BROADCAST_UNREGISTER_REQ message to transport task */
static void scm_msg_transport_unregister_req(SCMB *lib)
{
    MESSAGE_MAKE(message, SCM_BROADCAST_TRANSPORT_UNREGISTER_REQ_T);
    message->scm = lib;
    MessageSend(lib->transport_task, SCM_BROADCAST_TRANSPORT_UNREGISTER_REQ, message);
}

/*! @brief Send SCM_BROADCAST_ENABLE_CFM message to client task, */
static void scm_msg_client_broadcast_enable_cfm(Task client_task, SCMB* lib, scm_status status)
{
    MESSAGE_MAKE(message, SCM_BROADCAST_ENABLE_CFM_T);
    message->scm = lib;
    message->status = status;
    MessageSend(client_task, SCM_BROADCAST_ENABLE_CFM, message);
}

/*! @brief Send SCM_BROADCAST_DISABLE_CFM message to client task, */
static void scm_msg_client_broadcast_disable_cfm(Task client_task, SCMB* lib, scm_status status)
{
    MESSAGE_MAKE(message, SCM_BROADCAST_DISABLE_CFM_T);
    message->scm = lib;
    message->status = status;
    /* Wait till all the internal pending tasks are completed, only then send the cfm */
    MessageSendConditionally(client_task, SCM_BROADCAST_DISABLE_CFM, message, (uint16*)&lib->pending_internal_tasks);
}

/*!
    @brief Set the internal task pending mask.
*/
static void setPendingInternalTask(SCMB* lib, uint32 offset)
{
    lib->pending_internal_tasks |= ((uint32)1 << offset);
}

/*!
    @brief Clear the internal task pending mask.
*/
static void clearPendingInternalTask(SCMB* lib, uint32 offset)
{
    lib->pending_internal_tasks &= ~((uint32)1 << offset);
}

/*!
    @brief Create a segment header for message.
*/
static uint8_t createSegmentHeader(SCMB *scm, uint8 seq_num, uint8_t seg_num, bool more_data)
{
    UNUSED(scm);
    return (seq_num << 4) | (seg_num << 1) | (more_data ? 1 : 0);
}

/*!
    @brief Allocate a sequence number from the pool.
*/
static bool allocateSequenceNumber(SCMB *scm, uint8_t *seq_num_p)
{
    int index;
    for (index = 0; index < SCM_NUM_SEQ_NUMS; index++)
    {
        /* Calculate sequence number to try, starting with number after
           previously allocated number */
        int seq_num = (scm->alloc_seq_num + index) % SCM_NUM_SEQ_NUMS;

        /* Check if this number is available */
        if (scm->seq_num_pool & (1 << seq_num))
        {
            SCM_DEBUG(("SCM::allocateSequenceNumber %u\n", seq_num));

            /* Mark it as allocated */
            scm->seq_num_pool &= ~(1 << seq_num);

            /* Try to allocate next sequence number next time */
            scm->alloc_seq_num = (seq_num + 1) % SCM_NUM_SEQ_NUMS;

            /* Store allocated sequence number */
            *seq_num_p = seq_num;
            return TRUE;
        }
    }

    return FALSE;
}

/*!
    @brief Add a sequence number in the pool.
*/
static void addSequenceNumber(SCMB *scm, uint8_t seq_num)
{
    SCM_DEBUG(("SCM::addSequenceNumber %u\n", seq_num));
    scm->seq_num_pool |= (1 << seq_num);
}

/*!
    @brief Find a segment to transmit.
*/
static void handleInternalKickReq(SCMB *scm)
{
    SCM_INTERNAL_BROADCAST_MSG_T *msg_ptr;
    bool seg_sent;

    if (!scm->transport_task)
    {
        SCM_DEBUG(("SCM::handleInternalKickReq, not enabled\n"));
        return;
    }

    SCM_DEBUG(("SCM::handleInternalKickReq\n"));

    /* Walk down the the list finding a segment to send */
    msg_ptr = scm->msg_list;
    seg_sent = FALSE;
    while (msg_ptr && (scm->segments_pending < SCM_MAX_SEGMENTS))
    {
        /* Only consider segments that don't have all segments pending */
        if (msg_ptr->segment_pending_set != 0xFFFF)
        {
            uint8 seg_num;
            uint16 seg_mask;
            uint8 seg_offset;
            
            MAKE_MESSAGE(SCM_BROADCAST_SEGMENT_REQ);

             /* Attempt to allocate sequence number if not previously allocated */
            if (msg_ptr->sequence_number == SCM_SEQ_NUM_INVALID)
            {
                if (allocateSequenceNumber(scm, &msg_ptr->sequence_number))
                {
                    /* Initialise segment pending and sent sets */
                    const uint16 num_segments = (msg_ptr->data_size + (SCM_SEGMENT_DATA_SIZE - 1)) / SCM_SEGMENT_DATA_SIZE;
                    msg_ptr->segment_pending_set = (0xFFFFU << num_segments);
                    msg_ptr->segment_sent_set = (0xFFFFU << num_segments); 
                }
                else
                {
                    /* No sequence number available, exit */
                    break;
                }
            }

            /* Get lowest unset bit in segment pending set */
            seg_num = 0; seg_mask = 1;
            while (msg_ptr->segment_pending_set & seg_mask)
            {
                seg_num++;
                seg_mask = seg_mask << 1;
            }
            
            /* Create message */
            message->scm = scm;
            message->num_transmissions = msg_ptr->num_transmissions;
            seg_offset = seg_num * SCM_SEGMENT_DATA_SIZE;
            if (msg_ptr->data_size - seg_offset > SCM_SEGMENT_DATA_SIZE)
            {
                message->header = createSegmentHeader(scm, msg_ptr->sequence_number, seg_num, TRUE);
                memcpy(message->data, &msg_ptr->data[seg_offset], SCM_SEGMENT_DATA_SIZE);
            } 
            else
            {
                message->header = createSegmentHeader(scm, msg_ptr->sequence_number, seg_num, FALSE);
                memcpy(message->data, &msg_ptr->data[seg_offset], msg_ptr->data_size - seg_offset);
            }

            /* Pass segment to transport task for transmission */
            MessageSend(scm->transport_task, SCM_BROADCAST_SEGMENT_REQ, message);
            scm->segments_pending++;
            
            /* Set bit in segment pending set */
            msg_ptr->segment_pending_set |= seg_mask;
            
            /* Set segment_set flag */
            seg_sent = TRUE;
        }
        
        /* Move to next message */
        msg_ptr = msg_ptr->next;
        if (!msg_ptr && seg_sent)
        {
            seg_sent = FALSE;
            msg_ptr = scm->msg_list;
        }
    }
}

/*!
    Reset message segment sets
*/
static void resetMessage(SCMB *scm, SCM_INTERNAL_BROADCAST_MSG_T *msg_ptr)
{
    UNUSED(scm);
    msg_ptr->segment_pending_set = 0;
    msg_ptr->segment_sent_set = 0;
}

/*!
    clear message list
*/
static void clearMessage(SCM_INTERNAL_BROADCAST_MSG_T **msg_list, SCMB *scm)
{
    SCM_INTERNAL_BROADCAST_MSG_T *msg_del_ptr = *msg_list;
    SCM_INTERNAL_BROADCAST_MSG_T *msg_next_ptr;


    /* Walk down the the list and free the message pointers */
    while(msg_del_ptr != NULL)
    {
        msg_next_ptr = msg_del_ptr->next;
        if (msg_del_ptr->sequence_number != SCM_SEQ_NUM_INVALID)
        {
            /* Application needs to know that broadcast message wasn't successfully sent */
            MAKE_MESSAGE(SCM_BROADCAST_MESSAGE_CFM);
            /* Send confirmation back to application */
            message->scm = scm;
            message->handle = msg_del_ptr->handle;
            message->data = NULL;
            message->size_data = 0;
            MessageSend(scm->client_task, SCM_BROADCAST_MESSAGE_CFM, message);

            /* Remember the pending internal task to add the sequence number back to the queue */
            setPendingInternalTask(scm, msg_del_ptr->sequence_number);
            /* Send message to ourselves after 1 second to add sequence number
               back into pool to be re-used, so that receiver's can flush out partially segmented packets */
            MessageSendLater(&scm->lib_task, SCM_INTERNAL_SEQ_ADD_TO_POOL_BASE_IND + msg_del_ptr->sequence_number, NULL, D_SEC(1));
        }
        /* Remove from list and free */
        free((uint8 *)msg_del_ptr->data);
        free(msg_del_ptr);
        msg_del_ptr = msg_next_ptr;
    }
    /* deref the head node */
    *msg_list = NULL;
}

static void resetScmBroadcast(SCMB *scm)
{
    if(scm->msg_list)
    {
        clearMessage(&scm->msg_list, scm);
        /* reset the SCM structure */
        scm->segments_pending = 0;
    }
}

/*!
    @brief Reset message transmission.
*/
static void handleInternalResetReq(SCMB *scm)
{
    SCM_INTERNAL_BROADCAST_MSG_T *msg_p;

    SCM_DEBUG(("SCM::handleInternalResetReq\n"));

    /* Walk down the the list resetting messages */
    for (msg_p = scm->msg_list; msg_p != NULL; msg_p = msg_p->next)
    {
        if (msg_p->sequence_number != SCM_SEQ_NUM_INVALID)
        {
            /* Remember the pending internal task to add the sequence number back to the queue */
            setPendingInternalTask(scm, msg_p->sequence_number);
            /* Send message to ourselves after 1 second to add sequence number
               back into pool to be re-used */
            MessageSendLater(&scm->lib_task, SCM_INTERNAL_SEQ_ADD_TO_POOL_BASE_IND + msg_p->sequence_number, NULL, D_SEC(1));

            /* Reset message, so that it starts from first segment again */
            resetMessage(scm, msg_p);
        }
    }
}

/*!
    @brief Handler for SCM library messages.
*/
static void scm_msg_handler(Task task, MessageId id, Message message)
{
    UNUSED(message);

    if ((id >= SCM_INTERNAL_MSG_BASE) &&
        (id < SCM_INTERNAL_MSG_TOP))
    {
        /* Work out which library instance this is for */
        SCMB *scm = (SCMB *)task;

        switch (id)
        {
            /* Request to attempt to send another segment */
            case SCM_INTERNAL_KICK_REQ:
                handleInternalKickReq(scm);
                break;

            case SCM_INTERNAL_RESET_REQ:
                handleInternalResetReq(scm);
                /* clear the pending flag */
                clearPendingInternalTask(scm, SCM_BROADCAST_PENDING_INTERNAL_RESET);
                break;

            default:
                /* Timed message to add sequence back to pool */
                if ((id >= SCM_INTERNAL_SEQ_ADD_TO_POOL_BASE_IND) &&
                    (id <= SCM_INTERNAL_SEQ_ADD_TO_POOL_TOP_IND))
                {
                    addSequenceNumber(scm, id - SCM_INTERNAL_SEQ_ADD_TO_POOL_BASE_IND);
                    clearPendingInternalTask(scm, id - SCM_INTERNAL_SEQ_ADD_TO_POOL_BASE_IND);
                    handleInternalKickReq(scm);
                }
                break;
        }
    }
    else
    {
        Panic();
    }
}

/*!
    @brief Create an instance of the SCM broadcast library.
*/
void ScmBroadcastInit(Task app_task)
{
    SCMB *new_lib = NULL;

    /* create and setup a new SCM library instance */
    new_lib = PanicUnlessMalloc(sizeof(*new_lib));
    if (new_lib)
    {
        memset(new_lib, 0, sizeof(*new_lib));
        new_lib->lib_task.handler = scm_msg_handler;
        new_lib->client_task = app_task;
        new_lib->transport_task = NULL;
        new_lib->seq_num_pool = 0xFFFF;
        new_lib->segments_pending = 0;
        new_lib->next_handle = 0x8001;
        new_lib->pending_internal_tasks = 0;

        /* Initialisation complete, tell the client */
        scm_msg_client_init_cfm(app_task, new_lib, scm_success);
    }
    else
    {
        scm_msg_client_init_cfm(app_task, NULL, scm_init_failed);
    }
}

/*! @brief Destroy an instance of the SCM broadcast library. */
void ScmBroadcastDestroy(SCMB *scm)
{
    SCM_INTERNAL_BROADCAST_MSG_T **msg_pp, *msg_ptr;

    PanicNull(scm);

    PanicNotNull(scm->transport_task);

    /* Free any pending scm messages. */
    for (msg_pp = &scm->msg_list; (msg_ptr = *msg_pp) != NULL; msg_pp = &msg_ptr->next)
    {
        *msg_pp = msg_ptr->next;
        free(msg_ptr);
    }
    scm->msg_list = NULL;

    /* Flush any pending internal messages. */
    MessageFlushTask(&scm->lib_task);

    free(scm);
}

/*! @brief Confirmation by SCM transport that it is available for use. */
void ScmBroadcastTransportRegisterCfm(SCMB *scm, Task transport_task)
{
    /* Store the transport_task, we can now send SCM messages, using it */
    scm->transport_task = transport_task;

    /* Send internal message to kick segment transmission */
    MessageSend(&scm->lib_task, SCM_INTERNAL_KICK_REQ, NULL);
    MessageCancelAll(&scm->lib_task, SCM_INTERNAL_RESET_REQ);

    /* Report to client that SCM is now enabled. */
    scm_msg_client_broadcast_enable_cfm(scm->client_task, scm, scm_success);
}

/*! @brief Confirmation by SCM transport that it has returned any state and is no longer in use. */
void ScmBroadcastTransportUnRegisterCfm(SCMB *scm)
{
    /* clear the transport_task, will prevent any further SCM message TX */
    scm->transport_task = NULL;

    /* Send a delayed message to reset all messages after 250ms */
    MessageSendLater(&scm->lib_task, SCM_INTERNAL_RESET_REQ, NULL, 250);
    /* Flag the pending reset request, handle it and then send SCM_BROADCAST_DISABLE_CFM to client */
    setPendingInternalTask(scm, SCM_BROADCAST_PENDING_INTERNAL_RESET);
    MessageCancelAll(&scm->lib_task, SCM_INTERNAL_KICK_REQ);

    /* Report to client that SCM is now disabled. */
    scm_msg_client_broadcast_disable_cfm(scm->client_task, scm, scm_success);
}

/*!
    @brief Request broadcast of message.
*/
SCM_HANDLE ScmBroadcastMessageReq(SCMB *scm, const uint8 *data, uint16 size_data, uint16 num_transmissions)
{
    SCM_INTERNAL_BROADCAST_MSG_T *msg_ptr = PanicUnlessMalloc(sizeof(SCM_INTERNAL_BROADCAST_MSG_T));
    SCM_INTERNAL_BROADCAST_MSG_T **msg_pp;

    SCM_DEBUG(("ScmBroadcastMessageReq\n"));

    /* API cannot be used unless we have a transport */
    if (!scm->transport_task)
    {
        free(msg_ptr);
        return (SCM_HANDLE)0;
    }

    /* Create new broadcast message structure */
    msg_ptr->next = NULL;
    msg_ptr->data_size = size_data;
    msg_ptr->data = data;
    msg_ptr->sequence_number = SCM_SEQ_NUM_INVALID;
    msg_ptr->num_transmissions = num_transmissions;
    msg_ptr->handle = scm->next_handle;
    scm->next_handle = (scm->next_handle + 1) | 0x8000;

    /* Reset transmission state */
    resetMessage(scm, msg_ptr);
    
    /* Add to end of list */
    for (msg_pp = &scm->msg_list; *msg_pp != NULL; msg_pp = &(*msg_pp)->next);
    *msg_pp = msg_ptr;

    /* Send internal message to kick segment transmission */
    MessageSend(&scm->lib_task, SCM_INTERNAL_KICK_REQ, NULL);

    /* Return message handle */
    return msg_ptr->handle;
}

/*!
    @brief Enable/disable broadcasting.
*/
void ScmBroadcastEnable(SCMB *scm, Task transport_task)
{
    SCM_DEBUG(("ScmBroadcastEnable\n"));

    /* Ensure we're not already enabled, i.e. we don't have a transport_task */
    if (!scm->transport_task)
    {
        /* API must provide transport if enabling */
        PanicNull(transport_task);

        /* Tell the transport we exist */
        scm_msg_transport_register_req(transport_task, scm);
    }       
}

/*!
    @brief Disable broadcasting SCM messages.

    @param scm [IN] SCM broadcaster instance.
*/
void ScmBroadcastDisable(SCMB *scm)
{
    SCM_DEBUG(("ScmBroadcastDisable\n"));

    /* ensure we're already enabled, i.e. we have a transport_task */
    if (scm->transport_task)
    {
        /* application wants to disable SCM, so clear the pending message list */
        resetScmBroadcast(scm);
        /* Tell the transport we no longer want it's services and to
         * send back any state for us to hold onto */
        scm_msg_transport_unregister_req(scm);
    }
    else
    {
        /* Report to client that SCM was not enabled at the first place to disable it */
        scm_msg_client_broadcast_disable_cfm(scm->client_task, scm, scm_failed);
    }
}

/*!
    @brief Confirm segment transmission.
*/
void ScmBroadcastSegmentCfm(SCMB *scm, uint16 header, uint16 num_transmissions)
{
    SCM_INTERNAL_BROADCAST_MSG_T **msg_pp, *msg_ptr;

    /* One less segment is being transmitted */
    if(scm->segments_pending)
    {
        scm->segments_pending--;

        SCM_DEBUG(("ScmBroadcastSegmentCfm, num_pending %d\n", scm->segments_pending));

        /* API should not be used until initialisation is complete */
        PanicNull(scm->transport_task);

        /* Walk down the the list finding the matching message */
        for (msg_pp = &scm->msg_list; (msg_ptr = *msg_pp) != NULL; msg_pp = &msg_ptr->next)
        {
            /* Only consider segments that are pending confirmation */
            if (msg_ptr->segment_pending_set != msg_ptr->segment_sent_set)
            {
                uint8 seq_num = (header >> 4) & 0x0F;
                uint8 seg_num = (header >> 1) & 0x07;

                /* Check sequence numbers match */
                if (msg_ptr->sequence_number == seq_num)
                {
                    SCM_DEBUG(("ScmBroadcastSegmentCfm, match found\n"));
                        
                    if (num_transmissions == 0)
                    {
                        /* Set bit in segment sent set */
                        msg_ptr->segment_sent_set |= (1U << seg_num);
                        
                        /* Check if all segments have been sent */
                        if (msg_ptr->segment_sent_set == 0xFFFF)
                        {
                            MAKE_MESSAGE(SCM_BROADCAST_MESSAGE_CFM);

                            /* Send confirmation back to application */
                            message->scm = scm;
                            message->handle = msg_ptr->handle;
                            message->data = msg_ptr->data;
                            message->size_data = msg_ptr->data_size;
                            MessageSend(scm->client_task, SCM_BROADCAST_MESSAGE_CFM, message);

                            /* Remember the pending internal task to add the sequence number back to the queue */
                            setPendingInternalTask(scm, msg_ptr->sequence_number);
                            /* Send message to ourselves after 1 second to add sequence number
                               back into pool to be re-used */
                            MessageSendLater(&scm->lib_task, SCM_INTERNAL_SEQ_ADD_TO_POOL_BASE_IND + msg_ptr->sequence_number, NULL, D_SEC(1));

                            /* Remove from list and free */
                            *msg_pp = msg_ptr->next;
                            free(msg_ptr);
                        }
                    }
                    else
                    {
                        /* Clear bit in segment pending set */
                        msg_ptr->segment_pending_set &= ~(1U << seg_num);                    
                    }

                    /* Send internal message to kick segment transmission */
                    MessageSend(&scm->lib_task, SCM_INTERNAL_KICK_REQ, NULL);

                    /* Finish searching */
                    break;
                }
            }
        }
    }
}

/*!
    @brief Determine if the SCM library still has messages in progress.
*/
bool ScmBroadcastIsBusy(SCMB *scm)
{
    return scm->msg_list ? TRUE : FALSE;
}

