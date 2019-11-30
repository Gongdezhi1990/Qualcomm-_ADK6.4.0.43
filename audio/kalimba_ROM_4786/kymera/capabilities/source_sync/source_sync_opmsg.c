/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
#include "source_sync_defs.h"
#include "patch/patch.h"

/****************************************************************************
Private Type Definitions
*/

/*
 * Message structs
 */
typedef struct
{
    unsigned src_idx;
    unsigned snk_idx;
    unsigned sample_rate;
    unsigned gain;
    unsigned transition;
}SRC_SYNC_ROUTE_MSG_ENTRY;

typedef struct
{
    OPMSG_HEADER header;
    unsigned num_routes;
    SRC_SYNC_ROUTE_MSG_ENTRY  routes[];
} OPMSG_SRC_SYNC_SET_ROUTE_MSG;

typedef struct
{
    OPMSG_HEADER header;
    unsigned num_groups;
    unsigned data[];
} OPMSG_SRC_SYNC_SET_SINK_GROUPS_MSG;

typedef struct
{
    OPMSG_HEADER header;
    unsigned num_groups;
    unsigned data[];
} OPMSG_SRC_SYNC_SET_SOURCE_GROUPS_MSG;

/* Check that the metadata bit is the same in set_sink_groups and set_source_groups so that
 * code to parse it can be shared between the two messages' handlers.
 */
COMPILE_TIME_ASSERT(OPMSG_SOURCE_SYNC_SET_SINK_GROUPS_FLAG_METADATA==OPMSG_SOURCE_SYNC_SET_SOURCE_GROUPS_FLAG_METADATA,
                    OPMSG_SOURCE_SYNC_SET_SINK_GROUPS_FLAG_METADATA_equals_OPMSG_SOURCE_SYNC_SET_SOURCE_GROUPS_FLAG_METADATA);

#define SRC_SYNC_ROUTE_TERM_CONNECTED  0x0080
#define SRC_SYNC_ROUTE_CHANGE_PENDING  0x0040
#define SRC_SYNC_ROUTE_TERM_MASK       0x003F
#define SRC_SYNC_ROUTE_SRC_SHIFT       8

/* Check that SRC_SYNC_ROUTE_TERM_MASK is large enough to fit terminal numbers */
COMPILE_TIME_ASSERT((SRC_SYNC_ROUTE_TERM_MASK+1)>=SRC_SYNC_CAP_MAX_CHANNELS,
                    SRC_SYNC_ROUTE_TERM_MASK_fits_terminal_index);

typedef struct
{
    OPMSG_HEADER header;
    unsigned groups_msw;
    unsigned groups_lsw;
}OPMSG_SRC_SYNC_GET_ROUTE_MSG;


typedef struct
{
    unsigned route;
    unsigned sample_rate;
    unsigned gain;
}OPMSG_SRC_SYNC_GET_ROUTE_ENTRY;

/* COMMON_GET_STATUS comp_config field bits */
enum
{
    SRC_SYNC_COMP_CONFIG        = (
#ifdef INSTALL_METADATA
            SOURCE_SYNC_CONSTANT_COMP_METADATA |
#endif /* INSTALL_METADATA */
            SOURCE_SYNC_CONSTANT_COMP_SPLIT_ROUTE |
            0 )
};


/****************************************************************************
Private Function Declarations
*/


/****************************************************************************
Private Data Definitions
*/

/* Null terminated operator message handler table */
const opmsg_handler_lookup_table_entry src_sync_opmsg_handler_table[] =
{
    {OPMSG_COMMON_ID_GET_CAPABILITY_VERSION,    base_op_opmsg_get_capability_version},
    {OPMSG_COMMON_ID_SET_CONTROL,               src_sync_opmsg_obpm_set_control},
    {OPMSG_COMMON_ID_GET_PARAMS,                src_sync_opmsg_obpm_get_params},
    {OPMSG_COMMON_ID_GET_DEFAULTS,              src_sync_opmsg_obpm_get_defaults},
    {OPMSG_COMMON_ID_SET_PARAMS,                src_sync_opmsg_obpm_set_params},
    {OPMSG_COMMON_ID_GET_STATUS,                src_sync_opmsg_obpm_get_status},
    {OPMSG_COMMON_ID_SET_UCID,                  src_sync_opmsg_set_ucid},
    {OPMSG_COMMON_ID_GET_LOGICAL_PS_ID,         src_sync_opmsg_get_ps_id},
    {OPMSG_COMMON_ID_SET_BUFFER_SIZE,           src_sync_opmsg_set_buffer_size},
    {OPMSG_COMMON_SET_SAMPLE_RATE,              src_sync_opmsg_set_sample_rate},

    {OPMSG_COMMON_GET_CONFIGURATION,            src_sync_opmsg_ep_get_config},
    {OPMSG_COMMON_CONFIGURE,                    src_sync_opmsg_ep_configure},
    {OPMSG_COMMON_GET_CLOCK_ID,                 src_sync_opmsg_ep_clock_id},

    {OPMSG_SRC_SYNC_ID_SET_ROUTES,              src_sync_set_route},
    {OPMSG_SRC_SYNC_ID_GET_ROUTES,              src_sync_get_route},
    {OPMSG_SRC_SYNC_ID_SET_SINK_GROUPS,         src_sync_set_sink_groups},
    {OPMSG_SRC_SYNC_ID_SET_SOURCE_GROUPS,       src_sync_set_source_groups},
    {OPMSG_SRC_SYNC_ID_SET_TRACE_ENABLE,        src_sync_set_trace_enable},

    {0, NULL}
};

/* **************************** Operator message handlers ******************************** */

bool src_sync_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    if(!cps_control_setup(message_data, resp_length, resp_data,NULL))
    {
        return FALSE;
    }

    cps_response_set_result(resp_data,OPMSG_RESULT_STATES_UNSUPPORTED_CONTROL);

    return TRUE;
}

bool src_sync_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SRC_SYNC_OP_DATA   *op_extra_data = (SRC_SYNC_OP_DATA*)op_data->extra_op_data;

    return cpsGetParameterMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);
}


bool src_sync_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SRC_SYNC_OP_DATA   *op_extra_data = (SRC_SYNC_OP_DATA*)op_data->extra_op_data;

    patch_fn_shared(src_sync);

    return cpsGetDefaultsMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);
}

bool src_sync_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SRC_SYNC_OP_DATA   *op_extra_data = (SRC_SYNC_OP_DATA*)op_data->extra_op_data;
    bool retval;

    patch_fn_shared(src_sync);

    retval = cpsSetParameterMsgHandler(&op_extra_data->parms_def,message_data,
                                       resp_length,resp_data);

    SOSY_MSG2( SRC_SYNC_TRACE_ALWAYS, "0x%04x opmsg_obpm_set_params success %d",
                op_extra_data->id, retval);
    if (retval)
    {
        src_sync_trace_params(op_extra_data);
    }

    op_extra_data->bRinit=TRUE;
    return retval;
}

