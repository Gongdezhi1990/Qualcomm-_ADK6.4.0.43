/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  stream_operator.c
 * \ingroup stream
 *
 * stream operator type file. <br>
 * This file contains stream functions for operator
 * endpoints. <br>
 *
 * \section sec1 Contains:
 * stream_operator_get_endpoint_from_key <br>
 * stream_operator_get_endpoint <br>
 * stream_create_operator_endpoint <br>
 * stream_destroy_operator_endpoint <br>
 */

/****************************************************************************
Include Files
*/

#include "stream_private.h"
#include "opmgr/opmgr_endpoint_override.h"
#include "opmgr/opmgr_for_stream.h"

/****************************************************************************
Private Type Declarations
*/
/** Function prototype for endpoint enable/disable function */
typedef void (* ep_en_disable_fnc) (ENDPOINT *);

/****************************************************************************
Private Constant Declarations
*/

/****************************************************************************
Private Macro Declarations
*/

/****************************************************************************
Private Variable Definitions
*/

/****************************************************************************
Private Function Declarations
*/
static void enable_op_endpoints_core(ENDPOINT *eplist, unsigned opid, ep_en_disable_fnc en_disable_fnc);
static unsigned operator_create_stream_key(unsigned int opid, unsigned int idx,
                                         ENDPOINT_DIRECTION dir);
static bool operator_connect(ENDPOINT *endpoint, tCbuffer *Cbuffer_ptr, ENDPOINT *ep_to_kick, bool* start_on_connect);
static bool operator_disconnect (ENDPOINT *endpoint);
static bool operator_buffer_details (ENDPOINT *ep, BUFFER_DETAILS *details);
static bool operator_configure (ENDPOINT *endpoint, unsigned int key, uint32 value);
static bool operator_get_config (ENDPOINT *endpoint, unsigned int key, ENDPOINT_GET_CONFIG_RESULT* result);
static void operator_get_timing (ENDPOINT *endpoint, ENDPOINT_TIMING_INFORMATION *time_info);
static void operator_kick(ENDPOINT *ep, ENDPOINT_KICK_DIRECTION kick_dir);
static bool operator_start(ENDPOINT *ep, KICK_OBJECT *ko);
static bool operator_stop(ENDPOINT *ep );
static bool operator_sync_sids(ENDPOINT *ep1, ENDPOINT *ep2);
static AUDIO_DATA_FORMAT operator_get_data_format (ENDPOINT *endpoint);
static unsigned transfom_endpoint_key_to_operator_key(unsigned int key);
static bool destroy_op_ep_internal(ENDPOINT *ep);

DEFINE_ENDPOINT_FUNCTIONS (operator_functions, stream_close_dummy, operator_connect,
                           operator_disconnect, operator_buffer_details,
                           operator_kick, stream_sched_kick_dummy,
                           operator_start, operator_stop,
                           operator_configure, operator_get_config,
                           operator_get_timing, operator_sync_sids);

/****************************************************************************
Public Function Definitions
*/


ENDPOINT *stream_operator_get_endpoint_from_key(unsigned key)
{
    /* Derive the direction from the key which in the case of operator
     * endpoints is the same as the opidep.
     */
    ENDPOINT_DIRECTION dir = (key & STREAM_EP_SINK_BIT) ? SINK : SOURCE;

    return stream_get_endpoint_from_key_and_functions(key, dir,
                                                  &endpoint_operator_functions);
}

/****************************************************************************
 *
 * stream_operator_get_endpoint
 *
 */
ENDPOINT *stream_operator_get_endpoint(unsigned int opid, unsigned int idx,
                                       ENDPOINT_DIRECTION dir, unsigned int con_id)
{
    ENDPOINT *ep = NULL;

    unsigned key = operator_create_stream_key( opid, idx, dir );
    if (key != 0)
    {
        ep = stream_operator_get_endpoint_from_key( key );
    }

    return ep;
}

/****************************************************************************
 *
 * stream_create_operator_endpoint
 *
 */
