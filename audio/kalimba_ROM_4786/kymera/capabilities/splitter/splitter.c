/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  splitter.c
 * \ingroup  capabilities
 *
 *  Splitter capability.
 *
 */
/****************************************************************************
Include Files
*/
#include "splitter.h"
#include "buffer.h"
#include "pmalloc/pl_malloc.h"
#include "util.h"
#include "audio_log/audio_log.h"
#include "platform/pl_intrinsics.h"
#include "platform/pl_assert.h"
#include "op_msg_helpers.h"

#include "patch/patch.h"

/* TODO Stalls
 * start - flag error if started already?
 * stop - flag error if not started?
 */

/****************************************************************************
Private Constant Definitions
*/
/** Maximum number of splitter channels */
#define SPLITTER_MAX_CHANS 8
/** The maximum number of streams to split the input channel into */
#define SPLITTER_MAX_OUTPUTS_PER_CHANNEL 2
/** Maximum number of splitter input channels */
#define SPLITTER_MAX_INPUTS SPLITTER_MAX_CHANS
/** Maximum number of splitter channels */
#define SPLITTER_MAX_OUTPUTS (SPLITTER_MAX_OUTPUTS_PER_CHANNEL * SPLITTER_MAX_CHANS)
/** The block size of the splitter reported to streams */
#define SPLITTER_BLOCK_SIZE 1
/** Value indicating an invalid terminal on the splitter */
#define SPLITTER_INVALID_TERMINAL UINT_MAX

#ifdef CAPABILITY_DOWNLOAD_BUILD
#define SPLITTER_CAP_ID CAP_ID_DOWNLOAD_SPLITTER
#else
#define SPLITTER_CAP_ID CAP_ID_SPLITTER
#endif

/****************************************************************************
Private Type Definitions
*/

/*
 * It is important to understand that read pointers within the in[] tCBuffers will lag behind the actual
 * amount of input data read and written to the outputs. The read pointers within the in[] tCBuffers are
 * updated by an amount dictated by splitter output consumption (so all buffers are shared). The input read
 * pointer that should be used in the next process data call will be equal to the output write pointer.
 */
/** The data specific to the splitter capability */
/* Splitter channel descriptor  */
typedef struct splitter_channel_struct
{  
   struct splitter_channel_struct *next_valid;
   tCbuffer *input_buffer_ptr;
   tCbuffer *output_buffer_ptr[SPLITTER_MAX_OUTPUTS_PER_CHANNEL];
   bool output_active[SPLITTER_MAX_OUTPUTS_PER_CHANNEL];
} SPLITTER_CHANNEL_STRUC;

typedef struct
{
    /** Array of splitter channels */
    SPLITTER_CHANNEL_STRUC channels[SPLITTER_MAX_CHANS];
    SPLITTER_CHANNEL_STRUC *first_valid;

#ifdef INSTALL_METADATA
    /** The input buffer with metadata to transport from */
    tCbuffer *metadata_ip_buffer;
    /** The output buffer with metadata to transport to */
    tCbuffer *metadata_op_buffer;
#endif /* INSTALL_METADATA */
    /** The buffer size to request at connect */
    unsigned buffer_size;
    /** The data format to advertise at connect */
    AUDIO_DATA_FORMAT data_format;
    /** The sinks to kick on when kicking backwards */
    unsigned touched_sinks;
    /** The sources to kick on when kicking forwards */
    unsigned touched_sources;
    /** Bitfield indicating which output streams are active */
    unsigned active_streams;
    /** Buffer that was supplied by buffer_details and should be provided by the
     * connect call. This is mainly stored in case connect fails before splitter_connect()
     * is called and we need to release this memory at a later time. */
    tCbuffer *curr_connecting;
} SPLITTER_OP_DATA;

