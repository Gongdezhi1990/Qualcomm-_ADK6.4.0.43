/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  rtp_decode.c
 * \ingroup  operators
 *
 *  RTP decode operator
 *
 */
#ifndef INSTALL_METADATA
#error RTP decode cannot work without metadata
#endif
/****************************************************************************
Include Files
*/
#include "rtp_decode_private.h"

#include "patch/patch.h"

/****************************************************************************
Private Constant Definitions
*/

/****************************************************************************
Private Type Definitions
*/
#ifdef CAPABILITY_DOWNLOAD_BUILD
#define RTP_DECODE_CAP_ID CAP_ID_DOWNLOAD_RTP_DECODE
#else
#define RTP_DECODE_CAP_ID CAP_ID_RTP_DECODE
#endif

/*****************************************************************************
Private Constant Declarations
*/
/** The RTP decode capability function handler table */
const handler_lookup_struct rtp_decode_handler_table =
{
    rtp_decode_create,          /* OPCMD_CREATE */
    rtp_decode_destroy,         /* OPCMD_DESTROY */
    rtp_decode_start,           /* OPCMD_START */
    base_op_stop,               /* OPCMD_STOP */
    rtp_decode_reset,           /* OPCMD_RESET */
    rtp_decode_connect,         /* OPCMD_CONNECT */
    rtp_decode_disconnect,      /* OPCMD_DISCONNECT */
    rtp_decode_buffer_details,  /* OPCMD_BUFFER_DETAILS */
    rtp_decode_get_data_format, /* OPCMD_DATA_FORMAT */
    rtp_decode_get_sched_info   /* OPCMD_GET_SCHED_INFO */
};

/** Null terminated operator message handler table for rtp decode capability */
const opmsg_handler_lookup_table_entry rtp_decode_opmsg_handler_table[] =
    {{OPMSG_COMMON_ID_GET_CAPABILITY_VERSION,    base_op_opmsg_get_capability_version},
    {OPMSG_RTP_DECODE_ID_SET_WORKING_MODE,       rtp_decode_opmsg_set_working_mode},
    {OPMSG_RTP_DECODE_ID_SET_RTP_CODEC_TYPE,     rtp_decode_opmsg_set_codec_type},
    {OPMSG_RTP_DECODE_ID_SET_RTP_CONTENT_PROTECTION, rtp_decode_opmsg_set_content_protection},
    {OPMSG_RTP_DECODE_ID_SET_AAC_DECODER,        rtp_decode_opmsg_set_AAC_decoder},
    {OPMSG_RTP_DECODE_ID_SET_AAC_UTILITY,        rtp_decode_opmsg_set_AAC_utility},
    {OPMSG_RTP_DECODE_ID_SET_MAX_PACKET_LENGTH,  rtp_decode_opmsg_set_max_packet_length},
    {OPMSG_SET_CTRL,                             rtp_decode_opmsg_set_play_pointer},
    {OPMSG_COMMON_SET_TTP_LATENCY,               rtp_decode_opmsg_set_ttp_latency},
    {OPMSG_COMMON_SET_LATENCY_LIMITS,            rtp_decode_opmsg_set_latency_limits},
    {OPMSG_COMMON_SET_TTP_PARAMS,                rtp_decode_opmsg_set_ttp_params},
    {OPMSG_COMMON_SET_SAMPLE_RATE,               rtp_decode_opmsg_set_sample_rate},
    {OPMSG_COMMON_ID_SET_BUFFER_SIZE,            rtp_decode_opmsg_set_buffer_size},
    {0, NULL}};

/** Constant capability description of rtp_decode capability */
const CAPABILITY_DATA rtp_decode_cap_data =
{
    RTP_DECODE_CAP_ID,        /* Capability ID */
    1, 0,                           /* Version information - hi and lo parts */
    1, 1,                           /* Max number of sinks/inputs and sources/outputs */
    &rtp_decode_handler_table,      /* Pointer to message handler function table */
    rtp_decode_opmsg_handler_table, /* Pointer to operator message handler function table */
    rtp_decode_process_data,        /* Pointer to data processing function */
    0,                              /* TODO - Processing time information */
    sizeof(RTP_DECODE_OP_DATA)      /* Size of capability-specific per-instance data */
};


/****************************************************************************
Private Function Declarations
*/
/* ******************************* Helper functions ************************************ */

/**
 * Sets up the decode specific variables for the operator. The operator codec type
 * should be set primarily to the call of this function. The header size (and
 * potentially other parameters in the future) will be set according to the codec type.
 *
 * \param opx_data Pointer to the RTP operator data.
 */
static void rtp_set_payload_header_size(RTP_DECODE_OP_DATA *opx_data)
{
    switch (opx_data->codec_type)
    {
        case SBC:
        {
            opx_data->payload_header_size = SBC_PAYLOAD_HEADER_SIZE;
            break;
        }

        case ATRAC:
        {
            opx_data->payload_header_size = ATRAC_PAYLOAD_HEADER_SIZE;
            break;
        }
        case MP3:
        {
            opx_data->payload_header_size = MP3_PAYLOAD_HEADER_SIZE;
            break;
        }
        case APTX:
        {
            opx_data->payload_header_size = APTX_PAYLOAD_HEADER_SIZE;
            break;
        }
        case APTXHD:
        {
            opx_data->payload_header_size = APTXHD_PAYLOAD_HEADER_SIZE;
            break;
        }
        case AAC:
        {
            opx_data->payload_header_size = AAC_PAYLOAD_HEADER_SIZE;
            break;
        }
        default:
        {
            /*mode not supported yet*/
            panic(PANIC_AUDIO_RTP_UNSUPPORTED_CODEC);
            break;
        }
    }

}

/**
 * Initialise various working data parameters for the RTP operator.
 *
 * \param opx_data Pointer to the RTP operator data.
 */
static void rtp_decode_reset_working_data(OPERATOR_DATA *op_data)
{
    RTP_DECODE_OP_DATA* opx_data = (RTP_DECODE_OP_DATA*)(op_data->extra_op_data);

    opx_data->ip_format = AUDIO_DATA_FORMAT_16_BIT;
    opx_data->op_format = AUDIO_DATA_FORMAT_16_BIT;

    /* Set the default working mode as RTP header stripping. */
    opx_data->mode = RTP_STRIP;

    /* Disable content protection by default. */
    opx_data->scms_enable = 0;

    /* For the moment the default code is SBC. */
    opx_data->codec_type = SBC;

    rtp_set_payload_header_size(opx_data);
#ifdef DEBUG_RTP_DECODE
    opx_data->rtp_decode_last_tag_index = -1;
    opx_data->rtp_decode_last_tag_length = -1;
#endif
}

