/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

#include "scm_private.h"
#include "scm.h"

#include <panic.h>
#include <connection.h>
#include <message.h>
#include <stdlib.h>

/*! @brief Send SCM_RECEIVER_INIT_CFM message to client task */
static void scm_msg_client_init_cfm(Task client_task, SCMR *lib,
                                    scm_status status)
{
    MESSAGE_MAKE(message, SCM_RECEIVER_INIT_CFM_T);
    message->scm = lib;
    message->status = status;
    MessageSend(client_task, SCM_RECEIVER_INIT_CFM, message);
}

/*! @brief Send SCM_RECEIVER_REGISTER_REQ message to transport task */
static void scm_msg_transport_register_req(Task transport_task, SCMR *lib)
{
    MESSAGE_MAKE(message, SCM_RECEIVER_TRANSPORT_REGISTER_REQ_T);
    message->scm = lib;
    MessageSend(transport_task, SCM_RECEIVER_TRANSPORT_REGISTER_REQ, message);
}

/*! @brief Send SCM_RECEIVER_UNREGISTER_REQ message to transport task */
static void scm_msg_transport_unregister_req(SCMR *lib)
{
    MESSAGE_MAKE(message, SCM_RECEIVER_TRANSPORT_UNREGISTER_REQ_T);
    message->scm = lib;
    MessageSend(lib->transport_task, SCM_RECEIVER_TRANSPORT_UNREGISTER_REQ, message);
}

/*! @brief Send SCM_RECEIVER_ENABLE_CFM message to client task, */
static void scm_msg_client_receiver_enable_cfm(Task client_task, SCMR* lib, scm_status status)
{
    MESSAGE_MAKE(message, SCM_RECEIVER_ENABLE_CFM_T);
    message->scm = lib;
    message->status = status;
    MessageSend(client_task, SCM_RECEIVER_ENABLE_CFM, message);
}

/*! @brief Send SCM_RECEIVER_DISABLE_CFM message to client task, */
static void scm_msg_client_receiver_disable_cfm(Task client_task, SCMR* lib, scm_status status)
{
    MESSAGE_MAKE(message, SCM_RECEIVER_DISABLE_CFM_T);
    message->scm = lib;
    message->status = status;
    MessageSend(client_task, SCM_RECEIVER_DISABLE_CFM, message);
}

/*!
    @brief Handler for SCM library messages.
*/
static void scm_msg_handler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(message);

    if ((id >= SCM_INTERNAL_MSG_BASE) &&
        (id < SCM_INTERNAL_MSG_TOP))
    {
        /* Work out which library instance this is for */
        /*SCMR *scm = (SCMR *)task;*/

        switch (id)
        {
            default:
                break;
        }
    }
    else
    {
        Panic();
    }
}

/*!
    @brief Create an instance of the SCM receiver library.
*/
void ScmReceiverInit(Task app_task)
{
    SCMR *new_lib = NULL;

    /* create and setup a new SCM library instance */
    new_lib = PanicUnlessMalloc(sizeof(*new_lib));
    if (new_lib)
    {
        memset(new_lib, 0, sizeof(*new_lib));
        new_lib->lib_task.handler = scm_msg_handler;
        new_lib->client_task = app_task;

        /* Initialisation complete, tell the client */
        scm_msg_client_init_cfm(app_task, new_lib, scm_success);
    }
    else
    {
        scm_msg_client_init_cfm(app_task, NULL, scm_init_failed);
    }
}

/*! @brief Destroy an instance of the SCM receiver library. */
void ScmReceiverDestroy(SCMR *scm)
{
    PanicNull(scm);

    PanicNotNull(scm->transport_task);

    /* Clean the internal state. */
    ScmReceiverReset(scm);

    /* Flush any pending internal messages. */
    MessageFlushTask(&scm->lib_task);

    free(scm);
}

void ScmReceiverTransportRegisterCfm(SCMR *scm, Task transport_task)
{
    scm->transport_task = transport_task;
    scm_msg_client_receiver_enable_cfm(scm->client_task, scm, scm_success);
}

/*! @brief Confirmation by SCM transport that it has returned any state and is no longer in use. */
void ScmReceiverTransportUnRegisterCfm(SCMR *scm)
{
    /* clear the transport task, will prevent further SCM message RX */
    scm->transport_task = NULL;

    /* clear our state - transport will have already cleared all per-segment expiration via
     * callback */
    ScmReceiverReset(scm);

    /* send confirmation to client that SCM receiver is now disabled */
    scm_msg_client_receiver_disable_cfm(scm->client_task, scm, scm_success);
}

/*!
    @brief Enable receiving SCM messages.
*/
void ScmReceiverEnable(SCMR *scm, Task transport_task)
{
    if (!scm->transport_task)
    {
        /* API must provide transport if enabling */
        PanicNull(transport_task);

        /* Tell the transport we exist */
        scm_msg_transport_register_req(transport_task, scm);
    }
}

/*!
    @brief Disable receiving SCM messages.
*/
void ScmReceiverDisable(SCMR *scm)
{
    /* unregister the transport */
    if (scm->transport_task)
    {
        scm_msg_transport_unregister_req(scm); 
    }
	else
    {
        /* Report to client that SCM was not enabled at the first place to disable it */
        scm_msg_client_receiver_disable_cfm(scm->client_task, scm, scm_failed);
    }
}

