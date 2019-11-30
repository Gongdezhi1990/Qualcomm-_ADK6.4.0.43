/**
 * Copyright (c) 2015 - 2018 Qualcomm Technologies International, Ltd.
 * \defgroup ringtone_generator
 * \file  ringtone_generator_cap.c
 * \ingroup  capabilities
 *
 * This capability generates notes for ringtone.
 *
 */
#include <string.h>
#include "ringtone_generator_cap.h"
#include "capabilities.h"

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
    0,                                         /* Reserved */
    sizeof(RINGTONE_GENERATOR_OP_DATA)         /* Size of capability-specific per-instance data */
};
#if !defined(CAPABILITY_DOWNLOAD_BUILD)
MAP_INSTANCE_DATA(CAP_ID_RINGTONE_GENERATOR, RINGTONE_GENERATOR_OP_DATA)
#else
MAP_INSTANCE_DATA(CAP_ID_DOWNLOAD_RINGTONE_GENERATOR, RINGTONE_GENERATOR_OP_DATA)
#endif /* CAPABILITY_DOWNLOAD_BUILD */

/****************************************************************************
Private Function Definitions
*/
static inline RINGTONE_GENERATOR_OP_DATA *get_instance_data(OPERATOR_DATA *op_data)
{
    return (RINGTONE_GENERATOR_OP_DATA *) base_op_get_instance_data(op_data);
}

/* ********************************** API functions ************************************* */

