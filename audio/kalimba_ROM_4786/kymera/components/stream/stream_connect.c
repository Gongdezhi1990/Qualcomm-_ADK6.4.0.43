/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  stream_connect.c
 * \ingroup stream
 *
 * stream connection file. <br>
 * This file contains stream functions for connecting a source to a sink. <br>
 *
 * \section sec1 Contains:
 * stream_transform_from_external_id <br>
 * stream_transform_from_endpoint <br>
 * stream_external_id_from_transform <br>
 * stream_connect_endpoints <br>
 * stream_transform_disconnect <br>
 * stream_destroy_transform <br>
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

#if defined(INSTALL_DUAL_CORE_SUPPORT)
#define NEXT_ID(tid) ((tid) == 0)? stream_get_next_transform_id():tid
#else
#define NEXT_ID(tid) ((tid) == 0)? next_id():tid
#endif

#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
#define REMOTE_SUPPLIED_BUFFER(pbuf)  ((pbuf)->remote_buffer && (pbuf)->supplies_buffer)
#else
#define REMOTE_SUPPLIED_BUFFER(pbuf)  FALSE
#endif

/****************************************************************************
Private Variable Definitions
*/
TRANSFORM *transform_list;


/****************************************************************************
Functions
*/

/****************************************************************************
Private Function Declarations
*/
static unsigned next_id(void);
static TRANSFORM *transform_from_id(unsigned id);

static TRANSFORM *connect_transform( ENDPOINT *source_ep, ENDPOINT *sink_ep,
                                     STREAM_CONNECT_INFO* state_info,
                                     unsigned transform_id);

static bool override_endpoint(ENDPOINT* overidden_ep, ENDPOINT* overrider_ep);
/****************************************************************************
Public Function Definitions
*/

#ifdef INSTALL_DUAL_CORE_SUPPORT
/****************************************************************************
 *
 * stream_get_next_transform_id
 *
 */
unsigned stream_get_next_transform_id(void)
{
    unsigned id;

    do {
        id = next_id();
    } while( stream_kip_transform_info_from_id(id) != NULL);

    return id;
}
#endif /* INSTALL_DUAL_CORE_SUPPORT */

/****************************************************************************
 *
 * TRANSFORM *transform_from_external_id(unsigned id)
 *
 */
TRANSFORM *stream_transform_from_external_id(unsigned id)
{
    return transform_from_id((id ^ TRANSFORM_COOKIE));
}

/****************************************************************************
 *
 * TRANSFORM *transform_from_endpoint(ENDPOINT *endpoint)
 *
 */
TRANSFORM *stream_transform_from_endpoint(ENDPOINT *endpoint)
{
    TRANSFORM *t = transform_list;
    while (t && (t->source!=endpoint) && (t->sink!=endpoint))
    {
        t=t->next;
    }
    return t;
}

/****************************************************************************
 *
 * unsigned external_id_from_transform(TRANSFORM *transform)
 *
 */
unsigned stream_external_id_from_transform(TRANSFORM *transform)
{
    return (transform ? (transform->id ^ TRANSFORM_COOKIE) : 0);
}

/****************************************************************************
 *
 * validate endpoints before every connection stage
 *
 */
static bool stream_connect_validate_endpoints( ENDPOINT *source_ep,
                                               ENDPOINT *sink_ep)
{
    PL_PRINT_P2(TR_STREAM, "stream_connect_validate_endpoints: src 0x%x to sink 0x%x \n",
        stream_external_id_from_endpoint(source_ep), stream_external_id_from_endpoint(sink_ep));

    patch_fn_shared(stream_connect);
    /* ensure that we do have a valid source and sink */
    if(!source_ep || !sink_ep)
    {
#ifdef STREAM_CONNECT_FAULT_CODE
        stream_connect_fault = SC_INVALID_SOURCE_OR_SINK;
        L1_DBG_MSG("stream_connect_fault: Invalid source or sink.");
#endif /* STREAM_CONNECT_FAULT_CODE */

        return FALSE;
    }

    /* Check that source_ep is really a source, and so on. */
    if ( stream_direction_from_endpoint(source_ep) != SOURCE ||
         stream_direction_from_endpoint(sink_ep) != SINK)
    {
#ifdef STREAM_CONNECT_FAULT_CODE
        stream_connect_fault = SC_BAD_ENDPOINT_DIRECTION;
        L1_DBG_MSG("stream_connect_fault: Bad endpoint direction.");
#endif /* STREAM_CONNECT_FAULT_CODE */

        return FALSE;
    }

    /* Ensure that neither endpoint is already connected to anything */
    if(source_ep->connected_to || sink_ep->connected_to)
    {
#ifdef STREAM_CONNECT_FAULT_CODE
        stream_connect_fault = SC_SOURCE_OR_SINK_ALREADY_CONNECTED;
        L1_DBG_MSG("stream_connect_fault: Source or sink already connected.");
#endif /* STREAM_CONNECT_FAULT_CODE */

        return FALSE;
    }

    PL_PRINT_P0(TR_STREAM, "stream_connect_validate_endpoints: checks done \n");
    return TRUE;
}

/**
 * \brief internal function to validate endpoints and get the buffer details
 *  from source and sink endpoints
 *
 * \param source_ep -  The Source endpoint.
 * \param sink_ep   -  The Sink endpoint.
 * \param source_buffer_details - Pointer to source buffer details to get populated.
 * \param sink_buffer_details   - Pointer to sink buffer details to get populated.
 *
 * \return TRUE on successfully getting buffer details
 */
