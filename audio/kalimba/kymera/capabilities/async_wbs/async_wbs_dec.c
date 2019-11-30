/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  async_wbs_dec.c
 * \ingroup  operators
 *
 *  ASYNC_WBS_DEC operator
 *
 */

/****************************************************************************
Include Files
*/
#include "async_wbs_private.h"
#include "wbs.h"
#include "wbs/wbs_dec_gen_c.h"
#include "op_msg_utilities.h"
#include "capabilities.h"
#include "sbc_encode/sbc_encode.h"
#include "patch/patch.h"

/****************************************************************************
Private Constant Definitions
*/
#define MAX_COUNT_WBS_DEC_LOOP  3

/****************************************************************************
Private Type Definitions
*/


/****************************************************************************
Private Constant Declarations
*/

/** The WBS decoder capability function handler table */
const handler_lookup_struct async_wbs_dec_handler_table =
{
    async_wbs_dec_create,           /* OPCMD_CREATE */
    async_wbs_dec_destroy,          /* OPCMD_DESTROY */
    wbs_dec_start,            /* OPCMD_START */
    base_op_stop,             /* OPCMD_STOP */
    async_wbs_dec_reset,            /* OPCMD_RESET */
    wbs_dec_connect,          /* OPCMD_CONNECT */
    wbs_dec_disconnect,       /* OPCMD_DISCONNECT */
    async_wbs_dec_buffer_details, /* OPCMD_BUFFER_DETAILS */
    async_wbs_dec_get_data_format,  /* OPCMD_DATA_FORMAT */
    wbs_dec_get_sched_info    /* OPCMD_GET_SCHED_INFO */
};

