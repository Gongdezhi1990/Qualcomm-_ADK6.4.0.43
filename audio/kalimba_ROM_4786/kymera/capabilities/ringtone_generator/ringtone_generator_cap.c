/**
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
 * \defgroup ringtone_generator
 * \file  ringtone_generator_cap.c
 * \ingroup  capabilities
 *
 * This capability generates notes for ringtone.
 *
 */

#include "ringtone_generator_cap.h"

/****************************************************************************
Private Type Definitions
*/
enum {
    AWAITING_FIRST_KICK,
    FIRST_KICK_PERFORMED,
    DATA_FLOWING
};


/****************************************************************************
Private Constant Declarations
*/
#ifdef CAPABILITY_DOWNLOAD_BUILD
#define RINGTONE_GENERATOR_CAP_ID CAP_ID_DOWNLOAD_RINGTONE_GENERATOR
#else
#define RINGTONE_GENERATOR_CAP_ID CAP_ID_RINGTONE_GENERATOR
#endif

/** The ringtone generator capability function handler table */
const handler_lookup_struct ringtone_generator_handler_table =
{
    ringtone_generator_create,          /* OPCMD_CREATE */
    ringtone_generator_destroy,         /* OPCMD_DESTROY */
    ringtone_generator_start,           /* OPCMD_START */
    base_op_stop,                       /* OPCMD_STOP */
    ringtone_generator_reset,           /* OPCMD_RESET */
    ringtone_generator_connect,         /* OPCMD_CONNECT */
    ringtone_generator_disconnect,      /* OPCMD_DISCONNECT */
    ringtone_generator_buffer_details,  /* OPCMD_BUFFER_DETAILS */
    base_op_get_data_format,            /* OPCMD_DATA_FORMAT */
    ringtone_generator_get_sched_info   /* OPCMD_GET_SCHED_INFO */
};

/* Null terminated operator message handler table - this is the set of operator
 * messages that the capability understands and will attempt to service. */
const opmsg_handler_lookup_table_entry ringtone_generator_opmsg_handler_table[] =
{
    {OPMSG_COMMON_ID_GET_CAPABILITY_VERSION, base_op_opmsg_get_capability_version},
    {OPMSG_COMMON_SET_SAMPLE_RATE, ringtone_generator_opmsg_config},
    {OPMSG_RINGTONE_ID_TONE_PARAM, ringtone_generator_opmsg_tones},
    {0, NULL}
};


/* Capability data - This is the definition of the capability that Opmgr uses to
 * create the capability from. */
const CAPABILITY_DATA ringtone_generator_cap_data =
{
    RINGTONE_GENERATOR_CAP_ID,                 /* Capability ID */
    0, 1,                                      /* Version information - hi and lo parts */
    0, MAX_OUTPUT_CHANS,                       /* Max number of sinks/inputs and sources/outputs */
    &ringtone_generator_handler_table,         /* Pointer to message handler function table */
    ringtone_generator_opmsg_handler_table,    /* Pointer to operator message handler function table */
    ringtone_generator_process_data,           /* Pointer to data processing function */
    0,                                         /* TODO: this would hold processing time information */
    sizeof(RINGTONE_GENERATOR_OP_DATA)         /* Size of capability-specific per-instance data */
};

/* ********************************** API functions ************************************* */

