/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  source_sync.c
 * \ingroup  capabilities
 *
 *  src_sync operator
 *
 */
/*
   The capability ensures that time syncronized data is
   aligned from multiple sources.

   Ideally this would just process a fixed amount from
   each source at a fixed period.   This presents several
   issues.
      1) The capability does not know the clock rate of each source.
         To ensure continuous flow the capabaility has no choise
         but to rely on space a the source leting the external source
         throttle the stream.
      2) By block the data into frames at a fixed period a mismatch
         may occur which reduced available MIPs.


   Instead, the capability uses the output space to set its polling
   period.   The result is non-optimal latency as the intermidiate
   buffers fill.  However, it avoids the above issues.

   A minimum polling period is specified to handle stall transitions
   and to somewaht control the data flow.

 */
/****************************************************************************
Include Files
*/

#include "source_sync_defs.h"
#include "patch/patch.h"

/****************************************************************************
Private Constant Definitions
*/

/* #define SOSY_CHECK_BLOCK_FILL */

/*****************************************************************************
Private Constant Declarations
*/
/* Null terminated handler table for SOURCE_SYNC capability*/
static const handler_lookup_struct src_sync_handler_table =
{
    src_sync_create,            /* OPCMD_CREATE */
    src_sync_destroy,           /* OPCMD_DESTROY */
    src_sync_start,             /* OPCMD_START */
    src_sync_stop,              /* OPCMD_STOP */
    src_sync_reset,             /* OPCMD_RESET */
    src_sync_connect,           /* OPCMD_CONNECT */
    src_sync_disconnect,        /* OPCMD_DISCONNECT */
    src_sync_buffer_details,    /* OPCMD_BUFFER_DETAILS */
    base_op_get_data_format,    /* OPCMD_DATA_FORMAT */
    src_sync_get_sched_info     /* OPCMD_GET_SCHED_INFO */
};



const CAPABILITY_DATA source_sync_cap_data =
{
    CAP_ID_SOURCE_SYNC,                 /* Capability ID */
    SOURCE_SYNC_SRC_SYNC_VERSION_MAJOR, /* Version information */
    SRC_SYNC_VERSION_MINOR,             /* Version information */
    SRC_SYNC_CAP_MAX_CHANNELS,          /* Max number of sinks/inputs */
    SRC_SYNC_CAP_MAX_CHANNELS,          /* Max number of sources/outputs */
    &src_sync_handler_table,            /* Pointer to entry table */
    src_sync_opmsg_handler_table,       /* Pointer to operator message table */
    src_sync_process_data,              /* Pointer to processing function */
    0,                                  /* TODO Processing time information */
    sizeof(SRC_SYNC_OP_DATA)            /* Size of capability-specific per-instance data */
};

#ifdef SOSY_VERBOSE

#ifdef SOSY_NUMBERED_LOG_MESSAGES
unsigned src_sync_trace_serial = 0;
#ifdef SOSY_LOG_MESSAGE_LIMIT
unsigned src_sync_trace_limit = SOSY_LOG_MESSAGE_LIMIT;
#endif
#endif /* SOSY_NUMBERED_LOG_MESSAGES */

/* Define a debug string for each state */
#define SINK_STATE_NAME(S) AUDIO_LOG_STRING(SOSY_SINK_STATE_NAME_##S, #S);
SRC_SYNC_FOR_EACH_SINK_STATE(SINK_STATE_NAME)
#undef SINK_STATE_NAME

static const char* src_sync_sink_state_names[SRC_SYNC_NUM_SINK_STATES+1] =
{
#define SINK_STATE_NAME_TABLE(S) SOSY_SINK_STATE_NAME_##S,
    SRC_SYNC_FOR_EACH_SINK_STATE(SINK_STATE_NAME_TABLE)
#undef SINK_STATE_NAME_TABLE
    NULL
};
#endif /* SOSY_VERBOSE */


/****************************************************************************
Private Function Declarations
*/
/* ******************************* Helper functions ************************************ */

void src_sync_cleanup(SRC_SYNC_OP_DATA *op_extra_data)
{
    unsigned i;

    /* Kill timer Task */
    /* This needs to be atomic, to make sure the right timer gets cancelled */
    LOCK_INTERRUPTS;
    if(op_extra_data->kick_id!=TIMER_ID_INVALID)
    {
        timer_cancel_event(op_extra_data->kick_id);
        op_extra_data->kick_id=TIMER_ID_INVALID;
    }
    UNLOCK_INTERRUPTS;

    src_sync_free_buffer_histories(op_extra_data);

    op_extra_data->sinks_connected = 0;
    op_extra_data->sources_connected = 0;
    op_extra_data->source_group_mask = 0;
    op_extra_data->sink_group_mask = 0;

    pfree(op_extra_data->sink_groups);
    op_extra_data->sink_groups=NULL;

#ifdef INSTALL_METADATA
    SRC_SYNC_SOURCE_GROUP* src_grp;
    for ( src_grp = op_extra_data->source_groups;
          src_grp != NULL;
          src_grp = next_source_group(src_grp))
    {
        pfree(src_grp->metadata_dest.eof_tag);
    }
#endif /* INSTALL_METADATA */
    pfree(op_extra_data->source_groups);
    op_extra_data->source_groups = NULL;

    for(i=0;i<SRC_SYNC_CAP_MAX_CHANNELS;i++)
    {
        pfree(op_extra_data->sinks[i]);
        op_extra_data->sinks[i] = NULL;

        pfree(op_extra_data->sources[i]);
        op_extra_data->sources[i] = NULL;
    }
}

void src_sync_update_processing(OPERATOR_DATA *op_data)
{
    if(op_data->state==OP_RUNNING)
    {
        /* Raise a bg int to process */
        opmgr_kick_operator(op_data);  
    }
}


SRC_SYNC_SINK_ENTRY *src_sync_alloc_sink(SRC_SYNC_OP_DATA   *op_extra_data,unsigned term_idx)
{
    SRC_SYNC_SINK_ENTRY     *sink_data = op_extra_data->sinks[term_idx];

    patch_fn_shared(src_sync);

    if (sink_data == NULL)
    {
        sink_data = xzpnew(SRC_SYNC_SINK_ENTRY);
        if (sink_data != NULL)
        {
            sink_data->common.idx          = term_idx;
            op_extra_data->sinks[term_idx] = sink_data;
        }
    }
    return sink_data;
}

SRC_SYNC_SOURCE_ENTRY *src_sync_alloc_source(SRC_SYNC_OP_DATA *op_extra_data,unsigned term_idx)
{
    SRC_SYNC_SOURCE_ENTRY   *src_data  = op_extra_data->sources[term_idx];

    patch_fn_shared(src_sync);

    if (src_data == NULL)
    {
        src_data = xzpnew(SRC_SYNC_SOURCE_ENTRY);
        if (src_data != NULL)
        {
            src_data->common.idx             = term_idx;
            op_extra_data->sources[term_idx] = src_data;
            op_extra_data->src_route_switch_pending_mask &=
                    ~ (1 << term_idx);
        }
    }

    return src_data;
}

/**
 * A buffer history structure is used for a heuristic to tell when
 * the downstream buffers have been filled. Here this is done by checking
 * that after all kicks during an approximate kick period the output
 * buffers have not been emptied. Thus the number of output buffer levels
 * which need to be recorded is the estimated max number of kicks to this
 * operator during a kick period. Assume that every source/sink group
 * is connected to a different operator and thus the operator can
 * receive an independent series of kicks for every group.
 */
bool src_sync_alloc_buffer_histories(
        SRC_SYNC_OP_DATA *op_extra_data, SRC_SYNC_SINK_GROUP* sink_groups,
        SRC_SYNC_SOURCE_GROUP* source_groups)
{
    unsigned num_entries =
            src_sync_get_num_groups(&sink_groups->common)
            + src_sync_get_num_groups(&source_groups->common);

    if (! src_sync_alloc_buffer_history( &op_extra_data->source_buffer_history,
                                         num_entries))
    {
        return FALSE;
    }

    return TRUE;
}

void src_sync_free_buffer_histories(SRC_SYNC_OP_DATA *op_extra_data)
{
    src_sync_free_buffer_history(&op_extra_data->source_buffer_history);
}


SRC_SYNC_TERMINAL_GROUP* src_sync_find_group(
        SRC_SYNC_TERMINAL_GROUP* groups, unsigned channel_num)
{
    unsigned channel_bit = 1 << channel_num;
    SRC_SYNC_TERMINAL_GROUP* grp;

    for ( grp = groups; grp != NULL; grp = grp->next )
    {
        if ((grp->channel_mask & channel_bit) != 0)
        {
            return grp;
        }
    }
    return NULL;
}

SRC_SYNC_SINK_GROUP* src_sync_find_sink_group(
        SRC_SYNC_OP_DATA *op_extra_data, unsigned channel_num)
{
    return cast_sink_group(
               src_sync_find_group( &(op_extra_data->sink_groups->common),
                   channel_num) );
}

SRC_SYNC_SOURCE_GROUP* src_sync_find_source_group(
        SRC_SYNC_OP_DATA *op_extra_data, unsigned channel_num)
{
    return cast_source_group(
               src_sync_find_group( &(op_extra_data->source_groups->common),
                   channel_num) );
}

#ifdef SOSY_VERBOSE
void src_sync_set_sink_state( SRC_SYNC_OP_DATA *op_extra_data,
                              SRC_SYNC_SINK_GROUP* sink_grp,
                              src_sync_sink_state new_state )
{
    if (sink_grp->stall_state != new_state)
    {
        PL_ASSERT(new_state < SRC_SYNC_NUM_SINK_STATES);
        SOSY_MSG3( SRC_SYNC_TRACE_SINK_STATE,
                   "sink_g%d %s -> %s",
                   sink_grp->common.idx,
                   src_sync_sink_state_names[sink_grp->stall_state],
                   src_sync_sink_state_names[new_state] );
    }

    sink_grp->stall_state = new_state;
}
#endif /* SOSY_VERBOSE */

#ifdef INSTALL_METADATA
bool src_sync_connect_metadata_buffer( SRC_SYNC_TERMINAL_ENTRY* entry,
                                       SRC_SYNC_TERMINAL_GROUP* group)
{
    if (group->metadata_enabled
        && (group->metadata_buffer == NULL)
        && buff_has_metadata(entry->buffer))
    {
        group->metadata_buffer = entry->buffer;
        return TRUE;
    }
    return FALSE;
}

void src_sync_find_alternate_metadata_buffer(
        unsigned connected, SRC_SYNC_TERMINAL_ENTRY* entry,
        SRC_SYNC_TERMINAL_ENTRY** all_entries)
{
    SRC_SYNC_TERMINAL_GROUP* grp;
    unsigned ch;

    grp = entry->group;
    if (grp != NULL
        && grp->metadata_buffer == entry->buffer)
    {
        unsigned channels = grp->channel_mask & connected;
        bool found_alternative = FALSE;

        /* Try to find a replacement in the same group */
        for (ch = 0; ch < SRC_SYNC_CAP_MAX_CHANNELS; ++ ch)
        {
            if ((ch != entry->idx) && ((channels & (1 << ch)) != 0))
            {
                SRC_SYNC_TERMINAL_ENTRY* alt = all_entries[ch];

                /* Bit set in the connected bitmask should guarantee
                 * that the alt != NULL and alt->buffer != NULL
                 */
                if (buff_has_metadata(alt->buffer))
                {
                    grp->metadata_buffer = alt->buffer;
                    found_alternative = TRUE;
                    break;
                }
            }
        }
        if (!found_alternative)
        {
            grp->metadata_buffer = NULL;
        }
    }
}

#endif /* INSTALL_METADATA */

bool src_sync_valid_route(const SRC_SYNC_ROUTE_ENTRY* route)
{
    /* It is not necessary to check route != NULL; the argument
     * is always the address of the field current_route or switch_route
     */
    return (route->sample_rate != 0) && (route->sink != NULL);
}

unsigned src_sync_get_num_groups(const SRC_SYNC_TERMINAL_GROUP* groups)
{
    unsigned count = 0;
    while (groups != NULL)
    {
        count += 1;
        groups = groups->next;
    }
    return count;
}

/* ********************************** API functions ************************************* */


