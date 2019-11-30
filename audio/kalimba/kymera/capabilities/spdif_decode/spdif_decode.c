/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
% * \file  spdif_decode.c
 * \ingroup  capabilities
 *
 *  spdif_decode capability implementation. This capability is the main
 *  part of what we call "s/pdif rx driver", it accepts input from one or
 *  two s/pdif rx endpoint and it delivers pcm audio at the output.
 */

#include "capabilities.h"
#include "spdif_decode_c.h"
#include "stream/stream_endpoint.h"

#include "patch/patch.h"

/****************************************************************************
Private Constant Declarations
*/
#ifdef CAPABILITY_DOWNLOAD_BUILD
#define SPDIF_DECODE_CAP_ID CAP_ID_DOWNLOAD_SPDIF_DECODE
#else
#define SPDIF_DECODE_CAP_ID CAP_ID_SPDIF_DECODE
#endif

/** The SPDIF decoder capability function handler table */
const handler_lookup_struct spdif_decode_handler_table =
{
    spdif_decode_create,          /* OPCMD_CREATE */
    base_op_destroy,              /* OPCMD_DESTROY */
    spdif_decode_start,           /* OPCMD_START */
    base_op_stop,                 /* OPCMD_STOP */
    spdif_decode_reset,           /* OPCMD_RESET */
    spdif_decode_connect,         /* OPCMD_CONNECT */
    spdif_decode_disconnect,      /* OPCMD_DISCONNECT */
    spdif_decode_buffer_details,  /* OPCMD_BUFFER_DETAILS */
    spdif_decode_get_data_format, /* OPCMD_DATA_FORMAT */
    spdif_decode_get_sched_info   /* OPCMD_GET_SCHED_INFO */
};

/* Null terminated operator message handler table - this is the set of operator
 * messages that the capability understands and will attempt to service. */
const opmsg_handler_lookup_table_entry spdif_decode_opmsg_handler_table[] =
{
    {OPMSG_COMMON_ID_GET_CAPABILITY_VERSION, base_op_opmsg_get_capability_version},
    {OPMSG_SPDIF_DECODE_ID_SET_SUPPORTED_DATA_TYPES, spdif_decode_set_supported_data_types},
    {OPMSG_SPDIF_DECODE_ID_SET_OUTPUT_RATE, spdif_decode_set_output_rate},
    {OPMSG_SPDIF_DECODE_ID_SET_DECODER_DATA_TYPE, spdif_decode_set_decoder_data_type},
    {OPMSG_SPDIF_DECODE_ID_NEW_CHSTS_FROM_EP, spdif_decode_new_chsts_from_ep},

    {0, NULL}
};

/* Capability data - This is the definition of the capability that Opmgr uses to
 * create the capability from. */
const CAPABILITY_DATA spdif_decode_cap_data = {
    SPDIF_DECODE_CAP_ID,		              /* Capability ID */
    0, 1,				                      /* Version information - hi and lo parts */
    SPDIF_DECODE_SINK_MAX_INPUT_TERMINALS,
    SPDIF_DECODE_SOURCE_MAX_OUTPUT_TERMINALS, /* Max number of sinks/inputs and sources/outputs */
    &spdif_decode_handler_table,	      /* Pointer to message handler function table */
    spdif_decode_opmsg_handler_table,	      /* Pointer to operator message handler function table */
    spdif_decode_process_data,	              /* Pointer to data processing function */
    0,				                          /* TODO: this would hold processing time information */
    sizeof (SPDIF_DECODE_OP_DATA)	          /* Size of capability-specific per-instance data */
};

/**
 * \brief Reports the data format of the requested capability terminal
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the data format request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool spdif_decode_get_data_format (OPERATOR_DATA * op_data, void *message_data,
                              unsigned *response_id, void **response_data)
{
    unsigned terminal_id = ((unsigned *) message_data)[0];

    /* Create the response. If there aren't sufficient resources for this fail
     * early. */
    if (!base_op_build_std_response (STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }

    if (terminal_id & TERMINAL_SINK_MASK)
    {
        /* SINK terminal */
        terminal_id = terminal_id & TERMINAL_NUM_MASK;
        if(SPDIF_DECODE_SINK_IS_EP_TERMINAL(terminal_id))
        {
            /* coming from spdif endpoint */
            ((OP_STD_RSP *) * response_data)->resp_data.data =
                SPDIF_INPUT_DATA_FORMAT;
        }
        else if(SPDIF_DECODE_SINK_IS_DECODER_TERMINAL(terminal_id))
        {
            /* coming from a decoder output */
            ((OP_STD_RSP *) * response_data)->resp_data.data =
                AUDIO_DATA_FORMAT_FIXP;
        }
    }
    else
    {
        if (SPDIF_DECODE_SOURCE_IS_PCM_TERMINAL(terminal_id))
        {
            /* operator's outputs are all PCM channels */
            ((OP_STD_RSP *) * response_data)->resp_data.data =
                AUDIO_DATA_FORMAT_FIXP;
        }
        else if(SPDIF_DECODE_SOURCE_IS_DECODER_TERMINAL(terminal_id))
        {
            /* stream to decoder input */
            ((OP_STD_RSP *) * response_data)->resp_data.data =
                AUDIO_DATA_FORMAT_16_BIT;
        }
    }

    *response_id = OPCMD_DATA_FORMAT;

    return TRUE;
}

