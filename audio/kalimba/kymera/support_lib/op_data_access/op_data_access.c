#include "op_msg_helpers.h"
#include "base_op.h"
#include "pmalloc/pl_malloc.h"

/**
 * \brief Helper function for finding out where an operator is running
 * or not.
 *
 * \param op_data The data structure of the operator to query
 */
bool opmgr_op_is_running(OPERATOR_DATA *op_data)
{
    return op_data->state == OP_RUNNING;
}

/* will allocate and create a success message, with zeroed error_code field */
bool base_op_build_std_response_ex(OPERATOR_DATA *op_data, STATUS_KYMERA status, void **response_data)
{
    OP_STD_RSP* resp = xzpnew(OP_STD_RSP);

    if (resp == NULL)
    {
        return FALSE;
    }

    resp->op_id = op_data->id;
    resp->resp_data.err_code = 0;
    resp->status = status;

    *response_data = resp;
    return TRUE;
}

void* base_op_get_instance_data(OPERATOR_DATA *op_data)
{
    return op_data->extra_op_data;
}

CAP_ID base_op_get_cap_id(OPERATOR_DATA *op_data)
{
    return op_data->cap_data->id;
}

OP_SCHED_INFO_RSP* base_op_get_sched_info_ex(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id)
{
    OP_SCHED_INFO_RSP* resp;

    resp = xzpnew(OP_SCHED_INFO_RSP);
    if (resp == NULL)
    {
        return NULL;
    }

    resp->op_id = op_data->id;
    resp->status = STATUS_OK;

    return resp;
}

INT_OP_ID base_op_get_int_op_id(OPERATOR_DATA *op_data)
{
    return op_data->id;
}

EXT_OP_ID base_op_get_ext_op_id(OPERATOR_DATA *op_data)
{
    return INT_TO_EXT_OPID(op_data->id);
}

OPERATOR_DATA *base_op_clone_operator_data(OPERATOR_DATA *op_data)
{
    OPERATOR_DATA *clone;
    clone = xzpnew(OPERATOR_DATA);
    if (clone != NULL)
    {
        *clone = *op_data;
    }
    return clone;
}

#ifdef PROFILER_ON
void base_op_profiler_start(OPERATOR_DATA *op_data)
{
    if (op_data->profiler != NULL)
    {
        PROFILER_START(op_data->profiler);
    }
}

void base_op_profiler_stop(OPERATOR_DATA *op_data)
{
    if (op_data->profiler != NULL)
    {
        PROFILER_STOP(op_data->profiler);
    }
}

void base_op_profiler_add_kick(OPERATOR_DATA *op_data)
{
    if (op_data->profiler != NULL)
    {
        op_data->profiler->kick_inc++;
    }
}
#endif /* PROFILER_ON */

void base_op_stop_operator(OPERATOR_DATA *op_data)
{
    op_data->state = OP_NOT_RUNNING;
}

const CAPABILITY_DATA* base_op_get_cap_data(OPERATOR_DATA *op_data)
{
    return op_data->cap_data;
}

void* base_op_get_class_ext(OPERATOR_DATA *op_data)
{
    return op_data->cap_class_ext;
}

void base_op_set_class_ext(OPERATOR_DATA *op_data, void *class_data)
{
    op_data->cap_class_ext = class_data;
}

void opmgr_op_suspend_processing(OPERATOR_DATA *op_data)
{
    interrupt_block();
}

void opmgr_op_resume_processing(OPERATOR_DATA *op_data)
{
    interrupt_unblock();
}

