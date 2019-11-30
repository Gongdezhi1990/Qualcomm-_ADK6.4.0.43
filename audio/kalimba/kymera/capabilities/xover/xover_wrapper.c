/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  xover_wrapper.c
 * \ingroup  operators
 *
 *  xover operator
 *
 */
/****************************************************************************
Include Files
*/

#include "adaptor/adaptor.h"
#include "mem_utils/scratch_memory.h"
#include "ps/ps.h"
#include "obpm_prim.h"
#include "opmsg_prim.h"
#include "op_msg_helpers.h"
#include "xover_wrapper.h"
#include "xover_gen_c.h"

#include "patch/patch.h"


/****************************************************************************
Private Constant Definitions
*/
/** The terminal ID of the input terminal */
#define INPUT_TERMINAL_ID (0 | TERMINAL_SINK_MASK)
/** The terminal ID of the output terminal */
#define OUTPUT_TERMINAL_ID (0)
#define XOVER_XOVER_VERSION_MINOR 2

#ifdef CAPABILITY_DOWNLOAD_BUILD
#define XOVER_CAP_ID CAP_ID_DOWNLOAD_XOVER
#else
#define XOVER_CAP_ID CAP_ID_XOVER
#endif

/****************************************************************************
Private Type Definitions
*/

/****************************************************************************
Private Function Definitions
*/
/* Message handlers */
static bool xover_wrapper_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
static bool xover_wrapper_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
static bool xover_wrapper_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
static bool xover_wrapper_stop(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
static bool xover_wrapper_reset(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
static bool xover_wrapper_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
static bool xover_wrapper_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
static bool xover_wrapper_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
static bool xover_wrapper_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
static bool xover_wrapper_get_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);

/* Op msg handlers */
static bool xover_wrapper_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
static bool xover_wrapper_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
static bool xover_wrapper_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
static bool xover_wrapper_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
static bool xover_wrapper_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
static bool xover_wrapper_opmsg_set_ucid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
static bool xover_wrapper_opmsg_get_ps_id(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
static bool xover_wrapper_opmsg_set_sample_rate(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
#ifdef INSTALL_METADATA
static bool xover_wrapper_opmsg_set_metadata_delay(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
#endif

/* Data processing function */
static void xover_wrapper_process_data(OPERATOR_DATA*, TOUCHED_TERMINALS*);

bool XOVER_CAP_Create(XOVER_OP_DATA* p_ext_data);
void XOVER_CAP_Destroy(XOVER_OP_DATA* p_ext_data);
static bool ups_params_xover(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank,
                 uint16 length, unsigned* data, STATUS_KYMERA status,uint16 extra_status_info);
static bool get_pair_count_xover(XOVER_OP_DATA* p_ext_data);
bool allocate_xover_object(unsigned terminal_num, XOVER_OP_DATA* p_ext_data);
bool free_xover_object(unsigned terminal_num, XOVER_OP_DATA* p_ext_data);
bool allocate_peq_object_xover(t_xover_object *ptr_xover_dobject , XOVER_OP_DATA* p_ext_data);


/*****************************************************************************
Private Constant Declarations
*/
/** The cross over capability function handler table */
const handler_lookup_struct xover_wrapper_handler_table =
{
    xover_wrapper_create,          /* OPCMD_CREATE */
    xover_wrapper_destroy,         /* OPCMD_DESTROY */
    xover_wrapper_start,           /* OPCMD_START */
    xover_wrapper_stop,            /* OPCMD_STOP */
    xover_wrapper_reset,           /* OPCMD_RESET */
    xover_wrapper_connect,         /* OPCMD_CONNECT */
    xover_wrapper_disconnect,      /* OPCMD_DISCONNECT */
    xover_wrapper_buffer_details,  /* OPCMD_BUFFER_DETAILS */
    xover_wrapper_get_data_format, /* OPCMD_DATA_FORMAT */
    xover_wrapper_get_sched_info   /* OPCMD_GET_SCHED_INFO */
};

/* Null terminated operator message handler table */
const opmsg_handler_lookup_table_entry xover_wrapper_opmsg_handler_table[] =
    {
    {OPMSG_COMMON_ID_GET_CAPABILITY_VERSION,             base_op_opmsg_get_capability_version},
    {OPMSG_COMMON_ID_SET_CONTROL,                        xover_wrapper_opmsg_obpm_set_control},
    {OPMSG_COMMON_ID_GET_PARAMS,                         xover_wrapper_opmsg_obpm_get_params},
    {OPMSG_COMMON_ID_GET_DEFAULTS,                       xover_wrapper_opmsg_obpm_get_defaults},
    {OPMSG_COMMON_ID_SET_PARAMS,                         xover_wrapper_opmsg_obpm_set_params},
    {OPMSG_COMMON_ID_GET_STATUS,                         xover_wrapper_opmsg_obpm_get_status},
    {OPMSG_COMMON_ID_SET_UCID,                           xover_wrapper_opmsg_set_ucid},
    {OPMSG_COMMON_ID_GET_LOGICAL_PS_ID,                  xover_wrapper_opmsg_get_ps_id},
    {OPMSG_COMMON_SET_SAMPLE_RATE,                       xover_wrapper_opmsg_set_sample_rate},
#ifdef INSTALL_METADATA
    {OPMSG_COMMON_SET_METADATA_DELAY,                    xover_wrapper_opmsg_set_metadata_delay},
#endif
    {0, NULL}};

const CAPABILITY_DATA xover_cap_data =
{
    XOVER_CAP_ID,                                                  /* Capability ID */
    XOVER_XOVER_VERSION_MAJOR, XOVER_XOVER_VERSION_MINOR,          /* Version information - hi and lo parts */
    XOVER_CAP_MAX_CHANNELS, 2*XOVER_CAP_MAX_CHANNELS,              /* Max number of sinks/inputs and sources/outputs */
    &xover_wrapper_handler_table,                                  /* Pointer to message handler function table */
    xover_wrapper_opmsg_handler_table,                             /* Pointer to operator message handler function table */
    xover_wrapper_process_data,                                    /* Pointer to data processing function */
    0,                                                             /* TODO - Processing time information */
    sizeof(XOVER_OP_DATA)                                          /* Size of capability-specific per-instance data */
};

/****************************************************************************
Private Function Declarations
*/
/* ******************************* Helper functions ************************************ */


/* ********************************** API functions ************************************* */

static bool xover_wrapper_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    XOVER_OP_DATA *p_ext_data = (XOVER_OP_DATA *)(op_data->extra_op_data);
    /* call base_op create, which also allocates and fills response message */
    if (!base_op_create(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* allocate memory */
    if (XOVER_CAP_Create(p_ext_data) == FALSE )
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        XOVER_CAP_Destroy(p_ext_data);
        return TRUE;
    }

    /* terminal types */
    p_ext_data->ip_format = AUDIO_DATA_FORMAT_FIXP;
    p_ext_data->op_format = AUDIO_DATA_FORMAT_FIXP;


    /* Initialize extended data for operator.  Assume initialized to zero*/
    p_ext_data->ReInitFlag = 1;
    p_ext_data->Host_mode = XOVER_SYSMODE_FULL;
    p_ext_data->sample_rate = 8000;

#ifdef KICK_KEEP_BUFFERS_FULL
    p_ext_data->keep_buffers_full = TRUE;
#endif

    if(!cpsInitParameters(&p_ext_data->parms_def,(unsigned*)XOVER_GetDefaults(XOVER_CAP_ID),(unsigned*)p_ext_data->xover_cap_params,sizeof(XOVER_PARAMETERS)))
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        XOVER_CAP_Destroy(p_ext_data);
        return TRUE;
    }

    /* initialize state */
    op_data->state = OP_NOT_RUNNING;

    return TRUE;
}

static bool xover_wrapper_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    XOVER_OP_DATA *p_ext_data = (XOVER_OP_DATA *)(op_data->extra_op_data);

    /* call base_op destroy that creates and fills response message, too */
    if(!base_op_destroy(op_data, message_data, response_id, response_data))
    {
        return(FALSE);
    }
    XOVER_CAP_Destroy(p_ext_data);
    /* call base_op destroy that creates and fills response message, too */
    return TRUE;
}

static bool xover_wrapper_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    unsigned input_active_chans;
    unsigned output_active_chans;
    XOVER_OP_DATA *p_ext_data = (XOVER_OP_DATA *)(op_data->extra_op_data);
    if (!base_op_start(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    op_data->state = OP_RUNNING;
   

    /* Work out which channels are connected and cache this so that it's not
     * calculated every kick */
    input_active_chans = 0;
    if (p_ext_data->chan_flags & CHAN_MASK_0)
    {
        input_active_chans = (TOUCHED_SINK_0);
    }
    if (p_ext_data->chan_flags & CHAN_MASK_1)
    {
        input_active_chans |= (TOUCHED_SINK_1);
    }
    if (p_ext_data->chan_flags & CHAN_MASK_2)
    {
        input_active_chans |= (TOUCHED_SINK_2);
    }
    if (p_ext_data->chan_flags & CHAN_MASK_3)
    {
        input_active_chans |= (TOUCHED_SINK_3);
    }
    if (p_ext_data->chan_flags & CHAN_MASK_4)
    {
        input_active_chans |= (TOUCHED_SINK_4);
    }
    if (p_ext_data->chan_flags & CHAN_MASK_5)
    {
        input_active_chans |= (TOUCHED_SINK_5);
    }
    if (p_ext_data->chan_flags & CHAN_MASK_6)
    {
        input_active_chans |= (TOUCHED_SINK_6);
    }
    if (p_ext_data->chan_flags & CHAN_MASK_7)
    {
        input_active_chans |= (TOUCHED_SINK_7);
    }
    p_ext_data->input_active_chans = input_active_chans;

    output_active_chans = 0;
    if (p_ext_data->chan_flags & CHAN_MASK_0)
    {
        output_active_chans = (TOUCHED_SOURCE_0);
        output_active_chans |= (TOUCHED_SOURCE_1);
    }
    if (p_ext_data->chan_flags & CHAN_MASK_1)
    {
        output_active_chans |= (TOUCHED_SOURCE_2);
        output_active_chans |= (TOUCHED_SOURCE_3);
    }
    if (p_ext_data->chan_flags & CHAN_MASK_2)
    {
        output_active_chans |= (TOUCHED_SOURCE_4);
        output_active_chans |= (TOUCHED_SOURCE_5);
    }
    if (p_ext_data->chan_flags & CHAN_MASK_3)
    {
        output_active_chans |= (TOUCHED_SOURCE_6);
        output_active_chans |= (TOUCHED_SOURCE_7);
    }
    if (p_ext_data->chan_flags & CHAN_MASK_4)
    {
        output_active_chans |= (TOUCHED_SOURCE_8);
        output_active_chans |= (TOUCHED_SOURCE_9);
    }
    if (p_ext_data->chan_flags & CHAN_MASK_5)
    {
        output_active_chans |= (TOUCHED_SOURCE_10);
        output_active_chans |= (TOUCHED_SOURCE_11);
    }
    if (p_ext_data->chan_flags & CHAN_MASK_6)
    {
        output_active_chans |= (TOUCHED_SOURCE_12);
        output_active_chans |= (TOUCHED_SOURCE_13);
    }
    if (p_ext_data->chan_flags & CHAN_MASK_7)
    {
        output_active_chans |= (TOUCHED_SOURCE_14);
        output_active_chans |= (TOUCHED_SOURCE_15);
    }
    p_ext_data->output_active_chans = output_active_chans;

#ifdef INSTALL_METADATA
    buff_metadata_add_delay(p_ext_data->metadata_op_buffer, p_ext_data->metadata_delay);
#endif /* INSTALL_METADATA */
    return TRUE;
}

static bool xover_wrapper_stop(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
#ifdef INSTALL_METADATA
    OP_STATE state = op_data->state;
    XOVER_OP_DATA *p_ext_data = (XOVER_OP_DATA *)(op_data->extra_op_data);
#endif
    if (!base_op_stop(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

#ifdef INSTALL_METADATA
    /* do something only if the current state is "running" */
    if(state == OP_RUNNING)
    {
        buff_metadata_remove_delay(p_ext_data->metadata_op_buffer, p_ext_data->metadata_delay);
    }
#endif
    return TRUE;
}

static bool xover_wrapper_reset(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
#ifdef INSTALL_METADATA
    OP_STATE state = op_data->state;
    XOVER_OP_DATA *p_ext_data = (XOVER_OP_DATA *)(op_data->extra_op_data);
#endif
    if (!base_op_reset(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

#ifdef INSTALL_METADATA
    /* do something only if the current state is "running" */
    if(state == OP_RUNNING)
    {
        buff_metadata_remove_delay(p_ext_data->metadata_op_buffer, p_ext_data->metadata_delay);
    }
#endif
    return TRUE;
}

static bool xover_wrapper_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    XOVER_OP_DATA *p_ext_data = (XOVER_OP_DATA *)(op_data->extra_op_data);
    unsigned terminal_num, terminal_id = ((unsigned*)message_data)[0];
    terminal_num = terminal_id & TERMINAL_NUM_MASK;
    if (!base_op_connect(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }
    /* can't connect while running */
    if(op_data->state == OP_RUNNING)
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }
    if ( terminal_num >= XOVER_CAP_MAX_CHANNELS )
    {
        /* invalid terminal id */
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    p_ext_data->op_all_connected = FALSE;

    if (terminal_id & INPUT_TERMINAL_ID)
    {
        /* allocate xover object by input terminal */
        if ( allocate_xover_object(terminal_num, p_ext_data) )
        {
            p_ext_data->ip_buffer[terminal_num] = (tCbuffer*)(((uintptr_t *)message_data)[1]);
#ifdef INSTALL_METADATA
            if(p_ext_data->metadata_ip_buffer == NULL)
            {
                if (buff_has_metadata(p_ext_data->ip_buffer[terminal_num]))
                {
                    p_ext_data->metadata_ip_buffer = p_ext_data->ip_buffer[terminal_num];
                }
            }
#endif /* INSTALL_METADATA */
        }
        else
        {
            /* failed to allocate xover object for this channel */
            base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        }
    }
    else
    {
        p_ext_data->op_buffer[terminal_num] = (tCbuffer*)(((uintptr_t *)message_data)[1]);
#ifdef INSTALL_METADATA
        if(p_ext_data->metadata_op_buffer == NULL)
        {
            if (buff_has_metadata(p_ext_data->op_buffer[terminal_num]))
            {
                p_ext_data->metadata_op_buffer = p_ext_data->op_buffer[terminal_num];
            }
        }
#endif /* INSTALL_METADATA */
    }
    
    /* set internal capability state variable to "connected" if n_inputs and n_outputs are equal */
    p_ext_data->op_all_connected = get_pair_count_xover(p_ext_data);

    p_ext_data->ReInitFlag = 1;
    return TRUE;
}


static bool xover_wrapper_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    XOVER_OP_DATA *p_ext_data = (XOVER_OP_DATA *)(op_data->extra_op_data);
    unsigned terminal_num, terminal_id = ((unsigned*)message_data)[0];
    terminal_num = terminal_id & TERMINAL_NUM_MASK;

    if (!base_op_disconnect(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }
    /* can't disconnect while running */
    if(op_data->state == OP_RUNNING)
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }
    if ( terminal_num >= XOVER_CAP_MAX_CHANNELS )
    {
        /* invalid terminal id */
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    if (terminal_id & INPUT_TERMINAL_ID)
    {
#ifdef INSTALL_METADATA
        if (p_ext_data->metadata_ip_buffer == p_ext_data->ip_buffer[terminal_num])
        {
            unsigned i;
            bool found_alternative = FALSE;
            for (i = 0; i < XOVER_CAP_MAX_CHANNELS; i++)
            {
                if (i == terminal_num)
                {
                    continue;
                }
                if (p_ext_data->ip_buffer[i] != NULL && buff_has_metadata(p_ext_data->ip_buffer[i]))
                {
                    p_ext_data->metadata_ip_buffer = p_ext_data->ip_buffer[i];
                    found_alternative = TRUE;
                    break;
                }
            }
            if (!found_alternative)
            {
                p_ext_data->metadata_ip_buffer = NULL;
            }
        }
#endif /* INSTALL_METADATA */
        /* free the xover object associated with this input terminal */
        free_xover_object(terminal_num, p_ext_data);
        p_ext_data->ip_buffer[terminal_num] = NULL;
    }
    else
    {
#ifdef INSTALL_METADATA
        if (p_ext_data->metadata_op_buffer == p_ext_data->op_buffer[terminal_num])
        {
            unsigned i;
            bool found_alternative = FALSE;
            for (i = 0; i < XOVER_CAP_MAX_CHANNELS; i++)
            {
                if (i == terminal_num)
                {
                    continue;
                }
                if (p_ext_data->op_buffer[i] != NULL && buff_has_metadata(p_ext_data->op_buffer[i]))
                {
                    p_ext_data->metadata_op_buffer = p_ext_data->op_buffer[i];
                    found_alternative = TRUE;
                    break;
                }
            }
            if (!found_alternative)
            {
                p_ext_data->metadata_op_buffer = NULL;
            }
        }
#endif /* INSTALL_METADATA */
        p_ext_data->op_buffer[terminal_num] = NULL;
    }
    
    /* set internal capability state variable to "connected" if n_inputs and n_outputs are equal */
    p_ext_data->op_all_connected  = get_pair_count_xover(p_ext_data);

    p_ext_data->ReInitFlag = 1;

    return TRUE;
}


static bool xover_wrapper_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    if (!base_op_buffer_details(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

#ifdef INSTALL_METADATA
    {
        /* If an input/output connection is already present and has metadata then
         * we are obliged to return that buffer so that metadata can be shared
         * between channels. */
        tCbuffer *meta_buff;
        unsigned terminal_id = ((unsigned *)message_data)[0];
        if ((terminal_id & TERMINAL_SINK_MASK) == TERMINAL_SINK_MASK)
        {
            meta_buff = ((XOVER_OP_DATA *)(op_data->extra_op_data))->metadata_ip_buffer;
        }
        else
        {
            meta_buff = ((XOVER_OP_DATA *)(op_data->extra_op_data))->metadata_op_buffer;
        }

        ((OP_BUF_DETAILS_RSP*)*response_data)->metadata_buffer = meta_buff;
        ((OP_BUF_DETAILS_RSP*)*response_data)->supports_metadata = TRUE;
    }
#endif /* INSTALL_METADATA */

    L4_DBG_MSG1("xover_buffer_details  %d \n", ((OP_BUF_DETAILS_RSP*)*response_data)->b.buffer_size  );

    return TRUE;
}


static bool xover_wrapper_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    OP_SCHED_INFO_RSP* resp;

    if (!base_op_get_sched_info(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }
    /* Populate the response*/
    base_op_change_response_status(response_data, STATUS_OK);
    resp = *response_data;
    resp->op_id = op_data->id;
    /* Same buffer size for sink and source.
       No additional verification needed.*/
    resp->block_size = XOVER_CAP_DEFAULT_BLOCK_SIZE;
    resp->run_period = 0;

    *response_data = resp;
    return TRUE;
}


static bool xover_wrapper_get_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    XOVER_OP_DATA *p_ext_data;
    if (!base_op_get_data_format(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    p_ext_data = (XOVER_OP_DATA *)(op_data->extra_op_data);
    /* return the terminal's data format - since we are mono-to-mono, easy to get to terminal data purely based on direction flag */
    if((((unsigned*)message_data)[0] & TERMINAL_SINK_MASK) == 0)
    {
        ((OP_STD_RSP*)*response_data)->resp_data.data = p_ext_data->op_format;
    }
    else
    {
        ((OP_STD_RSP*)*response_data)->resp_data.data = p_ext_data->ip_format;
    }

    return TRUE;
}


/* ************************************* Data processing-related functions and wrappers **********************************/
static void xover_wrapper_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
#if XOVER_CAP_DEFAULT_BLOCK_SIZE != 1
#error The process data function kick backwards logic is valid only when block size is 1
#endif
    unsigned samples_to_process;
    unsigned i;
    bool process_all_input = FALSE;
    XOVER_OP_DATA *p_ext_data = (XOVER_OP_DATA *)(op_data->extra_op_data);
    
    
    patch_fn(xover_wrapper_process_data);


    /* work out amount of input data to process, based on output space and input data amount */
    samples_to_process = UINT_MAX;
    for (i=0; i<XOVER_CAP_MAX_CHANNELS; i++)
    {
        if ( p_ext_data->chan_flags & (1<<i) )
        {
            unsigned amount = cbuffer_calc_amount_data_in_words(p_ext_data->ip_buffer[i]);
            if ( samples_to_process > amount )
            {
                if (amount < XOVER_CAP_DEFAULT_BLOCK_SIZE)
                {
                    /* There is no data on at least 1 channel, so exit */
                    return;
                }
                samples_to_process = amount;
                process_all_input = TRUE;
            }
            amount = cbuffer_calc_amount_space_in_words(p_ext_data->op_buffer[2*i]);
            if ( samples_to_process > amount )
            {
                if (amount < XOVER_CAP_DEFAULT_BLOCK_SIZE)
                {
                    /* There is no space on at least 1 channel so exit. */
                    return;
                }
                samples_to_process = amount;
                process_all_input = FALSE;
            }
            amount = cbuffer_calc_amount_space_in_words(p_ext_data->op_buffer[2*i+1]);
            if ( samples_to_process > amount )
            {
                if (amount < XOVER_CAP_DEFAULT_BLOCK_SIZE)
                {
                    /* There is no space on at least 1 channel so exit. */
                    return;
                }
                samples_to_process = amount;
                process_all_input = FALSE;
            }
        }
    }

    /* if this will consume all the input data it needs to be pulled from up stream so request
     * it as well as propagating a kick down stream */
    touched->sources = p_ext_data->output_active_chans;
    if ((process_all_input) || ((samples_to_process > 0) && (p_ext_data->keep_buffers_full)))
    {
        touched->sinks = p_ext_data->input_active_chans;
    }

#ifdef INSTALL_METADATA
    /* Propagate any metadata to the output. Any handling of it's presence or
     * not is handled by the metadata library */
    metadata_strict_transport(p_ext_data->metadata_ip_buffer,
                                p_ext_data->metadata_op_buffer,
                                samples_to_process * OCTETS_PER_SAMPLE );
#endif  /* INSTALL_METADATA */

    /* call ASM processing function */
    xover_processing(p_ext_data, samples_to_process);

}



/* **************************** Operator message handlers ******************************** */

static bool xover_wrapper_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    XOVER_OP_DATA      *op_extra_data = (XOVER_OP_DATA*)op_data->extra_op_data;
    unsigned            i,num_controls,cntrl_value; 
    CPS_CONTROL_SOURCE  cntrl_src;
    unsigned result = OPMSG_RESULT_STATES_NORMAL_STATE;

    if(!cps_control_setup(message_data, resp_length, resp_data,&num_controls))
    {
       return FALSE;
    }

    for(i=0;i<num_controls;i++)
    {
        unsigned  cntrl_id=cps_control_get(message_data,i,&cntrl_value,&cntrl_src);

        if(cntrl_id != OPMSG_CONTROL_MODE_ID)
        {
            result = OPMSG_RESULT_STATES_UNSUPPORTED_CONTROL;
            break;
        }
        /* Only interested in lower 8-bits of value */
        cntrl_value &= 0xFF;
        if (cntrl_value >= XOVER_SYSMODE_MAX_MODES)
        {
            result = OPMSG_RESULT_STATES_INVALID_CONTROL_VALUE;
            break;
        }
        /* Control is Mode */
        if(cntrl_src == CPS_SOURCE_HOST)
        {
           op_extra_data->Host_mode = cntrl_value;
        }
        else
        {
            op_extra_data->Ovr_Control = (cntrl_src == CPS_SOURCE_OBPM_DISABLE) ?  0 : XOVER_CONTROL_MODE_OVERRIDE;
            op_extra_data->Obpm_mode = cntrl_value;
        }
    }

    cps_response_set_result(resp_data,result);

    /* Set the Reinit flag after setting the paramters */
    if (result == OPMSG_RESULT_STATES_NORMAL_STATE)
    {
        op_extra_data->ReInitFlag = 1;
    }
    return TRUE;
}

static bool xover_wrapper_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    XOVER_OP_DATA   *op_extra_data = (XOVER_OP_DATA*)op_data->extra_op_data;

    return cpsGetParameterMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);
}

static bool xover_wrapper_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    XOVER_OP_DATA   *op_extra_data = (XOVER_OP_DATA*)op_data->extra_op_data;
    
    return cpsGetDefaultsMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);
}

static bool xover_wrapper_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    XOVER_OP_DATA   *op_extra_data = (XOVER_OP_DATA*)op_data->extra_op_data;
    bool retval;

    retval = cpsSetParameterMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);

    /* Set the Reinit flag after setting the paramters */
    op_extra_data->ReInitFlag = 1;

    return retval;
}

static bool xover_wrapper_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    XOVER_OP_DATA   *op_extra_data = (XOVER_OP_DATA*)op_data->extra_op_data;

    unsigned  *resp;

    if(!common_obpm_status_helper(message_data,resp_length,resp_data,sizeof(XOVER_STATISTICS),&resp))
    {
         return FALSE;
    }

    if(resp)
    {
       OPSTATE_INTERNAL  op_state_status = op_extra_data->op_all_connected ? OPSTATE_INTERNAL_CONNECTED : OPSTATE_INTERNAL_READY;
        resp = cpsPackWords(&op_extra_data->Cur_mode,&op_extra_data->Ovr_Control ,resp);
        resp = cpsPackWords(&op_extra_data->comp_config,(unsigned*)&op_data->state ,resp);
        cpsPackWords((unsigned*)&op_state_status,NULL ,resp);
    }
    return TRUE;
}