/**
 * Check if the buffer is in sync with the metadata tag.
 *
 * \param tag Pointer to the metadata tag read from the input buffer.
 * \param opx_data Pointer to the RTP operator data.
 */
static bool tag_valid(metadata_tag* tag, RTP_DECODE_OP_DATA *opx_data )
{

    if (obuffer_get_read_index(opx_data->ip_buffer) != tag->index )
    {
        /* We missed something from the previous packet so the tag didn't start from
         * where it should.*/
        unsigned discard_buffer;
        unsigned discard_metadata;
        unsigned octets_2_first_tag, octets_post_last_tag;
        metadata_tag  *tag_list;
        /* Probably the metadata and the buffer will have different amount of data. */
        discard_buffer = obuffer_calc_amount_data_in_octets(opx_data->ip_buffer);
        obuffer_discard_data(opx_data->ip_buffer, discard_buffer);

        discard_metadata = buff_metadata_available_octets(opx_data->ip_buffer->buffer);
        tag_list = buff_metadata_remove(opx_data->ip_buffer->buffer, discard_metadata, &octets_2_first_tag, &octets_post_last_tag);
        buff_metadata_tag_list_delete(tag_list);

        L2_DBG_MSG2("RTP Decode buffer out of sink. discard_buffer = %d, discard_metadata =%d",
                discard_buffer, discard_metadata);
        return FALSE;
    }
#ifdef DEBUG_RTP_DECODE
    opx_data->rtp_decode_last_tag_index = tag->index;
    opx_data->rtp_decode_last_tag_length = tag->length;
#endif
    return TRUE;
}

/**
 * Returns the tag size, 0 if the tag is NULL.
 *
 * \param tag Pointer to the metadata tag read from the input buffer.
 */
static unsigned int get_tag_size(metadata_tag* tag)
{
    if (tag == NULL)
    {
        return 0;
    }
    return tag->length;
}


/**
 * Deletes the consumed metadata tag.
 *
 * \param o_buff Pointer to the octet buffer from which the tag will be consumed.
 * \param octets_consumed Octets consumed from the buffer. Must be only one tag.
 */
static void delete_consumed_metadata_tag(octet_buffer *o_buff, unsigned int  octets_consumed)
{

    unsigned int octets_b4idx, octets_afteridx;

    metadata_tag *tag = buff_metadata_remove(o_buff->buffer, octets_consumed, &octets_b4idx , &octets_afteridx);

    /* get_next_tag_size checks if the tag is at the beginning of the buffer. Therefore
     * octets_b4idx will be 0.*/
    if ((tag == NULL)||
        (octets_b4idx != 0) ||
        (octets_afteridx != octets_consumed))
    {
        /* Metadata went out of sync. */
        panic( PANIC_AUDIO_DEBUG_ASSERT_FAILED);
    }
    buff_metadata_delete_tag(tag, TRUE);
}

/**
 * Returns the time of arrival of the tag. If the time of arrival is not set for the tag
 * the hal get time is used.
 */
static unsigned get_time_of_arrival( metadata_tag* tag)
{
    if (IS_TIME_OF_ARRIVAL_TAG(tag))
    {
        return tag->timestamp;
    }
    else
    {
        /* Maybe we are connected to a tester/file endpoint.*/
        return hal_get_time();
    }
}

/**
 * Transports and timestamps the metadata tags from the input buffer to the output buffer.
 * frame_data will provide input on how many decoded frames we have. Note: for sbc, aptx
 * and aptxHD we will always have at least one frame/ rtp packet. This means that the
 * tag used for setting the packet boundaries and giving a time of arrival is reused
 * for timestamping the output frame. If there is more frame in the RTP packet, which is
 * usually true SBC, additional tags will be created. For AAC where multiple, usually
 * two RTP packets hold one AAC frame, one or more time of arrival tags are deleted from
 * the input. (Important to mention that even for AAC and RTP decode packet cannot contain
 * two separate encoded frame segment)
 *
 * \param opx_data Pointer to RTP decode extra op data.
 * \param frame_data Pointer frame header decode data.
 * \param packet_size Size of the tag from the input buffer.
 */