bool ringtone_generator_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    RINGTONE_GENERATOR_OP_DATA *rgop_data = (RINGTONE_GENERATOR_OP_DATA *)(op_data->extra_op_data);
    uint32 sample_rate;
    unsigned buffer_size;
    TIME_INTERVAL kick_period;

    /* call base_op create, which also allocates and fills response message */
    if (!base_op_create(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* Set the default sample rate and output block size */
    tone_synth_set_sample_rate(&rgop_data->info, BASE_SAMPLE_RATE);
    rgop_data->tone_data_block_size = RINGTONE_OUTPUT_LEN;

    /* Allocate the output buffer 
     * Normally this size would be calculated in the buffer_details function
     * but because this capability uses a single shared buffer we need the size here
     *
     * Also note +2 not +1, because other operators will use the same calculation, 
     * but there the size has one added to it in stream_connect
     */

    sample_rate = stream_if_get_system_sampling_rate();
    kick_period = stream_if_get_system_kick_period();

    /* Max possible supported sample rate is 192kHz, 
     * which allows up to about 22ms before this would overflow 
     */
    PL_ASSERT(kick_period <= 20*MILLISECOND);
    buffer_size = (unsigned)(2 + (sample_rate * kick_period)/1000000);

    L2_DBG_MSG1("ringtone_generator_create buffer_size = %u", buffer_size);

    rgop_data->out_cbuffer = cbuffer_create_with_malloc(buffer_size, BUF_DESC_SW_BUFFER);

    /* Connect synthesizer */
    tone_synth_set_output(&rgop_data->info, & ringtone_write, (void*)rgop_data);

    return TRUE;
}

bool ringtone_generator_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
   RINGTONE_GENERATOR_OP_DATA *rgop_data = (RINGTONE_GENERATOR_OP_DATA *)(op_data->extra_op_data);

   if(op_data->state == OP_RUNNING)
   {
        *response_id = OPCMD_DESTROY;

        /* We can't destroy a running operator */
        return base_op_build_std_response(STATUS_CMD_FAILED, op_data->id, response_data);
   }
   else
   {
        /* Release the capability specific memory allocation */
        cbuffer_destroy(rgop_data->out_cbuffer);
        cbuffer_destroy_struct(rgop_data->connecting_buf);

        /* Clean up any tone buffers */
        while (rgop_data->tone_list.head != NULL)
        {
            RINGTONE_BUFFER *tone_input_buf = rgop_data->tone_list.head;
            rgop_data->tone_list.head = rgop_data->tone_list.head->next_tone;
            pfree(tone_input_buf);
        }

        return base_op_destroy(op_data, message_data, response_id, response_data);
   }


}

bool ringtone_generator_reset(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    RINGTONE_GENERATOR_OP_DATA *rgop_data = (RINGTONE_GENERATOR_OP_DATA *)(op_data->extra_op_data);

    if(!base_op_reset(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* reset the tone data */
    ringtone_init(rgop_data);

    return TRUE;
}

bool ringtone_generator_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    RINGTONE_GENERATOR_OP_DATA *rgop_data = (RINGTONE_GENERATOR_OP_DATA *)(op_data->extra_op_data);

    *response_id = OPCMD_START;

    if (!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }

    /* Check if the operator is already running. If it is, just ignore the command */
    if(op_data->state != OP_RUNNING)
    {
        /* check if we have any connection */
        if(rgop_data->active_chans)
        {
            rgop_data->tone_end = FALSE;

            if(rgop_data->tone_list.head)
            {
                /* If we have received any tone definition, initialise the ringtone generator
                *  and create a ringtone stream with some ringtone note information */
                ringtone_init(rgop_data);
            }

            /* set state to running */
            op_data->state = OP_RUNNING;

            rgop_data->start_seq_stage = AWAITING_FIRST_KICK;
            /* Because ringtone generator is similar to endpoint, set a background kick at the start */
            opmgr_kick_operator(op_data);
        }
        else
        {
            /* must be connected to start */
            base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        }
    }
    return TRUE;
}

bool ringtone_generator_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    OP_SCHED_INFO_RSP* resp;


    if (!base_op_get_sched_info(op_data, message_data, response_id, response_data))
    {
        return base_op_build_std_response(STATUS_CMD_FAILED, op_data->id, response_data);
    }

    resp = *response_data;
    base_op_change_response_status(response_data, STATUS_OK);

    resp->op_id = op_data->id;
    resp->block_size = RINGTONE_GENERATOR_BLOCK_SIZE;
    resp->run_period = 0;

    *response_data = resp;

    return TRUE;
}

