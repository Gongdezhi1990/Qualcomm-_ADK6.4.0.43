/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  a2dp_decode.c
 * \ingroup  capabilities
 *
 * Common framework for A2DP codecs to use.
 *
 */

 /****************************************************************************
Include Files
*/
#include "capabilities.h"
#include "codec_c.h"
#include "a2dp_common_decode.h"
#include "mem_utils/shared_memory_ids.h"
#include "fault/fault.h"

/****************************************************************************
Public Constant Declarations
*/

const opmsg_handler_lookup_table_entry a2dp_decode_opmsg_handler_table[] =
{
    {OPMSG_COMMON_ID_GET_CAPABILITY_VERSION, base_op_opmsg_get_capability_version},
    {OPMSG_COMMON_ID_FADEOUT_ENABLE, a2dp_dec_opmsg_enable_fadeout},
    {OPMSG_COMMON_ID_FADEOUT_DISABLE, a2dp_dec_opmsg_disable_fadeout},
    {OPMSG_SET_CTRL, a2dp_dec_assign_buffering},
#ifndef TIMED_PLAYBACK_MODE
    {OPMSG_COMMON_SET_RM_ENACTING, a2dp_dec_opmsg_ratematch_enacting},
#endif
    {0, NULL}
};

/****************************************************************************
Public Function Declarations
*/

/**
 * \brief Connects a capability terminal to a buffer.
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the connect request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
 bool a2dp_decode_connect(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data)
{
    return a2dp_decode_connect_core(op_data, message_data,response_id,response_data ,A2DP_DECODE_OUTPUT_BUFFER_SIZE);
}


bool a2dp_decode_connect_core(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data , unsigned out_bufsize)
{
    A2DP_DECODER_PARAMS *decoder_data;
    unsigned terminal_id;

    /* Check that the capability is not running */
    if (OP_RUNNING == op_data->state)
    {
        return base_op_build_std_response(STATUS_CMD_FAILED, op_data->id, response_data);
    }

    if (!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }

    decoder_data = (A2DP_DECODER_PARAMS *)(op_data->extra_op_data);
    terminal_id = ((unsigned *)message_data)[0];

    switch (terminal_id)
    {
        case INPUT_TERMINAL_ID:
            if (NULL == decoder_data->codec.in_buffer)
            {
                decoder_data->codec.in_buffer = (tCbuffer*)(((uintptr_t *)message_data)[1]);
                return TRUE;
            }
            break;
        case LEFT_OUT_TERMINAL_ID:
            if (NULL == decoder_data->op_out_left)
            {
                decoder_data->op_out_left = (tCbuffer*)(((uintptr_t *)message_data)[1]);
#ifdef INSTALL_METADATA
                if(decoder_data->metadata_op_buffer == NULL)
                {
                    if (buff_has_metadata(decoder_data->op_out_left))
                    {
                        decoder_data->metadata_op_buffer = decoder_data->op_out_left;
                    }
                }
#endif /* INSTALL_METADATA */

#ifdef TIMED_PLAYBACK_MODE
                decoder_data->codec.out_left_buffer = decoder_data->op_out_left;
#else /* TIMED_PLAYBACK_MODE */
                /* If not performing SRA then the decoder runs directly into the
                 * operator output buffers. */
                if (NULL == decoder_data->sra)
                {
                    decoder_data->codec.out_left_buffer = decoder_data->op_out_left;
                }
                else
                {
                    unsigned buf_size = (out_bufsize>>1) * sizeof(unsigned) + sizeof(unsigned);
                    /* The codec will use a scratch buffer, which was reserved
                     * by the enactment message. Until we commit the RAM it
                     * doesn't have a pointer so we pass in NULL. */
                    decoder_data->codec.out_left_buffer = cbuffer_create(NULL, buf_size, BUF_DESC_SW_BUFFER);
                    if (decoder_data->codec.out_left_buffer == NULL)
                    {
                        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
                        return TRUE;
                    }
                }
#endif /* TIMED_PLAYBACK_MODE */
                return TRUE;
            }
            break;
        case RIGHT_OUT_TERMINAL_ID:
            if (NULL == decoder_data->op_out_right)
            {
                decoder_data->op_out_right = (tCbuffer*)(((uintptr_t *)message_data)[1]);
#ifdef INSTALL_METADATA
                if(decoder_data->metadata_op_buffer == NULL)
                {
                    if (buff_has_metadata(decoder_data->op_out_right))
                    {
                        decoder_data->metadata_op_buffer = decoder_data->op_out_right;
                    }
                }
#endif /* INSTALL_METADATA */

#ifdef TIMED_PLAYBACK_MODE
                decoder_data->codec.out_right_buffer = decoder_data->op_out_right;
#else /* TIMED_PLAYBACK_MODE */
                /* If not performing SRA then the decoder runs directly into the
                 * operator output buffers. */
                if (NULL == decoder_data->sra)
                {
                    decoder_data->codec.out_right_buffer = decoder_data->op_out_right;
                }
                else
                {
                    unsigned buf_size = (out_bufsize>>1) * sizeof(unsigned) + sizeof(unsigned);
                    /* The codec will use a scratch buffer, which was reserved
                     * by the enactment message. Until we commit the RAM it
                     * doesn't have a pointer so we pass in NULL. */
                    decoder_data->codec.out_right_buffer = cbuffer_create(NULL, buf_size, BUF_DESC_SW_BUFFER);
                    if (decoder_data->codec.out_right_buffer == NULL)
                    {
                        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
                        return TRUE;
                    }
                }
#endif /* TIMED_PLAYBACK_MODE */
                return TRUE;
            }
            break;
        /* NB No default as can't happen */
    }

    /* Streams should not have reached this point of sending us a connect for already existing connection */
    panic_diatribe(PANIC_AUDIO_ALREADY_CONNECTED_OPERATOR_TERMINAL, op_data->id);
}

