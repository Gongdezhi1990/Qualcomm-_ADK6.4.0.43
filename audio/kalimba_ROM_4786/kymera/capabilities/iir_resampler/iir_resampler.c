/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  iir_resampler.c
 * \ingroup  capabilities
 *
 * capability wrapper for iir_resamplerv2 algorithm.
 *
 */

/****************************************************************************
Include Files
*/

#include "capabilities.h"
#include "iir_resampler.h"
#include "common_conversions.h"
#include "mem_utils/scratch_memory.h"
#include "iir_resampler_private.h"
#include "platform/pl_fractional.h"
#include "platform/pl_assert.h"
#include "op_msg_helpers.h"
#include "iir_resampler_gen_c.h"

#ifdef INSTALL_METADATA
#include "ttp/ttp.h"
#include "ttp/timed_playback.h"
#include "ttp_utilities.h"
#endif

#include "patch/patch.h"

/****************************************************************************
Private Constant Declarations
*/
/** The number of bits of the conversion factor that are available for
 * representing the integer portion */
#define INT_PART_BITS 5

/* legacy set_conversion_rate handler look-up table */
#define IIR_RESAMPLER_RATE_INDEX_LUT_LEN                9
const unsigned iir_resampler_rate_index_lut[IIR_RESAMPLER_RATE_INDEX_LUT_LEN] =
          {8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000};

#ifdef CAPABILITY_DOWNLOAD_BUILD
#define IIR_RESAMPLER_CAP_ID CAP_ID_DOWNLOAD_IIR_RESAMPLER
#else
#define IIR_RESAMPLER_CAP_ID CAP_ID_IIR_RESAMPLER
#endif

/****************************************************************************
Private Structure Definitions
*/

/* iir_resampler data processing - asm functions */
extern void iir_resampler_reset_internal(IIR_RESAMPLER_OP_DATA* op_extra_data);
extern unsigned iir_resampler_amount_to_use(IIR_RESAMPLER_OP_DATA* op_extra_data, unsigned *process_all_input, unsigned *channel_disconnected);
extern unsigned iir_resampler_processing(IIR_RESAMPLER_OP_DATA* op_extra_data, unsigned amount_to_use);

/** The IIR resampler capability function handler table */
const handler_lookup_struct iir_resampler_handler_table =
{
    iir_resampler_create,         /* OPCMD_CREATE */
    iir_resampler_destroy,        /* OPCMD_DESTROY */
    iir_resampler_start,          /* OPCMD_START */
    iir_resampler_stop,           /* OPCMD_STOP */
    iir_resampler_reset,          /* OPCMD_RESET */
    iir_resampler_connect,        /* OPCMD_CONNECT */
    iir_resampler_disconnect,     /* OPCMD_DISCONNECT */
    iir_resampler_buffer_details, /* OPCMD_BUFFER_DETAILS */
    base_op_get_data_format,      /* OPCMD_DATA_FORMAT */
    iir_resampler_get_sched_info  /* OPCMD_GET_SCHED_INFO */
};

/* Null terminated operator message handler table - this is the set of operator
 * messages that the capability understands and will attempt to service. */
const opmsg_handler_lookup_table_entry iir_resampler_opmsg_handler_table[] =
{
    {OPMSG_COMMON_ID_GET_CAPABILITY_VERSION, base_op_opmsg_get_capability_version},
    {OPMSG_IIR_RESAMPLER_ID_SET_SAMPLE_RATES, iir_resampler_opmsg_set_sample_rates},
    {OPMSG_IIR_RESAMPLER_ID_SET_CONVERSION_RATE, iir_resampler_opmsg_set_conversion_rate},
    {OPMSG_IIR_RESAMPLER_ID_SET_CONFIG, iir_resampler_opmsg_set_config},
    {OPMSG_COMMON_SET_DATA_STREAM_BASED, iir_resampler_data_stream_based},
    /* obpm message */
    {OPMSG_COMMON_ID_SET_CONTROL,  iir_resampler_opmsg_obpm_set_control},
    {OPMSG_COMMON_ID_GET_PARAMS,   iir_resampler_opmsg_obpm_get_params},
    {OPMSG_COMMON_ID_GET_DEFAULTS, iir_resampler_opmsg_obpm_get_defaults},
    {OPMSG_COMMON_ID_SET_PARAMS,   iir_resampler_opmsg_obpm_set_params},
    {OPMSG_COMMON_ID_GET_STATUS,   iir_resampler_opmsg_obpm_get_status},
#ifdef INSTALL_METADATA
    {OPMSG_COMMON_SET_METADATA_DELAY, iir_resampler_opmsg_set_metadata_delay},
#endif
    {0, NULL}
};

/* Capability data - This is the definition of the capability that Opmgr uses to
 * create the capability from. */
const CAPABILITY_DATA iir_resampler_cap_data =
{
    IIR_RESAMPLER_CAP_ID,           /* Capability ID */
    0, 1,                           /* Version information - hi and lo parts */
    IIR_RESAMPLER_MAX_CHANNELS,     /* Max number of sinks/inputs */
    IIR_RESAMPLER_MAX_CHANNELS,     /* Max number of sources/outputs */
    &iir_resampler_handler_table,   /* Pointer to message handler function table */
    iir_resampler_opmsg_handler_table,      /* Pointer to operator message handler function table */
    iir_resampler_process_data,             /* Pointer to data processing function */
    0,                              /* TODO: this would hold processing time information */
    sizeof(IIR_RESAMPLER_OP_DATA)   /* Size of capability-specific per-instance data */
};

bool iir_resampler_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    OP_CHANNEL_LIST* chan, *last_chan;
    IIR_RESAMPLER_OP_DATA *op_extra_data = (IIR_RESAMPLER_OP_DATA*)op_data->extra_op_data;
    int num_channels;

    /* Form the response (assumes success) */
    *response_id = OPCMD_START;
    if(!base_op_build_std_response(STATUS_OK,op_data->id,response_data))
    {
        return(FALSE);
    }

    /* if the operator is already running, ignore the start_req */
    if(op_data->state == OP_RUNNING)
    {
        return(TRUE);
    }

    /* start will fail if sample rates have not been set */
    if( (op_extra_data->in_rate  == IIR_RESAMPLER_NO_SAMPLE_RATE) ||
        (op_extra_data->out_rate == IIR_RESAMPLER_NO_SAMPLE_RATE) )
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    /* determine how many active channels are connected */
    num_channels = op_channel_list_num_connected(&op_extra_data->channel_list);

    /* start will fail if no channels are connected, or if any channel is half
     * connected (indicated by num_channels value -1) */
    if(num_channels <= 0)
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }
    op_extra_data->num_channels = (unsigned) num_channels;

    /* compute touched mask for active channels, if we're in stream_based mode
     * then update amt_to_use_channel_list to point to the last element */
    op_extra_data->touched_mask = TOUCHED_NOTHING;
    last_chan = chan = op_extra_data->channel_list;
    while(chan)
    {
        op_extra_data->touched_mask |= (TOUCHED_SINK_0 << chan->index);
        last_chan = chan;
        chan = chan->next;
    }
    if (op_extra_data->config & IIR_RESAMPLER_CONFIG_STREAM_BASED)
    {
        op_extra_data->amt_to_use_channel_list = last_chan;
    }
    else
    {
        op_extra_data->amt_to_use_channel_list = op_extra_data->channel_list;
    }

    /* attempt to setup internal processing structures (iir_resamplerv2) */
    if(!init_iir_resampler_internal(op_extra_data))
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

