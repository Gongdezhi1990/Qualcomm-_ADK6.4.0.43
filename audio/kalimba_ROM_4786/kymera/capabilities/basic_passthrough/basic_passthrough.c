/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  basic_passthrough.c
 * \ingroup  operators
 *
 *  Basic passthrough operator
 *
 */
/****************************************************************************
Include Files
*/
#include "basic_passthrough_private.h"
#include "op_msg_helpers.h"
#include "basic_passthrough_gen_c.h"
#include "adaptor/adaptor.h"
#include "ps/ps.h"
#include "obpm_prim.h"

/****************************************************************************
Private Constant Definitions
*/


/****************************************************************************
Private Type Definitions
*/

#if defined(INSTALL_CBUFFER_EX)
static void basic_passthrough_processing_ex(BASIC_PASSTHROUGH_OP_DATA *op_data, unsigned octets);
#endif

#ifdef CAPABILITY_DOWNLOAD_BUILD
#define BASIC_PASS_CAP_ID CAP_ID_DOWNLOAD_PASSTHROUGH
#define TTP_PASS_CAP_ID CAP_ID_DOWNLOAD_TTP_PASS
#else
#define BASIC_PASS_CAP_ID CAP_ID_BASIC_PASS
#define TTP_PASS_CAP_ID CAP_ID_TTP_PASS

#endif
/*****************************************************************************
Private Constant Declarations
*/
/** The basic passthrough capability function handler table */
const handler_lookup_struct basic_passthrough_handler_table =
{
    basic_passthrough_create,          /* OPCMD_CREATE */
    basic_passthrough_destroy,         /* OPCMD_DESTROY */
    base_op_start,                     /* OPCMD_START */
    base_op_stop,                      /* OPCMD_STOP */
    basic_passthrough_reset,           /* OPCMD_RESET */
    basic_passthrough_connect,         /* OPCMD_CONNECT */
    basic_passthrough_disconnect,      /* OPCMD_DISCONNECT */
    basic_passthrough_buffer_details,  /* OPCMD_BUFFER_DETAILS */
    basic_passthrough_get_data_format, /* OPCMD_DATA_FORMAT */
    basic_passthrough_get_sched_info   /* OPCMD_GET_SCHED_INFO */
};

/** Null terminated operator message handler table for passthrough capabilities */
const opmsg_handler_lookup_table_entry basic_passthrough_opmsg_handler_table[] =
    {{OPMSG_COMMON_ID_GET_CAPABILITY_VERSION,    base_op_opmsg_get_capability_version},
    {OPMSG_COMMON_ID_FADEOUT_ENABLE,             basic_passthrough_opmsg_enable_fadeout},
    {OPMSG_COMMON_ID_FADEOUT_DISABLE,            basic_passthrough_opmsg_disable_fadeout},
    {OPMSG_PASSTHROUGH_ID_CHANGE_INPUT_DATA_TYPE,     basic_passthrough_change_input_data_type},
    {OPMSG_PASSTHROUGH_ID_CHANGE_OUTPUT_DATA_TYPE,    basic_passthrough_change_output_data_type},
    {OPMSG_COMMON_SET_DATA_STREAM_BASED,         basic_passthrough_data_stream_based},
    {OPMSG_COMMON_ID_SET_BUFFER_SIZE,            basic_passthrough_opmsg_set_buffer_size},
    {OPMSG_COMMON_ID_SET_CONTROL,                  basic_passthrough_opmsg_obpm_set_control},
    {OPMSG_COMMON_ID_GET_PARAMS,                   basic_passthrough_opmsg_obpm_get_params},
    {OPMSG_COMMON_ID_GET_DEFAULTS,                 basic_passthrough_opmsg_obpm_get_defaults},
    {OPMSG_COMMON_ID_SET_PARAMS,                   basic_passthrough_opmsg_obpm_set_params},
    {OPMSG_COMMON_ID_GET_STATUS,                   basic_passthrough_opmsg_obpm_get_status},
    {OPMSG_COMMON_ID_SET_UCID,                     basic_passthrough_opmsg_set_ucid},
    {OPMSG_COMMON_ID_GET_LOGICAL_PS_ID,            basic_passthrough_opmsg_get_ps_id},
    {OPMSG_PEQ_ID_LOAD_CONFIG,                     basic_passthrough_opmsg_load_config},
    {OPMSG_COMMON_SET_SAMPLE_RATE,                 basic_passthrough_opmsg_set_sample_rate},
    {0, NULL}};

#ifdef INSTALL_OPERATOR_TTP_PASS
/** TTP passthrough has some extra operator messages, so it gets its own copy of the table */
const opmsg_handler_lookup_table_entry ttp_passthrough_opmsg_handler_table[] =
    {{OPMSG_COMMON_ID_GET_CAPABILITY_VERSION,    base_op_opmsg_get_capability_version},
    {OPMSG_COMMON_ID_FADEOUT_ENABLE,             basic_passthrough_opmsg_enable_fadeout},
    {OPMSG_COMMON_ID_FADEOUT_DISABLE,            basic_passthrough_opmsg_disable_fadeout},
    {OPMSG_PASSTHROUGH_ID_CHANGE_INPUT_DATA_TYPE,     basic_passthrough_change_input_data_type},
    {OPMSG_PASSTHROUGH_ID_CHANGE_OUTPUT_DATA_TYPE,    basic_passthrough_change_output_data_type},
    {OPMSG_COMMON_SET_DATA_STREAM_BASED,         basic_passthrough_data_stream_based},
    {OPMSG_COMMON_SET_TTP_LATENCY,               ttp_passthrough_opmsg_set_ttp_latency},
    {OPMSG_COMMON_SET_LATENCY_LIMITS,            ttp_passthrough_opmsg_set_latency_limits},
    {OPMSG_COMMON_SET_TTP_PARAMS,                ttp_passthrough_opmsg_set_ttp_params},
    {OPMSG_COMMON_SET_SAMPLE_RATE,               ttp_passthrough_opmsg_set_sample_rate},
    {OPMSG_COMMON_ID_SET_BUFFER_SIZE,            basic_passthrough_opmsg_set_buffer_size},
    {OPMSG_COMMON_ID_SET_CONTROL,                  basic_passthrough_opmsg_obpm_set_control},
    {OPMSG_COMMON_ID_GET_PARAMS,                   basic_passthrough_opmsg_obpm_get_params},
    {OPMSG_COMMON_ID_GET_DEFAULTS,                 basic_passthrough_opmsg_obpm_get_defaults},
    {OPMSG_COMMON_ID_SET_PARAMS,                   basic_passthrough_opmsg_obpm_set_params},
    {OPMSG_COMMON_ID_GET_STATUS,                   basic_passthrough_opmsg_obpm_get_status},
    {OPMSG_COMMON_ID_SET_UCID,                     basic_passthrough_opmsg_set_ucid},
    {OPMSG_COMMON_ID_GET_LOGICAL_PS_ID,            basic_passthrough_opmsg_get_ps_id},
    {OPMSG_PEQ_ID_LOAD_CONFIG,                     basic_passthrough_opmsg_load_config},
    {0, NULL}};