static bool xover_wrapper_opmsg_set_ucid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    XOVER_OP_DATA   *op_extra_data = (XOVER_OP_DATA*)op_data->extra_op_data;
    PS_KEY_TYPE key;
    bool retval;

    retval = cpsSetUcidMsgHandler(&op_extra_data->parms_def,message_data,resp_length,resp_data);

    key = MAP_CAPID_UCID_SBID_TO_PSKEYID(XOVER_CAP_ID,op_extra_data->parms_def.ucid,OPMSG_P_STORE_PARAMETER_SUB_ID);
    ps_entry_read((void*)op_data,key,PERSIST_ANY,ups_params_xover);

    return retval;;
}

static bool xover_wrapper_opmsg_get_ps_id(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    XOVER_OP_DATA   *op_extra_data = (XOVER_OP_DATA*)op_data->extra_op_data;
    
    return cpsGetUcidMsgHandler(&op_extra_data->parms_def,CAP_ID_PEQ,message_data,resp_length,resp_data);
}


static bool xover_wrapper_opmsg_set_sample_rate(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    XOVER_OP_DATA *p_ext_data = (XOVER_OP_DATA *)(op_data->extra_op_data);

    /* We received client ID, length and then opmsgID and OBPM params */
    p_ext_data->sample_rate = 25 * ((unsigned*)message_data)[3];
    p_ext_data->ReInitFlag = 1;

    return(TRUE);
}