#ifdef INSTALL_METADATA
    /* Resampler has a delay, so add that delay to the metadata. TODO this will
     * need changing if the conversion is changed on the fly. */
    buff_metadata_add_delay(op_extra_data->metadata_op_buffer, op_extra_data->metadata_delay);

    op_extra_data->last_tag.type = UNSUPPORTED_TAG;
#endif /* INSTALL_METADATA */

    op_data->state = OP_RUNNING;
    return TRUE;
}

bool iir_resampler_stop(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    /* set response id and call shared stop_reset function */
    *response_id = OPCMD_STOP;
    return(iir_resampler_stop_reset(op_data,response_data));
}

bool iir_resampler_reset(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{

    /* set response id and call shared stop_reset function */
    *response_id = OPCMD_RESET;
    return(iir_resampler_stop_reset(op_data,response_data));
}

bool iir_resampler_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    IIR_RESAMPLER_OP_DATA *op_extra_data = (IIR_RESAMPLER_OP_DATA*)op_data->extra_op_data;

    /* call base_op destroy that creates and fills response message, too */
    if(!base_op_destroy(op_data, message_data, response_id, response_data))
    {
        return(FALSE);
    }

    /* make sure that all internal structures are free */
    free_iir_resampler_internal(op_extra_data);

    /* deregister intent to use scratch memory */
    scratch_deregister();

    /* free all allocated channel list data */
    op_channel_list_free_list(&op_extra_data->channel_list);

    return(TRUE);
}

bool iir_resampler_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    IIR_RESAMPLER_OP_DATA *op_extra_data = (IIR_RESAMPLER_OP_DATA*)op_data->extra_op_data;
    L3_DBG_MSG("iir_resampler create \n");

    /* Form the response (assumes success). Set operator to not running state */
    if(!base_op_create(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* register intent to use scratch memory utility */
    if(!scratch_register())
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    patch_fn_shared(iir_resampler);

    /* iir_resamplerv2 internal data will be allocated at opcmd start */
    op_extra_data->iir_resamplerv2 = NULL;

    /* iir_resamplerv2 shared filter memory will be requested at opcmd start */
    op_extra_data->lpconfig = NULL;

    /* scratch memory will not be reserved until opcmd start */
    op_extra_data->scratch_reserved = 0;

    /* in/out rates have not been set */
    op_extra_data->in_rate  = IIR_RESAMPLER_NO_SAMPLE_RATE;
    op_extra_data->out_rate = IIR_RESAMPLER_NO_SAMPLE_RATE;

    /* iir_resamplev2 intermediate buffer should be as large as input buffers
     * to avoid throttling data flow. Default to smaller buffer size, but may
     * be updated at iir_resampler_buffer_details */
    op_extra_data->temp_buffer_size = IIR_RESAMPLER_BUFFER_SIZE;

    /* dbl_precision, low_mips and stream_based processing are disabled by default */
    op_extra_data->config = 0;

#ifdef INSTALL_METADATA
    op_extra_data->last_tag.type = UNSUPPORTED_TAG;
#endif /* INSTALL_METADATA */

    return TRUE;
}

bool iir_resampler_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    IIR_RESAMPLER_OP_DATA *op_extra_data = (IIR_RESAMPLER_OP_DATA*)op_data->extra_op_data;

    unsigned terminal_id;
    OP_BUF_DETAILS_RSP *resp;

    /* Form default buffer details response */
    if(!base_op_buffer_details(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    resp = (OP_BUF_DETAILS_RSP*) *response_data;

    patch_fn_shared(iir_resampler);

#ifdef INSTALL_METADATA
    terminal_id = ((unsigned *)message_data)[0];
    resp->supports_metadata = TRUE;
    if ((terminal_id & TERMINAL_SINK_MASK) == TERMINAL_SINK_MASK)
    {
        resp->metadata_buffer = op_extra_data->metadata_ip_buffer;
    }
    else
    {
        resp->metadata_buffer = op_extra_data->metadata_op_buffer;
    }
#else
    NOT_USED(terminal_id);
#endif /* INSTALL_METADATA */

    resp->b.buffer_size = IIR_RESAMPLER_BUFFER_SIZE;
    /* update iir_resamplev2 intermediate buffer size */
    op_extra_data->temp_buffer_size = IIR_RESAMPLER_BUFFER_SIZE;

    return TRUE;
}

bool iir_resampler_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    IIR_RESAMPLER_OP_DATA *op_extra_data;
    unsigned terminal_num, terminal_id;
    tCbuffer* pterminal_buf;

    L3_DBG_MSG("iir_resampler connect  \n");

    /* build the message response assuming success */
    if(!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }

    /* Only allow connection when operator is not running */
    if(op_data->state != OP_NOT_RUNNING)
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    /* get the terminal_id and check if it is valid */
    terminal_id  = ((unsigned*)message_data)[0];
    if(!base_op_is_terminal_valid(op_data, terminal_id))
    {
        base_op_change_response_status(response_data, STATUS_INVALID_CMD_PARAMS);
        return TRUE;
    }

    /* set the buffer pointer for this operator terminal */
    op_extra_data = (IIR_RESAMPLER_OP_DATA*)op_data->extra_op_data;
    terminal_num = terminal_id & TERMINAL_NUM_MASK;
    pterminal_buf = (tCbuffer*)(uintptr_t)(((unsigned *)message_data)[1]);
    if(terminal_id & TERMINAL_SINK_MASK)
    {
#ifdef INSTALL_METADATA
        if( !op_channel_list_connect(&op_extra_data->channel_list, pterminal_buf, terminal_num,
                                    OP_CHANNEL_LIST_SINK, &op_extra_data->metadata_ip_buffer) )
#else
        if( !op_channel_list_connect(&op_extra_data->channel_list, pterminal_buf, terminal_num, OP_CHANNEL_LIST_SINK) )
#endif /* INSTALL_METADATA */
        {
            /* Should never try to connect an already connected terminal*/
            base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            return TRUE;
        }
    }
    else
    {
#ifdef INSTALL_METADATA
        if( !op_channel_list_connect(&op_extra_data->channel_list, pterminal_buf, terminal_num,
                                    OP_CHANNEL_LIST_SOURCE, &op_extra_data->metadata_op_buffer) )
#else
        if( !op_channel_list_connect(&op_extra_data->channel_list, pterminal_buf, terminal_num, OP_CHANNEL_LIST_SOURCE) )
#endif /* INSTALL_METADATA */
        {
            /* Should never try to connect an already connected terminal*/
            base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            return TRUE;
        }
    }

    return TRUE;
}

bool iir_resampler_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    IIR_RESAMPLER_OP_DATA *op_extra_data;
    unsigned terminal_num, terminal_id;

    L3_DBG_MSG("iir_resampler disconnect\n");

    /* build the message response assuming success */
    if(!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }

    /* get the terminal_id and check if it is valid */
    terminal_id  = ((unsigned*)message_data)[0];
    if(!base_op_is_terminal_valid(op_data, terminal_id))
    {
        base_op_change_response_status(response_data, STATUS_INVALID_CMD_PARAMS);
        return TRUE;
    }

    /* clear the buffer pointer for this operator terminal */
    op_extra_data = (IIR_RESAMPLER_OP_DATA*)op_data->extra_op_data;
    terminal_num = terminal_id & TERMINAL_NUM_MASK;
    {
        bool disconnected;

        /* Protect against process_data preempting us in the case where the
         * operator is running */
        interrupt_block();
        if(terminal_id & TERMINAL_SINK_MASK)
        {
#ifdef INSTALL_METADATA
            disconnected
                = op_channel_list_disconnect(&op_extra_data->channel_list,terminal_num,
                                             OP_CHANNEL_LIST_SINK, &op_extra_data->metadata_ip_buffer);
#else
            disconnected
                = op_channel_list_disconnect(&op_extra_data->channel_list, terminal_num,
                                             OP_CHANNEL_LIST_SINK);
#endif /* INSTALL_METADATA */
        }
        else
        {
#ifdef INSTALL_METADATA
            disconnected
                = op_channel_list_disconnect(&op_extra_data->channel_list, terminal_num,
                                             OP_CHANNEL_LIST_SOURCE, &op_extra_data->metadata_op_buffer);
#else
            disconnected
                = op_channel_list_disconnect(&op_extra_data->channel_list, terminal_num,
                                             OP_CHANNEL_LIST_SOURCE);
#endif /* INSTALL_METADATA */
        }
        interrupt_unblock();

        if (!disconnected)
        {
            /* Should never try to disconnect an already disconnected terminal*/
            base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            return TRUE;
        }
    }

    return TRUE;
}


