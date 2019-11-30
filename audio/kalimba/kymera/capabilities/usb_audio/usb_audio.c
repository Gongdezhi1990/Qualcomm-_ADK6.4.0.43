/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
 ****************************************************************************/
/**
 * \file  usb_audio.c
 * \ingroup  capabilities
 *
 *  Implementation of usb_audio_rx and usb_audio_tx capabilities. Neither of these
 *  capabilities can work in-place.
 *
 *  Except for 'capability data' structure, these two capabilities share the
 *  definition of all API functions and data tables.
 *
 *  usb_audio_rx input terminal can only connect to a usb_audio source endpoint,
 *  similarly the output of usb_audio_rx can connect to usb_audio sink endpoint only.
 */

#include "capabilities.h"
#include "usb_audio_c.h"
#include "buffer_interleave.h"
#include "stream/stream_for_usb_audio_operator.h"
#include "patch/patch.h"
#include "pl_assert.h"
#include "op_msg_helpers.h"
#include "usb_audio_gen_c.h"
#include "mem_utils/scratch_memory.h"
#include "mem_utils/shared_memory_ids.h"
#include "platform/pl_fractional.h"
/****************************************************************************
Private Constant Declarations
*/

#ifdef CAPABILITY_DOWNLOAD_BUILD
#define USB_AUDIO_RX_CAP_ID CAP_ID_DOWNLOAD_USB_AUDIO_RX
#define USB_AUDIO_TX_CAP_ID CAP_ID_DOWNLOAD_USB_AUDIO_TX
#else
#define USB_AUDIO_RX_CAP_ID CAP_ID_USB_AUDIO_RX
#define USB_AUDIO_TX_CAP_ID CAP_ID_USB_AUDIO_TX
#endif

/** The usb_audio capability function handler table */
const handler_lookup_struct usb_audio_handler_table =
{
    usb_audio_create,          /* OPCMD_CREATE */
    usb_audio_destroy,         /* OPCMD_DESTROY */
    usb_audio_start,           /* OPCMD_START */
    usb_audio_stop,            /* OPCMD_STOP */
    usb_audio_reset,           /* OPCMD_RESET */
    usb_audio_connect,         /* OPCMD_CONNECT */
    usb_audio_disconnect,      /* OPCMD_DISCONNECT */
    usb_audio_buffer_details,  /* OPCMD_BUFFER_DETAILS */
    usb_audio_get_data_format, /* OPCMD_DATA_FORMAT */
    usb_audio_get_sched_info   /* OPCMD_GET_SCHED_INFO */
};

/* Null terminated operator message handler table - this is the set of operator
 * messages that the capability understands and will attempt to service. */
const opmsg_handler_lookup_table_entry usb_audio_opmsg_handler_table[] =
{
    {OPMSG_COMMON_ID_GET_CAPABILITY_VERSION, base_op_opmsg_get_capability_version},
    {OPMSG_USB_AUDIO_ID_SET_CONNECTION_CONFIG, usb_audio_opmsg_set_connection_config},
    {OPMSG_COMMON_ID_SET_BUFFER_SIZE, usb_audio_opmsg_set_buffer_size},
#ifdef USB_AUDIO_SUPPORT_METADATA
    {OPMSG_COMMON_SET_TTP_LATENCY, usb_audio_opmsg_set_ttp_latency},
    {OPMSG_COMMON_SET_LATENCY_LIMITS, usb_audio_opmsg_set_latency_limits},
    {OPMSG_COMMON_SET_TTP_PARAMS,  usb_audio_opmsg_set_ttp_params},
#endif /* #ifdef USB_AUDIO_SUPPORT_METADATA */
    /* {OPMSG_AD2P_DEC_ID_CONTENT_PROTECTION_ENABLE, sbc_dec_opmsg_content_protection_enable}, */
    {OPMSG_COMMON_ID_SET_CONTROL,                  usb_audio_opmsg_obpm_set_control},
    {OPMSG_COMMON_ID_GET_PARAMS,                   usb_audio_opmsg_obpm_get_params},
    {OPMSG_COMMON_ID_GET_DEFAULTS,                 usb_audio_opmsg_obpm_get_defaults},
    {OPMSG_COMMON_ID_SET_PARAMS,                   usb_audio_opmsg_obpm_set_params},
    {OPMSG_COMMON_ID_GET_STATUS,                   usb_audio_opmsg_obpm_get_status},
    {0, NULL}
};

/* Capability data - This is the definition of the capability that Opmgr
 * uses to create the USB_AUDIO_RX capability from.
 */
const CAPABILITY_DATA usb_audio_rx_cap_data =
{
    USB_AUDIO_RX_CAP_ID,            /* Capability ID */
    USB_AUDIO_USB_AUDIO_RX_VERSION_MAJOR, 1, /* Version information - hi and lo parts */
    1, USB_AUDIO_OP_MAX_CHANNELS,   /* Max number of sinks/inputs and sources/outputs */
    &usb_audio_handler_table,       /* Pointer to message handler function table */
    usb_audio_opmsg_handler_table,  /* Pointer to operator message handler function table */
    usb_audio_process_data,         /* Pointer to data processing function */
    0,                              /* TODO: this would hold processing time information */
    sizeof (USB_AUDIO_OP_DATA)      /* Size of capability-specific per-instance data */
};

/* Capability data - This is the definition of the capability that Opmgr
 * uses to create the USB_AUDIO_TX capability from.
 */