bool src_sync_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SRC_SYNC_OP_DATA   *op_extra_data = (SRC_SYNC_OP_DATA*)op_data->extra_op_data;

    unsigned  *resp;

    patch_fn_shared(src_sync);

    if(!common_obpm_status_helper(message_data,resp_length,resp_data,sizeof(SOURCE_SYNC_STATISTICS),&resp))
    {
        return FALSE;
    }

    if (resp != NULL)
    {
        unsigned comp_config = SRC_SYNC_COMP_CONFIG;
        unsigned Cur_mode = 1;

        resp = cpsPackWords(&Cur_mode, &comp_config, resp);
        resp = cpsPackWords(&op_extra_data->Dirty_flag, &op_extra_data->stat_sink_stalled, resp);
        cpsPackWords(&op_extra_data->stat_sink_stall_occurred, NULL, resp);
        op_extra_data->Dirty_flag = 0;
        op_extra_data->stat_sink_stall_occurred = op_extra_data->stat_sink_stalled;
    }

    return TRUE;
}

bool src_sync_opmsg_set_ucid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SRC_SYNC_OP_DATA   *op_extra_data = (SRC_SYNC_OP_DATA*)op_data->extra_op_data;
    PS_KEY_TYPE key;
    bool retval;

    patch_fn_shared(src_sync);

    retval = cpsSetUcidMsgHandler(&op_extra_data->parms_def, message_data,
                                  resp_length, resp_data);
    key = MAP_CAPID_UCID_SBID_TO_PSKEYID(CAP_ID_SOURCE_SYNC,
                                         op_extra_data->parms_def.ucid,OPMSG_P_STORE_PARAMETER_SUB_ID);
    ps_entry_read((void*)op_data, key, PERSIST_ANY, src_sync_ups_params);

    return retval;
}

bool src_sync_opmsg_get_ps_id(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SRC_SYNC_OP_DATA   *op_extra_data = (SRC_SYNC_OP_DATA*)op_data->extra_op_data;

    patch_fn_shared(src_sync);

    return cpsGetUcidMsgHandler(&op_extra_data->parms_def, CAP_ID_SOURCE_SYNC,
                                message_data, resp_length, resp_data);
}



bool src_sync_ups_params(
        void* instance_data, PS_KEY_TYPE key,PERSISTENCE_RANK rank,
        uint16 length, unsigned* data, STATUS_KYMERA status,
        uint16 extra_status_info)
{
    /* returns true if succesful, false if failed */
    SRC_SYNC_OP_DATA   *op_extra_data = (SRC_SYNC_OP_DATA*)((OPERATOR_DATA*)instance_data)->extra_op_data;

    patch_fn_shared(src_sync);

    cpsSetParameterFromPsStore(&op_extra_data->parms_def, length, data, status);
    op_extra_data->bRinit=TRUE;
    pfree(data);
    return(TRUE);
}

/**
 * The message is the standard OPMSG_COMMON_SET_BUFFER_SIZE.
 * The operator accepts it at any time.
 *
 * The value is reported back for source terminals in the next
 * get_buffer_details call, which is part of connecting a terminal.
 * Different sizes can thus be set for each source terminal by
 * calling set_buffer_size before each connection.
 * Terminals in a group should have the same buffer size.
 */
bool src_sync_opmsg_set_buffer_size(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    patch_fn_shared(src_sync);

    SRC_SYNC_OP_DATA *op_extra_data =
            (SRC_SYNC_OP_DATA*)op_data->extra_op_data;
    op_extra_data->buffer_size =
            (int16)OPMSG_FIELD_GET( message_data, OPMSG_COMMON_SET_BUFFER_SIZE,
                                    BUFFER_SIZE);

    if (op_extra_data->buffer_size > 0)
    {
        SOSY_MSG2( SRC_SYNC_TRACE_ALWAYS, "0x%04x set_buffer_size %d samples",
                   op_extra_data->id,
                   op_extra_data->buffer_size);
    }
    else if (op_extra_data->buffer_size < 0)
    {
        /* Qn.10 to Q6.n */
        op_extra_data->buffer_size <<= (DAWTH-16);
        SOSY_MSG2( SRC_SYNC_TRACE_ALWAYS, "0x%04x set_buffer_size 0.%05d x 32 x kick period x fs",
                   op_extra_data->id,
                   src_sync_sec_frac_to_10usec(-op_extra_data->buffer_size));
    }
    else
    {
        SOSY_MSG1( SRC_SYNC_TRACE_ALWAYS, "0x%04x set_buffer_size auto", op_extra_data->id);
    }

    return TRUE;
}

/**
 * The message is the standard OPMSG_COMMON_SET_SAMPLE_RATE.
 * The operator accepts it at any time. The value is used to calculate
 * buffer sizes e.g. while connecting terminals, before routes have been set.
 */
bool src_sync_opmsg_set_sample_rate(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    patch_fn_shared(src_sync);

    SRC_SYNC_OP_DATA *op_extra_data =
            (SRC_SYNC_OP_DATA*)op_data->extra_op_data;

    op_extra_data->default_sample_rate = 25 * ((unsigned*)message_data)[3];
    SOSY_MSG2( SRC_SYNC_TRACE_ALWAYS, "0x%04x set_sample_rate %d Hz",
               op_extra_data->id, op_extra_data->default_sample_rate);

    return(TRUE);
}


