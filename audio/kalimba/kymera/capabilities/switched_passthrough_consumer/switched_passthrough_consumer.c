/****************************************************************************
 * Copyright (c) 2017 Qualcomm Technologies International, Ltd
****************************************************************************/
/**
 * \file  switched_passthrough_consumer.c
 * \ingroup  capabilities
 *
 *  Switch between consuming input data and passing though input data to
 *  the output.
 *
 */

#include "capabilities.h"
#include "switched_passthrough_consumer.h"
#include "fault/fault.h"
#include "common/interface/util.h"
#include "platform/pl_assert.h"

typedef enum SPC_MODE
{
    /* No data is passed through all the others consumed. */
    SPC_MODE_CONSUME = 0,
    /* Sink 0 is passed through; all the others consumed. */
    SPC_MODE_PASSTHROUGH_0,
    /* Sink 1 is passed through; all the others consumed. */
    SPC_MODE_PASSTHROUGH_1,
    SPC_MODE_PASSTHROUGH_2,
    SPC_MODE_PASSTHROUGH_3,
    SPC_MODE_PASSTHROUGH_4,
    SPC_MODE_PASSTHROUGH_5,
    SPC_MODE_PASSTHROUGH_6,
    SPC_MODE_PASSTHROUGH_7,
    /* Size of the enum */
    SPC_MODE_SIZE
}SPC_MODE;

#define SPC_NUMBER_INPUTS           (SPC_MODE_SIZE - 1)

typedef struct SPC_OP_DATA
{
    /** Consume data from input buffer. */
    tCbuffer*   ip_buffers[SPC_NUMBER_INPUTS];

    /** Passthrough data to output buffer. */
    tCbuffer*   op_buffer;

    /** Current mode */
    SPC_MODE      current_mode;

    /** Next mode */
    SPC_MODE      next_mode;

    /** The type of data consumed/passed by the capability.*/
    AUDIO_DATA_FORMAT data_format;

    /** The requested size of the output buffer, or 0 for the default size. */
    unsigned output_buffer_size;

} SPC_OP_DATA;

/* A structure containing a set of cbuffer processing function pointers */
struct _cbuffer_functions
{
    unsigned (*space)(tCbuffer *cbuffer);
    unsigned (*data)(tCbuffer *cbuffer);
    unsigned (*copy)(tCbuffer *dst, tCbuffer *src, unsigned to_copy);
    void (*advance_rd_ptr)(tCbuffer *cbuffer, unsigned to_advance);
};