static bool stream_connect_get_buffer_details( ENDPOINT *source_ep,
                                               ENDPOINT *sink_ep,
                                             BUFFER_DETAILS *source_buffer_details,
                                             BUFFER_DETAILS *sink_buffer_details )
{
     /* Validate the endpoints and get the buffer details for both endpoints */
    if((stream_connect_validate_endpoints( source_ep, sink_ep)) &&
       (source_ep->functions->buffer_details(source_ep,source_buffer_details)) &&
       (sink_ep->functions->buffer_details(sink_ep,sink_buffer_details)))
    {
        return TRUE;
    }

#ifdef STREAM_CONNECT_FAULT_CODE
    stream_connect_fault = SC_BUFFER_DETAILS_CONNECT_FAILED;
    L1_DBG_MSG("stream_connect_fault: buffer_details() failed.");
#endif /* STREAM_CONNECT_FAULT_CODE */

    return FALSE;
}



/****************************************************************************
 *
 * Create a buffer for the transform or get one for either source or sink endpoint
 *
 */
bool stream_connect_get_buffer( ENDPOINT *source_ep,
                                ENDPOINT *sink_ep,
                                STREAM_CONNECT_INFO* state_info)
{
    BUFFER_DETAILS source_buffer_details;
    BUFFER_DETAILS sink_buffer_details;
    tCbuffer *cbufstruct = NULL;
    ENDPOINT_TIMING_INFORMATION source_timing_info, sink_timing_info;
    unsigned int source_buf_size;
    unsigned int sink_buf_size;
    unsigned int flags = BUF_DESC_SW_BUFFER;

    STREAM_TRANSFORM_BUFFER_INFO *ep_buffer = &(state_info->buffer_info);

    patch_fn_shared(stream_connect);
    if(! stream_connect_get_buffer_details( source_ep,
                                            sink_ep,
                                            &source_buffer_details,
                                            &sink_buffer_details ))
    {
        return FALSE;
    }

    state_info->ep_to_kick = NULL;
    ep_buffer->flags.override_source = FALSE;
    ep_buffer->flags.override_sink = FALSE;
    if (sink_buffer_details.wants_override)
    {
        if (source_buffer_details.can_override)
        {
            /* Configure the source endpoint as override.
             * The buffer details may change after this configuration.
            */
            source_ep->functions->configure(source_ep, EP_OVERRIDE_ENDPOINT, TRUE);
            /* Get the buffer details again.*/
            source_ep->functions->buffer_details(source_ep, &source_buffer_details);
            ep_buffer->flags.override_source = TRUE;
        }
    }
    if (source_buffer_details.wants_override)
    {
        if (sink_buffer_details.can_override)
        {
            /* Configure the sink endpoint as override.
             * The buffer details may change after this configuration.
            */
            sink_ep->functions->configure(sink_ep, EP_OVERRIDE_ENDPOINT, TRUE);
            /* Get the buffer details again.*/
            sink_ep->functions->buffer_details(sink_ep, &sink_buffer_details);
            ep_buffer->flags.override_sink = TRUE;
        }
    }

   /* Get the timing informations. */
    source_ep->functions->get_timing_info(source_ep, &source_timing_info);
    sink_ep->functions->get_timing_info(sink_ep, &sink_timing_info);

    ep_buffer->flags.source_wants_kicks = source_timing_info.wants_kicks;
    ep_buffer->flags.sink_wants_kicks = sink_timing_info.wants_kicks;
    /* At the moment we ask for an MMU buffer, however we check which
     * end needs to have an MMU handle. If neither end has an MMU handle
     * the buffer subsystem will just create us a software buffer
     */
    flags |= get_buf_flags(&source_buffer_details);
    flags |= get_buf_flags(&sink_buffer_details);

    /* In order to make sure the buffer with the expected size is
     * created we add +1 only to buffers
     * that are not MMU, since they need to be multiple of 2.*/

    if ((BUF_DESC_SW_BUFFER == flags))
    {
        if(source_buffer_details.runs_in_place)
        {
            source_buffer_details.b.in_place_buff_params.size++;
        }
        else if(!source_buffer_details.supplies_buffer)
        {
            source_buffer_details.b.buff_params.size++;
        }
        if(sink_buffer_details.runs_in_place)
        {
            sink_buffer_details.b.in_place_buff_params.size++;
        }
        else if(!sink_buffer_details.supplies_buffer)
        {
            sink_buffer_details.b.buff_params.size++;
        }
    }

    /*
     * incase of dual core, source endpoint is expected to supply/create
     * the buffer. If the current context is sink side, it will not create
     * buffer but provide the buffer size information to the remote for it
     * to create the buffer.
     */
    source_buf_size = get_buf_size(&source_buffer_details);
    sink_buf_size = get_buf_size(&sink_buffer_details);

    ep_buffer->buffer_size = source_buf_size > sink_buf_size ?
                             source_buf_size : sink_buf_size;


    ep_buffer->flags.shared_buffer = FALSE;

    if( source_buffer_details.supplies_buffer &&
             sink_buffer_details.supplies_buffer)
    {
        /* Something is wrong if both sides are trying to supply the buffer. If
         * one of the endpoints supplies the buffer don't run in place.
         * supplies_buffer verified before the runs_in_place to achive this. */
        fault_diatribe(FAULT_AUDIO_BOTH_ENDPOINTS_SUPPLYING_BUFFER,
                                        source_buffer_details.runs_in_place);
        return FALSE;
    }
    else if(source_buffer_details.supplies_buffer)
    {
        if (sink_buf_size > source_buf_size)
        {
            /* The buffer size is incremented with one word (in most of the cases,
             * see logic above) so decrement it for a better error message. */
            L0_DBG_MSG4("The buffer supplied by endpoint 0x%04x (size: 0x%x) "
                    "is too small for the other endpoint 0x%04x (size asked: 0x%x)",
                    stream_external_id_from_endpoint(source_ep), source_buf_size - 1,
                    stream_external_id_from_endpoint(sink_ep), sink_buf_size - 1);

            /* This is not an indication of an actual problem. So not raising a Fault 
             * fault_diatribe(FAULT_AUDIO_SUPPLIED_BUFFER_TOO_SMALL, source_buf_size); */
        }
        cbufstruct = source_buffer_details.b.buffer;
        ep_buffer->flags.created_buffer = FALSE;

    }
    else if(sink_buffer_details.supplies_buffer)
    {
        if (source_buf_size > sink_buf_size)
        {
            L0_DBG_MSG4("The buffer supplied by endpoint 0x%04x (size: 0x%x) "
                    "is too small for the other endpoint 0x%04x (size asked: 0x%x)",
                    stream_external_id_from_endpoint(sink_ep), sink_buf_size - 1,
                    stream_external_id_from_endpoint(source_ep), source_buf_size - 1);

            /* This is not an indication of an actual problem. So not raising a Fault 
             * fault_diatribe(FAULT_AUDIO_SUPPLIED_BUFFER_TOO_SMALL, sink_buf_size); */
        }
        cbufstruct = sink_buffer_details.b.buffer;
        ep_buffer->flags.created_buffer = FALSE;
    }
    /* If one of the endpoint is running in place the created cbuffer will use the same
     * underlying memory (same base) as the other cbuffers in the in place chain.
     * Endpoints with hard deadline cannot be part of the in place chain because of the
     * kicking limitations between endpoints running in place. */
    else if (( can_run_inplace(&source_buffer_details)||
               can_run_inplace(&sink_buffer_details)) &&
            ((!source_timing_info.has_deadline)  &&
             (!sink_timing_info.has_deadline)) &&
             (!STREAM_EP_IS_SHADOW(sink_ep)) /* no inplace support for kip eps */
                )
    {
       cbufstruct = connect_in_place( source_ep,
                                      sink_ep,&source_buffer_details,
                                      &sink_buffer_details, &state_info->ep_to_kick);

        if (cbufstruct == NULL)
        {
#ifdef STREAM_CONNECT_FAULT_CODE
            stream_connect_fault = SC_CONNECT_IN_PLACE_FAILED;
            L1_DBG_MSG("stream_connect_fault: connect_in_place() failed.");
#endif /* STREAM_CONNECT_FAULT_CODE */
            return FALSE;
        }

        ep_buffer->flags.created_buffer = TRUE;
        ep_buffer->flags.shared_buffer = TRUE;
    }

    else
    {
        unsigned int buffer_size = ep_buffer->buffer_size;
        ep_buffer->flags.created_buffer = TRUE;


#if defined(CHIP_BASE_BC7)||defined(CHIP_BASE_A7DA_KAS)
        /* This should always be a software only buffer */
        if (BUF_DESC_SW_BUFFER != flags)
        {
            panic(PANIC_AUDIO_NOT_SW_BUFFER);
        }
        cbufstruct = cbuffer_create_with_malloc_fast(buffer_size, BUF_DESC_SW_BUFFER);
#elif defined(CHIP_BASE_HYDRA)
        cbufstruct = cbuffer_create_mmu_buffer_fast(flags, &buffer_size);
#else
#error "buffer creation is not implemented for this platform."
#endif /* CHIP_BASE_BC7 || CHIP_BASE_A7DA_KAS*/

/* ------------------------------------------------------------------------------------ */

        if (cbufstruct == NULL)
        {
            fault_diatribe(FAULT_AUDIO_INSUFFICIENT_MEMORY, buffer_size);
            return FALSE;
        }
    }
    ep_buffer->buffer = cbufstruct;

    return TRUE;
}

