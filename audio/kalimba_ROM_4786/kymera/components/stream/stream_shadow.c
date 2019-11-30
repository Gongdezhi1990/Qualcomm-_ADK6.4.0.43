/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  stream_shadow.c
 * \ingroup stream
 *
 * stream shadow type file. <br>
 * This file contains stream functions for shadow
 * endpoints. <br>
 *
 */

/****************************************************************************
Include Files
*/

#include "stream_private.h"
#include "opmgr/opmgr_endpoint_override.h"
#include "opmgr/opmgr_for_stream.h"
#include "ipc/ipc.h"
#include "kip_mgr/kip_mgr.h"

#ifdef INSTALL_METADATA_DUALCORE
#include "platform/pl_assert.h"
#include "buffer_metadata_kip.h"
#endif

/****************************************************************************
Private Type Declarations
*/

/****************************************************************************
Private Constant Declarations
*/
/* Provide the default buffer size in words */
#define DEFAULT_SHADOW_BUFFER_SIZE 128
#define DEFAULT_SHADOW_BLOCK_SIZE 1

/****************************************************************************
Private Macro Declarations
*/

#define SHADOW_EP_CLONE_REMOTE_BUFFER( ep, clone ) (clone)? \
                                                  shadow_clone_ipc_data_buffer( ep ) : \
                                                  shadow_free_cloned_ipc_data_buffer( ep )

/****************************************************************************
Private Variable Definitions
*/

/****************************************************************************
Private Function Declarations
*/

static unsigned shadow_create_stream_key(unsigned int epid);
static bool shadow_connect(ENDPOINT *endpoint, tCbuffer *Cbuffer_ptr, ENDPOINT* connected_wants_kicks, bool* start_on_connect);
static bool shadow_disconnect (ENDPOINT *endpoint);
static bool shadow_buffer_details (ENDPOINT *ep, BUFFER_DETAILS *details);
static bool shadow_configure (ENDPOINT *endpoint, unsigned int key, uint32 value);
static bool shadow_get_config (ENDPOINT *endpoint, unsigned int key, ENDPOINT_GET_CONFIG_RESULT* result);
static void shadow_get_timing (ENDPOINT *endpoint, ENDPOINT_TIMING_INFORMATION *time_info);
static void shadow_kick(ENDPOINT *ep, ENDPOINT_KICK_DIRECTION kick_dir);
static bool shadow_start(ENDPOINT *ep, KICK_OBJECT *ko);
static bool shadow_stop(ENDPOINT *ep );
static bool shadow_close(ENDPOINT *ep );
static bool shadow_sync_sids(ENDPOINT *ep1, ENDPOINT *ep2);
static bool shadow_free_cloned_ipc_data_buffer( ENDPOINT *ep );
static bool shadow_clone_ipc_data_buffer( ENDPOINT *ep );

DEFINE_ENDPOINT_FUNCTIONS (shadow_functions, shadow_close, shadow_connect,
                           shadow_disconnect, shadow_buffer_details,
                           shadow_kick, stream_sched_kick_dummy,
                           shadow_start, shadow_stop,
                           shadow_configure, shadow_get_config,
                           shadow_get_timing, shadow_sync_sids);

/* Get EP ptr from key - in this case key is the ID */
static ENDPOINT *stream_shadow_get_endpoint_from_key(unsigned key)
{
    /* Derive the direction from the key which in the case of shadow
     * endpoints is the same as the epid.
     */
    ENDPOINT_DIRECTION dir = (key & STREAM_EP_SINK_BIT) ? SINK : SOURCE;

    return stream_get_endpoint_from_key_and_functions(key, dir,
                                                  &endpoint_shadow_functions);
}

/****************************************************************************
Public Function Definitions
*/



/****************************************************************************
 *
 * stream_shadow_get_endpoint
 */
ENDPOINT *stream_shadow_get_endpoint(unsigned int epid)
{
    ENDPOINT *ep = NULL;

    unsigned key = shadow_create_stream_key(epid);
    if (key != 0)
    {
        ep = stream_shadow_get_endpoint_from_key(key);
    }

    return ep;
}