bool src_sync_opmsg_ep_get_config(
        OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length,
        OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SRC_SYNC_OP_DATA *op_extra_data = (SRC_SYNC_OP_DATA*)op_data->extra_op_data;
    OPMSG_GET_CONFIG *msg = message_data;
    OPMSG_GET_CONFIG_RESULT *result = msg->result;
    unsigned term_idx = msg->header.cmd_header.client_id;
    bool is_sink = (term_idx & TERMINAL_SINK_MASK) != 0;
    unsigned terminal_num = term_idx & TERMINAL_NUM_MASK;
    uint32 value = 0;

    patch_fn_shared(src_sync);

    if (terminal_num >= SRC_SYNC_CAP_MAX_CHANNELS)
    {
        return FALSE;
    }

    /* msg->value - Pointer which will be populated with the asked configuration value
       msg->cmd_header.client_id - Terminal ID (includes TERMINAL_SINK_MASK for sinks)
       msg->key - Parameter Key to return value for */

    switch(msg->key)
    {
        case OPMSG_OP_TERMINAL_DETAILS:
            /* Value is a boolean indicating whether the terminal emulates
             * a real endpoint. This is used here to tell the framework that
             * the endpoint can rate adjust.
               */
            if(is_sink && src_sync_is_sink_rm_enabled(op_extra_data, terminal_num))
            {
                value = OPMSG_GET_CONFIG_TERMINAL_DETAILS_REAL;
            }
            else if (!is_sink && (op_extra_data->sink_rm_enabled_mask != 0))
            {
                /* Request rate information from the downstream sink on the
                 * first channel of each output group.
                 * While it's not necessary to connect all terminals in an
                 * output group, it should be acceptable to demand that the
                 * first channel is connected (for rate matching to work.)
                 */
                SRC_SYNC_SOURCE_ENTRY* src_ptr = op_extra_data->sources[terminal_num];
                if (src_ptr->common.idx_in_group == 0)
                {
                    value = OPMSG_GET_CONFIG_TERMINAL_DETAILS_RATE_MATCH_AWARE;
                }
                else
                {
                    value = OPMSG_GET_CONFIG_TERMINAL_DETAILS_NONE;
                }
            }
            else
            {
                value = OPMSG_GET_CONFIG_TERMINAL_DETAILS_NONE;
            }

            SOSY_MSG3( SRC_SYNC_TRACE_ALWAYS, "0x%04x ep_get_config 0x%06x "
                       "OPMSG_OP_TERMINAL_DETAILS %d",
                       op_extra_data->id, term_idx, value);
            break;

        case OPMSG_OP_TERMINAL_RATEMATCH_ABILITY:
            /* Value is one of the values of RATEMATCHING_SUPPORT in stream.h */
            if(is_sink && src_sync_is_sink_rm_enabled(op_extra_data, terminal_num))
            {
                SRC_SYNC_SINK_ENTRY* sink_ptr = op_extra_data->sinks[terminal_num];
                if ((sink_ptr != NULL) && (sink_ptr->common.idx_in_group > 0))
                {
                    /* Higher channels report "auto" because for real operator
                     * endpoints there doesn't seem to be a concept like sync
                     * groups. I.e. even though all the terminals would report
                     * the same clock ID, if they report software ratematching
                     * support, each terminal would be asked to enact rate
                     * adjustment separately.
                     */
                    value = RATEMATCHING_SUPPORT_AUTO;
                }
                else
                {
                    value = RATEMATCHING_SUPPORT_SW;
                }
            }
            else if (!is_sink && (op_extra_data->sink_rm_enabled_mask != 0))
            {
                /* Request rate information from the downstream sink on the
                 * first channel of each output group.
                 * While it's not necessary to connect all terminals in an
                 * output group, it should be acceptable to demand that the
                 * first channel is connected (for rate matching to work.)
                 */
                SRC_SYNC_SOURCE_ENTRY* src_ptr = op_extra_data->sources[terminal_num];
                if (src_ptr->common.idx_in_group == 0)
                {
                    value = RATEMATCHING_SUPPORT_MONITOR;
                }
                else
                {
                    value = RATEMATCHING_SUPPORT_NONE;
                }
            }
            else
            {
                value = RATEMATCHING_SUPPORT_NONE;
            }
            SOSY_MSG3( SRC_SYNC_TRACE_ALWAYS, "0x%04x ep_get_config 0x%06x "
                       "OPMSG_OP_TERMINAL_RATEMATCH_ABILITY %d",
                       op_extra_data->id, term_idx, value);
           break;
        case OPMSG_OP_TERMINAL_RATEMATCH_RATE:
            /* All rate adjusting terminals use SRA, so they should
             * report the rate of the non-adjusting endpoints.
             * TODO a way to obtain a reference rate from the framework,
             * if it is not available from timestamp metadata.
             */
            /* Convert from sample period adjustment in Q1.N to
             * sample rate factor in QM.22
             */
            if(is_sink && src_sync_is_sink_rm_enabled(op_extra_data, terminal_num))
            {
                value = (uint32)
                        ((1 << STREAM_RATEMATCHING_FIX_POINT_SHIFT)
                         - ( (op_extra_data->primary_sp_adjust +
                              (1 << (SRC_SYNC_SCALE_SP_ADJUST_TO_RATE_MEASURE-1)))
                             >> SRC_SYNC_SCALE_SP_ADJUST_TO_RATE_MEASURE));
#ifdef SOSY_VERBOSE
                int signed_value = (int)(((int32)value) - (1 << STREAM_RATEMATCHING_FIX_POINT_SHIFT));
                unsigned print_value;
                char print_sign = src_sync_sign_and_magnitude(signed_value, &print_value);
                SOSY_MSG3( SRC_SYNC_TRACE_RATE_MATCH,
                           "rm rate sink_%d 1%c%d *2^-22",
                           terminal_num, print_sign, print_value);
#endif /* SOSY_VERBOSE */
            }
            else
            {
                value = ((uint32)1 << STREAM_RATEMATCHING_FIX_POINT_SHIFT);
#ifdef SOSY_VERBOSE
                if (is_sink)
                {
                    SOSY_MSG1( SRC_SYNC_TRACE_RATE_MATCH,
                               "rm_rate sink_%d rate nominal", terminal_num);
                }
                else
                {
                    SOSY_MSG1( SRC_SYNC_TRACE_RATE_MATCH,
                               "rm_rate src_%d rate nominal", terminal_num);
                }
#endif /* SOSY_VERBOSE */
            }

            break;
        case OPMSG_OP_TERMINAL_RATEMATCH_MEASUREMENT:
            /* TODO */
            if(is_sink && src_sync_is_sink_rm_enabled(op_extra_data, terminal_num))
            {
                result->rm_measurement.sp_deviation = op_extra_data->primary_sp_adjust;
            }
            else
            {
                result->rm_measurement.sp_deviation = 0;
            }
            result->rm_measurement.measurement.valid = FALSE;
            break;

        case OPMSG_OP_TERMINAL_BLOCK_SIZE:
        case OPMSG_OP_TERMINAL_KICK_PERIOD:
            /* Not sure if these are meaningful for an operator endpoint */
            break;
        case OPMSG_OP_TERMINAL_PROC_TIME:
            /* Unknown */
            break;
        default:
            return(FALSE);
    }
    result->value = value;
    return(TRUE);
}

bool src_sync_opmsg_ep_configure(
        OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length,
        OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SRC_SYNC_OP_DATA *op_extra_data = (SRC_SYNC_OP_DATA*)op_data->extra_op_data;
    OPMSG_CONFIGURE  *msg = message_data;
    unsigned term_idx = msg->header.cmd_header.client_id;
    bool is_sink = (term_idx & TERMINAL_SINK_MASK) != 0;
    unsigned terminal_num = term_idx & TERMINAL_NUM_MASK;
    uint32 value = msg->value;

    patch_fn_shared(src_sync);

    /* msg->value - Pointer or Value for Key
       msg->cmd_header.client_id - Terminal ID (includes TERMINAL_SINK_MASK for sinks)
       msg->key - Parameter Key to return value for */

    if (terminal_num >= SRC_SYNC_CAP_MAX_CHANNELS)
    {
        return FALSE;
    }

    switch(msg->key)
    {
        case OPMSG_OP_TERMINAL_DATA_FORMAT:
            /* value is AUDIO_DATA_FORMAT */
            if( ((AUDIO_DATA_FORMAT)value) != AUDIO_DATA_FORMAT_FIXP )
            {
                return(FALSE);
            }
            break;
        case OPMSG_OP_TERMINAL_KICK_PERIOD:
            /* uint32 polling period in usec - ignore */
            break;
        case OPMSG_OP_TERMINAL_PROC_TIME:
            /* uint32 - N/A an operator will never receive this (has_deadline always FALSE for operators)   */
            break;

        case OPMSG_OP_TERMINAL_CBOPS_PARAMETERS:
            /* value is CBOPS_PARAMETERS *
               CBOPS_RATEADJUST (EP_RATEMATCH_ENACTING) */
            break;
        case OPMSG_OP_TERMINAL_BLOCK_SIZE:
            /* expected block size per period */
            break;
        case OPMSG_OP_TERMINAL_RATEMATCH_ENACTING:
            /* boolean. Perform rate matching if TRUE  */
            SOSY_MSG4( SRC_SYNC_TRACE_ALWAYS, "0x%04x ep_configure is_sink %d term %d rm_enact %d",
                       op_extra_data->id, is_sink, terminal_num, value);
            src_sync_rm_enact(op_extra_data, is_sink, terminal_num, (value != 0));
            break;
        case OPMSG_OP_TERMINAL_RATEMATCH_ADJUSTMENT:
            src_sync_rm_adjust(op_extra_data, is_sink, terminal_num, value);
            break;
        case OPMSG_OP_TERMINAL_RATEMATCH_REFERENCE:
            /* TODO */
            /* break; */
        default:
            return(FALSE);
    }

    return(TRUE);
}