/****************************************************************************
Public Constant Declarations
*/
/** The splitter capability function handler table */
const handler_lookup_struct splitter_handler_table =
{
    splitter_create,           /* OPCMD_CREATE */
    splitter_destroy,          /* OPCMD_DESTROY */
    splitter_start,            /* OPCMD_START */
    base_op_stop,              /* OPCMD_STOP */
    base_op_reset,             /* OPCMD_RESET */
    splitter_connect,          /* OPCMD_CONNECT */
    splitter_disconnect,       /* OPCMD_DISCONNECT */
    splitter_buffer_details,   /* OPCMD_BUFFER_DETAILS */
    splitter_get_data_format,  /* OPCMD_DATA_FORMAT */
    splitter_get_sched_info    /* OPCMD_GET_SCHED_INFO */
};

/** The splitter capability operator message handler table */
const opmsg_handler_lookup_table_entry splitter_opmsg_handler_table[] =
{
    {OPMSG_COMMON_ID_GET_CAPABILITY_VERSION, base_op_opmsg_get_capability_version},
    {OPMSG_COMMON_ID_SET_BUFFER_SIZE, splitter_set_buffer_size},
    {OPMSG_SPLITTER_ID_SET_DATA_FORMAT, splitter_set_data_format},
    {OPMSG_SPLITTER_ID_SET_RUNNING_STREAMS, splitter_set_running_streams},
    {0, NULL}
};

/** splitter capability data */
const CAPABILITY_DATA splitter_cap_data =
{
    SPLITTER_CAP_ID,                                       /* Capability ID */
    0, 3,                                                  /* Version information - hi and lo parts */
    SPLITTER_MAX_INPUTS, SPLITTER_MAX_OUTPUTS,             /* Max number of sinks/inputs and sources/outputs */
    &splitter_handler_table,                               /* Pointer to message handler function table */
    splitter_opmsg_handler_table,                          /* Pointer to operator message handler function table */
    splitter_process_data,                                 /* Pointer to data processing function */
    0,                                                     /* TODO: this would hold processing time information */
    sizeof(SPLITTER_OP_DATA)
};

/****************************************************************************
Private Function Definitions
*/

/**
 * \brief Helper to unpack terminal information.
 *
 * \param packed terminal information from connect/disconnect calls.
 * \param terminal_num address where the terminal.
                       number will be stored. [output]
 * \param buf optional pointer to a CBuffer pointer where
 *            the Cbuffer pointer may be stored. [output]
 *
 * \return if the terminal is an input terminal.
 */
static bool get_terminal_info(const unsigned *packed, unsigned *terminal_num, tCbuffer **buf)
{
    bool is_input;

    *terminal_num = packed[0] & TERMINAL_NUM_MASK;
    is_input = packed[0] & TERMINAL_SINK_MASK;

    if (buf)
    {
        *buf = (tCbuffer *)(uintptr_t)(packed[1]);
    }

    if (is_input)
    {
        if (*terminal_num >= SPLITTER_MAX_INPUTS)
        {
            *terminal_num = SPLITTER_INVALID_TERMINAL;
        }
    }
    else
    {
        if (*terminal_num >= SPLITTER_MAX_OUTPUTS)
        {
            *terminal_num = SPLITTER_INVALID_TERMINAL;
        }
    }

    return is_input;
}

/**
 * \brief Relink all the channels that can stream.
 *
 * \param splitter Pointer to the operator instance data.
 */
static void splitter_relink(SPLITTER_OP_DATA *splitter)
{
    const OPMSG_SPLITTER_RUNNING_STREAMS stream_bit[SPLITTER_MAX_OUTPUTS_PER_CHANNEL] = {OPMSG_SPLITTER_STREAM_0, OPMSG_SPLITTER_STREAM_1};
    unsigned i, j;
    SPLITTER_CHANNEL_STRUC **last_valid = &splitter->first_valid;

    *last_valid = NULL;
    for (i=0; i<SPLITTER_MAX_CHANS; i++)
    {
        SPLITTER_CHANNEL_STRUC *channel = &splitter->channels[i];

        channel->next_valid = NULL;
        if (NULL != channel->input_buffer_ptr)
        {
            bool channel_active = FALSE;
            for (j=0; j<SPLITTER_MAX_OUTPUTS_PER_CHANNEL; j++)
            {
                channel->output_active[j] = FALSE;
                if (NULL != channel->output_buffer_ptr[j] && (splitter->active_streams & stream_bit[j]) == stream_bit[j])
                {
                    channel->output_active[j] = TRUE;
                    channel_active = TRUE;
                }
            }
            if (channel_active)
            {
                *last_valid = channel;
                last_valid = &channel->next_valid;
            }
        }
        else
        {
            for (j=0; j<SPLITTER_MAX_OUTPUTS_PER_CHANNEL; j++)
            {
                channel->output_active[j] = FALSE;
            }
        }
    }
}