/**
 * \brief Disconnects a capability terminal from a buffer.
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the disconnect request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool a2dp_decode_disconnect(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data)
{
    A2DP_DECODER_PARAMS * decoder_data;
    unsigned terminal_id = ((unsigned*)message_data)[0];

    /* Check that the capability is not running, Only the sink can be
     * disconnected whilst running!*/
    if (OP_RUNNING == op_data->state)
    {
        if (terminal_id != INPUT_TERMINAL_ID )
        {
            return base_op_build_std_response(STATUS_CMD_FAILED, op_data->id, response_data);
        }
    }

    if (!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }

    decoder_data = (A2DP_DECODER_PARAMS *)(op_data->extra_op_data);

    switch (terminal_id)
    {
        case INPUT_TERMINAL_ID:
            if (NULL != decoder_data->codec.in_buffer)
            {
                decoder_data->codec.in_buffer = NULL;
                return TRUE;
            }
            break;
        case LEFT_OUT_TERMINAL_ID:
            if (NULL != decoder_data->op_out_left)
            {
#ifdef INSTALL_METADATA
                if (decoder_data->metadata_op_buffer == decoder_data->op_out_left)
                {
                    decoder_data->metadata_op_buffer = NULL;
                }
#endif /* INSTALL_METADATA */

                decoder_data->op_out_left = NULL;

#ifndef TIMED_PLAYBACK_MODE
                if (NULL != decoder_data->sra)
                {
                    /* Only free the cbuffer struct and not the underlying physical memory
                     * that is managed using scratch_reserve and scratch_release.
                     */
                    cbuffer_destroy_struct(decoder_data->codec.out_left_buffer);
                }
#endif /* TIMED_PLAYBACK_MODE */

                decoder_data->codec.out_left_buffer = NULL;
                return TRUE;
            }
            break;
        case RIGHT_OUT_TERMINAL_ID:
            if (NULL != decoder_data->op_out_right)
            {
#ifdef INSTALL_METADATA
                if (decoder_data->metadata_op_buffer == decoder_data->op_out_right)
                {
                    decoder_data->metadata_op_buffer = NULL;
                }
#endif /* INSTALL_METADATA */
                
                decoder_data->op_out_right = NULL;

#ifndef TIMED_PLAYBACK_MODE
                if (NULL != decoder_data->sra)
                {
                    /* Only free the cbuffer struct and not the underlying physical memory
                     * that is managed using scratch_reserve and scratch_release.
                     */
                    cbuffer_destroy_struct(decoder_data->codec.out_right_buffer);
                }
#endif /* TIMED_PLAYBACK_MODE */

                decoder_data->codec.out_right_buffer = NULL;
                return TRUE;
            }
            break;
        /* NB No default as can't happen */
    }

    /* Streams should not have reached this point of sending us a disconnect for already disconnected terminal */
    panic_diatribe(PANIC_AUDIO_ALREADY_DISCONNECTED_OPERATOR_TERMINAL, op_data->id);
}