#endif /* INSTALL_OPERATOR_TTP_PASS */

/** Constant capability description of basic_passthrough capability */
const CAPABILITY_DATA basic_passthrough_cap_data =
{
    BASIC_PASS_CAP_ID,              /* Capability ID */
    BASIC_PASSTHROUGH_BPT_VERSION_MAJOR, 0, /* Version information - hi and lo parts */
    MAX_CHANS, MAX_CHANS,                           /* Max number of sinks/inputs and sources/outputs */
    &basic_passthrough_handler_table,      /* Pointer to message handler function table */
    basic_passthrough_opmsg_handler_table, /* Pointer to operator message handler function table */
    basic_passthrough_process_data,        /* Pointer to data processing function */
    0,                              /* TODO - Processing time information */
    sizeof(BASIC_PASSTHROUGH_OP_DATA)      /* Size of capability-specific per-instance data */
};

#ifdef INSTALL_OPERATOR_TTP_PASS
#ifndef INSTALL_TTP
#error TTP passthrough capability needs INSTALL_TTP
#endif
/** Constant capability description of passthrough capability with TTP generation */
const CAPABILITY_DATA ttp_passthrough_cap_data =
{
    TTP_PASS_CAP_ID,        /* Capability ID */
    BASIC_PASSTHROUGH_TTP_VERSION_MAJOR, 1,  /* Version information - hi and lo parts */
    MAX_CHANS, MAX_CHANS,                    /* Max number of sinks/inputs and sources/outputs */
    &basic_passthrough_handler_table,       /* Pointer to message handler function table */
    ttp_passthrough_opmsg_handler_table, /* Pointer to operator message handler function table */
    basic_passthrough_process_data,        /* Pointer to data processing function */
    0,                              /* TODO - Processing time information */
    sizeof(BASIC_PASSTHROUGH_OP_DATA)      /* Size of capability-specific per-instance data */
};
#endif /* INSTALL_OPERATOR_TTP_PASS */

/****************************************************************************
Private Function Declarations
*/

/* ******************************* Helper macros ************************************ */

/* return if value is less than BASIC_PASSTHROUGH_DEFAULT_BLOCK_SIZE */
#define EXIT_IF_INSUFFICIENT_DATA(val) \
    do {\
        if ((val) < BASIC_PASSTHROUGH_DEFAULT_BLOCK_SIZE)\
        {\
            return;\
        }\
    } while(0)


/* ********************************** API functions ************************************* */

bool basic_passthrough_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    BASIC_PASSTHROUGH_OP_DATA *opx_data;
    /* call base_op create, which also allocates and fills response message */
    if (!base_op_create(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    opx_data = (BASIC_PASSTHROUGH_OP_DATA *)op_data->extra_op_data;

    opx_data->latency_buffer_size = 0;

#ifdef INSTALL_TTP
    if (op_data->cap_data->id == TTP_PASS_CAP_ID)
    {
        /* Allocate and initialise the TTP data */
        opx_data->time_to_play = ttp_init();
        if (opx_data->time_to_play != NULL)
        {
            ttp_params params;
            ttp_get_default_params(&params, TTP_TYPE_PCM);
            ttp_configure_latency(opx_data->time_to_play, 5000);
            ttp_configure_params(opx_data->time_to_play, &params);
        }
        else
        {
            /* TTP init failed, so fail the operator creation */
            base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            return TRUE;
        }
    }
#endif

    opx_data->ip_format = AUDIO_DATA_FORMAT_FIXP;
    opx_data->op_format = AUDIO_DATA_FORMAT_FIXP;
    opx_data->num_active_chans = 0;
    opx_data->active_chans = 0;
    /* Unless the host says otherwise assume incoming channels are not a stream */
    opx_data->simple_data_test_safe = FALSE;

    switch (op_data->cap_data->id)
    {
        case BASIC_PASS_CAP_ID:
        case TTP_PASS_CAP_ID:
        default:
            opx_data->copy_function = (void *)(&mono_audio_loop);
            break;
    }

    if(!cpsInitParameters(&opx_data->parms_def,BASIC_PASSTHROUGH_GetDefaults(BASIC_PASS_CAP_ID),(unsigned*)(&opx_data->params),sizeof(BASIC_PASSTHROUGH_PARAMETERS)))
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    opx_data->ReInitFlag = 1;

    return TRUE;
}


bool basic_passthrough_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
#ifdef INSTALL_TTP
    BASIC_PASSTHROUGH_OP_DATA *opx_data = (BASIC_PASSTHROUGH_OP_DATA *)op_data->extra_op_data;
    /* Time to play pointer will only be non-NULL if this is the TTP_PASS capability
     * It's safe to call ttp_free whether it's NULL or not.
     */
    ttp_free(opx_data->time_to_play);
#endif
    /* call base_op destroy that creates and fills response message, too */
    return base_op_destroy(op_data, message_data, response_id, response_data);
}