/****************************************************************************
 *
 * stream_create_shadow_endpoint
 *
 */
ENDPOINT *stream_create_shadow_endpoint(unsigned epid, unsigned con_id)
{
    ENDPOINT *ep;
    ENDPOINT_DIRECTION dir;
    patch_fn_shared(stream_shadow);

    if ( epid == 0 )
    {
        /* Shadow endpoint must be associated with an existing epid */
        return NULL;
    }

    /* If EP ID is being imposed, get dir from it rather than argument,
     * although latter should match... */
    dir = (epid & STREAM_EP_SINK_BIT) ? SINK : SOURCE;

    if ((ep = STREAM_NEW_ENDPOINT(shadow, epid, dir, con_id)) != NULL)
    {

        PL_PRINT_P2(TR_PL_TEST_TRACE, "Created endpoint: %d for ep id: %d\n",
                                   (unsigned)(uintptr_t) ep, epid);

        /* Shadow endpoints cannot be closed explicitly */
        ep->can_be_closed = FALSE;

        /* it is neither a real endpoint nor a operator endpoint */
        ep->is_real = FALSE;

        /* shadow endpoints must be distroyed always on disconnect */
        ep->destroy_on_disconnect = TRUE;

        /* showdow buffer default size */
        ep->state.shadow.buffer_size = DEFAULT_SHADOW_BUFFER_SIZE;

    }

    return ep;
}

/****************************************************************************
 *
 * stream_enable_shadow_endpoint
 */
void stream_enable_shadow_endpoint( unsigned epid )
{
    ENDPOINT *ep;
    unsigned shadow_id = STREAM_GET_SHADOW_EP_ID(epid);

    if ((ep = stream_endpoint_from_extern_id(shadow_id)) != NULL)
    {
        /* stream_enable_endpoint will start any real endpoint that ep is connected
         * to. The shadow endpoint needs to be marked as enabled to. */
        stream_enable_endpoint(ep);
        ep->is_enabled = TRUE;
    }
}

/****************************************************************************
 *
 * stream_disable_shadow_endpoint
 */
void stream_disable_shadow_endpoint( unsigned epid )
{
    ENDPOINT *ep;
    unsigned shadow_id = STREAM_GET_SHADOW_EP_ID(epid);

    if ((ep = stream_endpoint_from_extern_id(shadow_id)) != NULL)
    {
        /* stream_disable_endpoint will stop any real endpoint that ep is connected
         * to. The shadow endpoint needs to be marked as disabled to. */
        stream_disable_endpoint(ep);
        ep->is_enabled = FALSE;
    }
}

/****************************************************************************
 *
 * shadow_close
 */
bool shadow_close(ENDPOINT *ep)
{
    bool status = FALSE;
    endpoint_shadow_state *state = &ep->state.shadow;

    if (ep != NULL)
    {
        status = TRUE;

        /* overwrite the closeable flag */
        ep->can_be_closed = TRUE;

        if (state->channel_id != 0)
        {
            status = stream_kip_data_channel_destroy(ep->state.shadow.channel_id);
        }
#ifdef INSTALL_METADATA_DUALCORE
        if(status)
        {
            if( (state->supports_metadata == TRUE) && (state->meta_channel_id !=0))
            {
                if(stream_kip_is_last_meta_connection(ep))
                {
                    status = stream_kip_data_channel_destroy_ipc(state->meta_channel_id);
                    if(!status)
                    {
                        return status;
                    }
                    else
                    {
                        state->supports_metadata = FALSE;
                        state->meta_channel_id = 0;
                    }
                }

            }
        }
#endif /* INSTALL_METADATA_DUALCORE */
        /* Make sure we don't try to destroy it twice */
        ep->destroy_on_disconnect = FALSE;
        state->channel_id = 0;
    }

    return status;
}

/****************************************************************************
 *
 * stream_destroy_shadow_endpoint
 */
