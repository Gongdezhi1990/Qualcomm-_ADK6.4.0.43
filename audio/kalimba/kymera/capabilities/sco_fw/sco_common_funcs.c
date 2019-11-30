/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  sco_common_funcs.c
 * \ingroup  capabilities
 *
 *  Common functions, used by NB and WB SCO capabilities.
 *  Functions "sco_common_rcv_..." are for receive capabilities (SCO_RCV & WBS_DEC).
 *  "sco_common_send_..." are for send capabilities (SCO_SEND, WBS_ENC).
 *  "sco_common_..." are for any SCO capability.
 *
 */

#include "capabilities.h"
#include "sco_struct.h"
#include "sco_common_funcs.h"

#include "patch/patch.h"
#ifdef SCO_RX_OP_GENERATE_METADATA
#include "ttp/ttp.h"
#endif
/****************************************************************************
Public Function Declarations
*/
#ifdef INSTALL_PLC100
extern void plc100_initialize(PLC100_STRUC*);
#endif

/****************************************************************************
 * Private Function Definitions
 */

#if defined(INSTALL_AUDIO_LOG) && defined(SCO_DEBUG)
static void sco_fw_sco_info_msg_set(uint16 opid, unsigned int tesco, uint16 first_TS, unsigned int pkt_size)
{
    L2_DBG_MSG4("Capid 0x%04x tesco %d firstTS 0x%04x pktsize %d", opid, tesco, first_TS, pkt_size);
}
#endif

/****************************************************************************
Public Function Definitions
*/


/* connect terminal to a buffer */
bool sco_common_connect(OPERATOR_DATA *op_data, void *message_data,
        unsigned *response_id, void **response_data, SCO_TERMINAL_BUFFERS *bufs,
        unsigned *terminal)
{

    unsigned terminal_id = ((unsigned*)message_data)[0];

    patch_fn(sco_common_connect);


    /* If the terminal number was requested then return it. */
    if (NULL != terminal)
    {
        *terminal = terminal_id;
    }

    *response_id = OPCMD_CONNECT;
    /* Create the response. If there aren't sufficient resources for this fail
     * early. */
    if (!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }

    switch (terminal_id)
    {
        case INPUT_TERMINAL_ID:
        {
            if (bufs->ip_buffer == NULL)
            {
                bufs->ip_buffer = (tCbuffer*)(((uintptr_t *)message_data)[1]);
            }
            else
            {
            	base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            }
            break;
        }
        case OUTPUT_TERMINAL_ID:
        {
            if (bufs->op_buffer == NULL)
            {
                bufs->op_buffer = (tCbuffer*)(((uintptr_t *)message_data)[1]);
            }
            else
            {
            	base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            }
            break;
        }
        default:
        {
        	base_op_change_response_status(response_data, STATUS_INVALID_CMD_PARAMS);
            break;
        }
    }

    return TRUE;
}

/* disconnect terminal from a buffer */
bool sco_common_disconnect(OPERATOR_DATA *op_data, void *message_data,
        unsigned *response_id, void **response_data, SCO_TERMINAL_BUFFERS *bufs,
        unsigned *terminal)
{

    unsigned terminal_id = ((unsigned*)message_data)[0];

    patch_fn(sco_common_disconnect);

    /* If the terminal number was requested then return it. */
    if (NULL != terminal)
    {
        *terminal = terminal_id;
    }

    *response_id = OPCMD_CONNECT;

    /* Create the response. If there aren't sufficient resources for this fail
     * early. */
    if (!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }

    switch (terminal_id)
    {
        case INPUT_TERMINAL_ID:
        {
            if (bufs->ip_buffer != NULL)
            {
                bufs->ip_buffer = NULL;
                /* If one or more terminals are disconnected the sco capabilities
                 * can't do anything useful so transition to stopping state to
                 * save MIPS.*/
                op_data->state = OP_NOT_RUNNING;
            }
            else
            {
            	base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            }
            break;
        }
        case OUTPUT_TERMINAL_ID:
        {
            if (bufs->op_buffer != NULL)
            {
                bufs->op_buffer = NULL;
                /* If one or more terminals are disconnected the sco capabilities
                 * can't do anything useful so transition to stopping state to
                 * save MIPS.*/
                op_data->state = OP_NOT_RUNNING;
            }
            else
            {
            	base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            }
            break;
        }
        default:
        {
        	base_op_change_response_status(response_data, STATUS_INVALID_CMD_PARAMS);
            break;
        }
    }

    return TRUE;
}