static void transport_metadata_tag(RTP_DECODE_OP_DATA *opx_data, RTP_FRAME_DECODE_DATA* frame_data,
        unsigned packet_size)
{
    tCbuffer* op_buffer = opx_data->op_buffer->buffer;
    tCbuffer* ip_buffer = opx_data->ip_buffer->buffer;
    unsigned octets_2_first_tag, octets_post_last_tag;
    unsigned frame_length = frame_data->frame_length;
    metadata_tag  *tag;
    ttp_status status;

    /* Remove the tag consumed in this cycle.
     */
    tag = buff_metadata_remove(ip_buffer, packet_size, &octets_2_first_tag, &octets_post_last_tag);

    if ((octets_2_first_tag != 0)||(octets_post_last_tag != packet_size) ||
        (tag == NULL) || (tag->next != NULL))
    {
        /* Metadata out if sync. */
        panic( PANIC_AUDIO_DEBUG_ASSERT_FAILED);
    }

    if (opx_data->codec_type == AAC)
    {
        unsigned clone_buf_avail_octets = obuffer_calc_amount_data_in_octets(opx_data->clone_op_buffer);
        /* There shouln't be more than one AAC frames in the clone buffer. */
        if (frame_data->nr_of_frames > 1)
        {
            L2_DBG_MSG1("RTP decode: Number of AAC frames grater than 1! nr frames = %d",
                    frame_data->nr_of_frames);
            frame_data->nr_of_frames = 1;
        }
        /* Mark the frame as invalid if the frame length is different from the available
         * octets in the clone buffer. Note: Only do this if the frame data is valid
         * and if we have at least one frames in the clone buffer. */
        if ((frame_data->valid) &&
            (frame_data->nr_of_frames > 0) &&
            (frame_length != clone_buf_avail_octets) )
        {
            L2_DBG_MSG2("RTP decode: AAC frame length = %d is different from the payload length= %d",
                frame_length, clone_buf_avail_octets);
            frame_data->valid = FALSE;
        }
    }

    if ((frame_data->valid) && (frame_data->nr_of_frames > 0))
    {
        /* Create tags for each frame. The tag read from the input buffer will be
         * reused for the first tag. */
        unsigned count;
        unsigned total_encoded_data;
        metadata_tag *last_tag, *tag_list, *new_tag;
        unsigned time_of_arrival;

        total_encoded_data = frame_length * frame_data->nr_of_frames;

        if (opx_data->continuation_for_frame)
        {
            time_of_arrival = opx_data->first_tag_time_of_arrival;
            opx_data->continuation_for_frame = FALSE;
        }
        else
        {
            time_of_arrival = get_time_of_arrival(tag);
        }

        /* Get the timestamp and sp adjust from the TTP module. Cannot put these two calls
         * to the for loop because the compiler gives status potentially unitialised. */
        ttp_update_ttp(opx_data->ttp_instance, time_of_arrival,
                frame_data->nr_of_frames * frame_data->frame_samples, &status);
        new_tag = tag_list = last_tag = tag;


        METADATA_TIME_OF_ARRIVAL_UNSET(tag);
        /* Populate the new metadata tag. */
        ttp_utils_populate_tag(new_tag, &status);

        /* Make sure only one tag gets the stream start flag */
        status.stream_restart = FALSE;

        new_tag->length = frame_length;

        /* Extrapolate the timestamps to the newly added tags. For one sbc frame only
         * there is no need to do anything. */
        for(count = 1; count < frame_data->nr_of_frames; count++)
        {
            /* The first tag is allocated outside the for loop.*/
            new_tag = buff_metadata_new_tag();
            status.ttp = ttp_get_next_timestamp(status.ttp, frame_data->frame_samples,
                opx_data->sample_rate, status.sp_adjustment);

            /* Populate the new metadata tag. */
            ttp_utils_populate_tag(new_tag, &status);
            METADATA_PACKET_START_SET(new_tag);
            METADATA_PACKET_END_SET(new_tag);
            new_tag->length = frame_length;

            /* Add the new tag to the list. */
            last_tag->next = new_tag;
            last_tag = new_tag;

        }

        /* Append the tag to the output buffer. Note: the after index is only the data
         * after the last index. */
        buff_metadata_append(op_buffer, tag_list, 0, frame_length);
        /* Make the data available in the output buffer. */
        obuffer_advance_write_ptr(opx_data->op_buffer, total_encoded_data);
        /* Remove all the encoded data from the clone buffer. */
        obuffer_discard_data(opx_data->clone_op_buffer, total_encoded_data);
    }
    else if (frame_data->valid)
    {
        opx_data->continuation_for_frame = TRUE;
        opx_data->first_tag_time_of_arrival = get_time_of_arrival(tag);
        /* TODO can any information hold in the tag get lost. My concern is the end of
         * file marker. */
        buff_metadata_delete_tag(tag, TRUE);
    }
    else
    {
        opx_data->continuation_for_frame = FALSE;
        /* A corrupt frame is in the input buffer. Discard it. Note we discard everything
         * accumulated in the clone buffer. */
        opx_data->clone_op_buffer->buffer->write_ptr = opx_data->op_buffer->buffer->write_ptr;
        opx_data->clone_op_buffer->write_offset = opx_data->op_buffer->write_offset;
        buff_metadata_delete_tag(tag, TRUE);

        L2_DBG_MSG("RTP decode corrupt frame discarded! ");
    }

}

/* ********************************** API functions ************************************* */

bool rtp_decode_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    RTP_DECODE_OP_DATA* opx_data;

    /* call base_op create, which also allocates and fills response message */
    if (!base_op_create(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* Make sure the input and output buffer is null. */
    opx_data = (RTP_DECODE_OP_DATA*)(op_data->extra_op_data);
    opx_data->ip_buffer = NULL;
    opx_data->op_buffer = NULL;
    opx_data->clone_op_buffer = NULL;

    /* Reset the first tag logic.*/
    opx_data->continuation_for_frame = FALSE;

    opx_data->latency_buffer_size = RTP_DECODE_DEFAULT_BUFFER_SIZE;
    opx_data->kick_on_full_output = FALSE;

    opx_data->aac_codec = NULL;

    opx_data->aac_utility_select = AAC_LC_OVER_LATM;

    opx_data->max_packet_len = 0;
    opx_data->last_toa_valid = FALSE;
    opx_data->last_op_tag_octets = opx_data->max_packet_len;

    /* Initialise specific data (was allocated and pointer to it filled by OpMgr  */
    rtp_decode_reset_working_data(op_data);

    return TRUE;
}


bool rtp_decode_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    /* check that we are not trying to destroy a running operator */
    if(op_data->state == OP_RUNNING)
    {
        *response_id = OPCMD_DESTROY;

        /* We can't destroy a running operator. */
        return base_op_build_std_response(STATUS_CMD_FAILED, op_data->id, response_data);
    }
    else
    {
        RTP_DECODE_OP_DATA* opx_data = (RTP_DECODE_OP_DATA*)(op_data->extra_op_data);

        /* Delete the octet buffer structure created by this operator. */
        octet_buffer_destroy(opx_data->ip_buffer);
        octet_buffer_destroy(opx_data->op_buffer);
        /* Delete the time to play instance*/
        ttp_free(opx_data->ttp_instance);

        /* call base_op destroy that creates and fills response message, too */
        return base_op_destroy(op_data, message_data, response_id, response_data);
    }
}

bool rtp_decode_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    RTP_DECODE_OP_DATA* opx_data = (RTP_DECODE_OP_DATA*)(op_data->extra_op_data);

    if ((opx_data->mode == RTP_DECODE)  || (opx_data->mode == RTP_TTP_ONLY))
    {
        /* Check if the sample rate is different than zero. */
        if(opx_data->sample_rate == 0)
        {
            *response_id = OPCMD_START;
            return base_op_build_std_response(STATUS_CMD_FAILED, op_data->id, response_data);
        }
    }

    if((opx_data->mode == RTP_DECODE) && (opx_data->codec_type == AAC) && (opx_data->aac_codec == NULL))
    {
        /* It is very hard to decode and AAC frame to get the sample count. Therefore
         * RTP decode needs the help of a decoder which is set through the
         * OPMSG_RTP_DECODE_ID_SET_AAC_DECODER message. */
        *response_id = OPCMD_START;
        return base_op_build_std_response(STATUS_CMD_FAILED, op_data->id, response_data);
    }

    return base_op_start(op_data, message_data, response_id, response_data);
}