bool src_sync_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    SRC_SYNC_OP_DATA *op_extra_data = (SRC_SYNC_OP_DATA *)(op_data->extra_op_data);

    patch_fn_shared(src_sync);

    SOSY_MSG1( SRC_SYNC_TRACE_ALWAYS, "0x%04x create", INT_TO_EXT_OPID(op_data->id));

    /* call base_op create, which also allocates and fills response message */
    if (!base_op_create(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* Initialize Data */    
    op_extra_data->id = INT_TO_EXT_OPID(op_data->id);
    op_extra_data->Dirty_flag = SRC_SYNC_CAP_CHANNELS_MASK;
    op_extra_data->stat_sink_stalled = 0;
    op_extra_data->stat_sink_stall_occurred = 0;
    op_extra_data->kick_id = TIMER_ID_INVALID;
    op_extra_data->buffer_size = SRC_SYNC_DEFAULT_OUTPUT_BUFFER_SIZE;
    op_extra_data->default_sample_rate = SRC_SYNC_DEFAULT_SAMPLE_RATE;
    op_extra_data->src_route_switch_pending_mask = 0;
#ifdef SOSY_VERBOSE
    op_extra_data->trace_enable = SRC_SYNC_DEFAULT_TRACE_ENABLE;
#ifdef SOSY_NUMBERED_LOG_MESSAGES
    src_sync_trace_serial = 0;
#endif /* SOSY_NUMBERED_LOG_MESSAGES */
#endif /* SOSY_VERBOSE */

    /* Setup CPS */
    if (!cpsInitParameters( &op_extra_data->parms_def,
                            (unsigned*)SOURCE_SYNC_GetDefaults(op_data->cap_data->id),
                            (unsigned*)&op_extra_data->cur_params,
                            sizeof(SOURCE_SYNC_PARAMETERS)))
    {
    	base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    src_sync_trace_params(op_extra_data);

    op_data->state = OP_NOT_RUNNING;

    return TRUE;
}

bool src_sync_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    SRC_SYNC_OP_DATA *op_extra_data = (SRC_SYNC_OP_DATA *)(op_data->extra_op_data);

    patch_fn_shared(src_sync);

    SOSY_MSG1( SRC_SYNC_TRACE_ALWAYS, "0x%04x destroy", op_extra_data->id);

    /* check that we are not trying to destroy a running operator */
    if (op_data->state == OP_RUNNING)
    {
        *response_id = OPCMD_DESTROY;

        /* We can't destroy a running operator. */
        return base_op_build_std_response(STATUS_CMD_FAILED, op_data->id, response_data);
    }
    else
    {
        /* set internal capability state variable to "not_created" */
        src_sync_cleanup(op_extra_data);
        /* call base_op destroy that creates and fills response message, too */
        return base_op_destroy(op_data, message_data, response_id, response_data);
    }
}

bool src_sync_start( OPERATOR_DATA *op_data, void *message_data,
                     unsigned *response_id, void **response_data)
{
    SRC_SYNC_OP_DATA *op_extra_data =
            (SRC_SYNC_OP_DATA *)(op_data->extra_op_data);
    bool start_success;

    patch_fn_shared(src_sync);

    /* do something only if the current state is not "RUNNING" */
    if (op_data->state != OP_RUNNING)
    {
        start_success = base_op_start( op_data, message_data,
                                       response_id,response_data);
        if (start_success)
        {
            /* Re-Init routes */
            src_sync_suspend_processing(op_extra_data);

            /* Startup values */
            op_extra_data->time_stamp = hal_get_time();
            op_extra_data->est_latency = src_sync_get_max_period(op_extra_data);
            op_extra_data->primary_sp_adjust = 0;

            src_sync_resume_processing(op_extra_data);
        }
    }
    else
    {
        start_success = base_op_start( op_data,message_data,
                                       response_id,response_data);
    }
    SOSY_MSG2( SRC_SYNC_TRACE_ALWAYS, "0x%04x start %d", op_extra_data->id, start_success);
    return start_success;
}

bool src_sync_stop_reset(OPERATOR_DATA *op_data,void **response_data)
{
    SRC_SYNC_OP_DATA *op_extra_data = (SRC_SYNC_OP_DATA *)(op_data->extra_op_data);

    LOCK_INTERRUPTS;
    if(op_extra_data->kick_id!=TIMER_ID_INVALID)
    {
        timer_cancel_event(op_extra_data->kick_id);
        op_extra_data->kick_id=TIMER_ID_INVALID;
    }
    op_extra_data->dont_process = TRUE;
    /* operator state variable set to not running state. based on base_op.c */
    op_data->state = OP_NOT_RUNNING;
    UNLOCK_INTERRUPTS;

    if (!base_op_build_std_response(STATUS_OK,op_data->id,response_data))
    {
        return(FALSE);
    }
    return TRUE;
}

bool src_sync_stop(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
#ifdef SOSY_VERBOSE
    SRC_SYNC_OP_DATA *op_extra_data = (SRC_SYNC_OP_DATA *)(op_data->extra_op_data);
#endif /* SOSY_VERBOSE */
    SOSY_MSG1( SRC_SYNC_TRACE_ALWAYS, "0x%04x stop", INT_TO_EXT_OPID(op_data->id));

    patch_fn_shared(src_sync);

    /* Setup Response to Stop Request.   Assume Failure*/
    *response_id = OPCMD_STOP;
    return(src_sync_stop_reset(op_data,response_data));
}

bool src_sync_reset(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
#ifdef SOSY_VERBOSE
    SRC_SYNC_OP_DATA *op_extra_data = (SRC_SYNC_OP_DATA *)(op_data->extra_op_data);
#endif /* SOSY_VERBOSE */
    SOSY_MSG1( SRC_SYNC_TRACE_ALWAYS, "0x%04x reset", INT_TO_EXT_OPID(op_data->id));

    patch_fn_shared(src_sync);

    /* Setup Response to Reset Request.   Assume Failure*/
    *response_id = OPCMD_RESET;
    return(src_sync_stop_reset(op_data,response_data));
}


bool src_sync_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    SRC_SYNC_OP_DATA *op_extra_data = (SRC_SYNC_OP_DATA *)(op_data->extra_op_data);
    unsigned terminal_num;
    unsigned terminal_id = ((unsigned*)message_data)[0];
    unsigned terminal_num_mask;

    patch_fn_shared(src_sync);

    terminal_num = terminal_id & TERMINAL_NUM_MASK;

    if (!base_op_connect(op_data, message_data, response_id, response_data))
    {
        L2_DBG_MSG1("src_sync 0x%04x connect FAILED base_op_connect failed",
                    op_extra_data->id);
        return FALSE;
    }

    if ( terminal_num >= SRC_SYNC_CAP_MAX_CHANNELS )
    {
        /* invalid terminal id */
        L2_DBG_MSG2("src_sync 0x%04x connect REJECTED invalid terminal %d",
                    op_extra_data->id, terminal_num);
    	base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }
    terminal_num_mask = (1 << terminal_num);

    if ((terminal_id & TERMINAL_SINK_MASK) != 0)
    {
        SRC_SYNC_SINK_ENTRY *sink_data;
        SRC_SYNC_SINK_GROUP* sink_grp;

        if ((op_extra_data->sink_group_mask & terminal_num_mask) == 0)
        {
            L2_DBG_MSG2("src_sync 0x%04x connect REJECTED sink %d: no group",
                        op_extra_data->id, terminal_num);
        	base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            return TRUE;
        }

        sink_grp = src_sync_find_sink_group(op_extra_data, terminal_num);
        if (sink_grp == NULL)
        {
            /* Must not happen */
            L2_DBG_MSG2("src_sync 0x%04x connect REJECTED sink %d no group",
                        op_extra_data->id, terminal_num);
        	base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            return TRUE;
        }

        sink_data = op_extra_data->sinks[terminal_num];
        if (sink_data == NULL)
        {
            /* shouldn't happen */
            L2_DBG_MSG2("src_sync 0x%04x connect REJECTED sink %d no entry",
                        op_extra_data->id, terminal_num);
        	base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            return TRUE;
        }

        src_sync_suspend_processing(op_extra_data);

        sink_data->common.buffer = (tCbuffer*)(((uintptr_t *)message_data)[1]);
        sink_data->input_buffer = sink_data->common.buffer;
        sink_data->common.group = &sink_grp->common;

#ifdef INSTALL_METADATA
        if (src_sync_connect_metadata_buffer(&sink_data->common,
                                             &sink_grp->common))
        {
            sink_grp->metadata_input_buffer = sink_grp->common.metadata_buffer;

            SOSY_MSG4( SRC_SYNC_TRACE_ALWAYS, "0x%04x connect sink %d: "
                        "using 0x%08x as metadata buffer on grp %d",
                        op_extra_data->id, terminal_num,
                        (unsigned)(uintptr_t)sink_data->common.buffer,
                        sink_grp->common.idx );
        }
#endif /* INSTALL_METADATA */

        op_extra_data->sinks_connected |= terminal_num_mask;
        if (! sink_grp->common.connected)
        {
            sink_grp->common.connected =
                    (op_extra_data->sinks_connected & sink_grp->common.channel_mask)
                    == sink_grp->common.channel_mask;

            if (sink_grp->common.connected && sink_grp->rate_adjust_enable)
            {
                src_sync_rm_init(op_extra_data, sink_grp);
            }
        }

        SOSY_MSG4( SRC_SYNC_TRACE_ALWAYS, "0x%04x connect sink %d: "
                    "grp %d grp_conn %d",
                    op_extra_data->id, terminal_num,
                    sink_grp->common.idx, sink_grp->common.connected);

        src_sync_resume_processing(op_extra_data);
    }
    else
    {
        SRC_SYNC_SOURCE_ENTRY *src_data;
        SRC_SYNC_SOURCE_GROUP* source_grp;

        if ((op_extra_data->source_group_mask & terminal_num_mask) == 0)
        {
            L2_DBG_MSG2("src_sync 0x%04x connect REJECTED source %d: no group",
                        op_extra_data->id, terminal_num);
        	base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            return TRUE;
        }

        source_grp = src_sync_find_source_group(op_extra_data, terminal_num);
        if (source_grp == NULL)
        {
            /* Must not happen */
            L2_DBG_MSG2("src_sync 0x%04x connect REJECTED source %d no group",
                        op_extra_data->id, terminal_num);
        	base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            return TRUE;
        }

        src_data = op_extra_data->sources[terminal_num];
        if (src_data == NULL)
        {
            /* Must not happen */
            L2_DBG_MSG2("src_sync 0x%04x connect REJECTED source %d no entry",
                        op_extra_data->id, terminal_num);
        	base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            return TRUE;
        }

        src_sync_suspend_processing(op_extra_data);

        src_data->common.buffer = (tCbuffer*)(((uintptr_t *)message_data)[1]);
        src_data->common.group = &source_grp->common;

        op_extra_data->sources_connected |= terminal_num_mask;
        source_grp->common.connected =
                (op_extra_data->sources_connected & source_grp->common.channel_mask)
                == source_grp->common.channel_mask;

        SOSY_MSG4( SRC_SYNC_TRACE_ALWAYS, "0x%04x connect source %d: "
                   "grp %d grp_conn %d",
                   op_extra_data->id, terminal_num,
                   source_grp->common.idx,
                   source_grp->common.connected);

#ifdef INSTALL_METADATA
        if (src_sync_connect_metadata_buffer(&src_data->common,
                                             &source_grp->common))
        {
            SOSY_MSG4( SRC_SYNC_TRACE_ALWAYS, "0x%04x connect source %d: "
                       "using 0x%08x as metadata buffer on grp %d",
                       op_extra_data->id, terminal_num,
                       (unsigned)(uintptr_t)src_data->common.buffer,
                       source_grp->common.idx );
            buff_metadata_set_usable_octets(source_grp->common.metadata_buffer,
                get_octets_per_word(AUDIO_DATA_FORMAT_FIXP));
        }
#endif /* INSTALL_METADATA */

        src_sync_resume_processing(op_extra_data);
    }

    src_sync_update_processing(op_data);

    return TRUE;
}


bool src_sync_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    SRC_SYNC_OP_DATA *op_extra_data = (SRC_SYNC_OP_DATA *)(op_data->extra_op_data);
    unsigned terminal_num;
    unsigned terminal_id = ((unsigned*)message_data)[0];

    patch_fn_shared(src_sync);

    terminal_num = terminal_id & TERMINAL_NUM_MASK;

    if (!base_op_disconnect(op_data, message_data, response_id, response_data))
    {
        L2_DBG_MSG1("src_sync 0x%04x disconnect FAILED base_op_disconnect failed",
                    op_extra_data->id);
        return FALSE;
    }
    if ( terminal_num >= SRC_SYNC_CAP_MAX_CHANNELS )
    {
        L2_DBG_MSG2("src_sync 0x%04x disconnect REJECTED invalid terminal %d",
                    op_extra_data->id, terminal_num);
        /* invalid terminal id */
    	base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    if ((terminal_id & TERMINAL_SINK_MASK) != 0)
    {
        SRC_SYNC_SINK_ENTRY *sink_data = op_extra_data->sinks[terminal_num];
        if ((sink_data != NULL) && (sink_data->common.buffer != NULL))
        {
            SRC_SYNC_SINK_GROUP* sink_grp;

            src_sync_suspend_processing(op_extra_data);

            op_extra_data->sinks_connected &= ~(1<<terminal_num);

#ifdef INSTALL_METADATA
            /* In casting the third argument, each array element is cast
             * from SRC_SYNC_SINK_ENTRY* to SRC_SYNC_TERMINAL_ENTRY*,
             * to point to its first member. This is valid even
             * if the entry is NULL.
             */
            src_sync_find_alternate_metadata_buffer(
                    op_extra_data->sinks_connected, &sink_data->common,
                    (SRC_SYNC_TERMINAL_ENTRY**)(op_extra_data->sinks));
#endif /* INSTALL_METADATA */

            sink_data->common.buffer = NULL;
            sink_data->input_buffer = NULL;

            /* set_route, then disconnect, could lead
             * to a NULL group pointer */
            sink_grp = sink_group_from_entry(sink_data);
            if (sink_grp != NULL)
            {
#ifdef INSTALL_METADATA
                sink_grp->metadata_input_buffer = sink_grp->common.metadata_buffer;
#endif /* INSTALL_METADATA */

                if (sink_grp->common.connected && sink_grp->rate_adjust_enable)
                {
                    src_sync_rm_fini(op_extra_data, sink_grp);
                }
                sink_grp->common.connected = FALSE;
            }
            src_sync_check_primary_clock_connected(op_extra_data);

            src_sync_resume_processing(op_extra_data);

            SOSY_MSG2( SRC_SYNC_TRACE_ALWAYS, "0x%04x disconnect sink %d",
                       op_extra_data->id, terminal_num);
        }
        else
        {
            L2_DBG_MSG2("src_sync 0x%04x disconnect REJECTED sink %d not connected",
                        op_extra_data->id, terminal_num);
            /* not connected */
        	base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        }
    }
    else
    {
        SRC_SYNC_SOURCE_ENTRY *src_data = op_extra_data->sources[terminal_num];
        if ((src_data != NULL) && (src_data->common.buffer != NULL))
        {
            src_sync_suspend_processing(op_extra_data);

            op_extra_data->sources_connected &= ~(1<<terminal_num);

#ifdef INSTALL_METADATA
            /* In casting the third argument, each array element is cast
             * from SRC_SYNC_SOURCE_ENTRY* to SRC_SYNC_TERMINAL_ENTRY*,
             * to point to its first member. This is valid even
             * if the entry is NULL.
             */
            src_sync_find_alternate_metadata_buffer(
                    op_extra_data->sinks_connected, &src_data->common,
                    (SRC_SYNC_TERMINAL_ENTRY**)(op_extra_data->sources));
#endif /* INSTALL_METADATA */

            src_data->common.buffer = NULL;

            /* set_route, then disconnect, could lead
             * to a NULL group pointer */
            if (src_data->common.group != NULL)
            {
                src_data->common.group->connected = FALSE;
            }
            src_sync_check_primary_clock_connected(op_extra_data);

            src_sync_resume_processing(op_extra_data);

            SOSY_MSG2( SRC_SYNC_TRACE_ALWAYS, "0x%04x disconnect source %d",
                       op_extra_data->id, terminal_num);
        }
        else
        {
            L2_DBG_MSG2("src_sync 0x%04x disconnect REJECTED source %d not connected",
                        op_extra_data->id, terminal_num);
            /* not connected */
        	base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        }
    }

    src_sync_update_processing(op_data);

    return TRUE;
}


