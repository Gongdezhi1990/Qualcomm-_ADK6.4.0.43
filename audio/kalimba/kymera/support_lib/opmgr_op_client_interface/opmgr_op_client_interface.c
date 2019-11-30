#include "opmgr/opmgr_op_client_interface.h"

bool opmgr_op_client_send_message(OPERATOR_DATA *op_data, OPERATOR_ID target_op_id,
                                  unsigned msg_length, unsigned * msg)
{
    return TRUE;
}

bool opmgr_op_client_start_operator(OPERATOR_DATA *op_data,
                                    unsigned num_ops, OPERATOR_ID *op_list)
{
    return TRUE;
}

bool opmgr_op_client_stop_operator(OPERATOR_DATA *op_data,
                                   unsigned num_ops, OPERATOR_ID *op_list)
{
    return TRUE;
}

bool opmgr_op_client_reset_operator(OPERATOR_DATA *op_data,
                                    unsigned num_ops, OPERATOR_ID *op_list)
{
    return TRUE;
}