bool rtp_decode_reset(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    if (!base_op_reset(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* now initialise specific working data */
    rtp_decode_reset_working_data(op_data);

    return TRUE;
}

bool rtp_decode_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    RTP_DECODE_OP_DATA *opx_data = (RTP_DECODE_OP_DATA *)(op_data->extra_op_data);
    unsigned terminal_id = ((unsigned*)message_data)[0];
    unsigned  channel;

    *response_id = OPCMD_CONNECT;
    if (!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        /* Shouldn't change anything if there is not enough memory for the response.*/
        return FALSE;
    }

    channel = terminal_id & (~TERMINAL_SINK_MASK);
    if (channel != 0)
    {
    	base_op_change_response_status(response_data, STATUS_INVALID_CMD_PARAMS);
        return TRUE;
    }

    if (terminal_id & TERMINAL_SINK_MASK)
    {
        if (opx_data->ip_buffer == NULL)
        {
            /* Wrap up the given buffer to achieve octet based access to it. */
            opx_data->ip_buffer = octet_buffer_create(USE_16BIT_PER_WORD, BIG_ENDIAN_BUFFER);
            if(!opx_data->ip_buffer)
            {
            	base_op_change_response_status(response_data, STATUS_CMD_FAILED);
                return TRUE;
            }
            opx_data->ip_buffer->buffer = (tCbuffer*)(((uintptr_t *)message_data)[1]);
        }
        else
        {
        	base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            return TRUE;
        }
    }
    else
    {
        if (opx_data->op_buffer == NULL)
        {
            tCbuffer *connection_buf = (tCbuffer*)(((uintptr_t *)message_data)[1]);
            /* Wrap up the given buffer to achieve octet based access to it. */
            opx_data->op_buffer = octet_buffer_create(USE_16BIT_PER_WORD, BIG_ENDIAN_BUFFER);
            if(!opx_data->op_buffer)
            {
            	base_op_change_response_status(response_data, STATUS_CMD_FAILED);
                return TRUE;
            }
            opx_data->op_buffer->buffer = connection_buf;
            buff_metadata_set_usable_octets(opx_data->op_buffer->buffer, NR_OF_OCTETS_IN_WORD(USE_16BIT_PER_WORD));

            {
                unsigned buffer_flags;
                tCbuffer *in_place_buffer;

                opx_data->clone_op_buffer = octet_buffer_create(USE_16BIT_PER_WORD, BIG_ENDIAN_BUFFER);
                if(opx_data->clone_op_buffer == NULL)
                {
                	base_op_change_response_status(response_data, STATUS_CMD_FAILED);
                    octet_buffer_destroy(opx_data->op_buffer);
                    return TRUE;
                }

                /* Now, clone the buffer which will be used internally. Make it an in-place
                 * buffer to save place and to avoid overwriting data. Note the output
                 * buffer is not in-place just the internal one. Also the clone buffer
                 * does not need to hold any metadata, it is only used to comfortably play
                 * with the output buffer.  */
                buffer_flags = connection_buf->descriptor;
                BUF_DESC_METADATA_UNSET(buffer_flags);
                BUF_DESC_IN_PLACE_SET(buffer_flags);
                in_place_buffer = cbuffer_create( connection_buf->base_addr,
                            cbuffer_get_size_in_words(connection_buf), buffer_flags);
                if(in_place_buffer == NULL)
                {
                	base_op_change_response_status(response_data, STATUS_CMD_FAILED);
                    octet_buffer_destroy(opx_data->op_buffer);
                    octet_buffer_destroy(opx_data->clone_op_buffer);
                    return TRUE;
                }
                opx_data->clone_op_buffer->buffer = in_place_buffer;
                opx_data->clone_op_buffer->buffer->aux_ptr = (void*)opx_data->op_buffer->buffer;
            }

        }
        else
        {
        	base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            return TRUE;
        }
    }


    return TRUE;
}


bool rtp_decode_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    RTP_DECODE_OP_DATA *opx_data = (RTP_DECODE_OP_DATA *)(op_data->extra_op_data);
    unsigned terminal_id = ((unsigned*)message_data)[0];
    unsigned channel;

    *response_id = OPCMD_DISCONNECT;
    if (!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        /* Shouldn't change anything if there is not enough memory for the response.*/
        return FALSE;
    }

    channel = terminal_id & (~TERMINAL_SINK_MASK);
    if (channel != 0)
    {
    	base_op_change_response_status(response_data, STATUS_INVALID_CMD_PARAMS);
        return TRUE;
    }


    if (terminal_id & TERMINAL_SINK_MASK)
    {
        if (opx_data->ip_buffer)
        {
            /* Delete the octet buffer structure created by this operator. */
            octet_buffer_destroy(opx_data->ip_buffer);
            opx_data->ip_buffer = NULL;
        }
        else
        {
            /* The input buffer is already connected. Fail. */
        	base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        }
    }
    else
    {
        if (opx_data->op_buffer)
        {
            /* Delete the octet buffer structure created by this operator. */
            octet_buffer_destroy(opx_data->op_buffer);
            opx_data->op_buffer = NULL;
            cbuffer_destroy_struct(opx_data->clone_op_buffer->buffer);
            octet_buffer_destroy(opx_data->clone_op_buffer);
            opx_data->clone_op_buffer = NULL;
        }
        else
        {
            /* The output buffer is already connected. Fail. */
        	base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        }
    }

    return TRUE;
}


bool rtp_decode_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    RTP_DECODE_OP_DATA *opx_data = (RTP_DECODE_OP_DATA *)(op_data->extra_op_data);
    unsigned terminal_id = ((unsigned *)message_data)[0];

    if (!base_op_buffer_details(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* RTP does not reuse any of its metadata on other channels. */
    ((OP_BUF_DETAILS_RSP*)*response_data)->metadata_buffer = NULL;

    /* Metadata on the output buffer is only present if the capability decodes the RTP
     * header but the working mode config message could arrive after the connect. */
    ((OP_BUF_DETAILS_RSP*)*response_data)->supports_metadata = TRUE;

    if ((terminal_id & TERMINAL_SINK_MASK) == 0)
    {
        /* Output, use the latency buffer size */
        ((OP_BUF_DETAILS_RSP*)*response_data)->b.buffer_size = opx_data->latency_buffer_size;
    }
    else
    {
        /* Default buffer size for input terminal */
        ((OP_BUF_DETAILS_RSP*)*response_data)->b.buffer_size = RTP_DECODE_DEFAULT_BUFFER_SIZE;
    }

    L4_DBG_MSG1("rtp_decode_buffer_details  %d \n", ((OP_BUF_DETAILS_RSP*)*response_data)->b.buffer_size  );

    return TRUE;
}


bool rtp_decode_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
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
    resp->block_size = RTP_DECODE_DEFAULT_BLOCK_SIZE;
    resp->run_period = 0;

    *response_data = resp;
    return TRUE;
}