/** The Ringtone generator is only supposed to be connected as source, not sink **/
bool ringtone_generator_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    RINGTONE_GENERATOR_OP_DATA *rgop_data = (RINGTONE_GENERATOR_OP_DATA *)(op_data->extra_op_data);
    unsigned terminal_id = ((unsigned *)message_data)[0];
    tCbuffer* buf;


    *response_id = OPCMD_CONNECT;
    if(!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }

    /* We only support limited number of output connections */
    if(((terminal_id & TERMINAL_SINK_MASK) != 0)||(terminal_id >= (MAX_OUTPUT_CHANS - 1)))
    {
    	base_op_change_response_status(response_data, STATUS_INVALID_CMD_PARAMS);

        return TRUE;
    }

    /* Check if this terminal has been connected */
    if(CHANNEL_MASK(terminal_id) & (rgop_data->active_chans))
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    buf = (tCbuffer*)(((uintptr_t *)message_data)[1]);

    /* If this buffer is not the same as the one assigned in the buffer details,
     * indicates something has gone wrong. */
    PL_ASSERT(buf == rgop_data->connecting_buf);
    rgop_data->connecting_buf = NULL;

    rgop_data->op_chnl_cbuffer[terminal_id] = buf;
#ifdef INSTALL_METADATA
    if (rgop_data->metadata_op_buffer == NULL)
    {
        if (buff_has_metadata(rgop_data->op_chnl_cbuffer[terminal_id]))
        {
            rgop_data->metadata_op_buffer = rgop_data->op_chnl_cbuffer[terminal_id];
        }
    }
#endif /* INSTALL_METADATA */
    /* Initialise the read/write pointers for each output channel */
    rgop_data->op_chnl_cbuffer[terminal_id]->read_ptr = rgop_data->out_cbuffer->write_ptr;
    rgop_data->op_chnl_cbuffer[terminal_id]->write_ptr = rgop_data->out_cbuffer->write_ptr;

    /* Set the ith number channel is active, mark the terminal id
    * and increment the number of connections to the capability */
    rgop_data->active_chans |=CHANNEL_MASK(terminal_id);

    return TRUE;

}



bool ringtone_generator_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    RINGTONE_GENERATOR_OP_DATA *rgop_data = (RINGTONE_GENERATOR_OP_DATA *)(op_data->extra_op_data);
    unsigned terminal_id = ((unsigned *)message_data)[0];


    if(!base_op_buffer_details(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

#ifdef INSTALL_METADATA
    {
        /* If an input/output connection is already present and has metadata then
         * we are obliged to return that buffer so that metadata can be shared
         * between channels. */
        tCbuffer *meta_buff;
        meta_buff = rgop_data->metadata_op_buffer;

        ((OP_BUF_DETAILS_RSP*)*response_data)->metadata_buffer = meta_buff;
        ((OP_BUF_DETAILS_RSP*)*response_data)->supports_metadata = TRUE;
    }
#endif /* INSTALL_METADATA */

    /* We only support limited number of output connections */
    if(((terminal_id & TERMINAL_SINK_MASK) != 0)||(terminal_id >= (MAX_OUTPUT_CHANS - 1)))
    {
    	base_op_change_response_status(response_data, STATUS_INVALID_CMD_PARAMS);
    }
    /* We have got an output valid channel connection request */
    else
    {
        OP_BUF_DETAILS_RSP *buf_resp = (OP_BUF_DETAILS_RSP*)*response_data;

        /* Provide a specific size for the output buffer in order to do a in-place copying to multiple output channels. */
        buf_resp->supplies_buffer = TRUE;
        tCbuffer* shared_buffer = rgop_data->out_cbuffer;

        /*
         * If there is an existing buffer, just reuse it.
         * This could be a result of this function getting called twice, or it could be left over
         * from a previously-aborted connect. Either way, the output buffers have the same base
         * address for all terminals, so it's safe to just use the same one again
         */
        if (rgop_data->connecting_buf == NULL)
        {
            rgop_data->connecting_buf = cbuffer_create(shared_buffer->base_addr, cbuffer_get_size_in_words(shared_buffer), BUF_DESC_SW_BUFFER);
        }
        buf_resp->b.buffer = rgop_data->connecting_buf;

        if (!(buf_resp->b.buffer))
        {
            /* There wasn't enough RAM so we have to fail */
            base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            return TRUE;
        }
    }
    return TRUE;
}



bool ringtone_generator_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    RINGTONE_GENERATOR_OP_DATA *rgop_data = (RINGTONE_GENERATOR_OP_DATA *)(op_data->extra_op_data);
    unsigned terminal_id = ((unsigned *)message_data)[0];
    tCbuffer *output_channel_buffer;


    *response_id = OPCMD_DISCONNECT;

    if(!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        /* Shouldn't change anything if there is not enough memory for the response.*/
        return FALSE;
    }

    /* Check if the channel is valid or not */
    if(((terminal_id & TERMINAL_SINK_MASK) != 0)||(terminal_id >= (MAX_OUTPUT_CHANS - 1)))
    {
    	base_op_change_response_status(response_data, STATUS_INVALID_CMD_PARAMS);
        return TRUE;
    }

    if(op_data->state == OP_RUNNING)
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }
#ifdef INSTALL_METADATA
    if (rgop_data->op_chnl_cbuffer[terminal_id] != NULL)
    {
        if (rgop_data->metadata_op_buffer == rgop_data->op_chnl_cbuffer[terminal_id])
        {
            unsigned j;
            bool found_alternative = FALSE;
            for (j = 0; j < MAX_OUTPUT_CHANS; j++)
            {
                if (j == terminal_id)
                {
                    continue;
                }
                if (rgop_data->op_chnl_cbuffer[j] != NULL
                    && buff_has_metadata(rgop_data->op_chnl_cbuffer[j]))
                {
                    rgop_data->metadata_op_buffer = rgop_data->op_chnl_cbuffer[j];
                    found_alternative = TRUE;
                    break;
                }
            }
            if (!found_alternative)
            {
                rgop_data->metadata_op_buffer = NULL;
            }
        }
    }