bool src_sync_opmsg_ep_clock_id(
        OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length,
        OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SRC_SYNC_OP_DATA *op_extra_data = (SRC_SYNC_OP_DATA*)op_data->extra_op_data;
    unsigned *resp;
    OP_MSG_REQ *msg = message_data;
    unsigned terminal_id = msg->header.cmd_header.client_id;
    bool is_sink = (terminal_id & TERMINAL_SINK_MASK) != 0;
    unsigned terminal_num = terminal_id & TERMINAL_NUM_MASK;

    patch_fn_shared(src_sync);

    if (terminal_num >= SRC_SYNC_CAP_MAX_CHANNELS)
    {
        return FALSE;
    }

    /* Payload is a single word containing the clock ID */
    resp = xpmalloc(sizeof(unsigned));
    if (!resp)
    {
        return FALSE;
    }
    *resp_data = (OP_OPMSG_RSP_PAYLOAD*)resp;
    *resp_length = 1;

    /* Format for the response word:
     * Bits 0..6: operator ID
     * Bits 7...: index within operator
     * Not sure if this format is official; some operators use it.
     */
    *resp = op_data->id;

    if (is_sink && src_sync_is_sink_rm_enabled(op_extra_data, terminal_num))
    {
        SRC_SYNC_SINK_GROUP* sink_grp =
                src_sync_find_sink_group(op_extra_data, terminal_num);

        PL_ASSERT((sink_grp != NULL) && sink_grp->rate_adjust_enable);

        *resp |= ((sink_grp->common.idx + 1) << SRC_SYNC_CLOCK_INDEX_LSB_POSN);
    }

    return TRUE;
}

bool src_sync_get_route(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    OPMSG_SRC_SYNC_GET_ROUTE_MSG     *msg = (OPMSG_SRC_SYNC_GET_ROUTE_MSG*)message_data;
    OPMSG_SRC_SYNC_GET_ROUTE_ENTRY   *route;
    unsigned  osize, route_mask, tmp_mask, srcidx;
    unsigned *resp;
    SRC_SYNC_OP_DATA   *op_extra_data = (SRC_SYNC_OP_DATA*)op_data->extra_op_data;

    patch_fn_shared(src_sync);

    route_mask = (((msg->groups_msw&0xFFFF)<<16) | (msg->groups_lsw&0xFFFF));
    route_mask &= SRC_SYNC_CAP_CHANNELS_MASK;

    /* Count routes */
    osize = pl_one_bit_count(route_mask);

    osize = ((osize*sizeof(OPMSG_SRC_SYNC_GET_ROUTE_ENTRY))>>LOG2_ADDR_PER_WORD) + 4;
    /* allocate and fill response data */
    *resp_length = osize;
    if ((resp = (unsigned*)xpmalloc(osize*sizeof(unsigned)))==0)
    {
        return (FALSE);
    }
    *resp_data = (OP_OPMSG_RSP_PAYLOAD *)resp;

    resp[0] = OPMGR_GET_OPMSG_MSG_ID(msg);             /*message ID*/
    resp[1] = OPMSG_RESULT_STATES_NORMAL_STATE;        /*result field*/
    resp[2] = msg->groups_msw;
    resp[3] = msg->groups_lsw;


    route = (OPMSG_SRC_SYNC_GET_ROUTE_ENTRY*)&resp[4];
    for (srcidx = 0; route_mask; route_mask >>= 1, srcidx++)
    {
        if ((route_mask & 0x1) != 0)
        {
            SRC_SYNC_SOURCE_ENTRY *src = op_extra_data->sources[srcidx];

            tmp_mask = srcidx & SRC_SYNC_ROUTE_TERM_MASK;
            if (src != NULL)
            {
                route->sample_rate = rate_sample_rate_div_25(
                                         src->current_route.sample_rate);
                route->gain        = src->current_route.gain_dB;

                if (src->common.buffer != NULL)
                {
                    tmp_mask |= SRC_SYNC_ROUTE_TERM_CONNECTED;
                }
                /* Report whether route change or transition is in progress */
                if ((src->switch_route.sink != NULL)
                    || (src->inv_transition != 0))
                {
                    tmp_mask |= SRC_SYNC_ROUTE_CHANGE_PENDING;
                }
                tmp_mask <<= SRC_SYNC_ROUTE_SRC_SHIFT;
                if (src->current_route.sink != NULL)
                {
                    tmp_mask |= src->current_route.sink->common.idx
                                & SRC_SYNC_ROUTE_TERM_MASK;
                    if (src->current_route.sink->common.buffer != NULL)
                    {
                        tmp_mask |= SRC_SYNC_ROUTE_TERM_CONNECTED;
                    }
                }
            }
            else
            {
                route->sample_rate = 0;
                route->gain        = 0;
                tmp_mask <<= SRC_SYNC_ROUTE_SRC_SHIFT;
            }
            route->route = tmp_mask;
            route++;
        }
    }

    return TRUE;
}

/*
 * Common code for the set_sink_groups and set_source_groups message
 * handlers to clear all routes. Processing should be suspended
 * when this is called.
 */
void src_sync_clear_all_routes(SRC_SYNC_OP_DATA *op_extra_data)
{
    unsigned i;

    patch_fn_shared(src_sync);

    for (i = 0; i < SRC_SYNC_CAP_MAX_CHANNELS; ++ i)
    {
        SRC_SYNC_SOURCE_ENTRY* src_ptr = op_extra_data->sources[i];

        /* Clearing a route should be done in a way that
         * perform_transition will process it fully, i.e.
         * set fields in switch_route, not directly in current_route.
         */
        if ((src_ptr != NULL) && src_sync_valid_route(&src_ptr->current_route))
        {
            src_ptr->switch_route.sink = src_ptr->current_route.sink;
            src_ptr->switch_route.sample_rate = 0;
            src_ptr->transition_pt = 0;
            src_ptr->inv_transition = 0;
            op_extra_data->src_route_switch_pending_mask |=
                    (1 << src_ptr->common.idx);
        }
    }
}