bool stream_destroy_shadow_endpoint(unsigned epid)
{
    ENDPOINT *ep = stream_endpoint_from_extern_id(epid);
    bool status = shadow_close(ep);

    if(status)
    {
        status = stream_close_endpoint(ep);
    }

    return status;
}


/****************************************************************************
Private Function Definitions
*/

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
static bool shadow_connect(ENDPOINT *endpoint, tCbuffer *Cbuffer_ptr, ENDPOINT* connected_wants_kicks, bool* start_on_connect)
{
    endpoint_shadow_state *state = &endpoint->state.shadow;


    /* If the buffer is already cloned locally for the data channel
     * the provided buffer is expected to be same
     */
    if( state->buffer == NULL )
    {
        state->buffer = Cbuffer_ptr;
         }

    /* The shadow state buffer present must be
     * same as the Cbuffer provided in connect.
     * unexpected error */
    if( state->buffer != Cbuffer_ptr )
    {
        return FALSE;
    }

    endpoint->ep_to_kick = connected_wants_kicks;
    *start_on_connect = FALSE;
    return TRUE;
}

/*
 * \brief Disconnects from an endpoint and stops the data from flowing
 *
 * \param *endpoint pointer to the endpoint that is being disconnected
 *
 */
static bool shadow_disconnect(ENDPOINT *endpoint)
{
    bool status = FALSE;
    endpoint_shadow_state *state = &endpoint->state.shadow;

#ifdef INSTALL_METADATA_DUALCORE
    if( (state->supports_metadata == TRUE) &&
       (state->meta_channel_id !=0))
    {
        if(stream_kip_is_last_meta_connection(endpoint))
        {
            status = stream_kip_data_channel_deactivate_ipc(state->meta_channel_id);
            if(!status)
            {
                return status;
            }
            else
            {
                state->metadata_shared_buf = NULL;
            }
        }
    }
#endif /*INSTALL_METADATA_DUALCORE*/

    status = stream_kip_data_channel_deactivate(state->channel_id);

    if( status )
    {
        shadow_free_cloned_ipc_data_buffer(endpoint);
    }

    return status;
}

/*
 * \brief Obtains details of the buffer required for this connection
 *
 * \param *endpoint pointer to the endpoint from which the buffer
 *         information is required
 *
 */
static bool shadow_buffer_details (ENDPOINT *ep, BUFFER_DETAILS *details)
{
    endpoint_shadow_state *state = &ep->state.shadow;

    /* Even though shadow endpoints commits to supply the buffer
     * It doesn't own any buffer now which will be updated while
     * activating the data channel.
     */

    details->can_override = FALSE;
    details->wants_override = FALSE;
    details->runs_in_place = FALSE;

    if(ep->direction == SOURCE)
    {
        if( state->cloned_buffer )
            {
            details->b.buffer = state->buffer;
        }
        else
        {
            details->b.buffer =  ipc_data_channel_get_cbuffer( state->channel_id );
        }

        /* Failed to provide a buffer. return early */
        if( details->b.buffer == NULL )
        {
            return FALSE;
        }

        details->supplies_buffer = TRUE;
    }
    else
    {
        details->b.buff_params.flags = BUF_DESC_SW_BUFFER;
        details->b.buff_params.size = state->buffer_size;
        details->supplies_buffer = FALSE;
    }
#ifdef INSTALL_METADATA_DUALCORE
    details->supports_metadata = state->supports_metadata;
    details->metadata_buffer = NULL;

    if(details->supports_metadata)
    {
        details->metadata_buffer = stream_kip_return_metadata_buf(ep);
    }

#endif /* INSTALL_METADATA_DUALCORE */

    return TRUE;
}

/*
 * \brief Free the cloned ipc data buffer
 *
 * \param *endpoint pointer to the shadow endpoint
 *
 * \return void
 */