bool rtp_decode_get_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    RTP_DECODE_OP_DATA *opx_data;
    if (!base_op_get_data_format(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    opx_data = (RTP_DECODE_OP_DATA *)(op_data->extra_op_data);

    /* return the terminal's data format - purely based on terminal data direction flag */
    if((((unsigned*)message_data)[0] & TERMINAL_SINK_MASK) == 0)
    {
        ((OP_STD_RSP*)*response_data)->resp_data.data = opx_data->op_format;
    }
    else
    {
        ((OP_STD_RSP*)*response_data)->resp_data.data = opx_data->ip_format;
    }

    return TRUE;
}


/* Transport metadata from input to output, reframing output tags
 * to a fixed length to constrain the maximum packet size
 * This is required when using TWS with aptX input
 */
static void metadata_transport_with_ttp_reframe(RTP_DECODE_OP_DATA *opx_data, unsigned octets_copied)
{
    metadata_tag *mtag_op, *mtag_ip, *mtag_ip_list = NULL;
    unsigned b4idx, afteridx, out_tagged_octets, append_length = 0;
    TIME base_toa, base_ttp;
    ttp_status status;

    if (BUFF_METADATA(opx_data->ip_buffer->buffer))
    {
        mtag_ip_list = buff_metadata_remove(opx_data->ip_buffer->buffer, octets_copied, &b4idx, &afteridx);
    }
    else
    {
        b4idx = 0;
    }

    /* It's not worth doing anything if the output doesn't have metadata */
    if (BUFF_METADATA(opx_data->op_buffer->buffer))
    {
        /* Find the first ToA tag */
        mtag_ip = mtag_ip_list;
        while ((mtag_ip != NULL) && (!IS_TIME_OF_ARRIVAL_TAG(mtag_ip)))
        {
            b4idx += mtag_ip->length;
            mtag_ip = mtag_ip->next;
        }

        if ((b4idx == 0) && (mtag_ip != NULL))
        {
            /* If the old tag is already at the start of the input data,
             * Just use its timestamp directly
             */
            base_toa = mtag_ip->timestamp;
            L3_DBG_MSG1("RTP reframe : tag at start, ToA = %d", base_toa);
        }
        else
        {
            /* Otherwise, use the previously-stashed timestamp if there is one */
            if (opx_data->last_toa_valid)
            {
                base_toa = opx_data->last_tag_timestamp;
                L3_DBG_MSG1("RTP reframe : old ToA = %d", base_toa);
            }
            else
            {
                /* Just use the current TIMER_TIME */
                base_toa = hal_get_time();
                L3_DBG_MSG1("RTP reframe : no tag, ToA = %d", base_toa);
            }
        }

        if (mtag_ip != NULL)
        {
            /* Save the timestamp info from the incoming metadata */
            opx_data->last_tag_timestamp = mtag_ip->timestamp;
            opx_data->last_toa_valid = TRUE;
        }

        ttp_update_ttp(opx_data->ttp_instance, base_toa, octets_copied, &status);
        base_ttp = status.ttp;

        /* Distance to the first tag */
        out_tagged_octets = opx_data->max_packet_len - opx_data->last_op_tag_octets;

        if (out_tagged_octets >= octets_copied)
        {
            /* No new tag in this chunk of data */
            buff_metadata_append(opx_data->op_buffer->buffer, NULL, octets_copied, 0);
            opx_data->last_op_tag_octets += octets_copied;
        }
        else
        {
            b4idx = out_tagged_octets;

            /* Make a new list of tags for the output */
            while (out_tagged_octets < octets_copied)
            {
                mtag_op = buff_metadata_new_tag();

                if (mtag_op != NULL)
                {
                    mtag_op->length = opx_data->max_packet_len;
                    METADATA_PACKET_START_SET(mtag_op);
                    METADATA_PACKET_END_SET(mtag_op);
                    status.ttp = ttp_get_next_timestamp(base_ttp, out_tagged_octets, opx_data->sample_rate, status.sp_adjustment);
                    ttp_utils_populate_tag(mtag_op, &status);
                    L3_DBG_MSG5("RTP reframe tag, ToA = %d, base = %d, TTP = %d, len = %d, offset = %d", base_toa, base_ttp, status.ttp, mtag_op->length, out_tagged_octets);
                    /* Make sure only one tag gets the stream start flag */
                    status.stream_restart = FALSE;
                }
                out_tagged_octets += opx_data->max_packet_len;
                if (out_tagged_octets > octets_copied)
                {
                    /* Partial tag, so constrain the append to the copied amount */
                    append_length = opx_data->max_packet_len - (out_tagged_octets - octets_copied);
                }
                else
                {
                    append_length = opx_data->max_packet_len;
                }

                buff_metadata_append(opx_data->op_buffer->buffer, mtag_op, b4idx, append_length);
                b4idx = 0;
            }
            opx_data->last_op_tag_octets = append_length;
        }
    }
    buff_metadata_tag_list_delete(mtag_ip_list);
}


/* Transport metadata from input to output,
 * either adding time-to-play on existing tags or creating new ones if none are present
 * This is only used by the "TTP ONLY" mode with aptX, because it relies on
 * a known fixed compression ratio. This is not used in aptX HD
 */
static void metadata_transport_with_ttp(RTP_DECODE_OP_DATA *opx_data, unsigned octets_copied)
{
    metadata_tag *mtag = NULL;
    unsigned b4idx, afteridx;
    PL_ASSERT(opx_data->codec_type == APTX);

    if (opx_data->max_packet_len != 0)
    {
        metadata_transport_with_ttp_reframe(opx_data, octets_copied);
        return;
    }

    if (BUFF_METADATA(opx_data->ip_buffer->buffer))
    {
        /* transport metadata, first (attempt to) consume tag associated with src */
        mtag = buff_metadata_remove(opx_data->ip_buffer->buffer, octets_copied, &b4idx, &afteridx);
    }
    else
    {
        /* Create a new tag for the output */
        b4idx = 0;
        afteridx = octets_copied;
        mtag = buff_metadata_new_tag();
        if (mtag != NULL)
        {
            mtag->length = octets_copied;
        }
    }

    if (mtag != NULL)
    {
        ttp_status status;
        metadata_tag *list_tag = mtag->next;
        unsigned list_samples = mtag->length;
        unsigned time_of_arrival;

        time_of_arrival = get_time_of_arrival(mtag);
        /* the tag will be reused for timestamp, so unset the time of arrival flag. */
        METADATA_TIME_OF_ARRIVAL_UNSET(mtag);

        /* Calculate the total tags length. to avoid losing data when no tags are read
         * from the input buffer. */
        while (list_tag != NULL)
        {
            METADATA_TIME_OF_ARRIVAL_UNSET(list_tag);
            list_samples += list_tag->length;
            list_tag = list_tag->next;
        }

        /* For aptX, each stereo pair of samples is encoded as one octet
         * This means the sample count for TTP is the same as the number of encoded octets
         */
        ttp_update_ttp(opx_data->ttp_instance, time_of_arrival, list_samples, &status);

        /* Populate the metadata tag from the TTP status */
        ttp_utils_populate_tag(mtag, &status);

        /* Make sure only one tag gets the stream start flag */
        status.stream_restart = FALSE;

        /* In case there were multiple metadata tags on the input,
         * extrapolate the timestamps to any subsequent tags
         */
        list_tag = mtag->next;
        list_samples = mtag->length;
        while (list_tag != NULL)
        {
            status.ttp = ttp_get_next_timestamp(mtag->timestamp, list_samples, opx_data->sample_rate, status.sp_adjustment);
            ttp_utils_populate_tag(list_tag, &status);
            list_samples += list_tag->length;
            list_tag = list_tag->next;
        }
    }

    if (BUFF_METADATA(opx_data->op_buffer->buffer))
    {
        buff_metadata_append(opx_data->op_buffer->buffer, mtag, b4idx, afteridx);
    }
    else
    {
        buff_metadata_tag_list_delete(mtag);
    }

}

void rtp_decode_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    RTP_DECODE_OP_DATA *opx_data = (RTP_DECODE_OP_DATA *)(op_data->extra_op_data);

    unsigned packet_size;
    octet_buffer *ip_buffer = opx_data->ip_buffer;
    octet_buffer *op_buffer = opx_data->op_buffer;
    unsigned available_octets, available_space;
    unsigned rtp_header_size = RTP_MINIMUM_HEADER_SIZE +
                   (opx_data->scms_enable ? SCMS_HEADER_SIZE : 0);
    metadata_tag* tag;

    patch_fn(rtp_decode_process_data);


    if ((opx_data->play != NULL) &&
        (!(*(opx_data->play))))
    {
        return;
    }

    if ((ip_buffer == NULL) || (op_buffer == NULL))
    {
        return;
    }

    /* we only process the amount of data we saw on entry.
     * this is because the source may become invalid after we've
     * read the last byte.
     */
    /* TODO There could be an octet difference between _ex (used by a2dp endpoint)
     * and cbuffer (used by octet_buffer therefore by RTP) when asking for available data.
     * To avoid that difference use the buff_metadata_available_octets until RTP is not
     * convert to _ex buffer. NOTE  _ex buffer does not support unaligned copy yet.  */
    available_octets = buff_metadata_available_octets(ip_buffer->buffer);
    available_space = obuffer_calc_amount_space_in_octets(op_buffer);

    L4_DBG_MSG2("RTP START available data (octets) = %d, available space (octets) = %d",
            obuffer_calc_amount_data_in_octets(ip_buffer), obuffer_calc_amount_space_in_octets(op_buffer));

    if (opx_data->kick_on_full_output && (available_octets > available_space))
    {
        /* Kick forwards to keep data flowing in case the next operator is a splitter */
        touched->sources = TOUCHED_SOURCE_0;
    }

    if(opx_data->mode == RTP_PASSTHROUGH)
    {
        unsigned octets_copied = obuffer_copy_octets(op_buffer, ip_buffer, available_octets);

        /* copy as many data as possible to the output buffer.*/
        if (octets_copied > 0)
        {
            /* Propagate any metadata to the output.*/
            metadata_strict_transport(ip_buffer->buffer, op_buffer->buffer, octets_copied);
            touched->sources = TOUCHED_SOURCE_0;
        }

        if (obuffer_calc_amount_data_in_octets(ip_buffer) == 0)
        {
            touched->sinks = TOUCHED_SINK_0;
        }
        return;
    }
    else if ((opx_data->mode == RTP_TTP_ONLY) && (opx_data->codec_type == APTX))
    {

        unsigned octets_copied = MIN(available_octets, available_space);

        /* Because the write pointer gets rounded up if there's an odd octet,
         * it's possible for the aptX decoder to see one more octet than
         * actually exists. It can only ever process 4 octets at a time,
         * so rounding down here should always be safe
         */
        octets_copied &= ~1;

        if (octets_copied > 0)
        {
            metadata_transport_with_ttp(opx_data, octets_copied);
            obuffer_copy_octets(op_buffer, ip_buffer, octets_copied);
            touched->sources = TOUCHED_SOURCE_0;
        }

        /* Kick back if less than one whole word in the input buffer */
        if (obuffer_calc_amount_data_in_octets(ip_buffer) < 2)
        {
            touched->sinks = TOUCHED_SINK_0;
        }
        return;
    }

    /* Get the next from the input buffer.*/
    tag = buff_metadata_peek(ip_buffer->buffer);
    packet_size = get_tag_size (tag);

    while ( (packet_size != 0) && (available_octets >= packet_size))
    {
        if(!tag_valid(tag, opx_data))
        {
            return;
        }
        /* Is there space in the sink for the unpacked data?
         * this needs to be a quick check and hence don't worry
         * about calculating the header size which isn't copied.
         */
        if ((obuffer_calc_amount_space_in_octets(opx_data->clone_op_buffer) - 4 ) < packet_size)
        {
            L4_DBG_MSG2("RTP END  available data (octets) = %d, available space (octets) = %d !!output space smaller than packet size!!",
                    obuffer_calc_amount_data_in_octets(ip_buffer), obuffer_calc_amount_space_in_octets(op_buffer));
            return;
        }

        if (packet_size < (rtp_header_size +
                opx_data->payload_header_size))
        {
            /* packet is too small to contain header so discard it.*/
            obuffer_discard_data(ip_buffer,packet_size);
            delete_consumed_metadata_tag(ip_buffer, packet_size);
        }
        else
        {
            unsigned int fixed_header_first_part[RTP_FIXED_HEADER_FIRST_PART_LENGTH];
            /* read in first few bytes of RTP header */
            obuffer_unpack((int*)fixed_header_first_part, ip_buffer, RTP_FIXED_HEADER_FIRST_PART_LENGTH);

            /* validate version */
            if ((fixed_header_first_part[0] & RTP0_VERSION_MASK) != RTP0_VERSION_2)
            {
                /* Unsupported version - discard */
                obuffer_discard_data(ip_buffer, packet_size - RTP_FIXED_HEADER_FIRST_PART_LENGTH);
                delete_consumed_metadata_tag(ip_buffer, packet_size);
#ifdef DEBUG_RTP_DECODE
                /* Increment decode lost */
                opx_data->rtp_decode_lost++;
#endif
            }
            else
            {
                unsigned csrc_count = (fixed_header_first_part[0] & RTP0_CSRC_COUNT_MASK);
                unsigned seq = fixed_header_first_part[3] | (fixed_header_first_part[2]<<8);
                unsigned padding_amount = 0;
                unsigned header_size;
                unsigned payload_size;
                RTP_FRAME_DECODE_DATA frame_data;

                /* Set the frame header decode structure to remove gcc warnings. */
                frame_data.valid = FALSE;
                frame_data.frame_length = 0;
                frame_data.frame_samples = 0;
                frame_data.nr_of_frames = 0;

                /* check the sequence number unless local value is 0 which
                 * means initialisation. */
                if ((opx_data->sequence_number != 0) &&
                    (seq != (opx_data->sequence_number + 1)))
                {
#ifdef DEBUG_RTP_DECODE
                    opx_data->rtp_decode_late += seq - (opx_data->sequence_number + 1);
                    /* TODO - insert dummy frame when data is lost? (TODO This is an old comment) */
#endif
                }

                /* update sequence number */
                opx_data->sequence_number = seq;

                /* Calculate the header size */
                header_size = rtp_header_size + (csrc_count * 4) +
                             opx_data->payload_header_size;

                if ((fixed_header_first_part[0] & RTP0_PADDING) != 0)
                {
                    /* padding amount is the last byte in the buffer. The first part
                     * of the header is already read. The current read offset is
                     * RTP_FIXED_HEADER_FIRST_PART_LENGTH. */
                    obuffer_unpack_from_offset((int*) &padding_amount, ip_buffer,
                            packet_size - RTP_FIXED_HEADER_FIRST_PART_LENGTH - 1, 1);

                    /* ignore rogue values  */
                    if (padding_amount > (packet_size - header_size))
                    {
                        padding_amount = 0;
                    }
                }

                /* calc size of the frame that needs to be written to DSP  */
                payload_size = packet_size - header_size - padding_amount;

                /* The RTP timestamp field is actually a sample count which is not
                 * reliable so the rest of the header is discarded. */
                obuffer_discard_data(ip_buffer, header_size - RTP_FIXED_HEADER_FIRST_PART_LENGTH);

                /* Copy the payload without the padding to the clone buffer. There the
                 * decoded frame is analysed to see how many sample is in it.  */
                obuffer_copy_octets(opx_data->clone_op_buffer, ip_buffer, payload_size);

                /* Partially decode the frame to get a robust sample count determination
                 * for the time to play. */
                get_samples_in_packet(opx_data, &frame_data);

                if(opx_data->mode == RTP_STRIP)
                {
                    /* Delete the tag as it is not used any more.  */
                    delete_consumed_metadata_tag(ip_buffer, packet_size);
                    /* Update the metadata in the output buffer.*/
                    buff_metadata_append(opx_data->op_buffer->buffer, NULL, payload_size, 0);
                    /* Kick forward. */
                    touched->sources = TOUCHED_SOURCE_0;
                }
                else if (opx_data->mode == RTP_DECODE)
                {
                    /* Timestamp and transport the tag. */
                    transport_metadata_tag(opx_data, &frame_data, packet_size);
                    if (frame_data.nr_of_frames > 0)
                    {
                        /* Kick forward if there was any valid frames in the RTP packet. */
                        touched->sources = TOUCHED_SOURCE_0;
                    }
                }

#ifdef DEBUG_RTP_DECODE
                opx_data->rtp_decode_packet_count++;
#endif
                /* discard any padding */
                if (padding_amount != 0)
                {
                    obuffer_discard_data(ip_buffer, padding_amount);
                }

            }
        }
        available_octets -= packet_size;

        /* Read the next packet tag. */
        tag = buff_metadata_peek(ip_buffer->buffer);
        packet_size = get_tag_size (tag);
    }

    /*
     * Consumed all the available data -> kick backwards.
     */
    touched->sinks = TOUCHED_SINK_0;

    L4_DBG_MSG2("RTP END available data (octets) = %d, available space (octets) = %d",
            obuffer_calc_amount_data_in_octets(ip_buffer), obuffer_calc_amount_space_in_octets(op_buffer));
}