/****************************************************************************
 *
 * TRANSFORM *stream_connect_endpoints(ENDPOINT *source_ep, ENDPOINT *sink_ep,
 *                      STREAM_CONNECT_INFO* state_info, <unsigned transform_id>)
 *
 * If we are on secondary core(s), this may have (internal) transform ID imposed
 * otherwise creating new id.
 */
TRANSFORM *stream_connect_endpoints(ENDPOINT *source_ep, ENDPOINT *sink_ep,
                                    STREAM_CONNECT_INFO *state_info,
                                    unsigned transform_id)
{
    TRANSFORM *transform = NULL;

    patch_fn_shared(stream_connect);
    transform = connect_transform(source_ep, sink_ep, state_info, transform_id);

    PL_PRINT_P1(TR_STREAM, "stream_connect_endpoints: created transform %d \n",
                           (unsigned)(uintptr_t)transform);

    /* A new transform potentially changes the timing through a chain so always
       recalculate the timing topology */
    if (transform)
    {
#ifndef CRESENDO_TODO_DUAL_CORE
        /* P1 updates its own chain
         * TODO - P0 should update the entire system chain.
         */
#endif
        if (!stream_chain_update(source_ep, sink_ep))
        {
#ifdef STREAM_CONNECT_FAULT_CODE
            stream_connect_fault = SC_CHAIN_UPDATE_FAILED;
            L1_DBG_MSG("stream_connect_fault: stream_chain_update() failed.");
#endif /* STREAM_CONNECT_FAULT_CODE */
            stream_transform_disconnect(transform);
            return NULL;
        }

        /* If one of the endpoints was real then we need to enable it if the thing
         * it has been connected to is already enabled.
         *
         * This MUST happen after chain_update in case ratematching et. al. needs
         * to insert a cbop. */
        if (source_ep->is_real)
        {
            if (sink_ep->is_enabled)
            {
                source_ep->is_enabled = TRUE;
            }
            if(is_op_running(get_opid_from_opidep(stream_external_id_from_endpoint(sink_ep))))
            {
                stream_enable_endpoint(sink_ep);
            }
#ifndef CRESENDO_TODO_DUAL_CORE
            /*TODO: Handle audio delegation to the second core */
#endif
        }
        if (sink_ep->is_real)
        {
            if (source_ep->is_enabled)
            {
                sink_ep->is_enabled = TRUE;
            }
            if(is_op_running(get_opid_from_opidep(stream_external_id_from_endpoint(source_ep))))
            {
                stream_enable_endpoint(source_ep);
            }
#ifndef CRESENDO_TODO_DUAL_CORE
            /*TODO: Handle audio delegation to the second core */
#endif
        }
        set_system_event(SYS_EVENT_EP_CONNECT);
    }
    else
    {
        L2_DBG_MSG("stream_connect_endpoints: transform = NULL");
    }
    return transform;
}


