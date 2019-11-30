/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  stream.c
 * \ingroup stream
 *
 * stream main file. <br>
 * This file contains generic stream functions. <br>
 *
 * \section sec1 Contains:
 * stream_endpoint_from_extern_id <br>
 * stream_external_id_from_endpoint <br>
 * stream_direction_from_endpoint <br>
 * stream_new_endpoint <br>
 * stream_endpoint_from_key_and_functions <br>
 * stream_destroy_endpoint <br>
 * stream_close_endpoint <br>
 */



/****************************************************************************
Include Files
*/

#include "stream_private.h"

/****************************************************************************
Private Type Declarations
*/

/****************************************************************************
Private Constant Declarations
*/

/****************************************************************************
Private Macro Declarations
*/


/****************************************************************************
Private Variable Definitions
*/

/*
 * This contains the next ids for the endpoints and the transforms.
 * NOTE for multicore case: Only P0 Stream assigns new EP IDs, therefore
 * this is not to be used by secondary processors' Stream. If same image used
 * on both cores, this gets compiled in but must not be used outside P0.
 * If separate images, this gets compiled out on secondary processor(s).
 */
stream_next_ids stream_next_id;

#ifdef STREAM_CONNECT_FAULT_CODE
/*
 * Set when a stream_connect command fails for any reason
 */
STREAM_CONNECT_FAULT stream_connect_fault = SC_OK;
#endif /* STREAM_CONNECT_FAULT_CODE */

/*
 *  These are the heads of the source and sink endpoint lists.
 *  In multicore case, there are no "remote" endpoints - each Stream entity
 *  manages its own 'local' EPs on the core it runs on. Below list heads
 *  are placed in each processor's private DRAM.
 */
ENDPOINT *source_endpoint_list;
ENDPOINT *sink_endpoint_list;

/****************************************************************************
Functions
*/

ENDPOINT *endpoint_from_id(unsigned id);

/****************************************************************************
Private Function Declarations
*/

static unsigned next_endpoint_id(ENDPOINT_DIRECTION direction);
static void stream_deferred_kick_handler(void** bg_int_data);

/****************************************************************************
Public Function Definitions
*/

#ifdef INSTALL_DUAL_CORE_SUPPORT
/****************************************************************************
 *
 * stream_get_next_endpoint_id
 *
 */
/* In multicore case, only P0 shall assign new endpoint IDs */
unsigned stream_get_next_endpoint_id(ENDPOINT_DIRECTION direction)
{
    return next_endpoint_id(direction);
}
#endif /* INSTALL_DUAL_CORE_SUPPORT */

/****************************************************************************
 *
 * stream_first_endpoint
 *
 */
ENDPOINT *stream_first_endpoint(ENDPOINT_DIRECTION dir)
{
    return (dir == SOURCE) ? source_endpoint_list : sink_endpoint_list;
}

/****************************************************************************
 *
 * stream_endpoint_from_extern_id
 *
 */
ENDPOINT *stream_endpoint_from_extern_id(unsigned int id)
{
    /* Convert the external id into an internal id */
    TOGGLE_EP_ID_BETWEEN_INT_AND_EXT(id);

    return endpoint_from_id(id);
}

/****************************************************************************
 *
 * stream_external_id_from_endpoint
 *
 */
unsigned stream_external_id_from_endpoint(ENDPOINT *endpoint)
{
    unsigned id = 0;

    if (endpoint != NULL)
    {
        id = endpoint->id;

        /* Convert the internal id into an external id */
        TOGGLE_EP_ID_BETWEEN_INT_AND_EXT(id);
    }

    return id;
}

/****************************************************************************
 *
 * stream_direction_from_endpoint
 *
 */
ENDPOINT_DIRECTION stream_direction_from_endpoint(ENDPOINT *endpoint)
{
    return endpoint ? endpoint->direction : SOURCE;
}

/****************************************************************************
 *
 * stream_get_connected_ep_from_id
 *
 */
ENDPOINT *stream_get_connected_ep_from_id(unsigned ep_id)
{
    ENDPOINT *ep = stream_endpoint_from_extern_id(ep_id);

    if (ep && ep->connected_to)
    {
        return ep->connected_to;
    }
    return NULL;
}

/****************************************************************************
 *
 * stream_ep_id_from_ep
 *
 */
