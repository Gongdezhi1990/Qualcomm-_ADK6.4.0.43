/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
#include "ipc/ipc_private.h"


#include <message.h>

void ipc_stream_handler(IPC_SIGNAL_ID id, const void *msg)
{
    switch(id)
    {
    case IPC_SIGNAL_ID_STREAM_DESTROYED:
    {
        const IPC_STREAM_DESTROYED *destroy_msg = msg;
        (void)MessageStreamTaskFromSink(SINK_FROM_ID(destroy_msg->sink), NULL);
    }
        break;

    case IPC_SIGNAL_ID_OPERATORS_DESTROYED:
    {
        uint16 op_num;
        const IPC_OPERATORS_DESTROYED *destroy_msg = msg;
        L3_DBG_MSG1("ipc_stream_handler: num_operators %u", 
                    destroy_msg->num_operators);
        for (op_num = 0; destroy_msg->op_list &&
                         op_num < destroy_msg->num_operators; op_num++)
        {
            L3_DBG_MSG1("ipc_stream_handler: Operator 0x%x destroyed",
                         destroy_msg->op_list[op_num]);
            (void)MessageOperatorTask(destroy_msg->op_list[op_num], NULL);
        }

        if (destroy_msg->op_list)
        {
            /* free op_list */
            pfree(destroy_msg->op_list);
        }
    }
        break;

    default:
        L1_DBG_MSG1("ipc_stream_handler: unexpected signal ID 0x%x received", id);
        assert(FALSE);
        break;
    }
}