bool basic_passthrough_reset(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    if (!base_op_reset(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    return TRUE;
}

bool basic_passthrough_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    BASIC_PASSTHROUGH_OP_DATA *opx_data = (BASIC_PASSTHROUGH_OP_DATA *)(op_data->extra_op_data);
    unsigned terminal_id = ((unsigned*)message_data)[0];
    unsigned i, channel;

    *response_id = OPCMD_CONNECT;
    if (!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        /* Shouldn't change anything if there is not enough memory for the response.*/
        return FALSE;
    }

    channel = terminal_id & (~TERMINAL_SINK_MASK);
    if (channel >= MAX_CHANS)
    {
        base_op_change_response_status(response_data, STATUS_INVALID_CMD_PARAMS);
        return TRUE;
    }

    /* A channel structure is needed for the buffer, find the existing channel
     * structure in the list otherwise create a new one.
     */
    for (i = 0; i < MAX_CHANS; i++)
    {
        if (opx_data->channel[i] != NULL )
        {
            if (channel == opx_data->channel[i]->channel_num)
            {
                unsigned j;
                /* There is already a structure representing this channel so populate
                 * it with this connection. If it's already populated this is an error
                 */
                if (terminal_id & TERMINAL_SINK_MASK)
                {
                    if (opx_data->channel[i]->ip_buffer == NULL)
                    {
                        opx_data->channel[i]->ip_buffer = (tCbuffer*)(((uintptr_t *)message_data)[1]);
#ifdef INSTALL_METADATA
                        if (opx_data->metadata_ip_buffer == NULL)
                        {
                            if (buff_has_metadata(opx_data->channel[i]->ip_buffer))
                            {
                                opx_data->metadata_ip_buffer = opx_data->channel[i]->ip_buffer;
                            }
                        }
#endif /* INSTALL_METADATA */
                    }
                    else
                    {
                        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
                        return TRUE;
                    }
                }
                else
                {
                    if (opx_data->channel[i]->op_buffer == NULL)
                    {
                        opx_data->channel[i]->op_buffer = (tCbuffer*)(((uintptr_t *)message_data)[1]);
#ifdef INSTALL_METADATA
                        if (opx_data->metadata_op_buffer == NULL)
                        {
                            if (buff_has_metadata(opx_data->channel[i]->op_buffer))
                            {
                                opx_data->metadata_op_buffer = opx_data->channel[i]->op_buffer;
                                /* Set the usable octets for the metadata. */
                                buff_metadata_set_usable_octets(opx_data->metadata_op_buffer,
                                        get_octets_per_word(opx_data->op_format));
                            }
                        }
#endif /* INSTALL_METADATA */
                    }
                    else
                    {
                        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
                        return TRUE;
                    }
                }

                /* This channel is now active, there could be other inactive
                 * channels above it in the list so it needs moving up if there
                 * are [ definition of active appears to be a single channel, which
                 * is banned, this could be confusing if add all IP then some OP, but
                 * probably OK - as for the SINK/SRC we first add a new channel and then
                 * the 2nd time around we come here, and have added the other ]
                 */
                for (j = i; j != 0; j --)
                {
                    if ((opx_data->channel[j-1] != NULL) &&
                        (opx_data->channel[j-1]->ip_buffer != NULL ||
                         opx_data->channel[j-1]->op_buffer != NULL ))
                    {
                        /* Next channel up is an active channel */
                        break;
                    }
                }
                if (i != j)
                {
                    /* Swap the last inactive channel with the newly active channel.  */
                    PASSTHROUGH_CHANNEL *activate = opx_data->channel[i];
                    opx_data->channel[i] = opx_data->channel[j];
                    opx_data->channel[j] = activate;
                }

                /* Mark that this channel is now active */
                opx_data->num_active_chans++;
                opx_data->active_chans |= CHANNEL_MASK(channel);

                break;
            }
            /* This isn't the right channel so keep looking */
        }
        else
        {
            /* There channel doesn't have a structure yet so create one and
             * populate it. It starts at the bottom of the list as its inactive
             * until both terminals are connected. */
            opx_data->channel[i] = xzpnew(PASSTHROUGH_CHANNEL);
            if (opx_data->channel[i] != NULL)
            {
                opx_data->channel[i]->channel_num = channel;
                opx_data->channel[i]->fadeout_parameters.fadeout_state = NOT_RUNNING_STATE;
                if (terminal_id & TERMINAL_SINK_MASK)
                {
                    opx_data->channel[i]->ip_buffer = (tCbuffer*)(((uintptr_t *)message_data)[1]);
#ifdef INSTALL_METADATA
                    if (opx_data->metadata_ip_buffer == NULL)
                    {
                        if (buff_has_metadata(opx_data->channel[i]->ip_buffer))
                        {
                            opx_data->metadata_ip_buffer = opx_data->channel[i]->ip_buffer;
                        }
                    }
#endif /* INSTALL_METADATA */
                }
                else
                {
                    opx_data->channel[i]->op_buffer = (tCbuffer*)(((uintptr_t *)message_data)[1]);
#ifdef INSTALL_METADATA
                    if (opx_data->metadata_op_buffer == NULL)
                    {
                        if (buff_has_metadata(opx_data->channel[i]->op_buffer))
                        {
                            opx_data->metadata_op_buffer = opx_data->channel[i]->op_buffer;
                            /* Set the usable octets for the metadata. */
                            buff_metadata_set_usable_octets(opx_data->metadata_op_buffer,
                                    get_octets_per_word(opx_data->op_format));

                        }
                    }
#endif /* INSTALL_METADATA */
                }
            }
            else
            {
                base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            }

            break;
        }
    }

    return TRUE;
}


bool basic_passthrough_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    BASIC_PASSTHROUGH_OP_DATA *opx_data = (BASIC_PASSTHROUGH_OP_DATA *)(op_data->extra_op_data);
    unsigned terminal_id = ((unsigned*)message_data)[0];
    unsigned i, channel;

    *response_id = OPCMD_DISCONNECT;
    if (!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        /* Shouldn't change anything if there is not enough memory for the response.*/
        return FALSE;
    }

    channel = terminal_id & (~TERMINAL_SINK_MASK);
    if (channel >= MAX_CHANS)
    {
        base_op_change_response_status(response_data, STATUS_INVALID_CMD_PARAMS);
        return TRUE;
    }

    /* To perform a disconnect find the appropriate channel from the list. Make
     * sure that channel is in the inactive part of the list, and then mark the
     * terminal's buffer as NULL. If that makes the channel empty then free
     * the memory for that channel.
     */
    for (i = 0; i < MAX_CHANS; i++)
    {
        if (opx_data->channel[i] != NULL)
        {
            if (channel == opx_data->channel[i]->channel_num)
            {
                /* Channel structure found */
                if (terminal_id & TERMINAL_SINK_MASK)
                {
                    if (opx_data->channel[i]->ip_buffer != NULL)
                    {
#ifdef INSTALL_METADATA
                        if (opx_data->metadata_ip_buffer == opx_data->channel[i]->ip_buffer)
                        {
                            unsigned j;
                            bool found_alternative = FALSE;
                            for (j = 0; j < MAX_CHANS; j++)
                            {
                                if (j == i)
                                {
                                    continue;
                                }
                                if (opx_data->channel[j] != NULL &&
                                   (opx_data->channel[j]->ip_buffer != NULL &&
                                    buff_has_metadata(opx_data->channel[j]->ip_buffer)))
                                {
                                    opx_data->metadata_ip_buffer = opx_data->channel[j]->ip_buffer;
                                    found_alternative = TRUE;
                                    break;
                                }
                            }
                            if (!found_alternative)
                            {
                                opx_data->metadata_ip_buffer = NULL;
                            }
                        }
#endif /* INSTALL_METADATA */

                        if (opx_data->channel[i]->op_buffer != NULL )
                        {
                            unsigned last_active_chan = opx_data->num_active_chans - 1;
                            PASSTHROUGH_CHANNEL *deactivate = opx_data->channel[i];

                            /* It's possible this was the last active entry in
                             * the list anyway in which case we don't need to do
                             * an expensive swap with interrupts blocked to stop
                             * a channel being executed twice.
                             */
                            if (i == last_active_chan)
                            {
                                LOCK_INTERRUPTS;
                                opx_data->channel[i] = opx_data->channel[last_active_chan];
                                opx_data->channel[last_active_chan] = deactivate;
                                UNLOCK_INTERRUPTS;
                            }
                            /* Deactivate the channel from being processed before
                             * nulling the buffer pointer. */
                            opx_data->num_active_chans--;
                            opx_data->active_chans &= ~CHANNEL_MASK(channel);
                            deactivate->ip_buffer = NULL;
                        }
                        else
                        {
                            /* If this channel no longer has buffers associated with it the
                             * channel structure can be safely freed. The array is an ordered
                             * list so move all the other channels up the list. The channel
                             * must have already been inactive and near the bottom of the list
                             * where other inactive channels live so as long as the list is
                             * reordered before we release the memory then there is no race
                             * condition if the operator is running so interrupts don't need
                             * to be blocked here.
                             */
                            PASSTHROUGH_CHANNEL *old_channel = opx_data->channel[i];

                            for (; i < MAX_CHANS - 1; i++)
                            {
                                opx_data->channel[i] = opx_data->channel[i + 1];
                            }
                            /* The last entry has to be NULL as we've removed an element */
                            opx_data->channel[MAX_CHANS - 1] = NULL;
                            pdelete(old_channel);
                        }
                    }
                    else
                    {
                        /* Fail here */
                        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
                    }
                }
                else
                {
                    if (opx_data->channel[i]->op_buffer != NULL)
                    {
#ifdef INSTALL_METADATA
                        if (opx_data->metadata_op_buffer == opx_data->channel[i]->op_buffer)
                        {
                            unsigned j;
                            bool found_alternative = FALSE;
                            for (j = 0; j < MAX_CHANS; j++)
                            {
                                if (j == i)
                                {
                                    continue;
                                }
                                if (opx_data->channel[j] != NULL &&
                                   (opx_data->channel[j]->op_buffer != NULL &&
                                    buff_has_metadata(opx_data->channel[j]->op_buffer)))
                                {
                                    opx_data->metadata_op_buffer = opx_data->channel[j]->op_buffer;
                                    found_alternative = TRUE;
                                    break;
                                }
                            }
                            if (!found_alternative)
                            {
                                opx_data->metadata_op_buffer = NULL;
                            }
                        }
#endif /* INSTALL_METADATA */

                        /* If the input buffer is still connected put this channel
                         * in the deactivated part of the channel list. If it
                         * isn't then remove the structure entirely.
                         */
                        if (opx_data->channel[i]->ip_buffer)
                        {
                            unsigned last_active_chan = opx_data->num_active_chans - 1;
                            PASSTHROUGH_CHANNEL *deactivate = opx_data->channel[i];

                            /* It's possible this was the last active entry in
                             * the list anyway in which case we don't need to do
                             * an expensive swap with interrupts blocked to stop
                             * a channel being executed twice.
                             */
                            if (i == last_active_chan)
                            {
                                LOCK_INTERRUPTS;
                                opx_data->channel[i] = opx_data->channel[last_active_chan];
                                opx_data->channel[last_active_chan] = deactivate;
                                UNLOCK_INTERRUPTS;
                            }
                            /* Deactivate the channel from being processed before
                             * nulling the buffer pointer. */
                            opx_data->num_active_chans--;
                            opx_data->active_chans &= ~CHANNEL_MASK(channel);
                            deactivate->op_buffer = NULL;
                        }
                        else
                        {
                            /* If this channel no longer has buffers associated with it the
                             * channel structure can be safely freed. The array is an ordered
                             * list so move all the other channels up the list. The channel
                             * must have already been inactive and near the bottom of the list
                             * where other inactive channels live so as long as the list is
                             * reordered before we release the memory then there is no race
                             * condition if the operator is running so interrupts don't need
                             * to be blocked here.
                             */
                            PASSTHROUGH_CHANNEL *old_channel = opx_data->channel[i];

                            for (; i < MAX_CHANS - 1; i++)
                            {
                                opx_data->channel[i] = opx_data->channel[i + 1];
                            }
                            /* The last entry has to be NULL as we've removed an element */
                            opx_data->channel[MAX_CHANS - 1] = NULL;
                            pdelete(old_channel);
                        }
                    }
                    else
                    {
                        /* Fail here */
                        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
                    }
                }
                /* Found the channel so done. */
                break;
            }
        }
        else
        {
            /* This terminal isn't connected so report an error */
            base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        }
    }

    return TRUE;
}