/* Null terminated operator message handler table */
const opmsg_handler_lookup_table_entry async_wbs_dec_opmsg_handler_table[] =
    {{OPMSG_COMMON_ID_GET_CAPABILITY_VERSION,      base_op_opmsg_get_capability_version},
    {OPMSG_COMMON_ID_FADEOUT_ENABLE,               wbs_dec_opmsg_enable_fadeout},
    {OPMSG_COMMON_ID_FADEOUT_DISABLE,              wbs_dec_opmsg_disable_fadeout},
    {OPMSG_ASYNC_WBS_DEC_ID_SET_FROM_AIR_INFO,           wbs_dec_opmsg_set_from_air_info},
#ifdef INSTALL_PLC100
    {OPMSG_ASYNC_WBS_DEC_ID_FORCE_PLC_OFF,               wbs_dec_opmsg_force_plc_off},
#endif /* INSTALL_PLC100 */
    {OPMSG_ASYNC_WBS_DEC_ID_FRAME_COUNTS,                wbs_dec_opmsg_frame_counts},
    {OPMSG_ASYNC_WBS_DEC_ID_SET_BITPOOL_VALUE,     awbs_dec_opmsg_set_encoding_params},
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



const CAPABILITY_DATA async_wbs_dec_cap_data =
    {
        ASYNC_WBS_DEC_CAP_ID,             /* Capability ID */
        1, 2,    /* Version information - hi and lo parts */
        1, 1,                           /* Max number of sinks/inputs and sources/outputs */
        &async_wbs_dec_handler_table,         /* Pointer to message handler function table */
        async_wbs_dec_opmsg_handler_table,    /* Pointer to operator message handler function table */
        async_wbs_dec_process_data,           /* Pointer to data processing function */
        0,                              /* TODO - Processing time information */
        sizeof(ASYNC_WBS_DEC_OP_DATA)         /* Size of capability-specific per-instance data */
    };

/****************************************************************************
Private Function Definitions
*/
static inline ASYNC_WBS_DEC_OP_DATA *get_instance_data(OPERATOR_DATA *op_data)
{
    return (ASYNC_WBS_DEC_OP_DATA *) op_data->extra_op_data;
}

/* ******************************* Helper functions ************************************ */

/* initialise various working data params of the specific operator */
static void async_wbs_dec_reset_working_data(OPERATOR_DATA *op_data)
{
    ASYNC_WBS_DEC_OP_DATA* x_data = get_instance_data(op_data);

    sco_common_rcv_reset_working_data(&x_data->wbs.sco_rcv_op_data);

    x_data->init_phase = AWBSD_NOT_INITIALIZED;

    /* clear first valid packet info */
    x_data->wbs.received_first_valid_pkt = 0;
#ifdef SCO_DEBUG
    x_data->wbs.wbs_dec_dbg_stats_enable = 0;
#endif

    /* Now reset the decoder - re-using old but slightly massaged function in ASM */
    wbs_dec_reset_sbc_data(op_data);
}


/* free the memory allocated for SBC dec (shared and non-shared) */
static void async_wbs_dec_free_state_data(OPERATOR_DATA* op_data)
{
    ASYNC_WBS_DEC_OP_DATA* x_data = get_instance_data(op_data);

    if (x_data->wbs.codec_data != NULL)
    {
        /* free the shared codec data */
        mem_table_free_shared((void *)(x_data->wbs.codec_data),
                            wbs_sbc_shared_malloc_table, WBS_SBC_SHARED_TABLE_LENGTH);

        /* free shared decoder data */
        mem_table_free_shared((void *)(x_data->wbs.codec_data),
                    wbs_sbc_dec_shared_malloc_table, WBS_SBC_DEC_SHARED_TABLE_LENGTH);

        /* free non-shared memory */
        mem_table_free((void *)(x_data->wbs.codec_data), wbs_dec_malloc_table,
                                                WBS_DEC_MALLOC_TABLE_LENGTH);

        /* now free the codec data object */
        pdelete(x_data->wbs.codec_data);
        x_data->wbs.codec_data = NULL;
    }
#ifdef INSTALL_PLC100
    sco_common_rcv_destroy_plc_data(&(x_data->wbs.sco_rcv_op_data));
#endif /* INSTALL_PLC100 */
}


/* ********************************** API functions ************************************* */


bool async_wbs_dec_get_data_format(OPERATOR_DATA *op_data, void *message_data,
        unsigned *response_id, void **response_data)
{
    return sco_common_get_data_format(op_data, message_data, response_id, response_data,
            AUDIO_DATA_FORMAT_16_BIT_WITH_METADATA, AUDIO_DATA_FORMAT_FIXP);
}

bool async_wbs_dec_create(OPERATOR_DATA *op_data, void *message_data,
        unsigned *response_id, void **response_data)
{
    ASYNC_WBS_DEC_OP_DATA* x_data;
    WBS_DEC_OP_DATA* wbs_dec;
    bool new_allocation;

    /* call base_op create, which also allocates and fills response message */
    if(!base_op_create(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    x_data = get_instance_data(op_data);
    x_data->init_phase = AWBSD_NOT_INITIALIZED;
    wbs_dec = &x_data->wbs;

#ifdef INSTALL_PLC100
    /* setup the PLC structure, zero initialise it so that we can detect if an
     * allocation has happened when we unwind */
    PLC100_STRUC* plcstruct = xzpnew(PLC100_STRUC);
    if(plcstruct == NULL)
    {
        /* Change the already allocated response to command failed. No extra error info. */
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    /* allocate speech buffer */
    plcstruct->speech_buf = xpnewn(SP_BUF_LEN_WB, int);
    if(plcstruct->speech_buf == NULL)
    {
        async_wbs_dec_free_state_data(op_data);
        /* Change the already allocated response to command failed. No extra error info. */
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }
    plcstruct->speech_buf_start = plcstruct->speech_buf;

    /* allocate the ola buffer */
    plcstruct->ola_buf = xpnewn(OLA_LEN_WB, int);
    if(plcstruct->ola_buf == NULL)
    {
        async_wbs_dec_free_state_data(op_data);
        /* Change the already allocated response to command failed. No extra error info. */
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    /* initialise the rest of the structure */
    plcstruct->consts = get_plc100_constants(PLC100_WB_CONSTANTS);
    if(plcstruct->consts == NULL)
    {
        async_wbs_dec_free_state_data(op_data);
        /* Change the already allocated response to command failed. No extra error info. */
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }
    plcstruct->packet_len = WBS_DEC_DEFAULT_OUTPUT_BLOCK_SIZE;

    wbs_dec->sco_rcv_op_data.sco_rcv_parameters.plc100_struc = plcstruct;
#endif /* INSTALL_PLC100 */

    wbs_dec->md_bad_kick_attmpt_fake = 0;
    wbs_dec->md_bad_kick_faked = 0;

    /* Initialise some of the operator data that is common between NB and WB receive. It can only be called
     * after the PLC structure is allocated (if PLC present in build) */
    sco_common_rcv_initialise(&wbs_dec->sco_rcv_op_data);

    wbs_dec->sco_rcv_op_data.sco_rcv_parameters.t_esco = ASYNC_WBS_DEFAULT_TESCO;

    /* create SBC data object */
    if((wbs_dec->codec_data = xzpnew(sbc_codec)) == NULL)
    {
        async_wbs_dec_free_state_data(op_data);
        /* Change the already allocated response to command failed. No extra error info. */
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    /* Share memory with decoders. */
    if( !mem_table_zalloc_shared((void *)(wbs_dec->codec_data), wbs_sbc_dec_shared_malloc_table,
            WBS_SBC_DEC_SHARED_TABLE_LENGTH, &new_allocation))
    {
        async_wbs_dec_free_state_data(op_data);
        /* Change the already allocated response to command failed. No extra error info. */
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    if( !mem_table_zalloc_shared((void *)(wbs_dec->codec_data), wbs_sbc_shared_malloc_table,
            WBS_SBC_SHARED_TABLE_LENGTH, &new_allocation))
    {
        async_wbs_dec_free_state_data(op_data);
        /* Change the already allocated response to command failed. No extra error info. */
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    /* now allocate the non-shareable memory */
    if(!mem_table_zalloc((void *)(wbs_dec->codec_data), wbs_dec_malloc_table,
                                                WBS_DEC_MALLOC_TABLE_LENGTH))
    {
        async_wbs_dec_free_state_data(op_data);
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
            async_wbs_dec_reset_working_data(op_data);
            wbsdec_init_dec_param(op_data);
            x_data->init_phase = AWBSD_RUN;

            return TRUE;
        }
        /* Fail free all the scratch memory we reserved */
        scratch_deregister();
    }
    /* Clear up all the allocated memory. */
    async_wbs_dec_free_state_data(op_data);
   /* Change the already allocated response to command failed. No extra error info. */
    base_op_change_response_status(response_data, STATUS_CMD_FAILED);
    return TRUE;
}


bool async_wbs_dec_destroy(OPERATOR_DATA *op_data, void *message_data,
        unsigned *response_id, void **response_data)
{
    if(base_op_destroy(op_data, message_data, response_id, response_data))
    {
        /* Free all the scratch memory we reserved */
        scratch_deregister();
        /* now destroy all the capability specific data */
        async_wbs_dec_free_state_data(op_data);
        return TRUE;
    }

    return FALSE;
}


bool async_wbs_dec_reset(OPERATOR_DATA *op_data, void *message_data,
        unsigned *response_id, void **response_data)
{
    if(!base_op_reset(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* now initialise specific working data */
    async_wbs_dec_reset_working_data(op_data);
    wbsdec_init_dec_param(op_data);
    return TRUE;
}


/* ************************************* Data processing-related functions and wrappers **********************************/

void async_wbs_dec_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    ASYNC_WBS_DEC_OP_DATA* x_data;
    SCO_COMMON_RCV_OP_DATA * sco_data;
    unsigned frame_size = 0;
    unsigned loopcount = 0;
    TOUCHED_TERMINALS tmp_touched;

    patch_fn(async_wbs_decode_process_data);

    x_data = get_instance_data(op_data);
    sco_data = &x_data->wbs.sco_rcv_op_data;

    /* There is a remote chance that this can run while async_wbs_dec_create
     * hasn't completed yet. Guard against this. */
    if (x_data->init_phase != AWBSD_RUN)
    {
        touched->sources = TOUCHED_NOTHING;
        return;
    }

    frame_size = awbs_encode_frame_size(x_data->wbs.codec_data);
    sco_data->sco_rcv_parameters.sco_pkt_size = frame_size;
    sco_data->sco_rcv_parameters.exp_pkts = 1;

#ifdef DEBUG_ASYNC_WBS_DEC
    static TIME last_called=0;
    TIME t= hal_get_time();
    L2_DBG_MSG2("@%d [+%d] wbs2_dec_process_data()",t,time_sub(t,last_called));
    last_called = t;
#endif

    /* There is a chance for the async_wbs_dec to occasionally miss a kick.
     * This could happen if two kicks can be generated in a quick succession,
     * or if some other operator is taking too much CPU time, and the decoder
     * doesn't run between one kick and the other: It will than see only
     * one kick and thus run only once, where there may be two packets in input.
     * Consume all available data in input, forcing the decoder to run
     * multiple times.
     */
    loopcount = 0;
    while (enough_data_to_run(sco_data, METADATA_HEADER_SIZE)
            && enough_space_to_run(sco_data, WBS_DEC_DEFAULT_OUTPUT_BLOCK_SIZE)
            && loopcount < MAX_COUNT_WBS_DEC_LOOP)
    {
        tmp_touched.sources = TOUCHED_NOTHING;

        wbs_dec_process_data(op_data, &tmp_touched);

        /* Neither wbs_dec nor async_wbs_dec functions kick backwards.
         * Update forward kicking with an "or" so we don't override.
         */
        touched->sources |= tmp_touched.sources;
        /* Guard this loop against any problem within the data processing and
         * limit the time spent here.
         */
        loopcount ++;
    }

}

bool awbs_dec_opmsg_set_encoding_params(OPERATOR_DATA *op_data, void *message_data,
        unsigned int *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    ASYNC_WBS_DEC_OP_DATA *awbs_data = get_instance_data(op_data);
    sbc_codec *codec_data = awbs_data->wbs.codec_data;
    unsigned bitpool = 0;

    if (OPMGR_GET_OPMSG_LENGTH((OP_MSG_REQ *)message_data) != OPMSG_ASYNC_WBS_DEC_SET_BITPOOL_VALUE_WORD_SIZE)
    {
        return FALSE;
    }

    bitpool = OPMSG_FIELD_GET(message_data, OPMSG_ASYNC_WBS_DEC_SET_BITPOOL_VALUE, BITPOOL);

    if (bitpool > ASYNC_WBS_MAX_BITPOOL_VALUE || bitpool < ASYNC_WBS_MIN_BITPOOL_VALUE)
    {
        return FALSE;
    }

    /* Populate the sbc structure with the new fields */
    codec_data->enc_setting_bitpool = bitpool;
    codec_data->bitpool = bitpool;
    codec_data->cur_frame_length = awbs_encode_frame_size(codec_data);

    return TRUE;
}

bool async_wbs_dec_buffer_details(OPERATOR_DATA *op_data, void *message_data,
        unsigned *response_id, void **response_data)
{
    unsigned wbs_buff_size = 0;

    if(!base_op_buffer_details(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    if (((unsigned *)message_data)[0] & TERMINAL_SINK_MASK)
    {
        wbs_buff_size = WBS_DEC_INPUT_BUFFER_SIZE;
    }
    else
    {
        /* for decoder, the output size might have been configured by the user */
        ASYNC_WBS_DEC_OP_DATA* awbs_data = get_instance_data(op_data);
        wbs_buff_size = MAX(WBS_DEC_OUTPUT_BUFFER_SIZE,
                            awbs_data->wbs.sco_rcv_op_data.sco_rcv_parameters.output_buffer_size);
    }

    ((OP_BUF_DETAILS_RSP*)*response_data)->b.buffer_size = wbs_buff_size;
    /* supports metadata in both side  */
    ((OP_BUF_DETAILS_RSP*)*response_data)->metadata_buffer = 0;
    ((OP_BUF_DETAILS_RSP*)*response_data)->supports_metadata = TRUE;

    L4_DBG_MSG2( "wbs_buffer_details (capID=%d)  %d \n", op_data->cap_data->id, ((OP_STD_RSP*)*response_data)->resp_data.data);

    return TRUE;
}