bool iir_resampler_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    OP_SCHED_INFO_RSP* resp;

    /* allocate sched info response */
    if (!base_op_get_sched_info(op_data, message_data, response_id, response_data))
    {
        return base_op_build_std_response(STATUS_CMD_FAILED, op_data->id, response_data);
    }

    resp = (OP_SCHED_INFO_RSP*) *response_data;
    /* Populate the response*/
    resp->op_id = op_data->id;
    resp->status = STATUS_OK;
    resp->block_size = IIR_RESAMPLER_DEFAULT_BLOCK_SIZE;
    resp->run_period = 0;

    *response_data = resp;
    return TRUE;
}

static unsigned call_iir_resampler_library(IIR_RESAMPLER_OP_DATA *op_extra_data, unsigned consumed)
{
    unsigned produced;
    /* get a pointer to the scratch memory (Commit) for iir_resamplerv2 intermediate buffer */
    unsigned *temp_buffer_ptr = (unsigned*) scratch_commit(op_extra_data->scratch_reserved, MALLOC_PREFERENCE_NONE);

    /* update intermediate data pointer in iir_resamplerv2 parameter structure */
    op_extra_data->iir_resamplerv2->common.intermediate_ptr = temp_buffer_ptr;

    /* lets see how many samples we can actually genetate */
    produced = iir_resampler_processing(op_extra_data, consumed);

    /* free the scratch memory used for temp intermediate buffer */
    scratch_free();

    return produced;
}

static void update_channels_rd_wr_pointers(OP_CHANNEL_LIST *chan, unsigned consumed, unsigned produced)
{
    while(chan)
    {
        cbuffer_advance_read_ptr(chan->pterminal_buf[OP_CHANNEL_LIST_SINK], consumed);
        cbuffer_advance_write_ptr(chan->pterminal_buf[OP_CHANNEL_LIST_SOURCE], produced);
        chan = chan->next;
    }
}


#ifdef INSTALL_METADATA

/**
 *  Returns the first tag which changes the type.
 *
 *  NOTE: There are three types of tags which can cause transition:
 *      1) Timestamped and time of arrival tags are in the same group.
 *      2) Void tags.
 *      3) End of file tags.
 *
 * \param   op_extra_data  The extra operator data of the resampler.
 * \param   tag_list Pointer to the head of tag list.
 *
 * \return  The first tag which will case a transition in the internal metadata state
 *          of the operator.
 */
static void insert_samples_to_output_channels(OP_CHANNEL_LIST *chan, unsigned samples)
{
    tCbuffer * output_buffer;
    unsigned buffer_size;
    int last_sample;
    if (samples == 0)
    {
        /* Nothing to do*/
        return;
    }
    IIR_DBG_MSG1("IIR resampler: insert samples 0x%08x", samples);

    patch_fn_shared(iir_resampler);

    while(chan)
    {
        output_buffer = chan->pterminal_buf[OP_CHANNEL_LIST_SOURCE];
        buffer_size = cbuffer_get_size_in_words(output_buffer);

        IIR_DBG_MSG3("IIR resampler channel 0x%08x: output_buffer space  = 0x%08x, output_buffer data = 0x%08x",
                  (uintptr_t)(chan), cbuffer_calc_amount_space_in_words(output_buffer), cbuffer_calc_amount_data_in_words(output_buffer));

        /*get the last sample by advanceing the read pointer by buffer size - 1 */
        cbuffer_advance_read_ptr(output_buffer, buffer_size - 1);
        cbuffer_read(output_buffer, &last_sample, 1);

        /* due to the cbuffer_read the read pointer should be in the same place. */
        if (output_buffer != NULL)
        {
            /* TODO use the last sample value! */
            cbuffer_block_fill(output_buffer, samples, last_sample);
        }

        IIR_DBG_MSG4("IIR resampler channel 0x%08x: output_buffer space  = 0x%08x, output_buffer data = 0x%08x, last_sample = 0x%08x",
                  (uintptr_t)(chan), cbuffer_calc_amount_space_in_words(output_buffer), cbuffer_calc_amount_data_in_words(output_buffer), last_sample);

        chan = chan->next;
    }
}


/**
 *  Returns the distance (in samples) from the beginning of the available metadata for
 *  a given tag.
 *
 * \param   tag  Pointer to the tag
 * \param   metadata_buff Pointer to the metadata buffer.
 *
 * \return  Distance in samples to the specified tag.
 */
