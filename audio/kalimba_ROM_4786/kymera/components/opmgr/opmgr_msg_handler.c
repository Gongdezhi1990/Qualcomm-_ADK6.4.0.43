/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  opmgr.c
 * \ingroup  opmgr
 *
 * Operator Manager main file. <br>
 * This file contains the operator manager functionality for handling messages
 * received by the operator manager from its operators.<br>
 */

/****************************************************************************
Include Files
*/
#include "opmgr_private.h"

/*****************************************************************************
Private Macros
*/
#define GET_OPID_FROM_SRC_ID(id) \
           ((id) & ~CONID_PACKED_RECV_PROC_ID_MASK)

#define GET_SRC_ID_FROM_PACKED_SRC_ID(id) \
           (GET_RECV_PROC_ID(id) << CONID_PROCESSOR_ID_SHIFT) 

/*****************************************************************************
Private Function Definitions
*/

/**
 * \brief Function is used to recover from a failure. It frees the data and signals
 * the failure trough the callback function.
 *
 * \param  data pointer to the data object containing multi request.
 * \param  rinfo pointer to the routing information that came with the message
 * \param  msg pointer to the message payload that was received from the operator.
 * \param status status
 */
static void operator_resp_handler_send_response_and_free_data(MULTI_OP_REQ_DATA* data,
        tRoutingInfo* rinfo, OP_STD_RSP* msg, unsigned status)
{
    /* Call callback to indicate that we failed after data->cur_index operators*/
    OP_STD_LIST_CBACK callback = (OP_STD_LIST_CBACK) (data->callback);

#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
    /* The original con_id was passed on in rinfo.dest_id. (See 'send_command_to_operator_list' function) */
    callback(REVERSE_CONNECTION_ID(rinfo->dest_id), status, data->cur_index, msg->resp_data.err_code);
#else
    callback(PACK_CON_ID(rinfo->src_id, rinfo->dest_id), status, data->cur_index, msg->resp_data.err_code);
#endif
    if (data->num_ops > 1)
    {
        pfree(data->id_info.op_list);
    }
    pfree(data);
}

/**
 * \brief common handler routine for messages that may have a further list of
 * operators to service before answering the callback. If the request succeeded
 * the next operator in the list is sent a request until all have handled the
 * request at which point the callback is called to answer the original request.
 * If a request fails the callback is called to indicate the failure.
 *
 * \param  rinfo pointer to the routing information that came with the message
 * \param  msg pointer to the message payload that was received from the operator.
 * \param  req_msg_id the message id to send to the next operator in the list if
 * there is one.
 * \param  specific_handler the handler to call to do message specific work. This
 * is only called
 * \param  preproc_func preprocessing function
 */
static void operator_list_resp_handler(tRoutingInfo *rinfo, OP_STD_RSP *msg,
        OPCMD_ID req_msg_id, void (*specific_handler)(unsigned int op_id), preproc_function preproc_func)
{
    OPERATOR_DATA *cur_op;
    unsigned int op_id;

    patch_fn_shared(opmgr);

    MULTI_OP_REQ_DATA *data = opmgr_retrieve_in_progress_task(rinfo->dest_id, rinfo->src_id);

    if (data == NULL)
    {
        /* No data, panic. Passing operator id as debug information. */
        panic_diatribe(PANIC_AUDIO_OPMGR_NO_TASK_DATA, rinfo->src_id);
    }

    if (msg->status != STATUS_OK)
    {
        operator_resp_handler_send_response_and_free_data(data, rinfo, msg, STATUS_CMD_FAILED);
        return;
    }

    if (data->num_ops == 1)
    {
        op_id = EXT_TO_INT_OPID(data->id_info.id);
    }
    else
    {
        op_id = EXT_TO_INT_OPID(data->id_info.op_list[data->cur_index]);
    }

    specific_handler(op_id);

    /* That's one more of the operator list dealt with successfully. If there is
     * another one send it a message, otherwise the list is finished and can send
     * our response.*/
    data->cur_index++;
    if (data->cur_index < data->num_ops)
    {
        tRoutingInfo new_rinfo;
        /* If we get to here, num_ops must be > 1
         * so don't bother with the checks for the single-op case
         */
        PL_ASSERT(data->num_ops > 1);

        cur_op = get_op_data_from_id(EXT_TO_INT_OPID(data->id_info.op_list[data->cur_index]));
        if (NULL == cur_op)
        {
            operator_resp_handler_send_response_and_free_data(data, rinfo, msg, STATUS_CMD_FAILED);
            return;
        }

        /* Ensure that if there is something to do prior to operator getting the
         * message, it is done here.
         * Currently the pre-processing function receives operator data only.
         * This may change in future if needed. */
        if(preproc_func)
        {
            if(!preproc_func(cur_op))
            {
                operator_resp_handler_send_response_and_free_data(data, rinfo, msg, STATUS_CMD_FAILED);
                return;
            }
        }

        new_rinfo.src_id = rinfo->dest_id;
        new_rinfo.dest_id = EXT_TO_INT_OPID(data->id_info.op_list[data->cur_index]);
        if (!opmgr_store_in_progress_task(rinfo->dest_id, new_rinfo.dest_id,
                (void *)data))
        {
            operator_resp_handler_send_response_and_free_data(data, rinfo, msg, STATUS_CMD_FAILED);
            return;
        }

        put_message_with_routing(mkqid(cur_op->task_id, 1), req_msg_id,
                NULL,
                &new_rinfo);
    }
    else
    {
        operator_resp_handler_send_response_and_free_data(data, rinfo, msg, STATUS_OK);
    }

}