bool basic_passthrough_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
#if !defined(DISABLE_IN_PLACE) || defined(INSTALL_METADATA)
    unsigned terminal_id = ((unsigned *)message_data)[0];
    BASIC_PASSTHROUGH_OP_DATA *opx_data = (BASIC_PASSTHROUGH_OP_DATA *)(op_data->extra_op_data);
#ifndef DISABLE_IN_PLACE
    unsigned i;
    unsigned channel;
    unsigned buffer_size;

#endif /* DISABLE_IN_PLACE */
#endif /* !defined(DISABLE_IN_PLACE) || defined(INSTALL_METADATA) */

    if (!base_op_buffer_details(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    buffer_size = ((OP_BUF_DETAILS_RSP*)*response_data)->b.buffer_size;

#ifdef INSTALL_METADATA
    {
        /* If an input/output connection is already present and has metadata then
         * we are obliged to return that buffer so that metadata can be shared
         * between channels. */
        tCbuffer *meta_buff;
        if ((terminal_id & TERMINAL_SINK_MASK) == TERMINAL_SINK_MASK)
        {
            meta_buff = opx_data->metadata_ip_buffer;
        }
        else
        {
            meta_buff = opx_data->metadata_op_buffer;
        }

        ((OP_BUF_DETAILS_RSP*)*response_data)->metadata_buffer = meta_buff;
        ((OP_BUF_DETAILS_RSP*)*response_data)->supports_metadata = TRUE;
    }
#endif /* INSTALL_METADATA */

#ifndef DISABLE_IN_PLACE
    channel = terminal_id & (~TERMINAL_SINK_MASK);
    if (channel >= MAX_CHANS)
    {
        base_op_change_response_status(response_data, STATUS_INVALID_CMD_PARAMS);
        return TRUE;
    }

    ((OP_BUF_DETAILS_RSP*)*response_data)->runs_in_place = TRUE;
    if (opx_data->latency_buffer_size != 0)
    {
        /* Use the requested latency buffer size */
        ((OP_BUF_DETAILS_RSP*)*response_data)->b.in_place_buff_params.size = opx_data->latency_buffer_size;
        opx_data->kick_on_full_output = TRUE;
    }
    else
    {
        /* return the calculated size from base_op_buffer_details */
        ((OP_BUF_DETAILS_RSP*)*response_data)->b.in_place_buff_params.size = buffer_size;
    }
    ((OP_BUF_DETAILS_RSP*)*response_data)->b.in_place_buff_params.buffer = NULL;

    /* search for the channel. */
    for (i = 0; i < MAX_CHANS; i++)
    {
        if (opx_data->channel[i])
        {
            if (channel == opx_data->channel[i]->channel_num)
            {

                if (terminal_id & TERMINAL_SINK_MASK)
                {

                    /*input terminal. give the output buffer for the channel */
                    ((OP_BUF_DETAILS_RSP*)*response_data)->b.in_place_buff_params.buffer =
                            opx_data->channel[i]->op_buffer;
                }
                else
                {
                    /*output terminal. give the input buffer for the channel */
                    ((OP_BUF_DETAILS_RSP*)*response_data)->b.in_place_buff_params.buffer =
                            opx_data->channel[i]->ip_buffer;
                }
            }
        }
    }

    /* Choose the opposite terminal. */
    ((OP_BUF_DETAILS_RSP*)*response_data)->b.in_place_buff_params.in_place_terminal = ((unsigned *)message_data)[0] ^TERMINAL_SINK_MASK;

    L4_DBG_MSG1("basic_passthrough_buffer_details  %d \n", ((OP_BUF_DETAILS_RSP*)*response_data)->b.in_place_buff_params.size);
#else
    if (((terminal_id & TERMINAL_SINK_MASK) == 0) && (opx_data->latency_buffer_size != 0))
    {
        /* Output, use the latency buffer size */
        ((OP_BUF_DETAILS_RSP*)*response_data)->b.buffer_size = opx_data->latency_buffer_size;
    }
    L4_DBG_MSG1("basic_passthrough_buffer_details  %d \n", ((OP_BUF_DETAILS_RSP*)*response_data)->b.buffer_size);
#endif /* DISABLE_IN_PLACE */

    return TRUE;
}


bool basic_passthrough_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
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
    resp->block_size = BASIC_PASSTHROUGH_DEFAULT_BLOCK_SIZE;
    resp->run_period = 0;

    *response_data = resp;
    return TRUE;
}