/**
 * \brief Given an output terminal number, check if that
 *        terminal or corresponding terminals in other
 *        output streams that follow are connected.
 *
 * \param out array of output CBuffer pointers
 * \param terminal_num terminal number
 * \return boolean indicating the existence of atleast one connection.
 */
static bool splitter_output_enabled(const SPLITTER_CHANNEL_STRUC *channel)
{

    if (channel->output_buffer_ptr[0] || channel->output_buffer_ptr[1])
    {
        return TRUE;
    }
    return FALSE;
}

/****************************************************************************
Public Function Definitions
*/
/**
 * \brief Configures the splitter defaults.
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the create request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool splitter_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    SPLITTER_OP_DATA *splitter;
    if (!base_op_create(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    splitter = (SPLITTER_OP_DATA*)op_data->extra_op_data;
    splitter->buffer_size = 0;
    splitter->data_format = AUDIO_DATA_FORMAT_FIXP;
    splitter->active_streams = OPMSG_SPLITTER_STREAM_0 | OPMSG_SPLITTER_STREAM_1;

    return TRUE;
}

/**
 * \brief Reports the buffer requirements of the splitter terminal
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the buffer size request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool splitter_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    OP_BUF_DETAILS_RSP *buf_details;
    SPLITTER_OP_DATA *splitter = (SPLITTER_OP_DATA *) (op_data->extra_op_data);

    if (!base_op_buffer_details(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* Alias the void* to the actual structure to make it easier to work with */
    buf_details = (OP_BUF_DETAILS_RSP*)*response_data;

#ifdef INSTALL_METADATA
    {
        /* If an input/output connection is already present and has metadata then
         * we are obliged to return that buffer so that metadata can be shared
         * between channels. */
        tCbuffer *meta_buff;
        unsigned terminal_id = ((unsigned *)message_data)[0];
        if ((terminal_id & TERMINAL_SINK_MASK) == TERMINAL_SINK_MASK)
        {
            meta_buff = splitter->metadata_ip_buffer;
        }
        else
        {
            meta_buff = splitter->metadata_op_buffer;
        }

        ((OP_BUF_DETAILS_RSP*)*response_data)->metadata_buffer = meta_buff;
        ((OP_BUF_DETAILS_RSP*)*response_data)->supports_metadata = TRUE;
    }