bool src_sync_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    unsigned terminal_id;
    unsigned terminal_num;
    SRC_SYNC_OP_DATA *op_extra_data = (SRC_SYNC_OP_DATA *)(op_data->extra_op_data);
#ifdef INSTALL_METADATA
    SRC_SYNC_TERMINAL_ENTRY* terminal;
#endif /* INSTALL_METADATA */

    patch_fn_shared(src_sync);

    if (!base_op_buffer_details(op_data, message_data, response_id, response_data))
    {
        L2_DBG_MSG1("src_sync 0x%04x buffer_details FAILED base_op_buffer_details failed",
                    op_extra_data->id);
        return FALSE;
    }

    terminal_id = ((unsigned *)message_data)[0];
    terminal_num = terminal_id & TERMINAL_NUM_MASK;

    if (terminal_num >= SRC_SYNC_CAP_MAX_CHANNELS)
    {
    	base_op_change_response_status(response_data, STATUS_INVALID_CMD_PARAMS);

        L2_DBG_MSG2("src_sync 0x%04x buffer_details REJECTED invalid terminal 0x%06x",
                    op_extra_data->id, terminal_id);
        return TRUE;
    }

    ((OP_BUF_DETAILS_RSP*)*response_data)->runs_in_place = FALSE;

#ifdef INSTALL_METADATA
    /* If an input/output connection is already present and has metadata then
     * we are obliged to return that buffer so that metadata can be shared
     * between channels. */

    ((OP_BUF_DETAILS_RSP*)*response_data)->supports_metadata = FALSE;

    if ((terminal_id & TERMINAL_SINK_MASK) == TERMINAL_SINK_MASK)
    {
        /* Taking the address of the first struct member is a type-safer
         * pointer cast, even if the pointer is NULL */
        terminal = &(op_extra_data->sinks[terminal_num]->common);
    }
    else
    {
        /* Taking the address of the first struct member is a type-safer
         * pointer cast, even if the pointer is NULL */
        terminal = &(op_extra_data->sources[terminal_num]->common);
    }

    if ((terminal != NULL)
        && (terminal->group != NULL))
    {
        ((OP_BUF_DETAILS_RSP*)*response_data)->supports_metadata =
                terminal->group->metadata_enabled;
        ((OP_BUF_DETAILS_RSP*)*response_data)->metadata_buffer =
                terminal->group->metadata_buffer;
    }
#endif /* INSTALL_METADATA */

    unsigned config_buffer_w;
    if (op_extra_data->buffer_size > 0)
    {
        config_buffer_w = op_extra_data->buffer_size;
    }
    else
    {
        unsigned config_buffer_us, buffer_kick_mult;
        if (op_extra_data->buffer_size == 0)
        {
            buffer_kick_mult = SRC_SYNC_AUTO_BUFFER_SS_PERIOD_MULT;
        }
        else
        {
            buffer_kick_mult = - op_extra_data->buffer_size;
        }
        config_buffer_us = frac_mult(buffer_kick_mult, (unsigned)stream_if_get_system_kick_period() << 5);
        config_buffer_w = frac_mult(src_sync_usec_to_sec_frac(config_buffer_us), op_extra_data->default_sample_rate);
    }

    ((OP_BUF_DETAILS_RSP*)*response_data)->b.buffer_size = config_buffer_w;

#ifdef INSTALL_METADATA
    SOSY_MSG4( SRC_SYNC_TRACE_ALWAYS, "0x%04x buffer_details 0x%06x bsz_w %d "
                "supp_md %d",
                op_extra_data->id, terminal_id,
                ((OP_BUF_DETAILS_RSP*)*response_data)->b.buffer_size,
                ((OP_BUF_DETAILS_RSP*)*response_data)->supports_metadata);
#else /* INSTALL_METADATA */
    SOSY_MSG3( SRC_SYNC_TRACE_ALWAYS, "0x%04x buffer_details 0x%06x bsz_w %d",
                op_extra_data->id, terminal_id,
                ((OP_BUF_DETAILS_RSP*)*response_data)->b.buffer_size);
#endif /* INSTALL_METADATA */

    return TRUE;
}


bool src_sync_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
#ifdef SOSY_VERBOSE
    SRC_SYNC_OP_DATA *op_extra_data = (SRC_SYNC_OP_DATA *)(op_data->extra_op_data);
#endif /* SOSY_VERBOSE */
    OP_SCHED_INFO_RSP* resp;

    patch_fn_shared(src_sync);

    if (!base_op_get_sched_info(op_data, message_data, response_id, response_data))
    {
        L2_DBG_MSG1("src_sync 0x%04x get_sched_info FAILED base_op_get_sched_info failed",
                    INT_TO_EXT_OPID(op_data->id));
        return FALSE;
    }
    /* Populate the response*/
    base_op_change_response_status(response_data, STATUS_OK);
    resp = *response_data;
    resp->op_id = op_data->id;
    /* Same buffer size for sink and source.
       No additional verification needed.*/
    resp->block_size = SRC_SYNC_DEFAULT_BLOCK_SIZE;
    resp->run_period = 0;

    SOSY_MSG1( SRC_SYNC_TRACE_ALWAYS, "0x%04x get_sched_info",
               INT_TO_EXT_OPID(op_data->id));

    *response_data = resp;
    return TRUE;
}

/**
 * \brief Check whether parameters allow a sink to enter RECOVERING_RESTARTING
 * state. That state is intended to prime downstream buffers with a predictable
 * amount of latency (MAX_LATENCY). If all the target latency can be contained
 * in the directly connected output buffers, go to FLOWING state instead.
 * The output buffer size condition is simplified to use the common output
 * buffer space, saved in max_space_t, rather than only the output buffers
 * affecting a sink group.
 */
void src_sync_set_sink_recovering_restarting(SRC_SYNC_OP_DATA *op_extra_data,
                                             SRC_SYNC_SINK_GROUP *sink_grp)
{
    if (op_extra_data->max_space_t >= src_sync_get_max_latency(op_extra_data))
    {
        src_sync_set_sink_state(op_extra_data, sink_grp,
                                SRC_SYNC_SINK_FLOWING);
    }
    else
    {
        sink_grp->inserted_silence_words = 0;
        src_sync_set_sink_state(op_extra_data, sink_grp,
                                SRC_SYNC_SINK_RECOVERING_RESTARTING );
    }
}

void src_sync_refresh_sink_list(SRC_SYNC_OP_DATA *op_extra_data)
{
    unsigned i;
    SRC_SYNC_SINK_ENTRY  **lpp_sinks;
    SRC_SYNC_SINK_GROUP  *group;
#ifdef INSTALL_METADATA
    bool have_rm_master = FALSE;
#endif /* INSTALL_METADATA */

    for (group = op_extra_data->sink_groups;
         group != NULL;
         group = next_sink_group(group))
    {
        SRC_SYNC_TERMINAL_ENTRY** ppentry = &(group->common.terminals);
        unsigned channels = group->common.channel_mask;

        for (i = 0, lpp_sinks = op_extra_data->sinks;
             i < SRC_SYNC_CAP_MAX_CHANNELS;
             i++, lpp_sinks++)
        {
            SRC_SYNC_SINK_ENTRY  *sink = *lpp_sinks;
            unsigned ch_bit = 1 << i;

            if ( ((channels & ch_bit) != 0)
                 && (sink != NULL) )
            {
                /* Add sink to list */
                *ppentry = &sink->common;
                ppentry = &(*ppentry)->next;

                SOSY_MSG2( SRC_SYNC_TRACE_REFRESH,
                           "rsil add sink %d to grp %d list",
                           sink->common.idx, group->common.idx );

                sink->input_buffer = src_sync_get_input_buffer(group, sink);
            }
        }
        *ppentry = NULL;

        if (! group->common.connected)
        {
            src_sync_set_sink_state( op_extra_data, group,
                                     SRC_SYNC_SINK_NOT_CONNECTED);
        }

#ifdef INSTALL_METADATA
        /*
         * Timestamped rate references:
         * Build up list of measurement contexts
         * Determine which sink group becomes timestamped rate master
         * i.e. provides tags from which to derive the measurement
         */
        else if (! have_rm_master && group->common.connected
                && group->common.metadata_enabled
                && (group->common.metadata_buffer != NULL))
        {
            group->ts_rate_master = TRUE;
            have_rm_master = TRUE;

            src_sync_ra_set_primary_rate(op_extra_data, group->common.sample_rate);

            SOSY_MSG1(SRC_SYNC_TRACE_RATE_MATCH, "rm sink_g%d becomes ts_rate_master",
                      group->common.idx);
        }
#endif /* INSTALL_METADATA */
    }
}

void src_sync_refresh_source_list(SRC_SYNC_OP_DATA *op_extra_data)
{
    unsigned src_kick;
    unsigned min_size_t;
    SRC_SYNC_SOURCE_ENTRY  **lpp_sources;
    SRC_SYNC_SOURCE_GROUP* src_grp;

    /* collect minimum of connected source buffer sizes, converted to time */
    min_size_t = MAXINT;
    /* collect bitmap of sources to kick */
    src_kick = 0;

    /*
     * For each source group, link the connected and routed sources
     * for quick traversal. These are the sources for which output
     * needs to be produced.
     *
     * It is not necessary for the routed sinks to be connected;
     * silence will be inserted for unconnected routed sinks.
     */
    for ( src_grp = op_extra_data->source_groups;
          src_grp != NULL;
          src_grp = next_source_group(src_grp) )
    {
        SRC_SYNC_TERMINAL_ENTRY** ppentry = &(src_grp->common.terminals);
        unsigned i;
        unsigned channels = src_grp->common.channel_mask;

        for ( i = 0, lpp_sources = op_extra_data->sources;
              i < SRC_SYNC_CAP_MAX_CHANNELS;
              i++, lpp_sources++, channels >>= 1 )
        {
            SRC_SYNC_SOURCE_ENTRY  *src_ptr = *lpp_sources;
            if ( ((channels & 1) != 0)
                 && (src_ptr != NULL)
                 && (src_ptr->common.buffer != NULL) )
            {
                SRC_SYNC_ROUTE_ENTRY* route = &src_ptr->current_route;

                if (src_sync_valid_route(route))
                {
                    unsigned buffer_size_words, buffer_size_time;

                    /* link into the source group's terminal list */
                    *ppentry = &src_ptr->common;
                    ppentry = &(*ppentry)->next;

                    /* Setup Source */
                    src_kick |= (1<<src_ptr->common.idx);

                    /* Get minimum buffer size (minus two) in time */
                    buffer_size_words =
                            cbuffer_get_size_in_words(src_ptr->common.buffer);
                    buffer_size_words -= 2;
                    buffer_size_time = src_sync_samples_to_time(
                        buffer_size_words, route->inv_sample_rate );

                    min_size_t = pl_min(min_size_t, buffer_size_time);

                    SOSY_MSG4( SRC_SYNC_TRACE_REFRESH,
                               "rsol grp %d src %d bsw %d bst 0.%05d",
                               src_grp->common.idx, i, buffer_size_words,
                               src_sync_sec_frac_to_10usec(buffer_size_time));
                }
            }
        }
        *ppentry = NULL;
    }

    SOSY_MSG2( SRC_SYNC_TRACE_REFRESH,
               "rsol max_space 0.%05d fwd_kick 0x%06x",
               src_sync_sec_frac_to_10usec(min_size_t), src_kick);

    /* Save State Info */
    op_extra_data->max_space_t = min_size_t;
    op_extra_data->forward_kicks = src_kick;
}


/* ************************************* Data processing-related functions and wrappers **********************************/
void src_sync_timer_task(void *kick_object)
{
    OPERATOR_DATA      *op_data = (OPERATOR_DATA*)kick_object;
    SRC_SYNC_OP_DATA   *op_extra_data = (SRC_SYNC_OP_DATA*) op_data->extra_op_data;

    patch_fn_shared(src_sync);

    op_extra_data->kick_id = TIMER_ID_INVALID;

    /* Raise a bg int to process */
    opmgr_kick_operator(op_data);
}