/**
 * \brief Specific Handler for Operator Start response messages. This informs
 * streams that an operator has started
 *
 * \param  op_id the operator id to take action for/on.
 */
static void start_resp_handler(unsigned int op_id)
{
    /* Tell streams that the operator has started and enable all the endpoints
     * associated with this operator.
     * Operator has sinks and source end points. Sink endpoint is the input to the
     * operator and source endpoint is output from the operator..
     */
    OPERATOR_DATA *cur_op = get_op_data_from_id(op_id);
    if (cur_op == NULL)
    {
        panic(PANIC_AUDIO_OPLIST_INVALID_OPID);
    }

    /* Enable the operator endpoints before raising bg int for operator process
     * The worst thing that can happen is that the endpoint enable will start
     * kicks and causes a bg int & operator might get kicked twice */
    stream_enable_operator_endpoints(INT_TO_EXT_OPID(op_id));
    /* Raise a bg int to process */
    raise_bg_int(cur_op->task_id); 

    set_system_event(SYS_EVENT_OP_START);
}

/**
 * \brief Core of Stop & Reset Handlers. This informs
 * streams that an operator has stopped
 *
 * \param  op_id the operator id to take action for/on.
 */
static void stop_connected_endpoints(unsigned int op_id)
{
    /* Tell streams that the operator has stopped and disable all the endpoints
     * associated with this operator.
     * Operator has sinks and source end points. Sink endpoint is the input to the
     * operator and source endpoint is output from the operator.
     * Due to mysterious reasons, only sinks were disabled in the past,
     * makes more sense to do both.
     */
    OPERATOR_DATA *cur_op = get_op_data_from_id(op_id);
    if (cur_op == NULL)
    {
        panic(PANIC_AUDIO_OPLIST_INVALID_OPID);
    }

    stream_disable_operator_endpoints(INT_TO_EXT_OPID(op_id));
}

/**
 * \brief Specific Handler for Operator Stop response messages. This informs
 * streams that an operator has stopped
 *
 * \param  op_id the operator id to take action for/on.
 */
static void stop_resp_handler(unsigned int op_id)
{
    stop_connected_endpoints(op_id);
    set_system_event(SYS_EVENT_OP_STOP);
}

/**
 * \brief Specific Handler for Operator reset response messages. 
 *
 * \param  op_id the operator id to take action for/on.
 */
static void reset_resp_handler(unsigned int op_id)
{
    stop_connected_endpoints(op_id);
    set_system_event(SYS_EVENT_OP_RESET);
}


/**
 * \brief Handler routine for messages that come from operators as unsolicited messages.
 *
 * \param  rinfo pointer to the routing information that came with the message
 * \param  msg pointer to the message payload that was received from the operator.
 */
