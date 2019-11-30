/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  encoder.c
 * \ingroup  capabilities
 *
 * Common framework for A2DP codecs to use.
 *
 */

 /****************************************************************************
Include Files
*/
#include "capabilities.h"
#include "common_encode.h"
#include "mem_utils/scratch_memory.h"
#include "platform/pl_assert.h"
#ifdef INSTALL_METADATA
#include "ttp/ttp.h"
#include "ttp/timed_playback.h"
#include "ttp_utilities.h"
#endif

#include "patch/patch.h"

/****************************************************************************
Public Constant Declarations
*/
#ifdef INSTALL_METADATA
#define LAST_TAG_SAMPLES_INVALID ((unsigned)-1)
#endif

/****************************************************************************
Public Function Declarations
*/

bool encoder_base_class_init(OPERATOR_DATA *op_data, ENCODER_PARAMS *enc_base,
                            void *codec_data, const ENCODER_CAP_VIRTUAL_TABLE * const vt)
{
    /* Check that valid pointers have been passed, they'll be dereferenced later
     * without being checked. */
    if (op_data == NULL || enc_base == NULL || codec_data == NULL || vt == NULL)
    {
        return FALSE;
    }
    enc_base->codec.encoder_data_object = codec_data;
    enc_base->vt = vt;
    op_data->cap_class_ext = enc_base;

    /* Now reserve the scratch memory */
    if (vt->scratch_allocs != NULL && scratch_register())
    {
        if (!mem_table_scratch_tbl_reserve(vt->scratch_allocs))
        {
            /* Fail free all the scratch memory we reserved */
            scratch_deregister();
        }
    }
    return TRUE;
}

bool encoder_destroy(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data)
{
    if(base_op_destroy(op_data, message_data, response_id, response_data))
    {
        ENCODER_PARAMS *encoder_data = (ENCODER_PARAMS *)(op_data->cap_class_ext);
        const ENCODER_CAP_VIRTUAL_TABLE *vt = encoder_data->vt;

        /* Clear up the encoder specific memory and then release what this
         * base class allocated. */
        if (vt->free_fn)
        {
            vt->free_fn(op_data);
        }
        /* Free all the scratch memory we reserved */
        if (vt->scratch_allocs != NULL)
        {
            scratch_deregister();
        }
        return TRUE;
    }

    return FALSE;
}


bool encoder_connect(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data)
{  
    ENCODER_PARAMS *encoder_data;
    unsigned terminal_id;

    if (!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }
    
    encoder_data = (ENCODER_PARAMS *)(op_data->cap_class_ext);
    terminal_id = ((unsigned *)message_data)[0];

    switch (terminal_id)
    {
        case OUTPUT_TERMINAL_ID:
            if (NULL == encoder_data->codec.out_buffer)
            {
                encoder_data->codec.out_buffer = (tCbuffer*)(((uintptr_t *)message_data)[1]);
#ifdef INSTALL_METADATA
                if (buff_has_metadata(encoder_data->codec.out_buffer))
                {
                    buff_metadata_set_usable_octets(encoder_data->codec.out_buffer, NR_OF_OCTETS_IN_WORD(USE_16BIT_PER_WORD));
                }
#endif
                return TRUE;
            }
                 
            break;
        case LEFT_IN_TERMINAL_ID:
            /* Check that the operator is not running */
            if (OP_RUNNING == op_data->state)
            {
                base_op_change_response_status(response_data, STATUS_CMD_FAILED);
                return TRUE;
            }

            if (NULL == encoder_data->codec.in_left_buffer)
            {
                encoder_data->codec.in_left_buffer = (tCbuffer*)(((uintptr_t *)message_data)[1]);
#ifdef INSTALL_METADATA
                if(encoder_data->metadata_ip_buffer == NULL)
                {
                    if (buff_has_metadata(encoder_data->codec.in_left_buffer))
                    {
                        encoder_data->metadata_ip_buffer = encoder_data->codec.in_left_buffer;
                    }
                }
#endif /* INSTALL_METADATA */
                return TRUE;
            }
            break;
        case RIGHT_IN_TERMINAL_ID:
            /* Check that the operator is not running */
            if (OP_RUNNING == op_data->state)
            {
                base_op_change_response_status(response_data, STATUS_CMD_FAILED);
                return TRUE;
            }

            if (NULL == encoder_data->codec.in_right_buffer)
            {
                encoder_data->codec.in_right_buffer = (tCbuffer*)(((uintptr_t *)message_data)[1]);
#ifdef INSTALL_METADATA
                if(encoder_data->metadata_ip_buffer == NULL)
                {
                    if (buff_has_metadata(encoder_data->codec.in_right_buffer))
                    {
                        encoder_data->metadata_ip_buffer = encoder_data->codec.in_right_buffer;
                    }
                }
#endif /* INSTALL_METADATA */

                return TRUE;
            }
            break;
        /* NB No default as can't happen */
    }

    /* Streams should not have reached this point of sending us a connect for already existing connection */
    panic_diatribe(PANIC_AUDIO_ALREADY_CONNECTED_OPERATOR_TERMINAL, op_data->id);
}


