/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  wbs_dec.c
 * \ingroup  operators
 *
 *  WBS_DEC operator
 *
 */

/****************************************************************************
Include Files
*/
#include "wbs_private.h"
#include "wbs_dec_gen_c.h"
#include "op_msg_utilities.h"
#include "op_msg_helpers.h"

#include "patch/patch.h"

/****************************************************************************
Private Constant Definitions
*/

/****************************************************************************
Private Type Definitions
*/


/****************************************************************************
Private Constant Declarations
*/

/** The WBS decoder capability function handler table */
const handler_lookup_struct wbs_dec_handler_table =
{
    wbs_dec_create,           /* OPCMD_CREATE */
    wbs_dec_destroy,          /* OPCMD_DESTROY */
    wbs_dec_start,            /* OPCMD_START */
    base_op_stop,             /* OPCMD_STOP */
    wbs_dec_reset,            /* OPCMD_RESET */
    wbs_dec_connect,          /* OPCMD_CONNECT */
    wbs_dec_disconnect,       /* OPCMD_DISCONNECT */
    wbs_buffer_details,       /* OPCMD_BUFFER_DETAILS */
    wbs_dec_get_data_format,  /* OPCMD_DATA_FORMAT */
    wbs_dec_get_sched_info    /* OPCMD_GET_SCHED_INFO */
};

/* Null terminated operator message handler table */
const opmsg_handler_lookup_table_entry wbs_dec_opmsg_handler_table[] =
    {{OPMSG_COMMON_ID_GET_CAPABILITY_VERSION,      base_op_opmsg_get_capability_version},
    {OPMSG_COMMON_ID_FADEOUT_ENABLE,               wbs_dec_opmsg_enable_fadeout},
    {OPMSG_COMMON_ID_FADEOUT_DISABLE,              wbs_dec_opmsg_disable_fadeout},
    {OPMSG_WBS_DEC_ID_SET_FROM_AIR_INFO,           wbs_dec_opmsg_set_from_air_info},
#ifdef INSTALL_PLC100
    {OPMSG_WBS_DEC_ID_FORCE_PLC_OFF,               wbs_dec_opmsg_force_plc_off},
#endif /* INSTALL_PLC100 */
    {OPMSG_WBS_DEC_ID_FRAME_COUNTS,                wbs_dec_opmsg_frame_counts},
    {OPMSG_COMMON_ID_SET_CONTROL,                  wbs_dec_opmsg_obpm_set_control},
    {OPMSG_COMMON_ID_GET_PARAMS,                   wbs_dec_opmsg_obpm_get_params},
    {OPMSG_COMMON_ID_GET_DEFAULTS,                 wbs_dec_opmsg_obpm_get_defaults},
    {OPMSG_COMMON_ID_SET_PARAMS,                   wbs_dec_opmsg_obpm_set_params},
    {OPMSG_COMMON_ID_GET_STATUS,                   wbs_dec_opmsg_obpm_get_status},
#ifdef SCO_RX_OP_GENERATE_METADATA
    {OPMSG_COMMON_ID_SET_BUFFER_SIZE,             sco_common_rcv_opmsg_set_buffer_size},
    {OPMSG_COMMON_SET_TTP_LATENCY,                sco_common_rcv_opmsg_set_ttp_latency},
#endif
    {0, NULL}};


const CAPABILITY_DATA wbs_dec_cap_data =
    {
        WBS_DEC_CAP_ID,             /* Capability ID */
        WBS_DEC_WB_VERSION_MAJOR, 1,    /* Version information - hi and lo parts */
        1, 1,                           /* Max number of sinks/inputs and sources/outputs */
        &wbs_dec_handler_table,         /* Pointer to message handler function table */
        wbs_dec_opmsg_handler_table,    /* Pointer to operator message handler function table */
        wbs_dec_process_data,           /* Pointer to data processing function */
        0,                              /* TODO - Processing time information */
        sizeof(WBS_DEC_OP_DATA)         /* Size of capability-specific per-instance data */
    };