bool ringtone_generator_create(OPERATOR_DATA *op_data, void *message_data,
                               unsigned *response_id, void **response_data)
{
    RINGTONE_GENERATOR_OP_DATA *rgop_data = get_instance_data(op_data);

    /* call base_op create, which also allocates and fills response message */
    if (!base_op_create(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* Set the default sample rate and output block size */
    tone_synth_set_sample_rate(&rgop_data->info, BASE_SAMPLE_RATE);
    rgop_data->tone_data_block_size = RINGTONE_OUTPUT_LEN;
    rgop_data->out_cbuffer = NULL;

    /* Connect synthesizer */
    tone_synth_set_output(&rgop_data->info, &ringtone_write, (void*)rgop_data);

    return TRUE;
}

bool ringtone_generator_destroy(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data)
{
    RINGTONE_GENERATOR_OP_DATA *rgop_data = get_instance_data(op_data);

    if (opmgr_op_is_running(op_data))
    {
        /* We can't destroy a running operator */
        return base_op_build_std_response_ex(op_data, STATUS_CMD_FAILED, response_data);
    }

        /* Release any possible capability specific memory allocation
           (stereo only) */
    cbuffer_destroy(rgop_data->out_cbuffer);

    /* Clean up any tone buffers */
    while (rgop_data->tone_list.head != NULL)
    {
        RINGTONE_BUFFER *tone_input_buf = rgop_data->tone_list.head;
        rgop_data->tone_list.head = rgop_data->tone_list.head->next_tone;
        pfree(tone_input_buf);
    }

    return base_op_destroy(op_data, message_data, response_id, response_data);
}

bool ringtone_generator_reset(OPERATOR_DATA *op_data, void *message_data,
                              unsigned *response_id, void **response_data)
{
    RINGTONE_GENERATOR_OP_DATA *rgop_data = get_instance_data(op_data);

    if (!base_op_reset(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* reset the tone data */
    ringtone_init(rgop_data);

    return TRUE;
}

bool ringtone_generator_start(OPERATOR_DATA *op_data, void *message_data,
                              unsigned *response_id, void **response_data)
{
    RINGTONE_GENERATOR_OP_DATA *rgop_data = get_instance_data(op_data);

    if (!base_op_build_std_response_ex(op_data, STATUS_OK, response_data))
    {
        return FALSE;
    }

    /* Check if the operator is already running. If it is, just ignore the command */
    if (opmgr_op_is_running(op_data))
    {
        return TRUE;
    }

    /* check if we have any connection */
    if (rgop_data->active_chans == 0)
    {
        /* must be connected to start */
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    rgop_data->tone_end = FALSE;

    if (rgop_data->tone_list.head)
    {
        /* If we have received any tone definition, initialise the ringtone generator
         * and create a ringtone stream with some ringtone note information */
        ringtone_init(rgop_data);
    }
    rgop_data->start_seq_stage = AWAITING_FIRST_KICK;
    /* Because ringtone generator is similar to endpoint,
     * set a background kick at the start */
    opmgr_kick_operator(op_data);

    return TRUE;
}

bool ringtone_generator_get_sched_info(OPERATOR_DATA *op_data, void *message_data,
                                       unsigned *response_id, void **response_data)
{
    OP_SCHED_INFO_RSP* resp;

    resp = base_op_get_sched_info_ex(op_data, message_data, response_id);
    if (resp == NULL)
    {
        return base_op_build_std_response_ex(op_data, STATUS_CMD_FAILED, response_data);
    }
    *response_data = resp;

    resp->block_size = RINGTONE_GENERATOR_BLOCK_SIZE;

    return TRUE;
}

/** The Ringtone generator is only supposed to be connected as source, not sink **/
bool ringtone_generator_connect(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data)
{
    RINGTONE_GENERATOR_OP_DATA *rgop_data = get_instance_data(op_data);
    unsigned terminal_id = OPMGR_GET_OP_CONNECT_TERMINAL_ID(message_data);
    tCbuffer* buf;

    if (!base_op_build_std_response_ex(op_data, STATUS_OK, response_data))
    {
        return FALSE;
    }

    /* We only support limited number of output connections */
    if (!base_op_is_terminal_valid(op_data, terminal_id))
    {
        base_op_change_response_status(response_data, STATUS_INVALID_CMD_PARAMS);

        return TRUE;
    }

    /* Check if this terminal has been connected */
    if (CHANNEL_MASK(terminal_id) & (rgop_data->active_chans))
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    buf = OPMGR_GET_OP_CONNECT_BUFFER(message_data);

    if (rgop_data->out_cbuffer == NULL)
    {
        /* first connection */
        rgop_data->out_cbuffer = buf;
    }
    else
    {
        if (pl_one_bit_count( rgop_data->active_chans) == 1)
        {
            /* stereo only: only for the second connected
               separate tone gen buffer from connection buffer;
               all subsequent terminals will use already connected output buffer;
               and copy generated data across
             */
            tCbuffer *cb = rgop_data->out_cbuffer;
            tCbuffer *new_buff;
            unsigned descriptor = BUF_DESC_SW_BUFFER;

            /* use the new buffer for tone gen
                and keep the connection buffer as it was set for mono */
            new_buff = cbuffer_create_with_malloc( cbuffer_get_size_in_words(cb), descriptor);
            if (new_buff != NULL)
            {
                rgop_data->out_cbuffer = new_buff;
            }
            else
            {
                /* Not enough RAM - fail connect */
                base_op_change_response_status(response_data, STATUS_CMD_FAILED);
                return TRUE;
            }
        }
    }
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

    /* Set the ith number channel is active, mark the terminal id
     * and increment the number of connections to the capability */
    rgop_data->active_chans |= CHANNEL_MASK(terminal_id);

    return TRUE;
}

bool ringtone_generator_buffer_details(OPERATOR_DATA *op_data, void *message_data,
                                       unsigned *response_id, void **response_data)
{
    unsigned terminal_id = OPMGR_GET_OP_BUF_DETAILS_TERMINAL_ID(message_data);

    if (!base_op_buffer_details(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* We only support limited number of output connections */
    if (!base_op_is_terminal_valid(op_data, terminal_id))
    {
        base_op_change_response_status(response_data, STATUS_INVALID_CMD_PARAMS);
        return TRUE;
    }

#ifdef INSTALL_METADATA
    {
        OP_BUF_DETAILS_RSP *buf_resp;
        RINGTONE_GENERATOR_OP_DATA *rgop_data = get_instance_data(op_data);
        /* If an input/output connection is already present and has metadata then
         * we are obliged to return that buffer so that metadata can be shared
         * between channels. */
        tCbuffer *meta_buff;
        meta_buff = rgop_data->metadata_op_buffer;

        buf_resp = (OP_BUF_DETAILS_RSP*) *response_data;
        buf_resp->metadata_buffer = meta_buff;
        buf_resp->supports_metadata = TRUE;
    }
#endif /* INSTALL_METADATA */

    return TRUE;
}

bool ringtone_generator_disconnect(OPERATOR_DATA *op_data, void *message_data,
                                   unsigned *response_id, void **response_data)
{
    RINGTONE_GENERATOR_OP_DATA *rgop_data = get_instance_data(op_data);
    unsigned terminal_id = OPMGR_GET_OP_DISCONNECT_TERMINAL_ID(message_data);

    if (!base_op_build_std_response_ex(op_data, STATUS_OK, response_data))
    {
        /* Shouldn't change anything if there is not enough memory for the response.*/
        return FALSE;
    }

    /* Check if the channel is valid or not */
    if (!base_op_is_terminal_valid(op_data, terminal_id))
    {
        base_op_change_response_status(response_data, STATUS_INVALID_CMD_PARAMS);
        return TRUE;
    }

    if (opmgr_op_is_running(op_data))
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

    /* make sure you clean up the mono case */
    if (rgop_data->op_chnl_cbuffer[terminal_id] == rgop_data->out_cbuffer)
    {
        rgop_data->out_cbuffer = NULL;
    }
    rgop_data->op_chnl_cbuffer[terminal_id] = NULL;
    rgop_data->active_chans ^= CHANNEL_MASK(terminal_id);

    return TRUE;
}


#ifdef INSTALL_METADATA
static void ringtone_eof_callback(unsigned data)
{
    OPERATOR_DATA *op_data = (OPERATOR_DATA *)(uintptr_t)data;

    common_send_simple_unsolicited_message(op_data, OPMSG_RINGTONE_REPLY_ID_TONE_END);
    pdelete(op_data);
}
#endif


static void ringtone_generator_startup_timer_handler(void *data)
{
    opmgr_kick_operator((OPERATOR_DATA *)data);
}


void ringtone_generator_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    RINGTONE_GENERATOR_OP_DATA *rgop_data = get_instance_data(op_data);
    unsigned before_space, after_space;
    unsigned min_space, i, active_chans;

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

    /* min space over all the active channels */
    min_space = UINT_MAX;
    active_chans = rgop_data->active_chans;
    for (i=0; i<MAX_OUTPUT_CHANS; i++)
    {
        if(active_chans & 01)
        {
            unsigned space;
            space = cbuffer_calc_amount_space_in_words( rgop_data->op_chnl_cbuffer[i]);
            if (min_space > space)
            {
                min_space = space;
            }
        }
        active_chans >>= 1;
    }


    /* Get the space in the buffer before the ringtone ran. */
    before_space = cbuffer_calc_amount_space_in_words(rgop_data->out_cbuffer);

    /* If there is any tone definition here, go to generate some tones */
    if (rgop_data->tone_list.head)
    {
        /* Generate the tone data with the received ringtone definitions;
            this is limited to space in out_cbuffer */
        if (min_space > before_space)
        {
            min_space = before_space;
        }
        ringtone_play(rgop_data, min_space);

        /* Are we reaching the end? */
        if (rgop_data->tone_end)
        {
#ifdef INSTALL_METADATA
            eof_tag = buff_metadata_new_tag();
            if (eof_tag != NULL)
            {
                eof_tag->length = 0;
            }
#endif
            if (!rgop_data->unsolicited_sent)
            {
                bool fallback = TRUE;
#ifdef INSTALL_METADATA
                if (eof_tag != NULL)
                {
                    cb_data = base_op_clone_operator_data(op_data);
                    if (cb_data != NULL)
                    {
                        buff_metadata_add_eof_callback(eof_tag, ringtone_eof_callback,
                                                       (unsigned)(uintptr_t)cb_data);
                        fallback = FALSE;
                    }
                }
#endif
                if (fallback)
                {
                    common_send_simple_unsolicited_message(op_data, OPMSG_RINGTONE_REPLY_ID_TONE_END);
                }
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

    /* Get the space in the buffer after the ringtone has run. */
    after_space = cbuffer_calc_amount_space_in_words(rgop_data->out_cbuffer);
    /* Ringtone should only kick if it has data in the buffer. */
    if (after_space < before_space)
    {
        unsigned generated_words = before_space - after_space;

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

        /* copy generated data to all the active channels
           (not needed for mono) */
        active_chans = rgop_data->active_chans;
        if (pl_one_bit_count(active_chans) > 1 && !rgop_data->tone_end)
        {
            for (i=0; i<MAX_OUTPUT_CHANS; i++)
            {
                if(active_chans & 01)
                {
                    cbuffer_copy( rgop_data->op_chnl_cbuffer[i],
                                    rgop_data->out_cbuffer, generated_words);
                }
                active_chans >>= 1;
                if(active_chans != 0)
                {
                    /* more channels, rewind read pointer */
                    cbuffer_advance_read_ptr( rgop_data->out_cbuffer,
                                     -generated_words);
                }
            }
        }

#ifdef INSTALL_METADATA
        /* Cover the generated samples by a tag. */
        generated_octets = generated_words * OCTETS_PER_SAMPLE;

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

        L4_DBG_MSG5("ringtone tag = 0x%08x, tag->length =  0x%08x, "
                    "generated_octets =  0x%08x, before_index = 0x%08x, "
                    "after_index = 0x%08x",
                    (uintptr_t)(empty_tag), empty_tag->length, generated_octets,
                    before_index, after_index);

        buff_metadata_append(rgop_data->metadata_op_buffer, empty_tag, before_index, after_index);
#endif
        touched->sources = rgop_data->active_chans;
    }
}

/********************************Operator message handlers ********************************************/

bool ringtone_generator_opmsg_config(OPERATOR_DATA *op_data, void *message_data,
                                     unsigned int *response_length,
                                     OP_OPMSG_RSP_PAYLOAD **response_data)
{
    RINGTONE_GENERATOR_OP_DATA *rgop_data = get_instance_data(op_data);
    OP_MSG_REQ *msg = (OP_MSG_REQ *)message_data;

    /* system frequencies are specified adjusted down by this CONVERSION factor */
    unsigned sample_rate = msg->payload[0] * CONVERSION_SAMPLE_RATE_TO_HZ;
    tone_synth_set_sample_rate(&rgop_data->info, sample_rate);

    PL_ASSERT( sample_rate <= UINT_MAX / RINGTONE_OUTPUT_LEN);
    rgop_data->tone_data_block_size = sample_rate * RINGTONE_OUTPUT_LEN / BASE_SAMPLE_RATE;

    return TRUE;
}

bool ringtone_generator_opmsg_tones(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned int *response_length,
                                    OP_OPMSG_RSP_PAYLOAD **response_data)
{
    RINGTONE_GENERATOR_OP_DATA *rgop_data = get_instance_data(op_data);
    OP_MSG_REQ *msg = (OP_MSG_REQ *)message_data;
    unsigned tone_param_len;
    RINGTONE_BUFFER *pnew_tone;

    tone_param_len = OPMGR_GET_OPMSG_LENGTH(msg) -
                     OPMSG_RINGTONE_GENERATOR_TONE_PARAM_TONE_PARAMETERS_WORD_OFFSET;

    /* If the tone length is too big, return false */
    if (tone_param_len > MAX_TONE_LEN)
    {
        return FALSE;
    }

    pnew_tone = (RINGTONE_BUFFER *) xzpmalloc(sizeof(RINGTONE_BUFFER) +
                                              tone_param_len * sizeof(unsigned));
    if (pnew_tone == NULL)
    {
       /* Out of space */
       return FALSE;
    }

    pnew_tone->index = (unsigned*)&(pnew_tone->cur_tone_buf);
    pnew_tone->tone_param_end = (unsigned*)&(pnew_tone->cur_tone_buf[tone_param_len]);
    memcpy(pnew_tone->cur_tone_buf, msg->payload, tone_param_len * sizeof(unsigned));

    /* Put it on the list */
    if (rgop_data->tone_list.head == NULL)
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

    if (rgop_data->tone_end)
    {
        rgop_data->tone_end = FALSE;
    }
    if (rgop_data->unsolicited_sent)
    {
        rgop_data->unsolicited_sent = FALSE;
    }

    if (opmgr_op_is_running(op_data) & (rgop_data->info.tone_info.decay == 0))
    {
        /* Initialise the ringtone generator */
        ringtone_init(rgop_data);
    }

    return TRUE;
}