bool basic_passthrough_get_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    BASIC_PASSTHROUGH_OP_DATA *opx_data;
    if (!base_op_get_data_format(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    opx_data = (BASIC_PASSTHROUGH_OP_DATA *)(op_data->extra_op_data);
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


/* ************************************* Data processing-related functions and wrappers **********************************/
#ifdef INSTALL_OPERATOR_TTP_PASS

/* Transport metadata from input to output,
 * either adding time-to-play on existing tags or creating new ones if none are present
 * This inevitably looks very similar to metadata_strict_transport
 */
static void metadata_transport_with_ttp(BASIC_PASSTHROUGH_OP_DATA *opx_data, unsigned samples)
{
    metadata_tag *mtag;
    unsigned b4idx, afteridx;
    if (opx_data->metadata_ip_buffer != NULL)
    {
        /* transport metadata, first (attempt to) consume tag associated with src */
        mtag = buff_metadata_remove(opx_data->metadata_ip_buffer, samples * OCTETS_PER_SAMPLE, &b4idx, &afteridx);
    }
    else
    {
        /* Create a new tag for the output */
        b4idx = 0;
        afteridx = samples * OCTETS_PER_SAMPLE;
        mtag = buff_metadata_new_tag();
        if (mtag != NULL)
        {
            mtag->length = samples * OCTETS_PER_SAMPLE;
        }
    }

    if (mtag != NULL)
    {
        ttp_status status;
        metadata_tag *list_tag = mtag->next;
        unsigned list_octets = mtag->length;
        TIME ttp_reference_time;
        if (IS_TIME_OF_ARRIVAL_TAG(mtag))
        {
            /* We have incoming ToA, so use that as the TTP reference */
            ttp_reference_time = mtag->timestamp;
        }
        else
        {
            /* No ToA, so just use the current time */
            ttp_reference_time = hal_get_time();
        }

        ttp_update_ttp(opx_data->time_to_play, ttp_reference_time, samples, &status);
        /* Populate the metadata tag from the TTP status */
        METADATA_TIME_OF_ARRIVAL_UNSET(mtag);
        ttp_utils_populate_tag(mtag, &status);
        /* Make sure only one tag gets the stream start flag */
        status.stream_restart = FALSE;

        /* In case there were multiple metadata tags on the input,
         * extrapolate the timestamps to any subsequent tags
         */
        while (list_tag != NULL)
        {
            status.ttp = ttp_get_next_timestamp(mtag->timestamp, list_octets / OCTETS_PER_SAMPLE, opx_data->sample_rate, status.sp_adjustment);
            METADATA_TIME_OF_ARRIVAL_UNSET(list_tag);
            ttp_utils_populate_tag(list_tag, &status);
            list_octets += list_tag->length;
            list_tag = list_tag->next;
        }

    }

    if (opx_data->metadata_op_buffer != NULL)
    {
        buff_metadata_append(opx_data->metadata_op_buffer, mtag, b4idx, afteridx);
    }
    else
    {
        buff_metadata_tag_list_delete(mtag);
    }

}

#endif /* INSTALL_OPERATOR_TTP_PASS */


RUN_FROM_PM_RAM
void basic_passthrough_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
#if BASIC_PASSTHROUGH_DEFAULT_BLOCK_SIZE != 1
#error The process data function kick backwards logic is only valid when block size is 1
#endif
    unsigned i, data_to_process, num_active_chans, output_space;
    /* Alias channels up front as all paths need it. */
    BASIC_PASSTHROUGH_OP_DATA *opx_data = (BASIC_PASSTHROUGH_OP_DATA *)(op_data->extra_op_data);
    PASSTHROUGH_CHANNEL **channels = opx_data->channel;
    bool buffers_16bit_ex = FALSE;
    unsigned (*input_samples_fn)(tCbuffer *cbuffer) = cbuffer_calc_amount_data_in_words;
    unsigned (*output_space_fn)(tCbuffer *cbuffer) = cbuffer_calc_amount_space_in_words;

    if (opx_data->num_active_chans == 0)
    {
        /* Unlike other capabilities, basic_passthrough can be started even if
        it is not connected properly. This is used for test purposes. The
        following early return prevents dereferencing NULL pointers. */
        return;
    }
    else if (opx_data->simple_data_test_safe)
    {
        num_active_chans = 1;
    }
    else
    {
        /* Alias the thing we want to iterate in the loop as this stops the compiler
         * deciding to access it in a suboptimal way.
         */
        num_active_chans = opx_data->num_active_chans;
    }

#if defined(INSTALL_CBUFFER_EX)
    if (opx_data->ip_format != opx_data->op_format)
    {
        fault_diatribe(FAULT_AUDIO_PASSTHROUGH_FORMAT_MISMATCH,op_data->id);
    }
    else
    {
        /* Check if the buffers need special processing
         * Checks during connect ensure that all of the buffers are the same
         * configuration, so we just need to check channel 0.
         */
        switch (opx_data->ip_format)
        {
        case AUDIO_DATA_FORMAT_16_BIT:
        case AUDIO_DATA_FORMAT_16_BIT_WITH_METADATA:
            buffers_16bit_ex = TRUE;
            input_samples_fn = cbuffer_calc_amount_data_ex;
            output_space_fn = cbuffer_calc_amount_space_ex;
            break;

        default:
            buffers_16bit_ex = FALSE;
            break;
        }
    }
#endif /* INSTALL_CBUFFER_EX */

    /* work out minimum amount of input data and output space this is what can
     * be processed and ensure synchronisation between channels. */
    data_to_process = input_samples_fn(channels[0]->ip_buffer);
    EXIT_IF_INSUFFICIENT_DATA(data_to_process);

    output_space = output_space_fn(channels[0]->op_buffer);

    if (opx_data->kick_on_full_output && (data_to_process > output_space))
    {
        /* Kick forwards to keep data flowing in case the next operator is a splitter */
        touched->sources = opx_data->active_chans;
    }

    EXIT_IF_INSUFFICIENT_DATA(output_space);

    for (i = 1; i < num_active_chans; i++)
    {
        /* Alias the channel as a hint to the compiler that it's useful (also
         * makes the code more readable) */
        PASSTHROUGH_CHANNEL *this_channel = channels[i];

        /* The Kymera scheduling strategy relies on operators yielding quickly
         * when they have nothing to do. The check is almost free so if there is
         * insufficient data or space then return.
         */
        unsigned amount = input_samples_fn(this_channel->ip_buffer);
        EXIT_IF_INSUFFICIENT_DATA(amount);
        data_to_process = MIN(data_to_process,amount);

        amount = output_space_fn(this_channel->op_buffer);
        EXIT_IF_INSUFFICIENT_DATA(amount);
        output_space = MIN(output_space,amount);
    }

    /* We have got this far, so we have something to do on every output */
    touched->sources = opx_data->active_chans;

    /* Now determine if we will empty (at least one) input */
    if (output_space >= data_to_process)
    {
        touched->sinks = touched->sources;
    }
    else
    {
        data_to_process = output_space;
    }

    /* Is fadeout enabled? if yes, do it on the current input data */
    if(   channels[0]->fadeout_parameters.fadeout_state != NOT_RUNNING_STATE
       && !buffers_16bit_ex)
    {
        bool fadeout_done = FALSE;

        /* The number of active channels is now known it's the output of the last
         * loop. So no need to check how many channels to process. Unless i == MAX_CHANS
         * add 1 as the loop was broken out of.
         */
        for (i = 0; i < opx_data->num_active_chans; i++)
        {
            /* the wrapper below takes operator data - this might be lifted out to a common function */
            if(mono_cbuffer_fadeout(channels[i]->ip_buffer, data_to_process,
                                       &(channels[i]->fadeout_parameters)))
            {
                fadeout_done = TRUE;
            }
        }

        if (fadeout_done)
        {
            common_send_simple_unsolicited_message(op_data, OPMSG_REPLY_ID_FADEOUT_DONE);
        }
    }
#ifdef INSTALL_OPERATOR_TTP_PASS
    if (op_data->cap_data->id == TTP_PASS_CAP_ID)
    {
        metadata_transport_with_ttp(opx_data, data_to_process);
    }
    else
#endif /* INSTALL_OPERATOR_TTP_PASS */

#ifdef INSTALL_METADATA
    {
        /* Propagate any metadata to the output. Any handling of its presence or
         * not is handled by the metadata library */
        unsigned data_size = buffers_16bit_ex ? 1 /* octets */
                                                    : buff_metadata_get_usable_octets(opx_data->metadata_ip_buffer);

        metadata_strict_transport(opx_data->metadata_ip_buffer, opx_data->metadata_op_buffer,
                                  data_to_process * data_size);
    }
#endif  /* INSTALL_METADATA */

    /* Now copy the actual data across channels */
#if defined(INSTALL_CBUFFER_EX)
    if (buffers_16bit_ex)
    {
        /* call ASM function */
        basic_passthrough_processing_ex(opx_data, data_to_process);
    }
    else
#endif /* INSTALL_CBUFFER_EX */
    {
        /* call ASM function */
        basic_passthrough_processing(opx_data, data_to_process);
    }
}


#if defined(INSTALL_CBUFFER_EX)
/* C version of the basic_passthrough_processing loop.
 * We don't need/want to worry about the copy function in this case as
 * the function is only called if we have _ex buffers.
 */
static void basic_passthrough_processing_ex(BASIC_PASSTHROUGH_OP_DATA *op_data, unsigned octets)
{
    PASSTHROUGH_CHANNEL **channels = op_data->channel;
    unsigned i;

    for (i = 0;i < op_data->num_active_chans;i++)
    {
        PASSTHROUGH_CHANNEL *channel = channels[i];
        if (channel->op_buffer->base_addr == channel->ip_buffer->base_addr)
        {
            /* In place buffer, just advance the pointers */
            cbuffer_advance_write_ptr_ex(channel->op_buffer, octets);
            cbuffer_advance_read_ptr_ex(channel->ip_buffer, octets);
        }
        else
        {
            cbuffer_copy_16bit_be_zero_shift_ex(channel->op_buffer,channel->ip_buffer,octets);
        }
    }
}
#endif

/* **************************** Operator message handlers ******************************** */

bool basic_passthrough_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    BASIC_PASSTHROUGH_OP_DATA     *op_extra_data = (BASIC_PASSTHROUGH_OP_DATA*)op_data->extra_op_data;
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

        cntrl_value &= 0xFF;
        if (cntrl_value >= BASIC_PASSTHROUGH_SYSMODE_MAX_MODES)
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
            op_extra_data->Ovr_Control = (cntrl_src == CPS_SOURCE_OBPM_DISABLE) ?  0 : BASIC_PASSTHROUGH_CONTROL_MODE_OVERRIDE;
            op_extra_data->Obpm_mode = cntrl_value;
        }
    }

    if(op_extra_data->Ovr_Control & BASIC_PASSTHROUGH_CONTROL_MODE_OVERRIDE)
    {
       op_extra_data->Cur_mode = op_extra_data->Obpm_mode;
    }
    else
    {
      op_extra_data->Cur_mode = op_extra_data->Host_mode;
    }

    cps_response_set_result(resp_data,result);

    /* Set the Reinit flag after setting the paramters */
    if (result == OPMSG_RESULT_STATES_NORMAL_STATE)
    {
        op_extra_data->ReInitFlag = 1;
    }
    return TRUE;
}