/** Memory owned by a decoder instance */
const malloc_t_entry wbs_dec_malloc_table[WBS_DEC_MALLOC_TABLE_LENGTH] =
{
    {30, MALLOC_PREFERENCE_NONE, offsetof(sbc_codec, wbs_frame_buffer_ptr)},
    {SBC_SYNTHESIS_BUFF_LENGTH, MALLOC_PREFERENCE_DM2, offsetof(sbc_codec, synthesis_vch1)}
};

const scratch_table wbs_dec_scratch_table =
{
    WBS_DM1_SCRATCH_TABLE_LENGTH,
    SBC_DEC_DM2_SCRATCH_TABLE_LENGTH,
    0,
    wbs_scratch_table_dm1,
    sbc_scratch_table_dm2,
    NULL
};

/****************************************************************************
Private Function Definitions
*/


/* ******************************* Helper functions ************************************ */

/* initialise various working data params of the specific operator */
static void wbs_dec_reset_working_data(OPERATOR_DATA *op_data)
{
    WBS_DEC_OP_DATA* x_data = (WBS_DEC_OP_DATA*)(op_data->extra_op_data);

    sco_common_rcv_reset_working_data(&((WBS_DEC_OP_DATA*)(op_data->extra_op_data))->sco_rcv_op_data);

    /* clear first valid packet info */
    x_data->received_first_valid_pkt = 0;
#ifdef SCO_DEBUG
    x_data->wbs_dec_dbg_stats_enable = 0;
#endif

    /* Now reset the decoder - re-using old but slightly massaged function in ASM */
    wbs_dec_reset_sbc_data(op_data);
}


/* free the memory allocated for SBC dec (shared and non-shared) */
static void wbs_dec_free_state_data(OPERATOR_DATA* op_data)
{
    WBS_DEC_OP_DATA* x_data = (WBS_DEC_OP_DATA*)(op_data->extra_op_data);

    if (x_data->codec_data != NULL)
    {
        /* free the shared codec data */
        mem_table_free_shared((void *)(x_data->codec_data),
                            wbs_sbc_shared_malloc_table, WBS_SBC_SHARED_TABLE_LENGTH);

        /* free shared decoder data */
        mem_table_free_shared((void *)(x_data->codec_data),
                    wbs_sbc_dec_shared_malloc_table, WBS_SBC_DEC_SHARED_TABLE_LENGTH);

        /* free non-shared memory */
        mem_table_free((void *)(x_data->codec_data), wbs_dec_malloc_table,
                                                WBS_DEC_MALLOC_TABLE_LENGTH);

        /* now free the codec data object */
        pdelete(x_data->codec_data);
        x_data->codec_data = NULL;
    }
#ifdef INSTALL_PLC100
    sco_common_rcv_destroy_plc_data(&(x_data->sco_rcv_op_data));
#endif /* INSTALL_PLC100 */
}



/* ********************************** API functions ************************************* */