static unsigned get_samples_to_tag(metadata_tag *tag, tCbuffer  *metadata_buff)
{
    unsigned offset;
    unsigned ret_val;
    unsigned buffer_size = cbuffer_get_size_in_octets(metadata_buff);
    offset = buff_metadata_get_read_offset(metadata_buff);

    /* Calculate the samples for until the tag. */
    ret_val = tag->index - offset;
    if (ret_val > buffer_size)
    {
        ret_val = ret_val + buffer_size;
    }
    return ret_val/ OCTETS_PER_SAMPLE;
}

/* Function checks if the tag is end of file or void.*/
static inline bool is_eof_or_void(metadata_tag* tag)
{
    return METADATA_STREAM_END(tag) || IS_VOID_TTP_TAG(tag) || IS_EMPTY_TAG(tag);
}

/* Function checks if the tag is end of file, timestamped or time of arrival tag.*/
static inline bool is_eof_or_timestamp_or_toa(metadata_tag* tag)
{
    return METADATA_STREAM_END(tag) ||
            IS_TIME_OF_ARRIVAL_TAG(tag) ||
            IS_TIMESTAMPED_TAG(tag);
}

/**
 *  Returns the first transitional tag. Transitional tag is a tag which has different
 *  type than the previous.
 *
 *  NOTE: There are three types of tags which can cause transition:
 *      1) Timestamped and time of arrival tags are in the same group.
 *      2) Void tags.
 *      3) End of file tags.
 *
 * \param   op_extra_data  The extra operator data of the resampler.
 * \param   tag_list Pointer to the head of tag list.
 *
 * \return  The first tag which will cause a transition in the internal metadata state
 *          of the operator.
 */
static metadata_tag* get_first_transitional_tag(IIR_RESAMPLER_OP_DATA *op_extra_data,
        metadata_tag* tag_list)
{
    if (op_extra_data->last_tag.type == UNSUPPORTED_TAG)
    {
        return tag_list;
    }
    else if ((op_extra_data->last_tag.type == TIMESTAMPED_TAG) ||
            (op_extra_data->last_tag.type == TIME_OF_ARRIVAL_TAG))
    {
        while ((tag_list != NULL) && !is_eof_or_void(tag_list))
        {
            tag_list = tag_list->next;
        }
    }
    else if (op_extra_data->last_tag.type == VOID_TAG)
    {
        while ((tag_list != NULL) && !is_eof_or_timestamp_or_toa(tag_list))
        {
            tag_list = tag_list->next;
        }
    }
    return tag_list;
}


/**
 * Returns the last tag from the list.
 *
 * \param    tag_list Pointer to the head of tag list.
 *
 * \return   Pointer to the last tag from the list.
 */
static metadata_tag* get_last_tag(metadata_tag* tag_list)
{
    if (tag_list == NULL)
    {
        return NULL;
    }

    while (tag_list->next != NULL)
    {
        tag_list = tag_list->next;
    }
    return tag_list;
}

/**
 *  Save a given metadata tag to the last_tag field..
 *
 * \param    op_extra_data  The extra operator data of the resampler.
 * \param    tag    Tag to save in the last tag field.
 *
 * \return   Pointer to the created tag.
 */
static void save_timestamped_tag_data(IIR_RESAMPLER_OP_DATA *op_extra_data, metadata_tag* tag)
{
    unsigned *err_offset_id;
    unsigned length;

    op_extra_data->last_tag.samples_after = 0;

    /* Save the timestamp info from the incoming metadata */
    op_extra_data->last_tag.timestamp = tag->timestamp;
    op_extra_data->last_tag.spa = tag->sp_adjust;
    if (buff_metadata_find_private_data(tag, META_PRIV_KEY_TTP_OFFSET, &length, (void **)&err_offset_id))
    {
        op_extra_data->last_tag.err_offset_id = (*err_offset_id);
    }
    else
    {
        op_extra_data->last_tag.err_offset_id = INFO_ID_INVALID;
    }
}

/**
 *  Function used to create the output tag. Depending on the last tag read from the
 *  input buffer the created tag can be timestamped, time of arrival or void.
 *
 *  NOTE: The next field of the created output tag is not set.
 *
 * \param    op_extra_data  The extra operator data of the resampler.
 * \param    length  Sets the length of the newly created tag.
 *
 * \return   Pointer to the created tag.
 */
static metadata_tag* create_output_tag(IIR_RESAMPLER_OP_DATA *op_extra_data, unsigned length)
{
    metadata_tag* ret_tag;

    /* Bail out early if the length is 0. This is could happen if the operator only
     * consumed data. */
    if (length == 0)
    {
        return NULL;
    }

    /* Create only one tag for the outgoing data.*/
    ret_tag = buff_metadata_new_tag();
    if(ret_tag == NULL)
    {
        IIR_DBG_MSG1("IIR 0x%08x: Metadata allocation failed.", (uintptr_t)(op_extra_data));
        return NULL;
    }

    if (op_extra_data->last_tag.type == TIMESTAMPED_TAG)
    {
        unsigned new_ttp;
        ttp_status status;

        status.sp_adjustment = op_extra_data->last_tag.spa;
        status.err_offset_id = op_extra_data->last_tag.err_offset_id;
        status.stream_restart = (METADATA_STREAM_START(ret_tag) != 0);

        /* Calculate new TTP from incoming data and sample offset */
        new_ttp = ttp_get_next_timestamp(
                op_extra_data->last_tag.timestamp,
                op_extra_data->last_tag.samples_after,
                op_extra_data->in_rate,
                op_extra_data->last_tag.spa);
        new_ttp = time_sub(new_ttp,
                convert_samples_to_time(
                        op_extra_data->metadata_delay,
                        op_extra_data->in_rate));
        status.ttp = new_ttp;
        ttp_utils_populate_tag(ret_tag, &status);

        IIR_DBG_MSG4(
                "IIR 0x%08x: TIMESTAMPED last tag timestamp = 0x%08x, "
                "samples_after = 0x%08x, input_buff_after_index = 0x%08x",
                (uintptr_t)(op_extra_data),
                op_extra_data->last_tag.timestamp,
                op_extra_data->last_tag.samples_after,
                new_ttp
                );
    }
    else if (op_extra_data->last_tag.type == TIME_OF_ARRIVAL_TAG)
    {
        unsigned time_of_arrival;
        /* Create a time of arrival tag. */
        time_of_arrival = ttp_get_next_timestamp(
                op_extra_data->last_tag.timestamp,
                op_extra_data->last_tag.samples_after,
                op_extra_data->in_rate,
                op_extra_data->last_tag.spa);
        time_of_arrival = time_sub(time_of_arrival,
                convert_samples_to_time(
                        op_extra_data->metadata_delay,
                        op_extra_data->in_rate));
        ret_tag->sp_adjust = op_extra_data->last_tag.spa;
        METADATA_TIME_OF_ARRIVAL_SET(ret_tag, time_of_arrival);

        IIR_DBG_MSG4(
                "IIR 0x%08x: TIME_OF_ARRIVAL last tag time of arrival = 0x%08x, "
                "samples_after = 0x%08x, input_buff_after_index = 0x%08x",
                (uintptr_t)(op_extra_data),
                op_extra_data->last_tag.timestamp,
                op_extra_data->last_tag.samples_after,
                time_of_arrival);
    }
    else if (op_extra_data->last_tag.type == VOID_TAG)
    {
        /* Just mark the tag as a void one. */
        METADATA_VOID_TTP_SET(ret_tag);
    }
    else
    {
        /* Check if the state is valid. */
        PL_ASSERT(op_extra_data->last_tag.type != UNSUPPORTED_TAG);
    }

    /* Set the length of the tag. */
    ret_tag->length = length;

    return ret_tag;
}