/*
 * Common code for the set_sink_groups and set_source_groups message
 * handlers to check group masks do not overlap, and to get the
 * union of all set bits
 */
bool src_sync_disjoint_union_group_masks(
        const unsigned* data_ptr, unsigned num_groups, unsigned* p_union_mask)
{
    unsigned i;
    unsigned union_mask = 0;

    for (i = 0; i < num_groups; i++)
    {
        unsigned group_mask;
        group_mask = ((data_ptr[2*i  ] & 0x00FF) << 16) |
                     ((data_ptr[2*i+1] & 0xFFFF)      );

        /* Check for sink in multiple groups */
        if ((group_mask & union_mask) != 0)
        {
            return FALSE;
        }
        union_mask |= group_mask;
    }

    *p_union_mask = union_mask;

    return TRUE;
}

/*
 * Common code for the set_sink_groups and set_source_groups message
 * handlers to populate the group lists from the message
 */
bool src_sync_populate_groups(
        SRC_SYNC_TERMINAL_GROUP* groups,
        const unsigned* data_ptr,
        unsigned num_groups,
        bool (*parse_flags)(SRC_SYNC_TERMINAL_GROUP*,unsigned) )
{
    unsigned i;
    SRC_SYNC_TERMINAL_GROUP* grp_ptr;
    bool success = TRUE;

    for (i = 0,            grp_ptr = groups;
         i < num_groups && grp_ptr != NULL;
         i++,              grp_ptr = grp_ptr->next)
    {
        unsigned x_msw, x_lsw;

        grp_ptr->idx = i;

        x_msw = (*(data_ptr++))&0xFFFF;
        x_lsw = (*(data_ptr++))&0xFFFF;
        grp_ptr->channel_mask = ((x_msw & 0x00FF) << 16) | x_lsw;

#ifdef INSTALL_METADATA
        grp_ptr->metadata_enabled =
            ((x_msw & OPMSG_SOURCE_SYNC_SET_SINK_GROUPS_FLAG_METADATA) != 0);
#endif /* INSTALL_METADATA */

        if (parse_flags != NULL)
        {
            if (! (*parse_flags)(grp_ptr, x_msw))
            {
                success = FALSE;
            }
        }
    }

    return success;
}

bool src_sync_parse_sink_flags(SRC_SYNC_TERMINAL_GROUP* grp_ptr,
                               unsigned x_msw)
{
    SRC_SYNC_SINK_GROUP* src_grp = cast_sink_group(grp_ptr);

    patch_fn_shared(src_sync);

    src_grp->purge =
            (x_msw & OPMSG_SOURCE_SYNC_SET_SINK_GROUPS_FLAG_PURGE) != 0;
    src_grp->rate_adjust_enable =
            (x_msw & OPMSG_SOURCE_SYNC_SET_SINK_GROUPS_FLAG_RATE_ADJUST) != 0;

    return TRUE;
}

bool src_sync_set_sink_groups(
        OPERATOR_DATA *op_data, void *message_data,
        unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SRC_SYNC_OP_DATA   *op_extra_data =
            (SRC_SYNC_OP_DATA*)op_data->extra_op_data;
    OPMSG_SRC_SYNC_SET_SINK_GROUPS_MSG *msg =
            (OPMSG_SRC_SYNC_SET_SINK_GROUPS_MSG*)message_data;
    const unsigned num_groups = msg->num_groups;
    unsigned i;
    unsigned *data_ptr = msg->data;
    unsigned sink_mask;
    unsigned rate_adjust_enabled_mask;
    SRC_SYNC_SINK_GROUP *groups = NULL;
    SRC_SYNC_SINK_GROUP *grp_ptr;
    bool success;

    patch_fn_shared(src_sync);

    if ( (num_groups >= SRC_SYNC_CAP_MAX_CHANNELS)
         || (num_groups == 0) )
    {
        L2_DBG_MSG2( "src_sync 0x%04x set_sink_group REJECTED: "
                     "invalid number of groups: %d",
                     op_extra_data->id, num_groups);
        return FALSE;
    }

    if (op_extra_data->sinks_connected != 0)
    {
        L2_DBG_MSG1( "src_sync 0x%04x set_sink_group REJECTED: "
                     "not allowed while connected",
                     op_extra_data->id);
        return FALSE;
    }

    if (! src_sync_disjoint_union_group_masks(data_ptr, num_groups, &sink_mask))
    {
        L2_DBG_MSG1( "src_sync 0x%04x set_sink_group REJECTED: sink in multiple groups",
                     op_extra_data->id);
        return FALSE;
    }

    /* Allocate and link so that iterating is possible
     * without knowledge of the size of each entry
     */
    groups = xzpnewn(num_groups, SRC_SYNC_SINK_GROUP);
    if (groups == NULL)
    {
        return FALSE;
    }
    for (i = 0; i < num_groups; i++)
    {
        grp_ptr = &groups[i];
        grp_ptr->common.next = &grp_ptr[1].common;
    }
    groups[num_groups-1].common.next = NULL;

    /* Prepare to clean up the just allocated groups array,
     * if there is a failure */
    grp_ptr = groups;

    success = src_sync_populate_groups(&groups->common, data_ptr,
                                       num_groups,
                                       &src_sync_parse_sink_flags );

    if (! success)
    {
        L2_DBG_MSG1( "src_sync 0x%04x set_sink_group REJECTED: invalid flags",
                     op_extra_data->id);
    }
    else
    {
        rate_adjust_enabled_mask = 0;

        for ( grp_ptr = groups;
              grp_ptr != NULL;
              grp_ptr = next_sink_group(grp_ptr))
        {
            if (grp_ptr->common.channel_mask == 0)
            {
                L2_DBG_MSG2( "src_sync 0x%04x set_sink_groups REJECTED sink_g%d empty",
                             op_extra_data->id, grp_ptr->common.idx);
                success = FALSE;
            }
            else
            {
                if (grp_ptr->rate_adjust_enable)
                {
                    rate_adjust_enabled_mask |= grp_ptr->common.channel_mask;
                }
                grp_ptr->common.sample_rate = op_extra_data->default_sample_rate;
                grp_ptr->common.inv_sample_rate =
                        src_sync_sample_rate_to_inv_sample_rate(
                                op_extra_data->default_sample_rate);

#ifdef INSTALL_METADATA
                SOSY_MSG4( SRC_SYNC_TRACE_ALWAYS, "0x%04x set_sink_groups sink_grp%d "
                             "mask 0x%06x md %d",
                             op_extra_data->id, grp_ptr->common.idx,
                             grp_ptr->common.channel_mask,
                             grp_ptr->common.metadata_enabled );
#else /* INSTALL_METADATA */
                SOSY_MSG3( SRC_SYNC_TRACE_ALWAYS, "0x%04x set_sink_groups sink_grp%d "
                           "mask 0x%06x",
                           op_extra_data->id, grp_ptr->common.idx,
                           grp_ptr->common.channel_mask);
#endif /* INSTALL_METADATA */
                SOSY_MSG4( SRC_SYNC_TRACE_ALWAYS, "0x%04x set_sink_groups sink_grp%d pr %d ra %d",
                           op_extra_data->id, grp_ptr->common.idx, grp_ptr->purge,
                           grp_ptr->rate_adjust_enable);
            }
        }

        if (success)
        {
            /* Replace groups */
            src_sync_suspend_processing(op_extra_data);

            /*
             * Allocate sink terminal entries if not already allocated;
             * initialise terminal's group pointer and channel index
             */
            SRC_SYNC_SINK_GROUP* sink_grp;
            for ( sink_grp = groups;
                  sink_grp != NULL;
                  sink_grp = next_sink_group(sink_grp))
            {
                unsigned group_mask = sink_grp->common.channel_mask;
                unsigned channel_idx = 0;

                for (i = 0; i < SRC_SYNC_CAP_MAX_CHANNELS; ++ i)
                {
                    if ((group_mask & (1 << i)) != 0)
                    {
                        SRC_SYNC_SINK_ENTRY* sink_ptr;

                        sink_ptr = src_sync_alloc_sink(op_extra_data, i);
                        if (NULL == sink_ptr)
                        {
                            success = FALSE;
                            break;
                        }
                        sink_ptr->common.group = &sink_grp->common;
                        sink_ptr->common.idx_in_group = channel_idx;
                        channel_idx += 1;
                    }
                }
                if (!success)
                {
                    break;
                }
            }

            if (success && (op_extra_data->source_group_mask != 0))
            {
                /* set_sink_groups and set_source_groups both have to be done.
                 * Don't allocate yet if only one has been done so far.
                 */
                success = src_sync_alloc_buffer_histories(
                        op_extra_data, groups, op_extra_data->source_groups);
            }

            if (success)
            {
                /* Commit and clean up the old groups array */
                grp_ptr = op_extra_data->sink_groups;
                op_extra_data->sink_groups = groups;
                op_extra_data->sink_group_mask = sink_mask;
                op_extra_data->sink_rm_enabled_mask = rate_adjust_enabled_mask;
                /* Clear routes */
                src_sync_clear_all_routes(op_extra_data);
            }

            src_sync_resume_processing(op_extra_data);
        }
    }

    /* Release old groups */
    pfree(grp_ptr);

    /* Make sure we process data */
    src_sync_update_processing(op_data);

    return success;
}