/* initialise some common parts of SCO operator data during creation - it trusts that everything referenced here was allocated before call */
void sco_common_rcv_initialise(SCO_COMMON_RCV_OP_DATA* sco_rcv_op_data)
{
    patch_fn(sco_common_rcv_initialise);

    // zero packet counts
    sco_rcv_op_data->sco_rcv_parameters.frame_count = 0;
    sco_rcv_op_data->sco_rcv_parameters.frame_error_count = 0;
#ifdef INSTALL_PLC100
    // PLC should be ON
    sco_rcv_op_data->sco_rcv_parameters.force_plc_off = 0;
    sco_rcv_op_data->sco_rcv_parameters.plc100_struc->per_threshold = PLC100_PER_THRESHOLD;
#endif /* INSTALL_PLC100 */

    // Initialise metadata handling fields and debug counters
    sco_rcv_op_data->sco_rcv_parameters.sco_pkt_size = 0;
    sco_rcv_op_data->sco_rcv_parameters.t_esco = 0;
    sco_rcv_op_data->sco_rcv_parameters.out_of_time_pkt_cnt = 0;
    sco_rcv_op_data->sco_rcv_parameters.exp_pkts = 0;
    sco_rcv_op_data->sco_rcv_parameters.expected_time_stamp = -1;
    sco_rcv_op_data->sco_rcv_parameters.num_bad_kicks = 0;
#ifdef INSTALL_PLC100
    sco_rcv_op_data->sco_rcv_parameters.md_pkt_faked = 0;
#endif

    sco_rcv_op_data->sco_rcv_parameters.md_num_kicks = 0;
    sco_rcv_op_data->sco_rcv_parameters.md_process_errors = 0;
    sco_rcv_op_data->sco_rcv_parameters.md_pkt_size_changed = 0;
    sco_rcv_op_data->sco_rcv_parameters.md_late_pkts = 0;
    sco_rcv_op_data->sco_rcv_parameters.md_early_pkts = 0;
    sco_rcv_op_data->sco_rcv_parameters.md_out_of_time_reset = 0;

    /* output buffer size, can be configured later to a larger size */
    sco_rcv_op_data->sco_rcv_parameters.output_buffer_size = SCO_DEFAULT_SCO_BUFFER_SIZE;

#ifdef SCO_RX_OP_GENERATE_METADATA
    /* these fields should already be cleared
     * at creation time, however we enforce it in
     * case creation method changed later.
     */
    sco_rcv_op_data->last_tag.valid = FALSE;
    sco_rcv_op_data->last_tag.time_offset = 0;
    sco_rcv_op_data->sco_rcv_parameters.generate_timestamp = FALSE;
#endif
}

/* get rid of PLC data structures and NULLify the PLC struct pointer */
#ifdef INSTALL_PLC100
void sco_common_rcv_destroy_plc_data(SCO_COMMON_RCV_OP_DATA* sco_rcv_op_data)
{
    patch_fn(sco_common_rcv_destroy_plc_data);

    // free the PLC data if PLC was used
    if(sco_rcv_op_data->sco_rcv_parameters.plc100_struc != NULL)
    {
        if (sco_rcv_op_data->sco_rcv_parameters.plc100_struc->ola_buf != NULL)
        {
            pdelete(sco_rcv_op_data->sco_rcv_parameters.plc100_struc->ola_buf);
        }
        if (sco_rcv_op_data->sco_rcv_parameters.plc100_struc->speech_buf_start != NULL)
        {
            pdelete(sco_rcv_op_data->sco_rcv_parameters.plc100_struc->speech_buf_start);
        }
        if (sco_rcv_op_data->sco_rcv_parameters.plc100_struc->consts != NULL)
        {
            release_plc100_constants(sco_rcv_op_data->sco_rcv_parameters.plc100_struc->consts);
        }
        pdelete(sco_rcv_op_data->sco_rcv_parameters.plc100_struc);
        sco_rcv_op_data->sco_rcv_parameters.plc100_struc = NULL;
    }
}
#endif /* INSTALL_PLC100 */