static bool shadow_free_cloned_ipc_data_buffer( ENDPOINT *ep )
{
    endpoint_shadow_state *state = &ep->state.shadow;

    if(state->cloned_buffer &&  (state->buffer != NULL))
    {
        cbuffer_destroy_struct(state->buffer);
        state->buffer = NULL;
    }

    state->cloned_buffer = FALSE;

    return TRUE;
}

/*
 * \brief Create a clone of the remote buffer and set the shadow state accordingly
 *
 * \param *endpoint pointer to the shadow endpoint
 *
 * \return Status of the clone request.
 */
static bool shadow_clone_ipc_data_buffer( ENDPOINT *ep )
{
    endpoint_shadow_state *state = &ep->state.shadow;

    /* Clone the buffer only if there is no buffer associated */
    if( state->buffer == NULL)
    {
        tCbuffer* cbuffer =  ipc_data_channel_get_cbuffer( state->channel_id );

        /* provide a cloned buffer and store it as shadow buffer
         * The same buffer will be supplied back during stream connect.
        * If it is being overridden at that point, we will use the new
        * supplied buffer.
        */
        state->buffer  = cbuffer_create( cbuffer->base_addr,
                                         cbuffer_get_size_in_words(cbuffer),
                                         BUF_DESC_SW_BUFFER);
        if (state->buffer != NULL)
        {
            cbuffer_buffer_sync(state->buffer, cbuffer);
            state->cloned_buffer = TRUE;
        }
        else
        {
            state->cloned_buffer = FALSE;
        }
    }

     return state->cloned_buffer;
}


/*
 * \brief configure an shadow endpoint with a key and value pair
 *
 * \param *endpoint pointer to the endpoint to be configured
 * \param key denoting what is being configured
 * \param value pointer to a value to which the key is to be configured
 *
 * Not all configurations are currenly accepted. This can be extended
 * when required.
 *
 * \return Whether the request succeeded.
 *
 */
static bool shadow_configure(ENDPOINT *endpoint, unsigned int key, uint32 value)
{
    switch(key)
    {
    case EP_DATA_FORMAT:
        endpoint->state.shadow.data_format = (AUDIO_DATA_FORMAT)value;
        return TRUE;

    case EP_OVERRIDE_ENDPOINT:
        /* This feature is not supported for the shadow endpoint */
        return FALSE;

    case EP_SET_DATA_CHANNEL:
        endpoint->state.shadow.channel_id = (uint16)value;
        return TRUE;

    case EP_CLONE_REMOTE_BUFFER:
        return SHADOW_EP_CLONE_REMOTE_BUFFER( endpoint, (value != 0) );

    case EP_SET_SHADOW_BUFFER_SIZE:
        endpoint->state.shadow.buffer_size = (uint16)value;
        return TRUE;

#ifdef INSTALL_METADATA_DUALCORE
    case EP_METADATA_SUPPORT:
        endpoint->state.shadow.supports_metadata = (bool)value;
        return TRUE;

    case EP_METADATA_CHANNEL_ID:
        endpoint->state.shadow.meta_channel_id = (uint16)value;
        return TRUE;

    case EP_METADATA_CHANNEL_BUFFER:
        endpoint->state.shadow.metadata_shared_buf = (tCbuffer*)(uintptr_t)value;
        return TRUE;
#endif /* INSTALL_METADATA_DUALCORE */

    case EP_CBOPS_PARAMETERS:
    case EP_KICK_PERIOD:
    case EP_BLOCK_SIZE:
    case EP_PROC_TIME:
    case EP_RATEMATCH_ENACTING:
    case EP_RATEMATCH_ADJUSTMENT:
    case EP_RATEMATCH_REFERENCE:
    case EP_KICKED_FROM_ENDPOINT:
    default:
        return FALSE;
    }
}


/*
 * \brief get shadow endpoint configuration
 *
 * \param *endpoint pointer to the endpoint to be configured
 * \param key denoting what is being configured
 * \param value pointer to a value which is populated with the current value
 *
 * \return Whether the request succeeded.
 *
 */