bool basic_passthrough_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    BASIC_PASSTHROUGH_OP_DATA   *op_extra_data = (BASIC_PASSTHROUGH_OP_DATA*)op_data->extra_op_data;

    return cpsGetParameterMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);
}

bool basic_passthrough_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    BASIC_PASSTHROUGH_OP_DATA   *op_extra_data = (BASIC_PASSTHROUGH_OP_DATA*)op_data->extra_op_data;

    return cpsGetDefaultsMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);
}

bool basic_passthrough_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    BASIC_PASSTHROUGH_OP_DATA   *op_extra_data = (BASIC_PASSTHROUGH_OP_DATA*)op_data->extra_op_data;
    bool retval;

    retval = cpsSetParameterMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);

    /* Set the Reinit flag after setting the paramters */
    op_extra_data->ReInitFlag = 1;

    return retval;
}

bool basic_passthrough_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    BASIC_PASSTHROUGH_OP_DATA   *op_extra_data = (BASIC_PASSTHROUGH_OP_DATA*)op_data->extra_op_data;
    unsigned  *resp;

    if(!common_obpm_status_helper(message_data,resp_length,resp_data,sizeof(BASIC_PASSTHROUGH_STATISTICS),&resp))
    {
         return FALSE;
    }

    if(resp)
    {
    resp = cpsPackWords(&op_extra_data->Cur_mode, &op_extra_data->peak_level_1, resp);
    resp = cpsPackWords(&op_extra_data->peak_level_2, &op_extra_data->peak_level_3, resp);
    resp = cpsPackWords(&op_extra_data->peak_level_4, &op_extra_data->peak_level_5, resp);
    resp = cpsPackWords(&op_extra_data->peak_level_6, &op_extra_data->peak_level_7, resp);
    cpsPackWords((unsigned*)&op_extra_data->peak_level_8, &op_extra_data->Ovr_Control, resp);
    op_extra_data->peak_level_1 = 0;
    op_extra_data->peak_level_2 = 0;
    op_extra_data->peak_level_3 = 0;
    op_extra_data->peak_level_4 = 0;
    op_extra_data->peak_level_5 = 0;
    op_extra_data->peak_level_6 = 0;
    op_extra_data->peak_level_7 = 0;
    op_extra_data->peak_level_8 = 0;
    }

    return TRUE;
}