#endif /* INSTALL_METADATA */

    if (((unsigned *)message_data)[0] & TERMINAL_SINK_MASK)
    {
        if (splitter->buffer_size != 0)
        {
            /* Override the calculated buffer size if one has been set */
            buf_details->b.buffer_size = splitter->buffer_size;
        }
    }
    else
    {
        unsigned terminal_num = ((unsigned *)message_data)[0] & TERMINAL_NUM_MASK;
        SPLITTER_CHANNEL_STRUC *channel = &splitter->channels[terminal_num >> 1];
        unsigned output_index = terminal_num & 1;
        if (channel->input_buffer_ptr == NULL)
        {
            /* Input needs to be connected prior to output */
            base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            return TRUE;
        }
        else
        {
            tCbuffer *buffer_to_wrap = channel->input_buffer_ptr;
            buf_details->supplies_buffer = TRUE;

            if (channel->output_buffer_ptr[output_index] != NULL)
            {
                /* If this terminal is already connected then make sure the buffer that
                 * is in use is returned to the caller. */
                buf_details->b.buffer = channel->output_buffer_ptr[output_index];
            }
            else
            {
                /* If buffer details is requested during connect for the same channel
                 * return the buffer being used for connect */
                if (splitter->curr_connecting == NULL || splitter->curr_connecting->base_addr != buffer_to_wrap->base_addr)
                {
                    /* A buffer orphaned in a previously aborted connect might exist, free it
                     * before we create a new buffer that might get orphaned that we want to track. */
                    cbuffer_destroy_struct(splitter->curr_connecting);

                    /* Store the buffer created here in case it gets orphaned by connect failing */
                    splitter->curr_connecting = cbuffer_create(buffer_to_wrap->base_addr,
                        cbuffer_get_size_in_words(buffer_to_wrap), BUF_DESC_SW_BUFFER);

                    if (splitter->curr_connecting != NULL)
                    {
                        splitter->curr_connecting->write_ptr = buffer_to_wrap->write_ptr;
                        splitter->curr_connecting->read_ptr = buffer_to_wrap->write_ptr;
                    }
                    else
                    {
                        /* There wasn't enough RAM so we have to fail */
                        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
                        return TRUE;
                    }
                }
                buf_details->b.buffer = splitter->curr_connecting;
            }
        }
    }

    return TRUE;
}

/**
 * \brief Returns the splitter terminal audio data format.
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the get_block_size request message payload
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool splitter_get_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    *response_id = OPCMD_DATA_FORMAT;

    /* Set up the a default success response information */
    if (base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        ((OP_STD_RSP*)*response_data)->resp_data.data =
                ((SPLITTER_OP_DATA *) (op_data->extra_op_data))->data_format;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/**
 * \brief Returns the splitter block size
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the get_block_size request message payload
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool splitter_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    OP_SCHED_INFO_RSP* resp;

    if (!base_op_get_sched_info(op_data, message_data, response_id, response_data))
    {
        return base_op_build_std_response(STATUS_CMD_FAILED, op_data->id, response_data);
    }
    /* Populate the response*/
    base_op_change_response_status(response_data, STATUS_OK);
    resp = *response_data;
    resp->op_id = op_data->id;
    /* Same buffer size for sink and source.
       No additional verification needed.*/
    resp->block_size = SPLITTER_BLOCK_SIZE;
    resp->run_period = 0;
    *response_data = resp;
    return TRUE;
}

/**
 * \brief Updates all buffer pointers so that outputs can read any new data and
 * inputs can see any new space created from down stream reads.
 *
 * \param op_data Pointer to the operator instance data.
 */