/**
 *  Function used to transport metadata from the input buffer to the output buffer.
 *
 *  NOTE: Tags from the input metadata buffer will be recreated (and not transported) on
 *  the output.
 *
 * \param    op_extra_data  The extra operator data of the resampler.
 * \param    consumed  Samples consumed form the input channels.
 * \param    produced  Samples porduce on the output channels.
 */
static void resampler_metadata_transport(IIR_RESAMPLER_OP_DATA *op_extra_data,
        unsigned  consumed,          unsigned produced)
{

    /* Remove the tags for the data that has been consumed during this kick
     * update the length fields by the conversion factor as the data has
     * been resampled then append the tags indicating the number of octets
     * the operation produced.
     */
    unsigned output_buff_after_index;
    unsigned input_buff_before_index, input_buff_after_index;
    unsigned produced_octets = produced * OCTETS_PER_SAMPLE;

    tCbuffer* metadata_ip_buff = op_extra_data->metadata_ip_buffer;
    tCbuffer* metadata_op_buff = op_extra_data->metadata_op_buffer;
    metadata_tag  *tag_list, *out_tag, *last_tag = NULL, *eof_tag = NULL;

    tag_list = buff_metadata_remove(metadata_ip_buff, consumed * OCTETS_PER_SAMPLE,
            &input_buff_before_index, &input_buff_after_index);

    IIR_DBG_MSG4(
            "IIR 0x%08x: tag_list = 0x%08x, input_buff_before_index = 0x%08x, input_buff_after_index = 0x%08x",
            (uintptr_t)(op_extra_data), (uintptr_t)(tag_list), input_buff_before_index, input_buff_after_index
            );

    if(tag_list != NULL)
    {
        /* Check if the tag is void or eof */
        if(IS_VOID_TTP_TAG(tag_list))
        {
            if (op_extra_data->last_tag.type != VOID_TAG)
            {
                IIR_DBG_MSG1("IIR 0x%08x: Switching to void mode",
                            (uintptr_t)(op_extra_data));
                op_extra_data->last_tag.type = VOID_TAG;
            }
            out_tag = create_output_tag(op_extra_data, produced_octets);
        }
        if(IS_EMPTY_TAG(tag_list))
        {
            if (op_extra_data->last_tag.type != EMPTY_TAG)
            {
                IIR_DBG_MSG1("IIR 0x%08x: Switching to empty mode",
                            (uintptr_t)(op_extra_data));
                op_extra_data->last_tag.type = EMPTY_TAG;
            }
            out_tag = create_output_tag(op_extra_data, produced_octets);
        }
        else if(IS_TIMESTAMPED_TAG(tag_list))
        {
            if(op_extra_data->last_tag.type != TIMESTAMPED_TAG)
            {
                IIR_DBG_MSG1("IIR 0x%08x: Switching to timestameped mode",
                           (uintptr_t)(op_extra_data));
                op_extra_data->last_tag.type = TIMESTAMPED_TAG;
                /* First valid tag save it for the first tag creation. */
                save_timestamped_tag_data(op_extra_data, tag_list);
            }
            out_tag = create_output_tag(op_extra_data, produced_octets);

        }
        else if(IS_TIME_OF_ARRIVAL_TAG(tag_list))
        {
            if(op_extra_data->last_tag.type != TIME_OF_ARRIVAL_TAG)
            {
                IIR_DBG_MSG1("IIR 0x%08x: Switching to time of arrival mode",
                            (uintptr_t)(op_extra_data));
                op_extra_data->last_tag.type = TIME_OF_ARRIVAL_TAG;
                /* First valid tag save it for the first tag creation. */
                save_timestamped_tag_data(op_extra_data, tag_list);
            }
            out_tag = create_output_tag(op_extra_data, produced_octets);
        }
        else if(METADATA_STREAM_END(tag_list))
        {
            IIR_DBG_MSG1("IIR 0x%08x: End of file tag received. ",
                        (uintptr_t)(op_extra_data));
            /* First create the tag based on the previous state. */
            if (produced_octets == 0)
            {
                /* In some cases the operator only consumes data (If the produced
                 * samples is 0). In this case if an eof tag is read a sample needs
                 * generating to transport this tag.*/
                insert_samples_to_output_channels(op_extra_data->channel_list, 1);
                /* To keep the eof tag 0 length an additional tag will be created. */
                produced_octets = OCTETS_PER_SAMPLE;
            }
            out_tag = create_output_tag(op_extra_data, produced_octets);
            op_extra_data->last_tag.type = UNSUPPORTED_TAG;
        }
        else
        {
            IIR_DBG_MSG1("IIR resampler 0x%08x: Unsupported tag.",
                        (uintptr_t)(op_extra_data));
            out_tag = NULL;
        }

        /* Save the last timestamp/time of arrival to calculate the timestamp/time of
         * arrival for the next run. If the last tag is a eof or a void tag then there
         * is no need to save it. */
        last_tag = get_last_tag(tag_list);
        if(!is_eof_or_void(tag_list))
        {
            save_timestamped_tag_data(op_extra_data, last_tag);
            /* Check if input_buff_after_index is sample aligned. */
            PL_ASSERT(input_buff_after_index%OCTETS_PER_SAMPLE == 0);
            op_extra_data->last_tag.samples_after =
                    (input_buff_after_index / OCTETS_PER_SAMPLE);
        }

    }
    else
    {
        /* Check if the state is different than INVALID.*/
        PL_ASSERT(op_extra_data->last_tag.type != UNSUPPORTED_TAG);

        out_tag = create_output_tag(op_extra_data, produced_octets);
        if(op_extra_data->last_tag.type != VOID_TAG)
        {
            op_extra_data->last_tag.samples_after += consumed;
        }
    }

    /* save the end of file tag */
    if ((last_tag != NULL) && METADATA_STREAM_END(last_tag))
    {
        PL_ASSERT(last_tag->next == NULL);
        eof_tag = buff_metadata_copy_tag(last_tag);
        output_buff_after_index = 0;
    }
    else
    {
        /* Set the after index values to the produced octets. */
        output_buff_after_index = produced_octets;
    }

    /* Free all the incoming tags */
    buff_metadata_tag_list_delete(tag_list);

    /*In some cases the operator only consumes data.*/
    if (out_tag)
    {
        /* Set the next tag to null or to the end of file tag. Not eof_tag is NULL by default.*/
        out_tag->next = eof_tag;

        buff_metadata_append(metadata_op_buff, out_tag, 0, output_buff_after_index);
    }
}
#endif