bool src_sync_parse_source_flags(SRC_SYNC_TERMINAL_GROUP* grp_ptr,
                                 unsigned x_msw)
{
    patch_fn_shared(src_sync);

#ifdef INSTALL_METADATA
    SRC_SYNC_SOURCE_GROUP* src_grp = cast_source_group(grp_ptr);

    src_grp->metadata_dest.provide_ttp =
            (x_msw & OPMSG_SOURCE_SYNC_SET_SOURCE_GROUPS_FLAG_PROVIDE_TTP) != 0;

    if (src_grp->metadata_dest.provide_ttp
        && ! src_grp->common.metadata_enabled)
    {
        L2_DBG_MSG("src_sync: WARNING source group flag provide_ttp"
                    " ignored without metadata enabled");
        src_grp->metadata_dest.provide_ttp = FALSE;
    }
#endif /* INSTALL_METADATA */
    return TRUE;
}

bool src_sync_set_source_groups(
        OPERATOR_DATA *op_data, void *message_data,
        unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SRC_SYNC_OP_DATA   *op_extra_data =
            (SRC_SYNC_OP_DATA*)op_data->extra_op_data;
    OPMSG_SRC_SYNC_SET_SOURCE_GROUPS_MSG *msg =
            (OPMSG_SRC_SYNC_SET_SOURCE_GROUPS_MSG*)message_data;
    const unsigned num_groups = msg->num_groups;
    unsigned i;
    unsigned *data_ptr = msg->data;
    unsigned source_mask;
    SRC_SYNC_SOURCE_GROUP *groups = NULL;
    SRC_SYNC_SOURCE_GROUP *grp_ptr;
    bool success = TRUE;

    patch_fn_shared(src_sync);

    if ( (num_groups >= SRC_SYNC_CAP_MAX_CHANNELS)
         || (num_groups == 0) )
    {
        L2_DBG_MSG2( "src_sync 0x%04x set_source_group REJECTED: "
                     "invalid number of groups: %d",
                     op_extra_data->id, num_groups);
        return FALSE;
    }

    if (op_extra_data->sources_connected != 0)
    {
        L2_DBG_MSG1( "src_sync 0x%04x set_source_group REJECTED: "
                     "not allowed while connected",
                     op_extra_data->id);
        return FALSE;
    }

    if (! src_sync_disjoint_union_group_masks(data_ptr, num_groups,
                                              &source_mask))
    {
        L2_DBG_MSG1("src_sync 0x%04x set_source_group REJECTED: overlapping groups",
                    op_extra_data->id);
        return FALSE;
    }

    /* Allocate and link so that iterating is possible
     * without knowledge of the size of each entry
     */
    groups = xzpnewn(msg->num_groups, SRC_SYNC_SOURCE_GROUP);
    if (groups == NULL)
    {
        return FALSE;
    }
    for (i = 0; i < num_groups; i++)
    {
        grp_ptr = &groups[i];
        grp_ptr->common.next = &grp_ptr[1].common;
    }
    groups[msg->num_groups-1].common.next = NULL;

    /* Prepare to clean up the just allocated groups array,
     * if there is a failure */
    grp_ptr = groups;

    success = src_sync_populate_groups( &groups->common, data_ptr, num_groups,
                                        &src_sync_parse_source_flags);
    if (! success)
    {
        L2_DBG_MSG1("src_sync 0x%04x set_source_groups REJECTED: flags error",
                    op_extra_data->id);
    }
    else
    {
        SRC_SYNC_SOURCE_GROUP* src_grp;

        for ( src_grp = groups;
              src_grp != NULL;
              src_grp = next_source_group(src_grp) )
        {
            if (src_grp->common.channel_mask == 0)
            {
                L2_DBG_MSG2("src_sync 0x%04x set_source_groups REJECTED: src_g%d empty",
                            op_extra_data->id, src_grp->common.idx);
                success = FALSE;
            }
            else
            {
                grp_ptr->common.sample_rate = op_extra_data->default_sample_rate;
                grp_ptr->common.inv_sample_rate =
                        src_sync_sample_rate_to_inv_sample_rate(
                                op_extra_data->default_sample_rate);
                SOSY_MSG3( SRC_SYNC_TRACE_ALWAYS,
                           "0x%04x set_source_groups: src_g%d mask 0x%06x",
                           op_extra_data->id, src_grp->common.idx,
                           src_grp->common.channel_mask);
#if defined(INSTALL_METADATA)
                SOSY_MSG4( SRC_SYNC_TRACE_ALWAYS,
                           "0x%04x set_source_groups: src_g%d md %d ttp %d",
                            op_extra_data->id, src_grp->common.idx,
                            src_grp->common.metadata_enabled,
                            src_grp->metadata_dest.provide_ttp);
#endif /* INSTALL_METADATA */
            }
        }

        if (success)
        {
            /* Replace groups */
            src_sync_suspend_processing(op_extra_data);

            /*
             * Allocate source terminal entries if not already allocated;
             * initialise terminal's group pointer and channel index
             */
            for ( src_grp = groups;
                  src_grp != NULL;
                  src_grp = next_source_group(src_grp))
            {
                unsigned group_mask = src_grp->common.channel_mask;
                unsigned channel_idx = 0;

                for (i = 0; i < SRC_SYNC_CAP_MAX_CHANNELS; ++ i)
                {
                    if ((group_mask & (1 << i)) != 0)
                    {
                        SRC_SYNC_SOURCE_ENTRY* src_ptr;

                        src_ptr = src_sync_alloc_source(op_extra_data, i);
                        if (NULL == src_ptr)
                        {
                            success = FALSE;
                            break;
                        }
                        src_ptr->common.group = &src_grp->common;
                        src_ptr->common.idx_in_group = channel_idx;
                        channel_idx += 1;
                    }
                }
                if (!success)
                {
                    break;
                }
            }

            if (success && (op_extra_data->sink_group_mask != 0))
            {
                /* set_sink_groups and set_source_groups both have to be done.
                 * Don't allocate yet if only one has been done so far.
                 */
                success = src_sync_alloc_buffer_histories(
                        op_extra_data, op_extra_data->sink_groups, groups);
            }

            if (success)
            {
                /* Commit and clean up the old groups array */
                grp_ptr = op_extra_data->source_groups;
                op_extra_data->source_groups = groups;
                op_extra_data->source_group_mask = source_mask;
                /* Clear routes */
                src_sync_clear_all_routes(op_extra_data);
            }

            src_sync_resume_processing(op_extra_data);
        }
    }

    /* Release old or new groups */
    pfree(grp_ptr);

    /* Make sure we process data */
    src_sync_update_processing(op_data);

    return success;
}