static void msg_from_op_handler(tRoutingInfo *rinfo, unsigned *msg)
{
    /* We may have situations later where message doesn't exit Kymera and it is routed to some
     * Kymera entity. At the moment we only pass it to adaptor, so the opID has to be converted to
     * external ID. */
     ((OP_STD_UNSOLICITED_MSG*)msg)->op_id = INT_TO_EXT_OPID(((OP_STD_UNSOLICITED_MSG*)msg)->op_id);

    /* The message, pretty much as is, needs to be relayed to the adaptor that will
     * send it to the recipient. NOTE that this message carries external op ID and creator
     * client ID. So in reality, these should be packaged into connection ID here, but TODO:
     * for now using the routing info that OpMgr passed to us here. Reason is that the identity
     * of components (e.g. OpMgr having its own ID etc.) and real proper use of the routing info
     * fields is yet to be seen. If the above described override does happen, then it should occur
     * in this function's caller, in which case payload fields may be redundant.
     */

    /* Some brain teasing: the payload that adaptors expect is generic. So although we may
     * have zero length of this operator unsolicited message payload, the overall message with
     * op ID etc. fields is what defines the arguments to the adaptor send message function.
     * The length is the lenth we receive plus the four fields that precede the "actual" payload.
     * So let's have some fun: */
    adaptor_send_message(PACK_CON_ID(rinfo->src_id, rinfo->dest_id), AMSGID_FROM_OPERATOR,
                         ((OP_STD_UNSOLICITED_MSG*)msg)->length + UNSOLICITED_MSG_HEADER_SIZE, msg);

}


/****************************************************************************
 *
 * destroy_operator
 *
 * Destroys the stream endpoints for the operator, deletes the scheduler task,
 * calls the operator specific destroy function and unlinks the operator from
 * the linked list of operators.
 * If the operator specific destroy function fails an error code is returned
 * and the operator remains in the linked list.
 *
 */
static void destroy_resp_handler(unsigned int op_id)
{
    OPERATOR_DATA **p, *cur_op;

    cur_op = get_op_data_from_id(op_id);
    if (cur_op == NULL)
    {
        panic(PANIC_AUDIO_OPLIST_INVALID_OPID);
    }

#if defined(INSTALL_CAP_DOWNLOAD_MGR)
    /* Check whether this operator is a download capability */
    CAP_DOWNLOAD_STATUS status;
    bool is_downloadable;
    is_downloadable = opmgr_get_download_cap_status(cur_op->cap_data->id, &status);
    if (is_downloadable && (status == CAP_DOWNLOADED))
    {
        /* Depending on how the file was downloaded, we might not want to
         * remove the KCS after destroying */
        if (cap_download_mgr_remove_after_destroy((CAP_ID)cur_op->cap_data->id))
        {
            /* Is this the last instance of the downloaded capability? */
            if (opmgr_get_ops_count(cur_op->cap_data->id) == 1)
            {
                /* Then tell cap_download_mgr */
                cap_download_mgr_no_more_instances((CAP_ID)cur_op->cap_data->id);
            }
        }
    }
#endif
    set_system_event(SYS_EVENT_OP_DESTROY);
    /* It is now safe to delete the task as nothing internal will try and speak
     * to it now as it isn't connected to anything.
     */
    delete_task(cur_op->task_id);

    /* Now everything is gone so delete the entry from local operator list */
    p = &oplist_head;
    while(*p && *p != cur_op) p = &((*p)->next);
    if(*p)
    {
        *p = cur_op->next;
        PROFILER_DEREGISTER(cur_op->profiler);
        PROFILER_DELETE(cur_op->profiler);
        pfree(cur_op);
    }
}

static void message_resp_handler(tRoutingInfo *rinfo, OP_OPMSG_RSP *msg)
{
    unsigned op_id;

    op_id = GET_OPID_FROM_SRC_ID(rinfo->src_id);
    rinfo->src_id = GET_SRC_ID_FROM_PACKED_SRC_ID(rinfo->src_id);

    OP_MSG_CBACK callback = (OP_MSG_CBACK)opmgr_retrieve_in_progress_task(rinfo->dest_id, op_id);

    if (callback == NULL)
    {
        /* No data, panic. Passing operator id as debug information. */
        panic_diatribe(PANIC_AUDIO_OPMGR_NO_OP_DATA, op_id);
    }

    if(msg != NULL)
    {
        /* convert the response to the shape that the adaptor wants - if there is a response message */
        callback(PACK_CON_ID(rinfo->src_id, rinfo->dest_id), msg->status, INT_TO_EXT_OPID(msg->op_id), msg->length, (void*)&(msg->payload));
    }
    else
    {
        /* the infamous case of no response - one convention here is that we can still call back with NULLs and let it take whatever action */
        callback(PACK_CON_ID(rinfo->src_id, rinfo->dest_id), 0, 0, 0, NULL);
    }
}