/**
 * \brief Starts the a2pd_decode capability so decoding will be attempted on a
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
bool a2dp_decode_start(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data)
{
    A2DP_DECODER_PARAMS *decoder_data;

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

    decoder_data = (A2DP_DECODER_PARAMS *)(op_data->extra_op_data);

    /* At least the sink and the 1st source need to be connected. TODO in
     * stereo mode 2 sources should be connected. */
    if (!(decoder_data->codec.in_buffer != NULL && decoder_data->codec.out_left_buffer != NULL))
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

#ifdef INSTALL_METADATA
    /* Any input metadata should have been configured as 2 octets per sample by
     * the source. Refuse to start if metadata is present and this is not the case. */
    if (buff_has_metadata(decoder_data->codec.in_buffer))
    {
        if(NR_OF_OCTETS_IN_WORD(USE_16BIT_PER_WORD) != buff_metadata_get_usable_octets(decoder_data->codec.in_buffer))
        {
            base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            return TRUE;
        }
    }
#endif /* INSTALL_METADATA */

    op_data->state = OP_RUNNING;


    /* Hydra flushes the local and remote buffer at this point and then sets up
     * warping algorithm. This feels like latency control and rate-matching
     * which is to be done in endpoints. */
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
 bool a2dp_decode_buffer_details(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data)
{
    return a2dp_decode_buffer_details_core(op_data, message_data,response_id, response_data,A2DP_DECODE_INPUT_BUFFER_SIZE,A2DP_DECODE_OUTPUT_BUFFER_SIZE);
}


bool a2dp_decode_buffer_details_core(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data,unsigned inp_bufsize, unsigned out_bufsize)
{
#ifdef INSTALL_METADATA
    A2DP_DECODER_PARAMS *op_extra_data;
#endif
    OP_BUF_DETAILS_RSP *resp;
    unsigned terminal_id;

    if (!base_op_buffer_details(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    terminal_id = ((unsigned *)message_data)[0];
    resp = (OP_BUF_DETAILS_RSP*) *response_data;

    if ((terminal_id & TERMINAL_SINK_MASK) == TERMINAL_SINK_MASK)
    {
        /* return the sink buffer size */
        resp->b.buffer_size = inp_bufsize;
    }
    else
    {
        /* return the source buffer size */
        resp->b.buffer_size = MAX(out_bufsize, resp->b.buffer_size);
    }

#ifdef INSTALL_METADATA
    op_extra_data = (A2DP_DECODER_PARAMS*)op_data->extra_op_data;
    if ((terminal_id & TERMINAL_SINK_MASK) == TERMINAL_SINK_MASK)
    {
        if(op_extra_data->codec.in_buffer != NULL &&
            BUFF_METADATA(op_extra_data->codec.in_buffer))
        {
            resp->metadata_buffer = op_extra_data->codec.in_buffer;
        }
        else
        {
            resp->metadata_buffer = NULL;
        }
    }
    else
    {
        resp->metadata_buffer = op_extra_data->metadata_op_buffer;
    }
    resp->supports_metadata = TRUE;
#endif /* INSTALL_METADATA */

    return TRUE;
}

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
bool a2dp_decode_get_data_format(OPERATOR_DATA *op_data, void *message_data,
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
        /* The sink is 16 bit sbc encoded data */
        ((OP_STD_RSP*)*response_data)->resp_data.data = AUDIO_DATA_FORMAT_16_BIT;
    }
    else
    {
        /* The sources are audio samples */
        ((OP_STD_RSP*)*response_data)->resp_data.data = AUDIO_DATA_FORMAT_FIXP;
    }

    *response_id = OPCMD_DATA_FORMAT;

    return TRUE;
}


/**
 * \brief Returns the block size
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the get_block_size request message payload
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool a2dp_decode_get_sched_info(OPERATOR_DATA *op_data, void *message_data,
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


/* **************************** Operator message handlers ******************************** */

/**
 * \brief Enables a fadeout on the a2dp decoder output channels.
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the request message payload
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool a2dp_dec_opmsg_enable_fadeout(OPERATOR_DATA *op_data,
                    void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    A2DP_DECODER_PARAMS *decoder_data = (A2DP_DECODER_PARAMS *)(op_data->extra_op_data);

    /* Fade out only makes sense if we have an output buffer connected */
    if (decoder_data->codec.out_left_buffer == NULL)
    {
        return FALSE;
    }
    common_set_fadeout_state(&decoder_data->left_fadeout, RUNNING_STATE);

    /* If there is only a mono connection then this is done. */
    if (decoder_data->codec.out_right_buffer != NULL)
    {
        common_set_fadeout_state(&decoder_data->right_fadeout, RUNNING_STATE);
    }

    return TRUE;
}