/* Data processing function */
void iir_resampler_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    IIR_RESAMPLER_OP_DATA *op_extra_data = (IIR_RESAMPLER_OP_DATA*)op_data->extra_op_data;
    unsigned consumed, produced, process_all_input, channel_disconnected;


#ifdef INSTALL_METADATA
    unsigned meta_available_samples;
    unsigned samples_to_tag;
    metadata_tag* tag_list;
    metadata_tag* transition_tag;
    tCbuffer* metadata_ip_buff = op_extra_data->metadata_ip_buffer;
    tCbuffer* metadata_op_buff = op_extra_data->metadata_op_buffer;
#endif
    patch_fn(iir_resampler_process_data);



    /* compute amount to use */
    consumed = iir_resampler_amount_to_use(op_extra_data, &process_all_input, &channel_disconnected);

    IIR_DBG_MSG2("IIR 0x%08x: consume = %d", (uintptr_t)(op_extra_data), consumed);

    /* bail early if channel disconnected */
    if(channel_disconnected == 1)
    {
        IIR_DBG_MSG1("IIR 0x%08x: channel disconnected", (uintptr_t)(op_extra_data));
        return;
    }

#ifdef INSTALL_METADATA
    if (metadata_ip_buff != NULL)
    {
        /* Limit the consumed data to the available metadata. */
        meta_available_samples = buff_metadata_available_octets(metadata_ip_buff) / OCTETS_PER_SAMPLE;
        if (consumed > meta_available_samples)
        {
            IIR_DBG_MSG2("IIR 0x%08x: fewer metadata than data, only consume = %d",
                                    (uintptr_t)(op_extra_data), meta_available_samples);
            consumed = meta_available_samples;
            /* Not processing all the inputs any more. */
            process_all_input = FALSE;
        }

        /* get the tag list */
        tag_list = buff_metadata_peek(metadata_ip_buff);

        transition_tag = get_first_transitional_tag(op_extra_data,tag_list);

        if(transition_tag != NULL)
        {
            samples_to_tag = get_samples_to_tag(transition_tag, metadata_ip_buff);
            /* Avoid to consume */
            if ((samples_to_tag != 0)&&(consumed > samples_to_tag))
            {
                IIR_DBG_MSG2("IIR 0x%08x: special tag, only consume = %d",
                                        (uintptr_t)(op_extra_data), meta_available_samples);
                consumed = samples_to_tag;
                /* Not processing all the inputs any more. */
                process_all_input = FALSE;
            }
        }
    }
#endif
    /* bail early if there is no input data */
    if(consumed == 0)
    {
        return;
    }

    if (process_all_input)
    {
        /* kick backward on the input terminals if all data is consumed. */
        touched->sinks = op_extra_data->touched_mask;
    }

    /* call library */
    produced = call_iir_resampler_library(op_extra_data, consumed);
    IIR_DBG_MSG2("IIR 0x%08x: produced = 0x%08x", (uintptr_t)(op_extra_data), produced);


#ifdef INSTALL_METADATA
    if ((metadata_ip_buff != NULL) && (metadata_op_buff != NULL))
    {
        /* transport the metadata. */
        resampler_metadata_transport(op_extra_data, consumed, produced);
    }
    else if ((metadata_ip_buff != NULL) && (metadata_op_buff == NULL))
    {
        unsigned b4idx, afteridx;
        /* Remove the tags from the input buffer. */
        tag_list = buff_metadata_remove(metadata_ip_buff, consumed* OCTETS_PER_SAMPLE ,
                                        &b4idx, &afteridx);
        buff_metadata_tag_list_delete(tag_list);
    }
    else if ((metadata_ip_buff == NULL) && (metadata_op_buff != NULL))
    {
        unsigned produced_octets = produced* OCTETS_PER_SAMPLE;
        /* there is no input metadata. Just create an empty tag to cover the samples. */
        tag_list = buff_metadata_new_tag();
        if (tag_list != NULL)
        {
            tag_list->length = produced_octets;
        }
        buff_metadata_append(metadata_op_buff, tag_list, 0, produced_octets);
    }
#endif /* INSTALL_METADATA */

    /* update terminal buffer read/write pointers */
    update_channels_rd_wr_pointers(op_extra_data->channel_list, consumed, produced);

    /* kick forward on the output terminals if data was produced */
    if(produced > 0)
    {
        touched->sources = op_extra_data->touched_mask;
    }
}

/**
 * handler for iir_resampler set rates opmsg. The message contains two words:
 *    payload: [in_rate/25], [out_rate/25]
 * where in_rate and out_rate are the input frequency and output frequency in Hz
 */
bool iir_resampler_opmsg_set_sample_rates(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    unsigned in_rate, out_rate;

    in_rate  = CONVERSION_SAMPLE_RATE_TO_HZ * OPMSG_FIELD_GET(message_data, OPMSG_MSG_IIR_RESAMPLER_SET_SAMPLE_RATES, INPUT_RATE);
    out_rate = CONVERSION_SAMPLE_RATE_TO_HZ * OPMSG_FIELD_GET(message_data, OPMSG_MSG_IIR_RESAMPLER_SET_SAMPLE_RATES, OUTPUT_RATE);

    return set_rates_iir_resampler_internal(op_data, in_rate, out_rate);
}

/**
 * handler for legacy set conversion rates opmsg. The message contains one word:
 *    payload: [rate_index]
 */
bool iir_resampler_opmsg_set_conversion_rate(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    unsigned rate_index = ((unsigned*)message_data)[3];
    unsigned in_rate    = (rate_index & 0xF0) >> 4;
    unsigned out_rate   = (rate_index & 0x0F);

    /* check that requested rate indeces are valid */
    if( (in_rate  >= IIR_RESAMPLER_RATE_INDEX_LUT_LEN) ||
        (out_rate >= IIR_RESAMPLER_RATE_INDEX_LUT_LEN) )
    {
        return FALSE;
    }

    /* read conversion rate frequencies (Hz) from rate index look-up table */
    in_rate  = iir_resampler_rate_index_lut[in_rate];
    out_rate = iir_resampler_rate_index_lut[out_rate];

    return set_rates_iir_resampler_internal(op_data, in_rate, out_rate);
}

