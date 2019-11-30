/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  async_wbs_enc.c
 * \ingroup  operators
 *
 *  ASYNC_WBS_ENC operator
 *
 */

/****************************************************************************
Include Files
*/
#include "async_wbs_private.h"
#include "async_wbs_struct.h"
#include "encoder/common_encode.h"

#include "patch/patch.h"

#include "ttp/ttp.h"
#include "ttp/timed_playback.h"
#include "ttp_utilities.h"

/****************************************************************************
Private Type Definitions
*/

/****************************************************************************
Private Constant Declarations
*/
#define LAST_TAG_SAMPLES_INVALID ((unsigned)-1)
#define SBC_DELAY_8_SUBBANDS 73
#define PLC_DELAY_OVERLAP_BUFFER OLA_LEN_WB
#define ASYNC_WBS_DELAY (SBC_DELAY_8_SUBBANDS + PLC_DELAY_OVERLAP_BUFFER)


/** The WBS encoder capability function handler table */
const handler_lookup_struct async_wbs_enc_handler_table =
{
    async_wbs_enc_create,           /* OPCMD_CREATE */
    wbs_enc_destroy,          /* OPCMD_DESTROY */
    async_wbs_enc_start,            /* OPCMD_START */
    base_op_stop,             /* OPCMD_STOP */
    wbs_enc_reset,            /* OPCMD_RESET */
    async_wbs_enc_connect,          /* OPCMD_CONNECT */
    async_wbs_enc_disconnect,       /* OPCMD_DISCONNECT */
    async_wbs_enc_buffer_details,       /* OPCMD_BUFFER_DETAILS */
    encoder_get_data_format,  /* OPCMD_DATA_FORMAT */
    encoder_get_sched_info        /* OPCMD_GET_SCHED_INFO */
};

/* Null terminated operator message handler table */
const opmsg_handler_lookup_table_entry async_wbs_enc_opmsg_handler_table[] =
    {{OPMSG_COMMON_ID_GET_CAPABILITY_VERSION,      base_op_opmsg_get_capability_version},
    {OPMSG_COMMON_ID_FADEOUT_ENABLE,               wbs_enc_opmsg_enable_fadeout},
    {OPMSG_COMMON_ID_FADEOUT_DISABLE,              wbs_enc_opmsg_disable_fadeout},
    {OPMSG_ASYNC_WBS_ENC_ID_SET_TO_AIR_INFO,       wbs_enc_opmsg_set_to_air_info},
    {OPMSG_ASYNC_WBS_ENC_ID_SET_BITPOOL_VALUE,     awbs_enc_opmsg_set_encoding_params},
    {0, NULL}};


const CAPABILITY_DATA async_wbs_enc_cap_data =
    {
        ASYNC_WBS_ENC_CAP_ID,             /* Capability ID */
        0, 2,                           /* Version information - hi and lo parts */
        1, 1,                           /* Max number of sinks/inputs and sources/outputs */
        &async_wbs_enc_handler_table,         /* Pointer to message handler function table */
        async_wbs_enc_opmsg_handler_table,    /* Pointer to operator message handler function table */
        async_wbs_enc_process_data,           /* Pointer to data processing function */
        0,                              /* TODO - Processing time information */
        sizeof(ASYNC_WBS_ENC_OP_DATA)         /* Size of capability-specific per-instance data */
    };

/****************************************************************************
Private Function Definitions
*/
static inline ASYNC_WBS_ENC_OP_DATA *get_instance_data(OPERATOR_DATA *op_data)
{
    return (ASYNC_WBS_ENC_OP_DATA *) op_data->extra_op_data;
}

/* ********************************** API functions ************************************* */