#ifdef INSTALL_METADATA
static bool xover_wrapper_opmsg_set_metadata_delay(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    XOVER_OP_DATA *p_ext_data = (XOVER_OP_DATA *)(op_data->extra_op_data);

    /* The message value is in samples. Convert it to octets */
    p_ext_data->metadata_delay = OCTETS_PER_SAMPLE * (((unsigned*)message_data)[3]);

    return (TRUE);
}
#endif /* INSTALL_METADATA */

static bool ups_params_xover(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank,
                 uint16 length, unsigned* data, STATUS_KYMERA status,uint16 extra_status_info)
{
    /* returns true if succesful, false if failed */
    XOVER_OP_DATA   *op_extra_data = (XOVER_OP_DATA*)((OPERATOR_DATA*)instance_data)->extra_op_data;

    cpsSetParameterFromPsStore(&op_extra_data->parms_def,length,data,status);

    /* Set the Reinit flag after setting the paramters */
    op_extra_data->ReInitFlag = 1;

    return(TRUE);
}

/**
 * \brief Allocate memory which will be used for the entire lifespan of the operator
 *
 * \param p_ext_data : pointer to the extra op data structure
 */
bool XOVER_CAP_Create(XOVER_OP_DATA* p_ext_data)
{
    /* allocate the "Curparams" block for xover */
    XOVER_PARAMETERS *p_xover_cap_params;
    p_xover_cap_params = (XOVER_PARAMETERS*)xzpmalloc(sizeof(XOVER_PARAMETERS));
    if (p_xover_cap_params == NULL)
    {
        return FALSE;
    }
    p_ext_data->xover_cap_params = p_xover_cap_params;


    /* allocate the PEQ coeff_params block for the low_freq object*/
    t_peq_params *p_coeff_params_low;
    p_coeff_params_low = xzpmalloc(PEQ_PARAMS_OBJECT_SIZE(PEQ_MAX_STAGES));
    if (p_coeff_params_low == NULL)
    {
        return FALSE;
    }
    p_ext_data->peq_coeff_params_low = p_coeff_params_low;
    /* allocate the PEQ cap_params block for the low_freq object*/
    unsigned *p_cap_params_low;
    p_cap_params_low = (unsigned int *)xzppmalloc(XOVER_PEQ_PARAMETERS_SIZE , MALLOC_PREFERENCE_DM1);
    if (p_cap_params_low == NULL)
    {
        return FALSE;
    }
    p_ext_data->peq_cap_params_low = p_cap_params_low;

    /* allocate the PEQ coeff_params block for the high_freq object*/
    t_peq_params *p_coeff_params_high;
    p_coeff_params_high = xzpmalloc(PEQ_PARAMS_OBJECT_SIZE(PEQ_MAX_STAGES));
    if (p_coeff_params_high == NULL)
    {
        return FALSE;
    }
    p_ext_data->peq_coeff_params_high = p_coeff_params_high;
    /* allocate the PEQ cap_params block for the high_freq object*/
    unsigned *p_cap_params_high;
    p_cap_params_high = (unsigned int *)xzppmalloc(XOVER_PEQ_PARAMETERS_SIZE , MALLOC_PREFERENCE_DM1);
    if (p_cap_params_high == NULL)
    {
        return FALSE;
    }
    p_ext_data->peq_cap_params_high = p_cap_params_high;


    return TRUE;
}