/* Initialise various working data params of the NB or WB SCO receive operators. */
bool sco_common_rcv_reset_working_data(SCO_COMMON_RCV_OP_DATA* sco_rcv_op_data)
{
    patch_fn(sco_common_rcv_reset_working_data);

    if(sco_rcv_op_data != NULL)
    {
        /* Initialise fadeout-related parameters */
        sco_rcv_op_data->fadeout_parameters.fadeout_state = NOT_RUNNING_STATE;
        sco_rcv_op_data->fadeout_parameters.fadeout_counter = 0;
        sco_rcv_op_data->fadeout_parameters.fadeout_flush_count = 0;

#ifdef INSTALL_PLC100
        /* reset PLC attenuation value and initialise PLC */
        sco_rcv_op_data->sco_rcv_parameters.plc100_struc->attenuation = PLC100_INITIAL_ATTENUATION;

        plc100_initialize(sco_rcv_op_data->sco_rcv_parameters.plc100_struc);
#endif /* INSTALL_PLC100 */
    }

    return TRUE;
}


bool sco_common_rcv_set_from_air_info_helper(OPERATOR_DATA *op_data, SCO_COMMON_RCV_OP_DATA* sco_rcv_op_data, void *message_data)
{
    patch_fn(sco_common_rcv_set_from_air_info_helper);

    /* read the from air length, which is in words and convert to octets */
    sco_rcv_op_data->sco_rcv_parameters.sco_pkt_size = ((unsigned*)message_data)[3] << 1;

    /* expected packets */
    sco_rcv_op_data->sco_rcv_parameters.exp_pkts = ((unsigned*)message_data)[4];

    /* T_esco */
    sco_rcv_op_data->sco_rcv_parameters.t_esco = ((unsigned*)message_data)[5];

    /* nuke the expected time stamp */
    sco_rcv_op_data->sco_rcv_parameters.expected_time_stamp = -1;

#if defined(SCO_DEBUG) && defined(INSTALL_AUDIO_LOG)
    sco_fw_sco_info_msg_set((uint16)op_data->cap_data->id, sco_rcv_op_data->sco_rcv_parameters.t_esco,
                             (uint16)sco_rcv_op_data->sco_rcv_parameters.expected_time_stamp,
                             sco_rcv_op_data->sco_rcv_parameters.sco_pkt_size);
#endif

    return TRUE;
}


#ifdef INSTALL_PLC100
bool sco_common_rcv_force_plc_off_helper(SCO_COMMON_RCV_OP_DATA* sco_rcv_op_data, void *message_data)
{
        patch_fn(sco_common_rcv_force_plc_off_helper);

    if(((unsigned*)message_data)[3] != 0)
    {
        /* set force PLC off */
        sco_rcv_op_data->sco_rcv_parameters.force_plc_off = 1;
    }
    else
    {
        /* unset force PLC off */
        sco_rcv_op_data->sco_rcv_parameters.force_plc_off = 0;
    }

    return TRUE;
}
#endif /* INSTALL_PLC100 */


bool sco_common_rcv_frame_counts_helper(SCO_COMMON_RCV_OP_DATA* sco_rcv_op_data, void *message_data,
                                         unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    patch_fn(sco_common_rcv_frame_counts_helper);

    if(((unsigned*)message_data)[3] != 0)
    {
        /* get the counts - length is payload (4 words) plus echoed msgID/keyID */
        *resp_length = OPMSG_RSP_PAYLOAD_SIZE_RAW_DATA(4);
        *resp_data = (OP_OPMSG_RSP_PAYLOAD *)xpnewn(*resp_length, unsigned);
        if (*resp_data == NULL)
        {
            return FALSE;
        }

        /* echo the opmsgID/keyID - 3rd field in the message_data */
        (*resp_data)->msg_id = OPMGR_GET_OPCMD_MESSAGE_MSG_ID((OPMSG_HEADER*)message_data);

        // get the frame counts
        (*resp_data)->u.raw_data[0] = sco_rcv_op_data->sco_rcv_parameters.frame_count >> 16;
        (*resp_data)->u.raw_data[1] = sco_rcv_op_data->sco_rcv_parameters.frame_count & 0xFFFF;
        (*resp_data)->u.raw_data[2] = sco_rcv_op_data->sco_rcv_parameters.frame_error_count >> 16;
        (*resp_data)->u.raw_data[3] = sco_rcv_op_data->sco_rcv_parameters.frame_error_count & 0xFFFF;
    }
    else
    {
        /* set counts to zero */
        sco_rcv_op_data->sco_rcv_parameters.frame_count = 0;
        sco_rcv_op_data->sco_rcv_parameters.frame_error_count = 0;
    }

    return TRUE;
}