void splitter_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    unsigned i, min_new_data, min_new_space;
    int *new_output_write_addr;
    int *new_input_read_addr;
    SPLITTER_OP_DATA *splitter = (SPLITTER_OP_DATA *)(op_data->extra_op_data);
    SPLITTER_CHANNEL_STRUC *channel = splitter->first_valid;

    
    patch_fn(splitter_process_data);

    /* This code is very naughty and reaches into the cbuffer structures. It can
     * be done safely because they have to be local, and it's lightning fast as
     * a result. Cbuffer API is subverted because it isn't designed for this.
     */
    min_new_data = min_new_space = UINT_MAX;
    /* Iterate through the list of all active channels. */
    while (NULL != channel)
    {
        int new_data, new_space;
        tCbuffer *out = NULL; /* Initialise to keep the compiler happy. */
        tCbuffer *in = channel->input_buffer_ptr;

        for (i=0; i<SPLITTER_MAX_OUTPUTS_PER_CHANNEL; i++)
        {
            if (channel->output_active[i])
            {
                out = channel->output_buffer_ptr[i];

                /* Find out minimum available space. */
                new_space = (char *)out->read_ptr - (char *)in->read_ptr;
                if (new_space < 0)
                {
                    new_space += in->size;
                }

                if (new_space < min_new_space)
                {
                    min_new_space = new_space;
                }
            }
        }

        /* Find out minimum available data. */
        new_data =  (char *)in->write_ptr -  (char *)out->write_ptr;
        if (new_data < 0)
        {
            new_data += out->size;
        }

        if (new_data < min_new_data)
        {
            min_new_data = new_data;
        }

        channel = channel->next_valid;
    }

    if (min_new_data == 0 && min_new_space == 0)
    {
        return;
    }

    /* Typically only one of  min_new_space OR min_new_data are non zero on a
     * given kick so we separate the looping out to reduce the amount of work done.
     */
    if (min_new_space > 0)
    {
        channel = splitter->first_valid;
        while (NULL != channel)
        {
            tCbuffer *in = channel->input_buffer_ptr;
            new_input_read_addr = (int *)((char *)in->read_ptr + min_new_space);
            if (new_input_read_addr >= (int *)((char *)in->base_addr + in->size))
            {
                new_input_read_addr = (int *)((char *)new_input_read_addr - in->size);
            }
            in->read_ptr = new_input_read_addr;
            channel = channel->next_valid;
        }
        /* N.B. Because the splitter runs in place and is designed to be
         * cascaded, it is necessary to kick back whenever data is consumed so
         * that the input buffer pointers of a splitter that proceeds this one
         * are updated. */
        touched->sinks = splitter->touched_sinks;
    }

    if (min_new_data > 0)
    {
#ifdef INSTALL_METADATA
        unsigned octets_moved;

        if (buff_has_metadata(splitter->metadata_ip_buffer))
        {
            /* Transport all the available metadata. The available tags will be limited by
             * the cbuffer available data. This will eliminate difference between
             * octet and _ex buffers. */
            octets_moved =  buff_metadata_available_octets(splitter->metadata_ip_buffer);
        }
        else
        {
            /* There is no metadata for the input buffer, but still update the output
             * buffer metadata write index. Due to the lack of the metadata (so no info on
             * the usable octets) min_new_data is a fair approximation.  */
            octets_moved = min_new_data;
        }

        /* Transport any metadata to the output. */
        metadata_strict_transport(splitter->metadata_ip_buffer,
                                    splitter->metadata_op_buffer,
                                    octets_moved);
#endif /* INSTALL_METADATA */
        channel = splitter->first_valid;
        while (NULL != channel)
        {
            for (i=0; i<SPLITTER_MAX_OUTPUTS_PER_CHANNEL; i++)
            {
                if (channel->output_active[i])
                {
                    tCbuffer *out = channel->output_buffer_ptr[i];
                    new_output_write_addr = (int *)((char *)out->write_ptr + min_new_data);
                    if (new_output_write_addr >= (int *)((char *)out->base_addr + out->size))
                    {
                        new_output_write_addr = (int *)((char *)new_output_write_addr - out->size);
                    }
                    out->write_ptr = new_output_write_addr;
                }
#ifdef INSTALL_METADATA
                else
                {
                    /* If the output is disabled and connected metadata is created
                     * for the output and not consumed by anyone. Delete those metadata.
                     * NOTE: It would be better not to create it in the first place, but
                     * that involves the splitter to take care of the metadata transport. */
                    tCbuffer *out = channel->output_buffer_ptr[i];
                    if (out)
                    {
                        metadata_tag *ret_mtag;
                        unsigned b4idx, afteridx;
                        ret_mtag = buff_metadata_remove(out, octets_moved, &b4idx, &afteridx);
                        buff_metadata_tag_list_delete(ret_mtag);
                    }
                }
#endif /* INSTALL_METADATA */
            }       
            channel = channel->next_valid;
        }
        touched->sources = splitter->touched_sources;
    }

}