ENDPOINT *stream_create_operator_endpoint( unsigned opidep, unsigned con_id)
{
    ENDPOINT *ep;
    uint32 op_endpoint_details = (uint32)OPMSG_GET_CONFIG_TERMINAL_DETAILS_NONE;
    ENDPOINT_DIRECTION dir = (opidep & STREAM_EP_SINK_BIT) ? SINK : SOURCE;
    
    patch_fn_shared(stream_operator);
    
    /* The endpoint should only be created if there is a valid underlying
     * operator terminal. */
    if (!opmgr_is_opidep_valid(opidep))
    {
        return NULL;
    }

    if ((ep = STREAM_NEW_ENDPOINT(operator, opidep, dir, con_id)) == NULL)
    {
        return NULL;
    }

    PL_PRINT_P2(TR_PL_TEST_TRACE, "Created endpoint: %d for ep id: %d\n", (unsigned)(uintptr_t) ep, opidep);
    /* ask for buffer details */
    OPMSG_GET_CONFIG_RESULT get_config_result;
    get_config_result.value = OPMSG_GET_CONFIG_TERMINAL_DETAILS_NONE;
    opmgr_get_config_msg_to_operator(opidep, OPMSG_OP_TERMINAL_DETAILS, &get_config_result);
    op_endpoint_details = get_config_result.value;

    ep->can_be_closed = FALSE;
    ep->is_real = (OPMSG_GET_CONFIG_TERMINAL_DETAILS_REAL == op_endpoint_details);
    ep->is_rate_match_aware = (OPMSG_GET_CONFIG_TERMINAL_DETAILS_RATE_MATCH_AWARE == op_endpoint_details);
    
    /* operator endpoints can be destroyed on disconnect */
    ep->destroy_on_disconnect = TRUE;

    ep->state.operator.op_bg_task = opmgr_get_op_task_from_epid(opidep);
    if (NULL == ep->state.operator.op_bg_task)
    {
        panic_diatribe(PANIC_AUDIO_OPERATOR_HAS_NO_TASK, opidep);
    }

    return ep;
}

/****************************************************************************
 *
 * stream_enable_operator_endpoint
 *
 */
void stream_enable_operator_endpoints(unsigned opid)
{
    enable_op_endpoints_core(sink_endpoint_list, opid, stream_enable_endpoint);
    enable_op_endpoints_core(source_endpoint_list, opid, stream_enable_endpoint);

}

/****************************************************************************
 *
 * stream_disable_operator_endpoint
 *
 */
void stream_disable_operator_endpoints( unsigned opid )
{
    enable_op_endpoints_core(sink_endpoint_list, opid, stream_disable_endpoint);
    enable_op_endpoints_core(source_endpoint_list, opid, stream_disable_endpoint);
}

/****************************************************************************
 *
 * stream_destroy_operator_endpoint
 *
 */
bool stream_destroy_operator_endpoint(unsigned opidep)
{
    ENDPOINT *ep;

    if ((ep = stream_endpoint_from_extern_id(opidep)) != NULL)
    {
        return destroy_op_ep_internal(ep);
    }
    /* An operator may not create all of its endpoints.
     * Return TRUE if the endpoint was not created. */
    return TRUE;
}

/****************************************************************************
 *
 * stream_destroy_all_operators_endpoints
 *
 */