/* TODO: a large part of this can be re-used from SCO RCV - so may move those out into a common helper function */
bool wbs_dec_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    WBS_DEC_OP_DATA* wbs_dec;
    bool new_allocation;

    /* call base_op create, which also allocates and fills response message */
    if(!base_op_create(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* operator buffer size - using the same default */
    wbs_dec = (WBS_DEC_OP_DATA*)op_data->extra_op_data;

#ifdef INSTALL_PLC100
    /* setup the PLC structure, zero initialise it so that we can detect if an
     * allocation has happened when we unwind */
    wbs_dec->sco_rcv_op_data.sco_rcv_parameters.plc100_struc = xzpnew(PLC100_STRUC);
    if(wbs_dec->sco_rcv_op_data.sco_rcv_parameters.plc100_struc == NULL)
    {
        /* Change the already allocated response to command failed. No extra error info. */
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    /* allocate speech buffer */
    wbs_dec->sco_rcv_op_data.sco_rcv_parameters.plc100_struc->speech_buf = xpnewn(SP_BUF_LEN_WB, int);
    if(wbs_dec->sco_rcv_op_data.sco_rcv_parameters.plc100_struc->speech_buf == NULL)
    {
        wbs_dec_free_state_data(op_data);
        /* Change the already allocated response to command failed. No extra error info. */
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }
    wbs_dec->sco_rcv_op_data.sco_rcv_parameters.plc100_struc->speech_buf_start =
                            wbs_dec->sco_rcv_op_data.sco_rcv_parameters.plc100_struc->speech_buf;

    /* allocate the ola buffer */
    wbs_dec->sco_rcv_op_data.sco_rcv_parameters.plc100_struc->ola_buf = xpnewn(OLA_LEN_WB, int);
    if(wbs_dec->sco_rcv_op_data.sco_rcv_parameters.plc100_struc->ola_buf == NULL)
    {
        wbs_dec_free_state_data(op_data);
        /* Change the already allocated response to command failed. No extra error info. */
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    /* initialise the rest of the structure */
    wbs_dec->sco_rcv_op_data.sco_rcv_parameters.plc100_struc->consts = get_plc100_constants(PLC100_WB_CONSTANTS);
    if(wbs_dec->sco_rcv_op_data.sco_rcv_parameters.plc100_struc->consts == NULL)
    {
        wbs_dec_free_state_data(op_data);
        /* Change the already allocated response to command failed. No extra error info. */
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }
    wbs_dec->sco_rcv_op_data.sco_rcv_parameters.plc100_struc->packet_len = WBS_DEC_DEFAULT_OUTPUT_BLOCK_SIZE;
#endif /* INSTALL_PLC100 */

    wbs_dec->md_bad_kick_attmpt_fake = 0;
    wbs_dec->md_bad_kick_faked = 0;

    /* Initialise some of the operator data that is common between NB and WB receive. It can only be called
     * after the PLC structure is allocated (if PLC present in build) */
    sco_common_rcv_initialise(&((WBS_DEC_OP_DATA*)(op_data->extra_op_data))->sco_rcv_op_data);

    /* create SBC data object */
    if((wbs_dec->codec_data = xzpnew(sbc_codec)) == NULL)
    {
        wbs_dec_free_state_data(op_data);
        /* Change the already allocated response to command failed. No extra error info. */
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    /* Share memory with decoders. */
    if( !mem_table_zalloc_shared((void *)(wbs_dec->codec_data), wbs_sbc_dec_shared_malloc_table,
            WBS_SBC_DEC_SHARED_TABLE_LENGTH, &new_allocation))
    {
        wbs_dec_free_state_data(op_data);
        /* Change the already allocated response to command failed. No extra error info. */
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    if( !mem_table_zalloc_shared((void *)(wbs_dec->codec_data), wbs_sbc_shared_malloc_table,
            WBS_SBC_SHARED_TABLE_LENGTH, &new_allocation))
    {
        wbs_dec_free_state_data(op_data);
        /* Change the already allocated response to command failed. No extra error info. */
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    /* now allocate the non-shareable memory */
    if(!mem_table_zalloc((void *)(wbs_dec->codec_data), wbs_dec_malloc_table,
                                                WBS_DEC_MALLOC_TABLE_LENGTH))
    {
        wbs_dec_free_state_data(op_data);
        /* Change the already allocated response to command failed. No extra error info. */
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    /* For future proofing, however this is one field in one block (and one field in opdata structure staring at force_plc_off) */
    if(!cpsInitParameters(&wbs_dec->sco_rcv_op_data.parms_def,(unsigned*)WBS_DEC_GetDefaults(op_data->cap_data->id),(unsigned*)&wbs_dec->sco_rcv_op_data.sco_rcv_parameters.force_plc_off,sizeof(WBS_DEC_PARAMETERS)))
    {
        wbs_dec_free_state_data(op_data);
        /* Change the already allocated response to command failed. No extra error info. */
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    /* Now reserve the scratch memory */
    if (scratch_register())
    {
        if (mem_table_scratch_tbl_reserve(&wbs_dec_scratch_table))
        {
            /* Successfully allocated everything! */
            /* initialise some more WBS decoder-specific data  */
            wbs_dec_reset_working_data(op_data);
            wbsdec_init_dec_param(op_data);

            return TRUE;
        }
        /* Fail free all the scratch memory we reserved */
        scratch_deregister();
    }
    /* Clear up all the allocated memory. */
    wbs_dec_free_state_data(op_data);
   /* Change the already allocated response to command failed. No extra error info. */
    base_op_change_response_status(response_data, STATUS_CMD_FAILED);
    return TRUE;
}


bool wbs_dec_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    if(base_op_destroy(op_data, message_data, response_id, response_data))
    {
        /* Free all the scratch memory we reserved */
        scratch_deregister();
        /* now destroy all the capability specific data */
        wbs_dec_free_state_data(op_data);
        return TRUE;
    }

    return FALSE;
}


bool wbs_dec_reset(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    if(!base_op_reset(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* now initialise specific working data */
    wbs_dec_reset_working_data(op_data);
    wbsdec_init_dec_param(op_data);
    return TRUE;
}


bool wbs_dec_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    WBS_DEC_OP_DATA *xdata = ((WBS_DEC_OP_DATA*)(op_data->extra_op_data));

    *response_id = OPCMD_START;

    /* Create the response. If there aren't sufficient resources for this fail
     * early. */
    if (!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }

    if (OP_RUNNING == op_data->state)
    {
        return TRUE;
    }

    /* Sanity check for buffers being connected.
     * We can't do much useful without */
    if (   xdata->sco_rcv_op_data.buffers.ip_buffer == NULL
        || xdata->sco_rcv_op_data.buffers.op_buffer == NULL)
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    /* clear first valid packet info */
    ((WBS_DEC_OP_DATA*)(op_data->extra_op_data))->received_first_valid_pkt = 0;

#ifdef SCO_RX_OP_GENERATE_METADATA
    /* connected endpoint tp sco rx endpoint is stored here, it's needed for its API call
     * to reset metadata when flushing buffer.
     */
    xdata->sco_rcv_op_data.sco_source_ep = stream_get_connected_endpoint_from_terminal_id(op_data->id, 0);
#endif

    op_data->state = OP_RUNNING;

    return TRUE;
}


bool wbs_dec_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
#ifdef INSTALL_PLC100

    unsigned terminal;
    WBS_DEC_OP_DATA *x_data = ((WBS_DEC_OP_DATA*)(op_data->extra_op_data));

    bool status = sco_common_connect(op_data, message_data, response_id, response_data,
            &(x_data->sco_rcv_op_data.buffers), &terminal);

    /* if connecting source terminal and previous steps were OK, hook up output buffer beauty with the PLC beast */
    if(status && (terminal == OUTPUT_TERMINAL_ID))
    {
        x_data->sco_rcv_op_data.sco_rcv_parameters.plc100_struc->output =
                                    x_data->sco_rcv_op_data.buffers.op_buffer;
    }

    return status;
#else
    return sco_common_connect(op_data, message_data, response_id, response_data,
            &(((WBS_DEC_OP_DATA*)(op_data->extra_op_data))->sco_rcv_op_data.buffers),
            NULL);
#endif /* INSTALL_PLC100 */
}


bool wbs_dec_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
#ifdef INSTALL_PLC100

    unsigned terminal;
    WBS_DEC_OP_DATA *x_data = ((WBS_DEC_OP_DATA*)(op_data->extra_op_data));

    bool status = sco_common_disconnect(op_data, message_data, response_id, response_data,
            &(x_data->sco_rcv_op_data.buffers), &terminal);

    /* if disconnecting source terminal and previous steps were OK, disconnect output buffer from PLC */
    if(status && (terminal == OUTPUT_TERMINAL_ID))
    {
        x_data->sco_rcv_op_data.sco_rcv_parameters.plc100_struc->output = NULL;
    }

    return status;
#else
    return sco_common_disconnect(op_data, message_data, response_id, response_data,
                &(((WBS_DEC_OP_DATA*)(op_data->extra_op_data))->sco_rcv_op_data.buffers),
                NULL);
#endif /* INSTALL_PLC100 */
}

bool wbs_dec_get_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id,
        void **response_data)
{
    return sco_common_get_data_format(op_data, message_data, response_id, response_data,
            AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP_WITH_METADATA, AUDIO_DATA_FORMAT_FIXP);
}

bool wbs_dec_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id,
        void **response_data)
{
    return sco_common_get_sched_info(op_data, message_data, response_id, response_data,
            WBS_DEC_DEFAULT_INPUT_BLOCK_SIZE, WBS_DEC_DEFAULT_OUTPUT_BLOCK_SIZE);
}

/* ************************************* Data processing-related functions and wrappers **********************************/

void wbs_dec_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    unsigned status;
    WBS_DEC_OP_DATA* x_data;
    SCO_COMMON_RCV_OP_DATA *sco_data;

    patch_fn(wbs_decode_process_data);

    x_data = (WBS_DEC_OP_DATA*)(op_data->extra_op_data);
    sco_data = &x_data->sco_rcv_op_data;

    /* Commit any scratch memory ideally this should be done later after the
     * decision to decode is made. */
    mem_table_scratch_tbl_commit(x_data->codec_data, &wbs_dec_scratch_table);

    /* We can't have started in this state, so if we lose a buffer carry on
     * and hope that it comes back or we are stopped. If not then the error
     * (lack of data) should propagate to something that cares. */
    if (sco_data->buffers.ip_buffer == NULL || sco_data->buffers.op_buffer == NULL)
    {
        return;
    }

    /* call ASM function */
    status = wbs_dec_processing(op_data);

    /* Free the scratch memory used */
    scratch_free();

    /* Is fadeout enabled? if yes, do it on the current output data, if processing has actually produced output.
     * Now the slight migraine is that deep inside decoder, it may have decided to decode two frames - so the last guy that
     * really had to know the final outcome of how many samples were produced is PLC. Its packet size after return will be
     * the most reliable indicator of how many samples we need to process. If PLC is not installed, then WBS validate() function
     * is the only thing that tells the real story - of course, without PLC this would work in a very funny way.
     */
    if( (sco_data->fadeout_parameters.fadeout_state != NOT_RUNNING_STATE) && status)
    {
        /* the wrapper below takes output Cbuffer and fadeout params, and the current packet size in words is from PLC */
        if(mono_cbuffer_fadeout(sco_data->buffers.op_buffer,
                                x_data->wbs_dec_output_samples,
                                &(sco_data->fadeout_parameters)))
        {
            common_send_simple_unsolicited_message(op_data, OPMSG_REPLY_ID_FADEOUT_DONE);
        }
    }

    touched->sources = status;
}


/* **************************** Operator message handlers ******************************** */


bool wbs_dec_opmsg_enable_fadeout(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    common_set_fadeout_state(&(((WBS_DEC_OP_DATA*)(op_data->extra_op_data))->sco_rcv_op_data.fadeout_parameters), RUNNING_STATE);

    return TRUE;
}

bool wbs_dec_opmsg_disable_fadeout(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    common_set_fadeout_state(&(((WBS_DEC_OP_DATA*)(op_data->extra_op_data))->sco_rcv_op_data.fadeout_parameters), NOT_RUNNING_STATE);

    return TRUE;
}


/* TODO: This function can be lifted into a sco_fw "sco_common.c" or similar, provided that wbs_dec_build_simple_response
 * is also lifted out there. The trick is that the extra op data parts it refers to are actually identical between NB and WB
 * operator data structs intentionally for such situations!
 * Therefore WB and/or NB SCO can make use of this as common sco helper function.
 */
bool wbs_dec_opmsg_set_from_air_info(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    return sco_common_rcv_set_from_air_info_helper(op_data, &(((WBS_DEC_OP_DATA*)(op_data->extra_op_data))->sco_rcv_op_data),
                                                   message_data);
}

#ifdef INSTALL_PLC100
bool wbs_dec_opmsg_force_plc_off(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    return sco_common_rcv_force_plc_off_helper(&(((WBS_DEC_OP_DATA*)(op_data->extra_op_data))->sco_rcv_op_data),
                                               message_data);
}
#endif /* INSTALL_PLC100 */

bool wbs_dec_opmsg_frame_counts(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    return sco_common_rcv_frame_counts_helper(&(((WBS_DEC_OP_DATA*)(op_data->extra_op_data))->sco_rcv_op_data),
                                              message_data, resp_length, resp_data);

}

bool wbs_dec_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    /* In the case of this capability, nothing is done for control message. Just follow protocol and ignore any content. */
    return cps_control_setup(message_data, resp_length, resp_data,NULL);
}


bool wbs_dec_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
#ifdef INSTALL_PLC100
   WBS_DEC_OP_DATA* wbs_dec = (WBS_DEC_OP_DATA*)op_data->extra_op_data;

   return cpsGetParameterMsgHandler(&wbs_dec->sco_rcv_op_data.parms_def,message_data, resp_length,resp_data);

#else
    return FALSE;
#endif /* INSTALL_PLC100 */
}

bool wbs_dec_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
   WBS_DEC_OP_DATA* wbs_dec = (WBS_DEC_OP_DATA*)op_data->extra_op_data;

   return cpsGetDefaultsMsgHandler(&wbs_dec->sco_rcv_op_data.parms_def,message_data, resp_length,resp_data);
}