/**
 * \brief Connects a splitter terminal to a buffer. This capability requires
 * that a channel's input terminal is connected before its output terminals.
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the connect request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool splitter_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    unsigned terminal_num;
    bool is_input;
    tCbuffer *buf;
    SPLITTER_OP_DATA *splitter;

    if (!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }

    is_input = get_terminal_info(message_data, &terminal_num, &buf);

    if (terminal_num == SPLITTER_INVALID_TERMINAL)
    {
        base_op_change_response_status(response_data, STATUS_INVALID_CMD_PARAMS);
        return TRUE;
    }

    splitter = (SPLITTER_OP_DATA *) (op_data->extra_op_data);

    if (is_input)
    {
        tCbuffer **in = &splitter->channels[terminal_num].input_buffer_ptr;
        if (NULL == *in)
        {
            /* If the output is already connected this will cause absolute carnage. */
            if (op_data->state == OP_RUNNING || splitter_output_enabled(&splitter->channels[terminal_num]))
            {
                base_op_change_response_status(response_data, STATUS_CMD_FAILED);
                return TRUE;
            }
            *in = buf;
#ifdef INSTALL_METADATA
            if(splitter->metadata_ip_buffer == NULL)
            {
                if (buff_has_metadata(*in))
                {
                    splitter->metadata_ip_buffer = *in;
                }
            }
#endif /* INSTALL_METADATA */
            splitter->touched_sinks |= TOUCHED_SINK_0 << terminal_num;
            return TRUE;
        }
    }
    else
    {
        tCbuffer **out = &splitter->channels[terminal_num >> 1].output_buffer_ptr[terminal_num & 1];
        if (NULL == *out)
        {
            /* This should be the curr_connecting buffer that buffer_details
             * stashed. If it isn't fail the request as something went wrong,
             * otherwise it's not in danger of being orphaned any more. */
            PL_ASSERT(buf == splitter->curr_connecting);
            splitter->curr_connecting = NULL;

            *out = buf;
#ifdef INSTALL_METADATA
            if (buff_has_metadata(*out))
            {
                if(splitter->metadata_op_buffer == NULL)
                {
                    splitter->metadata_op_buffer = *out;
                }
                buff_metadata_set_usable_octets(*out,
                        buff_metadata_get_usable_octets(splitter->channels[terminal_num >> 1].input_buffer_ptr));
            }
#endif /* INSTALL_METADATA */
            splitter_relink(splitter);
            splitter->touched_sources |= TOUCHED_SOURCE_0 << terminal_num;
            return TRUE;
        }
    }
    panic_diatribe(PANIC_AUDIO_ALREADY_CONNECTED_OPERATOR_TERMINAL, op_data->id);
}

/**
 * \brief Starts the splitter capability so that data will flow through this
 * capablity.
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the start request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool splitter_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    SPLITTER_OP_DATA *splitter = (SPLITTER_OP_DATA *) (op_data->extra_op_data);

    if (!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }

    *response_id = OPCMD_START;

    if (OP_RUNNING == op_data->state)
    {
        return TRUE;
    }

    if (splitter->first_valid != NULL)
    {
        /* At least one input and it's corresponding output(s) are connected */
        op_data->state = OP_RUNNING;
        return TRUE;
    }

    base_op_change_response_status(response_data, STATUS_CMD_FAILED);
    return TRUE;
}