/* **************************** Operator message handlers ******************************** */


bool rtp_decode_opmsg_set_working_mode(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    RTP_DECODE_OP_DATA *opx_data;
    RTP_WORKING_MODES wm;

    wm = (RTP_WORKING_MODES) OPMSG_FIELD_GET(message_data,
                                OPMSG_RTP_SET_WORKING_MODE, WORKING_MODE);

    if (wm >= RTP_NR_OF_MODES)
    {
        return FALSE;
    }

    opx_data = (RTP_DECODE_OP_DATA *)(op_data->extra_op_data);

    /* Check a new time to play instance is needed. */
    if ((wm == RTP_DECODE)||(wm == RTP_TTP_ONLY))
    {
        if (opx_data->ttp_instance == NULL)
        {
            ttp_params params;

            /* Create a new ttp instance*/
            opx_data->ttp_instance = ttp_init();
            if (opx_data->ttp_instance == NULL)
            {
                return FALSE;
            }

            ttp_get_default_params(&params, TTP_TYPE_A2DP);
            ttp_configure_params(opx_data->ttp_instance, &params);
        }
    }

    opx_data->mode = wm;

    return TRUE;
}


bool rtp_decode_opmsg_set_codec_type(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    RTP_DECODE_OP_DATA *opx_data;
    RTP_CODEC_TYPE codec_type;

    codec_type = (RTP_CODEC_TYPE) OPMSG_FIELD_GET(message_data,
                                OPMSG_RTP_SET_CODEC_TYPE, CODEC_TYPE);

    if (codec_type >= NR_OF_CODECS)
    {
        return FALSE;
    }

    opx_data = (RTP_DECODE_OP_DATA *)(op_data->extra_op_data);
    opx_data->codec_type = codec_type;

    rtp_set_payload_header_size(opx_data);

    return TRUE;
}