/* ********************************** API functions ************************************* */
/**
 * spdif_decode_destroy
 * \brief destroys the spdif_decode op
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the buffer size request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool spdif_decode_create (OPERATOR_DATA * op_data, void *message_data,
                                 unsigned *response_id, void **response_data)
{
    SPDIF_DECODE_OP_DATA *spdif_decode_data;

    /* call base_op create, which also allocates and fills response message */
    if (!base_op_create (op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* now initialise specific working data */
    spdif_decode_data = (SPDIF_DECODE_OP_DATA *) op_data->extra_op_data;
    spdif_decode_data->reset_needed = TRUE;

    return TRUE;
}


/**
 * \brief Starts the spdif_decode capability so decoding will be attempted on a
 * kick.
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the start request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool spdif_decode_start (OPERATOR_DATA * op_data, void *message_data,
                         unsigned *response_id, void **response_data)
{
    SPDIF_DECODE_OP_DATA *spdif_decode_data =
        (SPDIF_DECODE_OP_DATA *) (op_data->extra_op_data);

    *response_id = OPCMD_START;

    /* Create the response. If there aren't sufficient resources for this fail
                                                                              * early. */
    if (!base_op_build_std_response (STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }

    if (OP_RUNNING == op_data->state)
    {
        /* Operator already started nothing to do. */
        return TRUE;
    }

    /* At least one input must be connected,
     * it's the responsibility of the client to make
     * sure the second endpoint is also connected when
     * two-channel config is used
     */
    if (spdif_decode_data->input_buffer == NULL)
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    /* first two pcm output channels must always
     * be connected before starting the operator
     */
    if (spdif_decode_data->output_buffer_pcm_left == NULL
        || spdif_decode_data->output_buffer_pcm_right == NULL)
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    op_data->state = OP_RUNNING;
    return TRUE;
}

/**
 * spdif_decode_reset_operator_state
 * \brief utility function reset the operator to its initial state
 *
 * \param spdif_decode_data Pointer to the operator specific data.
 *
 */
void spdif_decode_reset_operator_state (SPDIF_DECODE_OP_DATA *spdif_decode_data)
{
    spdif_decode_data->reset_needed = TRUE;
    spdif_decode_data->state = SPOS_NORMAL_PCM;
    spdif_decode_data->valid = FALSE;
    spdif_decode_data->op_sample_rate = 0;
    spdif_decode_data->op_data_type=0;
}

/**
 * spdif_decode_reset
 * \brief reset the spdif_decode op
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the buffer size request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool spdif_decode_reset (OPERATOR_DATA * op_data, void *message_data,
                                unsigned *response_id, void **response_data)
{
    SPDIF_DECODE_OP_DATA *spdif_decode_data =
        (SPDIF_DECODE_OP_DATA *) (op_data->extra_op_data);

    if (!base_op_reset (op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* now initialise specific working data */
    spdif_decode_reset_operator_state(spdif_decode_data);

    return TRUE;
}


/**
 * spdif_decode_get_buffer_from_terminal_id
 * \brief helper function get the buffer corresponding to a terminal id
 *
 * \param op_data Pointer to the operator instance data.
 * \param terminal_id terminal id
 * \param can_change whether the buffer can change
 * \return buffer corresponding to a terminal id
 */
tCbuffer **spdif_decode_get_buffer_from_terminal_id(OPERATOR_DATA * op_data,
                                                           unsigned terminal_id,
                                                           bool *can_change)
{
    SPDIF_DECODE_OP_DATA *spdif_decode_data =
        (SPDIF_DECODE_OP_DATA *) (op_data->extra_op_data);
    tCbuffer **bufp = NULL;

    patch_fn_shared(spdif_decode);


    *can_change = TRUE;

    if (terminal_id & TERMINAL_SINK_MASK)
    {
        /* SINK terminal */
        terminal_id = terminal_id & TERMINAL_NUM_MASK;

        /* in RUNNING mode only
         * decoder buffers can change
         */
        if((OP_RUNNING == op_data->state) &&
           !SPDIF_DECODE_SINK_IS_DECODER_TERMINAL(terminal_id))
        {
            *can_change = FALSE;
        }

        if (SPDIF_DECODE_SINK_EP_TERMINAL == terminal_id)
        {
            /* main Endpoint source */
            bufp = &spdif_decode_data->input_buffer;
        }
        else if (SPDIF_DECODE_SINK_EP2_TERMINAL == terminal_id)
        {
            /* second Endpoint source */
            bufp = &spdif_decode_data->input_buffer_b;
        }
        else if(SPDIF_DECODE_SINK_IS_DECODER_TERMINAL(terminal_id))
        {
            /* first Decoder's output */
            /*TODO: check if we are in coded state */
            bufp = &spdif_decode_data->decoded_output_buffers[terminal_id-SPDIF_DECODE_SINK_DECODER_TERMINAL_0];
        }
        else
        {
            bufp = NULL;
        }
    } /* if (terminal_id & TERMINAL_SINK_MASK) */
    else
    {
        /* it's for a SOURCE terminal */
        /* only decoder buffers can change on the fly */
        if((OP_RUNNING == op_data->state) &&
           !SPDIF_DECODE_SOURCE_IS_DECODER_TERMINAL(terminal_id))
        {
            *can_change = FALSE;
        }

        if(SPDIF_DECODE_SOURCE_PCM_TERMINAL_LEFT == terminal_id)
        {
            /* left output channel */
            bufp = &spdif_decode_data->output_buffer_pcm_left;
        }
        else if (SPDIF_DECODE_SOURCE_PCM_TERMINAL_RIGHT == terminal_id)
        {
            /* right output channel */
            bufp = &spdif_decode_data->output_buffer_pcm_right;
        }
        else if (SPDIF_DECODE_SOURCE_IS_DECODER_TERMINAL(terminal_id))
        {
            /* coded output channel */
            bufp = &spdif_decode_data->output_buffer_coded;
        }
        else if(SPDIF_DECODE_SOURCE_IS_PCM_TERMINAL(terminal_id))
        {
            /* extra pcm output channels */
            bufp = &spdif_decode_data->extra_output_buffers[terminal_id-SPDIF_DECODE_SOURCE_PCM_TERMINAL_2];
        }
        else
        {
            bufp = NULL;
        }
    } /* if (terminal_id & TERMINAL_SINK_MASK) */

    return bufp;
}