bool src_sync_perform_transitions(SRC_SYNC_OP_DATA* op_extra_data)
{
    SRC_SYNC_SOURCE_GROUP* src_grp;
    unsigned i;
    bool routes_changed = FALSE;

    for ( src_grp = op_extra_data->source_groups;
          src_grp != NULL;
          src_grp = next_source_group(src_grp))
    {
        /* is transition_pt == 0 for all sources with a switch_route? */
        unsigned grp_switch_route_mask =
                ( src_grp->common.channel_mask
                  & op_extra_data->src_route_switch_pending_mask);

        if (0 == grp_switch_route_mask)
        {
            /* No sources in this group have a pending transition */
            continue;
        }

        bool unfinished_transition_out = FALSE;
        for (i = 0; i < SRC_SYNC_CAP_MAX_CHANNELS; ++ i)
        {
            if ((grp_switch_route_mask & (1<<i)) != 0)
            {
                SRC_SYNC_SOURCE_ENTRY* src_ptr = op_extra_data->sources[i];

                PL_ASSERT((src_ptr != NULL) && (src_ptr->switch_route.sink != NULL));

                if (src_ptr->transition_pt != 0)
                {
                    unfinished_transition_out = TRUE;
                    break;
                }
            }
        }

        if (! unfinished_transition_out)
        {
            SOSY_MSG2( SRC_SYNC_TRACE_ALWAYS, "src_g_%d switch route mask 0x%06x",
                src_grp->common.idx, grp_switch_route_mask);

            /* Change over now */
            for (i = 0; i < SRC_SYNC_CAP_MAX_CHANNELS; ++ i)
            {
                if ((grp_switch_route_mask & (1<<i)) != 0)
                {
                    SRC_SYNC_SOURCE_ENTRY* src_ptr = op_extra_data->sources[i];
                    PL_ASSERT((src_ptr != NULL) && (src_ptr->switch_route.sink != NULL));

                    unsigned sample_rate, inv_sample_rate;

                    /* Change direction of transition */
                    src_ptr->inv_transition = - src_ptr->inv_transition;

                    /* Switch Sinks */
                    src_ptr->current_route = src_ptr->switch_route;
                    src_ptr->switch_route.sink = NULL;
                    op_extra_data->src_route_switch_pending_mask &= ~(1<<i);

                    /* Make sure the sample rate is correct */
                    sample_rate = src_ptr->current_route.sample_rate;
                    inv_sample_rate = src_ptr->current_route.inv_sample_rate;

                    src_ptr->common.group->sample_rate = sample_rate;
                    src_ptr->common.group->inv_sample_rate = inv_sample_rate;

                    src_ptr->current_route.sink->common.group->sample_rate =
                            sample_rate;
                    src_ptr->current_route.sink->common.group->inv_sample_rate =
                            inv_sample_rate;

#ifdef INSTALL_METADATA
                    /* Update submodules which need to have the current sample rate */
                    SRC_SYNC_SINK_GROUP* sink_grp =
                            sink_group_from_entry(src_ptr->current_route.sink);
                    src_sync_ra_set_rate(op_extra_data, sink_grp, sample_rate);
                    if (sink_grp->ts_rate_master)
                    {
                        src_sync_ra_set_primary_rate(op_extra_data, sample_rate);
                    }
#endif /* INSTALL_METADATA */

                    routes_changed = TRUE;

                    op_extra_data->Dirty_flag |= (1<<i);

                    SOSY_MSG4( SRC_SYNC_TRACE_TRANSITION,
                               "route sink %d -> src %d fs %dHz g %d/60db",
                               src_ptr->current_route.sink->common.idx,
                               src_ptr->common.idx,
                               src_ptr->current_route.sample_rate,
                               src_ptr->current_route.gain_dB);
                }
            }
        }
    }
    return routes_changed;
}

void src_sync_refresh_forward_routes(SRC_SYNC_OP_DATA* op_extra_data)
{
    SRC_SYNC_SINK_GROUP* sink_grp;
    unsigned i;

    /* Clear, then recreate forward route linked lists
     * (sink_ptr->source and src_ptr->next_split_route,
     * sink_grp->route_dest)
     */
    for (i = 0; i < SRC_SYNC_CAP_MAX_CHANNELS; ++ i)
    {
        SRC_SYNC_SINK_ENTRY* sink_ptr = op_extra_data->sinks[i];
        if (sink_ptr != NULL)
        {
            sink_ptr->source = NULL;
        }
    }
    for (i = 0; i < SRC_SYNC_CAP_MAX_CHANNELS; ++ i)
    {
        SRC_SYNC_SOURCE_ENTRY* src_ptr = op_extra_data->sources[i];
        if (src_ptr != NULL)
        {
            src_ptr->next_split_source = NULL;
        }
    }
    for ( sink_grp = op_extra_data->sink_groups;
          sink_grp != NULL;
          sink_grp = next_sink_group(sink_grp))
    {
        sink_grp->route_dest = NULL;
    }
    for (i = 0; i < SRC_SYNC_CAP_MAX_CHANNELS; ++ i)
    {
        SRC_SYNC_SOURCE_ENTRY* src_ptr = op_extra_data->sources[i];
        if ( (src_ptr != NULL)
             && src_sync_valid_route(&src_ptr->current_route)
             && (src_ptr->common.buffer != NULL) )
        {
            SRC_SYNC_SINK_ENTRY* route_origin =
                    src_ptr->current_route.sink;
            src_ptr->next_split_source = route_origin->source;
            route_origin->source = src_ptr;

            SRC_SYNC_SINK_GROUP* route_origin_grp =
                    sink_group_from_entry(route_origin);
            SRC_SYNC_SOURCE_GROUP* dest_grp =
                    source_group_from_entry(src_ptr);

            PL_ASSERT((route_origin_grp->route_dest == NULL) || (route_origin_grp->route_dest == dest_grp));
            route_origin_grp->route_dest = dest_grp;

#ifdef SOSY_VERBOSE
            if (src_ptr->next_split_source != NULL)
            {
                SOSY_MSG3( SRC_SYNC_TRACE_TRANSITION,
                           "route changed: sink_%d -> src_%d (split src_%d)",
                           route_origin->common.idx,
                           src_ptr->common.idx,
                           src_ptr->next_split_source->common.idx);
            }
            else
            {
                SOSY_MSG2( SRC_SYNC_TRACE_TRANSITION,
                           "route changed: sink_%d -> src_%d",
                           route_origin->common.idx, src_ptr->common.idx);
            }
#endif /* SOSY_VERBOSE */
        }
    }
}

/**
 * Break out the outermost decision of src_sync_perform_transitions
 * as a performance optimization. Still, the caller should not need
 * to know about the src_route_switch_pending_mask field, so
 * keep this in an inline function.
 */
static inline bool src_sync_perform_transitions_if_needed(SRC_SYNC_OP_DATA* op_extra_data)
{
    if (op_extra_data->src_route_switch_pending_mask != 0)
    {
        return src_sync_perform_transitions(op_extra_data);
    }
    else
    {
        return FALSE;
    }
}

void src_sync_refresh_connections(SRC_SYNC_OP_DATA* op_extra_data)
{
    SRC_SYNC_SINK_GROUP* sink_grp;
    bool all_routed_sinks_are_unconnected = TRUE;

    for ( sink_grp = op_extra_data->sink_groups;
          sink_grp != NULL;
          sink_grp = next_sink_group(sink_grp))
    {
        if ((sink_grp->route_dest != NULL) && sink_grp->common.connected)
        {
            all_routed_sinks_are_unconnected = FALSE;
        }
    }

#ifdef SOSY_VERBOSE
    if (op_extra_data->all_routed_sinks_unconnected != all_routed_sinks_are_unconnected)
    {
        SOSY_MSG2( SRC_SYNC_TRACE_SINK_STATE | SRC_SYNC_TRACE_TRANSITION,
                   "route/connection changed: all_routed_sinks_unconnected %d -> %d",
                   op_extra_data->all_routed_sinks_unconnected,
                   all_routed_sinks_are_unconnected);
    }
#endif /* SOSY_VERBOSE */
    op_extra_data->all_routed_sinks_unconnected = all_routed_sinks_are_unconnected;
}

#ifdef INSTALL_METADATA
void src_sync_refresh_metadata_routes(SRC_SYNC_OP_DATA* op_extra_data)
{
    SRC_SYNC_SINK_GROUP* sink_grp;
    SRC_SYNC_SOURCE_GROUP* src_grp;

    /* Recalculate metadata routes */

    /* 1. Clear metadata routes */
    for ( sink_grp = op_extra_data->sink_groups;
          sink_grp != NULL;
          sink_grp = next_sink_group(sink_grp) )
    {
        sink_grp->metadata_dest = NULL;
    }
    for ( src_grp = op_extra_data->source_groups;
          src_grp != NULL;
          src_grp = next_source_group(src_grp) )
    {
        src_grp->metadata_in = NULL;
    }

    /* 2. From metadata routes along audio routes,
     *    chose the lowest numbered input group
     */
    for ( src_grp = op_extra_data->source_groups;
          src_grp != NULL;
          src_grp = next_source_group(src_grp) )
    {
        if ( src_grp->common.metadata_enabled &&
             (src_grp->common.metadata_buffer != NULL) )
        {
            SRC_SYNC_SOURCE_ENTRY* src_ptr;

            for ( src_ptr = source_entries_from_group(src_grp);
                  src_ptr != NULL;
                  src_ptr = next_source_entry(src_ptr) )
            {
                if (src_sync_valid_route( &(src_ptr->current_route) ))
                {
                    sink_grp = cast_sink_group(src_ptr->current_route.sink->common.group);

                    /* pointer arithmetic can be used because
                     * all sink group structs are in the array
                     * op_extra_data->sink_groups
                     */
                    if ( (src_grp->metadata_in == NULL)
                         || (sink_grp < src_grp->metadata_in) )
                    {
                        src_grp->metadata_in = sink_grp;
                    }
                }
            }
        }
    }

    /* 3. Build forward linked lists from backward pointers
     */
    for ( src_grp = op_extra_data->source_groups;
          src_grp != NULL;
          src_grp = next_source_group(src_grp) )
    {
        if (src_grp->metadata_in != NULL)
        {
            src_grp->metadata_in->metadata_dest = src_grp;

            SOSY_MSG2( SRC_SYNC_TRACE_TRANSITION,
                       "metadata route sink_g%d -> src_g%d",
                       src_grp->metadata_in->common.idx,
                       src_grp->common.idx );
        }
    }
}
#endif /* INSTALL_METADATA */

unsigned src_sync_calc_sink_group_available_data(SRC_SYNC_SINK_GROUP* sink_grp)
{
    SRC_SYNC_SINK_ENTRY* sink_ptr;
    unsigned sink_grp_data_w = MAXINT;

    if (! sink_grp->common.connected)
    {
        return 0;
    }
    if ((sink_grp->route_dest == NULL) && !sink_grp->purge)
    {
        return sink_grp_data_w;
    }

    for ( sink_ptr = sink_entries_from_group(sink_grp);
          sink_ptr != NULL;
          sink_ptr = next_sink_entry(sink_ptr) )
    {
        unsigned data_w;

        data_w = cbuffer_calc_amount_data_in_words(sink_ptr->input_buffer);

        sink_grp_data_w = pl_min(sink_grp_data_w, data_w);
        if (sink_grp_data_w == 0)
        {
            break;
        }
    }

    return sink_grp_data_w;
}

/**
 * \note Variables whose names end with _t are in fractional seconds,
 *       those with names ending with _w are in words.
 */
void src_sync_compute_space(
        SRC_SYNC_OP_DATA* op_extra_data,
        unsigned* p_min_src_space_t, unsigned* p_max_src_space_t)
{
    SRC_SYNC_SOURCE_ENTRY* src_ptr;
    SRC_SYNC_SOURCE_GROUP* src_grp;
    unsigned min_src_space_t;
    unsigned max_src_space_t;

    min_src_space_t = MAXINT;
    max_src_space_t = 0;

    for ( src_grp = op_extra_data->source_groups;
          src_grp != NULL;
          src_grp = next_source_group(src_grp))
    {
        unsigned src_grp_space_w, src_grp_space_t;

        if (src_grp->common.terminals != NULL)
        {
            src_grp_space_w = MAXINT;

            for ( src_ptr = source_entries_from_group(src_grp);
                  src_ptr != NULL;
                  src_ptr = next_source_entry(src_ptr))
            {
                unsigned space_w;

                /* Get amount of space */
                space_w = cbuffer_calc_amount_space_in_words(
                              src_ptr->common.buffer);

                SOSY_MSG2( SRC_SYNC_TRACE_SRC_TERM_SPACE,
                           "calc source %d space_w %d",
                           src_ptr->common.idx, space_w);

                src_grp_space_w = pl_min(src_grp_space_w, space_w);
                if (src_grp_space_w == 0)
                {
                    break;
                }
            }

#ifdef INSTALL_METADATA
            /* TODO cache the value of this condition */
            if ( src_grp->common.metadata_enabled
                 && (src_grp->common.metadata_buffer != NULL)
                 && (buff_has_metadata(src_grp->common.metadata_buffer)))
            {
                unsigned md_space_o =
                        buff_metadata_available_space(
                                src_grp->common.metadata_buffer);
                unsigned md_space_w = md_space_o / OCTETS_PER_SAMPLE;

                if (md_space_w < src_grp_space_w)
                {
                    SOSY_MSG4( SRC_SYNC_TRACE_SRC_SPACE,
                               "calc src_g%d space limited "
                               "by metadata buffer: "
                               "cb_sp_w %d > md_sp_o %d md_sp_w %d",
                               src_grp->common.idx, src_grp_space_w,
                               md_space_o, md_space_w);
                    src_grp_space_w = md_space_w;
                }
            }
#endif /* INSTALL_METADATA */

            src_grp->common.transfer = src_grp_space_w;

            src_grp_space_t = src_sync_samples_to_time(
                                  src_grp_space_w,
                                  src_grp->common.inv_sample_rate);

            SOSY_MSG3( SRC_SYNC_TRACE_SRC_SPACE,
                       "calc src_g%d sp_w %d sp_t 0.%05d",
                       src_grp->common.idx, src_grp_space_w,
                       src_sync_sec_frac_to_10usec(src_grp_space_t) );

            min_src_space_t = pl_min(min_src_space_t, src_grp_space_t);
            max_src_space_t = pl_max(max_src_space_t, src_grp_space_t);
        }
    }
    *p_min_src_space_t = min_src_space_t;
    *p_max_src_space_t = max_src_space_t;
}