/****************************************************************************
 * stream_create_endpoint
 *
 * Create a stream endpoint
 *
 */
ENDPOINT* stream_create_endpoint( unsigned ep_id, unsigned con_id)
{
    ENDPOINT*  ep = NULL;
    patch_fn_shared(stream_connect);

    if( STREAM_EP_IS_OPEP_ID(ep_id))
    {
        ep = stream_create_operator_endpoint( ep_id, con_id);
    }
#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
    else if( STREAM_EP_IS_SHADOW_ID( ep_id ) )
    {
        ep = stream_create_shadow_endpoint( (ep_id),con_id);
    }
#endif
    else
    {
        ep = stream_endpoint_from_extern_id( ep_id );
    }

    return ep;
}

/**
 * \brief internal function to check the type and destroy the endpoint
 *        It destroys only operator endpoint and KIP endpoint
 *
 * \param ep_id  The endpoint id
 *
 * \return Endpoint if successfull else NULL.
 */
bool stream_destroy_endpoint_id( unsigned ep_id)
{
    bool status = FALSE;
    patch_fn_shared(stream_connect);

    if( STREAM_EP_IS_OPEP_ID(ep_id))
    {
        status = stream_destroy_operator_endpoint( ep_id);
    }
#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
    else if( STREAM_EP_IS_SHADOW_ID( ep_id ))
    {
        status = stream_destroy_shadow_endpoint(ep_id);
    }
#endif

    return status;
}

/****************************************************************************
 *
 * bool stream_disconnect_endpoint_transform(ENDPOINT *endpoint)
 *
 */
bool stream_disconnect_endpoint_transform( ENDPOINT *endpoint)
{
    patch_fn_shared(stream_connect);

    if (endpoint->connected_to)
    {
#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
        /* If the connected endpoint is shadow, we need
         * do the remote transform disconnect as well.
         * Issuing the request for remote transform disconnect
         * and going ahead with local cleanup.
         *
         * Warning! If remote transform disconnect fails,
         * it will not be notified. Any connection
         * attempts to that remote operator fail in case this
         * disconnect attempt fails. Destroying the remote
         * operator will clean up that transform.
         */
        if(STREAM_EP_IS_SHADOW(endpoint->connected_to))
        {
            IPC_PROCESSOR_ID_NUM proc_id;

            /* There is no record wheere the shadow is connected
             * to. For dual core case, it is just the other core.
             * In mult-core case, shadow endpoints needs to store
             * the processor id
             */
            proc_id = KIP_PRIMARY_CONTEXT()? IPC_PROCESSOR_1:
                                             IPC_PROCESSOR_0;

            stream_kip_disconnect_endpoint( endpoint->connected_to, proc_id);
            endpoint->connected_to = NULL;
        }
#endif

#if defined(AUDIO_SECOND_CORE)
        /* For the secondary processor, the Px-Px transform entry in        */
        /* kip_transforms_list on P0 must also be removed (after disconnect */
        /* succeeded on Px). Get the Px-Px transform id (now, because after */
        /* the stream_transform_disconnect any information of it on Px will */
        /* have disappeared). Then after, request to remove entry on P0.    */
        unsigned tr_id = 0;
        bool tr_id_found = FALSE;

        if (KIP_SECONDARY_CONTEXT())
        {
            tr_id_found = stream_transform_id_from_endpoint(
                                             endpoint->connected_to, &tr_id);
        }
#endif
        if (!stream_transform_disconnect(
                                  stream_transform_from_endpoint(endpoint)))
        {
            return FALSE;
        }
#if defined(AUDIO_SECOND_CORE)
        /* Cleanup px copy of this transform in kip_transform_list on P0 */
        else
        if ((KIP_SECONDARY_CONTEXT()) && (tr_id_found))
        {
            stream_kip_cleanup_endpoint_transform( tr_id, IPC_PROCESSOR_0);
        }
#endif
    }

    return TRUE;
}


/****************************************************************************
 *
 * bool transform_disconnect(TRANSFORM *transform)
 *
 */