/**
 * spdif_decode_connect
 * \brief connecting a sink or source terminal of spdif_decode op
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the buffer size request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool spdif_decode_connect (OPERATOR_DATA * op_data, void *message_data,
                                  unsigned *response_id, void **response_data)
{
    SPDIF_DECODE_OP_DATA *spdif_decode_data =
        (SPDIF_DECODE_OP_DATA *) (op_data->extra_op_data);
    unsigned terminal_id = ((unsigned *) message_data)[0];
    tCbuffer **bufp = NULL;
    bool buffer_can_change = FALSE;

    patch_fn_shared(spdif_decode);

    *response_id = OPCMD_CONNECT;
    if (!base_op_build_std_response (STATUS_OK, op_data->id, response_data))
    {
        /* Shouldn't change anything if there is not enough memory for the response. */
        return FALSE;
    }

    /* get the buffer from this terminal id */
    bufp = spdif_decode_get_buffer_from_terminal_id(op_data,
                                                    terminal_id,
                                                    &buffer_can_change);

    /* see if we can change this buffer */
    if(!buffer_can_change)
    {
        /* not in a suitable state to disconnect this buffer */
    	base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    if (!bufp)
    {
        /* invalid terminal number */
    	base_op_change_response_status(response_data, STATUS_INVALID_CMD_PARAMS);

    }
    else if (*bufp)
    {
        /* looks that the terminal already connected */
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
    }
    else
    {
        /* connect the buffer */
        *bufp = (tCbuffer *) (((uintptr_t *) message_data)[1]);
    }

    /* update number of output channels */
    spdif_decode_update_nrof_output_channels(spdif_decode_data);

    return TRUE;

}

/**
 * spdif_decode_disconnect
 * \brief disconnecting a sink or source terminal of spdif_decode op
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the buffer size request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool spdif_decode_disconnect (OPERATOR_DATA * op_data, void *message_data,
                                     unsigned *response_id, void **response_data)
{
    SPDIF_DECODE_OP_DATA *spdif_decode_data =
        (SPDIF_DECODE_OP_DATA *) (op_data->extra_op_data);
    unsigned terminal_id = ((unsigned *) message_data)[0];
    tCbuffer **bufp = NULL;
    bool buffer_can_change = FALSE;

    patch_fn_shared(spdif_decode);

    *response_id = OPCMD_CONNECT;
    if (!base_op_build_std_response (STATUS_OK, op_data->id, response_data))
    {
        /* Shouldn't change anything if there is
         * not enough memory for the response.
         */
        return FALSE;
    }

    /* get the buffer from this terminal id */
    bufp = spdif_decode_get_buffer_from_terminal_id(op_data,
                                                    terminal_id,
                                                    &buffer_can_change);

    /* see if we can change this buffer */
    if(!buffer_can_change)
    {
        /* not in a suitable state to connect this buffer */
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    if (!bufp)
    {
        /* no buffer exists for the requested terminal */
    	base_op_change_response_status(response_data, STATUS_INVALID_CMD_PARAMS);
    }
    else if (*bufp)
    {
        /* disconnect */
        *bufp = NULL;
        if(bufp == &spdif_decode_data->output_buffer_coded)
        {
            /* coded output channel closed */
            spdif_decode_data->current_decoder_data_type = 0;
        }
    }
    else
    {
        /* terminal looks to be not connected */
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
    }

    /* update number of output channels */
    spdif_decode_update_nrof_output_channels(spdif_decode_data);

    return TRUE;
}

/**
 * \brief Reports the buffer requirements of the requested capability terminal
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the buffer size request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool spdif_decode_buffer_details (OPERATOR_DATA * op_data, void *message_data,
                                         unsigned *response_id, void **response_data)
{
    unsigned terminal_id = ((unsigned *) message_data)[0];
    unsigned buf_size = 1;

    if (!base_op_buffer_details
        (op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    if (terminal_id & TERMINAL_SINK_MASK)
    {
        /* SINK terminal */
        terminal_id = terminal_id & TERMINAL_NUM_MASK;
        if(SPDIF_DECODE_SINK_IS_EP_TERMINAL(terminal_id))
        {
            /* EP input terminals */
            SPDIF_DECODE_OP_DATA *spdif_decode_data =
                (SPDIF_DECODE_OP_DATA *) (op_data->extra_op_data);

            buf_size = MAX(SPDIF_DECODE_INPUT_BUFFER_SIZE, spdif_decode_data->min_input_buffer_size);
        }
        else if(SPDIF_DECODE_SINK_IS_DECODER_TERMINAL(terminal_id))
        {
            /* no specific requirements
             * for decoder output channels
             */
        }
    }
    else
    {
        /* SOURCE terminal */
        if (SPDIF_DECODE_SOURCE_IS_PCM_TERMINAL(terminal_id))
        {
            /* PCM outputs */
            buf_size = SPDIF_DECODE_OUTPUT_BUFFER_SIZE;
        }
        else if(SPDIF_DECODE_SOURCE_IS_DECODER_TERMINAL(terminal_id))
        {
            /* coded output */
            buf_size = SPDIF_DECODE_CODED_BUFFER_SIZE;
        }
    }

    ((OP_BUF_DETAILS_RSP *) * response_data)->b.buffer_size = buf_size;
    return TRUE;
}

