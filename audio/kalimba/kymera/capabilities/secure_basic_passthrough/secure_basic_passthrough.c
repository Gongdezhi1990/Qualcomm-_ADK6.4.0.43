/****************************************************************************
 * Copyright (c) 2013 - 2018 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  secure_basic_passthrough.c
 * \ingroup  operators
 *
 *  Basic passthrough operator
 *
 */
/****************************************************************************
Include Files
*/
#include "secure_basic_passthrough_private.h"
#include "op_msg_helpers.h"
#include "adaptor/adaptor.h"
#include "ps/ps.h"
#include "obpm_prim.h"
#include "common/interface/util.h"

/****************************************************************************
Private Constant Definitions
*/
/* Capability IDs 0xC000-0xC0FF are for private use in customer projects */
#define SECURE_BASIC_PASS_CAP_ID     0xC000
#define SECURE_TTP_PASS_CAP_ID       0xC001

/* Capability IDs 0xC100 -0xFFFF are reserved for use by Qualcomm eXtension
 * programme partners. Contact Qualcomm to get a range reserved for your
 * exclusive use, allowing your capabilities to coexist with others'.
 */

#define SECURE_BASIC_PASSTHROUGH_BPT_VERSION_MAJOR	1
#define SECURE_BASIC_PASSTHROUGH_TTP_VERSION_MAJOR	1

/****************************************************************************
Private Type Definitions
*/

#if defined(INSTALL_CBUFFER_EX)
static void secure_basic_passthrough_processing_ex(SECURE_BASIC_PASSTHROUGH_OP_DATA *op_data, unsigned octets);
#endif

/*****************************************************************************
Private Constant Declarations
*/
/** The basic passthrough capability function handler table */
const handler_lookup_struct secure_basic_passthrough_handler_table =
{
    secure_basic_passthrough_create,          /* OPCMD_CREATE */
    secure_basic_passthrough_destroy,         /* OPCMD_DESTROY */
    base_op_start,                     /* OPCMD_START */
    base_op_stop,                      /* OPCMD_STOP */
    secure_basic_passthrough_reset,           /* OPCMD_RESET */
    secure_basic_passthrough_connect,         /* OPCMD_CONNECT */
    secure_basic_passthrough_disconnect,      /* OPCMD_DISCONNECT */
    secure_basic_passthrough_buffer_details,  /* OPCMD_BUFFER_DETAILS */
    secure_basic_passthrough_get_data_format, /* OPCMD_DATA_FORMAT */
    secure_basic_passthrough_get_sched_info   /* OPCMD_GET_SCHED_INFO */
};

/** Null terminated operator message handler table for passthrough capabilities */
const opmsg_handler_lookup_table_entry secure_basic_passthrough_opmsg_handler_table[] =
    {{OPMSG_COMMON_ID_GET_CAPABILITY_VERSION,         base_op_opmsg_get_capability_version},
    {OPMSG_COMMON_ID_FADEOUT_ENABLE,                  secure_basic_passthrough_opmsg_enable_fadeout},
    {OPMSG_COMMON_ID_FADEOUT_DISABLE,                 secure_basic_passthrough_opmsg_disable_fadeout},
    {OPMSG_PASSTHROUGH_ID_CHANGE_INPUT_DATA_TYPE,     secure_basic_passthrough_change_input_data_type},
    {OPMSG_PASSTHROUGH_ID_CHANGE_OUTPUT_DATA_TYPE,    secure_basic_passthrough_change_output_data_type},
    {OPMSG_COMMON_SET_DATA_STREAM_BASED,              secure_basic_passthrough_data_stream_based},
    {OPMSG_COMMON_ID_SET_BUFFER_SIZE,                 secure_basic_passthrough_opmsg_set_buffer_size},
    {OPMSG_COMMON_ID_SET_UCID,                        secure_basic_passthrough_opmsg_set_ucid},
    {OPMSG_COMMON_ID_GET_LOGICAL_PS_ID,               secure_basic_passthrough_opmsg_get_ps_id},
    {OPMSG_COMMON_SET_SAMPLE_RATE,                    secure_basic_passthrough_opmsg_set_sample_rate},
#if defined(INSTALL_LICENSE_CHECK) && defined(LEGACY_LICENSING)
    {OPMSG_FRAMEWORK_SET_BDADDR,                      secure_basic_passthrough_opmsg_receive_bt_addr},
#endif
    {0, NULL}};