/**
 * \brief Disconnects the splitter terminal from a buffer.
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the disconnect request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool splitter_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    unsigned i, terminal_num;
    bool is_input;
    SPLITTER_OP_DATA *splitter = (SPLITTER_OP_DATA *) (op_data->extra_op_data);

    if (!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }

    is_input = get_terminal_info(message_data, &terminal_num, NULL);

    if (terminal_num == SPLITTER_INVALID_TERMINAL)
    {
        base_op_change_response_status(response_data, STATUS_INVALID_CMD_PARAMS);
        return TRUE;
    }

    if (is_input)
    {
        tCbuffer *in = splitter->channels[terminal_num].input_buffer_ptr;
        if (in)
        {
            if (op_data->state != OP_RUNNING)
            {
#ifdef INSTALL_METADATA
                if (splitter->metadata_ip_buffer == in)
                {
                    bool found_alternative = FALSE;
                    for (i = 0; i < SPLITTER_MAX_CHANS; i++)
                    {
                        tCbuffer *alternative = splitter->channels[i].input_buffer_ptr;
                        if (i == terminal_num)
                        {
                            continue;
                        }
                        if (alternative != NULL && buff_has_metadata(alternative))
                        {
                            splitter->metadata_ip_buffer = alternative;
                            found_alternative = TRUE;
                            break;
                        }
                    }
                    if (!found_alternative)
                    {
                        splitter->metadata_ip_buffer = NULL;
                    }
                }
#endif /* INSTALL_METADATA */
                splitter->channels[terminal_num].input_buffer_ptr = NULL;
                splitter->touched_sinks &= ~(TOUCHED_SINK_0 << terminal_num);

                /* If there are outputs connected we need to make them look empty.
                 * This shouldn't be done by the user, but we can't reject the
                 * scenario when we're stopped as destroy operator needs to
                 * disconnect all terminals before the operator are destroyed. */
                for (i = 0; i < SPLITTER_MAX_OUTPUTS_PER_CHANNEL; i++)
                {
                    tCbuffer *out = splitter->channels[terminal_num].output_buffer_ptr[i];
                    if (out != NULL)
                    {
                        LOCK_INTERRUPTS;
                        out->write_ptr = out->read_ptr;
                        UNLOCK_INTERRUPTS;
                    }
                }
                return TRUE;
            }
            base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            return TRUE;
        }
    }
    else
    {
        SPLITTER_CHANNEL_STRUC *channel = &splitter->channels[terminal_num >> 1];
        if (channel->output_buffer_ptr[terminal_num & 1])
        {
            tCbuffer *buff;
            /* If the splitter is running then you can't disconnect both output
             * streams of a channel. */
            if (op_data->state == OP_RUNNING)
            {
                unsigned other_out_terminal = (terminal_num & 1) ^ 1;
                if (NULL == channel->output_buffer_ptr[other_out_terminal])
                {
                    base_op_change_response_status(response_data, STATUS_CMD_FAILED);
                    return TRUE;
                }
            }
#ifdef INSTALL_METADATA
            if (splitter->metadata_op_buffer == channel->output_buffer_ptr[terminal_num & 1])
            {
                bool found_alternative = FALSE;
                unsigned j;

                for (i = 0; i < SPLITTER_MAX_CHANS; i++)
                {
                    for (j = 0; j < SPLITTER_MAX_OUTPUTS_PER_CHANNEL; j++)
                    {
                        if ((i == (terminal_num >> 1)) && (j == (terminal_num & 1)))
                        {
                            continue;
                        }
                        if (splitter->channels[i].output_buffer_ptr[j] != NULL && buff_has_metadata(splitter->channels[i].output_buffer_ptr[j]))
                        {
                            splitter->metadata_op_buffer = splitter->channels[i].output_buffer_ptr[j];
                            found_alternative = TRUE;
                            break;
                        }
                    }
                }                   
                if (!found_alternative)
                {
                    splitter->metadata_op_buffer = NULL;
                }
            }
#endif /* INSTALL_METADATA */
            /* The buffer was supplied by the capability so we need to release the structure */
            buff = channel->output_buffer_ptr[terminal_num & 1];
            splitter->touched_sources &= ~(TOUCHED_SOURCE_0 << terminal_num);
            channel->output_buffer_ptr[terminal_num & 1] = NULL;
            cbuffer_destroy_struct(buff);
            splitter_relink(splitter);
            
            return TRUE;
        }
    }
    panic_diatribe(PANIC_AUDIO_ALREADY_DISCONNECTED_OPERATOR_TERMINAL, op_data->id);
}

/**
 * \brief Clears up the splitter operator if it's in the correct state.
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the destroy request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool splitter_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    /* Clear up the capability specific data. */
    /* A buffer may have been orphaned during a failed connect make sure it
     * gets freed. */
    tCbuffer *con_buff = ((SPLITTER_OP_DATA *)op_data->extra_op_data)->curr_connecting;
    cbuffer_destroy_struct(con_buff);

    return base_op_destroy(op_data, message_data, response_id, response_data);
}