/**
 * \note Variables whose names end with _t are in fractional seconds,
 *       those with names ending with _w are in words.
 * \return a negative value if computing and processing transfers should continue,
 *         a positive time value in order to reschedule,
 *         0 if there is not enough output space to transfer this time.
 */
int src_sync_compute_transfer_space(SRC_SYNC_OP_DATA* op_extra_data,
                                    SRC_SYNC_COMP_CONTEXT* comp)
{
    TIME now;
    unsigned us_since;

    patch_fn_shared(src_sync);

    comp->max_transfer_t = 0;
    comp->min_transfer_t = 0;
    comp->downstream_filled = FALSE;

    now = hal_get_time();
    /* The subtraction will wrap when the hal_get_time() value wraps,
     * every ca. 71 minutes. The result is always expected to be a
     * small positive number (up to low thousands) due to scheduling.
     */
    us_since = (unsigned)(now - op_extra_data->time_stamp);
    op_extra_data->time_stamp = now;

    comp->est_latency_t =
            pl_max( (int)op_extra_data->est_latency
                    - (int)src_sync_usec_to_sec_frac(us_since),
                    0 );

    SOSY_MSG4( SRC_SYNC_TRACE_KICK,
               "calc: t %d est_latency(in) 0.%05d us_since %d el 0.%05d",
               now, src_sync_sec_frac_to_10usec(op_extra_data->est_latency),
               us_since, src_sync_sec_frac_to_10usec(comp->est_latency_t));

    const unsigned SS_PERIOD = src_sync_get_period(op_extra_data);
    const unsigned SS_MAX_PERIOD = src_sync_get_max_period(op_extra_data);
    const unsigned SS_MAX_LATENCY = src_sync_get_max_latency(op_extra_data);

    /* Check Sources.
     * This only includes sources that are connected with a route defined.
     *
     *  1) Check for sink switch (MUX)
     *  2) Mark associated sink as active
     *  3) Convert space to time and verify greater than system time period
     *      Otherwise, wait for next period.
     *
     *  Note:  If any source route stalls, all source routes are stalled.
     *         This is the intended design and consistent with the
     *         purpose of this capability of syncronizing sources
     */
    if (op_extra_data->forward_kicks == 0)
    {
        SOSY_MSG1( SRC_SYNC_TRACE_COMPUTE_TRANSFER,
                   "calc: nothing to do, est_latency 0.%05d",
                   src_sync_sec_frac_to_10usec(comp->est_latency_t));

        op_extra_data->est_latency = (unsigned)comp->est_latency_t;
        return 0; /* means, nothing to do, wait for next kick */
    }

    unsigned min_src_space_t;
    unsigned max_src_space_t;

    src_sync_compute_space(op_extra_data,&min_src_space_t,&max_src_space_t);

    SOSY_MSG2( SRC_SYNC_TRACE_SRC_SPACE,
               "calc srcs min_sp 0.%05d max_sp 0.%05d",
               src_sync_sec_frac_to_10usec(min_src_space_t),
               src_sync_sec_frac_to_10usec(max_src_space_t));

    if (max_src_space_t < SS_PERIOD)
    {
        /* Strict timing (i.e. non-stallable) source groups don't have
         * enough space to process. Skip to next period.
         */
        SOSY_MSG1( SRC_SYNC_TRACE_COMPUTE_TRANSFER,
                   "calc transfer 0.%05d not enough to do",
                   src_sync_sec_frac_to_10usec(max_src_space_t));
        op_extra_data->est_latency = (unsigned)comp->est_latency_t;
        return 0; /* means, nothing to do, wait for next kick */
    }

    /*
     * A special case which occurs frequently enough:
     * If all inputs are unconnected, i.e. this operator only
     * generates silence, limit the estimated amount to MAX_LATENCY
     * and thus limit the rate to approx. the nominal rate plus margin
     * for rate deviations
     */
    if (op_extra_data->all_routed_sinks_unconnected)
    {
        if (comp->est_latency_t > (SS_MAX_LATENCY - SS_PERIOD))
        {
            SOSY_MSG1( SRC_SYNC_TRACE_COMPUTE_TRANSFER,
                       "calc transfer, all inputs unconnected, est 0.%05d wait",
                       src_sync_sec_frac_to_10usec(comp->est_latency_t));
            op_extra_data->est_latency = (unsigned)comp->est_latency_t;

            return SS_PERIOD; /* means, check periodically while in this state */
        }

        unsigned silence_amount_t = SS_MAX_LATENCY - comp->est_latency_t;
        /* 5% margin for non-ideal rates */
        silence_amount_t += frac_mult(silence_amount_t, FRACTIONAL(0.05));
        min_src_space_t = pl_min(min_src_space_t, silence_amount_t);

        SOSY_MSG3( SRC_SYNC_TRACE_COMPUTE_TRANSFER,
                   "calc transfer, all inputs unconnected, est 0.%05d, silence 0.%05d, min_tr 0.%05d",
                   src_sync_sec_frac_to_10usec(comp->est_latency_t),
                   src_sync_sec_frac_to_10usec(silence_amount_t),
                   src_sync_sec_frac_to_10usec(min_src_space_t));
    }

    /* Potential fixups: may decide to change the heuristic for detecting
     * downstream buffer full (B-243778)
     */
    patch_fn_shared(src_sync);

    /*
     * The downstream buffer not emptied condition is used to reset
     * the estimated latency to the configured upper threshold (MAX_LATENCY).
     * To avoid triggering this by a kick between writing to downstream
     * and the downstream operator consuming, use the running history maximum.
     */
    src_sync_put_buffer_history( &op_extra_data->source_buffer_history,
                                 op_extra_data->time_stamp, max_src_space_t);
    unsigned max_src_space_over_history_t =
            src_sync_get_buffer_history_max(
                    &op_extra_data->source_buffer_history,
                    op_extra_data->time_stamp,
                    SRC_SYNC_BUFFER_LEVEL_HISTORY_PERIOD );

    if (max_src_space_over_history_t < op_extra_data->max_space_t)
    {
        /* max_space was calculated to be the minimum of the buffer sizes
        ** connected to the sources.
        ** If the minimum of the space available on all sources falls below this
        ** threshold, it means that all source buffers contain unconsumed data.
        ** This is interpreted as meaning that the downstream chains
        ** have been filled. Then we take SS_MAX_LATENCY as a given amount of
        ** buffered data, something like: minimum over all downstream chains,
        ** of sum of buffer size for the source endpoint, and average buffer
        ** contents for all further buffers, scaled as time. Subtract min_period
        ** (unused buffer space still included in the calculation so far)
        ** to get a new est_latency.
        */
        if (pl_min(SS_MAX_LATENCY,SS_MAX_PERIOD) > max_src_space_over_history_t)
        {
            int new_latency_t = SS_MAX_LATENCY - max_src_space_over_history_t;
            if (new_latency_t != comp->est_latency_t)
            {
                comp->est_latency_t = new_latency_t;
                SOSY_MSG3( SRC_SYNC_TRACE_SRC_SPACE_FILLED,
                           "calc source buffers non-empty "
                           "(max_t 0.%05d < max_s 0.%05d), "
                           "set est_latency 0.%05d",
                           src_sync_sec_frac_to_10usec(max_src_space_over_history_t),
                           src_sync_sec_frac_to_10usec(op_extra_data->max_space_t),
                           src_sync_sec_frac_to_10usec(comp->est_latency_t));
            }

            comp->downstream_filled = TRUE;
        }
    }

    /* Initialize the limit on the global transfer based on space
     * at (non-stallable) sources. This will be capped by other
     * amounts such as data available at non-stalled sinks later.
     */
    comp->max_transfer_t = min_src_space_t;

    /* Minimum amount to write to sources in order to avoid underrun for
     * the next SS_PERIOD
     */
    if (comp->est_latency_t >= (int)(SS_MAX_PERIOD + SS_PERIOD))
    {
        comp->min_transfer_t = 0;
    }
    else
    {
        comp->min_transfer_t = (int)(SS_MAX_PERIOD + SS_PERIOD) - comp->est_latency_t;

        /* Avoid writing tiny amounts of silence while
         * scraping along near minimum buffer fill
         */
        comp->min_transfer_t = pl_max(comp->min_transfer_t,
                                   pl_min(SS_PERIOD, comp->max_transfer_t));
    }

    if (comp->min_transfer_t > comp->max_transfer_t)
    {
        /* Make the values consistent. This should not happen with
         * sane parameters: when source buffers are full, est_latency
         * should have been set to nearly SS_MAX_LATENCY above.
         *
         * Sanity check:
         * SS_MAX_PERIOD + SS_PERIOD <= SS_MAX_LATENCY
         *
         * The way to get here may be: output buffers suddenly filled
         * but history still contains some recent not-full records
         * and meanwhile passing time brought est_latency below threshold.
         */
        L2_DBG_MSG("src_sync WARNING cannot write minimum "
                   "to avoid underrun (check parameters)");

        comp->min_transfer_t = comp->max_transfer_t;
    }
    /* Return a negative value to proceed.
     * Note only the sign matters here.
     */
    return -1;
}