/**
 * spdif_decode_get_sched_info
 * \brief get schedule info for spdif_decode op
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the buffer size request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool spdif_decode_get_sched_info (OPERATOR_DATA * op_data, void *message_data,
                                         unsigned *response_id, void **response_data)
{
    OP_SCHED_INFO_RSP *resp;

    if (!base_op_get_sched_info
        (op_data, message_data, response_id, response_data))
    {
        return base_op_build_std_response (STATUS_CMD_FAILED, op_data->id,
                                           response_data);
    }
    /* Populate the response */
    base_op_change_response_status(response_data, STATUS_OK);
    resp = *response_data;
    resp->op_id = op_data->id;
    /* Same buffer size for sink and source.
       No additional verification needed. */
    resp->block_size = 0;
    resp->run_period = 0;

    *response_data = resp;
    return TRUE;
}

/**
 * spdif_decode_process_data
 * \brief process function when spdif_decode operator is kicked
 *
 * \param op_data Pointer to the operator instance data.
 * \param touched Location to write which terminals touched at this kick
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
void spdif_decode_process_data (OPERATOR_DATA * op_data, TOUCHED_TERMINALS *touched)
{

    SPDIF_DECODE_OP_DATA *spdif_decode_data;

    patch_fn(spdif_decode_process_data);

    /* spdif_decode op processes data when they become available therefore
     * no influence is needed by this operator to kick the input terminals
     */
    touched->sinks = 0;

    /* reset the touch sources */
    touched->sources = 0;

    /* If we're not running then do nothing. */
    if (OP_NOT_RUNNING == op_data->state)
    {
        return;
    }

    /* get the spdif_decode data */
    spdif_decode_data = (SPDIF_DECODE_OP_DATA *) (op_data->extra_op_data);

    /* Check the input hasn't gone away */
    if (NULL == spdif_decode_data->input_buffer)
    {
        /* This shouldn't happen */
        return;
    }

    /* update the latest state of the operator */
    spdif_decode_process_state (op_data);

    /* see what we should do based on
     * the state of the operator */
    switch(spdif_decode_data->state)
    {
        case SPOS_NORMAL_PCM:
        case SPOS_WAITING_RATE_ADAPT:
            /* process input and see if any output
             * generated, only pcm output will be generated
             */
            if(spdif_frame_decode ((void *) &(spdif_decode_data->valid)))
            {
                /* touch all pcm output terminals */
                touched->sources |= TOUCH_ALL_PCM_OUTPUTS_TERMINALS(spdif_decode_data);
            }
            break;

        case SPOS_NORMAL_CODED:
            /* process input and see if any output
             * generated, only coded output will be generated
             */
            (void)spdif_frame_decode ((void *) &(spdif_decode_data->valid));

            /* kick the decoder if new coded data arrived or,
             * older data haven't been used by the decoder.
             * TODO: optimise this approach
             */
            if(cbuffer_calc_amount_data_in_words(spdif_decode_data->output_buffer_coded))
            {
                /* touch coded output */
                touched->sources |= (1<<SPDIF_DECODE_SOURCE_CODED_TERMINAL);
            }

            /* decoder runs separately, route the decoder
             * outputs to the pcm output channels*/
            if(spdif_copy_channels(spdif_decode_data->decoded_output_buffers,
                                   &spdif_decode_data->output_buffer_pcm_left,
                                   spdif_decode_data->nrof_output_channels, 1024))
            {
                /* touch all pcm output terminals */
                touched->sources |= TOUCH_ALL_PCM_OUTPUTS_TERMINALS(spdif_decode_data);
            }

            break;

        case SPOS_WAITING_DECODER_DONE:
            /* always kick the decoder in this mode */
            touched->sources |= (1<<SPDIF_DECODE_SOURCE_CODED_TERMINAL);

            /* route the decoder output to pcm output channels */
            if(spdif_copy_channels(spdif_decode_data->decoded_output_buffers,
                                   &spdif_decode_data->output_buffer_pcm_left,
                                   spdif_decode_data->nrof_output_channels, 1024))
            {
                /* touch all pcm output terminals */
                touched->sources |= TOUCH_ALL_PCM_OUTPUTS_TERMINALS(spdif_decode_data);
            }
            break;
        default:
            break;

    }
    return;
}

/**
 * spdif_decode_set_output_rate
 * \brief message handler for letting the operator know the new system sample rate
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the start request message
 * \param resp_length pointer to location to write the response message length
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool spdif_decode_set_output_rate(OPERATOR_DATA *op_data, void *message_data,
                                         unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{

    SPDIF_DECODE_OP_DATA *spdif_decode_data = (SPDIF_DECODE_OP_DATA *) (op_data->extra_op_data);

    /* This message is expected to be the response to
     * SPDIF_DECODE_FROM_OP_ADAPT_TO_NEW_RATE, unsolicited sending
     * of this message to the op will cause the output to get
     * mute if the received rate isn't the same as input rate.
     * Now set the output rate
     */

    spdif_decode_data -> output_sample_rate = ((unsigned) OPMSG_FIELD_GET(message_data,
                                                                           OPMSG_SPDIF_DECODE_SET_OUTPUT_RATE,
                                                                           OUTPUT_RATE))*25;

   
    L2_DBG_MSG2("SPDIF_DECODE_OP, set output rate message from to client: time=%08x,  rate=%d",
                hal_get_time(), spdif_decode_data -> output_sample_rate);

    /* if in running mode do a one-off kick */
    if(op_data->state == OP_RUNNING)
    {
        ENDPOINT *terminal0 =
            stream_operator_get_endpoint_from_key (STREAM_EP_OP_SINK|INT_TO_EXT_OPID(op_data->id)|0);

        terminal0->functions->kick(terminal0, STREAM_KICK_FORWARDS);
    }
    return TRUE;
}