void src_sync_clear_group_sample_rate(SRC_SYNC_TERMINAL_GROUP* groups)
{
    SRC_SYNC_TERMINAL_GROUP* grp_ptr;

    for ( grp_ptr = groups; grp_ptr != NULL; grp_ptr = grp_ptr->next )
    {
        grp_ptr->tmp_sample_rate = 0;
    }
}

bool src_sync_mismatched_group_sample_rate(
        SRC_SYNC_TERMINAL_GROUP* groups, unsigned channel_idx,
        unsigned sample_rate)
{
    SRC_SYNC_TERMINAL_GROUP* group = src_sync_find_group(groups, channel_idx);

    if (group != NULL)
    {
        if (group->tmp_sample_rate == 0)
        {
            group->tmp_sample_rate = sample_rate;
        }
        else
        {
            return (group->tmp_sample_rate != sample_rate);
        }
    }
    return FALSE;
}

bool src_sync_set_route(OPERATOR_DATA *op_data, void *message_data,
                        unsigned *resp_length,
                        OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SRC_SYNC_OP_DATA *op_extra_data =
            (SRC_SYNC_OP_DATA*)op_data->extra_op_data;
    OPMSG_SRC_SYNC_SET_ROUTE_MSG *msg =
            (OPMSG_SRC_SYNC_SET_ROUTE_MSG*)message_data;
    SRC_SYNC_ROUTE_MSG_ENTRY *route;
    unsigned routed_sources;
    uint8 routed_sink_grps[SRC_SYNC_CAP_MAX_CHANNELS];
    unsigned i;

    patch_fn_shared(src_sync);

    /* Check for valid input */

    if (msg->num_routes >= SRC_SYNC_CAP_MAX_CHANNELS)
    {
        L2_DBG_MSG2("src_sync 0x%04x set_route REJECTED: invalid number of routes %d",
                    op_extra_data->id, msg->num_routes);
        return FALSE;
    }

    src_sync_clear_group_sample_rate(&(op_extra_data->source_groups->common));
    src_sync_clear_group_sample_rate(&(op_extra_data->sink_groups->common));

    routed_sources = 0;
    for (i = 0; i < SRC_SYNC_CAP_MAX_CHANNELS; ++ i)
    {
        routed_sink_grps[i] = SRC_SYNC_CAP_MAX_CHANNELS;
    }

    for ( i = 0, route = msg->routes;
          i < msg->num_routes;
          i++, route++ )
    {
        unsigned sample_rate = route->sample_rate;
        unsigned src_bit;

        /* Check valid terminal numbers */
        if ((route->src_idx>=SRC_SYNC_CAP_MAX_CHANNELS) ||
            (route->snk_idx>=SRC_SYNC_CAP_MAX_CHANNELS))
        {
            L2_DBG_MSG2("src_sync 0x%04x set_route REJECTED: "
                        "invalid terminal number %d",
                        op_extra_data->id,
                        MAX(route->src_idx, route->snk_idx));
            return FALSE;
        }

        /* Check that terminals are in groups */
        if ((op_extra_data->source_group_mask & (1<<route->src_idx)) == 0)
        {
            L2_DBG_MSG2("src_sync 0x%04x set_route REJECTED: "
                        "source %d not in any group",
                        op_extra_data->id, route->src_idx);
            return FALSE;
        }
        if ((op_extra_data->sink_group_mask & (1<<route->snk_idx)) == 0)
        {
            L2_DBG_MSG2("src_sync 0x%04x set_route REJECTED: sink %d not in any group",
                        op_extra_data->id, route->snk_idx);
            return FALSE;
        }

        /* Safety, after allocation failure in set_..._groups */
        if (op_extra_data->sources[route->src_idx] == NULL)
        {
            return FALSE;
        }
        if (op_extra_data->sinks[route->snk_idx] == NULL)
        {
            return FALSE;
        }

        /* Reject duplicate source numbers.
         * Do not check current routes for this:
         * those will be overwritten.
         */
        src_bit = 1 << route->src_idx;
        if ((routed_sources & src_bit) != 0)
        {
            L2_DBG_MSG2("src_sync 0x%04x set_route REJECTED: "
                        "duplicate source terminal number %d",
                        op_extra_data->id, route->src_idx);
            return FALSE;
        }
        routed_sources |= src_bit;

        /* Check that sample rates are the same within each group. */
        if (sample_rate != 0)
        {
            if ( src_sync_mismatched_group_sample_rate(
                     &(op_extra_data->source_groups->common),
                     route->src_idx,
                     sample_rate)
                 || src_sync_mismatched_group_sample_rate(
                     &(op_extra_data->sink_groups->common),
                     route->snk_idx,
                     sample_rate) )
            {
                L2_DBG_MSG1("src_sync 0x%04x set_route REJECTED: "
                            "different sample rates in group",
                            op_extra_data->id);
                return FALSE;
            }
        }
    }

    /* Check that after applying the changes,
     * channels from each sink group go to the
     * same source group.
     */
    /* Record routes which will not be replaced */
    for ( i = 0; i < SRC_SYNC_CAP_MAX_CHANNELS; ++ i )
    {
        if ((routed_sources & (1 << i)) == 0)
        {
            SRC_SYNC_SOURCE_ENTRY *src_data = op_extra_data->sources[i];
            if ( (src_data != NULL)
                 && (src_data->current_route.sample_rate != 0) )
            {
                SRC_SYNC_SINK_ENTRY* sink_ptr = src_data->current_route.sink;
                if (sink_ptr != NULL)
                {
                    PL_ASSERT(src_data->common.group != NULL);
                    PL_ASSERT(sink_ptr->common.group != NULL);
                    unsigned src_grp_idx = src_data->common.group->idx;
                    unsigned sink_grp_idx = sink_ptr->common.group->idx;

                    if (routed_sink_grps[sink_grp_idx] == SRC_SYNC_CAP_MAX_CHANNELS)
                    {
                        routed_sink_grps[sink_grp_idx] = src_grp_idx;
                    }
                    else
                    {
                        PL_ASSERT(routed_sink_grps[sink_grp_idx] == src_grp_idx);
                    }
                }
            }
        }
    }
    /* Check new routes */
    for ( i = 0, route = msg->routes;
          i < msg->num_routes;
          i++, route++ )
    {
        if (route->sample_rate != 0)
        {
            unsigned src_grp_idx, sink_grp_idx;

            /* Check that all routes from a sink group
             * go to the same source group
             */
            src_grp_idx = op_extra_data->sources[route->src_idx]
                                   ->common.group->idx;
            sink_grp_idx = op_extra_data->sinks[route->snk_idx]
                                    ->common.group->idx;

            if ( routed_sink_grps[sink_grp_idx]
                 == SRC_SYNC_CAP_MAX_CHANNELS )
            {
                routed_sink_grps[sink_grp_idx] = src_grp_idx;
            }
            else if (routed_sink_grps[sink_grp_idx] != src_grp_idx)
            {
                L2_DBG_MSG5("src_sync 0x%04x set_route REJECTED: route%d splits "
                            "sink group g%d to src groups g%d, g%d",
                            op_extra_data->id, i, sink_grp_idx,
                            routed_sink_grps[sink_grp_idx], src_grp_idx);
                return FALSE;
            }
        }
    }

    src_sync_suspend_processing(op_extra_data);

    for ( i = 0, route = msg->routes;
          i < msg->num_routes;
          i++, route++ )
    {
        SRC_SYNC_SOURCE_ENTRY   *src_data;
        SRC_SYNC_SINK_ENTRY     *sink_data;
        unsigned transition;
        unsigned sample_rate;
        unsigned inv_sample_rate = 0;

        /* Get Sink */
        sink_data = op_extra_data->sinks[route->snk_idx];
        PL_ASSERT(sink_data != NULL); /* checked above */

        /* Get Source */
        src_data = op_extra_data->sources[route->src_idx];
        PL_ASSERT(src_data != NULL);

        /* Signal route change to OBPM */
        op_extra_data->Dirty_flag |= (1<<route->src_idx);

        transition = route->transition & 0xFFFF;
        sample_rate = 25 * route->sample_rate;
        if (sample_rate != 0)
        {
            inv_sample_rate = src_sync_sample_rate_to_inv_sample_rate(sample_rate);
        }

        SOSY_MSG4( SRC_SYNC_TRACE_ALWAYS, "set_route %d -> %d fs %d g %d/60dB",
                   route->snk_idx, route->src_idx,
                   sample_rate, (int16)route->gain);

        /* Save Route info */
        /* route->gain is a 16-bit signed integer in a wider unsigned variable.
         * Force a sign extension. */
        int16 gain_dB = route->gain;
        src_data->switch_route.gain_dB          = gain_dB;
        src_data->switch_route.gain_lin         = dB60toLinearQ5(gain_dB);
        src_data->switch_route.sample_rate      = sample_rate;
        src_data->switch_route.inv_sample_rate  = inv_sample_rate;

        /* Setup sink to transition to */
        src_data->switch_route.sink = sink_data;
        op_extra_data->src_route_switch_pending_mask |= (1 << src_data->common.idx);
        if ( (src_data->current_route.sink == NULL)
             || (transition == 0)
             || (src_data->current_route.sink == sink_data) )
        {
            /* Immediately switch route */
            src_data->inv_transition = 0;
            src_data->transition_pt  = 0;
        }
        else
        {
            /* Transition route to new sink */
            if(src_data->inv_transition==0)
            {
                src_data->transition_pt  = FRACTIONAL(1.0);
            }
            src_data->inv_transition =
                    -(int)pl_fractional_divide(1, transition);
        }
    }

    src_sync_resume_processing(op_extra_data);

    /* Make sure we process data */
    src_sync_update_processing(op_data);

    return TRUE;
}