int src_sync_compute_transfer_sinks(SRC_SYNC_OP_DATA* op_extra_data,
                                    SRC_SYNC_COMP_CONTEXT* comp)
{
    const unsigned SS_PERIOD = src_sync_get_period(op_extra_data);
    const unsigned SS_MAX_PERIOD = src_sync_get_max_period(op_extra_data);
    SRC_SYNC_SOURCE_GROUP* src_grp;
    SRC_SYNC_SINK_GROUP* sink_grp;
    bool have_pending_sinks;

    patch_fn_shared(src_sync);

    /* Check Sinks.   Sink list is all connected sinks.
       Only interested in sinks for active routes or with purge flag set

       1) Get amount of data at terminal
       2) Limit transfer based on data available.   Check for stalls  */
    have_pending_sinks = FALSE;

    for ( sink_grp = op_extra_data->sink_groups;
          sink_grp != NULL;
          sink_grp = next_sink_group(sink_grp))
    {
        if (sink_grp->common.terminals != NULL)
        {
            unsigned sink_grp_data_w = src_sync_calc_sink_group_available_data(sink_grp);

            SOSY_MSG2( SRC_SYNC_TRACE_SINK_AVAIL,
                       "sink_g%d avail_w %d",
                       sink_grp->common.idx, sink_grp_data_w );

            sink_grp->common.transfer = sink_grp_data_w;

            if (sink_grp_data_w < MAXINT)
            {
                unsigned sink_grp_data_t = src_sync_samples_to_time(
                        sink_grp_data_w, sink_grp->common.inv_sample_rate);
                bool have_data;

                have_data = (sink_grp_data_t >= SS_PERIOD);

                sink_grp->copy_before_silence = FALSE;

                if ( (sink_grp->stall_state != SRC_SYNC_SINK_NOT_CONNECTED)
                     && ! sink_grp->common.connected )
                {
                    SOSY_MSG1( SRC_SYNC_TRACE_COMPUTE_TRANSFER,
                               "sink_g%d disconnected",
                               sink_grp->common.idx);
                    src_sync_set_sink_state( op_extra_data, sink_grp,
                                             SRC_SYNC_SINK_NOT_CONNECTED);
                }

                /* Transition based on input buffer or downstream_filled */
                switch (sink_grp->stall_state)
                {
                case SRC_SYNC_SINK_FLOWING:
                    if (! have_data)
                    {
                        if (comp->min_transfer_t == 0)
                        {
                            SOSY_MSG3( SRC_SYNC_TRACE_PENDING,
                                       "calc sink_g%d sg_w %d "
                                       "sg_t 0.%05d: late",
                                       sink_grp->common.idx, sink_grp_data_w,
                                       src_sync_sec_frac_to_10usec(
                                           sink_grp_data_t));

                            src_sync_set_sink_state( op_extra_data, sink_grp,
                                                     SRC_SYNC_SINK_PENDING);
                            have_pending_sinks = TRUE;
                            /* No transfers */
                        }
                        else
                        {
                            SOSY_MSG3( SRC_SYNC_TRACE_STALLED,
                                       "calc sink_grp %d sg_w %d "
                                       "sg_t 0.%05d: stalled",
                                       sink_grp->common.idx, sink_grp_data_w,
                                       src_sync_sec_frac_to_10usec(
                                           sink_grp_data_t));

                            op_extra_data->stat_sink_stalled |=
                                    sink_grp->common.channel_mask;
                            op_extra_data->stat_sink_stall_occurred |=
                                    sink_grp->common.channel_mask;
                            src_sync_set_sink_state( op_extra_data, sink_grp,
                                                     SRC_SYNC_SINK_STALLED );

                            /* Enter stalled state: reset silence counter */
                            sink_grp->inserted_silence_words = 0;

                            /* copy sink_grp_data_t, then silence
                             * up to min_transfer_t == max_transfer_t
                             */
                            sink_grp->copy_before_silence = TRUE;
                        }
                    }
                    else
                    {
                        SOSY_MSG4( SRC_SYNC_TRACE_FLOWING,
                                   "calc sink_g%d sg_w %d sg_t 0.%05d "
                                   "flowing max_t 0.%05d",
                                   sink_grp->common.idx,
                                   sink_grp_data_w,
                                   src_sync_sec_frac_to_10usec(sink_grp_data_t),
                                   src_sync_sec_frac_to_10usec(comp->max_transfer_t));
                        /* only copy */
                    }
                    break;

                case SRC_SYNC_SINK_PENDING:
                    if (! have_data)
                    {
                        /* not enough data */
                        /* Have we waited too long? */
                        if (comp->min_transfer_t != 0)
                        {
                            op_extra_data->stat_sink_stalled |=
                                    sink_grp->common.channel_mask;
                            op_extra_data->stat_sink_stall_occurred |=
                                    sink_grp->common.channel_mask;
                            src_sync_set_sink_state( op_extra_data, sink_grp,
                                                     SRC_SYNC_SINK_STALLED);

                            /* Enter stalled state: reset silence counter */
                            sink_grp->inserted_silence_words = 0;

                            /* copy sink_grp_data_t,
                             * then silence up to min_transfer_t == max_transfer_t
                             */
                            sink_grp->copy_before_silence = TRUE;
                        }
                        else
                        {
                            SOSY_MSG1( SRC_SYNC_TRACE_PENDING,
                                       "calc src_g%d still pending",
                                       sink_grp->common.idx);
                            have_pending_sinks = TRUE;
                            /* No transfers */
                        }
                    }
                    else
                    {
                        op_extra_data->stat_sink_stalled &=
                                ~ sink_grp->common.channel_mask;
                        src_sync_set_sink_state( op_extra_data, sink_grp,
                                                 SRC_SYNC_SINK_FLOWING);
                    }
                    break;

                case SRC_SYNC_SINK_STALLED:
#ifdef INSTALL_METADATA
                case SRC_SYNC_SINK_RECOVERING_WAITING_FOR_TAG:
#endif /* INSTALL_METADATA */
                    /* Look at data when there is more than SS_PERIOD of it */
                    if (have_data)
                    {
#ifdef INSTALL_METADATA
                        if (sink_grp->common.metadata_enabled)
                        {
                            unsigned peek_beforeidx;
                            unsigned gap_samples;
                            metadata_tag* peeked = buff_metadata_peek_ex(
                                    sink_grp->metadata_input_buffer,
                                    &peek_beforeidx);

                            src_sync_stall_recovery_type recovery =
                                src_sync_peek_resume(
                                    op_extra_data,
                                    peeked, &sink_grp->timestamp_state,
                                    peek_beforeidx, sink_grp_data_w,
                                    sink_grp->common.sample_rate,
                                    &gap_samples, sink_grp->common.idx );

                            switch (recovery)
                            {
                            default:
                                /* src_sync_peek_resume does not return other values */
                            case SRC_SYNC_STALL_RECOVERY_UNKNOWN:
                                /* Fill downstream with a configured amount
                                 * of silence. Don't consume data or metadata
                                 * until filled.
                                 */
                                sink_grp->stall_recovery_silence_words =
                                    rate_time_to_samples(
                                        src_sync_get_stall_recovery_default_fill(op_extra_data),
                                        sink_grp->common.sample_rate );
                                sink_grp->filling_until_full = FALSE;
                                sink_grp->inserted_silence_words = 0;
                                src_sync_set_sink_state(
                                        op_extra_data, sink_grp,
                                        SRC_SYNC_SINK_RECOVERING_FILLING );
                                break;

                            case SRC_SYNC_STALL_RECOVERY_WAITING_FOR_TAG:
                                /* Consume the data and metadata,
                                 * send minimum amount of silence downstream
                                 * to prevent underruns
                                 */
                                src_sync_set_sink_state(
                                        op_extra_data, sink_grp,
                                        SRC_SYNC_SINK_RECOVERING_WAITING_FOR_TAG );
                                /* only silence */
                                break;

                            case SRC_SYNC_STALL_RECOVERY_RESTART:
                                /* Fill downstream until full. Don't
                                 * consume data or metadata until then.
                                 */
                                src_sync_set_sink_recovering_restarting(
                                        op_extra_data, sink_grp );
                                break;

                            case SRC_SYNC_STALL_RECOVERY_GAP:
                                /* gap_samples is valid only in this case */
                                if ( gap_samples
                                     < sink_grp->inserted_silence_words)
                                {
                                    /* Already sent more silence than
                                     * the gap turned out to be.
                                     * Discard some data to re-align.
                                     * Insert minimum amount of silence; add this
                                     * to the amount of input that needs to be
                                     * discarded.
                                     */
                                    sink_grp->stall_recovery_discard_words =
                                        sink_grp->inserted_silence_words
                                        - gap_samples;
                                    src_sync_set_sink_state(
                                            op_extra_data, sink_grp,
                                            SRC_SYNC_SINK_RECOVERING_DISCARDING );

                                    /* Limit on waiting for input to catch up,
                                     * expressed in length of silence sent while
                                     * waiting
                                     */
                                    sink_grp->stall_recovery_discard_remaining =
                                        rate_time_to_samples(
                                            op_extra_data->cur_params
                                            .OFFSET_SS_STALL_RECOVERY_CATCHUP_LIMIT,
                                            sink_grp->common.sample_rate);
                                }
                                else if ( gap_samples
                                          > sink_grp->inserted_silence_words )
                                {
                                    /* Gap is longer than the silence sent so far.
                                     * Send more silence as fast as the output
                                     * accepts it.
                                     */
                                    sink_grp->stall_recovery_silence_words =
                                        gap_samples
                                        - sink_grp->inserted_silence_words;
                                    sink_grp->filling_until_full = FALSE;
                                    src_sync_set_sink_state(
                                            op_extra_data, sink_grp,
                                            SRC_SYNC_SINK_RECOVERING_FILLING );

                                }
                                else
                                {
                                    /* Gap and silence happen to match. */
                                    /* Forward data and metadata. */
                                    op_extra_data->stat_sink_stalled &=
                                            ~ sink_grp->common.channel_mask;
                                    src_sync_set_sink_state(
                                            op_extra_data, sink_grp,
                                            SRC_SYNC_SINK_FLOWING );
                                }
                                break;
                            }
                        }
                        else
#endif /* INSTALL_METADATA */
                        {
                            /* Fill downstream with a configured amount
                             * of silence. Don't consume data or metadata
                             * until filled.
                             */
                            sink_grp->stall_recovery_silence_words =
                                rate_time_to_samples(
                                    src_sync_get_stall_recovery_default_fill(op_extra_data),
                                    sink_grp->common.sample_rate );
                            sink_grp->filling_until_full = TRUE;
                            sink_grp->inserted_silence_words = 0;
                            src_sync_set_sink_state(
                                    op_extra_data, sink_grp,
                                    SRC_SYNC_SINK_RECOVERING_FILLING );
                            /* Recheck after overall max_transfer_t is known,
                             * whether the inserted silence fits within
                             * this kick.
                             */
                        }
                    }
                    break;

                case SRC_SYNC_SINK_RECOVERING_RESTARTING:
                    /* If downstream buffers have been filled, start copying.
                     * Until then, write as much silence as the outputs
                     * accept.
                     */
                    if (comp->downstream_filled)
                    {
                        SOSY_MSG1( SRC_SYNC_TRACE_RECOVERED,
                                   "restarted after fill_w %d",
                                   sink_grp->inserted_silence_words);
                        op_extra_data->stat_sink_stalled &=
                                ~ sink_grp->common.channel_mask;
                        src_sync_set_sink_state( op_extra_data, sink_grp,
                                                 SRC_SYNC_SINK_FLOWING);
                    }
                    break;

                case SRC_SYNC_SINK_RECOVERING_FILLING:
                    /* If we get here at the start of process_data, it means
                     * the next step is sending silence downstream, and not
                     * consuming data; so not dependent on sink state.
                     */
                    /* Recheck after overall max_transfer_t is known,
                     * whether the inserted silence fits within
                     * this kick.
                     */
                    /* If filling_until_full is set, check if downstream
                     * buffers have been filled once, and if so,
                     * switch to copying.
                     */
                    if (sink_grp->filling_until_full && comp->downstream_filled)
                    {
                        SOSY_MSG1( SRC_SYNC_TRACE_RECOVERED,
                                   "recovered after fill_w %d",
                                   sink_grp->inserted_silence_words);
                        op_extra_data->stat_sink_stalled &=
                                ~ sink_grp->common.channel_mask;
                        src_sync_set_sink_state( op_extra_data, sink_grp,
                                                 SRC_SYNC_SINK_FLOWING);
                    }
                    break;

#ifdef INSTALL_METADATA
                case SRC_SYNC_SINK_RECOVERING_DISCARDING:
                    /* If we get here at the start of process_data, it means
                     * that at the end of the last process_data, still more
                     * samples had to be discarded from input.
                     * If additional silence samples have to be sent in the
                     * meantime to prevent underruns, add that amount to
                     * stall_recovery_discard_words.
                     */
                    break;
#endif /* INSTALL_METADATA */

                case SRC_SYNC_SINK_NOT_CONNECTED:
                    /* sink_grp_data_w < MAXINT implied connected,
                     * so transition
                     */
                    if (sink_grp->common.connected)
                    {
                        SOSY_MSG1( SRC_SYNC_TRACE_COMPUTE_TRANSFER,
                                   "sink_g%d connected",
                                   sink_grp->common.idx);

                        src_sync_set_sink_recovering_restarting(
                                op_extra_data, sink_grp );
                    }
                    break;

                default:
                    break;
                }

                switch (sink_grp->stall_state)
                {
                case SRC_SYNC_SINK_FLOWING:
                    /* Limit Transfer to available data */
                    comp->max_transfer_t = pl_min(comp->max_transfer_t, sink_grp_data_t);
                    break;

                case SRC_SYNC_SINK_STALLED:
#ifdef INSTALL_METADATA
                case SRC_SYNC_SINK_RECOVERING_WAITING_FOR_TAG:
                case SRC_SYNC_SINK_RECOVERING_DISCARDING:
#endif /* INSTALL_METADATA */
                    /* While stalled, transfer minimum */
                    comp->max_transfer_t = comp->min_transfer_t;
                    break;

                case SRC_SYNC_SINK_RECOVERING_FILLING:
                    {
                        /* Limit current transfer to what could
                         * be sent immediately
                         */
                        unsigned available_now_w =
                            sink_grp->stall_recovery_silence_words
                            + sink_grp_data_w;
                        unsigned available_now_t =
                            src_sync_samples_to_time(
                                available_now_w,
                                sink_grp->common.inv_sample_rate);
                        comp->max_transfer_t = pl_min(comp->max_transfer_t,
                                                      available_now_t);
                    }
                    break;

                case SRC_SYNC_SINK_RECOVERING_RESTARTING:
                case SRC_SYNC_SINK_NOT_CONNECTED:
                    /* Transfer is not limited by this route;
                     * means produce as much silence
                     * as needed.
                     */
                    break;

                case SRC_SYNC_SINK_PENDING:
                    /* Nothing to adjust, will exit below and
                     * not transfer anything
                     */
                default:
                    break;
                }
            } /* active */
        }
    }

    /* Fixups, initial conditions have been modified */
    patch_fn_shared(src_sync);

    if (have_pending_sinks)
    {
        /* have_pending_sinks is only set when min_transfer_t == 0,
         * that guarantees est_latency_t >= (int)(SS_MAX_PERIOD + SS_PERIOD),
         * thus wait_period >= SS_PERIOD.
         */
        /* time left until underrun */
        unsigned wait_period = comp->est_latency_t - SS_MAX_PERIOD;

        SOSY_MSG4( SRC_SYNC_TRACE_PENDING,
                   "calc pending est_lat_t 0.%05d max_per_t 0.%05d ss_period 0.%05d still time 0.%05d",
                   src_sync_sec_frac_to_10usec(comp->est_latency_t),
                   src_sync_sec_frac_to_10usec(SS_MAX_PERIOD),
                   src_sync_sec_frac_to_10usec(SS_PERIOD),
                   src_sync_sec_frac_to_10usec(wait_period));

        op_extra_data->est_latency = (unsigned)comp->est_latency_t;
        return wait_period;
    }

    SOSY_MSG1( SRC_SYNC_TRACE_COMPUTE_TRANSFER,
               "calc after sinks, max_transfer 0.%05d",
               src_sync_sec_frac_to_10usec(comp->max_transfer_t));

    /* Actual transferred amount is rounded down to whole samples */
    unsigned act_transfer_t = 0;

    for ( src_grp = op_extra_data->source_groups;
          src_grp != NULL;
          src_grp = next_source_group(src_grp))
    {
        /* Limit source transfer amounts to the overall
         * amount, converted to samples.
         * The result may be smaller than the overall amount,
         * if a source is stalled.
         */

        if (src_grp->common.terminals != NULL)
        {
            unsigned src_grp_transfer_w;

            src_grp_transfer_w = rate_time_to_samples_trunc(
                    comp->max_transfer_t,
                    src_grp->common.sample_rate );
            src_grp->min_transfer_w = rate_time_to_samples_trunc(
                    comp->min_transfer_t,
                    src_grp->common.sample_rate );

            SOSY_MSG4( SRC_SYNC_TRACE_COMPUTE_TRANSFER,
                       "calc src_g%d: transfer space %d write %d min %d",
                       src_grp->common.idx, src_grp->common.transfer,
                       src_grp_transfer_w, src_grp->min_transfer_w);

            src_grp->common.transfer = pl_min(src_grp->common.transfer,
                                              src_grp_transfer_w);

            act_transfer_t = pl_max(act_transfer_t,
                                    src_sync_samples_to_time(src_grp->common.transfer,
                                                             src_grp->common.inv_sample_rate));
        }
    }

    if (act_transfer_t != 0)
    {
        comp->est_latency_t += act_transfer_t;
        op_extra_data->est_latency = (unsigned)comp->est_latency_t;
        SOSY_MSG3( SRC_SYNC_TRACE_COMPUTE_TRANSFER,
                   "calc max_transfer 0.%05d act_transfer 0.%05d el 0.%05d",
                   src_sync_sec_frac_to_10usec(comp->max_transfer_t),
                   src_sync_sec_frac_to_10usec(act_transfer_t),
                   src_sync_sec_frac_to_10usec(comp->est_latency_t));

        /* Save transfer (negative to differentiate from stall timer */
        /* note the magnitude is not used, only the sign */
        return (- act_transfer_t);
    }
    else
    {
        /* After rounding down to whole samples, there was nothing to do.
         * Recheck soon. */
        op_extra_data->est_latency = (unsigned)comp->est_latency_t;
        return SS_PERIOD;
    }
}