/**
 * spdif_decode_new_chsts_from_ep
 * \brief message handler for receiving channel status from endpoint
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the start request message
 * \param resp_length pointer to location to write the response message length
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool spdif_decode_new_chsts_from_ep(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length,
                                           OP_OPMSG_RSP_PAYLOAD **resp_data)
{

    SPDIF_DECODE_OP_DATA *spdif_decode_data = (SPDIF_DECODE_OP_DATA *) (op_data->extra_op_data);
    const unsigned *msg = (const unsigned *)message_data;

    /* currently the op only uses one bit of status
     * The bit that indicates whether the input is PCM or
     * compressed audio. Other bits will be interpreted
     * by the client and the proper action will be performed
     * by the client.
     *
     * TODO: store the whole first 16 bit of channels status
     */
    spdif_decode_data -> chsts_data_mode = (msg[4] >> 1) & 1;

    /* send the channel status to the client,
     * TODO: client might not want to receive full 192 bits of channel
     * status.
     */
    spdif_decode_send_message_to_the_client(op_data,
                                            OPMSG_FROM_OP_SPDIF_DECODE_ID_NEW_CHANNEL_STATUS,
                                            13, /* channel number + 12 status words */
                                            &msg[3]);
    return TRUE;
}

/**
 * spdif_decode_set_decoder_data_type
 * \brief message handler for letting the operator know that the decoder is ready to use
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the start request message
 * \param resp_length pointer to location to write the response message length
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool spdif_decode_set_decoder_data_type(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length,
                                               OP_OPMSG_RSP_PAYLOAD **resp_data)
{

    SPDIF_DECODE_OP_DATA *spdif_decode_data = (SPDIF_DECODE_OP_DATA *) (op_data->extra_op_data);

    /* update new decoder data type */
    spdif_decode_data -> current_decoder_data_type = OPMSG_FIELD_GET(message_data,
                                                                     OPMSG_SPDIF_DECODE_SET_DECODER_DATA_TYPE,
                                                                     DECODER_DATA_TYPE);
    L3_DBG_MSG2("SPDIF_DECODE_OP, set decoder data type message from to client: time=%08x,  type=%d",
                hal_get_time(), spdif_decode_data -> current_decoder_data_type);

    /* store the opid of the decoder operator, no use currently */
    spdif_decode_data -> decoder_opid = OPMSG_FIELD_GET(message_data,
                                                        OPMSG_SPDIF_DECODE_SET_DECODER_DATA_TYPE,
                                                        DECODER_OPID);

    /* store the decoder number of output channels,
     * no use currently */
    spdif_decode_data -> decoder_nrof_output_channels = OPMSG_FIELD_GET(message_data,
                                                        OPMSG_SPDIF_DECODE_SET_DECODER_DATA_TYPE,
                                                        DECODER_NROF_CHANNELS);
    if(op_data->state == OP_RUNNING)
    {
        /* This message is expected to be the response to
         * SPDIF_DECODE_FROM_OP_NEW_DECODER_REQUIRED, it's fine that the
         * client unsolicited sends this message as long as the operator
         * isn't actively receiving coded data.
         */
        ENDPOINT *terminal0 =
            stream_operator_get_endpoint_from_key (STREAM_EP_OP_SINK|INT_TO_EXT_OPID(op_data->id)|0);

        terminal0->functions->kick(terminal0, STREAM_KICK_FORWARDS);
    }

    return TRUE;
}

/**
 * spdif_decode_set_supported_data_types
 * \brief message handler for setting the supported data types
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the start request message
 * \param resp_length pointer to location to write the response message length
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool spdif_decode_set_supported_data_types(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length,
                                                  OP_OPMSG_RSP_PAYLOAD **resp_data)
{

    SPDIF_DECODE_OP_DATA *spdif_decode_data = (SPDIF_DECODE_OP_DATA *) (op_data->extra_op_data);

    L3_DBG_MSG3("SPDIF_DECODE_OP, set supported data data types message from to client: time=%08x,  type=%04x,%0x4x",
                hal_get_time(), ((unsigned *)message_data)[3], ((unsigned *)message_data)[4]);

    if(op_data->state == OP_RUNNING)
    {
        /* supported data types cannot change
         * while the operator is running
         */
        return FALSE;
    }

    /* we can support up to 32 coded data types,
     * two 16-bit words is used to store bitmap of
     * supported data types so each bit will be
     * corresponding to supporting a specific type
     * of data.
     *
     * When receiving a coded data type,
     * if corresponding bit in supported_data_types
     * is cleared the stream will be muted, if however
     * the bit is set it will be decoded if we have
     * a suitable decoder for that type of data.
     */

    /* set the supported data types group1 */
    spdif_decode_data -> supported_data_types[0] = OPMSG_FIELD_GET(message_data,
                                                                   OPMSG_SPDIF_DECODE_SET_SUPPORTED_DATA_TYPES,
                                                                   SUPPORTED_DATA_TYPES_GROUP1);
        /* set the supported data types group2 */
    spdif_decode_data -> supported_data_types[1] = OPMSG_FIELD_GET(message_data,
                                                                   OPMSG_SPDIF_DECODE_SET_SUPPORTED_DATA_TYPES,
                                                                   SUPPORTED_DATA_TYPES_GROUP2);

    /* we support one user-specific data type,
     * see if it is enabled
     */
    if(spdif_decode_data -> supported_data_types[1] & (1<<15))
    {
        /* set the expected period for the specific data type */
        spdif_decode_data -> user_data_type_period = OPMSG_FIELD_GET(message_data,
                                                                   OPMSG_SPDIF_DECODE_SET_SUPPORTED_DATA_TYPES,
                                                                   USER_DATA_TYPE_PERIOD);
    }
    else
    {
        spdif_decode_data -> user_data_type_period = 0;
    }

    return TRUE;
}

/**
 * spdif_decode_update_nrof_output_channels
 * \brief updates number of output channels
 *
 * \param spdif_decode_data Pointer to the spdif_decode data
 */