bool encoder_disconnect(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data)
{
    ENCODER_PARAMS * encoder_data;
    unsigned terminal_id = ((unsigned*)message_data)[0];

    /* Check that the capability is not running, Only the sink can be
     * disconnected whilst running!*/
    if (OP_RUNNING == op_data->state)
    {
        if (terminal_id != OUTPUT_TERMINAL_ID )
        {
            return base_op_build_std_response(STATUS_CMD_FAILED, op_data->id, response_data);
        }
    }

    if (!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }

    encoder_data = (ENCODER_PARAMS *)(op_data->cap_class_ext);

    switch (terminal_id)
    {
        case OUTPUT_TERMINAL_ID:
            if (NULL != encoder_data->codec.out_buffer)
            {
                encoder_data->codec.out_buffer = NULL;
                return TRUE;
            }
            break;
        case LEFT_IN_TERMINAL_ID:
            if (NULL != encoder_data->codec.in_left_buffer)
            {
#ifdef INSTALL_METADATA
                if (encoder_data->metadata_ip_buffer == encoder_data->codec.in_left_buffer)
                {
                    /* If the right input is still connected, get metadata from there. 
                     * If it's not, this will be NULL anyway.
                     */
                    encoder_data->metadata_ip_buffer = encoder_data->codec.in_right_buffer;
                }
#endif /* INSTALL_METADATA */
                encoder_data->codec.in_left_buffer = NULL;
                return TRUE;
            }
            break;
        case RIGHT_IN_TERMINAL_ID:
            if (NULL != encoder_data->codec.in_right_buffer )
            {
#ifdef INSTALL_METADATA
                if (encoder_data->metadata_ip_buffer == encoder_data->codec.in_right_buffer)
                {
                    /* If the left input is still connected, get metadata from there. 
                     * If it's not, this will be NULL anyway.
                     */
                    encoder_data->metadata_ip_buffer = encoder_data->codec.in_left_buffer;
                }
#endif /* INSTALL_METADATA */             
                encoder_data->codec.in_right_buffer = NULL;
                return TRUE;
            }
            break;
        /* NB No default as can't happen */
    }

    /* Streams should not have reached this point of sending us a disconnect for already disconnected terminal */
    panic_diatribe(PANIC_AUDIO_ALREADY_DISCONNECTED_OPERATOR_TERMINAL, op_data->id);
}


bool encoder_start(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data)
{
    ENCODER_PARAMS *encoder_data;
    
    *response_id = OPCMD_START;

    /* Create the response. If there aren't sufficient resources for this fail
     * early. */
    if (!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }

    if (OP_RUNNING == op_data->state)
    {
        /* Operator already started nothing to do. */
        return TRUE;
    }

    encoder_data = (ENCODER_PARAMS *)(op_data->cap_class_ext);
    
    /* At least the 1st source needs to be connected. */
    if (encoder_data->codec.in_left_buffer == NULL)
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    op_data->state = OP_RUNNING;
#ifdef INSTALL_METADATA
    encoder_data->last_tag_samples = LAST_TAG_SAMPLES_INVALID;
    encoder_data->last_tag_err_offset_id = INFO_ID_INVALID;
#endif /* INSTALL_METADATA */

    return TRUE;
}