/**
 * \brief Disables a fadeout on the a2dp decoder output channels.
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the request message payload
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool a2dp_dec_opmsg_disable_fadeout(OPERATOR_DATA *op_data,
                    void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    /* No need to check which terminals are connected as setting to
     * NOT_RUNNING_STATE is safe. */
    common_set_fadeout_state(&(((A2DP_DECODER_PARAMS*)(op_data->extra_op_data))->left_fadeout), NOT_RUNNING_STATE);
    common_set_fadeout_state(&(((A2DP_DECODER_PARAMS*)(op_data->extra_op_data))->right_fadeout), NOT_RUNNING_STATE);

    return TRUE;
}

bool a2dp_dec_assign_buffering(OPERATOR_DATA *op_data,
                    void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    ((A2DP_DECODER_PARAMS*)(op_data->extra_op_data))->play = (bool *)((uintptr_t)((unsigned*)(message_data))[3]);
    return TRUE;
}

#ifndef TIMED_PLAYBACK_MODE

/**
 * \brief Enable or disable ratematching.
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the request message payload
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 *
 * \Note The recieved message format is:
 *    word 0 - client id
 *    word 1 - length
 *    word 2 - op message id
 *    word 3 - boolean TRUE = enable ratematching, FALSE = disable
 *    word 4 - pointer to where the rateadjustment value will be written
 */
bool a2dp_dec_opmsg_ratematch_enacting(OPERATOR_DATA *op_data,
                    void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
   return a2dp_dec_opmsg_ratematch_enacting_core(op_data,message_data, A2DP_DECODE_OUTPUT_BUFFER_SIZE);
}