/**
 * \brief Free memory which is be used for the entire lifespan of the operator
 *
 * \param p_ext_data : pointer to the extra op data structure
 */
void XOVER_CAP_Destroy(XOVER_OP_DATA* p_ext_data)
{
    /* returns true if succesful, false if failed */
    int i;

    /* free any leftover xover data objects */
    for (i=0; i<XOVER_CAP_MAX_CHANNELS; i++)
    {
        free_xover_object(i, p_ext_data);
    }

    /* free the capability parameters */
    pfree(p_ext_data->xover_cap_params);
    p_ext_data->xover_cap_params = NULL;
    
    /* free the XOVER buffers */
    pfree(p_ext_data->peq_coeff_params_low);
    p_ext_data->peq_coeff_params_low = NULL;
    
    pfree(p_ext_data->peq_coeff_params_high);
    p_ext_data->peq_coeff_params_high = NULL;
    
    pfree(p_ext_data->peq_cap_params_low);
    p_ext_data->peq_cap_params_low = NULL;
    
    pfree(p_ext_data->peq_cap_params_high);
    p_ext_data->peq_cap_params_high = NULL;
    

}

/**
 * \brief Allocate a xover data object, which contains (among other things), the
 * xover's history buffers. Returns true if succesful, false if failed
 *
 * \param terminal_num : (input) Channel which will be associated with the xover object
 * \param p_ext_data : pointer to the extra op data structure
 */