unsigned src_sync_route_write_silence( SRC_SYNC_OP_DATA *op_extra_data,
                                       SRC_SYNC_SINK_GROUP *sink_grp,
                                       unsigned words )
{
    unsigned written = 0;
    SRC_SYNC_SINK_ENTRY* sink_ptr;

    patch_fn(src_sync_route_silence);

#ifdef INSTALL_METADATA
    if (sink_grp->metadata_dest != NULL)
    {
        src_sync_metadata_silence( op_extra_data,
                                   &(sink_grp->metadata_dest->common),
                                   &(sink_grp->metadata_dest->metadata_dest),
                                   words * OCTETS_PER_SAMPLE);
    }
#endif /* INSTALL_METADATA */

    for ( sink_ptr = sink_entries_from_group(sink_grp);
          sink_ptr != NULL;
          sink_ptr = next_sink_entry(sink_ptr) )
    {
        SRC_SYNC_SOURCE_ENTRY* src_ptr = sink_ptr->source;

        if (src_ptr != NULL)
        {
            do
            {
#ifdef SOSY_CHECK_BLOCK_FILL
                unsigned space_before_w, space_after_w;
                space_before_w =
                    cbuffer_calc_amount_space_in_words(src_ptr->common.buffer);
#endif /* SOSY_CHECK_BLOCK_FILL */

                if (src_ptr->common.buffer != NULL)
                {
                    cbuffer_block_fill( src_ptr->common.buffer, words, 0);
                }
                else
                {
                    fault_diatribe(FAULT_AUDIO_SRC_SYNC_WRITE_UNCONNECTED_TERMINAL,
                                   src_ptr->common.idx);
                }

#ifdef SOSY_CHECK_BLOCK_FILL
                space_after_w =
                    cbuffer_calc_amount_space_in_words(src_ptr->common.buffer);
                SOSY_MSG3( SRC_SYNC_TRACE_PERFORM_TRANSFER,
                           "src_sync block fill before_w %d w %d after_w %d",
                           space_before_w, words, space_after_w);
#endif /* SOSY_CHECK_BLOCK_FILL */

                src_ptr = src_ptr->next_split_source;
            }
            while (src_ptr != NULL);

            written = words;
        }
    }

    /* Keep state */
    sink_grp->inserted_silence_words += written;

    SOSY_MSG4( SRC_SYNC_TRACE_PERFORM_TRANSFER,
               "pt g%d->g%d silence_w %d tot %d",
               sink_grp->common.idx, sink_grp->route_dest->common.idx,
               words, sink_grp->inserted_silence_words);

    return written;
}

#ifdef INSTALL_METADATA
/**
 * Discard specified number of words from input buffer,
 * only on the inputs which have a route
 * (because the unrouted ones are handled separately)
 */
unsigned src_sync_route_discard_input( SRC_SYNC_OP_DATA *op_extra_data,
                                       SRC_SYNC_SINK_GROUP *sink_grp,
                                       unsigned words )
{
    unsigned consumed = 0;
    SRC_SYNC_SINK_METADATA* md = &sink_grp->timestamp_state;

    patch_fn(src_sync_route_discard);

    /* Shall not be called in SINK_NOT_CONNECTED state */
    PL_ASSERT(sink_grp->common.connected);

#ifdef INSTALL_METADATA
    src_sync_get_sink_metadata( op_extra_data, sink_grp,
                                words*OCTETS_PER_SAMPLE );

    if (sink_grp->metadata_dest != NULL)
    {
        /* Cache any EOF tags */
        src_sync_metadata_drop_tags( op_extra_data, md->received,
                                     &sink_grp->metadata_dest->metadata_dest);
    }
    else
    {
#ifdef SOSY_CHECK_METADATA_TAG_COUNTS
        op_extra_data->num_tags_deleted +=
                src_sync_metadata_count_tags(md->received);
#endif /* SOSY_CHECK_METADATA_TAG_COUNTS */
        buff_metadata_tag_list_delete(md->received);
    }
    md->received = NULL;
#endif /* INSTALL_METADATA */

    SRC_SYNC_SINK_ENTRY* sink_ptr;
    for ( sink_ptr = sink_entries_from_group(sink_grp);
          sink_ptr != NULL;
          sink_ptr = next_sink_entry(sink_ptr) )
    {
        SRC_SYNC_SOURCE_ENTRY* src_ptr = sink_ptr->source;

        if (src_ptr != NULL)
        {
            PL_ASSERT(sink_ptr->input_buffer != NULL);

            cbuffer_advance_read_ptr( sink_ptr->input_buffer, words );
            consumed = words;
        }
    }

    SOSY_MSG3( SRC_SYNC_TRACE_PERFORM_TRANSFER,
               "pt g%d->g%d discard_w %d",
               sink_grp->common.idx, sink_grp->route_dest->common.idx, words);

    return consumed;
}
#endif /* INSTALL_METADATA */

unsigned src_sync_route_copy( SRC_SYNC_OP_DATA *op_extra_data,
                              SRC_SYNC_SINK_GROUP *sink_grp,
                              unsigned words )
{
    unsigned written = 0;

    patch_fn(src_sync_route_copy);

    if (words > 0)
    {
        PL_ASSERT(sink_grp->common.connected);

#ifdef INSTALL_METADATA
        SRC_SYNC_SINK_METADATA* md = &sink_grp->timestamp_state;
        unsigned copy_octets = words*OCTETS_PER_SAMPLE;

        src_sync_get_sink_metadata( op_extra_data, sink_grp, copy_octets );

        if (sink_grp->metadata_dest != NULL)
        {
            src_sync_metadata_forward(
                    op_extra_data, md, copy_octets,
                    &(sink_grp->metadata_dest->common),
                    &(sink_grp->metadata_dest->metadata_dest));
        }
        else
        {
#ifdef SOSY_CHECK_METADATA_TAG_COUNTS
            op_extra_data->num_tags_deleted +=
                    src_sync_metadata_count_tags(md->received);
#endif /* SOSY_CHECK_METADATA_TAG_COUNTS */
            buff_metadata_tag_list_delete(md->received);
        }
        md->received = NULL;
#endif /* INSTALL_METADATA */

        SRC_SYNC_SINK_ENTRY* sink_ptr;
        for ( sink_ptr = sink_entries_from_group(sink_grp);
              sink_ptr != NULL;
              sink_ptr = next_sink_entry(sink_ptr) )
        {
            SRC_SYNC_SOURCE_ENTRY* src_ptr = sink_ptr->source;

            if (src_ptr != NULL)
            {
                PL_ASSERT(sink_ptr->input_buffer != NULL);
                PL_ASSERT(src_ptr->current_route.sink == sink_ptr);

                /* This takes care of gain but doesn't move
                 * input buffer pointers
                 */
                if (src_ptr->common.buffer != NULL)
                {
                    src_sync_transfer_route(src_ptr, sink_ptr->input_buffer, words);
                }
                else
                {
                    fault_diatribe(FAULT_AUDIO_SRC_SYNC_WRITE_UNCONNECTED_TERMINAL,
                                   src_ptr->common.idx);
                }

                while (src_ptr->next_split_source != NULL)
                {
                    src_ptr = src_ptr->next_split_source;
                    PL_ASSERT(src_ptr->current_route.sink == sink_ptr);
                    if (src_ptr->common.buffer != NULL)
                    {
                        src_sync_transfer_route(src_ptr, sink_ptr->input_buffer, words);
                    }
                    else
                    {
                        fault_diatribe(FAULT_AUDIO_SRC_SYNC_WRITE_UNCONNECTED_TERMINAL,
                                       src_ptr->common.idx);
                    }
                }

                cbuffer_advance_read_ptr( sink_ptr->input_buffer, words);

                written = words;
            }
        }
    }

    SOSY_MSG3( SRC_SYNC_TRACE_PERFORM_TRANSFER,
               "pt g%d->g%d copy_w %d",
               sink_grp->common.idx, sink_grp->route_dest->common.idx, words);

    return written;
}