void spdif_decode_update_nrof_output_channels(SPDIF_DECODE_OP_DATA *spdif_decode_data)
{
    tCbuffer** buf = spdif_decode_data->extra_output_buffers;
    unsigned num = 2; /* L and R channels */
    while((*buf) && (num<SPDIF_DECODE_MAX_OUTPUT_CHANNELS))
    {
        num++;
        buf++;
    }
    spdif_decode_data -> nrof_output_channels = num;
}

/**
 * spdif_decode_send_message_to_the_client
 * \brief sends message from the operator to the client
 *
 * \param op_data Pointer to the operator instance data.
 * \param msg_id ID of the message
 * \param length length of the message
 * \param payload message payload
 */
void spdif_decode_send_message_to_the_client(OPERATOR_DATA *op_data, unsigned msg_id, unsigned length, const unsigned *payload)
{
    if(!common_send_unsolicited_message(op_data, msg_id, length, payload))
    {
        /* for some reason failed to start sending the message,
         * e.g. because of not having enough memory.
         *
         * Doesn't look we need anything to do, important messages like
         * OPMSG_FROM_OP_SPDIF_DECODE_ID_ADAPT_TO_NEW_RATE will be retried
         * after sometime, failure of other messages isn't fatal although
         * it will impact the performance of overall system.
         */
    L3_DBG_MSG4("SPDIF_DECODE_OP, Sending Message to client FAILED: time=%08x,  id=%04x, data1=%4x, data2=%04x", hal_get_time(), msg_id,
                length>0?payload[0]:0xFFFF, length>1?payload[1]:0xFFFF);

    }

    L3_DBG_MSG4("SPDIF_DECODE_OP, Message sent to client: time=%08x,  id=%04x, data1=%4x, data2=%04x", hal_get_time(), msg_id,
                length>0?payload[0]:0xFFFF, length>1?payload[1]:0xFFFF);
}

/**
 * spdif_decode_signal_reset_to_decoder
 * \brief asks decoder to reset itself before starting to decode
 *
 * \param spdif_decode_data Pointer to the spdif data decode structure
 */
void spdif_decode_signal_reset_to_decoder(SPDIF_DECODE_OP_DATA *spdif_decode_data)
{
    /* TODO:
     * - clean the input buffer
     * - somehow let the decoder know that it needs
     *   to reset itself before first decode
     */
    UNUSED(spdif_decode_data);
}

/**
 * spdif_decode_signal_end_of_input_to_decoder
 * \brief called when there is no more coded input for the
 * current decoder.
 *
 * \param spdif_decode_data Pointer to the spdif data decode structure
 */
void spdif_decode_signal_end_of_input_to_decoder(SPDIF_DECODE_OP_DATA *spdif_decode_data)
{
    /* TODO: somehow let the decoder know that it
     * wont have any more input
     */
    UNUSED(spdif_decode_data);
}

/**
 * spdif_decode_unsupport
 * \brief remove a specific types from supported data types
 *
 * \param spdif_decode_data Pointer to the spdif data decode structure
 * \param data_type data type to remove from the supported list
 */
static void spdif_decode_unsupport(SPDIF_DECODE_OP_DATA *spdif_decode_data, SPDIF_DATA_TYPE data_type)
{
    /* ignore if this is a PCM type */
    if(SPDIF_DATA_TYPE_IS_PCM(data_type))
        return;

    /* user can only set the supported data types, however if for some reason we cannot
     * decode a specific type, e.g. because there is no suitable decoder capability, or
     * if the decoder cannot be loaded due to lack of resource, then we remove the that
     * specific data type from supported list.
     */

    /* see if it is in first group,
     * see SPDIF_DATA_TYPE, first 3 are for PCM */
    data_type = data_type - 3;
    if(data_type <16)
    {
        /* it's for fist group, remove it from fist group */
        spdif_decode_data -> supported_data_types[0] &= ~(1<<data_type);
        return;
    }

    /* see if it is in second group */
    data_type = data_type - 16;
    if(data_type <16)
    {
        /* it's for second group, remove it from second group */
        spdif_decode_data -> supported_data_types[1] &= ~(1<<data_type);
        return;
    }

    /* any other value is ignored*/
    return;
}

/**
 * spdif_decode_report_sample_rate_change
 * \brief reports any changes in the sample rate
 *
 * \param op_data Pointer to the operator instance data.
 * \return whether the sample rate change reported
 */