const CAPABILITY_DATA usb_audio_tx_cap_data =
{
    USB_AUDIO_TX_CAP_ID,           /* Capability ID */
    USB_AUDIO_USB_AUDIO_TX_VERSION_MAJOR, 1, /* Version information - hi and lo parts */
    USB_AUDIO_OP_MAX_CHANNELS, 1,  /* Max number of sinks/inputs and sources/outputs */
    &usb_audio_handler_table,      /* Pointer to message handler function table */
    usb_audio_opmsg_handler_table, /* Pointer to operator message handler function table */
    usb_audio_process_data,        /* Pointer to data processing function */
    0,                             /* TODO: this would hold processing time information */
    sizeof (USB_AUDIO_OP_DATA)     /* Size of capability-specific per-instance data */
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
bool usb_audio_get_data_format (OPERATOR_DATA * op_data, void *message_data,
                                unsigned *response_id, void **response_data)
{

    /* get the operator specific data structure */
    USB_AUDIO_OP_DATA *usb_audio_data = (USB_AUDIO_OP_DATA *) op_data->extra_op_data;
    unsigned terminal_id = ((unsigned *) message_data)[0];

    /* Create the response. If there aren't sufficient resources
     * for this fail early. */
    if (!base_op_build_std_response (STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }

    if(USB_AUDIO_TERMINAL_IS_USB_AUDIO_EP(usb_audio_data, terminal_id))
    {
        /* this is for endpoint side of the operator,
         * it only accepts USB data format
         */
        ((OP_STD_RSP *) * response_data)->resp_data.data = USB_AUDIO_DATA_FORMAT;
    }
    else
    {
        /* the other side will always be in PCM fixed point format
         *
         * TODO NOTE: when we support coded data over usb in future
         * then this will change
         */
        ((OP_STD_RSP *) * response_data)->resp_data.data = AUDIO_DATA_FORMAT_FIXP;
    }

    *response_id = OPCMD_DATA_FORMAT;

    return TRUE;
}

/**
 * usb_audio_opmsg_set_connection_config
 * \brief message handler when sending connection config info to the operator
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the start request message
 * \param resp_length pointer to location to write the response message length
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool usb_audio_opmsg_set_connection_config(OPERATOR_DATA *op_data, void *message_data,
                                           unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{

    USB_AUDIO_OP_DATA *usb_audio_data = (USB_AUDIO_OP_DATA *) (op_data->extra_op_data);
    bool config_valid = TRUE;

    /* only allow this when the op isn't running */
    if(OP_RUNNING == op_data->state)
    {
        return FALSE;
    }

    /* also we cannot change the config while the endpoint side is connected */
    if(usb_audio_data->usb_audio_buf)
    {
        return FALSE;
    }

    /* set the usb data format */
    usb_audio_data->data_format = OPMSG_FIELD_GET(message_data, OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG, DATA_FORMAT);
    if(usb_audio_data->data_format != UADF_T1_PCM)
    {
        /* currently only linear PCM is supported */
        config_valid = FALSE;
    }

    /* set sample rate */
    usb_audio_data->sample_rate = (((unsigned) OPMSG_FIELD_GET(message_data, OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG, SAMPLE_RATE))*25);
    if(usb_audio_data->sample_rate < USB_AUDIO_MIN_SAMPLE_RATE ||
       usb_audio_data->sample_rate > USB_AUDIO_MAX_SAMPLE_RATE)
    {
        /* a sanity check on sample rate failed */
        config_valid = FALSE;
    }

    /* set number of channels */
    usb_audio_data->nrof_channels = OPMSG_FIELD_GET(message_data, OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG, NROF_CHANNELS);
    if(usb_audio_data->nrof_channels > USB_AUDIO_OP_MAX_CHANNELS ||
       !usb_audio_data->nrof_channels)
    {
        config_valid = FALSE;
    }

    /* set subframe size in bits */
    usb_audio_data->subframe_size = OPMSG_FIELD_GET(message_data, OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG, SUBFRAME_SIZE);
    if(usb_audio_data->subframe_size !=16
       && usb_audio_data->subframe_size != 24)
    {
        /* only 16 and 24 bit config are supported */
        config_valid = FALSE;
    }

    /* set bit resolution */
    usb_audio_data->bit_resolution = OPMSG_FIELD_GET(message_data, OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG, BIT_RESOLUTION);
    if(!usb_audio_data->bit_resolution)
    {
        /* 0 is interpreted as 'same as subframe size'*/
        usb_audio_data->bit_resolution = usb_audio_data->subframe_size;
    }

    if(usb_audio_data->bit_resolution != usb_audio_data->subframe_size)
    {
        /* number of valid bits in each subframe can be less than subframe
         * size, but we don't support it (no need to waste the throughput) */
        config_valid = FALSE;
    }

    if(!config_valid)
    {
        /* user has sent us a wrong config or a config that we cannot support,
         * it will be informed however we also invalidate current config
         * that we have as we don't know whether we can continue using the current config.
         */
        usb_audio_data->op_configured = FALSE;
        return FALSE;
    }

#ifdef USB_AUDIO_SUPPORT_METADATA
    if(USB_AUDIO_IS_RX_OPERATOR(usb_audio_data))
    {
        /* rate is known now, update the ttp context */
        ttp_configure_rate(usb_audio_data->time_to_play, usb_audio_data->sample_rate);
    }
    else
    {
        /* set the sample rate of the sra_mtrans struct */
        usb_audio_data->sra_mtrans.sample_rate = usb_audio_data->sample_rate;
    }
#endif /* #ifdef USB_AUDIO_SUPPORT_METADATA */

    /* all done */
    usb_audio_data->op_configured = TRUE;

    return TRUE;
}

/**
 * \brief Starts the usb_audio capability so decoding will be attempted on a
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
bool usb_audio_start (OPERATOR_DATA * op_data, void *message_data,
                      unsigned *response_id, void **response_data)
{
    USB_AUDIO_OP_DATA *usb_audio_data =
        (USB_AUDIO_OP_DATA *) (op_data->extra_op_data);
    unsigned chan;

    *response_id = OPCMD_START;

    /* Create the response. If there aren't sufficient resources for this fail early. */
    if (!base_op_build_std_response (STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }

    if (OP_RUNNING == op_data->state)
    {
        /* Operator already started nothing to do. */
        return TRUE;
    }

    /* The endpoint terminal must be connected
     */
    if (!usb_audio_data->usb_audio_buf)
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    /*  all the pcm channels must be connected
     *  Note: nrof_channels is checked to be between 1
     *  and USB_AUDIO_OP_MAX_CHANNELS at the time of connection
     */
    for(chan=0; chan < usb_audio_data->nrof_channels; ++chan)
    {
        if(chan >= USB_AUDIO_OP_MAX_CHANNELS ||
           usb_audio_data->pcm_channel_buf[chan] == NULL)
        {
            base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            return TRUE;

        }
    }

#ifdef USB_AUDIO_SUPPORT_METADATA

    if(USB_AUDIO_IS_TX_OPERATOR(usb_audio_data))
    {
        unsigned *rate_adjust_val_addr = NULL;
        if(!stream_usb_audio_can_enact_rate_adjust(usb_audio_data->usb_audio_ep, &rate_adjust_val_addr))
        {
            /* Failed to see if it needs to perform rate adjustment */
            base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            return TRUE;
        }

        if(rate_adjust_val_addr != NULL)
        {
            usb_audio_data->usb_tx_rate_adjust_enable = TRUE;
            /* if rate adjust is enabled (so this op is enacting) then the input channels first
             * rate adjusted then they will be interleaved to single output channel. The interim
             * buffer between rate adjustment and interleave process is a scratch buffer. We could
             * use allocated memory from heap, but the interim buffer isn't really needed between
             * calls so opted for scratch buffer.
             *
             *         +-------------------+             +-------------------+
             *  CH0--->|    rate adjust    |------------>|    interleave     |
             *         |                   |             |                   |------> USB-TX EP
             *  CH1--->|    (optional)     |------------>|    process        |
             *         +-------------------+      ^      +-------------------+
             *                                    |
             *     These are scratch buffers  ----+
             */

            CBOP_VALS vals;

            /* reserve scratch buffer for doing cbops, scratch buffers are per channel,
             * maximum total size that is required will be to fill the entire interleaved buffer.
             */
            unsigned scratch_buff_size = cbuffer_get_size_in_words(usb_audio_data->usb_audio_buf)*sizeof(unsigned);
            if(!scratch_reserve(scratch_buff_size, MALLOC_PREFERENCE_NONE))
            {
                base_op_change_response_status(response_data, STATUS_CMD_FAILED);
                return TRUE;
            }

            /* store the reserved scratch size for future use */
            usb_audio_data->scratch_buff_size = scratch_buff_size;

            /* populate the fields that are relevant to rate adjustment only */
            vals.rate_adjustment_amount = rate_adjust_val_addr;
            vals.shift_amount = 0;

            /* all channels are now ready, so connect the cbops channels,
             * this will save memory and MIPS, as we know the number of channels
             * that needed and don't need to setup cbops for maximum number of
             * channels that might be required.
             */
            if (!cbops_mgr_connect(usb_audio_data->cbops,
                                   usb_audio_data->nrof_channels,   /* number of channels */
                                   usb_audio_data->pcm_channel_buf, /* input channels */
                                   usb_audio_data->pcm_scratch_buf, /* output channels */
                                   &vals))
            {
                /* failed, release the scratch buffers as well */
                scratch_release(usb_audio_data->scratch_buff_size, MALLOC_PREFERENCE_NONE);

                base_op_change_response_status(response_data, STATUS_CMD_FAILED);
                return TRUE;
            }

            /* get the sra op for using its APIs */
            usb_audio_data->sra = (void *) find_cbops_op(usb_audio_data->cbops->graph, cbops_rate_adjust_table);

            /* we don't use pass-through mode, either have it for actual enacting or not having
             * it. This isn't expected to change for the lifetime of this operator.
             */
            cbops_mgr_rateadjust_passthrough_mode(usb_audio_data->sra, FALSE);

            /* now cbops is fully setup, the endpoint might need to have SRA op */
            stream_usb_audio_set_cbops_sra_op(usb_audio_data->usb_audio_ep, usb_audio_data->sra);

        }
        else /* rate_adjust_val_addr != NULL */
        {   /* No scratch buffer is needed */
            usb_audio_data->scratch_buff_size = 0;
            usb_audio_data->usb_tx_rate_adjust_enable = FALSE;
        }
    }
#endif

    op_data->state = OP_RUNNING;
    return TRUE;
}

/**
 * \brief Starts the usb_audio capability so decoding will be attempted on a
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
bool usb_audio_stop (OPERATOR_DATA * op_data, void *message_data,
                     unsigned *response_id, void **response_data)
{
    USB_AUDIO_OP_DATA *usb_audio_data =
        (USB_AUDIO_OP_DATA *) (op_data->extra_op_data);

    *response_id = OPCMD_START;

    /* Create the response. If there aren't sufficient resources for this fail early. */
    if (!base_op_build_std_response (STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }

    if (OP_RUNNING != op_data->state)
    {
        /* Operator not running nothing to do. */
        return TRUE;
    }


#ifdef USB_AUDIO_SUPPORT_METADATA
    /* release scratch buffers */
    if(usb_audio_data->scratch_buff_size != 0)
    {
        /* release the scratch buffer that was reserved */
        scratch_release(usb_audio_data->scratch_buff_size, MALLOC_PREFERENCE_NONE);
    }

    if(usb_audio_data->cbops != NULL)
    {
        /* disconnect cbops */
        cbops_mgr_disconnect(usb_audio_data->cbops );
    }
#endif

    op_data->state = OP_NOT_RUNNING;
    return TRUE;
}

/**
 * usb_audio_reset
 * \brief reset the usb_audio op
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the buffer size request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool usb_audio_reset (OPERATOR_DATA * op_data, void *message_data,
                      unsigned *response_id, void **response_data)
{
    if (!base_op_reset (op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    return TRUE;
}

/**
 * usb_audio_create
 * \brief destroys the usb_audio op
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the buffer size request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool usb_audio_create (OPERATOR_DATA * op_data, void *message_data,
                       unsigned *response_id, void **response_data)
{
    USB_AUDIO_OP_DATA *usb_audio_data;

    /* call base_op create, which also allocates and fills response message */
    if (!base_op_create (op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* now initialise specific working data */
    usb_audio_data = (USB_AUDIO_OP_DATA *) op_data->extra_op_data;
    usb_audio_data->data_format = AUDIO_DATA_FORMAT_FIXP;

    /* is this usb audio tx operator? */
    if(op_data->cap_data->id == USB_AUDIO_TX_CAP_ID)
    {
        usb_audio_data->is_usb_audio_tx = TRUE;
    }

#ifdef USB_AUDIO_SUPPORT_METADATA
    if (op_data->cap_data->id == USB_AUDIO_RX_CAP_ID)
    {
        /* Allocate and initialise the TTP data */
        usb_audio_data->time_to_play = ttp_init();
        if (usb_audio_data->time_to_play != NULL)
        {
            ttp_params params;
            /* set the buffer size and the target required latency to  default value here,
             * these are expected to be overwritten by user message */
            usb_audio_data->pcm_buffer_size = USB_AUDIO_RX_DEFAULT_PCM_BUFFER_SIZE;
            usb_audio_data->target_latency = USB_AUDIO_RX_DEFAULT_LATENCY_REQUIRED;

            /* configure the ttp context */
            ttp_get_default_params(&params, TTP_TYPE_PCM);
            ttp_configure_latency(usb_audio_data->time_to_play, usb_audio_data->target_latency);
            ttp_configure_params(usb_audio_data->time_to_play, &params);
        }
        else
        {
            /* TTP init failed, so fail the operator creation */
            base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            return TRUE;
        }
    }
    else
    {
        /* create a cbops for doing sra processing, we could do dc remove as well,
         * but not strictly required. It will only be used if rate adjust is enabled
         * but the size of the object is very small, so create it unconditionally for TX op.
         */
        if((usb_audio_data->cbops = cbops_mgr_create(SINK, CBOPS_RATEADJUST)) == NULL)
        {
            /* failed, release the scratch buffers as well */
            base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            return TRUE;
        }

        /* rate adjustment will become enabled at start time
         * if requested by that time
         */
        usb_audio_data->usb_tx_rate_adjust_enable = FALSE;

        /* we also need scratch buffer for rate adjustment */
        scratch_register();
    }

#endif /* #ifdef USB_AUDIO_SUPPORT_METADATA */

    op_data->state = OP_NOT_RUNNING;
    return TRUE;
}

/**
 * usb_audio_destroy
 * \brief destroys the usb_audio op
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the buffer size request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool usb_audio_destroy (OPERATOR_DATA * op_data, void *message_data,
                        unsigned *response_id, void **response_data)
{
#ifdef USB_AUDIO_SUPPORT_METADATA
    {
        USB_AUDIO_OP_DATA *usb_audio_data = (USB_AUDIO_OP_DATA *) op_data->extra_op_data;
        if(usb_audio_data->time_to_play)
        {
            /* destroy the ttp context */
            PL_ASSERT(USB_AUDIO_IS_RX_OPERATOR(usb_audio_data));
            ttp_free(usb_audio_data->time_to_play);
        }

        if(USB_AUDIO_IS_TX_OPERATOR(usb_audio_data))
        {
            /* de-register scratch */
            scratch_deregister();
        }

        if(usb_audio_data->cbops != NULL)
        {
            /* destroy the cbops and its graph */
            cbops_mgr_destroy(usb_audio_data->cbops);
        }
    }