unsigned src_sync_perform_transfer(SRC_SYNC_OP_DATA *op_extra_data)
{
    unsigned sink_kicks = 0;
    SRC_SYNC_SINK_GROUP* sink_grp;

#ifdef INSTALL_METADATA
#ifdef SOSY_CHECK_METADATA_TRANSPORT_POINTERS
    src_sync_check_md_transport_pre(op_extra_data);
#endif /* SOSY_CHECK_METADATA_TRANSPORT_POINTERS */
#ifdef SOSY_CHECK_METADATA_TAG_COUNTS
    src_sync_clear_tag_counts(op_extra_data);
#endif /* SOSY_CHECK_METADATA_TAG_COUNTS */
#endif /* INSTALL_METADATA */

    for ( sink_grp = op_extra_data->sink_groups;
          sink_grp != NULL;
          sink_grp = next_sink_group(sink_grp))
    {
        /* This state should not get here */
        PL_ASSERT(sink_grp->stall_state != SRC_SYNC_SINK_PENDING);

        /* Any other state may expect all sink buffers to be connected */
        PL_ASSERT( sink_grp->common.connected
                   || (sink_grp->stall_state == SRC_SYNC_SINK_NOT_CONNECTED));

        if (sink_grp->route_dest != NULL)
        {
            /* Keep track of number of words which were copied
             * to a route destination; afterwards, unrouted sinks
             * have to discard data to match.
             */
            unsigned consumed_via_route_w = 0;
            unsigned sink_w = sink_grp->common.transfer;
            unsigned src_w = sink_grp->route_dest->common.transfer;
            unsigned src_min_w = sink_grp->route_dest->min_transfer_w;

            switch (sink_grp->stall_state)
            {
            case SRC_SYNC_SINK_FLOWING:
                /* copy to source */
                consumed_via_route_w =
                        src_sync_route_copy( op_extra_data, sink_grp,
                                             pl_min(sink_w, src_w));
                break;

            case SRC_SYNC_SINK_STALLED:
                if (sink_grp->copy_before_silence)
                {
                    /* copy remaining sink data to source */
                    sink_grp->copy_before_silence = FALSE;

                    consumed_via_route_w =
                            src_sync_route_copy( op_extra_data,
                                                 sink_grp,
                                                 pl_min(sink_w, src_w));

                    /* write silence up to minimum transfer */
                    if (consumed_via_route_w < src_min_w)
                    {
                        src_sync_route_write_silence(
                                op_extra_data, sink_grp,
                                src_min_w - consumed_via_route_w);
                    }
                }
                else
                {
                    /* write minimum silence */
                    src_sync_route_write_silence(
                            op_extra_data, sink_grp, src_min_w );
                }
                break;

            case SRC_SYNC_SINK_NOT_CONNECTED:
                /* There is no input data to consume. Write silence,
                 * limited to MAX_LATENCY (see all_routed_sinks_unconnected). */
                src_sync_route_write_silence( op_extra_data, sink_grp, src_w );
                break;

            case SRC_SYNC_SINK_RECOVERING_RESTARTING:
                /* Do not consume input, write maximum silence */
                src_sync_route_write_silence( op_extra_data, sink_grp, src_w );

                /* est_latency has already been updated to include this silence.
                 * Limit amount of initial silence: switch this sink to FLOWING
                 * before the next output buffer full of silence would put
                 * the latency above the target.
                 * src_sync_set_sink_recovering_restarting() ensures that
                 * when in this state, SS_MAX_LATENCY > max_space_t. */
                if (op_extra_data->est_latency
                    >= (src_sync_get_max_latency(op_extra_data)
                        - op_extra_data->max_space_t))
                {
                    SOSY_MSG4( SRC_SYNC_TRACE_COMPUTE_TRANSFER,
                               "sink_g%d recovering_restarting el reached 0.%05d target 0.%05d threshold 0.%05d",
                               sink_grp->common.idx,
                               src_sync_sec_frac_to_10usec(op_extra_data->est_latency),
                               src_sync_sec_frac_to_10usec(src_sync_get_max_latency(op_extra_data)),
                               src_sync_sec_frac_to_10usec(src_sync_get_max_latency(op_extra_data)
                                                           - op_extra_data->max_space_t));
                    src_sync_set_sink_state( op_extra_data, sink_grp,
                                             SRC_SYNC_SINK_FLOWING);
                }
                break;

            case SRC_SYNC_SINK_RECOVERING_FILLING:
                {
                    /* Write silence up to stall_recovery_silence_words,
                     * subtract silence words written from
                     * stall_recovery_silence_words,
                     * if zero switch to flowing,
                     * if that was less than overall transfer, copy from input
                     */
                    unsigned silence_words =
                            pl_min( sink_grp->stall_recovery_silence_words,
                                    src_w );

                    sink_grp->inserted_silence_words +=
                            src_sync_route_write_silence(
                                    op_extra_data, sink_grp, silence_words );

                    sink_grp->stall_recovery_silence_words -= silence_words;
                    if (sink_grp->stall_recovery_silence_words == 0)
                    {
                        op_extra_data->stat_sink_stalled &=
                                ~ sink_grp->common.channel_mask;
                        src_sync_set_sink_state( op_extra_data, sink_grp,
                                                 SRC_SYNC_SINK_FLOWING );
                    }

                    if (silence_words < src_w)
                    {
                        consumed_via_route_w =
                                src_sync_route_copy(
                                        op_extra_data, sink_grp,
                                        src_w - silence_words );
                    }
                }
                break;

#ifdef INSTALL_METADATA
            case SRC_SYNC_SINK_RECOVERING_WAITING_FOR_TAG:
                /* Discard input */

                consumed_via_route_w =
                        src_sync_route_discard_input(
                                op_extra_data, sink_grp, sink_w );

                /* write minimum silence */
                src_sync_route_write_silence( op_extra_data, sink_grp,
                                              src_min_w);
                break;

            case SRC_SYNC_SINK_RECOVERING_DISCARDING:
                /* Discard input up to stall_recovery_discard_words
                 * subtract words discarded from stall_recovery_discard_words
                 * if that becomes zero, and there is still input, and that is
                 * at least minimum transfer, switch to flowing, and copy from
                 * input.
                 * Else write minimum silence, add words written to
                 * stall_recovery_discard_words, subtract silence words
                 * written from stall_recovery_discard_remaining,
                 * if that runs out, switch to restarting
                 */
                {
                    unsigned input_remaining_w;
                    unsigned discard_w =
                            pl_min( sink_w,
                                    sink_grp->stall_recovery_discard_words );

                    consumed_via_route_w =
                            src_sync_route_discard_input(
                                    op_extra_data, sink_grp, discard_w );

                    sink_grp->stall_recovery_discard_words -=
                            consumed_via_route_w;

                    input_remaining_w = sink_w - consumed_via_route_w;

                    if ( (sink_grp->stall_recovery_discard_words == 0)
                         && (input_remaining_w >= src_min_w) )
                    {
                        unsigned copy_w = pl_min(input_remaining_w, src_w);

                        SOSY_MSG1( SRC_SYNC_TRACE_RECOVERED,
                                   "input caught up, copy_w %d",
                                   copy_w);

                        consumed_via_route_w +=
                                src_sync_route_copy( op_extra_data, sink_grp,
                                                     copy_w );

                        op_extra_data->stat_sink_stalled &=
                                ~ sink_grp->common.channel_mask;
                        src_sync_set_sink_state( op_extra_data, sink_grp,
                                                 SRC_SYNC_SINK_FLOWING);
                    }
                    else
                    {
                        src_sync_route_write_silence( op_extra_data, sink_grp,
                                                      src_min_w );

                        sink_grp->stall_recovery_discard_words += src_min_w;

                        if ( sink_grp->stall_recovery_discard_remaining
                             <= src_min_w )
                        {
                            SOSY_MSG( SRC_SYNC_TRACE_RECOVERED,
                                      "input did not catch up, restart");

                            sink_grp->stall_recovery_discard_remaining = 0;
                            src_sync_set_sink_recovering_restarting(
                                    op_extra_data, sink_grp);
                        }
                        else
                        {
                            sink_grp->stall_recovery_discard_remaining -=
                                    src_min_w;

                            SOSY_MSG4( SRC_SYNC_TRACE_RECOVER_DISCARD,
                                       "catchup consume_w %d silence_w %d "
                                       "balance_w %d remain_w %d",
                                       consumed_via_route_w, src_min_w,
                                       sink_grp->stall_recovery_discard_words,
                                       sink_grp
                                           ->stall_recovery_discard_remaining);
                        }
                    }
                }
                break;
#endif /* INSTALL_METADATA */

            default:
                L2_DBG_MSG1( "src_sync: unhandled sink state %d",
                             sink_grp->stall_state);
                break;
            }

            /* Consume equal amount on unrouted sinks */
            if (consumed_via_route_w > 0)
            {
                SRC_SYNC_SINK_ENTRY* sink_ptr;

                PL_ASSERT(sink_grp->common.connected);

                for ( sink_ptr = sink_entries_from_group(sink_grp);
                      sink_ptr != NULL;
                      sink_ptr = next_sink_entry(sink_ptr) )
                {
                    if (sink_ptr->source == NULL)
                    {
                        PL_ASSERT(sink_ptr->input_buffer != NULL);

                        cbuffer_advance_read_ptr( sink_ptr->input_buffer,
                                                  consumed_via_route_w );
                    }
                }
            }

            if (sink_w < 2*src_w)
            {
                sink_kicks |= sink_grp->common.channel_mask;
            }
        }
        else
        {
            /* Connected and no route destination */
            if (sink_grp->common.connected && sink_grp->purge)
            {
                SRC_SYNC_SINK_ENTRY* sink_ptr;
                unsigned sink_w = sink_grp->common.transfer;
#ifdef INSTALL_METADATA
                unsigned sink_octets = sink_w * OCTETS_PER_SAMPLE;

                /* TODO cache as "metadata_connected" */
                if ( sink_grp->common.metadata_enabled
                     && (sink_grp->common.metadata_buffer != NULL))
                {
                    metadata_tag* tags;
                    unsigned before, after;

                    tags = buff_metadata_remove(
                            sink_grp->metadata_input_buffer,
                            sink_octets, &before, &after);

#ifdef SOSY_CHECK_METADATA_TAG_COUNTS
                    unsigned num_tags =
                            src_sync_metadata_count_tags(tags);
                    op_extra_data->num_tags_received += num_tags;
                    op_extra_data->num_tags_deleted += num_tags;
#endif /* SOSY_CHECK_METADATA_TAG_COUNTS */

                    buff_metadata_tag_list_delete(tags);
                }
#endif /* INSTALL_METADATA */

                for ( sink_ptr = sink_entries_from_group(sink_grp);
                      sink_ptr != NULL;
                      sink_ptr = next_sink_entry(sink_ptr) )
                {
                    PL_ASSERT(sink_ptr->input_buffer != NULL);

                    cbuffer_advance_read_ptr( sink_ptr->input_buffer, sink_w );
                }
            }
        }
    }

#ifdef INSTALL_METADATA
#ifdef SOSY_CHECK_METADATA_TRANSPORT_POINTERS
    src_sync_check_md_transport_post(op_extra_data);
#endif /* SOSY_CHECK_METADATA_TRANSPORT_POINTERS */
#ifdef SOSY_CHECK_METADATA_TAG_COUNTS
    src_sync_check_tag_counts(op_extra_data);
#endif /* SOSY_CHECK_METADATA_TAG_COUNTS */
#endif /* INSTALL_METADATA */

    return sink_kicks & op_extra_data->sinks_connected;
}

/* Prevent src_sync_process_data from doing anything.
 * It still runs when kicked but won't look at any state
 * other than the dont_process field.
 */
void src_sync_suspend_processing(SRC_SYNC_OP_DATA *op_extra_data)
{
    /* If assignment to a bool is atomic, disabling interrupts isn't necessary.
     * However, if the field was packed, the operation might not be atomic.
     * Isolate from having to know this.
     */
    LOCK_INTERRUPTS;
    op_extra_data->dont_process = TRUE;
    UNLOCK_INTERRUPTS;
}

/* Allow src_sync_process_data to process again,
 * after updating control data.
 * The caller has to also call src_sync_update_processing().
 */
void src_sync_resume_processing(SRC_SYNC_OP_DATA *op_extra_data)
{
    LOCK_INTERRUPTS;
    op_extra_data->bRinit=TRUE;
    op_extra_data->dont_process = FALSE;
    UNLOCK_INTERRUPTS;
}

void src_sync_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    SRC_SYNC_OP_DATA *op_extra_data =
            (SRC_SYNC_OP_DATA*) op_data->extra_op_data;
    int min_period;
    bool routes_changed;

    patch_fn_shared(src_sync);

    if (op_extra_data->dont_process)
    {
        return;
    }

    LOCK_INTERRUPTS;
    if (op_data->state != OP_RUNNING)
    {
        UNLOCK_INTERRUPTS;
        SOSY_MSG1( SRC_SYNC_TRACE_KICK, "0x%04x process_data not running", op_extra_data->id);
        return;
    }

    if (op_extra_data->kick_id != TIMER_ID_INVALID)
    {
        timer_cancel_event(op_extra_data->kick_id);
        op_extra_data->kick_id = TIMER_ID_INVALID;
    }
    UNLOCK_INTERRUPTS;

#ifdef VERBOSE_PROCESS_DATA
    static unsigned int serial = 0;
    SOSY_PD_MSG5("src_sync 0x%04x pd #%d t %d el 0.%05d rinit %d",
                 op_extra_data->id, serial, hal_get_time(),
                 src_sync_sec_frac_to_10usec(op_extra_data->est_latency),
                 op_extra_data->bRinit);
    serial += 1;
#endif /* VERBOSE_PROCESS_DATA */

    routes_changed = src_sync_perform_transitions_if_needed(op_extra_data);

    /* Check for Configuration change */
    if (routes_changed || op_extra_data->bRinit)
    {
        op_extra_data->bRinit = FALSE;

        /* Update state of all routes */
        src_sync_refresh_forward_routes(op_extra_data);
        src_sync_refresh_sink_list(op_extra_data);
        src_sync_refresh_source_list(op_extra_data);
#ifdef INSTALL_METADATA
        src_sync_refresh_metadata_routes(op_extra_data);
#endif /* INSTALL_METADATA */
        src_sync_refresh_connections(op_extra_data);

        /* Wakeup sinks just in case */
        touched->sinks = op_extra_data->sinks_connected;

        /* Fixups */
        patch_fn_shared(src_sync);
    }

    SRC_SYNC_COMP_CONTEXT compute_context;
    /* src_sync_compute_transfer_space returns 0 there
     * is not enough to do this time (limited by output space),
     * or a positive value to reschedule, or a negative one to proceed
     */
    min_period = src_sync_compute_transfer_space(op_extra_data, &compute_context);
    if (min_period != 0)
    {
        if (min_period < 0)
        {
            /* Pre-process rate-adjusted sink groups.
             * For those groups, the later steps work with
             * the output of rate adjustment.
             */
            src_sync_rm_process(op_extra_data, &touched->sinks);

            /* Check sinks and compute the transfer amount
            min_period == 0 if insufficient space
            min_period < 0 if data to transfer
            min_period > 0 if waiting for data
            */
            min_period = src_sync_compute_transfer_sinks(op_extra_data, &compute_context);
        }

        if (min_period > 0)
        {
            SOSY_MSG2( SRC_SYNC_TRACE_PERFORM_TRANSFER,
                       "calc min_p 0.%05d el 0.%05d",
                       src_sync_sec_frac_to_10usec(min_period),
                       src_sync_sec_frac_to_10usec(op_extra_data->est_latency));
            /* Set a timer to kick task before stall causes glitch in channels */
            LOCK_INTERRUPTS;
            op_extra_data->kick_id = timer_schedule_event_in(
                src_sync_time_to_usec(min_period),
                src_sync_timer_task,
                (void*)op_data );
            UNLOCK_INTERRUPTS;

            /* If we are waiting for a sink, lets kick it */
            SRC_SYNC_SINK_GROUP* sink_grp;
            for ( sink_grp = op_extra_data->sink_groups;
                  sink_grp != NULL;
                  sink_grp = next_sink_group(sink_grp) )
            {
                if (sink_grp->stall_state == SRC_SYNC_SINK_PENDING)
                {
                    touched->sinks |= sink_grp->common.channel_mask;
                }
            }

            /* If source buffers are nearly full, remind consumer */
            if (compute_context.downstream_filled)
            {
                touched->sources |= op_extra_data->sources_connected;
            }
        }
        else if (min_period < 0)
        {
            SOSY_MSG2( SRC_SYNC_TRACE_PERFORM_TRANSFER,
                       "calc min_p -0.%05d el 0.%05d",
                       src_sync_sec_frac_to_10usec(-min_period),
                       src_sync_sec_frac_to_10usec(op_extra_data->est_latency));
            /* Perform sink to source transfers */
            touched->sinks   |= src_sync_perform_transfer(op_extra_data);
            touched->sources = op_extra_data->forward_kicks;
        }
    }
    SOSY_MSG3( SRC_SYNC_TRACE_KICK, "at exit el 0.%05d kick bwd 0x%06x fwd 0x%06x",
               src_sync_sec_frac_to_10usec(op_extra_data->est_latency),
               touched->sinks, touched->sources);
}