static bool spdif_decode_report_sample_rate_change(OPERATOR_DATA * op_data)
{
    unsigned ep_sample_rate;
    uint32 val32;
    bool valid;

    SPDIF_DECODE_OP_DATA *spdif_decode_data =
        (SPDIF_DECODE_OP_DATA *) (op_data->extra_op_data);

    ENDPOINT *spdif_ep =
        stream_operator_get_endpoint_from_key (STREAM_EP_OP_SINK|INT_TO_EXT_OPID(op_data->id)|0)->connected_to;

    /* get the latest sample rate */
    stream_get_endpoint_config(spdif_ep, EP_SAMPLE_RATE, &val32);
    ep_sample_rate = (unsigned) val32;

    /* make sure it is a multiple of 25, this should be always the case given the sample rates we currently support */
    ep_sample_rate = (ep_sample_rate / 25) * 25;

    /* 0 mean the input is invalid for some reason */
    valid =  ep_sample_rate != 0;

    /* see if validity of the input just changed */
    if(spdif_decode_data->valid != valid)
    {
        unsigned msg_sample_rate = (ep_sample_rate / 25);

        if(valid)
        {
            /* just started to receive new valid input,
             * we need to reset the process module
             */
            spdif_decode_data->reset_needed = TRUE;
        }
        else
        {

            if(SPDIF_DATA_TYPE_IS_PCM(spdif_decode_data->op_data_type))
            {
                /* consume the last bits of the input */
                (void)spdif_frame_decode ((void *) &(spdif_decode_data->valid));

                /* input just became invalid, try to fade out outputs if
                 * still there are some samples there
                 */
                spdif_soft_mute_output_buffers(
                    &spdif_decode_data->output_buffer_pcm_left, /* start from left channel */
                    2,                                          /* L & R channels only, it's PCM stereo */
                    256,                                        /* number of samples to fade */
                    0);                                         /* if more samples in the buffer, it starts from this offset to fade
                                                                 * and the tail will be silenced */

            }
            else
            {
                /* rate has changed during coded data, copy the decoder output
                 * and then fade out output channels.
                 */
                spdif_copy_channels(spdif_decode_data->decoded_output_buffers,
                                     &spdif_decode_data->output_buffer_pcm_left,
                                    spdif_decode_data->nrof_output_channels, 1024);

                /* input just became invalid, try to fade out outputs if
                 * still there are some samples there
                 */
                spdif_soft_mute_output_buffers(
                    &spdif_decode_data->output_buffer_pcm_left, /* start from left channel */
                    spdif_decode_data->nrof_output_channels,    /* all channels, coded stream can produce multi-channel output */
                    256,                                        /* number of samples to fade */
                    0);                                         /* if more samples in the buffer, it starts from this offset to fade
                                                                 * and the tail will be silenced */

            }
        }
        spdif_decode_data->valid = valid;
        /* inform the client that input becomes valid or invalid,
         * This op doesn't require client to do any action/response
         * to this message but the client might need it for other reasons.
         */

        L2_DBG_MSG1 ("OPMSG_FROM_OP_SPDIF_DECODE_ID_INPUT_RATE_VALID: %d  ", msg_sample_rate);

        spdif_decode_send_message_to_the_client(op_data, OPMSG_FROM_OP_SPDIF_DECODE_ID_INPUT_RATE_VALID,
                                                1, &msg_sample_rate);
    }

    if(spdif_decode_data->valid)
    {
        spdif_decode_data->op_sample_rate = (unsigned) ep_sample_rate;
    }

    /* see if rate has changed */
    if (spdif_decode_data->op_sample_rate !=
        spdif_decode_data->output_sample_rate)
    {
        /* sample rate has changed, client needs to adapt*/
        unsigned msg_sample_rate = (unsigned) (spdif_decode_data->op_sample_rate / 25);
        spdif_decode_data->valid = FALSE;
        spdif_decode_data->state = SPOS_WAITING_RATE_ADAPT;

        L2_DBG_MSG1 ("OPMSG_FROM_OP_SPDIF_DECODE_ID_ADAPT_TO_NEW_RATE: %d  ", msg_sample_rate);
        
        spdif_decode_send_message_to_the_client(op_data, OPMSG_FROM_OP_SPDIF_DECODE_ID_ADAPT_TO_NEW_RATE, 1, &msg_sample_rate);
        spdif_decode_data->timeout_start_time = hal_get_time();
        return TRUE;
    }
    return FALSE;
}

/**
 * spdif_decode_process_state
 * \brief updates the latest state of the operator
 *
 * \param op_data Pointer to the operator instance data.
 */