#endif /* #ifdef USB_AUDIO_SUPPORT_METADATA */

    /* Check that we are not trying to destroy a running operator */
    if (op_data->state == OP_RUNNING)
    {
        *response_id = OPCMD_DESTROY;

        /* We can't destroy a running operator. */
        return base_op_build_std_response (STATUS_CMD_FAILED, op_data->id,
                                           response_data);
    }
    else
    {
        /* call base_op destroy that creates and fills response message, too */
        return base_op_destroy (op_data, message_data, response_id,
                                response_data);
    }
}

/**
 * usb_audio_configure_connected_usb_audio_endpoint
 * \brief configuring the connected usb audio endpoint
 *
 * \param op_data Pointer to the operator instance data.
 *
 * \return Whether the endpoint was configured successfully
 */
bool usb_audio_configure_connected_usb_audio_endpoint(OPERATOR_DATA * op_data)
{

    /* get the operator specific data */
    USB_AUDIO_OP_DATA *usb_audio_data =  (USB_AUDIO_OP_DATA *) (op_data->extra_op_data);

    if(NULL == usb_audio_data->usb_audio_ep)
    {
        /* shouldn't happen, just for peace of mind */
        return FALSE;
    }

    /* configure the endpoint */
    return stream_usb_audio_configure_ep(usb_audio_data->usb_audio_ep,
                                         usb_audio_data->sample_rate,
                                         usb_audio_data->nrof_channels,
                                         usb_audio_data->subframe_size);
}