bool stream_destroy_all_operators_endpoints(unsigned opid, unsigned num_sinks, unsigned num_sources)
{
    patch_fn_shared(stream_operator);
    
    if (num_sinks > 0)
    {
        ENDPOINT *ep = sink_endpoint_list;
        unsigned fnd_cnt = 0;

        while (ep != NULL && fnd_cnt < num_sinks)
        {
            if ((ep->stream_endpoint_type == endpoint_operator) &&
                    ((ep->key & (STREAM_EP_OPID_MASK | STREAM_EP_OP_BIT)) == opid))
            {
                fnd_cnt++;
                if (!destroy_op_ep_internal(ep))
                {
                    return FALSE;
                }
            }
            ep = ep->next;
        }
    }
    if (num_sources > 0)
        {
            ENDPOINT *ep = source_endpoint_list;
            unsigned fnd_cnt = 0;

            while (ep != NULL && fnd_cnt < num_sources)
            {
                if ((ep->stream_endpoint_type == endpoint_operator) &&
                        ((ep->key & (STREAM_EP_OPID_MASK | STREAM_EP_OP_BIT)) == opid))
                {
                    fnd_cnt++;
                    if (!destroy_op_ep_internal(ep))
                    {
                        return FALSE;
                    }
                }
                ep = ep->next;
            }
        }
    /* An operator may not create all of its endpoints.
     * Return TRUE if the endpoint was not created. */
    return TRUE;
}

/****************************************************************************
 *
 * stream_get_connected_endpoint_from_terminal_id
 *
 */
ENDPOINT *stream_get_connected_endpoint_from_terminal_id(unsigned opid,  unsigned terminal_id)
{
    ENDPOINT_DIRECTION dir = ((terminal_id & TERMINAL_SINK_MASK) != 0)? SINK : SOURCE;
    unsigned terminal_number = terminal_id & TERMINAL_NUM_MASK;

    /* create key for the operator endpoint */
    unsigned key = operator_create_stream_key(INT_TO_EXT_OPID(opid), terminal_number, dir);

    /* get the terminal endpoint */
    ENDPOINT *ep = stream_operator_get_endpoint_from_key(key);
    if(ep != NULL)
    {
        /* get the connected endpoint */
        ep = ep -> connected_to;
    }
    return ep;
}

/****************************************************************************
Private Function Definitions
*/

/**
 * \brief Internal helper function for enabling/disabling all endpoints belonging
 * to a particular operator.
 *
 * \param eplist The list of endpoints to search for the operator's endpoints
 * \param opid The external ID of the operator
 * \param en_disable_fnc Function to call to enable/disable the endpoint
 */
static void enable_op_endpoints_core(ENDPOINT *eplist, unsigned opid, ep_en_disable_fnc en_disable_fnc)
{
    while (eplist != NULL)
    {
        if ((eplist->id & (STREAM_EP_OPID_MASK | STREAM_EP_OP_BIT)) == opid)
        {
            en_disable_fnc(eplist);
        }
       eplist = eplist->next;
    }
}

/**
 * \brief Internal helper function for destroying an operator endpoint
 *
 * \param ep The operator endpoint to destroy
 *
 * \return Whether the endpoint was successfully destroyed or not.
 */
static bool destroy_op_ep_internal(ENDPOINT *ep)
{
    /* overwrite the closeable flag */
    ep->can_be_closed = TRUE;
    /* Make sure we don't try to destroy it twice */
    ep->destroy_on_disconnect = FALSE;

    return stream_close_endpoint(ep);
}

/**
 * \brief Connect to the endpoint.
 *
 * \param *endpoint pointer to the endpoint that is being connected
 * \param *Cbuffer_ptr pointer to the Cbuffer struct for the buffer that is being connected.
 * \param *ep_to_kick pointer to the endpoint which will be kicked after a successful
 *              run. Note: this can be different from the connected to endpoint when
 *              in-place running is enabled.
 * \param *start_on_connect return flag which indicates if the endpoint wants be started
 *              on connect. Note: The endpoint will only be started if the connected
 *              to endpoint wants to be started too.
 *
 * \return success or failure
 */
static bool operator_connect(ENDPOINT *endpoint, tCbuffer *Cbuffer_ptr, ENDPOINT *ep_to_kick, bool* start_on_connect)
{
   /* Populate the ep_to_kick field which is used when the operator explicitly kicks
    * one of its terminal.
    * Explicit kicks are kicks through stream_if_propagate_kick or ompgr_kick_from_operator functions.
    *   The ompgr_kick_from_operator is the preferred approach as it is much more efficient
    * Implicit kicks are kicks with the TOUCHED_TERMINALS *touched field in process
    * data function of the operator.*/
    endpoint->ep_to_kick = ep_to_kick;
    *start_on_connect = FALSE;
    return opmgr_connect_buffer_to_endpoint (
               stream_external_id_from_endpoint(endpoint), Cbuffer_ptr, stream_external_id_from_endpoint(ep_to_kick));
}