bool a2dp_dec_opmsg_ratematch_enacting_core(OPERATOR_DATA *op_data,
                    void *message_data, unsigned int bsize)
{
    A2DP_DECODER_PARAMS *decoder_data = (A2DP_DECODER_PARAMS *)(op_data->extra_op_data);

    if (op_data->state == OP_RUNNING)
    {
        /* Indicate failure */
        return FALSE;
    }

    /* Allocate a buffer that is big enough to contain a single frame. That is
     * buffer_size/ 2. There needs to show buffer_size/2 space when wrapped as a
     * cbuffer so we add 1 word.
     */
    unsigned buf_size = (bsize >>1) * sizeof(unsigned) + sizeof(unsigned);
    /* If this is an enable then prepare the SRA algorithm. If it's a disable
     * then tidy it away. */
    if ((bool)((unsigned *)message_data)[3])
    {
        /* reserve a left and right scratch buffer for the decoder output prior to
         * the SRA operation. */
        if (!scratch_reserve(buf_size, MALLOC_PREFERENCE_NONE)) /* Left channel */
        {
            fault_diatribe(FAULT_AUDIO_CODEC_RM_OUT_OF_RAM, op_data->id);
            return FALSE;
        }
        if (!scratch_reserve(buf_size, MALLOC_PREFERENCE_NONE)) /* Right channel */
        {
            scratch_release(buf_size, MALLOC_PREFERENCE_NONE); /* Left channel */
            fault_diatribe(FAULT_AUDIO_CODEC_RM_OUT_OF_RAM, op_data->id);
            return FALSE;
        }
        if (NULL != decoder_data->op_out_left)
        {
            /* The codec will use a scratch buffer. Until we commit the RAM it
             * doesn't have a pointer so we pass in NULL. */
            decoder_data->codec.out_left_buffer = cbuffer_create(NULL, buf_size, BUF_DESC_SW_BUFFER);
            if (decoder_data->codec.out_left_buffer == NULL)
            {
                decoder_data->codec.out_left_buffer = decoder_data->op_out_left;
                scratch_release(buf_size, MALLOC_PREFERENCE_NONE); /* Left channel */
                scratch_release(buf_size, MALLOC_PREFERENCE_NONE); /* Right channel */
                fault_diatribe(FAULT_AUDIO_CODEC_RM_OUT_OF_RAM, op_data->id);
                return FALSE;
            }
        }
        if (NULL != decoder_data->op_out_right)
        {
            /* The codec will use a scratch buffer. Until we commit the RAM it
             * doesn't have a pointer so we pass in NULL. */
            decoder_data->codec.out_right_buffer = cbuffer_create(NULL, buf_size, BUF_DESC_SW_BUFFER);
            if (decoder_data->codec.out_right_buffer == NULL)
            {
                decoder_data->codec.out_right_buffer = decoder_data->op_out_right;
                if (NULL != decoder_data->op_out_left)
                {
                    cbuffer_destroy_struct(decoder_data->codec.out_left_buffer);
                }
                decoder_data->codec.out_left_buffer = decoder_data->op_out_left;

                scratch_release(buf_size, MALLOC_PREFERENCE_NONE); /* Left channel */
                scratch_release(buf_size, MALLOC_PREFERENCE_NONE); /* Right channel */

                fault_diatribe(FAULT_AUDIO_CODEC_RM_OUT_OF_RAM, op_data->id);
                return FALSE;
            }
        }

        decoder_data->sra = (void *) sra_create(2, SW_RA_HIGH_QUALITY_COEFFS,
                            (unsigned *)(uintptr_t)((unsigned *)message_data)[4], 0);
    }
    else
    {
        /* Free any existing sra and buffers. */
        if (decoder_data->sra != NULL)
        {
            sra_destroy(decoder_data->sra);
            decoder_data->sra = NULL;
        }

        scratch_release(buf_size, MALLOC_PREFERENCE_NONE); /* Left channel */
        scratch_release(buf_size, MALLOC_PREFERENCE_NONE); /* Right channel */
        if (NULL != decoder_data->op_out_left)
        {
            /* Only free the cbuffer struct and not the underlying physical memory
             * that is managed using scratch_reserve and scratch_release.
             */
            cbuffer_destroy_struct(decoder_data->codec.out_left_buffer);
        }
        decoder_data->codec.out_left_buffer = decoder_data->op_out_left;
        if (NULL != decoder_data->op_out_right)
        {
            /* Only free the cbuffer struct and not the underlying physical memory
             * that is managed using scratch_reserve and scratch_release.
             */
            cbuffer_destroy_struct(decoder_data->codec.out_right_buffer);
        }
        decoder_data->codec.out_right_buffer = decoder_data->op_out_right;
    }

    return FALSE;
}

#endif /* TIMED_PLAYBACK_MODE */