/**
 * usb_audio_connect
 * \brief connecting a sink or source terminal of usb_audio op
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the buffer size request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool usb_audio_connect (OPERATOR_DATA * op_data, void *message_data,
                        unsigned *response_id, void **response_data)
{
    USB_AUDIO_OP_DATA *usb_audio_data =
        (USB_AUDIO_OP_DATA *) (op_data->extra_op_data);
    unsigned terminal_id = ((unsigned *) message_data)[0];
    tCbuffer **bufp = NULL;
    unsigned terminal_number = terminal_id & TERMINAL_NUM_MASK;

    *response_id = OPCMD_CONNECT;
    if (!base_op_build_std_response (STATUS_OK, op_data->id, response_data))
    {
        /* Shouldn't change anything if there is not enough memory for the response. */
        return FALSE;
    }

    /* No connection while the operator is running */
    if(OP_RUNNING == op_data->state)
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    /* at this time all the configurations must have been
     * done
     */
    if(!usb_audio_data->op_configured)
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return FALSE;
    }
    /* See if this is for a usb_audio data channel, for
     * tx capability this will be a source terminal
     * but for rx capability it will be a sink
     * terminal;
     */
    if(USB_AUDIO_TERMINAL_IS_USB_AUDIO_EP(usb_audio_data, terminal_id))
    {
        /* Only terminal 0 used for usb_audio endpoint */
        if(!terminal_number)
        {
            /* Connection is for usb_audio terminal
             * (endpoint side) */
            bufp = &usb_audio_data->usb_audio_buf;
        }
    }
    else
    {
        /* check max terminal number,though it must
         * already have been checked by frame work
         */
        if(terminal_number < usb_audio_data->nrof_channels)
        {
            /* it's for a pcm  terminal */
            bufp = &usb_audio_data->pcm_channel_buf[terminal_number];
        }
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
        if(bufp == &usb_audio_data->usb_audio_buf)
        {
            /* This is for the terminal connected to USB endpoint
             * get the USB endpoint and store it. It is used for
             * configuring the endpoint (and/or calling other EP's APIs)
             */
            usb_audio_data->usb_audio_ep =  stream_get_connected_endpoint_from_terminal_id(op_data->id, terminal_id);

            /* the connection is for endpoint side of the operator,
             * we need to configure the endpoint at this point */
            if(!usb_audio_configure_connected_usb_audio_endpoint(op_data))
            {
                /* couldn't configure the endpoint */
                base_op_change_response_status(response_data, STATUS_CMD_FAILED);
                return TRUE;
            }
        }
        /* connect the buffer */
        *bufp = (tCbuffer *) (((uintptr_t *) message_data)[1]);
#ifdef USB_AUDIO_SUPPORT_METADATA
        if(buff_has_metadata(*bufp))
        {
            if(bufp == &usb_audio_data->usb_audio_buf)
            {
                if(USB_AUDIO_IS_RX_OPERATOR(usb_audio_data))
                {
                    /* at least currently, we only expect metadata
                     * for output PCM channels of rx operator */
                    /* doesn't expect metadata on this terminal */
                    *bufp = NULL;
                    base_op_change_response_status(response_data, STATUS_CMD_FAILED);
                }
                return TRUE;
            }

            if(USB_AUDIO_IS_TX_OPERATOR(usb_audio_data) &&
               usb_audio_data->pcm_scratch_buf[terminal_number] == NULL)
            {
                /* create scratch buffer for rate adjustment */
                usb_audio_data->pcm_scratch_buf[terminal_number] = cbuffer_create(NULL, 0, BUF_DESC_SW_BUFFER);
                if(usb_audio_data->pcm_scratch_buf[terminal_number] == NULL)
                {
                    *bufp = NULL;
                    base_op_change_response_status(response_data, STATUS_CMD_FAILED);
                    return TRUE;
                }
            }

            if(usb_audio_data->pcm_metadata_buffer == NULL)
            {
                /* first channel to have metadata, store it */
                usb_audio_data->pcm_metadata_buffer = *bufp;
            }
        }
#endif /* #ifdef USB_AUDIO_SUPPORT_METADATA */

    }
    return TRUE;
}