bool rtp_decode_opmsg_set_content_protection(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    RTP_DECODE_OP_DATA *opx_data;

    opx_data = (RTP_DECODE_OP_DATA *)(op_data->extra_op_data);
    opx_data->scms_enable = OPMSG_FIELD_GET(message_data,
                             OPMSG_RTP_SET_CONTENT_PROTECTION, CONTENT_PROTECTION_ENABLED) & 1;

    return TRUE;
}

bool rtp_decode_opmsg_set_AAC_decoder(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    RTP_DECODE_OP_DATA *opx_data;
    unsigned aac_operator_id;
    void *aac_codec;

    opx_data = (RTP_DECODE_OP_DATA *)(op_data->extra_op_data);
    aac_operator_id = OPMSG_FIELD_GET(message_data,
                        OPMSG_RTP_SET_AAC_CODEC, OPERATOR_ID);

    aac_codec = get_AAC_codec(aac_operator_id);
    if (aac_codec != NULL)
    {
        opx_data->aac_codec = aac_codec;
        return TRUE;
    }
    /* Operator not found, signal an error back. */
    return FALSE;
}
bool rtp_decode_opmsg_set_AAC_utility(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    RTP_DECODE_OP_DATA *opx_data;
    AAC_UTILITY_SEL utility_selector;

    opx_data = (RTP_DECODE_OP_DATA *)(op_data->extra_op_data);
    utility_selector = (AAC_UTILITY_SEL) OPMSG_FIELD_GET(message_data,
                        OPMSG_RTP_SET_AAC_UTILITY, UTILITY_SEL);

    if ((utility_selector == AAC_LC_OVER_LATM)||(utility_selector == AAC_GENERIC))
    {
        opx_data->aac_utility_select = utility_selector;
        return TRUE;
    }
    /* Operator not found, signal an error back. */
    return FALSE;
}