bool basic_passthrough_opmsg_set_ucid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    BASIC_PASSTHROUGH_OP_DATA   *op_extra_data = (BASIC_PASSTHROUGH_OP_DATA*)op_data->extra_op_data;
    PS_KEY_TYPE key;
    bool retval;

    retval = cpsSetUcidMsgHandler(&op_extra_data->parms_def,message_data,resp_length,resp_data);

    key = MAP_CAPID_UCID_SBID_TO_PSKEYID(op_data->cap_data->id, op_extra_data->parms_def.ucid, OPMSG_P_STORE_PARAMETER_SUB_ID);
    ps_entry_read((void*)op_data,key,PERSIST_ANY,ups_params_basic_passthrough);

    return retval;
}

bool basic_passthrough_opmsg_get_ps_id(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    BASIC_PASSTHROUGH_OP_DATA   *op_extra_data = (BASIC_PASSTHROUGH_OP_DATA*)op_data->extra_op_data;

    return cpsGetUcidMsgHandler(&op_extra_data->parms_def,CAP_ID_PEQ,message_data,resp_length,resp_data);
}


bool basic_passthrough_opmsg_load_config(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    return(TRUE);
}

bool basic_passthrough_opmsg_set_sample_rate(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    BASIC_PASSTHROUGH_OP_DATA *p_ext_data = (BASIC_PASSTHROUGH_OP_DATA *)(op_data->extra_op_data);

    /* We received client ID, length and then opmsgID and OBPM params */
    p_ext_data->sample_rate = 25 * ((unsigned*)message_data)[3];
    p_ext_data->ReInitFlag = 1;

    return(TRUE);
}

bool ups_params_basic_passthrough(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank,
                 uint16 length,  unsigned* data, STATUS_KYMERA status,uint16 extra_status_info)
{
    /* returns true if succesful, false if failed */
    BASIC_PASSTHROUGH_OP_DATA   *op_extra_data = (BASIC_PASSTHROUGH_OP_DATA*)((OPERATOR_DATA*)instance_data)->extra_op_data;

    cpsSetParameterFromPsStore(&op_extra_data->parms_def,length,data,status);

    /* Set the Reinit flag after setting the paramters */
    op_extra_data->ReInitFlag = 1;

    return(TRUE);
}
/* **************************** Operator message handlers ******************************** */