/**
 * usb_audio_disconnect
 * \brief disconnecting a sink or source terminal of usb_audio op
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the buffer size request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool usb_audio_disconnect (OPERATOR_DATA * op_data, void *message_data,
                           unsigned *response_id, void **response_data)
{
    USB_AUDIO_OP_DATA *usb_audio_data =
        (USB_AUDIO_OP_DATA *) (op_data->extra_op_data);
    unsigned terminal_id = ((unsigned *) message_data)[0];
    tCbuffer **bufp = NULL;
    unsigned terminal_number = terminal_id & TERMINAL_NUM_MASK;

    *response_id = OPCMD_CONNECT;
    if (!base_op_build_std_response (STATUS_OK, op_data->id, response_data))
    {
        /* Shouldn't change anything if there is
         * not enough memory for the response.
         */
        return FALSE;
    }

    /* No disconnection while the operator is running */
    if(OP_RUNNING == op_data->state)
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    /* See if this is for a usb_audio data channel, for
     * tx capability this will be a source terminal
     * but for rx capability it will be a sink
     * terminal;
     */
    if(USB_AUDIO_TERMINAL_IS_USB_AUDIO_EP(usb_audio_data, terminal_id))
    {
        /* Only terminal 0 used for usb_audio endpoint */
        if(!terminal_number)
        {
            /* disconnection is for endpoint side of operator */
            bufp = &usb_audio_data->usb_audio_buf;
        }
    }
    else
    {
        /* it's for a pcm  terminal */
        /* check max terminal number,though it must
         * already have been checked by frame work
         */
        if(terminal_number < usb_audio_data->nrof_channels)
        {
            /* disconnection is for individual channels */
            bufp = &usb_audio_data->pcm_channel_buf[terminal_number];
        }
    }

    if (!bufp)
    {
        /* no buffer exists for the requested terminal */
        base_op_change_response_status(response_data, STATUS_INVALID_CMD_PARAMS);
    }
    else if (*bufp)
    {
#ifdef USB_AUDIO_SUPPORT_METADATA
        if(usb_audio_data->pcm_metadata_buffer == *bufp)
        {
            unsigned chan;
            tCbuffer *new_metadata_buf = NULL;
            /* This channel is the metadata reference channel,
             * see if any remaining channel has metadata, and
             * use that channel for metadata.
             *
             * NOTE: This is less likely to be needed
             * as operators terminals can't be
             * connected/disconnected at running time
             */
            PL_ASSERT(!(USB_AUDIO_TERMINAL_IS_USB_AUDIO_EP(usb_audio_data, terminal_id)));
            if(usb_audio_data->pcm_scratch_buf[terminal_number] != NULL)
            {
                /* cbuffer structure created for scratch buf of this channel,
                 * no longer needed
                 */
                cbuffer_destroy_struct(usb_audio_data->pcm_scratch_buf[terminal_number]);
            }

            for(chan = 0; chan < usb_audio_data->nrof_channels; ++chan)
            {
                if(usb_audio_data->pcm_channel_buf[chan] != NULL &&          /* channel is still valid */
                   usb_audio_data->pcm_channel_buf[chan] != *bufp &&         /* not this channel */
                   buff_has_metadata(usb_audio_data->pcm_channel_buf[chan])) /* and has metadata */
                {
                    new_metadata_buf = usb_audio_data->pcm_channel_buf[chan];
                    break;
                }
            }
            usb_audio_data->pcm_metadata_buffer = new_metadata_buf;
        }
#endif /* #ifdef USB_AUDIO_SUPPORT_METADATA */
        /* disconnect */
        *bufp = NULL;

        /* TODO: if this is for endpoint side terminal, we perhaps better
         * to invalidate current config, so any new connection will
         * have to be done after a fresh configuration.
         */
    }
    else
    {
        /* terminal looks to be not connected */
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
    }

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
bool usb_audio_buffer_details (OPERATOR_DATA * op_data, void *message_data,
                               unsigned *response_id, void **response_data)
{
    USB_AUDIO_OP_DATA *usb_audio_data =
        (USB_AUDIO_OP_DATA *) (op_data->extra_op_data);
    unsigned terminal_id = ((unsigned *) message_data)[0];

    unsigned buf_size;

    if (!base_op_buffer_details
        (op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* expects the operator has been configured by this time */
    if(!usb_audio_data->op_configured)
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return FALSE;
    }

    /* see how much buffer it needs,
     * NOTE: computation below isn't valid for encoded usb audio*/

    /* 2ms buffer per channel, extra 1% for rate mismatch */
    buf_size = frac_mult(usb_audio_data->sample_rate, FRACTIONAL(0.00202));

    if(USB_AUDIO_TERMINAL_IS_USB_AUDIO_EP(usb_audio_data, terminal_id))
    {
        /* endpoint side is interleaved */
        buf_size *= usb_audio_data->nrof_channels;
    }
    else
    {
        /* if pcm buffer size has been set by user, then
         * at least that size should be used */
        if(buf_size < usb_audio_data->pcm_buffer_size)
        {
            buf_size = usb_audio_data->pcm_buffer_size;
        }
    }

#ifdef USB_AUDIO_SUPPORT_METADATA
    if(!(USB_AUDIO_TERMINAL_IS_USB_AUDIO_EP(usb_audio_data, terminal_id)))
    {
        /* supply metadata buffer */
        ((OP_BUF_DETAILS_RSP*)*response_data)->metadata_buffer = usb_audio_data->pcm_metadata_buffer;
        ((OP_BUF_DETAILS_RSP*)*response_data)->supports_metadata = TRUE;
    }
    else
    {
        /* For endpoint side terminal, currently Rx doesn't support it */
        ((OP_BUF_DETAILS_RSP*)*response_data)->supports_metadata = USB_AUDIO_IS_TX_OPERATOR(usb_audio_data);
        ((OP_BUF_DETAILS_RSP*)*response_data)->metadata_buffer = NULL;
    }
#endif /* #ifdef USB_AUDIO_SUPPORT_METADATA */

    ((OP_BUF_DETAILS_RSP *) * response_data)->b.buffer_size = buf_size;

    return TRUE;
}

/**
 * usb_audio_get_sched_info
 * \brief get schedule info for usb_audio op
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the buffer size request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool usb_audio_get_sched_info (OPERATOR_DATA * op_data, void *message_data,
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
 * usb_audio_opmsg_set_buffer_size
 * \brief message handler to set required buffer size
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the start request message
 * \param resp_length pointer to location to write the response message length
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool usb_audio_opmsg_set_buffer_size(OPERATOR_DATA *op_data, void *message_data,
                                     unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{

    USB_AUDIO_OP_DATA *usb_audio_data =
        (USB_AUDIO_OP_DATA *) (op_data->extra_op_data);

    /* We cant change this setting while running */
    if (OP_RUNNING == op_data->state)
    {
        return FALSE;
    }

    /* set the buffer size, it will only be used for PCM terminals */
    usb_audio_data->pcm_buffer_size = OPMSG_FIELD_GET(message_data, OPMSG_COMMON_SET_BUFFER_SIZE, BUFFER_SIZE);
    return TRUE;

}

#ifdef USB_AUDIO_SUPPORT_METADATA
/**
 * usb_audio_opmsg_set_ttp_latency
 * \brief message handler to set ttp target latency for pcm channels
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the start request message
 * \param resp_length pointer to location to write the response message length
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool usb_audio_opmsg_set_ttp_latency(OPERATOR_DATA *op_data, void *message_data,
                                     unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    USB_AUDIO_OP_DATA *usb_audio_data =
        (USB_AUDIO_OP_DATA *) (op_data->extra_op_data);

    if(USB_AUDIO_IS_TX_OPERATOR(usb_audio_data))
    {
        return FALSE;
    }

    /* We cant change this setting while running */
    if (OP_RUNNING == op_data->state)
    {
        return FALSE;
    }

    /* configure the latency */
    usb_audio_data->target_latency = ttp_get_msg_latency(message_data);
    ttp_configure_latency(usb_audio_data->time_to_play, usb_audio_data->target_latency);

    return TRUE;
}