bool wbs_dec_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    /* Set the parameter(s). For future proofing, it is using the whole mechanism, although currently there is only one field
     * in opdata structure that is a setable parameter. If later there will be more (ever), must follow contiquously the first field,
     * as commented and instructed in the op data definition. Otherwise consider moving them into a dedicated structure.
     */
#ifdef INSTALL_PLC100
   WBS_DEC_OP_DATA* wbs_dec = (WBS_DEC_OP_DATA*)op_data->extra_op_data;

   return cpsSetParameterMsgHandler(&wbs_dec->sco_rcv_op_data.parms_def,message_data, resp_length,resp_data);
#else
    return FALSE;
#endif /* INSTALL_PLC100 */
}

bool wbs_dec_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    WBS_DEC_OP_DATA* wbs_dec_params = (WBS_DEC_OP_DATA*)op_data->extra_op_data;
    SCO_RCV_PARAMS* sco_rcv_params = &wbs_dec_params->sco_rcv_op_data.sco_rcv_parameters;
    unsigned* resp;

    if(!common_obpm_status_helper(message_data,resp_length,resp_data,sizeof(WBS_DEC_STATISTICS),&resp))
    {
         return FALSE;
    }

    /* Fill the statistics as needed */
    if(resp)
    {
        resp = cpsPackWords(&sco_rcv_params->sco_pkt_size, &sco_rcv_params->t_esco, resp);
        resp = cpsPackWords(&sco_rcv_params->frame_count, &sco_rcv_params->frame_error_count, resp);
        resp = cpsPackWords(&sco_rcv_params->md_late_pkts, &sco_rcv_params->md_early_pkts, resp);
        resp = cpsPackWords(&sco_rcv_params->out_of_time_pkt_cnt, &sco_rcv_params->md_out_of_time_reset, resp);
        resp = cpsPackWords(&wbs_dec_params->wbs_dec_no_output, &wbs_dec_params->wbs_dec_fake_pkt, resp);
        resp = cpsPackWords(&wbs_dec_params->wbs_dec_good_output, &wbs_dec_params->wbs_dec_bad_output, resp);
    }

    return TRUE;
}