bool rtp_decode_opmsg_set_play_pointer(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    L0_DBG_MSG("rtp_decode_opmsg_set_play_pointer" );
    ((RTP_DECODE_OP_DATA*)(op_data->extra_op_data))->play = (bool *)((uintptr_t)((unsigned*)(message_data))[3]);
    return TRUE;
}

bool rtp_decode_opmsg_set_ttp_latency(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    RTP_DECODE_OP_DATA *opx_data;

    opx_data = (RTP_DECODE_OP_DATA *)(op_data->extra_op_data);
    if ((opx_data->mode !=RTP_DECODE) && (opx_data->mode != RTP_TTP_ONLY))
    {
        return FALSE;
    }

    ttp_configure_latency(opx_data->ttp_instance, ttp_get_msg_latency(message_data));
    return TRUE;
}

bool rtp_decode_opmsg_set_latency_limits(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    RTP_DECODE_OP_DATA *opx_data;
    TIME_INTERVAL min_latency, max_latency;

    opx_data = (RTP_DECODE_OP_DATA *)(op_data->extra_op_data);
    if ((opx_data->mode !=RTP_DECODE) && (opx_data->mode != RTP_TTP_ONLY))
    {
        return FALSE;
    }
    ttp_get_msg_latency_limits(message_data, &min_latency, &max_latency);
    ttp_configure_latency_limits(opx_data->ttp_instance, min_latency, max_latency);

    return TRUE;
}

bool rtp_decode_opmsg_set_ttp_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    RTP_DECODE_OP_DATA *opx_data;
    ttp_params params;

    opx_data = (RTP_DECODE_OP_DATA *)(op_data->extra_op_data);
    if ((opx_data->mode != RTP_DECODE) && (opx_data->mode != RTP_TTP_ONLY))
    {
        return FALSE;
    }

    ttp_get_msg_params(&params, message_data);
    ttp_configure_params(opx_data->ttp_instance, &params);

    return TRUE;
}


bool rtp_decode_opmsg_set_sample_rate(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    RTP_DECODE_OP_DATA *opx_data = (RTP_DECODE_OP_DATA *)(op_data->extra_op_data);
    unsigned sample_rate = 25 * OPMSG_FIELD_GET(message_data, OPMSG_COMMON_MSG_SET_SAMPLE_RATE, SAMPLE_RATE);

    if ((opx_data->mode != RTP_DECODE) && (opx_data->mode != RTP_TTP_ONLY))
    {
        return FALSE;
    }

    opx_data->sample_rate = sample_rate;
    ttp_configure_rate(opx_data->ttp_instance, sample_rate);

    return TRUE;
}

bool rtp_decode_opmsg_set_buffer_size(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    RTP_DECODE_OP_DATA *opx_data = (RTP_DECODE_OP_DATA *)(op_data->extra_op_data);
    opx_data->latency_buffer_size = OPMSG_FIELD_GET(message_data, OPMSG_COMMON_SET_BUFFER_SIZE, BUFFER_SIZE);
    
    if (opx_data->latency_buffer_size > RTP_DECODE_DEFAULT_BUFFER_SIZE)
    {
        opx_data->kick_on_full_output = TRUE;
    }

    return TRUE;
}

bool rtp_decode_opmsg_set_max_packet_length(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    RTP_DECODE_OP_DATA *opx_data = (RTP_DECODE_OP_DATA *)(op_data->extra_op_data);
    if(op_data->state == OP_RUNNING)
    {
        return FALSE;
    }
    else
    {
        opx_data->max_packet_len = OPMSG_FIELD_GET(message_data, OPMSG_RTP_SET_MAX_PACKET_LENGTH, LENGTH);
        opx_data->last_op_tag_octets = opx_data->max_packet_len;
        return TRUE;
    }
}