unsigned stream_ep_id_from_ep(ENDPOINT *ep)
{
    return ep->id;
}

/****************************************************************************
 *
 * stream_is_endpoint_real
 *
 */
bool stream_is_endpoint_real(ENDPOINT *endpoint)
{
    return endpoint->is_real;
}

/****************************************************************************
 *
 * stream_is_connected_to_same_entity
 *
 */
bool stream_is_connected_to_same_entity(ENDPOINT *ep1, ENDPOINT *ep2)
{
    if (ep1->connected_to && ep2->connected_to)
    {
        /* If both are connected to the same operator then the opid part of their
         * key is the same.
         */
        if (endpoint_operator == ep1->connected_to->stream_endpoint_type &&
                endpoint_operator == ep2->connected_to->stream_endpoint_type)
        {
            if ((ep1->connected_to->key & STREAM_EP_OPID_MASK) ==
                    (ep2->connected_to->key & STREAM_EP_OPID_MASK))
            {
                return TRUE;
            }
        }
        /* If both are connected to audio endpoints then they are connected to the same
         * entity if the audio endpoints are synchronised.
         */
        else if (endpoint_audio == ep1->connected_to->stream_endpoint_type &&
                    endpoint_audio == ep2->connected_to->stream_endpoint_type)
        {
            if (ep1->connected_to->state.audio.head_of_sync ==
                    ep2->connected_to->state.audio.head_of_sync)
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

/****************************************************************************
 *
 * stream_get_endpoint_device
 *
 */
bool stream_get_endpoint_device(ENDPOINT *endpoint, unsigned* device)
{
    patch_fn_shared(stream);
    if((endpoint == NULL) || (device == NULL))
    {
        return FALSE;
    }

    /* We have type and device info. However, former is internal and an overall type,
     * then latter is stored in key bit fields in various ways, may contain HCI handle, operator
     * endpoint direction & type flags etc. So here we collapse & map all this into the device codes
     * exposed via public/external interface(s).
     */

    switch(endpoint->stream_endpoint_type)
    {
        case endpoint_audio:
        case endpoint_spdif:
        {
            *device = GET_DEVICE_FROM_AUDIO_EP_KEY(endpoint->key);
            break;
        }

        case endpoint_sco:
        {
            *device = STREAM_DEVICE_SCO;
            break;
        }

        case endpoint_operator:
        {
            /* Here we don't return direction and other such info, the overall type is "operator" */
            *device = STREAM_DEVICE_OPERATOR;
            break;
        }

        case endpoint_a2dp:
        {
            /* Kymera assumed L2CAP implies A2DP; see comment in
             * stream/stream_if.c. Reverse that here. */
            *device = STREAM_DEVICE_L2CAP;
            break;
        }

        case endpoint_shunt:
        {
            *device = STREAM_DEVICE_SHUNT;
            break;
        }
        case endpoint_file:
        {
            *device = STREAM_DEVICE_FILE;
            break;
        }

        default:
        {
            /* For all other cases, something very fishy so device is unknown / invalid */
            *device = 0;
            break;
        }
    }

    return TRUE;
}

/****************************************************************************
 *
 * stream_get_connecnted_to_endpoint_config
 *
 */
bool stream_get_connected_to_endpoint_config(ENDPOINT *endpoint, unsigned key, ENDPOINT_GET_CONFIG_RESULT* result)
{
    if (!endpoint || !endpoint->connected_to || !endpoint->connected_to->functions->get_config)
    {
        return FALSE;
    }
    return endpoint->connected_to->functions->get_config(endpoint->connected_to, key, result);
}

/****************************************************************************
 *
 * stream_configure_connected_to_endpoint
 *
 */
bool stream_configure_connected_to_endpoint(ENDPOINT *endpoint, unsigned key, uint32 value)
{
    if (!endpoint || !endpoint->connected_to || !endpoint->connected_to->functions->configure)
    {
        return FALSE;
    }
    return endpoint->connected_to->functions->configure(endpoint->connected_to, key, value);
}

/****************************************************************************
 *
 * stream_connected_to_endpoint_is_locally_clocked
 *
 */
bool stream_connected_to_endpoint_is_locally_clocked(ENDPOINT *endpoint)
{
    ENDPOINT_TIMING_INFORMATION tinfo_ep;

    if (!endpoint || !endpoint->connected_to)
    {
        return FALSE;
    }

    endpoint->connected_to->functions->get_timing_info(endpoint->connected_to, &tinfo_ep);
    return tinfo_ep.locally_clocked;
}

/****************************************************************************
 *
 * stream_connected_to_endpoints_have_same_clock_source
 *
 */
bool stream_connected_to_endpoints_have_same_clock_source(ENDPOINT *ep1, ENDPOINT *ep2)
{
    if (!ep1 || !ep2 || !ep1->connected_to || !ep2->connected_to)
    {
        return FALSE;
    }
    return stream_rm_endpoints_have_same_clock_source(ep1->connected_to, ep2->connected_to);
}

/****************************************************************************
 *
 * stream_new_endpoint
 *
 */
ENDPOINT *stream_new_endpoint(const ENDPOINT_FUNCTIONS *functions,
                              unsigned int key,
                              unsigned int state_size,
                              ENDPOINT_DIRECTION dir,
                              ENDPOINT_TYPE ep_type,
                              unsigned int con_id)
{
    ENDPOINT *ep;
    patch_fn(stream_new_endpoint);

    ep = xzpmalloc(sizeof(ENDPOINT) - sizeof(union ENDPOINT_STATE)
                             + state_size);
    /* If we're out of memory fail here. */
    if (NULL == ep)
    {
        return ep;
    }

#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
    if(ep_type == endpoint_shadow)
    {
        /* For operators or shadow EPs, the key and the endpoint id are equivalent. If key is zero, we
         * need to invent an EP ID here. Otherwise it means that P0 has imposed EP ID via the key
         */
        ep->id = (key != 0) ? key : next_endpoint_id(dir);
    }
    else
#endif /* INSTALL_DUAL_CORE_SUPPORT || AUDIO_SECOND_CORE */
    if (ep_type == endpoint_operator)

    {
        /* For operators or shadow EPs, the key and the endpoint id are equivalent */
        ep->id = key;
    }
    else
    {
        /* Get the next available endpoint id */
        ep->id = next_endpoint_id(dir);
    }

    ep->functions = functions;
    ep->key = key;
    ep->direction = dir;
    ep->stream_endpoint_type = ep_type;
    ep->connected_to = NULL;
    ep->con_id = con_id;
    ep->destroy_on_disconnect = FALSE;

    if (dir == SOURCE)
    {
        ep->next = source_endpoint_list;
        source_endpoint_list = ep;
    }
    else
    {
        ep->next = sink_endpoint_list;
        sink_endpoint_list = ep;
    }

    return ep;
}

/****************************************************************************
 *
 * stream_endpoint_from_key_and_functions
 *
 */
ENDPOINT *stream_get_endpoint_from_key_and_functions(unsigned int key,
                                          ENDPOINT_DIRECTION dir,
                                          const ENDPOINT_FUNCTIONS *functions)
{
    ENDPOINT *ep;

    if (dir == SOURCE)
    {
        ep = source_endpoint_list;
    }
    else
    {
        ep = sink_endpoint_list;
    }

    while ((ep != NULL) && ((ep->functions != functions) || (ep->key != key)))
    {
        ep = ep->next;
    }

    return ep;
}

/****************************************************************************
 *
 * stream_destroy_endpoint
 *
 */
bool stream_destroy_endpoint(ENDPOINT *endpoint)
{
    ENDPOINT *ep, **ep_p;

    patch_fn_shared(stream);
    /* There should probably be some error checking here */
    if (stream_direction_from_endpoint(endpoint) == SOURCE)
    {
        ep_p = &source_endpoint_list;
    }
    else
    {
        ep_p = &sink_endpoint_list;
    }

    /* Iterate through the selected endpoint list */
    while ((ep = *ep_p) != NULL)
    {
        /* Is this the one we're looking for? */
        if (ep == endpoint)
        {
            /* Free associated resources */
            stream_destroy_deferred_kick(ep);

            /* Remove entry from list and free it */
            *ep_p = ep->next;
            pfree(ep);

            return TRUE;
        }
        ep_p = &ep->next;
    }

    panic(PANIC_AUDIO_INVALID_ENDPOINT);
#ifdef DESKTOP_TEST_BUILD
    return FALSE;
#endif
}

/****************************************************************************
 *
 * stream_close_endpoint
 *
 */
bool stream_close_endpoint(ENDPOINT *endpoint)
{
    patch_fn_shared(stream);
    if (!endpoint || !(endpoint->can_be_closed))
    {
        return FALSE;
    }

    if( !stream_disconnect_endpoint_transform(endpoint))
    {
        return FALSE;
    }

    if (!endpoint->functions->close(endpoint))
    {
        return FALSE;
    }

    if (endpoint->stream_endpoint_type != endpoint_operator)
    {
       /* signal a real endpoint has been closed */
       set_system_event(SYS_EVENT_REAL_EP_DESTROY);
    }
    return stream_destroy_endpoint(endpoint);
}

/****************************************************************************
 *
 * stream_close_endpoints_with_con_id
 *
 */
void stream_close_endpoints_with_con_id(unsigned con_id)
{
    ENDPOINT *ep;
    ENDPOINT *tmp;
    unsigned int idx;
    patch_fn_shared(stream);

    /* Below idx is used to make us go round the loop
       twice. The first time ep is set to the souce
       list, and the second time it is set to the sink
       list. The body could have been moved to a seperate
       function however the win there would be small */
    for (idx = 0, ep = source_endpoint_list;
         idx < 2;
         idx ++, ep = sink_endpoint_list)
    {
        while(ep != NULL)
        {
            if (ep->con_id == con_id)
            {
                /* remember to take a copy of the endpoint
                   and advance before calling the close as
                   the close will free the memory we are using
                 */
                tmp = ep;
                ep = ep->next;

                stream_close_endpoint (tmp);
            }
            else
            {
                ep = ep->next;
            }
        }
    }
}


/**
 * stream_disable_kicks_from_endpoint
 */
void stream_disable_kicks_from_endpoint(ENDPOINT *ep)
{
    if (ep)
    {
        ep->ep_to_kick = NULL;
    }
}

/**
 * \brief Gets the endpoint with the specified id
 *
 * \param id id of endpoint to search for
 *
 * \returns the endpoint with the specified id, or NULL if not found
 *
 */
ENDPOINT *endpoint_from_id(unsigned id)
{
    /* Get the head of the appropriate list to search */
    ENDPOINT *ep = (id & STREAM_EP_SINK_BIT) ? sink_endpoint_list :
                                               source_endpoint_list;
    /* Search for an endpoint with a matching id */
    while (ep != NULL && ep->id != id)
    {
       ep = ep->next;
    }

    return ep;
}

#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
/* Find shadow EP from data channel ID and direction. TODO MULTICORE: an optimisation would be if
 * shadow EPs are always kept separately, so less to search, but this then has other overheads.
 */
ENDPOINT* stream_shadow_ep_from_data_channel(uint16 data_chan_id)
{
    /* Get the head of the appropriate list to search. A write channel means it belongs to sink shadow EP and viceversa. */
    ENDPOINT *ep;
    patch_fn_shared(stream);

    ep = (ipc_get_data_channelid_dir(data_chan_id) == IPC_DATA_CHANNEL_WRITE) ? sink_endpoint_list : source_endpoint_list ;
    /* Search for a shadow endpoint with a matching channel id */
    while ((ep != NULL) && (!STREAM_EP_IS_SHADOW(ep) || (ep->state.shadow.channel_id != data_chan_id)))
    {
       ep = ep->next;
    }

    return ep;
}

/**
 * stream_kick_kip_eps
 *
 * kick kip endpoint and other kip endpoints in the same sync group
 */
void stream_kick_kip_eps( uint16 data_chan_id, 
                          ENDPOINT_KICK_DIRECTION kick_dir)
{
    ENDPOINT* sync_ep = NULL; 
    ENDPOINT* ep = stream_shadow_ep_from_data_channel(data_chan_id);
    uint16 port_id = ipc_get_data_channelid_port( data_chan_id);

    patch_fn_shared(stream);
    if(ep != NULL)
    {
        ep->functions->kick(ep, kick_dir);
    }

    if(kick_dir == STREAM_KICK_BACKWARDS )
    {
        /* don't kick the group in case of remote bakward kicks */
        return; 
    }

    sync_ep = ( ep->direction == SINK )? sink_endpoint_list:   
                                         source_endpoint_list;

    while(sync_ep != NULL)  
    {
        uint16 pid;

        if( (sync_ep != ep) && STREAM_EP_IS_SHADOW(sync_ep) )
        {
            pid= ipc_get_data_channelid_port( sync_ep->state.shadow.channel_id);
            if(pid == port_id)
            {
                sync_ep->functions->kick(sync_ep, kick_dir);
            }
        }

       sync_ep = sync_ep->next;
    }

}

#endif /* INSTALL_DUAL_CORE_SUPPORT || AUDIO_SECOND_CORE */


/****************************************************************************
 *
 * stream_kick_dummy
 *
 */
void stream_kick_dummy(ENDPOINT *ep, bool valid, ENDPOINT_KICK_DIRECTION kick_dir)
{
    ENDPOINT *next_ep;
    ENDPOINT_KICK_DIRECTION kick_direction;

    next_ep = ep->connected_to;

    /* The next_ep can legitimately be NULL. */
    if (next_ep != NULL)
    {
        if (ep->direction == SOURCE)
        {
            kick_direction = STREAM_KICK_FORWARDS;
        }
        else
        {
            kick_direction = STREAM_KICK_BACKWARDS;
        }

        /* pass along the kick and signal that the buffer data is valid */
        next_ep->functions->kick(next_ep, kick_direction);
    }
    return;
}

/****************************************************************************
 *
 * stream_sched_kick_dummy
 *
 */
void stream_sched_kick_dummy(ENDPOINT *ep, KICK_OBJECT *ko)
{
    return;
}

/****************************************************************************
 *
 * stream_close_dummy
 *
 */
bool stream_close_dummy(ENDPOINT *ep)
{
    return TRUE;
}

/****************************************************************************
 *
 * stream_sync_sids_dummy
 *
 */
bool stream_sync_sids_dummy(ENDPOINT *ep1, ENDPOINT *ep2)
{
    /* if the endpoints do not support syncing then tell the caller */
    return FALSE;
}

#ifdef CHIP_BASE_BC7

/****************************************************************************
 * stream_get_portid_from_endpoint
 */
unsigned int stream_get_portid_from_endpoint(ENDPOINT* ep)
{
    /* If we were given unsupported endpoint type, so we must signal a fail.
     * It can not be 0, as that is a valid port number. Let's adopt convention of
     * returning an invalid port ID that, if not tested by caller, later will be
     * caught by other things using this port ID.
     */
    unsigned int portid = 0xFFFFFFu;

    patch_fn_shared(stream);
    /* This function or even the whole philosophy may later change
     * (e.g. may even vanish if some things become
     * common to all state info structures etc.). For now solves the immediate need
     * to get to ports for sundry endpoints..
     */
    if(ep != NULL)
    {
        switch(ep->stream_endpoint_type)
        {
            case endpoint_audio:
            {
                portid = ep->state.audio.portid;
                break;
            }
#ifdef INSTALL_SCO
            case endpoint_sco:
            {
                portid = ep->state.sco.portid;
                break;
            }
#endif /* INSTALL_SCO */
#ifdef INSTALL_FILE
            case endpoint_file:
            {
                portid = ep->state.file.portid;
                break;
            }
#endif /* INSTALL_FILE */
#ifdef INSTALL_SIMULATION_FILE
            case endpoint_sim_file:
            {
            	portid = ep->state.sim_file.portid;
            	break;
            }
#endif
#ifdef INSTALL_A2DP
            case endpoint_a2dp:
            {
                portid = ep->state.a2dp.portid;
                break;
            }
#endif /* INSTALL_A2DP */
#ifdef INSTALL_SHUNT
            case endpoint_shunt:
            {
                portid = ep->state.shunt.portid;
                break;
            }
#endif /* INSTALL_SHUNT */
#ifdef INSTALL_SPDIF
            case endpoint_spdif:
            {
                portid = ep->state.spdif.portid;
                break;
            }
#endif /* #ifdef INSTALL_SPDIF */

            /* Expand with other endpoint types here */
            /* . . .  */

            /* For unsupported case, invalid portid will be returned */
            default: break;
        }
    }

    return portid;
}


/****************************************************************************
 * stream_get_endpointid_from_portid
 */
unsigned int stream_get_endpointid_from_portid(unsigned portid,
                                                    ENDPOINT_DIRECTION dir)
{
    ENDPOINT *ep;

    /* read ports belong to sources, write ports to sinks. Only look for "real" endpoints. */
    ep = (dir == SINK) ? sink_endpoint_list : source_endpoint_list;

    while((ep != NULL) && ((!ep->is_real) || (stream_get_portid_from_endpoint(ep) != portid)))
    {
        ep = ep->next;
    }

    /* return endpoint ID - if EP not found, then zero is returned (assuming it behaves like the Hydra implementation) */
    return stream_external_id_from_endpoint(ep);
}


#endif /* CHIP_BASE_BC7 */

/****************************************************************************
 * stream_get_endpointid_from_op_terminal
 */
unsigned stream_get_endpointid_from_op_terminal(unsigned opid, unsigned terminal)
{
    unsigned ep_flags;
    if (terminal & STREAM_EP_SINK_BIT)
    {
        ep_flags = STREAM_EP_OP_SINK;
    }
    else
    {
        ep_flags = STREAM_EP_OP_SOURCE;
    }
    return (opid | terminal | ep_flags);
}

/****************************************************************************
 * stream_get_endpoint_connected_to
 */
ENDPOINT *stream_get_endpoint_connected_to(ENDPOINT *ep)
{
    if (ep != NULL)
    {
        return ep->connected_to;
    }
    return NULL;
}

/****************************************************************************
 * stream_endpoint_is_real
 */
bool stream_endpoint_is_real(ENDPOINT *ep)
{
    if (ep != NULL)
    {
        return ep->is_real;
    }
    return FALSE;
}


/****************************************************************************
 * stream_get_new_endpoint_key
 */
unsigned stream_get_new_endpoint_key(ENDPOINT_DIRECTION dir, ENDPOINT_TYPE type)
{
    unsigned key;

    /* Keep checking keys until we break out of the loop */
    for (key = 0; ; key++)
    {
        /* Get the first endpoint in the list */
        ENDPOINT *ep = stream_first_endpoint(dir);

        /* Check each endpoint */
        while (ep != NULL)
        {
            /* Is this a shunt endpoint with the key we're checking for? */
            if ((ep->stream_endpoint_type == type) && (ep->key == key))
            {
                /* Yes, the key is already in use. Try the next one */
                break;
            }
            ep = ep->next;
        }

        /* The current key is unused and available - break out of for loop */
        if (ep == NULL)
        {
            break;
        }
    }

    return key;
}

/****************************************************************************
 *
 * stream_does_ep_exist
 *
 */
bool stream_does_ep_exist(ENDPOINT* ep)
{
    ENDPOINT* temp;

    if(ep != NULL)
    {
        for(temp = sink_endpoint_list; temp != NULL; temp = temp->next)
        {
            if (ep == temp)
            {
                return TRUE;
            }
        }

        for(temp = source_endpoint_list; temp != NULL; temp = temp->next)
        {
            if (ep == temp)
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

/****************************************************************************
 *
 * stream_get_head_of_sync
 *
 */
ENDPOINT *stream_get_head_of_sync(ENDPOINT* ep)
{
    if(ep->stream_endpoint_type == endpoint_audio)
    {
        return ep->state.audio.head_of_sync;
    }
#if defined(INSTALL_FILE) && defined(A7DA_KAS)
    else if(ep->stream_endpoint_type == endpoint_file)
    {
        return ep->state.file.head_of_sync;
    }
#endif /* INSTALL_FILE && A7DA_KAS*/
    else
    {
        // no sync, return itself
        return ep;
    }
}

/****************************************************************************
 *
 * stream_get_nep_in_sync
 *
 */
ENDPOINT *stream_get_nep_in_sync(ENDPOINT* ep)
{
    if(ep->stream_endpoint_type == endpoint_audio)
    {
        return ep->state.audio.nep_in_sync;
    }
#if defined(INSTALL_FILE) && defined(A7DA_KAS)
    else if(ep->stream_endpoint_type == endpoint_file)
    {
        return ep->state.file.nep_in_sync;
    }
#endif /* INSTALL_FILE && A7DA_KAS*/
    else
    {
        // no sync, return NULL
        return NULL;
    }
}

#ifdef ENDPOINT_BUFFER_DEBUG
/****************************************************************************
 *
 * stream_debug_buffer_levels
 *
 */
void stream_debug_buffer_levels ( ENDPOINT *endpoint)
{
    tCbuffer * in_cbuf, * out_cbuf;
    TIME curr_time;
    curr_time = hal_get_time() & 0xFFFFFF;

    unsigned int in_lvl, out_lvl;
    switch(endpoint->stream_endpoint_type)
    {
        case endpoint_audio:
            in_cbuf = endpoint->state.audio.source_buf;
            out_cbuf = endpoint->state.audio.sink_buf;
            break;
#ifdef INSTALL_A2DP
        case endpoint_a2dp:
            in_cbuf = endpoint->state.a2dp.source_buf;
            out_cbuf = endpoint->state.a2dp.sink_buf;
            break;
#endif /* INSTALL_A2DP */
#ifdef INSTALL_SHUNT
        case endpoint_shunt:
            in_cbuf = endpoint->state.shunt.source_buf;
            out_cbuf = endpoint->state.shunt.sink_buf;
            break;
#endif /* INSTALL_SHUNT */
#ifdef INSTALL_SPDIF
        case endpoint_spdif:
            in_cbuf = endpoint->state.spdif.source_buf;
            out_cbuf = endpoint->state.spdif.sink_buf;
            break;
#endif /* INSTALL_SPDIF */
#ifdef INSTALL_FILE
        case endpoint_file:
            in_cbuf = endpoint->state.file.source_buf;
            out_cbuf = endpoint->state.file.sink_buf;
            break;
#endif /* INSTALL_FILE */

#ifdef INSTALL_AUDIO_DATA_SERVICE_TESTER
        case endpoint_tester:
            in_cbuf = endpoint->state.tester.source_data_buf;
            out_cbuf = endpoint->state.tester.sink_data_buf;
            break;
#endif /* INSTALL_AUDIO_DATA_SERVICE_TESTER */

        default:
            /* No valid endpoint type has been returned. Do nothing just return*/
            return;
    }
    /* If 1 endpoint is a sink*/
    if(endpoint->direction)
    {
        /* This means in buffer is a c_buffer out buffer is a write port*/
        in_lvl = cbuffer_calc_amount_data_in_words(in_cbuf) & 0xFFF;
#if defined(CHIP_BASE_BC7)
        /* Need a specific method to read BC write port level. Due to the nature of buffers
         * in hydra a simple calc amount of data can be done for any remote buffers.*/
        out_lvl = cbuffer_write_port_level(out_cbuf) & 0xFFF;
#elif defined(CHIP_BASE_HYDRA) || defined(CHIP_BASE_A7DA_KAS)
        out_lvl = cbuffer_calc_amount_data_in_words(out_cbuf) & 0xFFF;
#else
        /* Not currently supported for other architectures.  */
        #error "Buffer level monitoring not implemented for this chip"
#endif  //CHIP_BASE_BC7
    }
    else /* Endpoint is a source */
    {
        /* This means out buffer is a c_buffer   and in buffer is a read port*/
        in_lvl = cbuffer_calc_amount_data_in_words(in_cbuf);
        out_lvl = cbuffer_calc_amount_data_in_words(out_cbuf);
    }
    L0_DBG_MSG3("buffer level log: endpoint_id:16 %04x time:24 %06x in:12 out:12 %06x",
                endpoint->id, (unsigned)(curr_time+1), out_lvl | (in_lvl << 12));
}
#endif /* ENDPOINT_BUFFER_DEBUG */

/*
 * Handle deferred kick
 */
static void stream_deferred_kick_handler(void** bg_int_data)
{
    ENDPOINT* ep = *bg_int_data;

    if (ep->deferred.kick_is_deferred)
    {
        ep->functions->kick(ep, ep->deferred.kick_dir);
    }
}

/*
 * Enable deferred kick
 */
bool stream_set_deferred_kick(ENDPOINT* ep, bool deferred)
{
    if (ep->deferred.kick_is_deferred != deferred)
    {
        if (deferred && (ep->deferred.bg_task == NO_TASK
                         || ! ID_IS_BG_INT_ID(ep->deferred.bg_task)))
        {
            if (!create_uncoupled_bgint(HIGHEST_PRIORITY, ep,
                                        stream_deferred_kick_handler,
                                        &ep->deferred.bg_task))
            {
                return FALSE;
            }
        }
        ep->deferred.kick_is_deferred = deferred;
    }
    return TRUE;
}

/*
 * Destroy deferred kick bg_int
 */
void stream_destroy_deferred_kick(ENDPOINT* ep)
{
    if (ep->deferred.bg_task != NO_TASK && ID_IS_BG_INT_ID(ep->deferred.bg_task))
    {
        delete_task(ep->deferred.bg_task);
        ep->deferred.bg_task = NO_TASK;
    }
}

/****************************************************************************
Private Function Definitions
*/



/**
 * \brief Obtain the next available id for an endpoint
 *
 * \param direction the direction of the endpoint for which an id is being
 *        requested
 *
 * \returns the next available id
 *
 * NOTE: This function should not be used for obtaining operator ids.
 *
 */
static unsigned next_endpoint_id(ENDPOINT_DIRECTION direction)
{
    unsigned type = (direction == SINK) ? STREAM_EP_EXT_SINK :
                                        STREAM_EP_EXT_SOURCE;
    unsigned id = stream_next_id.endpoint;
    patch_fn_shared(stream);

    do {
        ++id;
        /* Check for the wrap value of the id */
        if (id > (STREAM_EP_EXT_ID_MASK >> STREAM_EP_EXT_ID_POSN))
        {
            id = 1;
        }
    } while (endpoint_from_id( (type | id) ) != NULL);
    stream_next_id.endpoint = id;

    return (type | id);
}

/**
 * Return the asked buffer size form the buffer details.
 *
 * @param buf_details - Pointer to the buffer details.
 * @return The asked buffer size present in the buffer details.
 */
unsigned int get_buf_size(BUFFER_DETAILS *buf_details)
{
    if(buf_details->supplies_buffer)
    {
        /* Buffer must be different than NULL if the supplied flag is set.*/
        return cbuffer_get_size_in_words(buf_details->b.buffer);
    }
    else if (buf_details->runs_in_place)
    {
        return buf_details->b.in_place_buff_params.size;
    }
    else
    {
        return buf_details->b.buff_params.size;
    }
}

/**
 * Return the asked buffer flags from the buffer details.
 *
 * @param buf_details - Pointer to the buffer details.
 * @return The asked buffer flags present in the buffer details.
 */
unsigned int get_buf_flags(BUFFER_DETAILS *buf_details)
{
    if(buf_details->supplies_buffer)  
    {
        /* Buffer must be different than NULL if the supplied flag is set.*/
        return buf_details->b.buffer->descriptor;
    }
    else if (buf_details->runs_in_place)
    {
        return 0;
    }
    else
    {
        return buf_details->b.buff_params.flags;
    }
}


bool can_run_inplace(BUFFER_DETAILS *buf_details)
{
    patch_fn_shared(stream);
    if (buf_details->runs_in_place)
    {
        if (buf_details->b.in_place_buff_params.buffer &&
                (BUF_DESC_IN_PLACE(buf_details->b.in_place_buff_params.buffer->descriptor) == 0))
        {
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }
    else
    {
        return FALSE;
    }
}


tCbuffer *get_inplace_buff(BUFFER_DETAILS *buf_details)
{
    if (buf_details->runs_in_place && buf_details->b.in_place_buff_params.buffer &&
                (BUF_DESC_IN_PLACE(buf_details->b.in_place_buff_params.buffer->descriptor) != 0))
    {
        return buf_details->b.in_place_buff_params.buffer;
    }
    return NULL;
}

bool stream_get_endpoint_config(ENDPOINT* ep, unsigned key, uint32* value)
{
    ENDPOINT_GET_CONFIG_RESULT result;
    bool success;

    result.u.value = *value;
    success = ep->functions->get_config(ep, key, &result);
    *value = result.u.value;
    return success;
}

void stream_set_endpoint_connection_id(ENDPOINT **ep_list, unsigned num_eps, unsigned connection_id)
{
    unsigned i;
    LOCK_INTERRUPTS;
    for (i = 0; i < num_eps; i++)
    {
        ep_list[i]->con_id = connection_id;
    }
    UNLOCK_INTERRUPTS;
}