/****************************************************************************
Private Function Definitions
*/
static void spc_process_data(OPERATOR_DATA*, TOUCHED_TERMINALS*);
static bool spc_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
static bool spc_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
static bool spc_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
static bool spc_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
static bool spc_get_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
static bool spc_opmsg_transition_request(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
static bool spc_opmsg_set_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
static bool spc_opmsg_set_buffer_size(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
static bool spc_opmsg_select_passthrough_request(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

static bool spc_connect_disconnect_common(OPERATOR_DATA *op_data, void *message_data, tCbuffer *buffer);

/****************************************************************************
Private Constant Declarations
*/
#ifdef CAPABILITY_DOWNLOAD_BUILD
#define SWITCHED_PASSTHROUGH_CONSUMER_ID CAP_ID_DOWNLOAD_SWITCHED_PASSTHROUGH_CONSUMER
#else
#define SWITCHED_PASSTHROUGH_CONSUMER_ID CAP_ID_SWITCHED_PASSTHROUGH_CONSUMER
#endif

#define SPC_DEFAULT_BLOCK_SIZE 1

/** The stub capability function handler table */
const handler_lookup_struct spc_handler_table =
{
    base_op_create,       /* OPCMD_CREATE */
    base_op_destroy,      /* OPCMD_DESTROY */
    base_op_start,        /* OPCMD_START */
    base_op_stop,         /* OPCMD_STOP */
    base_op_reset,        /* OPCMD_RESET */
    spc_connect,          /* OPCMD_CONNECT */
    spc_disconnect,       /* OPCMD_DISCONNECT */
    spc_buffer_details,   /* OPCMD_BUFFER_DETAILS */
    spc_get_data_format,  /* OPCMD_DATA_FORMAT */
    spc_get_sched_info    /* OPCMD_GET_SCHED_INFO */
};

/* Null terminated operator message handler table - this is the set of operator
 * messages that the capability understands and will attempt to service. */
const opmsg_handler_lookup_table_entry spc_opmsg_handler_table[] =
{
    {OPMSG_COMMON_ID_GET_CAPABILITY_VERSION, base_op_opmsg_get_capability_version},
    {OPMSG_SPC_ID_TRANSITION, spc_opmsg_transition_request},
    {OPMSG_SPC_ID_SET_DATA_FORMAT, spc_opmsg_set_data_format},
    {OPMSG_SPC_ID_SELECT_PASSTHROUGH, spc_opmsg_select_passthrough_request},
    {OPMSG_COMMON_ID_SET_BUFFER_SIZE, spc_opmsg_set_buffer_size},
    {0, NULL}
};


/* Capability data - This is the definition of the capability that Opmgr uses to
 * create the capability from. */
const CAPABILITY_DATA switched_passthrough_consumer_cap_data =
{
    SWITCHED_PASSTHROUGH_CONSUMER_ID,             /* Capability ID */
    1, 4,                               /* Version information - hi and lo parts */
    SPC_NUMBER_INPUTS, 1,               /* Max number of sinks/inputs and sources/outputs */
    &spc_handler_table,                 /* Pointer to message handler function table */
    spc_opmsg_handler_table,            /* Pointer to operator message handler function table */
    spc_process_data,                   /* Pointer to data processing function */
    0,                                  /* Reserved */
    sizeof(SPC_OP_DATA)                 /* Size of capability-specific per-instance data */
};

/* Standard cbuffer function pointers */
const struct _cbuffer_functions cbuffer_functions = {
    cbuffer_calc_amount_space_in_words,
    cbuffer_calc_amount_data_in_words,
    cbuffer_copy,
    cbuffer_advance_read_ptr,
};

/* Octet access (_ex) cbuffer function pointers */
const struct _cbuffer_functions cbuffer_ex_functions = {
    cbuffer_calc_amount_space_ex,
    cbuffer_calc_amount_data_ex,
    cbuffer_copy_16bit_be_zero_shift_ex,
    cbuffer_advance_read_ptr_ex,
};

#if !defined(CAPABILITY_DOWNLOAD_BUILD)
MAP_INSTANCE_DATA(CAP_ID_SWITCHED_PASSTHROUGH_CONSUMER, SPC_OP_DATA)
#else
MAP_INSTANCE_DATA(CAP_ID_DOWNLOAD_SWITCHED_PASSTHROUGH_CONSUMER, SPC_OP_DATA)
#endif /* CAPABILITY_DOWNLOAD_BUILD */

static inline SPC_OP_DATA *get_instance_data(OPERATOR_DATA *op_data)
{
    return (SPC_OP_DATA *) base_op_get_instance_data(op_data);
}

static inline unsigned get_input_number(SPC_MODE mode)
{
    PL_ASSERT(mode > SPC_MODE_CONSUME);
    return mode - 1;
}

/* Data processing function */
static void spc_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    SPC_OP_DATA *opx_data = get_instance_data(op_data);
    const struct _cbuffer_functions *cbuffer;
    unsigned data_size;
    unsigned input_num;

    /* Select which set of cbuffer functions to use. The normal cbuffer functions
       are used to process PCM data. The _ex cbuffer functions are used to process
       other data types (which may contain frames with an odd number of octets). */
    switch (opx_data->data_format)
    {
        case AUDIO_DATA_FORMAT_FIXP:
        case AUDIO_DATA_FORMAT_FIXP_WITH_METADATA: 
            data_size = OCTETS_PER_SAMPLE;
            cbuffer = &cbuffer_functions;
            break; 
        default:
            data_size = 1;
            cbuffer = &cbuffer_ex_functions;
            break;
    }

    if (opx_data->current_mode != opx_data->next_mode)
    {
        /* We need to transition. Either between inputs, or between an 
           input and consume all */

        opx_data->current_mode = opx_data->next_mode;
        L2_DBG_MSG1("*** SPC transition success. Current now %d.",
                        opx_data->current_mode);
    }


    if (opx_data->current_mode > SPC_MODE_CONSUME &&
            opx_data->ip_buffers[get_input_number(opx_data->current_mode)] == NULL)
    {
        /* This was a passthrough input and has been disconnected.
         * Now we are actually consuming all, so change the state.
         */
        opx_data->current_mode = opx_data->next_mode = SPC_MODE_CONSUME;
    }

    tCbuffer *op_buffer = opx_data->op_buffer;
    if (opx_data->current_mode > SPC_MODE_CONSUME &&
            op_buffer == NULL)
    {
        /* Output buffer has been disconnected while a passthrough.
         * BT link might have dropped, so nothing to do about it.
         * Consume all.
         */
        opx_data->current_mode = opx_data->next_mode = SPC_MODE_CONSUME;
    }

    /* Now process all inputs, consuming or passing through as relevant */
    for (input_num = 0; input_num < SPC_NUMBER_INPUTS; input_num++)
    {
        unsigned input_data, data_to_handle, complete_data;
        unsigned buffer_size;
        tCbuffer *ip_buffer;
        bool passthrough = FALSE;
        unsigned touched_sink_mask = 1 << input_num ;

        ip_buffer = opx_data->ip_buffers[input_num];
        if (!ip_buffer)
        {
            /* This input is disconnected. */
            /* Nothing more to do with this */
            continue;
        }

        buffer_size = cbuffer_get_size_in_octets(ip_buffer);
        input_data = cbuffer->data(ip_buffer);
        complete_data = 0;
        data_to_handle = input_data;
        if (opx_data->current_mode > SPC_MODE_CONSUME &&
                input_num == get_input_number(opx_data->current_mode))
        {
            unsigned output_space = cbuffer->space(op_buffer);
            unsigned out_buf_size = cbuffer_get_size_in_octets(op_buffer);
            if (buffer_size > out_buf_size)
            {
                buffer_size = out_buf_size;
            }

            passthrough = TRUE;
            if (output_space < input_data)
            {
                L3_DBG_MSG3("SPC: ### [input%d] input data %d didn't fit in space %d",
                                            input_num, input_data, output_space);
                data_to_handle = output_space;
            }
        }

        if (buff_has_metadata(ip_buffer))
        {
            metadata_tag * mtag = buff_metadata_peek(ip_buffer);
            /* Scan all the available tags and compute the amount of data
             * in the input buffer that corresponds to the complete tags.
             * Only this data will be processed, together with the
             * associated metadata. This way we will always output
             * data aligned to metadata and can switch mode at any time.
             */
            while (mtag != NULL)
            {
                if (mtag->length > buffer_size)
                {
                    /* This tag is bigger than either the input or output buffer.
                     * This is not going to work: warn the user to increase
                     * buffer sizes.
                     */
                    fault_diatribe(FAULT_AUDIO_SPC_TAG_BIGGER_THAN_BUFFER, mtag->length);
                    L2_DBG_MSG4("SPC: [input%d] Tag is too big: %d. "
                            "buffer sizes: in %d, out %d",
                            input_num, mtag->length, ip_buffer->size, op_buffer->size);

                    /* For the time being, just go on, we might be lucky and
                     * keep working in the current mode.
                     */
                    complete_data = buffer_size;
                    break;
                }
                else
                {
                    unsigned tmp = complete_data + mtag->length/data_size;
                    if (tmp > data_to_handle)
                    {
                        /* This is the last tag, for which we haven't received all
                         * the data yet. We will not consider any of its data.
                         */
                        break;
                    }

                    mtag = mtag->next;
                    complete_data = tmp;
                }
            }
        }
        else
        {
            /* No metadata. Process all available data. */
            complete_data = input_data;
        }

        if (complete_data != 0)
        {
            if (passthrough)
            {
                unsigned copied = cbuffer->copy(op_buffer, ip_buffer, complete_data);
                L3_DBG_MSG3("SPC passed through [input%d] %d of %d",
                        input_num, copied, input_data);

                touched->sources |= 1;
                metadata_strict_transport(ip_buffer, op_buffer, copied * data_size);
                if (copied != complete_data)
                {
                    L2_DBG_MSG3("SPC error on [input%d] %d of %d",
                            input_num, copied, input_data);
                }
                touched->sinks |= touched_sink_mask;
            }
            else
            {
                metadata_tag *mtag;
                unsigned b4idx, afteridx;

                L3_DBG_MSG3("SPC discard [input%d] %d of %d",
                        input_num, complete_data, input_data);

                cbuffer->advance_rd_ptr(ip_buffer, complete_data);
                mtag = buff_metadata_remove(ip_buffer, complete_data * data_size, &b4idx, &afteridx);
                buff_metadata_tag_list_delete(mtag);
                touched->sinks |= touched_sink_mask;
            }
        }
    }
}

static bool spc_connect_disconnect_common(OPERATOR_DATA *op_data, void *message_data, tCbuffer *buffer)
{
    SPC_OP_DATA *opx_data = get_instance_data(op_data);
    unsigned terminal_id = ((unsigned*)message_data)[0];
    bool is_sink = terminal_id & TERMINAL_SINK_MASK;

    if (is_sink)
    {
        terminal_id = terminal_id & ~TERMINAL_SINK_MASK;
        if (terminal_id >= SPC_NUMBER_INPUTS)
        {
            /* Invalid input terminal id */
            return FALSE;
        }
        if (buffer == NULL)
        {
            /* An input terminal is being disconnected.
             */

            if (opx_data->next_mode > SPC_MODE_CONSUME &&
                    terminal_id == get_input_number(opx_data->next_mode))
            {
                /* Input is passthrough (or will soon be)
                 * This will be handled in process_data
                 */
                L2_DBG_MSG("SPC Disconnection of Passthrough input.");
            }
        }
        else
        {
            /* An input terminal is being connected. */
        }

        /* Apply the connection/disconnection */
        opx_data->ip_buffers[terminal_id] = buffer;
    }
    else
    {
        if (buffer == NULL)
        {
            /* The output terminal is being disconnected.
             */
            if (opx_data->next_mode != SPC_MODE_CONSUME)
            {
                /* Operator is in passthrough mode (or will soon be).
                 * Cannot passthrough without an output.
                 * This will be handled in process_data
                 */
                L2_DBG_MSG("SPC Disconnection of output during Passthrough.");
                /* If the operator doesn't get kicked (input buffers can be
                 * full) we need to wait for the application to set a mode switch.
                 * That will also kick the operator and consume the inputs.
                 * Add a kick here to remove dependency from apps.
                 */
                opmgr_kick_operator(op_data);
            }
        }
        else
        {
            /* The output terminal is being connected. */
            if (buff_has_metadata(buffer))
            {
                unsigned usable_octets = get_octets_per_word(opx_data->data_format);
                buff_metadata_set_usable_octets(buffer, usable_octets);
            }
        }

        /* Apply the connection/disconnection */
        opx_data->op_buffer = buffer;
    }

    return TRUE;
}

static bool spc_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    tCbuffer *buffer = (tCbuffer*)(((uintptr_t *)message_data)[1]);

    if (!base_op_connect(op_data, message_data, response_id, response_data))
    {
        /* Shouldn't change anything if there is not enough memory for the response.*/
        return FALSE;
    }

    if (!spc_connect_disconnect_common(op_data, message_data, buffer))
    {
         base_op_change_response_status(response_data, STATUS_CMD_FAILED);
         return TRUE;
    }

    return TRUE;
}

static bool spc_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    if (!base_op_disconnect(op_data, message_data, response_id, response_data))
    {
        /* Shouldn't change anything if there is not enough memory for the response.*/
        return FALSE;
    }

    if (!spc_connect_disconnect_common(op_data, message_data, NULL))
    {
         base_op_change_response_status(response_data, STATUS_CMD_FAILED);
         return TRUE;
    }

    return TRUE;
}