bool stream_transform_disconnect(TRANSFORM *transform)
{
    KICK_OBJECT *ko;
    bool restart_sink = FALSE;
    bool restart_src = FALSE;
    bool start_on_connect;
    patch_fn_shared(stream_connect);

    if (!transform)
    {
        return FALSE;
    }

    IN_PLACE_DISCONNECT_PARAMETERS in_place_disconnect_parameters;
    if(!in_place_disconnect_valid(transform, &in_place_disconnect_parameters))
    {
        return FALSE;
    }

    /* If the endpoint is real then it needs to cease activity as after the
     * disconnect the operator that activated it can no longer communicate with
     * the endpoint. */
    if (transform->sink->is_real)
    {
        /* If the endpoint was responsible for kicks then stop it before
         * disconnecting. As the operator can't once it's disconnected */
        if (kick_obj_from_sched_endpoint(transform->sink) != NULL)
        {
            restart_sink = transform->sink->functions->stop(transform->sink);
        }
        transform->sink->is_enabled = FALSE;
    }

    /* If the endpoint is real then it needs to cease activity as after the
     * disconnect the operator that activated it can no longer communicate with
     * the endpoint. */
    if (transform->source->is_real)
    {
        /* If the endpoint was responsible for kicks then stop it before
         * disconnecting. As the operator can't once it's disconnected */
        if (kick_obj_from_sched_endpoint(transform->source) != NULL)
        {
            restart_src = transform->source->functions->stop(transform->source);
        }
        transform->source->is_enabled = FALSE;
    }

    disconnect_in_place_update(transform, &in_place_disconnect_parameters);

    if (!transform->sink->functions->disconnect(transform->sink))
    {
        /* If the disconnect was refused then attempt to restore things we've
         * already undone.
         */
        if (transform->source->is_real)
            {
                transform->source->is_enabled = TRUE;

                if (restart_src)
                {
                    ko = kick_obj_from_sched_endpoint(transform->source);
                    if (ko != NULL)
                    {
                        transform->source->functions->start(transform->source, ko);
                    }
                }
            }

        if (transform->sink->is_real)
        {
            transform->sink->is_enabled = TRUE;

            if(restart_sink)
            {
                ko = kick_obj_from_sched_endpoint(transform->sink);
                if (ko != NULL)
                {
                    transform->sink->functions->start(transform->sink, ko);
                }
            }
        }
        return FALSE;
    }

    if (!transform->source->functions->disconnect(transform->source))
    {
        ENDPOINT_TIMING_INFORMATION tinfo;
        /* If the disconnect was refused then attempt to restore things we've
         * already undone.
         */
        if (transform->source->is_real)
        {
            transform->source->is_enabled = TRUE;

            if (restart_src)
            {
                ko = kick_obj_from_sched_endpoint(transform->source);
                if (ko != NULL)
                {
                    transform->source->functions->start(transform->source, ko);
                }
            }
        }
        transform->source->functions->get_timing_info(transform->source, &tinfo);
        L0_DBG_MSG1("Source wants kicks %d", tinfo.wants_kicks);
        struct ENDPOINT* ep_to_kick = tinfo.wants_kicks ? transform->source: NULL;
        if(!transform->sink->functions->connect(transform->sink, transform->buffer, ep_to_kick, &start_on_connect))
        {
            /* If we could disconnect, nothing should prevent a re-connect. If
             * we get here all we can do is fault and return FALSE */
            fault_diatribe(FAULT_AUDIO_RECONNECT_DURING_DISCONNECT_FAILED,
                                (DIATRIBE_TYPE)(uintptr_t)transform->source);
            return FALSE;
        }
        if (transform->sink->is_real)
        {
            transform->sink->is_enabled = TRUE;

            if (restart_sink)
            {
                ko = kick_obj_from_sched_endpoint(transform->sink);
                if (ko != NULL)
                {
                    transform->sink->functions->start(transform->sink, ko);
                }
            }
        }
        return FALSE;
    }
    transform->sink->connected_to = NULL;
    transform->source->connected_to = NULL;

    set_system_event(SYS_EVENT_EP_DISCONNECT);
    /* If either end was responsible for kicks the kick object should be deleted */
    if(NULL != (ko = kick_obj_from_sched_endpoint(transform->source)))
    {
        kick_obj_destroy(ko);
    }
    if(NULL != (ko = kick_obj_from_sched_endpoint(transform->sink)))
    {
        kick_obj_destroy(ko);
    }

    /* These endpoints could have been involved in ratematching so stop that */
#ifndef A7DA_KAS_DISABLE_STREAM_RM
    cease_ratematching(transform->source->id);
    cease_ratematching(transform->sink->id);
#endif /* A7DA_KAS_DISABLE_STREAM_RM */

    /* Destroy any endpoints that might have been created at stream_connect */

    if (transform->source->destroy_on_disconnect)
    {
        transform->source->can_be_closed = TRUE;

        if (stream_close_endpoint(transform->source))
        {
            transform->source = NULL;
        }
    }

    if (transform->sink->destroy_on_disconnect)
    {
        transform->sink->can_be_closed = TRUE;

        if (stream_close_endpoint(transform->sink))
        {
            transform->sink = NULL;
        }
    }

    /* right then if we have got this far then everything should be ok.*/
    return stream_destroy_transform(transform);
}

/****************************************************************************
 *
 * bool stream_destroy_transform(TRANSFORM *transform)
 *
 */
bool stream_destroy_transform(TRANSFORM *transform)
{
    TRANSFORM *t, **t_p;
    patch_fn_shared(stream_connect);

    /* Iterate through the transform list */
    for (t_p = &transform_list; (t = *t_p) != NULL; t_p = &t->next)
    {
        /* Is this the one we're looking for? */
        if (t == transform)
        {
            /* Close the transform */
            if (!stream_transform_close(t))
            {
                panic_diatribe(PANIC_AUDIO_STREAM_TRANSFORM_INVALID_STATE,t->id);
            }

            /* Remove entry from list and free it */
            *t_p = t->next;
            pfree(t);
            return TRUE;
        }
    }

    panic(PANIC_AUDIO_INVALID_TRANSFORM);
#ifdef DESKTOP_TEST_BUILD
    return FALSE;
#endif

}

/****************************************************************************
 *
 * TRANSFORM *stream_new_transform(ENDPOINT *source_ep, ENDPOINT *sink_ep, unsigned transform_id)
 *
 */
TRANSFORM *stream_new_transform( ENDPOINT *source_ep, ENDPOINT *sink_ep,
                                 unsigned transform_id)
{
    TRANSFORM *t = xzpnew(TRANSFORM);
    patch_fn_shared(stream_connect);
    if (t != NULL)
    {
        t->source = source_ep;
        t->sink = sink_ep;
       /*
        * Use imposed transform ID or generate new one.
        * On dual core, transform id will be decided
        * earlier and the transform_id will be a
        * non-zero value. The secondary core must have validated
        * the transform_id value at KIP.
        */
        t->id = NEXT_ID(transform_id);
        t->next = transform_list;
        transform_list = t;
    }
    return t;
}

/****************************************************************************
 * stream_transform_close_buffer
 */
bool stream_transform_close (TRANSFORM *transform)
{
    patch_fn_shared(stream_connect);
    if (transform->shared_buffer)
    {
        /* If the buffer is shared, get rid of the cbuffer struct
         * and any metadata, leaving the underlying buffer alone
         */
        cbuffer_destroy_struct(transform->buffer);
    }
    else if (transform->created_buffer)
    {
        /* Not shared, so clean up the whole lot */
        cbuffer_destroy(transform->buffer);
    }
    return TRUE;
}

