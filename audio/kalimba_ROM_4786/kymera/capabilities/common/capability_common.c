/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  capability_common.c
 * \ingroup  capabilities
 *
 * Common code that can be used by any capability, but not abstract enough to be in base_op.
 *
 */

#include "capabilities.h"

#include "patch/patch.h"

/* set fadeout state and initialise fadeout counter and flush count */
void common_set_fadeout_state(FADEOUT_PARAMS* fadeout_parameters, FADEOUT_STATE state)
{
    patch_fn(common_set_fadeout_state);
    /* if input terminal has audio data type, extra op data is allocated and
     * fadeout is not already enabled/running */
    if (state == RUNNING_STATE)
    {
        if (fadeout_parameters->fadeout_state != NOT_RUNNING_STATE)
        {
            return;
        }
    }
    else
    {
        if (fadeout_parameters->fadeout_state == NOT_RUNNING_STATE)
        {
            return;
        }
    }

    fadeout_parameters->fadeout_state = state;
    fadeout_parameters->fadeout_counter = 0;
    fadeout_parameters->fadeout_flush_count = 0;
}

/* Send simple unsolicited message that has no real payload just the message ID */
void common_send_simple_unsolicited_message(OPERATOR_DATA *op_data, unsigned msg_id)
{
    common_send_unsolicited_message(op_data, msg_id, 0, NULL);
}

/* Send unsolicited message  */
bool common_send_unsolicited_message(OPERATOR_DATA *op_data, unsigned msg_id, unsigned length, const unsigned *payload)
{
    tRoutingInfo rinfo;
    OP_UNSOLICITED_MSG *msg_from_op;

    patch_fn(common_send_unsolicited_message);    

	msg_from_op = (OP_UNSOLICITED_MSG *) xpmalloc(sizeof(OP_UNSOLICITED_MSG) + sizeof(unsigned)*length);
    


    if(msg_from_op == NULL)
    {
        /* failed to allocate memory for the message
         * let the sender know immediately
         */
        return FALSE;
    }

    /* set up the fields */
    msg_from_op->length = length;
    msg_from_op->op_id = op_data->id;
    msg_from_op->client_id = op_data->creator_client_id;
    msg_from_op->msg_id = msg_id;

    /* Copy the payload if there is any */
    if (length != 0)
    {
        memcpy(msg_from_op->payload, payload, length*sizeof(unsigned));
    }

    /* route it to the gods */
    rinfo.src_id = op_data->id;
    rinfo.dest_id = op_data->creator_client_id; /* using creator client ID - bit redundant as it's also inside message */

    /* Currently, there isn't any connection id passed by the operaotr when
    * sending unsolicited message. Since the destination id uses create_client_id,
    * we can use the connection id from the op_data */
    unsigned con_id = op_data -> con_id;
    unsigned processor_id = op_data-> processor_id;

    opmgr_unsolicited_message(con_id, processor_id, msg_from_op, &rinfo);

    return TRUE;
}