bool allocate_xover_object(unsigned terminal_num, XOVER_OP_DATA* p_ext_data)
{
    t_xover_object *ptr_xover_dobject;

    if (terminal_num >= XOVER_CAP_MAX_CHANNELS)
    {
        /* invalid terminal ID, this is equivilent to an allocation failure */
        return FALSE;
    }
    ptr_xover_dobject = p_ext_data->xover_object[terminal_num];
    if (ptr_xover_dobject != NULL)
    {
        /* already a xover object allocated for this channel */
        return TRUE;
    }
    ptr_xover_dobject = xzpmalloc(XOVER_OBJECT_SIZE);
    if (ptr_xover_dobject == NULL)
    {
        /* failed to allocate */
        return FALSE;
    }

    /* set the pointer to xover_params for the xover_data_object */
    ptr_xover_dobject->params_ptr =(t_xover_params*) p_ext_data->xover_cap_params;

    /* set the pointers to cap_params for the low_freq and high_freq */
    ptr_xover_dobject->peq_cap_params_low = p_ext_data->peq_cap_params_low;
    ptr_xover_dobject->peq_cap_params_high = p_ext_data->peq_cap_params_high;

    p_ext_data->xover_object[terminal_num] = ptr_xover_dobject;

    /* allocates low_freq and high_freq peq objects */
    allocate_peq_object_xover(ptr_xover_dobject , p_ext_data);


    return TRUE;
}