static bool spc_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    SPC_OP_DATA *opx_data = get_instance_data(op_data);
    bool result = base_op_buffer_details(op_data, message_data, response_id, response_data);

    if (result)
    {
        unsigned terminal_id = ((unsigned*)message_data)[0];
        OP_BUF_DETAILS_RSP *resp = *response_data;

        if (0 == (terminal_id & TERMINAL_SINK_MASK))
        {
            /* Output, use the output buffer size */
            resp->b.buffer_size = opx_data->output_buffer_size;
        }
        resp->metadata_buffer = NULL;
        resp->supports_metadata = TRUE;
    }
    return result;
}

static bool spc_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    OP_SCHED_INFO_RSP* resp;

    resp = base_op_get_sched_info_ex(op_data, message_data, response_id);
    if (resp == NULL)
    {
        return base_op_build_std_response_ex(op_data, STATUS_CMD_FAILED, response_data);
    }
    *response_data = resp;

    resp->block_size = SPC_DEFAULT_BLOCK_SIZE;

    return TRUE;
}

static bool spc_get_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    SPC_OP_DATA *opx_data = get_instance_data(op_data);
    bool result = base_op_get_data_format(op_data, message_data, response_id, response_data);

    if (result)
    {
        ((OP_STD_RSP*)*response_data)->resp_data.data = opx_data->data_format;
    }
    return result;
}