/****************************************************************************
 * stream_transform_get_buffer
 */
tCbuffer* stream_transform_get_buffer(TRANSFORM *transform)
{
    return transform->buffer;
}

/****************************************************************************
 *
 * stream_resolve_endpoint_data_formats
 *
 */
bool stream_resolve_endpoint_data_formats(ENDPOINT *source_ep, ENDPOINT *sink_ep)
{
    uint32 sink_data_format,source_data_format;
    patch_fn_shared(stream_connect);

    if (sink_ep->is_real)
    {
        if(source_ep->is_real)
        {
            /* We only connect real endpoints to other real endpoints in tests.
             * But this is sufficiently common to be handled separately.
             * Configure source endpoint to use FIXP data.  */
            source_data_format = AUDIO_DATA_FORMAT_FIXP;
            if (!source_ep->functions->configure(source_ep, EP_DATA_FORMAT, source_data_format))
            {
                /* Conversion can not be handled */
                return FALSE;
            }
        }
        /* Get the data formats of the source endpoint and then ask the sink
         * to match it. */
        if (stream_get_endpoint_config(source_ep, EP_DATA_FORMAT, &source_data_format))
        {
            if (sink_ep->functions->configure(sink_ep, EP_DATA_FORMAT, source_data_format))
            {
                /* Conversion can be handled and has been set up */
                return TRUE;
            }
        }

    }
    else
    {
        if (stream_get_endpoint_config(sink_ep, EP_DATA_FORMAT, &sink_data_format))
        {
            if(source_ep->functions->configure(source_ep, EP_DATA_FORMAT, sink_data_format))
            {
                /* Conversion can be handled and has been set up */
                return TRUE;
            }
        }
    }

    /* Conversion between the two formats is not supported. */
    return FALSE;
}

/****************************************************************************
 *
 * stream_chain_update
 *
 */

bool stream_chain_update(ENDPOINT *src_ep, ENDPOINT *sink_ep)
{
    /* Stop any existing ratematching activity involving these endpoints
     * This might seem unlikely, but if this endpoint is the head of sync and
     * another synchronised endpoint is already connected, the ratematching pair
     * will point to this one.
     */
    patch_fn_shared(stream_connect);
#ifndef A7DA_KAS_DISABLE_STREAM_RM
    if (!cease_ratematching(src_ep->id))
    {
        return FALSE;
    }
#endif /* A7DA_KAS_DISABLE_STREAM_RM */

    if(sink_ep && src_ep && IS_AUDIO_ENDPOINT(sink_ep) && (IS_AUDIO_ENDPOINT(src_ep)
#ifdef INSTALL_SPDIF
                                                           || IS_SPDIF_ENDPOINT(src_ep)
#endif  
                                                           ))
    {
        /* if a audio (including spdif) source is connected to a audio sink, nothing to do */
        return TRUE;
    }
    if (src_ep && src_ep->is_real)
    {
        set_timing_information_for_real_source(src_ep);
    }
    if (sink_ep && sink_ep->is_real)
    {
        set_timing_information_for_real_sink(sink_ep);
    }

    return TRUE;
}


/**
 * \brief  Helper function to override an endpoint.
 *
 * \param  overidden_ep - pointer to the overriden endpoint.
 * \param  overrider_ep - pointer to the overrider endpoint.
 *
 * \return  Indicates if the operation was successful.
 */