/**
 * \brief free xover data object. returns true if the object was deleted,
 * false otherwise
 *
 * \param terminal_num : channel to free
 * \param p_ext_data : pointer to the extra op data structure
 */
bool free_xover_object(unsigned terminal_num, XOVER_OP_DATA* p_ext_data)
{
    t_xover_object *p_dobject;

    if (terminal_num >= XOVER_CAP_MAX_CHANNELS)
    {
        return FALSE;
    }
    p_dobject = p_ext_data->xover_object[terminal_num];
    if (p_dobject == NULL)
    {
        /* object has already been freed */
        return FALSE;
    }

    /* free the PEQ data_objects */
    if (p_dobject->peq_object_ptr_low)
    {
      pfree(p_dobject->peq_object_ptr_low);
      p_dobject->peq_object_ptr_low = NULL;
    }
    if (p_dobject->peq_object_ptr_high)
    {
      pfree(p_dobject->peq_object_ptr_high);
      p_dobject->peq_object_ptr_high = NULL;
    }

    pfree(p_dobject);
    p_ext_data->xover_object[terminal_num] = NULL;
    return TRUE;
}


/**
 * \brief Analyze the channel configuration, count pairs and form channel mask.
 * this function does not return any values, but does update several internal member variables:
 *  -"pair_count" member is set to the number of connected channel pairs
 *  -a logic "1" is set in the "chan_flags" member bitmask at each bit position corresponding to a connected channel pair
 *
 * \param p_ext_data : pointer to the extra op data structure
 */