static bool shadow_get_config(ENDPOINT *endpoint, unsigned int key, ENDPOINT_GET_CONFIG_RESULT* result)
{
    switch(key)
    {
    case EP_DATA_FORMAT:
        result->u.value = (uint32)endpoint->state.shadow.data_format;
        return TRUE;

    case EP_BLOCK_SIZE:
        /* TODO -  Currently always defaulted */
        result->u.value = (uint32)DEFAULT_SHADOW_BLOCK_SIZE;
        return TRUE;

    case EP_KICK_PERIOD:
        result->u.value = 0;
        return TRUE;
#ifdef INSTALL_METADATA_DUALCORE

    case EP_METADATA_SUPPORT:
        result->u.value = (uint32)endpoint->state.shadow.supports_metadata;
         return TRUE;

    case EP_METADATA_CHANNEL_ID:
        result->u.value = (uint32)endpoint->state.shadow.meta_channel_id;
         return TRUE;

#endif
    case EP_PROC_TIME:
    case EP_RATEMATCH_ABILITY:
    case EP_RATEMATCH_RATE:
    case EP_RATEMATCH_MEASUREMENT:
       
    default:
        return FALSE;
    }
}


/*
 * \brief generates an shadow key (same as an shadow endpoint id) from an
 *        shadow id, index and direction
 *
 * \param epid the shadow ep id (NB must be the PUBLIC opid)
 *
 * \return generated shadow key
 */
static unsigned shadow_create_stream_key(unsigned int epid)
{
    return epid;
}


static void shadow_get_timing (ENDPOINT *endpoint, ENDPOINT_TIMING_INFORMATION *time_info)
{
    /* Locally clocked can only be not true for real op endpoints */
    time_info->locally_clocked = TRUE;

    time_info->has_deadline = FALSE;
    time_info->wants_kicks = TRUE;

    /* TODO MULTICORE - This needs to be extracted from the shadowed capability.
     *  Currently using fixed values */
    time_info->period = 0;
    time_info->block_size = DEFAULT_SHADOW_BLOCK_SIZE;


    return;
}

/**
 * \brief  Kick the shadow endpoint
 *
 * \param ep - The endpoint
 * \param kick_dir - The kick direction
 *
 * Kick via KIP signals that yell data produced/consumed to the other end.
 * Latter events will cause data read/write at the other end in the respective handlers.
 * when implicit sync'ing is introduced for terminals of same operator, a kick to any shadow EP
 * data channel in a certain IPC port results in kick across all shadow EPs of all
 * data channels in that port!
 *
 * \return  TRUE on success
 */