#endif /* INSTALL_METADATA */
    /* Free the memory specific to the requested disconnecting channel */
    output_channel_buffer = rgop_data->op_chnl_cbuffer[terminal_id];
    rgop_data->op_chnl_cbuffer[terminal_id] = NULL;
    cbuffer_destroy_struct(output_channel_buffer);
    rgop_data->active_chans^= CHANNEL_MASK(terminal_id);

    return TRUE;
 }
#ifdef INSTALL_METADATA
static void ringtone_eof_callback(unsigned data)
{
    common_send_simple_unsolicited_message((OPERATOR_DATA *)(uintptr_t)data, OPMSG_RINGTONE_REPLY_ID_TONE_END);
    pdelete((OPERATOR_DATA *)(uintptr_t)data);
}
#endif

static void ringtone_generator_startup_timer_handler(void *data)
{
    opmgr_kick_operator((OPERATOR_DATA *)data);
}

void ringtone_generator_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    RINGTONE_GENERATOR_OP_DATA *rgop_data = (RINGTONE_GENERATOR_OP_DATA *)(op_data->extra_op_data);
    unsigned before_space, after_space;

    patch_fn(ringtone_generator_process_data);
#ifdef INSTALL_METADATA
    metadata_tag *eof_tag = NULL, *empty_tag;
    unsigned generated_octets;
    unsigned before_index, after_index;
    OPERATOR_DATA *cb_data = NULL;