static bool get_pair_count_xover(XOVER_OP_DATA* p_ext_data)
{
    int i;

    p_ext_data->pair_count = 0;
    p_ext_data->chan_flags = 0;
    for (i=0; i<XOVER_CAP_MAX_CHANNELS; i++)
    {
        if ( p_ext_data->ip_buffer[i] != NULL && p_ext_data->op_buffer[2*i] != NULL && p_ext_data->op_buffer[2*i+1] != NULL)
        {
            p_ext_data->chan_flags |= (1<<i);
            ++p_ext_data->pair_count;
        }
    }
    return (p_ext_data->pair_count > 0) ? TRUE : FALSE;
}

bool allocate_peq_object_xover(t_xover_object *ptr_xover_dobject , XOVER_OP_DATA* p_ext_data)
{
    t_peq_object *ptr_peq_dobject_low;
    t_peq_object *ptr_peq_dobject_high;

    /* allocate memory for the PEQ object low frequency band */
    ptr_peq_dobject_low = xzppmalloc(DH_PEQ_OBJECT_SIZE(PEQ_MAX_STAGES) , MALLOC_PREFERENCE_DM2);
    if (ptr_peq_dobject_low == NULL)
    {
        /* failed to allocate */
        return FALSE;
    }
    ptr_peq_dobject_low->max_stages = PEQ_MAX_STAGES;
    ptr_peq_dobject_low->params_ptr = p_ext_data->peq_coeff_params_low;
    ptr_xover_dobject->peq_object_ptr_low = ptr_peq_dobject_low;


    /* allocate memory for the PEQ object high frequency band */
    ptr_peq_dobject_high = xzppmalloc(DH_PEQ_OBJECT_SIZE(PEQ_MAX_STAGES) , MALLOC_PREFERENCE_DM2);
    if (ptr_peq_dobject_high == NULL)
    {
        /* failed to allocate */
        return FALSE;
    }
    ptr_peq_dobject_high->max_stages = PEQ_MAX_STAGES;
    ptr_peq_dobject_high->params_ptr = p_ext_data->peq_coeff_params_high;
    ptr_xover_dobject->peq_object_ptr_high = ptr_peq_dobject_high;

    return TRUE;
}