static bool set_fadeout_state(OPERATOR_DATA *op_data, FADEOUT_STATE state)
{
    unsigned i;
    BASIC_PASSTHROUGH_OP_DATA *opx_data = (BASIC_PASSTHROUGH_OP_DATA *)(op_data->extra_op_data);

    /* Fadeout is only possible when the input data format is audio */
    if (AUDIO_DATA_FORMAT_FIXP != opx_data->ip_format)
    {
        return FALSE;
    }

    for (i = 0; i < MAX_CHANS; i++)
    {
        /* There are no more active channels if an empty channel entry is reached */
        if (!opx_data->channel[i])
        {
            break;
        }
        /* if input terminal has audio data type, extra op data is allocated and
         * fadeout is not already enabled/running */
        common_set_fadeout_state(&opx_data->channel[i]->fadeout_parameters, state);
    }
    return TRUE;
}

bool basic_passthrough_opmsg_enable_fadeout(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    return set_fadeout_state(op_data, RUNNING_STATE);
}


bool basic_passthrough_opmsg_disable_fadeout(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    return set_fadeout_state(op_data, NOT_RUNNING_STATE);
}

/**
 * \brief Helper function to modify the terminal data format of basic passthrough.
 *
 * \param  op_data  Pointer to the operator structure.
 * \param  is_input  boolean indicating whether it is the input/output terminal is to be changed.
 * \param  message_data  Pointer to message structure.
 * \param  resp_length  Pointer to response message length in words
 * \param  resp_data  Pointer to a pointer to response message structure, allocated in this function.
 */
static bool common_change_terminal_data_type(OPERATOR_DATA *op_data, bool is_input, void *message_data)
{
    /* success/failure outcome will be signalled to caller via return value */
    if(op_data->state != OP_NOT_RUNNING)
    {
        /* Let opmsg handler pick up the pieces - it will construct a "command failed" response.
         * No extra info is given from this level.
         */
        return FALSE;
    }

    if(is_input)
    {
        /* 4th field in message is the data format */
        ((BASIC_PASSTHROUGH_OP_DATA*)(op_data->extra_op_data))->ip_format = ((unsigned*)message_data)[3];
    }
    else
    {
        ((BASIC_PASSTHROUGH_OP_DATA*)(op_data->extra_op_data))->op_format = ((unsigned*)message_data)[3];
    }

    /* here we don't validate data type - other ops may well do */
    return TRUE;
}


/*
 * basic_passthrough_change_single_input_data_type
 */
bool basic_passthrough_change_input_data_type(OPERATOR_DATA *op_data, void *message_data,
                                            unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    /* The input data type determines whether to perform a straight copy
     * or apply a gain, if this is a basic passthrough operator.
     * Audio = apply a gain.
     */
    if ((BASIC_PASS_CAP_ID == op_data->cap_data->id) || (TTP_PASS_CAP_ID == op_data->cap_data->id))
    {
        if (AUDIO_DATA_FORMAT_FIXP == ((unsigned*)message_data)[3])
        {
            ((BASIC_PASSTHROUGH_OP_DATA*)(op_data->extra_op_data))->copy_function = (void *)(&mono_audio_loop);
        }
        else
        {
            ((BASIC_PASSTHROUGH_OP_DATA*)(op_data->extra_op_data))->copy_function = (void *)(&mono_data_loop);
        }
    }
    /* Call with sink terminal 0 (which then basically has ID == TERMINAL_SINK_MASK) */
    return common_change_terminal_data_type(op_data, TRUE, message_data);
}


/*
 * basic_passthrough_change_single_output_data_type
 */
bool basic_passthrough_change_output_data_type(OPERATOR_DATA *op_data, void *message_data,
                                           unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    /* Call with source terminal ID 0 */
    return common_change_terminal_data_type(op_data, FALSE, message_data);
}

/*
 * basic_passthrough_data_stream_based
 */
bool basic_passthrough_data_stream_based(OPERATOR_DATA *op_data, void *message_data,
                                            unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    /* The 3rd message word is a boolean value */
    ((BASIC_PASSTHROUGH_OP_DATA*)(op_data->extra_op_data))->simple_data_test_safe = ((unsigned*)message_data)[3];

    return TRUE;
}

bool basic_passthrough_opmsg_set_buffer_size(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    BASIC_PASSTHROUGH_OP_DATA *opx_data = (BASIC_PASSTHROUGH_OP_DATA *)(op_data->extra_op_data);
    opx_data->latency_buffer_size = OPMSG_FIELD_GET(message_data, OPMSG_COMMON_SET_BUFFER_SIZE, BUFFER_SIZE);

    return TRUE;
}

#ifdef INSTALL_OPERATOR_TTP_PASS

bool ttp_passthrough_opmsg_set_ttp_latency(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    BASIC_PASSTHROUGH_OP_DATA *opx_data = (BASIC_PASSTHROUGH_OP_DATA *)(op_data->extra_op_data);

    opx_data = (BASIC_PASSTHROUGH_OP_DATA *)(op_data->extra_op_data);
    ttp_configure_latency(opx_data->time_to_play, ttp_get_msg_latency(message_data));

    return TRUE;
}

bool ttp_passthrough_opmsg_set_latency_limits(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    BASIC_PASSTHROUGH_OP_DATA *opx_data = (BASIC_PASSTHROUGH_OP_DATA *)(op_data->extra_op_data);
    TIME_INTERVAL min_latency, max_latency;

    opx_data = (BASIC_PASSTHROUGH_OP_DATA *)(op_data->extra_op_data);
    ttp_get_msg_latency_limits(message_data, &min_latency, &max_latency);
    ttp_configure_latency_limits(opx_data->time_to_play, min_latency, max_latency);

    return TRUE;
}

bool ttp_passthrough_opmsg_set_ttp_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    BASIC_PASSTHROUGH_OP_DATA *opx_data = (BASIC_PASSTHROUGH_OP_DATA *)(op_data->extra_op_data);
    ttp_params params;

    ttp_get_msg_params(&params, message_data);
    ttp_configure_params(opx_data->time_to_play, &params);

    return TRUE;
}


bool ttp_passthrough_opmsg_set_sample_rate(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    BASIC_PASSTHROUGH_OP_DATA *opx_data = (BASIC_PASSTHROUGH_OP_DATA *)(op_data->extra_op_data);
    unsigned sample_rate = 25 * OPMSG_FIELD_GET(message_data, OPMSG_COMMON_MSG_SET_SAMPLE_RATE, SAMPLE_RATE);

    opx_data->sample_rate = sample_rate;
    ttp_configure_rate(opx_data->time_to_play, sample_rate);

    opx_data->ReInitFlag = 1;

    return TRUE;

}

#endif /* INSTALL_OPERATOR_TTP_PASS */