static void create_resp_handler(tRoutingInfo *rinfo, OP_STD_RSP *msg)
{
    unsigned op_id;

    op_id = GET_OPID_FROM_SRC_ID(rinfo->src_id);
    rinfo->src_id = GET_SRC_ID_FROM_PACKED_SRC_ID(rinfo->src_id);

    OP_CREATE_CBACK callback = (OP_CREATE_CBACK) opmgr_retrieve_in_progress_task(rinfo->dest_id, op_id);

    /* Callback function pointer had better be valid; either path below will call it. */
    PL_ASSERT(callback != NULL);

    /* Check that the request succeeded. */
    if (STATUS_OK == msg->status)
    {
        /* Let the client know the operator has been created. */
        callback(PACK_CON_ID(rinfo->src_id, rinfo->dest_id), msg->status, INT_TO_EXT_OPID(op_id));

        /* The operator was created successfully so return. */
        set_system_event(SYS_EVENT_OP_CREATE);
    }
    else
    {
        OPERATOR_DATA *op_data;

        /* Find and delete task associated with the operator. */
        op_data = get_op_data_from_id(op_id);
        if (op_data == NULL)
        {
            panic_diatribe(PANIC_AUDIO_OPERATOR_NOT_FOUND, op_id);
        }
        delete_task(op_data->task_id);

        /* Remove the given operator's data structure from the local operator list. */
        remove_op_data_from_list(op_id, &oplist_head);

        /* Send the failure before returning. */
        callback(PACK_CON_ID(rinfo->src_id, rinfo->dest_id), msg->status, INT_TO_EXT_OPID(op_id));
    }
}

/*****************************************************************************
Public Function Definitions
*/
/*****************************************************************************
 *
 * opmgr_task_handler
 *
 */
void opmgr_task_handler(void **msg_data)
{
    uint16 cmd_id;
    void* msg_body;
    tRoutingInfo rinfo;

    patch_fn_shared(opmgr);

    while(get_message_with_routing(OPMGR_TASK_QUEUE_ID, &cmd_id, &msg_body, &rinfo))
    {
        switch (cmd_id)
        {
            case OPCMD_CREATE:
            {
                OP_STD_RSP *rsp = (OP_STD_RSP *)msg_body;
                create_resp_handler(&rinfo, rsp);
                break;
            }

            case OPCMD_DESTROY:
            {
                /* Destroy with a pre-processing step communicated to list response handler */
                OP_STD_RSP *rsp = (OP_STD_RSP *)msg_body;
                operator_list_resp_handler(&rinfo, rsp, OPCMD_DESTROY,
                        destroy_resp_handler, opmgr_destroy_op_endpoints);
                break;
            }
            case OPCMD_MESSAGE:
            {
                OP_OPMSG_RSP *rsp = (OP_OPMSG_RSP *)msg_body;
                message_resp_handler(&rinfo, rsp);
                break;
            }
            case OPCMD_START:
            {
                OP_STD_RSP *rsp = (OP_STD_RSP *)msg_body;
                operator_list_resp_handler(&rinfo, rsp, OPCMD_START,
                        start_resp_handler, NULL);
                break;
            }
            case OPCMD_STOP:
            {
                OP_STD_RSP *rsp = (OP_STD_RSP *)msg_body;
                operator_list_resp_handler(&rinfo, rsp, OPCMD_STOP,
                                        stop_resp_handler, NULL);
                break;
            }

            case OPCMD_RESET:
            {
                OP_STD_RSP *rsp = (OP_STD_RSP *)msg_body;
                operator_list_resp_handler(&rinfo, rsp, OPCMD_RESET,
                                        reset_resp_handler, NULL);
                break;
            }

            case OPCMD_DISCONNECT:

            case OPCMD_BUFFER_DETAILS:

            case OPCMD_DATA_FORMAT:

            case OPCMD_GET_SCHED_INFO:

            case OPCMD_CONNECT:
            {
                break;
            }
            case OPCMD_FROM_OPERATOR:
            {
                /* The unsolicited message can be either a "standard" short message with no parameters,
                 * or a long one with some number of params.
                 */
                msg_from_op_handler(&rinfo, msg_body);
                break;
            }
            default:
            {
                /* This should never happen, panic. Passing message id as debug information. */
                panic_diatribe(PANIC_AUDIO_OPMGR_INVALID_MSG_ID, cmd_id);
                break;
            }
        }
        /* Free the message */
        if (msg_body != NULL)
        {
            pfree(msg_body);
        }
    }
}