/*!
    @brief Reset receiver state, clear any partial messages.
*/
void ScmReceiverReset(SCMR *scm)
{
    SCM_INTERNAL_RECEIVE_MSG_T *msg_p = scm->msg_list;
    
    /* Free all partially reassembled messages */
    while (msg_p)
    {
        SCM_INTERNAL_RECEIVE_MSG_T *free_msg_p = msg_p;
        SCM_DEBUG(("SCM::ScmReceiverReset, freeing message seq_num %d, seg_expired_set %04x, seg_received_set %04x\n",
                   msg_p->sequence_number, msg_p->segment_expired_set, msg_p->segment_received_set));
        msg_p = msg_p->next;
        free(free_msg_p);        
    }
    
    /* Make sure list is NULL */
    scm->msg_list = NULL;
}

/*!
    @brief Handle received message segment
*/
void ScmReceiverSegmentInd(SCMR *scm, uint16 header, const uint8 *data)
{
    SCM_INTERNAL_RECEIVE_MSG_T *msg_p;
    uint8 seq_num = (header >> 4) & 0x0F;

    /* Walk down the the list finding a message matching this segment */
    for (msg_p = scm->msg_list; msg_p != NULL; msg_p = msg_p->next)
    {
        if (msg_p->sequence_number == seq_num)
        {
            SCM_DEBUG(("SCM::ScmReceiverSegmentInd, found seq_num %d\n", seq_num));
            break;
        }
    }

    /* No match, this is a segment from a new message */
    if (!msg_p)
    {
        msg_p = malloc(sizeof(*msg_p));
        if (msg_p != NULL)
        {
            msg_p->sequence_number = seq_num;
            msg_p->segment_received_set = 0;
            msg_p->segment_expired_set = 0;
            msg_p->data_size = 0;
            msg_p->data = NULL;
    
            SCM_DEBUG(("SCM::ScmReceiverSegmentInd, new message seq_num %d, seg_num %d\n", seq_num, seq_num));
    
            /* Add to queue of message being reassembled */
            msg_p->next = scm->msg_list;
            scm->msg_list = msg_p;    
        }
    }

    /* Check we now have message and we haven't received all the segments yet */
    if (msg_p && (msg_p->segment_received_set != 0xFFFF))
    {
        uint8 more_data = header & 0x01;
        uint8 seg_num = (header >> 1) & 0x07;
        uint16 data_size;

        /* If more_data bit is not set, we now know how many segments we are
           expecting, so set the segment_set correctly.  Once we've got all
           segments, segment_set will be 0xFFFF */
        if (!more_data)
        {
            msg_p->segment_received_set |= (uint16_t)(0xFFFE << seg_num);
            msg_p->segment_expired_set |= (uint16_t)(0xFFFE << seg_num);
        }

        /* Resize message as required as more segments arrive */
        data_size = (seg_num * SCM_SEGMENT_DATA_SIZE + SCM_SEGMENT_DATA_SIZE);
        if (data_size > msg_p->data_size)
        {
            uint8 *msg_data = realloc(msg_p->data, data_size);
            if (msg_data != NULL)
            {
                msg_p->data = msg_data;
                msg_p->data_size = data_size;
            }
            else            
                return;
        }

        /* Copy segment data into message */
        memcpy(&msg_p->data[seg_num * SCM_SEGMENT_DATA_SIZE], data, SCM_SEGMENT_DATA_SIZE);

        /* Add this segment into set of those received */
        msg_p->segment_received_set |= (1 << seg_num);

        SCM_DEBUG(("SCM::ScmReceiverSegmentInd, seg_num %d, seg_received_set %04x\n", seg_num, msg_p->segment_received_set));

        /* Check we have all the segment we need */
        if (msg_p->segment_received_set == 0xFFFF)
        {
            MAKE_MESSAGE(SCM_RECEIVER_MESSAGE_IND);

            /* Bingo, send received message to client */
            message->scm = scm;
            message->data = msg_p->data;
            message->size_data = msg_p->data_size;
            MessageSend(scm->client_task, SCM_RECEIVER_MESSAGE_IND, message);

            /* Remove data from message.  Keep message on list until all segments expire */
            msg_p->data = NULL;
            msg_p->data_size = 0;
        }
    }
}

/*!
    @brief Handle expiration of message segment
*/
void ScmReceiverSegmentExpiredInd(SCMR *scm, uint16 header)
{
    SCM_INTERNAL_RECEIVE_MSG_T **msg_pp, *msg_p;
    uint8 seq_num = (header >> 4) & 0x0F;

    /* Search for message that contains this segment */
    for (msg_pp = &scm->msg_list; (msg_p = *msg_pp) != NULL; msg_pp = &msg_p->next)
    {
        if (msg_p->sequence_number == seq_num)
        {
            uint8 seg_num = (header >> 1) & 0x07;

            /* Mark this segment as expired */
            msg_p->segment_expired_set |= (1 << seg_num);

            SCM_DEBUG(("SCM::ScmReceiverSegmentExpiredInd, seg_num %d, seg_expired_set %04x, seg_received_set %04x\n",
                       seg_num, msg_p->segment_expired_set, msg_p->segment_received_set));
            
            /* Check if all segments we have received have expired now */
            if (msg_p->segment_expired_set == msg_p->segment_received_set)
            {
                /* Remove from list and free */
                *msg_pp = msg_p->next;
                free(msg_p->data);
                free(msg_p);                        
            }

            /* Finished searching */
            break;
        }
    }
}