/**
 * handler for set_config opmsg. set resampler configuration bit flags.
 *     one 16-bit word payload:
 *          +---------------+---+---+
 *          | 15 <------> 2 | 1 | 0 |
 *          |    RESERVED   | L | D |    L=low_mips, D=dbl_precision
 *          +---------------+---+---+
 */
bool iir_resampler_opmsg_set_config(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    IIR_RESAMPLER_OP_DATA   *op_extra_data = (IIR_RESAMPLER_OP_DATA*)op_data->extra_op_data;
    unsigned config = ((unsigned*)message_data)[3];

    /* ignore undefined config bits */
    config &= IIR_RESAMPLER_EX_CONFIG_MASK;

    if((op_extra_data->config & IIR_RESAMPLER_EX_CONFIG_MASK) == config)
    {
        /* configuration has not changed, so nothing to do */
        return TRUE;
    }

    op_extra_data->config = (op_extra_data->config & IIR_RESAMPLER_INT_CONFIG_MASK) | config;

    /* if the op is running, need to clear up and reinitialize internal data */
    if(op_data->state == OP_RUNNING)
    {
        /* set state to not running, while we attempt to reinitialize */
        op_data->state = OP_NOT_RUNNING;

        /* attempt to setup internal processing for the new configuration */
        if(!init_iir_resampler_internal(op_extra_data))
        {
            /* return failure, op will remain in not running state */
            return FALSE;
        }

        /* successfully initialized new configuration, resume running state */
        op_data->state = OP_RUNNING;
    }

    return TRUE;
}

/*
 * iir_resampler_data_stream_based
 */
bool iir_resampler_data_stream_based(OPERATOR_DATA *op_data, void *message_data,
                                            unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    IIR_RESAMPLER_OP_DATA   *op_extra_data = (IIR_RESAMPLER_OP_DATA*)op_data->extra_op_data;

    /* The 3rd message word is a boolean value */
    if (((unsigned*)message_data)[3])
    {
        op_extra_data->config |= IIR_RESAMPLER_CONFIG_STREAM_BASED;
        if (op_data->state == OP_RUNNING)
        {
            OP_CHANNEL_LIST *element = op_extra_data->channel_list;
            /* Find the last entry in the list and just consider that for amount to use calculations */
            for (; (element != NULL) && (element->next != NULL); element = element->next);
            op_extra_data->amt_to_use_channel_list = element;
        }
    }
    else
    {
        op_extra_data->config &= ~IIR_RESAMPLER_CONFIG_STREAM_BASED;
        /* If the operator is running then we should start using the whole list immediately */
        if (op_data->state == OP_RUNNING)
        {
            op_extra_data->amt_to_use_channel_list = op_extra_data->channel_list;
        }
    }

    return TRUE;
}

#ifdef INSTALL_METADATA
bool iir_resampler_opmsg_set_metadata_delay(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    IIR_RESAMPLER_OP_DATA *op_extra_data = (IIR_RESAMPLER_OP_DATA*)op_data->extra_op_data;

    /* The message value is in samples. Convert it to octets */
    op_extra_data->metadata_delay = OCTETS_PER_SAMPLE * (((unsigned*)message_data)[3]);

    return (TRUE);
}
#endif /* INSTALL_METADATA */

/**
 * \brief Calculates the conversion ratio as a fraction which represents integer
 * and fractional parts shifted to be stored as single precision fraction.
 *
 * \param in_rate The input terminal's sample rate
 * \param out_rate The output terminal's sample rate
 * \return The conversion factor encoded as a fraction
 */
static unsigned calc_conv_factor(unsigned in_rate, unsigned out_rate)
{
    /* As the fractional representation begins at the DAWTH-1 bit the integer
     * part of the conversion factor is bits DAWTH-1 to DAWTH-1 -INT_PART_BITS. */
    if (in_rate > out_rate)
    {
        unsigned conv_fact = frac_div(out_rate, in_rate);
        return conv_fact >> INT_PART_BITS;
    }
    else if (in_rate == out_rate)
    {
        return 1 << (DAWTH -INT_PART_BITS -1);
    }
    else
    {
        unsigned int_part = out_rate/in_rate;
        unsigned remainder = out_rate - int_part * in_rate;
        unsigned conv_fact = frac_div(remainder, in_rate);

        conv_fact >>= INT_PART_BITS;

        conv_fact |= int_part << (DAWTH -INT_PART_BITS -1);
        return conv_fact;
    }
}



/**
 * set_rates_iir_resampler_internal
 *   helper function to handle changes in sample rate.
 *
 * For multichannel iir_resampler operators, the same conversion will be applied
 * to every active channel, so all input channels of an iir_resampler must
 * receive data at the same rate, and all output channels will produce data at
 * the same rate. If different rates are desired per channel, multiple instances
 * of iir_resampler should be used.
 *
 * The sample rates may be changed while the operator is running, but this will
 * require the operator to free and reallocate/reinitialize all internal data
 * within the opmsg handler. If this fails for a reason such as insufficient RAM
 * the operator will return to a not-running state and must be restarted when
 * enough resources are available before processing will resume.
 */
bool set_rates_iir_resampler_internal(OPERATOR_DATA *op_data, unsigned in_rate, unsigned out_rate)
{
    bool low_mips;
    IIR_RESAMPLER_OP_DATA   *op_extra_data = (IIR_RESAMPLER_OP_DATA*)op_data->extra_op_data;

    patch_fn_shared(iir_resampler);

    if( (in_rate  == op_extra_data->in_rate) &&
        (out_rate == op_extra_data->out_rate) )
    {
        /* sample rates have not changed, so nothing to do */
        return TRUE;
    }

    /* based on current config, check if a low_mips config is desired */
    low_mips = (op_extra_data->config & IIR_RESAMPLER_CONFIG_LOW_MIPS) ? TRUE : FALSE;

    /* check if the requested sample rate corresponds to an existing resampler
     * configuration */
    if( (in_rate != out_rate) &&
        (iir_resamplerv2_get_id_from_rate(in_rate, out_rate, low_mips) == 0) )
    {
        return FALSE;
    }

    op_extra_data->in_rate = in_rate;
    op_extra_data->out_rate = out_rate;

    op_extra_data->conv_fact = calc_conv_factor(in_rate, out_rate);

    /* if the op is running, need to clear up and reinitialize internal data */
    if(op_data->state == OP_RUNNING)
    {
        /* set state to not running, while we attempt to reinitialize */
        op_data->state = OP_NOT_RUNNING;

        /* attempt to setup internal processing for the new configuration */
        if(!init_iir_resampler_internal(op_extra_data))
        {
            /* return failure, op will remain in not running state */
            return FALSE;
        }

        /* successfully initialized new configuration, resume running state */
        op_data->state = OP_RUNNING;
    }

    return TRUE;
}