static bool override_endpoint(ENDPOINT* overidden_ep, ENDPOINT* overrider_ep)
{
    uint32 cbops_parameters;
    patch_fn_shared(stream_connect);
    /* Pass the additional cbops parameters from the overridden endpoint.*/
    if (!stream_get_endpoint_config(overidden_ep, EP_CBOPS_PARAMETERS, &cbops_parameters)
            || !overrider_ep->functions->configure(overrider_ep, EP_CBOPS_PARAMETERS, cbops_parameters))
    {
        /* recover from the error. */
        overidden_ep->functions->disconnect(overidden_ep);
        overrider_ep->functions->disconnect(overrider_ep);
        overidden_ep->connected_to = NULL;
        overrider_ep->connected_to = NULL;
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/****************************************************************************
 *
 * TRANSFORM *connect_transform(ENDPOINT *source_ep, ENDPOINT *sink_ep, <unsigned transform_id>)
 *
 * If on secondary core(s), internal transform ID may be imposed from above (well, from P0).
 */
static TRANSFORM *connect_transform( ENDPOINT *source_ep, ENDPOINT *sink_ep,
                                     STREAM_CONNECT_INFO* state_info,
                                     unsigned transform_id)
{
    TRANSFORM *transform;
    BUFFER_DETAILS source_buffer_details;
    BUFFER_DETAILS sink_buffer_details;
    ENDPOINT* ep_to_kick;
    STREAM_TRANSFORM_BUFFER_INFO *ep_buffer = &(state_info->buffer_info);
    bool source_start_on_connect;
    bool sink_start_on_connect;
    patch_fn_shared(stream_connect);

    if(!stream_connect_get_buffer_details( source_ep,
                                            sink_ep,
                                            &source_buffer_details,
                                            &sink_buffer_details ))
    {
        return NULL;
    }

#ifdef INSTALL_METADATA

    /* The connection may need metadata support. Now that there is a buffer
     * configure any metadata support it might need.
     *
     * It's only possible if both sides of the connection support metadata.
     * Not enabling metadata support, is enough information for the metadata
     * module to gracefully handle the one-sided connection cases.*/

    if (source_buffer_details.supports_metadata && sink_buffer_details.supports_metadata)
    {
        if (!buff_metadata_connect(ep_buffer->buffer,
                                   source_buffer_details.metadata_buffer,
                                   sink_buffer_details.metadata_buffer))
        {
            /**
             * To unwind from here all of the transform information is needed, so
             * temporarily make a transform on the stack to pass in to the unwind
             * functions.
             */
            TRANSFORM local_transform;
            local_transform.source = source_ep;
            local_transform.sink = sink_ep;
            local_transform.buffer = ep_buffer->buffer;
            local_transform.created_buffer = ep_buffer->flags.created_buffer;
            local_transform.shared_buffer = ep_buffer->flags.shared_buffer;

            /* TODO in place buffers should unwind here */

            stream_transform_close(&local_transform);

#ifdef STREAM_CONNECT_FAULT_CODE
            stream_connect_fault = SC_METADATA_CONNECT_FAILED;
            L1_DBG_MSG("stream_connect_fault: buff_metadata_connect() failed.");
#endif /* STREAM_CONNECT_FAULT_CODE */

            PL_PRINT_P0(TR_STREAM, "connect_transform::  buff_metadata_connect() failed.\n");
            return NULL;
        }
    }
#endif /* INSTALL_METADATA */
    
    if ((transform = stream_new_transform(source_ep, sink_ep, transform_id)) == NULL)
    {
        /* To unwind from here all of the transform information is needed, so
         * temporarily make a transform on the stack to pass in to the unwind functions. */
        TRANSFORM local_transform;
        local_transform.source = source_ep;
        local_transform.sink = sink_ep;
        local_transform.buffer = ep_buffer->buffer;
        local_transform.created_buffer = ep_buffer->flags.created_buffer;
        local_transform.shared_buffer = ep_buffer->flags.shared_buffer;

        /* Disconnect the in place buffers.*/
        in_place_disconnect_on_error(&local_transform);

        stream_transform_close(&local_transform);

#ifdef STREAM_CONNECT_FAULT_CODE
        stream_connect_fault = SC_NEW_TRANSFORM_FAILED;
        L1_DBG_MSG("stream_connect_fault: stream_new_transform() failed.");
#endif /* STREAM_CONNECT_FAULT_CODE */

        PL_PRINT_P0(TR_STREAM, "connect_transform:: Failed stream_new_transform \n");
        return NULL;
    }
    transform->buffer = ep_buffer->buffer;
    transform->created_buffer = ep_buffer->flags.created_buffer;
    transform->shared_buffer = ep_buffer->flags.shared_buffer;

    /* Attempt to resolve any data format mismatch between the source and sink
     * endpoints before connecting them.
     */
    if ( !stream_resolve_endpoint_data_formats(source_ep, sink_ep))
    {
        /* Disconnect the in place buffers.*/
        in_place_disconnect_on_error(transform);

        stream_destroy_transform(transform);

#ifdef STREAM_CONNECT_FAULT_CODE
        stream_connect_fault = SC_ENDPOINT_FORMATS_UNRESOLVABLE;
        L1_DBG_MSG("stream_connect_fault: Endpoint formats are unresolvable.");
#endif /* STREAM_CONNECT_FAULT_CODE */

        PL_PRINT_P0(TR_STREAM, "connect_transform:: Failed to resolve data mismatch \n");
        return NULL;
    }

    if (!transform->shared_buffer)
    {
        /* Need both backwards and forwards kicks. */
        ep_to_kick = ep_buffer->flags.source_wants_kicks ? source_ep:
                            EP_TO_KICK_FOR_SHADOW_SINK(source_ep, sink_ep);
    }
    else
    {
        ep_to_kick = state_info->ep_to_kick;
    }
    /*
     * connected_to field is initialised before the connect function call because during
     * connect an endpoint or operator might use the override module. The override
     * module is dependent on the endpoint connected to.
     */
    source_ep->connected_to = sink_ep;
    sink_ep->connected_to = source_ep;

    if (sink_ep->functions->connect(sink_ep, ep_buffer->buffer, ep_to_kick, &sink_start_on_connect))
    {
        ep_to_kick = ep_buffer->flags.sink_wants_kicks ? sink_ep:
                            EP_TO_KICK_FOR_SHADOW_SOURCE(source_ep, sink_ep);

        if (!source_ep->functions->connect(source_ep, ep_buffer->buffer, ep_to_kick, &source_start_on_connect))
        {
            sink_ep->functions->disconnect(sink_ep);

            /* Disconnect the in place buffers.*/
            in_place_disconnect_on_error(transform);

            stream_destroy_transform(transform);

#ifdef STREAM_CONNECT_FAULT_CODE
            stream_connect_fault = SC_FAILED_TO_CONNECT_SOURCE_TO_BUFFER;
            L1_DBG_MSG("stream_connect_fault: Failed to connect source to buffer.");
#endif /* STREAM_CONNECT_FAULT_CODE */

            PL_PRINT_P0(TR_STREAM, "connect_transform:: Failed to connect source to buffer \n");
            source_ep->connected_to = NULL;
            sink_ep->connected_to = NULL;
            return NULL;
        }
        /* In-place manager sets up the ep_to_kick if the in-place running was possible.
         * Otherwise, if in place running wasn't possible, the source endpoint will be
         * kicked if it wants kick.*/
        if (!transform->shared_buffer)
        {
            /* If the in-place running was not possible, it is possible that the kicks setup
             * is unfinished for an already complete in-place chain. */
            if (source_buffer_details.runs_in_place)
            {
                complete_kick_for_in_place_chain(source_ep, &source_buffer_details);
            }
            if (sink_buffer_details.runs_in_place)
            {
                complete_kick_for_in_place_chain(sink_ep, &sink_buffer_details);
            }
            /* Need both backwards and forwards kicks. */
            ep_to_kick = ep_buffer->flags.source_wants_kicks ? source_ep:
                                EP_TO_KICK_FOR_SHADOW_SINK(source_ep, sink_ep);
        }
        /* Start the endpoints if both endpoints wants to be started at connected. */
        if (sink_start_on_connect && source_start_on_connect)
        {
            KICK_OBJECT *ko;
            /* Create two separate kick object because the two endpoint scheduling is
             * not related. */
            ko = kick_obj_create(sink_ep, sink_ep);
            sink_ep->functions->start(sink_ep, ko);
            ko = kick_obj_create(source_ep, source_ep);
            source_ep->functions->start(source_ep, ko);
        }
    }
    else
    {
        /* Disconnect the in place buffers.*/
        in_place_disconnect_on_error(transform);

        /*Nothing got connected so just destroy the transform */
        stream_destroy_transform(transform);

#ifdef STREAM_CONNECT_FAULT_CODE
        stream_connect_fault = SC_FAILED_TO_CONNECT_SINK_TO_BUFFER;
        L1_DBG_MSG("stream_connect_fault: Failed to connect sink to buffer.");
#endif /* STREAM_CONNECT_FAULT_CODE */
        PL_PRINT_P0(TR_STREAM, "connect_transform:: Failed to connect sink to buffer\n");
        source_ep->connected_to = NULL;
        sink_ep->connected_to = NULL;
        return NULL;
    }

    if (ep_buffer->flags.override_source)
    {
        if (!override_endpoint(source_ep, sink_ep))
        {
            /* Disconnect the in place buffers.*/
            in_place_disconnect_on_error(transform);
            stream_destroy_transform(transform);

#ifdef STREAM_CONNECT_FAULT_CODE
            stream_connect_fault = SC_SOURCE_OVERRIDE_FAILED;
            L1_DBG_MSG("stream_connect_fault: Source endpoint override failed.");
#endif /* STREAM_CONNECT_FAULT_CODE */

            return NULL;
        }
    }
    if (ep_buffer->flags.override_sink)
    {
        if (!override_endpoint(sink_ep, source_ep))
        {
            /* Disconnect the in place buffers.*/
            in_place_disconnect_on_error(transform);
            stream_destroy_transform(transform);

#ifdef STREAM_CONNECT_FAULT_CODE
            stream_connect_fault = SC_SINK_OVERRIDE_FAILED;
            L1_DBG_MSG("stream_connect_fault: Sink endpoint override failed.");
#endif /* STREAM_CONNECT_FAULT_CODE */

            return NULL;
        }
    }

    return transform;
}

/**
 * \brief Returns the value that is most suitable for zeroing buffer content.
 *
 */
unsigned int get_ep_buffer_zero_value(ENDPOINT* ep)
{
    uint32 data_format;
    unsigned zero_value;

    stream_get_endpoint_config(ep, EP_DATA_FORMAT, &data_format);

    switch ((AUDIO_DATA_FORMAT)data_format)
    {
        case AUDIO_DATA_FORMAT_MU_LAW:
        {
            zero_value = 0xFF;
            break;
        }
        case AUDIO_DATA_FORMAT_A_LAW:
        {
            zero_value = 0x55;
            break;
        }

        default:
            zero_value = 0;
    }

    return zero_value;
}

/****************************************************************************
Private Function Definitions
*/

/**
 * \brief Returns the next transform id to be used
 *
 */
static unsigned next_id(void)
{
    unsigned id = stream_next_id.transform;

    do {
        ++id;
        /* Check the wrap value for the id (limited to 8 bits) */
        if (id >= 255)
            id = 1;
    } while(transform_from_id(id));
    stream_next_id.transform = id;
    return id;
}

/**
 * \brief returns a pointer to the transform with the id
 *        provided <br> will return NULL if there is no
 *        transform with provided id
 *
 * \param id internal id number of the transform
 *
 */
static TRANSFORM *transform_from_id(unsigned id)
{
    TRANSFORM *t= transform_list;
    while (t && (t->id != id))
    {
        t = t->next;
    }
    return t;
}


/**
 * \brief get a "list" of transform ID, source & sink terminal ID triads.
 *
 * \param source_id  internal source ID if filtering list, or 0 for unfiltered list
 * \param sink_id internal sink ID if filtering list, or 0 for unfiltered list
 * \param length  pointer to location where the constructed list is
 * \param conn_list pointer to a pointer to vector where the required information was assembled
 */
void stream_get_connection_list(unsigned source_id, unsigned sink_id, unsigned *length, unsigned** conn_list)
{
    unsigned count = 0, i = 0, temp_id;
    TRANSFORM* t = transform_list;
    patch_fn_shared(stream_connect);

    /* Count how many connections with the specified external source ID, or all if latter is 0 */
    /* The logic can be optimised but a) more readable in raw form for debug, b) life is precious. */
    /* Also relies on internal EP ID being non-zero. */
    while(t != NULL)
    {
        if( ((t->sink->id == sink_id) && (t->source->id == source_id)) ||
            (t->sink->id == sink_id && source_id == 0) || (t->source->id == source_id && sink_id == 0) ||
            (source_id == 0 && sink_id ==0) )
        {
            count++;
        }

        t = t->next;
    }


    /* build list of ID triads */
    if(count != 0)
    {
        *length = 3*count;
        *conn_list = (unsigned*)pmalloc(*length * sizeof(unsigned));

        t = transform_list;

        /* build the joint list - not checking for now the fields it needs, if those are not correct, */
        /* something truly nasty happened - possibly later put some checks and panic or error code */
        while(t != NULL)
        {
            if( ((t->sink->id == sink_id) && (t->source->id == source_id)) ||
                (t->sink->id == sink_id && source_id == 0) || (t->source->id == source_id && sink_id == 0) ||
                (source_id == 0 && sink_id ==0) )
            {
                /* get external transform ID */
                (*conn_list)[i++] = stream_external_id_from_transform(t);

                /* get external source ID */
                temp_id = t->source->id;
                TOGGLE_EP_ID_BETWEEN_INT_AND_EXT(temp_id);
                (*conn_list)[i++] = temp_id;

                /* get external sink ID */
                temp_id = t->sink->id;
                TOGGLE_EP_ID_BETWEEN_INT_AND_EXT(temp_id);
                (*conn_list)[i++] = temp_id;
            }

            t = t->next;
        }
    }
    else
    {
        *length = 0;
        *conn_list = NULL;
    }
}