void spdif_decode_process_state (OPERATOR_DATA * op_data)
{

    SPDIF_DECODE_OP_DATA *spdif_decode_data =
        (SPDIF_DECODE_OP_DATA *) (op_data->extra_op_data);

    SPDIF_OP_STATE prev_state = spdif_decode_data->state;
    TIME current_time = hal_get_time();

    switch (spdif_decode_data->state)
    {
        case SPOS_NORMAL_PCM:
            /* handle sample rate change, if sample rate has changed
             * process of other events is postponed until the system is
             * adapted to new rate*/
            if(spdif_decode_report_sample_rate_change(op_data))
                break;

            /* see if the data type is still PCM format */
            if(!SPDIF_DATA_TYPE_IS_PCM(spdif_decode_data->op_data_type))
            {

                /* we fade out whatever we already have in the output channels */
                spdif_soft_mute_output_buffers(
                    &spdif_decode_data->output_buffer_pcm_left, /* start from left channel */
                    2,                                          /* L & R channels only, it's PCM stereo only */
                    128,                                        /* number of samples to fade, less samples to fade since
                                                                 * this most probably is already faded in the source */
                    256);                                       /* mostly fading the tail of the buffer not the head, so start
                                                                 * from a big offset */

                if(spdif_decode_data->current_decoder_data_type == spdif_decode_data->op_data_type)
                {
                    /* data type isn't pcm anymore, but a suitable
                     * decoder is already available we only need
                     * to reset the decoder
                     */
                    spdif_decode_signal_reset_to_decoder(spdif_decode_data);
                    spdif_decode_data->state = SPOS_NORMAL_CODED;
                }
                else
                {
                    /* new decoder is needed, so we need to ask
                     * the client to load it.
                     */
                    spdif_decode_data->state = SPOS_WAITING_DATA_TYPE_ADAPT;
                    spdif_decode_send_message_to_the_client(op_data, OPMSG_FROM_OP_SPDIF_DECODE_ID_NEW_DECODER_REQUIRED,
                                                            1, (unsigned *) &spdif_decode_data->op_data_type);
                    spdif_decode_data->timeout_start_time = hal_get_time();

                }
            }
            break;

        case SPOS_NORMAL_CODED:
            /* handle sample rate change, if sample rate has changed
             * process of other events is postponed until the system is
             * adapted to new rate
             */
            if(spdif_decode_report_sample_rate_change(op_data))
                break;

            else if(spdif_decode_data->op_data_type != spdif_decode_data->current_decoder_data_type)
            {
                /* Data type changed, we need to wait until the
                 * decoder is done with its current input
                 */
                spdif_decode_data->state = SPOS_WAITING_DECODER_DONE;
                spdif_decode_data->decoder_done = FALSE;
                spdif_decode_data->timeout_start_time = hal_get_time();
                spdif_decode_signal_end_of_input_to_decoder(spdif_decode_data);
            }
            break;

        case SPOS_WAITING_RATE_ADAPT:
            if (spdif_decode_data->op_sample_rate ==
                spdif_decode_data->output_sample_rate)
            {
                /* system is now ready for new rate */
                spdif_decode_data->state = SPOS_NORMAL_PCM;
            }
            else if(time_sub(current_time, spdif_decode_data->timeout_start_time) >= SPDIF_DECODE_MAX_WAIT_FOR_CLIENT_RESPONSE)
            {
                /* it's relatively long time and the client
                 * hasn't responded, it could be that the message wasn't
                 * delivered, and/or the client wasn't responsive at that
                 * time, so we need to resend it again if still needed.
                 */
                spdif_decode_reset_operator_state(spdif_decode_data);
            }
            break;

        case SPOS_WAITING_DECODER_DONE:
            if(spdif_decode_data->decoder_done ||
               (time_sub(current_time, spdif_decode_data->timeout_start_time) >= SPDIF_DECODE_MAX_WAIT_FOR_DECODER_DONE))
            {
                /* decoder is done, soft mute the output */
                spdif_soft_mute_output_buffers(
                    &spdif_decode_data->output_buffer_pcm_left, /* start from left channel */
                    spdif_decode_data->nrof_output_channels,    /* all channels, coded stream can produce multi-channel output */
                    64,                                         /* number of samples to fade, less samples to fade since
                                                                 * this most probably is already faded in the source */
                    1024);                                      /* mostly fading the tail of the buffer not the head, quite
                                                                 * possible that decoder has generated a big chunk of real audio
                                                                 * so we fade just a bit at the end of that */


                if (SPDIF_DATA_TYPE_IS_PCM (spdif_decode_data->op_data_type))
                {
                    /* we can now output pcm audio */
                    spdif_decode_data->state = SPOS_NORMAL_PCM;
                }
                else if(spdif_decode_data->current_decoder_data_type == spdif_decode_data->op_data_type)
                {
                    /* we already have the decoder,
                     * so we start producing coded data
                     */
                    spdif_decode_data->state = SPOS_NORMAL_CODED;
                }
                else
                {
                    /* new decoder is needed, so we need to ask
                     * the client to load it. */
                    spdif_decode_data->state = SPOS_WAITING_DATA_TYPE_ADAPT;
                    spdif_decode_data -> decoder_opid = 0;
                    spdif_decode_send_message_to_the_client(op_data, OPMSG_FROM_OP_SPDIF_DECODE_ID_NEW_DECODER_REQUIRED,
                                                            1, (unsigned *) &spdif_decode_data->op_data_type);
                    spdif_decode_data->timeout_start_time = hal_get_time();

                }
            }
            break;

        case SPOS_WAITING_DATA_TYPE_ADAPT:
            if (spdif_decode_data->op_data_type ==
                spdif_decode_data->current_decoder_data_type)
            {
                /* client has loaded the required decoder successfully
                 * and it's ready to use
                 */
                spdif_decode_data->state = SPOS_NORMAL_CODED;
            }
            else if(SPDIF_DECODE_DECODER_LOAD_FAILED(spdif_decode_data->op_data_type,spdif_decode_data->current_decoder_data_type))
            {
                /* Client has told us that this data type can't be supported, either it failed
                 * to load it or it has decided that it doesn't need to support this data type
                 * we can't support this data type, so we remove it from supported data types list
                 */
                spdif_decode_unsupport(spdif_decode_data, spdif_decode_data->op_data_type);
                spdif_decode_data->reset_needed = TRUE;
                spdif_decode_data->state = SPOS_NORMAL_PCM;
            }
            else if(time_sub(current_time, spdif_decode_data->timeout_start_time) >= SPDIF_DECODE_MAX_WAIT_FOR_CLIENT_RESPONSE)
            {
                /* it's relatively long time and the client
                 * hasn't responded, it could be that the message wasn't
                 * delivered, and/or the client wasn't responsive at that
                 * time, so we need to resend it again if still needed.
                 */
                spdif_decode_reset_operator_state(spdif_decode_data);
            }
            break;

        default:
            /* TODO: panic is more suitable */
            break;
    }

    if(prev_state != spdif_decode_data->state)
    {
        /* if we have switched to a long term state
         * we tell the client
         */
        if(spdif_decode_data->state == SPOS_NORMAL_PCM ||
           spdif_decode_data->state == SPOS_NORMAL_CODED)
        {
            tCbuffer** bufs =  spdif_decode_data->decoded_output_buffers;
            unsigned num = 0;
            /* inform the client about new data type
             * This op doesn't require client to do any action/response
             * to this message but the client might need it for other reasons.
             */
            spdif_decode_send_message_to_the_client(op_data, OPMSG_FROM_OP_SPDIF_DECODE_ID_NEW_DATA_TYPE,
                                                    1, (unsigned *) &spdif_decode_data->op_data_type);
            spdif_decode_data->valid = TRUE;
            if(spdif_decode_data->output_buffer_coded)
            {
                /* Clear the encoded buffer */
                cbuffer_empty_buffer(spdif_decode_data->output_buffer_coded);
            }

            /* at this point all the decoder output buffers shall be empty,
             * but explicitly clear them to prevent glitches */
            while((*bufs) && (num<SPDIF_DECODE_MAX_OUTPUT_CHANNELS))
            {
                cbuffer_empty_buffer(*bufs);
                num++;
                bufs++;
            }
        }
    }
    return;
}