bool sco_common_get_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id,
        void **response_data, AUDIO_DATA_FORMAT input_format, AUDIO_DATA_FORMAT output_format)
{
    patch_fn(sco_common_get_data_format);

    if(!base_op_get_data_format(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* return the terminal's data format - since we are mono-to-mono, easy to get to terminal data purely based on direction flag */
    if((((unsigned*)message_data)[0] & TERMINAL_SINK_MASK) == 0)
    {
        ((OP_STD_RSP*)*response_data)->resp_data.data = output_format;
    }
    else
    {
        ((OP_STD_RSP*)*response_data)->resp_data.data = input_format;
    }

    return TRUE;
}


bool sco_common_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id,
        void **response_data, unsigned input_block_size, unsigned output_block_size)
{

    OP_SCHED_INFO_RSP* resp;

    patch_fn(sco_common_get_sched_info);


    if (!base_op_get_sched_info(op_data, message_data, response_id, response_data))
    {
        return base_op_build_std_response(STATUS_CMD_FAILED, op_data->id, response_data);
    }

    /* Populate the response*/
    base_op_change_response_status(response_data, STATUS_OK);
    resp = *response_data;
    resp->op_id = op_data->id;
    /* return the terminal's block size - since we are mono-to-mono, easy to get to terminal data purely based on direction flag */
    if((((unsigned*)message_data)[0] & TERMINAL_SINK_MASK) == 0)
    {
        resp->block_size = output_block_size;
    }
    else
    {
        resp->block_size = input_block_size;;
    }
    resp->run_period = 0;

    *response_data = resp;
    return TRUE;
}

#ifdef SCO_RX_OP_GENERATE_METADATA
/**
 * sco_common_rcv_opmsg_set_ttp_latency
 * \brief sets the rcv op to generate timestamp tags instead of default toa
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the start request message
 * \param resp_length pointer to location to write the response message length
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool sco_common_rcv_opmsg_set_ttp_latency(OPERATOR_DATA *op_data, void *message_data,
                                     unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SCO_COMMON_RCV_OP_DATA *sco_common_rcv_data =
        (SCO_COMMON_RCV_OP_DATA *) (op_data->extra_op_data);

    /* We cant change this setting while running */
    if (OP_RUNNING == op_data->state)
    {
        return FALSE;
    }

    /* configure the latency, once set it cannot go back to toa mode,
     * but the latency can change  while the operator isn't running
     */
    sco_common_rcv_data->sco_rcv_parameters.generate_timestamp = TRUE;
    sco_common_rcv_data->sco_rcv_parameters.timestamp_latency = ttp_get_msg_latency(message_data);

    L4_DBG_MSG2("sco rcv output ttp latency set, op=%d, latency=%d", op_data->id, sco_common_rcv_data->sco_rcv_parameters.timestamp_latency);
    return TRUE;
}

/**
 * sco_common_rcv_opmsg_set_buffer_size
 * \brief message handler to set required sco rcv output buffer size
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the start request message
 * \param resp_length pointer to location to write the response message length
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool sco_common_rcv_opmsg_set_buffer_size(OPERATOR_DATA *op_data, void *message_data,
                                          unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{

    SCO_COMMON_RCV_OP_DATA *sco_common_rcv_data =
        (SCO_COMMON_RCV_OP_DATA *) (op_data->extra_op_data);

    /* We cant change this setting while running */
    if (OP_RUNNING == op_data->state)
    {
        return FALSE;
    }

    /* set the buffer size, it will only be used for PCM terminals */
    sco_common_rcv_data->sco_rcv_parameters.output_buffer_size = OPMSG_FIELD_GET(message_data, OPMSG_COMMON_SET_BUFFER_SIZE, BUFFER_SIZE);

    L4_DBG_MSG2("sco rcv output buffer size, op=%d, size=%d", op_data->id, sco_common_rcv_data->sco_rcv_parameters.output_buffer_size);

    return TRUE;

}
#endif