void src_sync_trace_params(const SRC_SYNC_OP_DATA* op_extra_data)
{
    SOSY_MSG1( SRC_SYNC_TRACE_ALWAYS, "SS_PERIOD                       = 0.%05d",
        src_sync_sec_frac_to_10usec (src_sync_get_period(op_extra_data)));
    SOSY_MSG1( SRC_SYNC_TRACE_ALWAYS, "SS_MAX_PERIOD                   = 0.%05d",
        src_sync_sec_frac_to_10usec (src_sync_get_max_period(op_extra_data)));
    SOSY_MSG1( SRC_SYNC_TRACE_ALWAYS, "SS_MAX_LATENCY                  = 0.%05d",
        src_sync_sec_frac_to_10usec (src_sync_get_max_latency(op_extra_data)));
    SOSY_MSG1( SRC_SYNC_TRACE_ALWAYS, "SS_STALL_RECOVERY_DEFAULT_FILL  = 0.%05d",
        src_sync_sec_frac_to_10usec (src_sync_get_stall_recovery_default_fill(op_extra_data)));
    SOSY_MSG1( SRC_SYNC_TRACE_ALWAYS, "SS_STALL_RECOVERY_CATCHUP_LIMIT = 0.%05d",
        src_sync_sec_frac_to_10usec (op_extra_data->cur_params.OFFSET_SS_STALL_RECOVERY_CATCHUP_LIMIT));
}

bool src_sync_set_trace_enable(
        OPERATOR_DATA *op_data, void *message_data,
        unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    patch_fn_shared(src_sync);

#ifdef SOSY_VERBOSE
    SRC_SYNC_OP_DATA *op_extra_data = (SRC_SYNC_OP_DATA*)op_data->extra_op_data;

    /* 32-bit MSW first */
    op_extra_data->trace_enable =
            (((unsigned*)message_data)[3] << 16) |
            (((unsigned*)message_data)[4]);
#endif /* SOSY_VERBOSE */

    return TRUE;
}