/**
 * usb_audio_opmsg_set_latency_limits
 * \brief message handler to set ttp latency limits for pcm channels
 */
bool usb_audio_opmsg_set_latency_limits(OPERATOR_DATA *op_data, void *message_data,
                                        unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    TIME_INTERVAL min_latency, max_latency;
    USB_AUDIO_OP_DATA *usb_audio_data =
        (USB_AUDIO_OP_DATA *) (op_data->extra_op_data);

    if(USB_AUDIO_IS_TX_OPERATOR(usb_audio_data))
    {
        return FALSE;
    }

    ttp_get_msg_latency_limits(message_data, &min_latency, &max_latency);
    ttp_configure_latency_limits(usb_audio_data->time_to_play, min_latency, max_latency);

    return TRUE;
}

/**
 * usb_audio_opmsg_set_ttp_params
 * \brief message handler to set ttp parameters for rx operator
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the start request message
 * \param resp_length pointer to location to write the response message length
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool usb_audio_opmsg_set_ttp_params(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    ttp_params params;

    USB_AUDIO_OP_DATA *usb_audio_data =
        (USB_AUDIO_OP_DATA *) (op_data->extra_op_data);

    if(USB_AUDIO_IS_TX_OPERATOR(usb_audio_data))
    {
        return FALSE;
    }

    /* We cant change this setting while running */
    if (OP_RUNNING == op_data->state)
    {
        return FALSE;
    }
    ttp_get_msg_params(&params, message_data);

    if(usb_audio_data->op_configured &&
       params.nominal_sample_rate != usb_audio_data->sample_rate)
    {
        return FALSE;
    }

    ttp_configure_params(usb_audio_data->time_to_play, &params);

    return TRUE;
}

/**
 * usb_audio_rx_generate_metadata_with_ttp
 * \brief generates metadata for each chunk of samples
 * \param usb_audio_data op specific data structure for usb audio rx operator
 * \param samples number of samples in the to-be-copied chunk
 */
void usb_audio_rx_generate_metadata_with_ttp(USB_AUDIO_OP_DATA *usb_audio_data, unsigned samples)
{
    metadata_tag *mtag;
    unsigned b4idx, afteridx;

    /* This function is only for Rx op that has a metadata buffer */
    PL_ASSERT(NULL != usb_audio_data->pcm_metadata_buffer);
    PL_ASSERT(USB_AUDIO_IS_RX_OPERATOR(usb_audio_data));

    /* Create a new tag for the output */
    b4idx = 0;
    afteridx = samples * OCTETS_PER_SAMPLE;
    mtag = buff_metadata_new_tag();
    if (mtag != NULL)
    {
        ttp_status status;
        mtag->length = samples * OCTETS_PER_SAMPLE;
        ttp_update_ttp(usb_audio_data->time_to_play, hal_get_time(), samples, &status);
        /* Populate the metadata tag from the TTP status */
        ttp_utils_populate_tag(mtag, &status);
    }

    /* append generated metadata to the output buffer */
    buff_metadata_append(usb_audio_data->pcm_metadata_buffer, mtag, b4idx, afteridx);

}

/**
 * timestamp_metadata_reframe_with_sra
 * \brief translate metadata tags from input buffer of sra to its output
 * \param sra_mtrans pointer to sra time stamp metadata transfer structure
 * \param input_mtag the input tag for the block of audio that SRA has read
 *        will be deleted by this function.
 * \param input_mtag_b4ix any before index for the input_mtag
 * \consumed_samples number of samples consumed by SRA
 * \written_frames number of sample written by SRA
 * \return a fresh timestamp tag with time stamp associated to first sample of output block
 *
 * NOTE: this is a generic function and not specific to usb audio and might
 *       be moved to a generic common place in future.
 */
metadata_tag *timestamp_metadata_reframe_with_sra(SRA_TIMESTAMP_METADATA_TRANSFER *sra_mtrans,
                                                  metadata_tag *input_mtag,
                                                  unsigned input_mtag_b4ix,
                                                  unsigned consumed_samples,
                                                  unsigned written_frames)
{

    /* time stamp for the output tag */
    TIME timestamp = 0;
    metadata_tag *out_mtag = NULL;

    if(!sra_mtrans->first_ts_tag_observed &&
       input_mtag != NULL &&
       IS_TIMESTAMPED_TAG(input_mtag))
    {
        /* wait until seeing first time stamped tag, normally should be the first tag
         * if graph properly configured
         */
        sra_mtrans->first_ts_tag_observed = TRUE;
    }

    if(sra_mtrans->first_ts_tag_observed)
    {
        /* input is time stamped, we want to work out the time stamp for fist output sample */

        INTERVAL time_passed = (INTERVAL) (((uint48)written_frames*SECOND)/sra_mtrans->sample_rate);
        time_passed -= frac_mult(time_passed, sra_mtrans->current_sra_sp_adjust);

        /* calculate time stamp for fist output sample */
        if(input_mtag != NULL && IS_TIMESTAMPED_TAG(input_mtag))
        {
            /* if this tag is time stamped we can calculate the time stamp for output fairly accurately */
            unsigned samples_b4 = input_mtag_b4ix / OCTETS_PER_SAMPLE;

            /* go back to first input sample */
            INTERVAL time_back = (INTERVAL) (((uint48)samples_b4*SECOND)/sra_mtrans->sample_rate);

            /* adjust for the phase difference between first input and first output sample */
            time_back -= (int)frac_mult(SECOND, (int)sra_mtrans->last_sra_phase) / (int)sra_mtrans->sample_rate;

            /* set the time stamp */
            timestamp = time_sub(input_mtag-> timestamp, time_back);
            sra_mtrans->last_input_sp_adjust = input_mtag->sp_adjust;
        }
        else
        {
            /* if no time stamp is seen, or may be tag is NULL because it has been partially read, then
             * work out the the time stamp using previous tags, this is fine for short period only, if
             * the we stop receiving time stamped tags, this will have gradual accumulation error.
             */
            timestamp = sra_mtrans->last_output_time_stamp;
        }

        /* compute the time stamp for end of this chunk, so it can be used in next run */
        sra_mtrans->last_output_time_stamp = time_add(timestamp, time_passed);
    }

    /* we don't need metadata any more */
    buff_metadata_tag_list_delete(input_mtag);
    input_mtag = NULL;

    /* we have the time stamp and the sp_adjust value for new output chunk,
     * create a new tag and append it to output buffer
     */
    out_mtag = buff_metadata_new_tag();
    if(out_mtag != NULL)
    {
        /* set the output length */
        out_mtag->length = written_frames  * OCTETS_PER_SAMPLE;
        if(sra_mtrans->first_ts_tag_observed)
        {
            /* timestamp tag, only if we have a reference from input */
            out_mtag->sp_adjust = sra_mtrans->last_input_sp_adjust - sra_mtrans->current_sra_sp_adjust;
            if(sra_mtrans->first_ts_tag_observed)
            {
                METADATA_TIMESTAMP_SET(out_mtag, timestamp, METADATA_TIMESTAMP_LOCAL);
            }
        }
    }
    return out_mtag;
}