/**
 * Attempt to (re)initailize all internal structures for iir_resamplerv2
 * processing.
 *     returns - FALSE if initialization failed, TRUE on success
 */
bool init_iir_resampler_internal(IIR_RESAMPLER_OP_DATA* op_extra_data)
{
    void *lpconfig;
    iir_resampler_internal *iir_resamplerv2;
    unsigned num_channels, in_rate, out_rate, hist_size=0;
    bool dbl_precision, low_mips;

    patch_fn_shared(iir_resampler);

    /* check to ensure that no internal structures are currently allocated */
    free_iir_resampler_internal(op_extra_data);

    /* reserve the scratch memory required for iir_resamplerv2 intermediate buffer */
    if(!scratch_reserve(op_extra_data->temp_buffer_size*sizeof(unsigned), MALLOC_PREFERENCE_NONE))
    {
        free_iir_resampler_internal(op_extra_data);
        return(FALSE);
    }

    /* scratch memory has been reserved, remember how much to release */
    op_extra_data->scratch_reserved = op_extra_data->temp_buffer_size*sizeof(unsigned);

    dbl_precision = (op_extra_data->config & IIR_RESAMPLER_CONFIG_DBL_PRECISION) ? TRUE : FALSE;
    low_mips = (op_extra_data->config & IIR_RESAMPLER_CONFIG_LOW_MIPS) ? TRUE : FALSE;

    in_rate = op_extra_data->in_rate;
    out_rate = op_extra_data->out_rate;
    num_channels = op_extra_data->num_channels;

    /* allocate filter */
    lpconfig = iir_resamplerv2_allocate_config_by_rate(in_rate, out_rate, low_mips);
    if(lpconfig)
    {
        hist_size = iir_resamplerv2_get_buffer_sizes(lpconfig,dbl_precision);
    }
    /* filter should have been allocated unless passthrough mode */
    else if (in_rate != out_rate)
    {
        free_iir_resampler_internal(op_extra_data);
        return(FALSE);
    }
    /* save pointer to iir_resamplerv2 shared filter memory */
    op_extra_data->lpconfig = lpconfig;

    /* allocate internal iir_resamplerv2 data structure for N channels */
    iir_resamplerv2 = (iir_resampler_internal*) xzppmalloc(sizeof(iir_resampler_internal) +
                                            num_channels*(sizeof(iir_resamplerv2_channel) + hist_size*sizeof(unsigned)), MALLOC_PREFERENCE_DM1);
    if(!iir_resamplerv2)
    {
        free_iir_resampler_internal(op_extra_data);
        return(FALSE);
    }
    /* save pointer to iir_resamplerv2 internal data in op_extra_data */
    op_extra_data->iir_resamplerv2 = iir_resamplerv2;

    /* initialize common parameters */
    iir_resamplerv2->common.intermediate_size = op_extra_data->temp_buffer_size;
    iir_resamplerv2->common.dbl_precission    = dbl_precision;

    /* the history buffer starts after the end of the channel struct array */
    iir_resamplerv2->working = (unsigned*) (iir_resamplerv2->channel + num_channels);

    /* shift input to Q9.xx for processing head-room */
    iir_resamplerv2->common.input_scale  = -8;

    /* shift output back to Q1.xx */
    iir_resamplerv2->common.output_scale =  8;

    /* set internal pointer to shared memory iir_resamplerv2 data structure */
    iir_resamplerv2_set_config(&iir_resamplerv2->common,lpconfig);

    /* initialize iir_resamplerv2 channel data */
    iir_resampler_reset_internal(op_extra_data);

    return TRUE;
}

void free_iir_resampler_internal(IIR_RESAMPLER_OP_DATA* op_extra_data)
{
    patch_fn_shared(iir_resampler);

    /* release iir_resamplerv2 shared filter memory */
    if(op_extra_data->lpconfig)
    {
        iir_resamplerv2_release_config(op_extra_data->lpconfig);
        op_extra_data->lpconfig = NULL;
    }

    /* free iir_resamplerv2 internal data */
    if(op_extra_data->iir_resamplerv2)
    {
        pfree(op_extra_data->iir_resamplerv2);
        op_extra_data->iir_resamplerv2 = NULL;
    }

    /* release any scratch memory that the task reserved */
    if(op_extra_data->scratch_reserved)
    {
        scratch_release(op_extra_data->scratch_reserved, MALLOC_PREFERENCE_NONE);
        op_extra_data->scratch_reserved = 0;
    }
}

bool iir_resampler_stop_reset(OPERATOR_DATA *op_data,void **response_data)
{
    if(!base_op_build_std_response(STATUS_OK,op_data->id,response_data))
    {
        return(FALSE);
    }
    /* do nothing if iir_resampler is not running */
    if(op_data->state == OP_RUNNING)
    {
        free_iir_resampler_internal(op_data->extra_op_data);
#ifdef INSTALL_METADATA
        /* Resampler has a delay, so remove that delay from the metadata as it
         * is configured at start. It was set at start based on conversion rate
         * so to maintain symmetry remove it here. */
        {
            IIR_RESAMPLER_OP_DATA *op_extra_data = (IIR_RESAMPLER_OP_DATA*)op_data->extra_op_data;
            buff_metadata_remove_delay(op_extra_data->metadata_op_buffer, op_extra_data->metadata_delay);
            op_extra_data->last_tag.type = UNSUPPORTED_TAG;
        }
#endif /* INSTALL_METADATA */
    }

    /* set state to not running */
    op_data->state = OP_NOT_RUNNING;
    return TRUE;
}


/* iir resampler obpm support */
bool iir_resampler_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    /* In the case of this capability, nothing is done for control message. Just follow protocol and ignore any content. */
    return cps_control_setup(message_data, resp_length, resp_data,NULL);
}
bool iir_resampler_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    return FALSE;
}
bool iir_resampler_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    /* Set the parameter(s). For future proofing, it is using the whole mechanism, although currently there is only one field
     * in opdata structure that is a setable parameter. If later there will be more (ever), must follow contiquously the first field,
     * as commented and instructed in the op data definition. Otherwise consider moving them into a dedicated structure.
     */

    return FALSE;
}
bool iir_resampler_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
     return FALSE;
}
bool iir_resampler_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    IIR_RESAMPLER_OP_DATA * op_extra_data =  (IIR_RESAMPLER_OP_DATA*)op_data->extra_op_data;

    unsigned* resp = NULL;

    if(!common_obpm_status_helper(message_data,resp_length,resp_data,sizeof(IIR_RESAMPLER_STATISTICS) ,&resp))
    {
         return FALSE;
    }

    if(resp)
    {
        resp = cpsPackWords(&op_extra_data->in_rate, &op_extra_data->out_rate, resp);
    }

    return TRUE;
}