/**
 * \brief Handles the opmsg that changes the buffer size requested by the splitter
 * at connect.
 * 
 * \param op_data Pointer to the operator instance data
 * \param message_data Pointer to the destroy request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 * 
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool splitter_set_buffer_size(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SPLITTER_OP_DATA *splitter = (SPLITTER_OP_DATA *)op_data->extra_op_data;

    splitter->buffer_size = OPMSG_FIELD_GET(message_data, OPMSG_COMMON_SET_BUFFER_SIZE, BUFFER_SIZE);

    return TRUE;
}

/**
 * \brief Handles the opmsg that changes the data format advertised by the splitter
 * at connect.
 *
 * \param op_data Pointer to the operator instance data
 * \param message_data Pointer to the destroy request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool splitter_set_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SPLITTER_OP_DATA *splitter = (SPLITTER_OP_DATA *)op_data->extra_op_data;

    splitter->data_format = OPMSG_FIELD_GET(message_data, OPMSG_SPLITTER_SET_DATA_FORMAT, DATA_TYPE);

    return TRUE;
}

/**
 * \brief Handles the opmsg that configures which output streams are to be
 * considered as running/active
 *
 * \param op_data Pointer to the operator instance data
 * \param message_data Pointer to the set_running_streams request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool splitter_set_running_streams(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SPLITTER_OP_DATA *splitter;
    unsigned i, cur_channel, new_channel;
    unsigned bitfield = OPMSG_FIELD_GET(message_data, OPMSG_SPLITTER_SET_RUNNING_STREAMS, RUNNING_STREAMS);

    /* Check that the value sent was within range. Only 2 streams are supported
     * and you can't disable both! */
    if (bitfield == 0 || bitfield > (OPMSG_SPLITTER_STREAM_0 | OPMSG_SPLITTER_STREAM_1))
    {
        return FALSE;
    }

    splitter = (SPLITTER_OP_DATA *)op_data->extra_op_data;
    if (splitter->active_streams == bitfield)
    {
        /* No change so nothing to do */
        return TRUE;
    }

    /* If the operator isn't running just set the field. If it is running then
     * the transition needs to happen gracefully */
    else if (op_data->state == OP_NOT_RUNNING)
    {
        splitter->active_streams = bitfield;
        return TRUE;
    }

    /* Before setting the stream active make all the channels buffer pointers
     * look empty like it's just been connected, BUT aligned with the write 
     * pointer of the other stream so that there is no chance of stalls. */
    if ((splitter->active_streams ^ bitfield) == OPMSG_SPLITTER_STREAM_1)
    {
        new_channel = 1;
        cur_channel = 0;
    }
    else
    {
        new_channel = 0;
        cur_channel = 1;
    }

    /* Block interrupts whilst we align the read and write ptrs for the stream that
     * is coming online to the current write pointer of the running stream.
     */
    LOCK_INTERRUPTS;
    for (i = 0; i < SPLITTER_MAX_CHANS; i++)
    {
        SPLITTER_CHANNEL_STRUC *channel = &splitter->channels[i];
        if (channel->output_buffer_ptr[new_channel] != NULL)
        {
            int *new_addr = channel->output_buffer_ptr[cur_channel]->write_ptr;
            channel->output_buffer_ptr[new_channel]->read_ptr = new_addr;
            channel->output_buffer_ptr[new_channel]->write_ptr = new_addr;
#ifdef INSTALL_METADATA
            /* Set both metadata read/write indices on the new stream 
             * to match the write index for the running stream 
             * This ensures that new tags have the same indices on both streams
             */
            channel->output_buffer_ptr[new_channel]->metadata->prev_wr_index = channel->output_buffer_ptr[cur_channel]->metadata->prev_wr_index;
            channel->output_buffer_ptr[new_channel]->metadata->prev_rd_index = channel->output_buffer_ptr[cur_channel]->metadata->prev_wr_index;
#endif /* INSTALL_METADATA */
        }
    }
    UNLOCK_INTERRUPTS;
    splitter->active_streams = bitfield;
    splitter_relink(splitter);

    return TRUE;
}