/*
 * \brief Disconnects from an endpoint and stops the data from flowing
 *
 * \param *endpoint pointer to the endpoint that is being disconnected
 *
 */
static bool operator_disconnect(ENDPOINT *endpoint)
{
    return opmgr_disconnect_buffer_from_endpoint(
                            stream_external_id_from_endpoint(endpoint));
}

/*
 * \brief Obtains details of the buffer required for this connection
 *
 * \param *endpoint pointer to the endpoint from which the buffer
 *         information is required
 *
 */
static bool operator_buffer_details (ENDPOINT *ep, BUFFER_DETAILS *details)
{
    return opmgr_get_buffer_details(ep->id, details);
}


/*
 * \brief configure an operator endpoint with a key and value pair
 *
 * \param *endpoint pointer to the endpoint to be configured
 * \param key denoting what is being configured
 * \param value pointer to a value to which the key is to be configured
 *
 * \return Whether the request succeeded.
 *
 */
static bool operator_configure(ENDPOINT *endpoint, unsigned int key, uint32 value)
{
    switch(key)
    {
    case EP_DATA_FORMAT:
        /* N.B. The operator data format cannot currently be set. So if it
         * doesn't match fail. */
        if (!endpoint->is_real)
        {
            return (operator_get_data_format(endpoint) == (AUDIO_DATA_FORMAT)value);
        }
        else
        {
            if (opmgr_config_msg_to_operator(stream_external_id_from_endpoint(endpoint), \
                    transfom_endpoint_key_to_operator_key(key), value))
            {
                return TRUE;
            }
        }
        return FALSE;

    case EP_OVERRIDE_ENDPOINT:
        /* This feature is not supported for the operator endpoint */
        return FALSE;

    case EP_CBOPS_PARAMETERS:
    {
        bool retval = FALSE;

        /* Send message to the operator. */
        /* For EP_CBOPS_PARAMETERS, value is a pointer */
        if (opmgr_config_msg_to_operator(stream_external_id_from_endpoint(endpoint), \
                transfom_endpoint_key_to_operator_key(key), value))
        {
            retval = TRUE;
        }
        free_cbops_parameters((CBOPS_PARAMETERS *)(uintptr_t) value);
        return retval;
    }


    case EP_KICK_PERIOD:
    case EP_BLOCK_SIZE:
    case EP_PROC_TIME:
        if (endpoint->is_real)
        {
            L4_DBG_MSG3("Real endpoint scheduling %06X, key = %04X, value = %04X", endpoint, key, value);
            return opmgr_config_msg_to_operator(stream_external_id_from_endpoint(endpoint),
                    transfom_endpoint_key_to_operator_key(key), value);
        }
        else
        {
            L4_DBG_MSG("endpoint is not real");
            return FALSE;
        }

    case EP_RATEMATCH_ENACTING:
    case EP_RATEMATCH_ADJUSTMENT:
    case EP_RATEMATCH_REFERENCE:
        if (endpoint->is_real || endpoint->is_rate_match_aware)
        {
            L4_DBG_MSG3("Real endpoint enacting %06X, key = %04X, value = %04X", endpoint, key, value);
            /* For EP_RATEMATCH_REFERENCE, value is a pointer */
            return opmgr_config_msg_to_operator(stream_external_id_from_endpoint(endpoint),
                    transfom_endpoint_key_to_operator_key(key), value);
        }
        else
        {
            L4_DBG_MSG("endpoint is not real or rate match aware");
            return FALSE;
        }

    case EP_KICKED_FROM_ENDPOINT:
        /* Operator endpoint kicked by hard deadline
         * tell the operator whether to ignore incoming kicks from other
         * operators in the chain. */
        if((bool)value)
        {
            opmgr_stop_kicks(get_opid_from_opidep(endpoint->id),
                    (STOP_KICK)((endpoint->direction == SINK)?SOURCE_SIDE:SINK_SIDE));
        }
        return TRUE;
    default:
        return TRUE;
    }
}