bool async_wbs_enc_create(OPERATOR_DATA *op_data, void *message_data,
        unsigned *response_id, void **response_data)
{
    ASYNC_WBS_ENC_OP_DATA* encoder = get_instance_data(op_data);

    if (! wbs_enc_create(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* Force the sample rate for this encoder. */
    encoder->aswbs.sample_rate = 16000;
    encoder->aswbs.delay_samples = ASYNC_WBS_DELAY;
    return TRUE;
}

bool async_wbs_enc_start(OPERATOR_DATA *op_data, void *message_data,
        unsigned *response_id, void **response_data)
{
    ASYNC_WBS_ENC_OP_DATA *xdata = get_instance_data(op_data);

    if (!base_op_start(op_data, message_data, response_id,response_data))
    {
        return FALSE;
    }

    /* Sanity check for buffers being connected.
     * We can't do much useful without */
    if (   xdata->wbs.buffers.ip_buffer == NULL
        || xdata->wbs.buffers.op_buffer == NULL)
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    return TRUE;
}

bool async_wbs_enc_connect(OPERATOR_DATA *op_data, void *message_data,
        unsigned *response_id, void **response_data)
{
    ASYNC_WBS_ENC_OP_DATA *wbs_enc;
    unsigned terminal_id;

    if (!wbs_enc_connect(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    if (((OP_STD_RSP *)(*response_data))->status != STATUS_OK)
    {
        return FALSE;
    }

    /* Note, encoder code uses subclass ! */
    wbs_enc = get_instance_data(op_data);
    terminal_id = ((unsigned *)message_data)[0];

    switch (terminal_id)
    {
        case OUTPUT_TERMINAL_ID:
            if (buff_has_metadata(wbs_enc->wbs.buffers.op_buffer))
            {
                buff_metadata_set_usable_octets(wbs_enc->wbs.buffers.op_buffer,
                        NR_OF_OCTETS_IN_WORD(USE_16BIT_PER_WORD));
            }
            break;

        case LEFT_IN_TERMINAL_ID:
            if (buff_has_metadata(wbs_enc->wbs.buffers.ip_buffer))
            {
                wbs_enc->aswbs.metadata_ip_buffer = wbs_enc->wbs.buffers.ip_buffer;
            }
            break;
        default:
            return FALSE;
    }
    return TRUE;
}

bool async_wbs_enc_disconnect(OPERATOR_DATA *op_data, void *message_data,
        unsigned *response_id, void **response_data)
{
    ASYNC_WBS_ENC_OP_DATA *wbs_enc;
    unsigned terminal_id = ((unsigned*)message_data)[0];

    if (! wbs_enc_disconnect(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    if (((OP_STD_RSP *)(*response_data))->status != STATUS_OK)
    {
        return FALSE;
    }

    /* Note, encoder code uses subclass ! */
    wbs_enc = get_instance_data(op_data);

    if (terminal_id == LEFT_IN_TERMINAL_ID)
    {
        if (NULL != wbs_enc->wbs.buffers.ip_buffer)
        {
            wbs_enc->wbs.buffers.ip_buffer = NULL;
            wbs_enc->aswbs.metadata_ip_buffer = NULL;
        }
    }
    return TRUE;
}



/* ************************************* Data processing-related functions and wrappers **********************************/

void async_wbs_enc_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    WBS_ENC_OP_DATA* x_data = &(get_instance_data(op_data)->wbs);
    ASYNC_WBS_ENC_SPECIFIC_DATA* aswbs_data = &(get_instance_data(op_data)->aswbs);
    unsigned count;
    unsigned available_space, samples_to_process;
    unsigned ip_proc_data, ip_proc_data_octets;
    unsigned wbs_enc_output_block_size, wbs_enc_output_block_size_w;
    unsigned b4idx, afteridx;
    metadata_tag *mtag_ip, *mtag_ip_list;
    metadata_tag *mtag;
    tCbuffer *src, *dst;
    unsigned frame_len_enc_octets;
    unsigned new_ttp, base_ttp = 0, sample_offset = 0;
    ttp_status status;

    patch_fn(async_wbs_encode_process_data);

    wbs_enc_output_block_size = awbs_encode_frame_size(x_data->codec_data);
    wbs_enc_output_block_size_w = CONVERT_OCTETS_TO_SAMPLES(wbs_enc_output_block_size);

    available_space = cbuffer_calc_amount_space_in_words(x_data->buffers.op_buffer);

    samples_to_process = cbuffer_calc_amount_data_in_words(x_data->buffers.ip_buffer);
    if (aswbs_data->metadata_ip_buffer != NULL)
    {
        /* Check available data according to metadata and avoid processing more
         * data than metadata. */
        unsigned available_input_meta = CONVERT_OCTETS_TO_SAMPLES(
                                buff_metadata_available_octets(aswbs_data->metadata_ip_buffer));
        if (available_input_meta < samples_to_process)
        {
            samples_to_process = available_input_meta;
        }
    }

    /* Loop until we've done all we need to.
     */
    count = 0;
    while ((available_space >= wbs_enc_output_block_size_w) &&
           (samples_to_process >= WBS_ENC_DEFAULT_INPUT_BLOCK_SIZE))
    {
        /* Is fadeout enabled? if yes, do it on the current input data */
        if(x_data->fadeout_parameters.fadeout_state != NOT_RUNNING_STATE)
        {
            /* the wrapper below takes output Cbuffer and fadeout params, use input block size */
            if(mono_cbuffer_fadeout(x_data->buffers.ip_buffer, WBS_ENC_DEFAULT_INPUT_BLOCK_SIZE,
                                     &(x_data->fadeout_parameters)))
            {
                common_send_simple_unsolicited_message(op_data, OPMSG_REPLY_ID_FADEOUT_DONE);
            }
        }

        /* Commit any scratch memory ideally this should be done later after the
         * decision to decode is made. */
        mem_table_scratch_tbl_commit(x_data->codec_data, &wbs_enc_scratch_table);

        wbsenc_process_frame(op_data);

        /* Free the scratch memory used */
        scratch_free();

        available_space -= wbs_enc_output_block_size_w;
        samples_to_process -= WBS_ENC_DEFAULT_INPUT_BLOCK_SIZE;

        touched->sources =  TOUCHED_SOURCE_0;
        count++;
    }

    if (samples_to_process < WBS_ENC_DEFAULT_INPUT_BLOCK_SIZE)
    {
        /* If there isn't enough data to process another frame kick backwards */
        touched->sinks = TOUCHED_SINK_0;
    }

    if (count > 0)
    {
        /* total data processed on the input */
        ip_proc_data = count * WBS_ENC_DEFAULT_INPUT_BLOCK_SIZE;

        /* total data output (octets) */
        frame_len_enc_octets = count * wbs_enc_output_block_size;

        src = aswbs_data->metadata_ip_buffer;
        dst = x_data->buffers.op_buffer;
        ip_proc_data_octets = ip_proc_data * OCTETS_PER_SAMPLE;

        /* Extract metadata tag from input */
        mtag_ip_list = buff_metadata_remove(src, ip_proc_data_octets, &b4idx, &afteridx);
        L4_DBG_MSG4("meta: %p %d b4%d after %d",mtag_ip_list,!mtag_ip_list?0:(mtag_ip_list->timestamp),b4idx,afteridx);

        /* Find the first timestamped tag */
        mtag_ip = mtag_ip_list;
        while ((mtag_ip != NULL) && (!IS_TIMESTAMPED_TAG(mtag_ip)))
        {
            b4idx += mtag_ip->length;
            mtag_ip = mtag_ip->next;
        }

        if ((b4idx == 0) && (mtag_ip != NULL))
        {
            /* If the old tag is already at the start of the encoded frame,
             * Just use its timestamp directly
             */
            base_ttp = mtag_ip->timestamp;
            sample_offset = 0;
        }
        else
        {
            /* Otherwise, use the previously-stashed timestamp.
             * There had better be one ! */
            if (aswbs_data->last_tag_samples != LAST_TAG_SAMPLES_INVALID)
            {
                base_ttp = aswbs_data->last_tag_timestamp;
                sample_offset = aswbs_data->last_tag_samples;
            }
        }

        if (mtag_ip != NULL)
        {
            unsigned *err_offset_id;
            unsigned length;
            /* Save the timestamp info from the incoming metadata */
            aswbs_data->last_tag_timestamp = mtag_ip->timestamp;
            aswbs_data->last_tag_spa = mtag_ip->sp_adjust;
            aswbs_data->last_tag_samples = ip_proc_data - (b4idx / OCTETS_PER_SAMPLE);
            if (buff_metadata_find_private_data(mtag_ip, META_PRIV_KEY_TTP_OFFSET,
                                                &length, (void **)&err_offset_id))
            {
                aswbs_data->last_tag_err_offset_id = (*err_offset_id);
            }
            else
            {
                aswbs_data->last_tag_err_offset_id = INFO_ID_INVALID;
            }
        }
        else
        {
            if (aswbs_data->last_tag_samples != LAST_TAG_SAMPLES_INVALID)
            {
                aswbs_data->last_tag_samples += ip_proc_data;
            }
        }

        status.sp_adjustment = aswbs_data->last_tag_spa;
        status.err_offset_id = aswbs_data->last_tag_err_offset_id;
        status.stream_restart = ((mtag_ip != NULL)
                                 && (METADATA_STREAM_START(mtag_ip) != 0));

        /* Create new tags for each output frame */
        mtag = buff_metadata_new_tag();

        if (mtag != NULL)
        {
            mtag->length = frame_len_enc_octets;
            METADATA_PACKET_START_SET(mtag);
            METADATA_PACKET_END_SET(mtag);

            if (aswbs_data->last_tag_samples != LAST_TAG_SAMPLES_INVALID)
            {

                /***** We want delay samples to be set here, and also to check that the
                    Next timestamp is being set correctly before the subtraction */

                /* Calculate new TTP from incoming data and sample offset */
                new_ttp = ttp_get_next_timestamp(base_ttp,
                                                 sample_offset,
                                                 aswbs_data->sample_rate,
                                                 aswbs_data->last_tag_spa);
                new_ttp = time_sub( new_ttp,
                                    convert_samples_to_time(aswbs_data->delay_samples,
                                                            aswbs_data->sample_rate));
                status.ttp = new_ttp;
                ttp_utils_populate_tag(mtag, &status);
                status.stream_restart = FALSE;
            }
            else
            {
                L4_DBG_MSG("WBS2Enc last tag samples invalid");
            }
        }
        else
        {
            L4_DBG_MSG("WBS2Enc failed to allocate tag");
        }

        buff_metadata_append(dst, mtag, 0, frame_len_enc_octets);
        L4_DBG_MSG2("Meta: TTP:x%x Len:%d",mtag->timestamp,mtag->length);

        /* Free all the incoming tags */
        buff_metadata_tag_list_delete(mtag_ip_list);
    }
}


bool awbs_enc_opmsg_set_encoding_params(OPERATOR_DATA *op_data, void *message_data,
        unsigned int *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    ASYNC_WBS_ENC_OP_DATA *awbs_data = get_instance_data(op_data);
    sbc_codec *codec_data = awbs_data->wbs.codec_data;
    unsigned bitpool = 0;

    if (op_data->state == OP_RUNNING)
    {
        return FALSE;
    }

    if (OPMGR_GET_OPMSG_LENGTH((OP_MSG_REQ *)message_data) != OPMSG_ASYNC_WBS_ENC_SET_BITPOOL_VALUE_WORD_SIZE)
    {
        return FALSE;
    }

    bitpool = OPMSG_FIELD_GET(message_data, OPMSG_ASYNC_WBS_ENC_SET_BITPOOL_VALUE, BITPOOL);

    if (bitpool > ASYNC_WBS_MAX_BITPOOL_VALUE || bitpool < ASYNC_WBS_MIN_BITPOOL_VALUE)
    {
        return FALSE;
    }

    /* Populate the sbc structure with the new fields */
    codec_data->enc_setting_bitpool = bitpool;
    codec_data->bitpool = bitpool;

    return TRUE;
}

bool async_wbs_enc_buffer_details(OPERATOR_DATA *op_data, void *message_data,
        unsigned *response_id, void **response_data)
{
    unsigned wbs_buff_size = 0;

    if(!base_op_buffer_details(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* Currently these have the same value but this isn't guaranteed */
    if (((unsigned *)message_data)[0] & TERMINAL_SINK_MASK)
    {
        wbs_buff_size = WBS_ENC_INPUT_BUFFER_SIZE;
    }
    else
    {
        wbs_buff_size = WBS_ENC_OUTPUT_BUFFER_SIZE;
    }

    ((OP_BUF_DETAILS_RSP*)*response_data)->b.buffer_size = wbs_buff_size;
    /* supports metadata in both side  */
    ((OP_BUF_DETAILS_RSP*)*response_data)->metadata_buffer = 0;
    ((OP_BUF_DETAILS_RSP*)*response_data)->supports_metadata = TRUE;

    L4_DBG_MSG2( "wbs_buffer_details (capID=%d)  %d \n", op_data->cap_data->id, ((OP_STD_RSP*)*response_data)->resp_data.data);

    return TRUE;
}