#ifdef INSTALL_OPERATOR_TTP_SECPASS
/** TTP passthrough has some extra operator messages, so it gets its own copy of the table */
const opmsg_handler_lookup_table_entry secure_ttp_passthrough_opmsg_handler_table[] =
    {{OPMSG_COMMON_ID_GET_CAPABILITY_VERSION,    base_op_opmsg_get_capability_version},
    {OPMSG_COMMON_ID_FADEOUT_ENABLE,             secure_basic_passthrough_opmsg_enable_fadeout},
    {OPMSG_COMMON_ID_FADEOUT_DISABLE,            secure_basic_passthrough_opmsg_disable_fadeout},
    {OPMSG_PASSTHROUGH_ID_CHANGE_INPUT_DATA_TYPE,     secure_basic_passthrough_change_input_data_type},
    {OPMSG_PASSTHROUGH_ID_CHANGE_OUTPUT_DATA_TYPE,    secure_basic_passthrough_change_output_data_type},
    {OPMSG_COMMON_SET_DATA_STREAM_BASED,         secure_basic_passthrough_data_stream_based},
    {OPMSG_COMMON_SET_TTP_LATENCY,               secure_ttp_passthrough_opmsg_set_ttp_latency},
    {OPMSG_COMMON_SET_LATENCY_LIMITS,            secure_ttp_passthrough_opmsg_set_latency_limits},
    {OPMSG_COMMON_SET_TTP_PARAMS,                secure_ttp_passthrough_opmsg_set_ttp_params},
    {OPMSG_COMMON_SET_SAMPLE_RATE,               secure_ttp_passthrough_opmsg_set_sample_rate},
    {OPMSG_COMMON_ID_SET_BUFFER_SIZE,            secure_basic_passthrough_opmsg_set_buffer_size},
    {OPMSG_COMMON_ID_SET_UCID,                     secure_basic_passthrough_opmsg_set_ucid},
    {OPMSG_COMMON_ID_GET_LOGICAL_PS_ID,            secure_basic_passthrough_opmsg_get_ps_id},
    {0, NULL}};
#endif /* INSTALL_OPERATOR_TTP_SECPASS */

/** Constant capability description of secure_basic_passthrough capability */
const CAPABILITY_DATA secure_basic_passthrough_cap_data =
{
    SECURE_BASIC_PASS_CAP_ID,                       /* Capability ID */
    SECURE_BASIC_PASSTHROUGH_BPT_VERSION_MAJOR, 0,  /* Version information - hi and lo parts */
    MAX_CHANS, MAX_CHANS,                           /* Max number of sinks/inputs and sources/outputs */
    &secure_basic_passthrough_handler_table,        /* Pointer to message handler function table */
    secure_basic_passthrough_opmsg_handler_table,   /* Pointer to operator message handler function table */
    secure_basic_passthrough_process_data,          /* Pointer to data processing function */
    0,                                              /* reserved */
    sizeof(SECURE_BASIC_PASSTHROUGH_OP_DATA)        /* Size of capability-specific per-instance data */
};
MAP_INSTANCE_DATA(SECURE_BASIC_PASS_CAP_ID, SECURE_BASIC_PASSTHROUGH_OP_DATA)