/*
 * \brief get operator endpoint configuration
 *
 * \param *endpoint pointer to the endpoint to be configured
 * \param key denoting what is being configured
 * \param result pointer to a value which is populated with the current value
 *
 * \return Whether the request succeeded.
 *
 */
static bool operator_get_config(ENDPOINT *endpoint, unsigned int key, ENDPOINT_GET_CONFIG_RESULT *result)
{
    switch(key)
    {
    case EP_DATA_FORMAT:
        result->u.value = (uint32)operator_get_data_format(endpoint);
        return TRUE;

    case EP_KICK_PERIOD:
    case EP_BLOCK_SIZE:
    case EP_PROC_TIME:
        if (! endpoint->is_real)
        {
            return FALSE;
        }
        break;

    case EP_RATEMATCH_ABILITY:
    case EP_RATEMATCH_RATE:
    case EP_RATEMATCH_MEASUREMENT:
        if (! endpoint->is_real && !endpoint->is_rate_match_aware)
        {
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    OPMSG_GET_CONFIG_RESULT msg_result;
    msg_result.value = 0;
    bool success = opmgr_get_config_msg_to_operator(
            stream_external_id_from_endpoint(endpoint),
            transfom_endpoint_key_to_operator_key(key), &msg_result);

    switch (key)
    {
    case EP_RATEMATCH_MEASUREMENT:
        result->u.rm_meas.sp_deviation = msg_result.rm_measurement.sp_deviation;
        result->u.rm_meas.measurement = msg_result.rm_measurement.measurement;
        break;
    default:
        result->u.value = msg_result.value;
        break;
    }
    return success;
}


/*
 * \brief generates an operator key (same as an operator endpoint id) from an
 *        operator id, index and direction
 *
 * \param opid the operator id (NB must be the PUBLIC opid)
 * \param idx the source or sink channel index
 * \param dir the direction, either \c SOURCE or \SINK
 *
 * \return generated operator key, or zero if opid or idx are invalid
 */
static unsigned operator_create_stream_key(unsigned int opid, unsigned int idx,
                                         ENDPOINT_DIRECTION dir)
{
    unsigned type;

    /* Validate opid */
    if ((opid & STREAM_EP_TYPE_MASK) != STREAM_EP_OP_BIT ||
        (opid & STREAM_EP_CHAN_MASK) != 0)
    {
        /* opid is invalid */
        PL_PRINT_P0(TR_STREAM, "operator_create_stream_key: opid is invalid\n");
        return 0;
    }

    /* Validate idx */
    if ((idx & ~STREAM_EP_CHAN_MASK) != 0)
    {
        /* idx is invalid */
        PL_PRINT_P0(TR_STREAM, "operator_create_stream_key: idx is invalid\n");
        return 0;
    }

    /* Derive operator type bits from direction */
    type = (dir == SOURCE) ? STREAM_EP_OP_SOURCE : STREAM_EP_OP_SINK;

    /* Generate the operator key */
    return (type | opid | idx);
}


static void operator_get_timing (ENDPOINT *endpoint, ENDPOINT_TIMING_INFORMATION *time_info)
{
    bool is_locally_clocked;
    bool wants_kicks;
    
    opmgr_get_sched_info(endpoint->id,  &(time_info->block_size), &(time_info->period),
            &is_locally_clocked, &wants_kicks);

    /* Locally clocked can only be not true for real operator endpoints
     * or those that play a role in rate matching
     */
    if (endpoint->is_real || endpoint->is_rate_match_aware)
    {
        time_info->locally_clocked = is_locally_clocked;
    }
    else
    {
        time_info->locally_clocked = TRUE;
    }
    
    time_info->has_deadline = FALSE;
    time_info->wants_kicks = wants_kicks;

    return;
}

static void operator_kick(ENDPOINT *ep, ENDPOINT_KICK_DIRECTION kick_dir)
{
    /* Kick the operator task. */
    raise_bg_int_with_bgint(ep->state.operator.op_bg_task);
}

static bool operator_start(ENDPOINT *ep, KICK_OBJECT *ko)
{
    /* This function can be called when an operator have a real endpoint. The start
     * operator command is used to start an operator. Because if this reason ignore
     * this call.  */
    return TRUE;
}

static bool operator_stop(ENDPOINT *ep)
{
    /* This function can be called when an operator have a real endpoint. The stop
     * operator command is used to stop an operator. Because if this reason ignore
     * this call.  */
    return TRUE;
}

static bool operator_sync_sids(ENDPOINT *ep1, ENDPOINT *ep2)
{
    /* If theses are both operators then we can synchronise them. */
    if (ep1->stream_endpoint_type == endpoint_operator &&
            ep2->stream_endpoint_type == endpoint_operator)
    {
        /* At the moment we are only supporting endpoints that are in the same
         * direction. ie. both Sources or both sinks (Previn allows this in
         * some cases.)
         */
        if (ep1->direction == ep2->direction)
        {
            return TRUE;
        }
    }
    return FALSE;
}

static AUDIO_DATA_FORMAT operator_get_data_format (ENDPOINT *endpoint)
{
    /* Return the data format required by the operator endpoint */
    return opmgr_get_data_format(stream_external_id_from_endpoint(endpoint));
}

/**
 * \brief Transform stream specific key to operator specific key
 *
 * \param  key  - endpoint key which will be translated to operator key
 *
 * \return  Operator key.
 */
static unsigned transfom_endpoint_key_to_operator_key(unsigned int key)
{
    patch_fn_shared(stream_operator);

    switch(key)
    {
        case EP_CBOPS_PARAMETERS:
            return OPMSG_OP_TERMINAL_CBOPS_PARAMETERS;
        case EP_DATA_FORMAT:
            return OPMSG_OP_TERMINAL_DATA_FORMAT;
        case EP_KICK_PERIOD:
            return OPMSG_OP_TERMINAL_KICK_PERIOD;
        case EP_BLOCK_SIZE:
            return OPMSG_OP_TERMINAL_BLOCK_SIZE;
        case EP_PROC_TIME:
            return OPMSG_OP_TERMINAL_PROC_TIME;
        case EP_RATEMATCH_ABILITY:
            return OPMSG_OP_TERMINAL_RATEMATCH_ABILITY;
        case EP_RATEMATCH_RATE:
            return OPMSG_OP_TERMINAL_RATEMATCH_RATE;
        case EP_RATEMATCH_ENACTING:
            return OPMSG_OP_TERMINAL_RATEMATCH_ENACTING;
        case EP_RATEMATCH_ADJUSTMENT:
            return OPMSG_OP_TERMINAL_RATEMATCH_ADJUSTMENT;
        case EP_RATEMATCH_MEASUREMENT:
            return OPMSG_OP_TERMINAL_RATEMATCH_MEASUREMENT;
        case EP_RATEMATCH_REFERENCE:
            return OPMSG_OP_TERMINAL_RATEMATCH_REFERENCE;
        default:
            panic_diatribe(PANIC_AUDIO_INVALID_KEY_OR_ENDPOINT_ID, key);
#ifdef DESKTOP_TEST_BUILD
            return 0;
#endif
    }
}

/*
 * stream_operator_get_clock_id
 */
unsigned stream_operator_get_clock_id(ENDPOINT *ep)
{
    if (!ep->is_real)
    {
        /* If the operator endpoint isn't real it's locally clocked and has the
         * same rate as any other operator endpoint that isn't real. */
        return 0;
    }

    /* It's real so request this from the operator */
    return opmgr_get_operator_ep_clock_id(stream_external_id_from_endpoint(ep));
}