/**
 * usb_audio_tx_transfer_metadata
 * \brief transfers metadata from input to output
 * \param usb_audio_data op specific data structure for usb audio tx operator
 * \param consumed_samples number of samples that is read from pcm input buffers
 * \param written_frames number of frames written to output (interleaved) buffer
 *
 */
void usb_audio_tx_transfer_metadata(USB_AUDIO_OP_DATA *usb_audio_data, unsigned consumed_samples, unsigned written_frames)
{
    unsigned b4idx=0, afteridx=0;
    unsigned nrof_channels = usb_audio_data->nrof_channels;

    /* get metadata tag from input */
    metadata_tag *mtag = buff_metadata_remove(usb_audio_data->pcm_metadata_buffer,
                                              consumed_samples*OCTETS_PER_SAMPLE,
                                              &b4idx,
                                              &afteridx);


    if(!usb_audio_data->usb_tx_rate_adjust_enable)
    {
        /* output buffer is interleaved,
         * so extend the length of tags accordingly
         */
        b4idx = b4idx * nrof_channels;
        afteridx = afteridx * nrof_channels;
        if(NULL != mtag)
        {
            /* traverse through all tags in the removed list and fix the length of the tags */
            metadata_tag *mtag_temp = mtag;
            while(mtag_temp != NULL)
            {
                mtag_temp->length = mtag_temp->length * nrof_channels;
                mtag_temp = mtag_temp->next;
            }
        }
        /* Append modified tag to output buffer */
        buff_metadata_append(usb_audio_data->usb_audio_buf, mtag, b4idx, afteridx);
    }
    else
    {
        /* get the current warp value */
        usb_audio_data->sra_mtrans.current_sra_sp_adjust = (int) cbops_sra_get_current_rate_adjust(usb_audio_data->sra);

        /* translate input tag to a suitable output tag, considering SRA.
         * The input tag will be deleted by this function, and a new
         * output tag with suitable timestamp will be returned
         */
        metadata_tag *out_mtag = timestamp_metadata_reframe_with_sra(&usb_audio_data->sra_mtrans,
                                                                     mtag,
                                                                     b4idx,
                                                                     consumed_samples,
                                                                     written_frames);
        if(out_mtag != NULL)
        {
            /* we have an interleave of buffers afterwards,
             * so extend the length of the tag
             */
            out_mtag->length *= nrof_channels;
            afteridx = out_mtag->length;
            L4_DBG_MSG4("USB TX OP - Metadata transfer: time=%d, tims_stamp=%d, amount_read=%d, amount_written=%d",
                        hal_get_time(),
                        out_mtag->timestamp,
                        consumed_samples,
                        written_frames);
        }
        else
        {
            /* octets covered by null tag */
            afteridx = written_frames * nrof_channels * OCTETS_PER_SAMPLE;

            /* failed to create a new tag, still a null tag will be appended */
            L4_DBG_MSG3("USB TX OP: Metadata transfer: NULL TAG Written, time=%d, amount_read=%d, amount_written",
                        hal_get_time(),
                        consumed_samples,
                        written_frames);
        }

        /* new write the fresh time-stamped tag for output buffer*/
        b4idx = 0;
        buff_metadata_append(usb_audio_data->usb_audio_buf, out_mtag, b4idx, afteridx);
    }
}

#endif /* #ifdef USB_AUDIO_SUPPORT_METADATA */