#ifdef INSTALL_OPERATOR_TTP_SECPASS
#ifndef INSTALL_TTP
#error TTP passthrough capability needs INSTALL_TTP
#endif
/** Constant capability description of passthrough capability with TTP generation */
const CAPABILITY_DATA secure_ttp_passthrough_cap_data =
{
    SECURE_TTP_PASS_CAP_ID,                         /* Capability ID */
    SECURE_BASIC_PASSTHROUGH_TTP_VERSION_MAJOR, 1,  /* Version information - hi and lo parts */
    MAX_CHANS, MAX_CHANS,                           /* Max number of sinks/inputs and sources/outputs */
    &secure_basic_passthrough_handler_table,        /* Pointer to message handler function table */
    secure_ttp_passthrough_opmsg_handler_table,     /* Pointer to operator message handler function table */
    secure_basic_passthrough_process_data,          /* Pointer to data processing function */
    0,                                              /* reserved */
    sizeof(SECURE_BASIC_PASSTHROUGH_OP_DATA)        /* Size of capability-specific per-instance data */
};
MAP_INSTANCE_DATA(SECURE_TTP_PASS_CAP_ID, SECURE_BASIC_PASSTHROUGH_OP_DATA)
#endif /* INSTALL_OPERATOR_TTP_SECPASS */

/****************************************************************************
Private Function Declarations
*/

/* ******************************* Helper macros ************************************ */

/* return if value is less than BASIC_PASSTHROUGH_DEFAULT_BLOCK_SIZE */
#define EXIT_IF_INSUFFICIENT_DATA(val) \
    do {\
        if ((val) < SECURE_BASIC_PASSTHROUGH_DEFAULT_BLOCK_SIZE)\
        {\
            return;\
        }\
    } while(0)

static inline SECURE_BASIC_PASSTHROUGH_OP_DATA *get_instance_data(OPERATOR_DATA *op_data)
{
    return (SECURE_BASIC_PASSTHROUGH_OP_DATA *) base_op_get_instance_data(op_data);
}

/* ********************************** API functions ************************************* */

bool secure_basic_passthrough_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    SECURE_BASIC_PASSTHROUGH_OP_DATA *opx_data = get_instance_data(op_data);
    CAP_ID cap_id = base_op_get_cap_id(op_data);

    /* call base_op create, which also allocates and fills response message */
    if (!base_op_create(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    opx_data->latency_buffer_size = 0;

#ifdef INSTALL_TTP
    if (cap_id == SECURE_TTP_PASS_CAP_ID)
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
    opx_data->copy_function = (void *)(&secure_mono_data_loop);
    opx_data->ucid = 0;

#ifdef INSTALL_LICENSE_CHECK
    /* Start the license request process. First off, ask for */
    /* license stored in a ps key. */
#ifdef LEGACY_LICENSING
    basic_passthrough_retrieve_license_req(op_data);
#else /* LEGACY_LICENSING */
    if (!basic_passthrough_retrieve_license_req(op_data, 0xC000))
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }
#endif /* LEGACY_LICENSING */
#endif
    opx_data->ReInitFlag = 1;

    return TRUE;
}


bool secure_basic_passthrough_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
#if (defined(LEGACY_LICENSING) && defined(INSTALL_LICENSE_CHECK)) || defined(INSTALL_TTP)
    SECURE_BASIC_PASSTHROUGH_OP_DATA *opx_data = get_instance_data(op_data);
#endif

#ifdef INSTALL_TTP
    /* Time to play pointer will only be non-NULL if this is the TTP_PASS capability
     * It's safe to call ttp_free whether it's NULL or not.
     */
    ttp_free(opx_data->time_to_play);
#endif

#ifdef INSTALL_LICENSE_CHECK
#ifdef LEGACY_LICENSING
    if (opx_data->lic.license_key != NULL)
    {
        pfree(opx_data->lic.license_key);
        opx_data->lic.license_key = NULL;
    }
#else /* LEGACY_LICENSING */
    basic_passthrough_release_license();
#endif /* LEGACY_LICENSING */
#endif

    /* call base_op destroy that creates and fills response message, too */
    return base_op_destroy(op_data, message_data, response_id, response_data);
}