static void shadow_kick(ENDPOINT *ep, ENDPOINT_KICK_DIRECTION kick_dir)
{
    patch_fn_shared(stream_shadow);

    endpoint_shadow_state *shadow = &ep->state.shadow;
    tCbuffer *buffer = shadow->buffer;
    uint16 channel_id = shadow->channel_id;

    if (!ep->is_enabled)
    {
        /* There is a potential race between the cores, so we can get kicked
         * when things are getting torn down so check the running state and
         * exit if we shouldn't be here. */
        return;
    }

    /* This can be called from multiple contexts. It's not re-entrant so flag it
     * and return. The kick call that is executing will check when it finishes
     * and will run again. */
    if (shadow->kick_in_progress)
    {
        shadow->kick_blocked = kick_dir;
        return;
    }

    do
    {
        /* Set kick_blocked to false first so that we know when we finish this
         * iteration if another kick came in whilst running. */
        shadow->kick_blocked = STREAM_KICK_INVALID;
        shadow->kick_in_progress = TRUE;

        if( ep->direction == SINK )
        {
#ifdef INSTALL_METADATA_DUALCORE
            if ((shadow->metadata_shared_buf != NULL) && (buffer->metadata != NULL))
            {
                LOCK_INTERRUPTS;
                metadata_tag* tag = buffer->metadata->tags.head;

                /**
                 * In addition to moving tags, we also need to provide an update
                 * of prev_wr_index for the other core. If we succeed in pushing
                 * all the tags to the KIP buffer this will be as simple as
                 * writing the current value prev_wr_index to the shared memory
                 * but when there's not enough space in the KIP buffer, we should
                 * pass a prev_wr_index that reflects the status of tags in the
                 * KIP buffer. This can be achieved by using the return value of
                 * buff_metadata_push_tags_to_KIP(), which is the first tag we
                 * *failed* to copy to the KIP buffer. The index of this tag is
                 * effectively the write index of the tags we have successfully
                 * copied to the KIP buffer. Let's start by assuming success and
                 * adjust in case of failure:
                 */
                unsigned effective_wr_index = buffer->metadata->prev_wr_index;

                if (tag != NULL)
                {
                    metadata_tag* head_tag = buff_metadata_push_tags_to_KIP(shadow->metadata_shared_buf, tag);

                    /**
                     * We only need to update tail if above returns NULL which means
                     * we've successfully pushed ALL tags to KIP buffer.
                     */
                    buffer->metadata->tags.head = head_tag;

                    if (head_tag == NULL)
                    {
                        /** We've successfully copied everything */
                        buffer->metadata->tags.tail = NULL;
                    }
                    else
                    {
                        /**
                         * If here, head_tag points to the tag we failed to copy
                         * to the KIP buffer - we can use its write index for
                         * syncing.
                         */
                        effective_wr_index = head_tag->index;
                    }
                }

                KIP_METADATA_BUFFER* kip_metadata_buf = (KIP_METADATA_BUFFER*)shadow->metadata_shared_buf;

                /* Provide an update to the other side */
                kip_metadata_buf->prev_wr_index = effective_wr_index;

                /* Pick up from the other side */
                buffer->metadata->prev_rd_index = kip_metadata_buf->prev_rd_index;

                UNLOCK_INTERRUPTS;
            }
#endif /* INSTALL_METADATA_DUALCORE */
            unsigned data = cbuffer_calc_amount_data_in_words (buffer);

            if(kick_dir == STREAM_KICK_FORWARDS)
            {
                /*  kick forward */
                shadow->remote_kick = TRUE;
            }

            /* write through IPC and update the read pointer as well
             * If some data was already consumed.
             */
             ipc_data_channel_write_sync(buffer, channel_id);

             /* If there is less data in the buffer after write sync
              * to data channel,
              * means the data has been read as well.
              * So we need to kick backwards and get more data.
              */
              if( (data <= DEFAULT_SHADOW_BLOCK_SIZE ) ||
                  (data > cbuffer_calc_amount_data_in_words (buffer)) )
              {
                  propagate_kick(ep, STREAM_KICK_BACKWARDS);
              }



        }
        else /* source */
        {
#ifdef INSTALL_METADATA_DUALCORE
            if (shadow->metadata_shared_buf != NULL)
            {
                KIP_METADATA_BUFFER* kip_metadata_buf = (KIP_METADATA_BUFFER*)shadow->metadata_shared_buf;
                metadata_tag *head_tag, *tail_tag;

                LOCK_INTERRUPTS;
                head_tag = buff_metadata_pop_tags_from_KIP(shadow->metadata_shared_buf, &tail_tag);

                if (buffer->metadata == NULL)
                {
                    /**
                     * This condition is here to make dual-core behave similar to
                     * buff_metadata_append() function which throws tags away if
                     * there's no metadata list in the destination buffer. This
                     * is not great. Ideally we'd like to keep the logic here as
                     * dumb as possible so that it only moves tags across cores.
                     */
                    buff_metadata_tag_list_delete(head_tag);

                    /**
                     * Provide an update to the other side: all tags were
                     * consumed.
                     */
                    kip_metadata_buf->prev_rd_index = kip_metadata_buf->prev_wr_index;
                }
                else
                {
                    /* In startup situations, it is possible for a capability such as SBC
                     * to have processed a couple of frames and incremented the buffer index.
                     * If no tags were available (if 'shadow_kick' hasn't delivered them
                     * yet at that point), the buffer index will be larger than index in
                     * the first (few) tag(s) that 'shadow_kick' receives from the metadata
                     * buffer. This causes problems because 'buff_metadata_remove' (called
                     * by capabilities such as SBC) expect tags to have indices equal or
                     * larger than the buffer index. So here we verify and remove any tags
                     * with indices that are less than the buffer index. We do this only
                     * at startup, e.g. when 'shadow_kick' has first popped tags from KIP
                     * from the KIP buffer.
                     */
                    if (!kip_metadata_buf->kip_sync)
                    {
                        kip_metadata_buf->kip_sync = 
                             buff_metadata_flush_KIP_tags(buffer, &head_tag, &tail_tag);
                    }


                    /* Pick up from the other side */
                    buffer->metadata->prev_wr_index = kip_metadata_buf->prev_wr_index;

                    /* Provide an update to the other side */
                    kip_metadata_buf->prev_rd_index = buffer->metadata->prev_rd_index;

                    if (head_tag != NULL)
                    {
                        if (buffer->metadata->tags.head == NULL) // Empty list
                        {
                            PL_ASSERT(buffer->metadata->tags.tail == NULL);
                            buffer->metadata->tags.head = head_tag;
                        }
                        else
                        {
                            PL_ASSERT(buffer->metadata->tags.tail != NULL);
                            buffer->metadata->tags.tail->next = head_tag;
                        }

                        buffer->metadata->tags.tail = tail_tag;
                    }
                }
                UNLOCK_INTERRUPTS;

            }
#endif /* INSTALL_METADATA_DUALCORE */

            ipc_data_channel_read_sync(channel_id, buffer);

            /* If there is no space in the buffer, no need to kick further
             * backwards
             */
            if ( kick_dir == STREAM_KICK_BACKWARDS)
            {
                shadow->remote_kick = TRUE;
            }
            else if(cbuffer_calc_amount_data_in_words (buffer) >= DEFAULT_SHADOW_BLOCK_SIZE)
            {
                /* Kick forward only if there is some data in the buffer. Otherwise,
                 * there is no need to kick forward
                 */
                propagate_kick(ep, STREAM_KICK_FORWARDS);
            }


        }
        shadow->kick_in_progress = FALSE;
        /* It's possible for an interrupt to fire here, and service kick_blocked
         * and the loop will run again with nothing to do. This should happen
         * very rarely. */
        /* We need to update kick direction so that a signal is raised if it
         * should have been given where the blocked kick originated from. */
        kick_dir = shadow->kick_blocked;
    } while (kick_dir != STREAM_KICK_INVALID);

    /* Raise the signal for Kick only if the transform is present in the kip transform
     * list and if it is enabled
     */
    if(shadow->remote_kick )
    {
        ipc_signal sig;
        IPC_PROCESSOR_ID_NUM proc_id = (KIP_PRIMARY_CONTEXT()) ? IPC_PROCESSOR_1 : IPC_PROCESSOR_0;
        uint16 signal_id = (ep->direction == SINK) ? KIP_SIGNAL_ID_KICK : KIP_SIGNAL_ID_REVERSE_KICK;

        sig.signal_size = sizeof(uint16);
        sig.signal_ptr = kip_get_signal_ptr(channel_id, signal_id);

        /* if raising the signal failed, attempt it again in the next kick */
        if(ipc_raise_signal(proc_id, signal_id, &sig) == IPC_SUCCESS)
        {
            shadow->remote_kick = FALSE;
        }
    }
}

static bool shadow_start(ENDPOINT *ep, KICK_OBJECT *ko)
{
    return TRUE;
}

static bool shadow_stop(ENDPOINT *ep)
{
    if (!ep->is_enabled)
    {
        return FALSE;
    }

    return TRUE;
}

static bool shadow_sync_sids(ENDPOINT *ep1, ENDPOINT *ep2)
{
    /* If theses are both shadows then we can synchronise them. */
    if (ep1->stream_endpoint_type == endpoint_shadow &&
            ep2->stream_endpoint_type == endpoint_shadow)
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