#endif

    if (rgop_data->start_seq_stage != DATA_FLOWING)
    {
        touched->sources = rgop_data->active_chans;
        /* Schedule a subsequent kick until the data actually starts flowing */
        timer_schedule_event_in(500, ringtone_generator_startup_timer_handler, (void *)op_data);
    }
    /* Update the read pointer before generating new tone data*/
    ringtone_update_read_ptr(rgop_data);
    /* Get the space in the buffer before the ringtone ran. */
    before_space = cbuffer_calc_amount_space_in_words(rgop_data->out_cbuffer);
    /* If there is any tone definition here, go to generate some tones */
    if(rgop_data->tone_list.head)
    {
        /* Generate the tone data with the received ringtone definitions */
        ringtone_play(rgop_data, rgop_data->tone_data_block_size);
        /* Are we reaching the end? */
        if(rgop_data->tone_end)
        {
#ifdef INSTALL_METADATA
            eof_tag = buff_metadata_new_tag();
            if(eof_tag != NULL)
            {
                eof_tag->length = 0;
            }
#endif
            if(!rgop_data->unsolicited_sent)
            {
#ifdef INSTALL_METADATA
                cb_data = xzpnew(OPERATOR_DATA);
                if(cb_data != NULL && eof_tag != NULL)
                {
                    *cb_data = *op_data;
                    buff_metadata_add_eof_callback(eof_tag, ringtone_eof_callback, (unsigned)(uintptr_t)cb_data);
                }
                else
                {
                    /* fall back to the no-metadata behaviour */
                    common_send_simple_unsolicited_message(op_data, OPMSG_RINGTONE_REPLY_ID_TONE_END);
                }
#else
                common_send_simple_unsolicited_message(op_data, OPMSG_RINGTONE_REPLY_ID_TONE_END);
#endif
                rgop_data->unsolicited_sent = TRUE;
            }
            /* Fill zeros into the out_buffer when no tone data */
            cbuffer_fill_buffer(rgop_data->out_cbuffer, 0);
        }
    }
    else
    {
        /* Fill zeros into the out_buffer when no tone data */
        cbuffer_fill_buffer(rgop_data->out_cbuffer, 0);
    }

    /* update the write pointer for each output channel */
    ringtone_update_write_ptr(rgop_data);
    /* Get the space in the buffer after the ringtone has run. */
    after_space = cbuffer_calc_amount_space_in_words(rgop_data->out_cbuffer);
    /* Ringtone should only kick if it has data in the buffer. */
    if(after_space < before_space)
    {
        if (rgop_data->start_seq_stage != DATA_FLOWING)
        {
            if (rgop_data->start_seq_stage == AWAITING_FIRST_KICK)
            {
                rgop_data->start_seq_stage = FIRST_KICK_PERFORMED;
            }
            else
            {
                rgop_data->start_seq_stage = DATA_FLOWING;
            }
        }
#ifdef INSTALL_METADATA
        /* Cover the generated samples by a tag. */
        generated_octets = (before_space - after_space) * OCTETS_PER_SAMPLE;

        before_index = 0;
        after_index = generated_octets;

        /* Ringtone only generates empty tags to keep the metadata transport happy. */
        empty_tag = buff_metadata_new_tag();
        PL_ASSERT(empty_tag != NULL);

        empty_tag->length = generated_octets;
        if (eof_tag != NULL)
        {
            empty_tag->next = eof_tag;
            after_index = 0;
        }

        L4_DBG_MSG5("ringtone tag = 0x%08x, tag->length =  0x%08x, generated_octets =  0x%08x, before_index = 0x%08x, after_index = 0x%08x",
                  (uintptr_t)(empty_tag), empty_tag->length, generated_octets, before_index, after_index);

        buff_metadata_append(rgop_data->metadata_op_buffer, empty_tag, before_index, after_index);
#endif
        touched->sources = rgop_data->active_chans;
    }

}
/** Update the read pointer of the output of ringtone generator **/
void ringtone_update_read_ptr(RINGTONE_GENERATOR_OP_DATA *op_data)
{

    int space;
    unsigned min_space, i, active_chans;
    int *updated_read_ptr;
    unsigned int buffer_size;


    /* Initialise the minimum space */
    min_space = UINT_MAX;

    active_chans = op_data->active_chans;
    updated_read_ptr = op_data->out_cbuffer->read_ptr;

    /* All the connected channels are sharing the same buffer as the output of ringtone generator,
    * therefore, we could simply cache the size of output buffer to improve the code efficiency */
    buffer_size = cbuffer_get_size_in_words(op_data->out_cbuffer);

    for (i=0; i<MAX_OUTPUT_CHANS; i++)
    {
        if(active_chans & 01)
        {
            space = op_data->op_chnl_cbuffer[i]->read_ptr - updated_read_ptr;

            if(space < 0)
            {
                space += buffer_size;
            }

            if(space < min_space)
            {
                min_space = space;
            }

            active_chans>>=1;
        }
    }

    /* Update the read pointer of the output buffer of ringtone generator */
    updated_read_ptr +=min_space;

    if(updated_read_ptr >= (op_data->out_cbuffer->base_addr + buffer_size) )
    {
        updated_read_ptr -= buffer_size;
    }

    op_data->out_cbuffer->read_ptr = updated_read_ptr;
 }

 /** Update the write pointer of the buffer associated to each output channel **/