bool secure_basic_passthrough_reset(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    if (!base_op_reset(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    return TRUE;
}

bool secure_basic_passthrough_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    SECURE_BASIC_PASSTHROUGH_OP_DATA *opx_data = get_instance_data(op_data);
    unsigned terminal_id = OPMGR_GET_OP_CONNECT_TERMINAL_ID(message_data);
    unsigned i, channel;

    if (!base_op_build_std_response_ex(op_data, STATUS_OK, response_data))
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
                        opx_data->channel[i]->ip_buffer = OPMGR_GET_OP_CONNECT_BUFFER(message_data);
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
                        opx_data->channel[i]->op_buffer = OPMGR_GET_OP_CONNECT_BUFFER(message_data);
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
                    opx_data->channel[i]->ip_buffer = OPMGR_GET_OP_CONNECT_BUFFER(message_data);
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
                    opx_data->channel[i]->op_buffer = OPMGR_GET_OP_CONNECT_BUFFER(message_data);
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


bool secure_basic_passthrough_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    SECURE_BASIC_PASSTHROUGH_OP_DATA *opx_data = get_instance_data(op_data);
    unsigned terminal_id = OPMGR_GET_OP_DISCONNECT_TERMINAL_ID(message_data);
    unsigned i, channel;

    if (!base_op_build_std_response_ex(op_data, STATUS_OK, response_data))
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
                             * the list anyway in which case doing a swap is
                             * not done as this requires suspending processing
                             * which is time consumming.
                             */
                            if (i == last_active_chan)
                            {
                                opmgr_op_suspend_processing(op_data);
                                opx_data->channel[i] = opx_data->channel[last_active_chan];
                                opx_data->channel[last_active_chan] = deactivate;
                                opmgr_op_resume_processing(op_data);
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
                             * where other inactive channels live. So as long as the list is
                             * reordered before we release the memory then a race condition
                             * with the processing function is not possible. It is therefore
                             * not necessary to suspend processing.
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
                             * the list anyway in which case doing a swap is
                             * not done as this requires suspending processing
                             * which is time consumming.
                             */
                            if (i == last_active_chan)
                            {
                                opmgr_op_suspend_processing(op_data);
                                opx_data->channel[i] = opx_data->channel[last_active_chan];
                                opx_data->channel[last_active_chan] = deactivate;
                                opmgr_op_resume_processing(op_data);
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
                             * where other inactive channels live. So as long as the list is
                             * reordered before we release the memory then a race condition
                             * with the processing function is not possible. It is therefore
                             * not necessary to suspend processing.
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


bool secure_basic_passthrough_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
#if !defined(DISABLE_IN_PLACE) || defined(INSTALL_METADATA)
    unsigned terminal_id = OPMGR_GET_OP_BUF_DETAILS_TERMINAL_ID(message_data);
    SECURE_BASIC_PASSTHROUGH_OP_DATA *opx_data = get_instance_data(op_data);
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

#ifndef DISABLE_IN_PLACE
    buffer_size = ((OP_BUF_DETAILS_RSP*)*response_data)->b.buffer_size;
#endif /* DISABLE_IN_PLACE */

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
    ((OP_BUF_DETAILS_RSP*)*response_data)->b.in_place_buff_params.in_place_terminal = terminal_id ^ TERMINAL_SINK_MASK;

    L4_DBG_MSG1("secure_basic_passthrough_buffer_details  %d \n", ((OP_BUF_DETAILS_RSP*)*response_data)->b.in_place_buff_params.size);
#else
    if (((terminal_id & TERMINAL_SINK_MASK) == 0) && (opx_data->latency_buffer_size != 0))
    {
        /* Output, use the latency buffer size */
        ((OP_BUF_DETAILS_RSP*)*response_data)->b.buffer_size = opx_data->latency_buffer_size;
    }
    L4_DBG_MSG1("secure_basic_passthrough_buffer_details  %d \n", ((OP_BUF_DETAILS_RSP*)*response_data)->b.buffer_size);
#endif /* DISABLE_IN_PLACE */

    return TRUE;
}


bool secure_basic_passthrough_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    OP_SCHED_INFO_RSP* resp;

    resp = base_op_get_sched_info_ex(op_data, message_data, response_id);
    if (resp == NULL)
    {
        return base_op_build_std_response_ex(op_data, STATUS_CMD_FAILED, response_data);
    }
    *response_data = resp;

    resp->block_size = SECURE_BASIC_PASSTHROUGH_DEFAULT_BLOCK_SIZE;

    return TRUE;
}


bool secure_basic_passthrough_get_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    SECURE_BASIC_PASSTHROUGH_OP_DATA *opx_data = get_instance_data(op_data);
    if (!base_op_get_data_format(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

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
#ifdef INSTALL_OPERATOR_TTP_SECPASS

/* Transport metadata from input to output,
 * either adding time-to-play on existing tags or creating new ones if none are present
 * This inevitably looks very similar to metadata_strict_transport
 */
static void metadata_transport_with_ttp(SECURE_BASIC_PASSTHROUGH_OP_DATA *opx_data, unsigned samples)
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
        unsigned list_octets;
        TIME ttp_reference_time;

        /* calculate the total length of tag list */
        unsigned tot_length = mtag->length;
        while (list_tag != NULL)
        {
            tot_length += list_tag->length;
            list_tag = list_tag->next;
        }

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

        /* Update ttp context and get the new time to play.
         *
         * Note: We don't pass 'samples' to this function, instead we pass
         * the total length of mtag (in samples).The reason is that this
         * function is called only when mtag isn't NULL, and by passing 'samples'
         * some samples might get skipped (when mtag is NULL) so there is a
         * risk that the ttp_update module doesn't see all the samples resulting in
         * frequent resetting of TTP generator.
         */
        ttp_update_ttp(opx_data->time_to_play, ttp_reference_time, tot_length / OCTETS_PER_SAMPLE, &status);

        /* Populate the metadata tag from the TTP status */
        METADATA_TIME_OF_ARRIVAL_UNSET(mtag);
        ttp_utils_populate_tag(mtag, &status);
        /* Make sure only one tag gets the stream start flag */
        status.stream_restart = FALSE;

        /* In case there were multiple metadata tags on the input,
         * extrapolate the timestamps to any subsequent tags
         */
        list_tag = mtag->next;
        list_octets = mtag->length;
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

#endif /* INSTALL_OPERATOR_TTP_SECPASS */


RUN_FROM_PM_RAM
void secure_basic_passthrough_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
#if SECURE_BASIC_PASSTHROUGH_DEFAULT_BLOCK_SIZE != 1
#error The process data function kick backwards logic is only valid when block size is 1
#endif
    unsigned i, data_to_process, num_active_chans, output_space;
    /* Alias channels up front as all paths need it. */
    SECURE_BASIC_PASSTHROUGH_OP_DATA *opx_data = get_instance_data(op_data);
    PASSTHROUGH_CHANNEL **channels = opx_data->channel;
    bool buffers_16bit_ex = FALSE;
    unsigned (*input_samples_fn)(tCbuffer *cbuffer) = cbuffer_calc_amount_data_in_words;
    unsigned (*output_space_fn)(tCbuffer *cbuffer) = cbuffer_calc_amount_space_in_words;

    if (opx_data->num_active_chans == 0)
    {
        /* Unlike other capabilities, secure_basic_passthrough can be started even if
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
        fault_diatribe(FAULT_AUDIO_PASSTHROUGH_FORMAT_MISMATCH,
                       base_op_get_ext_op_id(op_data));
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
#ifdef INSTALL_OPERATOR_TTP_SECPASS
    if (op_data->cap_data->id == TTP_PASS_CAP_ID)
    {
        metadata_transport_with_ttp(opx_data, data_to_process);
    }
    else
#endif /* INSTALL_OPERATOR_TTP_SECPASS */

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
        secure_basic_passthrough_processing_ex(opx_data, data_to_process);
    }
    else
#endif /* INSTALL_CBUFFER_EX */
    {
        /* call ASM function */
        secure_basic_passthrough_processing(opx_data, data_to_process);
    }
}


#if defined(INSTALL_CBUFFER_EX)
/* C version of the secure_basic_passthrough_processing loop.
 * We don't need/want to worry about the copy function in this case as
 * the function is only called if we have _ex buffers.
 */
static void secure_basic_passthrough_processing_ex(SECURE_BASIC_PASSTHROUGH_OP_DATA *op_data, unsigned octets)
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

bool secure_basic_passthrough_opmsg_set_ucid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SECURE_BASIC_PASSTHROUGH_OP_DATA *op_extra_data = get_instance_data(op_data);
    OPMSG_SET_UCID_MSG *op_msg = (OPMSG_SET_UCID_MSG *)message_data;
    PS_KEY_TYPE key;

    op_extra_data->ucid = op_msg->ucid;
    key = MAP_CAPID_UCID_SBID_TO_PSKEYID(base_op_get_cap_id(op_data), op_extra_data->ucid, OPMSG_P_STORE_PARAMETER_SUB_ID);
    ps_entry_read((void*)op_data,key,PERSIST_ANY,secure_ups_params_secure_basic_passthrough);

    return TRUE;
}

bool secure_basic_passthrough_opmsg_get_ps_id(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SECURE_BASIC_PASSTHROUGH_OP_DATA *op_extra_data = get_instance_data(op_data);
    OPMSG_GET_LOGICAL_PS_ID_MSG *op_msg = (OPMSG_GET_LOGICAL_PS_ID_MSG *)message_data;
    unsigned *resp;

    resp = (unsigned *)xpmalloc(4 * sizeof(unsigned));
    if (!resp)
    {
        return FALSE;
    }

    resp[0] = OPMGR_GET_OPMSG_MSG_ID(op_msg); /* message ID */
    resp[1] = OPMSG_RESULT_STATES_NORMAL_STATE; /* result field */
    resp[2] = op_extra_data->ucid; /* ucid */
    resp[3] = base_op_get_cap_id(op_data); /* capability ID */
  
    /* set response length */
    *resp_length = 4;
    /* allocate and fill response data */
    *resp_data = (void *)resp;

    return TRUE;
}

bool secure_basic_passthrough_opmsg_set_sample_rate(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SECURE_BASIC_PASSTHROUGH_OP_DATA *p_ext_data = get_instance_data(op_data);

    /* We received client ID, length and then opmsgID and OBPM params */
    p_ext_data->sample_rate = 25 * (OPMSG_FIELD_GET(message_data, OPMSG_COMMON_MSG_SET_SAMPLE_RATE, SAMPLE_RATE));
    p_ext_data->ReInitFlag = 1;

    return(TRUE);
}

bool secure_ups_params_secure_basic_passthrough(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank,
                 uint16 length,  unsigned* data, STATUS_KYMERA status,uint16 extra_status_info)
{
    /* returns true if succesful, false if failed */
    SECURE_BASIC_PASSTHROUGH_OP_DATA *op_extra_data = get_instance_data((OPERATOR_DATA*)instance_data);

    /* Set the Reinit flag after setting the paramters */
    op_extra_data->ReInitFlag = 1;

    return(TRUE);
}
/* **************************** Operator message handlers ******************************** */

static bool set_fadeout_state(OPERATOR_DATA *op_data, FADEOUT_STATE state)
{
    unsigned i;
    SECURE_BASIC_PASSTHROUGH_OP_DATA *opx_data = get_instance_data(op_data);

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

bool secure_basic_passthrough_opmsg_enable_fadeout(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    return set_fadeout_state(op_data, RUNNING_STATE);
}


bool secure_basic_passthrough_opmsg_disable_fadeout(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
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
    SECURE_BASIC_PASSTHROUGH_OP_DATA *opx_data = get_instance_data(op_data);

    /* success/failure outcome will be signalled to caller via return value */
    if (opmgr_op_is_running(op_data))
    {
        /* Let opmsg handler pick up the pieces - it will construct a "command failed" response.
         * No extra info is given from this level.
         */
        return FALSE;
    }

    if (is_input)
    {
        /* 4th field in message is the data format */
        opx_data->ip_format = ((unsigned*)message_data)[3];
    }
    else
    {
        opx_data->op_format = ((unsigned*)message_data)[3];
    }

    /* here we don't validate data type - other ops may well do */
    return TRUE;
}


/*
 * secure_basic_passthrough_change_single_input_data_type
 */
bool secure_basic_passthrough_change_input_data_type(OPERATOR_DATA *op_data, void *message_data,
                                            unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SECURE_BASIC_PASSTHROUGH_OP_DATA *opx_data = get_instance_data(op_data);

    /* The input data type determines whether to perform a straight copy
     * or apply a gain, if this is a basic passthrough operator.
     * Audio = apply a gain.
     */
    opx_data->copy_function = (void *)(&secure_mono_data_loop);

    /* Call with sink terminal 0 (which then basically has ID == TERMINAL_SINK_MASK) */
    return common_change_terminal_data_type(op_data, TRUE, message_data);
}


/*
 * secure_basic_passthrough_change_single_output_data_type
 */
bool secure_basic_passthrough_change_output_data_type(OPERATOR_DATA *op_data, void *message_data,
                                           unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    /* Call with source terminal ID 0 */
    return common_change_terminal_data_type(op_data, FALSE, message_data);
}

/*
 * secure_basic_passthrough_data_stream_based
 */
bool secure_basic_passthrough_data_stream_based(OPERATOR_DATA *op_data, void *message_data,
                                            unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SECURE_BASIC_PASSTHROUGH_OP_DATA *opx_data = get_instance_data(op_data);
    /* The 3rd message word is a boolean value */
    opx_data->simple_data_test_safe = ((unsigned*)message_data)[3];

    return TRUE;
}

bool secure_basic_passthrough_opmsg_set_buffer_size(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SECURE_BASIC_PASSTHROUGH_OP_DATA *opx_data = get_instance_data(op_data);
    opx_data->latency_buffer_size = OPMSG_FIELD_GET(message_data, OPMSG_COMMON_SET_BUFFER_SIZE, BUFFER_SIZE);

    return TRUE;
}

#ifdef INSTALL_OPERATOR_TTP_SECPASS

bool secure_ttp_passthrough_opmsg_set_ttp_latency(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SECURE_BASIC_PASSTHROUGH_OP_DATA *opx_data = get_instance_data(op_data);

    opx_data = (SECURE_BASIC_PASSTHROUGH_OP_DATA *)(op_data->extra_op_data);
    ttp_configure_latency(opx_data->time_to_play, ttp_get_msg_latency(message_data));

    return TRUE;
}

bool secure_ttp_passthrough_opmsg_set_latency_limits(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SECURE_BASIC_PASSTHROUGH_OP_DATA *opx_data = get_instance_data(op_data);
    TIME_INTERVAL min_latency, max_latency;

    opx_data = (SECURE_BASIC_PASSTHROUGH_OP_DATA *)(op_data->extra_op_data);
    ttp_get_msg_latency_limits(message_data, &min_latency, &max_latency);
    ttp_configure_latency_limits(opx_data->time_to_play, min_latency, max_latency);

    return TRUE;
}

bool secure_ttp_passthrough_opmsg_set_ttp_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SECURE_BASIC_PASSTHROUGH_OP_DATA *opx_data = get_instance_data(op_data);
    ttp_params params;

    ttp_get_msg_params(&params, message_data);
    ttp_configure_params(opx_data->time_to_play, &params);

    return TRUE;
}


bool secure_ttp_passthrough_opmsg_set_sample_rate(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SECURE_BASIC_PASSTHROUGH_OP_DATA *opx_data = (SECURE_BASIC_PASSTHROUGH_OP_DATA *)(op_data->extra_op_data);
    unsigned sample_rate = 25 * OPMSG_FIELD_GET(message_data, OPMSG_COMMON_MSG_SET_SAMPLE_RATE, SAMPLE_RATE);

    opx_data->sample_rate = sample_rate;
    ttp_configure_rate(opx_data->time_to_play, sample_rate);

    opx_data->ReInitFlag = 1;

    return TRUE;

}

#endif /* INSTALL_OPERATOR_TTP_SECPASS */