/**
 * usb_audio_process_data
 * \brief process function when usb_audio operator is kicked
 *
 * \param op_data Pointer to the operator instance data.
 * \param touched Location to write which terminals touched at this kick
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
void usb_audio_process_data (OPERATOR_DATA * op_data, TOUCHED_TERMINALS *touched)
{

    USB_AUDIO_OP_DATA *usb_audio_data;


    patch_fn(usb_audio_process_data);


    /* reset the touch sinks */
    touched->sinks = 0;

    /* reset the touch sources */
    touched->sources = 0;

    /* If we're not running then do nothing. */
    if (OP_NOT_RUNNING == op_data->state)
    {
        return;
    }

    /* get the usb_audio data */
    usb_audio_data = (USB_AUDIO_OP_DATA *) (op_data->extra_op_data);

    if(usb_audio_data->data_format == UADF_T1_PCM)
    {
        unsigned amount_to_read;
        unsigned chan;
        unsigned amount_written = 0;

        if(USB_AUDIO_IS_TX_OPERATOR(usb_audio_data))
        {
            /* 1ms worth of audio */
            unsigned packet_len = frac_mult(usb_audio_data->sample_rate, FRACTIONAL(0.001));

            /* will show the current amount of data left in the input buffer */
            unsigned amount_data_available;

            /* calculate the amount of samples we can interleave from each channel */
            amount_to_read = cbuffer_calc_amount_space_in_words(usb_audio_data->usb_audio_buf)/usb_audio_data->nrof_channels;

            /* get the amount data available
             * use a large value to init amount data available
             */
            amount_data_available = (1<<16)-1;
            for(chan = 0; chan < usb_audio_data->nrof_channels; ++chan)
            {
                unsigned channel_amount = cbuffer_calc_amount_data_in_words(usb_audio_data->pcm_channel_buf[chan]);
                amount_data_available = MIN(amount_data_available, channel_amount);
            }

#ifdef USB_AUDIO_SUPPORT_METADATA
            if(usb_audio_data->usb_tx_rate_adjust_enable)
            {
                /* rate adjustment is enabled, so we run SRA, there is post interleaving process after cbops,
                 * we need to make sure there will be spaces for all the outputs of cbops processing, because the
                 * interim buffers are scratch.
                 */
                amount_to_read -= frac_mult(amount_to_read, cbops_sra_get_current_rate_adjust(usb_audio_data->sra));

                /* knock off two, but don't get negative */
                amount_to_read = MAX(amount_to_read,2) - 2;
            }

            if (buff_has_metadata(usb_audio_data->pcm_metadata_buffer))
            {
                /* Also don't copy more than the amount of metadata available */
                unsigned input_meta_available =  (buff_metadata_available_octets(usb_audio_data->pcm_metadata_buffer)/OCTETS_PER_SAMPLE);
                amount_data_available = MIN(amount_data_available, input_meta_available);
            }
#endif
            /* limit the amount to read to amount data available */
            amount_to_read = MIN(amount_to_read, amount_data_available);

            /* interleave pcm input buffers */
            if(amount_to_read != 0)
            {

#ifdef USB_AUDIO_SUPPORT_METADATA
                if(usb_audio_data->usb_tx_rate_adjust_enable)
                {
                    unsigned amount_to_interleave;
                    unsigned scratch_size = (usb_audio_data->scratch_buff_size / sizeof(unsigned) / usb_audio_data->nrof_channels) * sizeof(unsigned);
                    /* get scratch buffer to do SRA */
                    for(chan = 0; chan < usb_audio_data->nrof_channels; ++chan)
                    {
                        void *scratch_buf = scratch_commit(scratch_size, MALLOC_PREFERENCE_NONE);
                        usb_audio_data->pcm_scratch_buf[chan]->size = scratch_size;
                        cbuffer_scratch_commit_update(usb_audio_data->pcm_scratch_buf[chan], scratch_buf);
                    }

                    /* get the sra phase, this shows the relative phase of first output sample. This will
                     * be updated by cbops, so we get it before running cbops
                     */
                    usb_audio_data->sra_mtrans.last_sra_phase = cbops_sra_get_phase(usb_audio_data->sra);

                    /* outputs are scratch buffer, They don't retain read/write pointers
                     * between calls, let cbops know about this
                     */
                    cbops_mgr_buffer_reinit(usb_audio_data->cbops);

                    /* run cbops process
                     */
                    cbops_mgr_process_data(usb_audio_data->cbops, amount_to_read);

                    /* get number of samples cbops processed */
                    amount_to_read = cbops_get_amount(usb_audio_data->cbops->graph, 0);

                    /* update amount data available */
                    amount_data_available -= amount_to_read;

                    /* see how many sample cbops produced, all of them needs to be
                     * consumed this run, because they are in scratch buffers
                     */
                    amount_to_interleave = cbuffer_calc_amount_data_in_words(usb_audio_data->pcm_scratch_buf[0]);

                    /* de-interleave */
                    amount_written = interleave_buffers(usb_audio_data->usb_audio_buf,
                                                        usb_audio_data->pcm_scratch_buf,
                                                        usb_audio_data->nrof_channels,
                                                        amount_to_interleave);

                    for(chan = 0; chan < usb_audio_data->nrof_channels; ++chan)
                    {
                        /* We are using scratch, all the scratch buffer must have been used */
                        PL_ASSERT(cbuffer_calc_amount_data_in_words(usb_audio_data->pcm_scratch_buf[chan]) == 0);
                    }

                    /* no reason any all the samples not written to the output buffer */
                    PL_ASSERT(amount_written == amount_to_interleave);
                }
                else /* usb_audio_data->usb_tx_rate_adjust_enable */
#endif /* #ifdef USB_AUDIO_SUPPORT_METADATA */
                {
                    /* interleave the pcm channel into usb buffer */
                    amount_written = interleave_buffers(usb_audio_data->usb_audio_buf,
                                                        usb_audio_data->pcm_channel_buf,
                                                        usb_audio_data->nrof_channels,
                                                        amount_to_read);

                    /* update amount data available */
                    amount_data_available -= amount_to_read;

                    /* something is wrong if not all expected samples copied */
                    PL_ASSERT(amount_written == amount_to_read);
                }  /* usb_audio_data->usb_tx_rate_adjust_enable */

#ifdef USB_AUDIO_SUPPORT_METADATA
                if(usb_audio_data->pcm_metadata_buffer != NULL)
                {
                    /* transfer metadata from input to output */
                    usb_audio_tx_transfer_metadata(usb_audio_data, amount_to_read, amount_written);
                }
#endif
#ifdef USB_AUDIO_SUPPORT_METADATA
                scratch_free();
#endif
            } /* amount_to_read != 0 */

            /* kick the input side if less than
             * 2ms audio left in the input buffer
             */
            if(amount_data_available < (2*packet_len))
            {
                /* The output and all the input terminals are touched */
                touched->sinks = (1<<usb_audio_data->nrof_channels) - 1;
            }
            /* Note: No kick for source side, USB TX EP kicks are timer-based */

        } /* USB_AUDIO_IS_TX_OPERATOR(usb_audio_data) */

        else
        {
            /* calculate the amount of samples we can de-interleave into each channel */
            amount_to_read = cbuffer_calc_amount_data_in_words(usb_audio_data->usb_audio_buf)/usb_audio_data->nrof_channels;
            for(chan = 0; chan < usb_audio_data->nrof_channels; ++chan)
            {
                unsigned channel_amount = cbuffer_calc_amount_space_in_words(usb_audio_data->pcm_channel_buf[chan]);
                amount_to_read = MIN(amount_to_read, channel_amount);
            }

            /* de-interleave received usb audio buffer */
            if(amount_to_read)
            {
#ifdef USB_AUDIO_SUPPORT_METADATA
                if(usb_audio_data->pcm_metadata_buffer != NULL)
                {
                    /* needs to generate metadata */
                    usb_audio_rx_generate_metadata_with_ttp(usb_audio_data, amount_to_read);
                }
#endif /* #ifdef USB_AUDIO_SUPPORT_METADATA */

                amount_written = deinterleave_buffers(usb_audio_data->usb_audio_buf,
                                                      usb_audio_data->pcm_channel_buf,
                                                      usb_audio_data->nrof_channels,
                                                      amount_to_read);

                /* something is wrong if not all expected samples copied */
                PL_ASSERT(amount_written == amount_to_read);

                if(amount_written)
                {
                    /* The input and all the output terminals are touched,
                     * NOTE: for optimisation purpose we could kick the sink
                     *       only if there is nothing to use anymore, but
                     *       in this case usb rx EP doesn't want kick anyway
                     */
                    touched->sinks = 1;
                    touched->sources = (1<<usb_audio_data->nrof_channels) - 1;
                }
            }
        }
    }
    else /* usb_audio_data->data_format == UADF_T1_PCM */
    {
        /* currently only linear PCM is supported */
    }

    return;
}

/*  obpm msg     */
bool usb_audio_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    /* In the case of this capability, nothing is done for control message. Just follow protocol and ignore any content. */
    return cps_control_setup(message_data, resp_length, resp_data,NULL);
}


bool usb_audio_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    return FALSE;
}

bool usb_audio_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{

    return FALSE;
}

bool usb_audio_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    /* Set the parameter(s). For future proofing, it is using the whole mechanism, although currently there is only one field
     * in opdata structure that is a setable parameter. If later there will be more (ever), must follow contiguously the first field,
     * as commented and instructed in the op data definition. Otherwise consider moving them into a dedicated structure.
     */

    return FALSE;
}

bool usb_audio_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    USB_AUDIO_OP_DATA * op_extra_data =  (USB_AUDIO_OP_DATA*)op_data->extra_op_data;

    unsigned* resp = NULL;

    if(!common_obpm_status_helper(message_data,resp_length,resp_data,sizeof(USB_AUDIO_STATISTICS) ,&resp))
    {
        return FALSE;
    }

    if(resp)
    {
        resp = cpsPackWords(&op_extra_data->sample_rate, &op_extra_data->nrof_channels, resp);
        resp = cpsPackWords((unsigned*)&op_extra_data->data_format, &op_extra_data->bit_resolution, resp);
        cpsPackWords((unsigned*)&op_extra_data->is_usb_audio_tx, NULL, resp);
    }

    return TRUE;
}