static bool spc_opmsg_transition_request(OPERATOR_DATA *op_data, void *message_data, 
                                            unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SPC_OP_DATA *opx_data = get_instance_data(op_data);
    OPMSG_SPC_MODE new_mode = (OPMSG_SPC_MODE) OPMSG_FIELD_GET(message_data, OPMSG_SPC_CHANGE_MODE, NEW_MODE);

    if (new_mode == OPMSG_SPC_MODE_PASSTHROUGH)
    {
        opx_data->next_mode = SPC_MODE_PASSTHROUGH_0;
        return TRUE;
    } 
    else if (new_mode == OPMSG_SPC_MODE_CONSUMER)
    {
        opx_data->next_mode = SPC_MODE_CONSUME;
        return TRUE;
    }

    return FALSE;

}

static bool spc_opmsg_select_passthrough_request(OPERATOR_DATA *op_data, void *message_data, 
                                                 unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SPC_OP_DATA *opx_data = get_instance_data(op_data);
    uint16 new_input = OPMSG_FIELD_GET(message_data, OPMSG_SPC_SELECT_PASSTHROUGH, NEW_INPUT);
    SPC_MODE new_mode;
    
    if (new_input >= SPC_MODE_SIZE)
    {
        /* Invalid input */
        return FALSE;
    }

    new_mode = (SPC_MODE)new_input;

    if (opx_data->current_mode != opx_data->next_mode)
    {
        /* We need to wait until the previous mode switch is complete. */
        return FALSE;
    }

    if (new_mode > SPC_MODE_CONSUME)
    {
        /* Switching to passthrough */
        if (opx_data->ip_buffers[get_input_number(new_mode)] == NULL ||
            opx_data->op_buffer == NULL )
        {
            /* Cannot passthrough if selected input or the output
             * is not connected
             */
            return FALSE;
        }
    }

    opx_data->next_mode = new_mode;

    if (opx_data->current_mode != opx_data->next_mode &&
            opmgr_op_is_running(op_data))
    {
        /* A switch is needed. Try to kick the operator to perform the switch. */
        opmgr_kick_operator(op_data);
    }
    return TRUE;
}

static bool spc_opmsg_set_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SPC_OP_DATA *opx_data = get_instance_data(op_data);
    opx_data->data_format = OPMSG_FIELD_GET(message_data, OPMSG_SPC_SET_DATA_FORMAT, DATA_TYPE);
    return TRUE;
}

static bool spc_opmsg_set_buffer_size(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SPC_OP_DATA *opx_data = get_instance_data(op_data);
    opx_data->output_buffer_size = OPMSG_FIELD_GET(message_data, OPMSG_COMMON_SET_BUFFER_SIZE, BUFFER_SIZE);
    return TRUE;
}