bool encoder_reset(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data)
{
    ENCODER_PARAMS *encoder_data;
    const ENCODER_CAP_VIRTUAL_TABLE *vt;
#ifdef INSTALL_METADATA
    bool status_resp_ok = TRUE;
#endif

    if (!base_op_reset(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    encoder_data = (ENCODER_PARAMS *)(op_data->cap_class_ext);
    vt = encoder_data->vt;

    if (vt->reset_fn)
    {
        if (!vt->reset_fn(op_data))
        {
            base_op_change_response_status(response_data, STATUS_CMD_FAILED);
#ifdef INSTALL_METADATA
            status_resp_ok = FALSE;
#endif
        }
    }
#ifdef INSTALL_METADATA
    /* do something only if the current state is "running" */
    if (op_data->state == OP_RUNNING && status_resp_ok)
    {
        buff_metadata_remove_delay(encoder_data->codec.out_buffer, 0 * OCTETS_PER_SAMPLE);
    }
#endif /* INSTALL_METADATA */    
    return TRUE;
}

 bool encoder_buffer_details(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data)
{
    return encoder_buffer_details_core(op_data, message_data,response_id, response_data,A2DP_DECODE_INPUT_BUFFER_SIZE,A2DP_DECODE_OUTPUT_BUFFER_SIZE);
}


bool encoder_buffer_details_core(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data,unsigned inp_bufsize, unsigned out_bufsize)
{
#ifdef INSTALL_METADATA
    ENCODER_PARAMS *encoder_data = (ENCODER_PARAMS *)(op_data->cap_class_ext);
#endif
    unsigned terminal_id = ((unsigned *)message_data)[0];
    OP_BUF_DETAILS_RSP *resp;

    if (!base_op_buffer_details(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    resp = (OP_BUF_DETAILS_RSP *)*response_data;

    if ((terminal_id & TERMINAL_SINK_MASK) == TERMINAL_SINK_MASK)
    {
        /* return the sink buffer size */
        resp->b.buffer_size = inp_bufsize;
    }
    else
    {
        /* return the source buffer size */
        resp->b.buffer_size = out_bufsize;
    }

#ifdef INSTALL_METADATA
    if ((terminal_id & TERMINAL_SINK_MASK) == TERMINAL_SINK_MASK)
    {
        resp->metadata_buffer = encoder_data->metadata_ip_buffer;
    }
    else
    {
        if(encoder_data->codec.out_buffer != NULL &&
            BUFF_METADATA(encoder_data->codec.out_buffer))
        {
            resp->metadata_buffer = encoder_data->codec.out_buffer;
        }
        else
        {
            resp->metadata_buffer = NULL;
        }
    }
    resp->supports_metadata = TRUE;
#endif /* INSTALL_METADATA */

    return TRUE;
}


bool encoder_get_data_format(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data)
{
    /* Create the response. If there aren't sufficient resources for this fail
     * early. */
    if (!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }

    if (((unsigned *)message_data)[0] & TERMINAL_SINK_MASK)
    {
        /* The sinks are audio samples */
        ((OP_STD_RSP*)*response_data)->resp_data.data = AUDIO_DATA_FORMAT_FIXP;
    }
    else
    {
        /* The source is encoded SBC data */
        ((OP_STD_RSP*)*response_data)->resp_data.data = AUDIO_DATA_FORMAT_16_BIT;
    }

    *response_id = OPCMD_DATA_FORMAT;

    return TRUE;
}


bool encoder_get_sched_info(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data)
{    
    OP_SCHED_INFO_RSP* resp;
    
    if (!base_op_get_sched_info(op_data, message_data, response_id, response_data))
    {
        return base_op_build_std_response(STATUS_CMD_FAILED, op_data->id, response_data);
    }
    
    /* Populate the response */
    base_op_change_response_status(response_data, STATUS_OK);
    resp = *response_data;
    resp->op_id = op_data->id;
    /* Input and output are both unknown */
    resp->block_size = 0; 
    resp->run_period = 0;
    
    *response_data = resp;
    return TRUE;
}


void encode_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    ENCODER_PARAMS *enc_base = (ENCODER_PARAMS *)(op_data->cap_class_ext);
    const ENCODER_CAP_VIRTUAL_TABLE *vt;
    bool output_produced;
    unsigned ip_avail_data_pre_enc , ip_avail_data_post_enc;
    unsigned ip_proc_data;

    patch_fn(encoder_process_data);
    
    /* Check the ouput hasn't gone away, if it has then nothing we can do. It
     * may be a radio link so this can happen */
    if (NULL == enc_base->codec.out_buffer)
    {
        /* One option is to consume the input data in this case.*/
        return;
    }
    /* available data in input buffer before processing */
    ip_avail_data_pre_enc = cbuffer_calc_amount_data_in_words(enc_base->codec.in_left_buffer);

    vt = enc_base->vt;
    if (vt->scratch_allocs != NULL)
    {
        mem_table_scratch_tbl_commit(enc_base->codec.encoder_data_object, vt->scratch_allocs);
        output_produced = encoder_encode(&(enc_base->codec), vt->encode_fn);
        /* Free the scratch memory used */
        scratch_free();
    }
    else
    {
        output_produced = encoder_encode(&(enc_base->codec), vt->encode_fn);
    }
    /* available data in input buffer after processing */
    ip_avail_data_post_enc = cbuffer_calc_amount_data_in_words(enc_base->codec.in_left_buffer);
    /* total data processed on the input */
    ip_proc_data = ip_avail_data_pre_enc - ip_avail_data_post_enc;

    if (output_produced)
    {

#ifdef INSTALL_METADATA
        unsigned ip_proc_data_octets;
        unsigned b4idx, afteridx;
        metadata_tag *mtag_ip, *mtag_ip_list;
        metadata_tag *mtag;
        tCbuffer *src, *dst;
        unsigned frame, frame_count, frame_len_enc_octets, frame_len_samples;
        unsigned new_ttp, base_ttp = 0, sample_offset = 0;
        bool framed_data;
        ttp_status status;
   
        src = enc_base->metadata_ip_buffer;
        dst = enc_base->codec.out_buffer;
        ip_proc_data_octets = ip_proc_data * OCTETS_PER_SAMPLE;

        framed_data = vt->frame_size_fn(op_data, &frame_len_samples, &frame_len_enc_octets);

        if (framed_data)
        {
            /* Data is framed, so handle multiple output frames if necessary */
            frame_count = ip_proc_data / frame_len_samples;
        }
        else
        {
            /* Treat whatever was produced as a single "frame" */
            frame_count = 1;
            frame_len_enc_octets = frame_len_enc_octets * (ip_proc_data / frame_len_samples);
            frame_len_samples = ip_proc_data;
        }

        /* Extract metadata tag from input */
        mtag_ip_list = buff_metadata_remove(src, ip_proc_data_octets, &b4idx, &afteridx);

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
            if (enc_base->last_tag_samples != LAST_TAG_SAMPLES_INVALID)
            {
                base_ttp = enc_base->last_tag_timestamp;
                sample_offset = enc_base->last_tag_samples;
            }
        }

        if (mtag_ip != NULL)
        {
            unsigned *err_offset_id;
            unsigned length;
            /* Save the timestamp info from the incoming metadata */
            enc_base->last_tag_timestamp = mtag_ip->timestamp;
            enc_base->last_tag_spa = mtag_ip->sp_adjust;
            enc_base->last_tag_samples = ip_proc_data - (b4idx / OCTETS_PER_SAMPLE);
            if (buff_metadata_find_private_data(mtag_ip, META_PRIV_KEY_TTP_OFFSET, &length, (void **)&err_offset_id))
            {
                enc_base->last_tag_err_offset_id = (*err_offset_id);
            }
            else
            {
                enc_base->last_tag_err_offset_id = INFO_ID_INVALID;
            }
        }
        else 
        {
            if (enc_base->last_tag_samples != LAST_TAG_SAMPLES_INVALID)
            {
                enc_base->last_tag_samples += ip_proc_data;
            }
        }

        status.sp_adjustment = enc_base->last_tag_spa;
        status.err_offset_id = enc_base->last_tag_err_offset_id;
        status.stream_restart = (METADATA_STREAM_START(mtag_ip) != 0);

        /* Create new tags for each output frame */
        for (frame = 0; frame < frame_count; frame++)
        {
            mtag = buff_metadata_new_tag();

            if (mtag != NULL)
            {
                mtag->length = frame_len_enc_octets;
                METADATA_PACKET_START_SET(mtag);
                METADATA_PACKET_END_SET(mtag);

                if (enc_base->last_tag_samples != LAST_TAG_SAMPLES_INVALID)
                {
                    /* Calculate new TTP from incoming data and sample offset */
                    new_ttp = ttp_get_next_timestamp(base_ttp, sample_offset + frame * frame_len_samples, 
                        enc_base->sample_rate, enc_base->last_tag_spa);
                    new_ttp = time_sub(new_ttp, convert_samples_to_time(enc_base->delay_samples, enc_base->sample_rate));
                    status.ttp = new_ttp;
                    ttp_utils_populate_tag(mtag, &status);
                    status.stream_restart = FALSE;
                }
                else
                {
                    L2_DBG_MSG1("Encoder op %d last tag samples invalid", op_data->id);
                }
            }
            else
            {
                L2_DBG_MSG1("Encoder op %d failed to allocate tag", op_data->id);
            }

            buff_metadata_append(dst, mtag, 0, frame_len_enc_octets);
        }

        /* Free all the incoming tags */
        buff_metadata_tag_list_delete(mtag_ip_list);
#else
        NOT_USED(ip_proc_data);
#endif      /* INSTALL_METADATA */    
    
        /* It ran and produced output so kick forwards */
        touched->sources = TOUCHED_SOURCE_0;

        /* If input data was stopped us running again then kick back to get some more.
         * TODO what if output space was found to be full first?
         */
        if (enc_base->codec.mode == CODEC_NOT_ENOUGH_INPUT_DATA)
        {
            touched->sinks = TOUCHED_SOURCE_0 | TOUCHED_SOURCE_1;
        }
    }

}


/* **************************** Operator message handlers ******************************** */