void ringtone_update_write_ptr(RINGTONE_GENERATOR_OP_DATA *rgop_data)
{
    unsigned i, active_chans;
    int *out_buffer_wrt_ptr = rgop_data->out_cbuffer->write_ptr;


    active_chans = rgop_data->active_chans;

    for(i=0; i<MAX_OUTPUT_CHANS;i++)
    {
        if(active_chans & 0x01)
        {
            /* update the write pointer to be the same as the out_buffer */
            rgop_data->op_chnl_cbuffer[i]->write_ptr = out_buffer_wrt_ptr;
        }
        /* Move to the next channel */
        active_chans >>=1;
    }
}



/********************************Operator message handlers ********************************************/

bool ringtone_generator_opmsg_config(OPERATOR_DATA *op_data, void *message_data, unsigned int *response_length, OP_OPMSG_RSP_PAYLOAD **response_data)
{
    RINGTONE_GENERATOR_OP_DATA *rgop_data = (RINGTONE_GENERATOR_OP_DATA *)(op_data->extra_op_data);
    OP_MSG_REQ *msg = (OP_MSG_REQ *)message_data;

    unsigned sample_rate = msg->payload[0] * CONVERSION_SAMPLE_RATE_TO_HZ;
    tone_synth_set_sample_rate(&rgop_data->info, sample_rate);
    rgop_data->tone_data_block_size = sample_rate / BASE_SAMPLE_RATE * RINGTONE_OUTPUT_LEN;

    return TRUE;
}

bool ringtone_generator_opmsg_tones(OPERATOR_DATA *op_data, void *message_data, unsigned int *response_length, OP_OPMSG_RSP_PAYLOAD **response_data)
{

    RINGTONE_GENERATOR_OP_DATA *rgop_data = (RINGTONE_GENERATOR_OP_DATA *)(op_data->extra_op_data);
    OP_MSG_REQ *msg = (OP_MSG_REQ *)message_data;
    unsigned tone_param_len;
    RINGTONE_BUFFER *pnew_tone;


    tone_param_len = OPMGR_GET_OPMSG_LENGTH(msg) - 1;

    /* If the tone length is too big, return false */
    if(tone_param_len > MAX_TONE_LEN)
    {
        return FALSE;
    }

    pnew_tone = (RINGTONE_BUFFER *)xzpmalloc(sizeof(RINGTONE_BUFFER) + tone_param_len * sizeof(unsigned));

    if(!pnew_tone)
    {
       /* Out of space */
       return FALSE;
    }

    pnew_tone->index = (unsigned*)&(pnew_tone->cur_tone_buf);
    pnew_tone->tone_param_end = (unsigned*)&(pnew_tone->cur_tone_buf[tone_param_len]);
    memcpy( pnew_tone->cur_tone_buf, msg->payload, tone_param_len*sizeof(unsigned));

    /* Put it on the list */
    if(!rgop_data->tone_list.head)
    {
        /* First tone received */
        rgop_data->tone_list.head = pnew_tone;
        rgop_data->tone_list.tail = pnew_tone;
    }
    else
    {
        /* Put the current tone on the tail of the tone list */
        rgop_data->tone_list.tail->next_tone = pnew_tone;
        rgop_data->tone_list.tail = pnew_tone;
    }

    if(rgop_data->tone_end)
    {
        rgop_data->tone_end = FALSE;
    }
    if(rgop_data->unsolicited_sent)
    {
        rgop_data->unsolicited_sent = FALSE;
    }

    if((op_data->state == OP_RUNNING) & (rgop_data->info.tone_info.decay == 0) )
    {
        /*initialise the ringtone generator */
        ringtone_init(rgop_data);
    }

     return TRUE;
}
