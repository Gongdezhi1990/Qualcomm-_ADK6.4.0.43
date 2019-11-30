/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  stream_inplace_mgr.c
 * \ingroup stream
 *
 * In place manager main file. <br>
 * This file contains functionality for making decisions on where and how 
 * to create in-place buffer chains. <br>
 *
 */

/****************************************************************************
Include Files
*/
#include "stream_private.h"
#include "opmgr/opmgr_for_stream.h"

/****************************************************************************
Private Type Declarations
*/

/****************************************************************************
Private Constant Declarations
*/
/* Enable disable debug logging with IN_PLACE_DEBUG */
/*#define IN_PLACE_DEBUG*/

/****************************************************************************
Private Macro Declarations
*/
#define GET_OPID_FROM_OPIDEP(x) (((x) & STREAM_EP_OPID_MASK) >> STREAM_EP_OPID_POSN)



/* Enable disable debug logging with IN_PLACE_DEBUG */
#ifdef IN_PLACE_DEBUG
/* Debug logging macros. */
#define IN_PLACE_DBG_MSG(x)                 L5_DBG_MSG(x)
#define IN_PLACE_DBG_MSG1(x, a)             L5_DBG_MSG1(x, a)
#define IN_PLACE_DBG_MSG2(x, a, b)          L5_DBG_MSG2(x, a, b)
#define IN_PLACE_DBG_MSG3(x, a, b, c)       L5_DBG_MSG3(x, a, b, c)
#define IN_PLACE_DBG_MSG4(x, a, b, c, d)    L5_DBG_MSG4(x, a, b, c, d)
#define IN_PLACE_DBG_MSG5(x, a, b, c, d, e) L5_DBG_MSG5(x, a, b, c, d, e)
#define PRINT_ENDPOINT(EP)                  print_endpoint(EP)
#define PRINT_EP_BUFFER_DETAILS(EP)         print_ep_buffer_details(EP)
#define PRINT_IN_PLACE_CHAIN(IP_CHAIN)      print_in_place_chain(IP_CHAIN)
#define PRINT_TRANSFORM(TRANSFORM)          print_transform(TRANSFORM)
#define PRINT_ASSOCIATED_TRANSFORM(BUFF)    print_associated_transform(BUFF)

extern TRANSFORM *transform_list;

static void print_transform(TRANSFORM *transform)
{
    IN_PLACE_DBG_MSG2("         connects:  0x%4x  -  0x%4x ;",transform->source->id,transform->sink->id);
}

static void print_associated_transform(tCbuffer *buffer)
{
    TRANSFORM *transform = transform_list;
    while(transform)
    {
        if (transform->buffer == buffer)
        {
            print_transform(transform);
        }
        transform = transform->next;
    }
}

static void print_endpoint(ENDPOINT *ep)
{
    IN_PLACE_DBG_MSG3("         endpoint id: 0x%4x; direction: %1d (SOURCE = 0);type: %3d (operator = 2);",
            ep->id, ep->direction,ep->stream_endpoint_type);
}

static void print_ep_buffer_details(BUFFER_DETAILS *buffer_details)
{
    IN_PLACE_DBG_MSG4("         buffer_details runs_in_place: %1d; in_place_terminal: %x; size: %d; buffer:0x%4x;",
            buffer_details->runs_in_place, buffer_details->b.in_place_buff_params.in_place_terminal,
            buffer_details->b.in_place_buff_params.size, buffer_details->b.in_place_buff_params.buffer);
}

static void print_in_place_chain(IN_PLACE_BUFFER_LIST *inplace_buffer_list)
{
    int i;
    IN_PLACE_DBG_MSG("in_place_chain:");
    if (inplace_buffer_list->direction == HEAD_FIRST)
    {
        for (i=0 ; i < inplace_buffer_list->count; i++)
        {
            IN_PLACE_DBG_MSG1("         cbuffer : %x;",inplace_buffer_list->buffers[i]);
            print_associated_transform(inplace_buffer_list->buffers[i]);
        }
    }
    else
    {
        for (i=inplace_buffer_list->count -1 ; i >= 0; i--)
        {
            IN_PLACE_DBG_MSG1("         cbuffer : %x;",inplace_buffer_list->buffers[i]);
            print_associated_transform(inplace_buffer_list->buffers[i]);
        }
    }
}

#else
#define IN_PLACE_DBG_MSG(x)                 ((void)0)
#define IN_PLACE_DBG_MSG1(x, a)             ((void)0)
#define IN_PLACE_DBG_MSG2(x, a, b)          ((void)0)
#define IN_PLACE_DBG_MSG3(x, a, b, c)       ((void)0)
#define IN_PLACE_DBG_MSG4(x, a, b, c, d)    ((void)0)
#define IN_PLACE_DBG_MSG5(x, a, b, c, d, e) ((void)0)
#define PRINT_ENDPOINT(EP)                  ((void)0)
#define PRINT_EP_BUFFER_DETAILS(EP)         ((void)0)
#define PRINT_IN_PLACE_CHAIN(IP_CHAIN)      ((void)0)
#define PRINT_TRANSFORM(TRANSFORM)          ((void)0)
#define PRINT_ASSOCIATED_TRANSFORM(BUFF)    ((void)0)
#endif

/****************************************************************************
Private Variable Definitions
*/

/****************************************************************************
Private Function Definitions
*/

/**
 *
 *  \brief  Initialise an in place buffer list.
 *
 *  \param  inplace_buffer_list - pointer to the in place buffer list.
 *
 */
static void init_in_place_buffer_list(IN_PLACE_BUFFER_LIST *inplace_buffer_list)
{
    IN_PLACE_DBG_MSG("init_in_place_buffer_list:");

    inplace_buffer_list->direction = NO_DIRECTION;
    inplace_buffer_list->count = 0;
    inplace_buffer_list->max_buffer_size = 0;
    inplace_buffer_list->ending_ep = NULL;
}

/**
 *
 *  \brief  Frees an in place buffer list.
 *
 *  \param  inplace_buffer_list - pointer to the in place buffer list.
 *
 */
static inline void destroy_in_place_buffer_list(IN_PLACE_BUFFER_LIST *inplace_buffer_list)
{
    IN_PLACE_DBG_MSG("destroy_in_place_buffer_list:");

    pdelete(inplace_buffer_list);
}

/****************************************************************************
 *
 *  \brief  Adds and in place buffer to the list. The in place max buffer size is not
 *          updated here in this function.
 *
 *  \param  inplace_buffer_list - in place buffer list (chain).
 *  \param  buffer - the new in place buffer.
 */
static inline bool in_place_list_add(IN_PLACE_BUFFER_LIST *inplace_buffer_list,
        tCbuffer *buffer)
{
#ifdef IN_PLACE_DEBUG
    if(inplace_buffer_list->direction == NO_DIRECTION)
    {
        fault_diatribe(FAULT_AUDIO_IN_PLACE_LIST_DIRECTION_ERROR, NO_DIRECTION);
        return FALSE;
    }
#endif
    if (inplace_buffer_list->count < MAX_INPLACE_LENGTH)
    {
        /* Add the new buffer to the list.*/
        inplace_buffer_list->buffers[inplace_buffer_list->count++] = buffer;
    }
    else
    {
        fault_diatribe(FAULT_AUDIO_IN_PLACE_CHAIN_TOO_LONG, MAX_INPLACE_LENGTH);
        return FALSE;
    }

    return TRUE;
}

/****************************************************************************
 *
 *  \brief  Updates the last operator endpoint which is part of the in place
 *          chain
 *
 *  \param  inplace_buffer_list - in place buffer list (chain).
 *  \param  buffer - the new in place buffer.
 */
static inline void in_place_list_update_ending_endpoint(IN_PLACE_BUFFER_LIST *inplace_buffer_list,
        ENDPOINT *ep)
{
    inplace_buffer_list->ending_ep = ep;
}


/****************************************************************************
 *
 *  \brief  Updates the last operator endpoint which is part of the in place
 *          chain
 *
 *  \param  inplace_buffer_list - in place buffer list (chain).
 *  \param  buffer - the new in place buffer.
 */
static inline ENDPOINT * in_place_get_head_source_endpoint(IN_PLACE_BUFFER_LIST *inplace_buffer_list)
{
#ifdef IN_PLACE_DEBUG
    if(inplace_buffer_list->direction == NO_DIRECTION)
    {
        fault_diatribe(FAULT_AUDIO_IN_PLACE_LIST_DIRECTION_ERROR, NO_DIRECTION);
        return FALSE;
    }
#endif

    if (inplace_buffer_list->direction == TAIL_FIRST)
    {
        return inplace_buffer_list->ending_ep;
    }

    return NULL;
}

/****************************************************************************
 *
 *  \brief  Updates the last operator endpoint which is part of the in place
 *          chain
 *
 *  \param  inplace_buffer_list - in place buffer list (chain).
 *  \param  buffer - the new in place buffer.
 */
static inline ENDPOINT *in_place_get_tail_sink_endpoint(IN_PLACE_BUFFER_LIST *inplace_buffer_list)
{
#ifdef IN_PLACE_DEBUG
    if(inplace_buffer_list->direction == NO_DIRECTION)
    {
        fault_diatribe(FAULT_AUDIO_IN_PLACE_LIST_DIRECTION_ERROR, NO_DIRECTION);
        return FALSE;
    }
#endif

    if (inplace_buffer_list->direction == HEAD_FIRST)
    {
        return inplace_buffer_list->ending_ep;
    }

    return NULL;
}

/****************************************************************************
 *
 *  \brief  Sets the direction of the in place buffer list.
 *
 *  \param  inplace_buffer_list - in place buffer list (chain).
 *  \param  direction - Direction of the list.
 */
static inline void in_place_list_set_direction(IN_PLACE_BUFFER_LIST *inplace_buffer_list,
        LIST_DIRECTION direction)
{
#ifdef IN_PLACE_DEBUG
    if((inplace_buffer_list->direction == TAIL_FIRST) ||
       (inplace_buffer_list->direction == HEAD_FIRST))
    {
        /* The direction was already set. */
        fault_diatribe(FAULT_AUDIO_IN_PLACE_LIST_DIRECTION_ERROR, inplace_buffer_list->direction);
    }
#endif
    inplace_buffer_list->direction= direction;

}

/****************************************************************************
 *
 *  \brief  Returns the head of the in place buffer list.
 *
 *  \param  inplace_buffer_list - The in place buffer list
 *  \return head of list.
 */
static inline tCbuffer *in_place_list_get_head(IN_PLACE_BUFFER_LIST *inplace_buffer_list)
{
    if (inplace_buffer_list->direction == HEAD_FIRST)
    {
        return inplace_buffer_list->buffers[0];
    }
    else if (inplace_buffer_list->direction == TAIL_FIRST)
    {
        return inplace_buffer_list->buffers[inplace_buffer_list->count - 1];
    }
    return NULL;
}

static void in_place_set_head_aux_ptr(IN_PLACE_BUFFER_LIST *inplace_buffer_list, void *aux_ptr)
{
    tCbuffer *head = in_place_list_get_head(inplace_buffer_list);

    if(head)
    {
        head->aux_ptr = aux_ptr;
    }
}


/****************************************************************************
 *
 *  \brief  Returns the tail of the in place buffer list.
 *
 *  \param  inplace_buffer_list - The in place buffer list
 *  \return head of tail.
 */
static tCbuffer *in_place_list_get_tail(IN_PLACE_BUFFER_LIST *inplace_buffer_list)
{
    if (inplace_buffer_list->direction == HEAD_FIRST)
    {
        return inplace_buffer_list->buffers[inplace_buffer_list->count - 1];
    }
    else if (inplace_buffer_list->direction == TAIL_FIRST)
    {
        return inplace_buffer_list->buffers[0];
    }
    return NULL;
}


/****************************************************************************
 *
 *  \brief  Sets up the kicking for an in place chain.
 *
 *  \param  head_source_ep - This endpoint will be kicked.
 *  \param  tail_sink_ep - This endpoint will kick.
 *
 *  \param  Returns the head of the in-place chain if the chain is complete.
 */
static ENDPOINT * in_place_list_set_up_kicks(ENDPOINT *head_source_ep, ENDPOINT *tail_sink_ep)
{
    ENDPOINT_TIMING_INFORMATION tinfo;
    BUFFER_DETAILS ep_buffer_details;

    patch_fn_shared(stream_in_place);

#ifdef IN_PLACE_DEBUG
    if (!head_source_ep)
    {
        panic_diatribe(PANIC_AUDIO_IN_PLACE_KICK_SETUP_FAILED, 0);
    }

    if (!tail_sink_ep)
    {
        panic_diatribe(PANIC_AUDIO_IN_PLACE_KICK_SETUP_FAILED, 1);
    }
#endif
    /* Check if the in place chain is complete. Both the head source and the tail sink
     * endpoints must not run in place. */
    if (!head_source_ep->functions->buffer_details(head_source_ep,&ep_buffer_details))
    {
        panic_diatribe(PANIC_AUDIO_IN_PLACE_KICK_SETUP_FAILED, 2);
    }

    /* We know that this is an ending endpoint. So if it still can run in-place it means
     * that the chain is not completed or the head and tail search is not correct.
     * Let's assume the first condition. */
    if (can_run_inplace(&ep_buffer_details))
    {
        IN_PLACE_DBG_MSG("in_place_list_set_up_kicks: The in place chain is not complete yet (head_source_ep). ");
        return NULL;
    }

    if (!tail_sink_ep->functions->buffer_details(tail_sink_ep,&ep_buffer_details))
    {
        panic_diatribe(PANIC_AUDIO_IN_PLACE_KICK_SETUP_FAILED, 3);
    }
    if (can_run_inplace(&ep_buffer_details))
    {
        IN_PLACE_DBG_MSG("in_place_list_set_up_kicks: The in place chain is not complete yet (tail_sink_ep). ");
        return NULL;
    }



    head_source_ep->functions->get_timing_info(head_source_ep, &tinfo);

    if (tinfo.wants_kicks)
    {

        if (tail_sink_ep->stream_endpoint_type == endpoint_operator)
        {
            opmgr_kick_prop_table_add(tail_sink_ep->key, stream_external_id_from_endpoint(head_source_ep));
        }
        else
        {
            tail_sink_ep->ep_to_kick = head_source_ep;
        }

        /* Return the head endpoint of the in-place chain. This will be used by streams
         * to set up the kicking over the in-place chain for the sink endpoint of the
         * transform.*/
        return head_source_ep;
    }

    return NULL;

}

/****************************************************************************
 *
 *  \brief  Cancel the kicking for an endpoint.
 *
 *  \param  ep -  Endpoint which will not kick any more.
 */
static inline void in_place_list_cancel_kicks(ENDPOINT *ep)
{
#ifdef IN_PLACE_DEBUG
    if (!ep)
    {
        panic_diatribe(PANIC_AUDIO_IN_PLACE_KICK_CANCEL_FAILED, 0);
    }
#endif
    if (ep->stream_endpoint_type == endpoint_operator)
    {
        opmgr_kick_prop_table_remove(ep->key);
    }
    else
    {
        ep->ep_to_kick = NULL;
    }
}


/****************************************************************************
 *
 *  \brief  Returns the operator endpoint of if the terminal id and another
 *      endpoint id of the operator is known.
 *
 *  \param  terminal_id - The terminal id of the endpoint.
 *  \param  ep_id - The operator is identified with an endpoint.
 */
static inline ENDPOINT* get_op_connected_to_ep(unsigned terminal_id, unsigned ep_id)
{
    /* remove terminal id. */
    ep_id &= (STREAM_EP_OPID_MASK | STREAM_EP_EP_BIT |STREAM_EP_OP_BIT |STREAM_EP_SINK_BIT);
    /* swap direction. */
    ep_id ^= STREAM_EP_SINK_BIT;
    /* set the new terminal id. */
    ep_id |= (terminal_id & STREAM_EP_CHAN_MASK);

    return endpoint_from_id(ep_id);
}

/****************************************************************************
 *
 *  \brief  Checks if the operator associated with the endpoint is running.
 *      If the operator doesnt't exist the function returns FALSE.
 *
 *  \param  ep_id - The operator is identified with an endpoint.
 */
static inline bool operator_running(unsigned ep_id)
{
    unsigned op_id = GET_OPID_FROM_OPIDEP(ep_id);

    return is_op_running(op_id);
}

/****************************************************************************
 *
 *  \brief  Traverse the in place chains starting with the given endpoint.
 *      Note: If the endpoint is source the traversal will go towards to the source,
 *      if is sink will go towards to the sink.
 *      The traverse can fail if one of the in place operators are already running,
 *      or if the buffer details cannot be read.
 *      If the function fails the in place buffer list created before the fail
 *      will be free. In case of failure no recovery is needed when this function is
 *      called. (The in place list should still be destroying if the function succeed!)
 *
 *  \param  inplace_buffer_list - The already allocated in place buffer list. The
 *      element of the list will be allocated by this functions and destroyed by
 *      destroy_in_place_buffer_list.
 *  \param  ep -  Endpoint to start with.
 *  \param  stop_for_running_op - If true the algorithm will fail when arrives to the
 *      first running operator.
 *
 *  \return True if traversing was successful, false otherwise.
 */
static bool traverse_chain(IN_PLACE_BUFFER_LIST *inplace_buffer_list, ENDPOINT *ep,
        bool stop_for_running_op)
{
    ENDPOINT *temp_ep;
    BUFFER_DETAILS ep_buffer_details;
    IN_PLACE_DBG_MSG("traverse_chain: beginning");
    ENDPOINT_DIRECTION direction = ep->direction ;
    unsigned buffer_size_in_addresses;

    patch_fn_shared(stream_in_place);

    /* set the in place list direction */
    if(direction == SOURCE)
    {
        in_place_list_set_direction(inplace_buffer_list,
                TAIL_FIRST);
    }
    else
    {

        in_place_list_set_direction(inplace_buffer_list,
                HEAD_FIRST);
    }


    do{
        PRINT_ENDPOINT(ep);

        in_place_list_update_ending_endpoint(inplace_buffer_list, ep);

        /* Stop for endpoints different than operator. */
        if (ep->stream_endpoint_type != endpoint_operator)
        {
            return TRUE;
        }


        if (!ep->functions->buffer_details(ep,&ep_buffer_details))
        {
            IN_PLACE_DBG_MSG("traverse_chain: failed to read buffer_details.");
            return FALSE;
        }

        ep_buffer_details.b.in_place_buff_params.size++;

        PRINT_EP_BUFFER_DETAILS(&ep_buffer_details);

        if(!ep_buffer_details.runs_in_place)
        {
            /* Not in place. */
            return TRUE;
        }
        /* Cannot connect to running in-place operators. */
        else if (stop_for_running_op && operator_running(ep->id))
        {
            IN_PLACE_DBG_MSG("traverse_chain: operator running.");
            return FALSE;
        }
        else if (!ep_buffer_details.b.in_place_buff_params.buffer)
        {
            /* the other side is not connected yet. But the asked buffer size should
             * be considered. */
            buffer_size_in_addresses = ep_buffer_details.b.in_place_buff_params.size << LOG2_ADDR_PER_WORD;
            if (buffer_size_in_addresses > inplace_buffer_list->max_buffer_size)
            {
                inplace_buffer_list->max_buffer_size = buffer_size_in_addresses;
            }
            return TRUE;
        }
        else if (BUF_DESC_IN_PLACE(ep_buffer_details.b.in_place_buff_params.buffer->descriptor) == 0)
        {
            /* the other side is connected, but for some reason (for ex the other side
             * supplied the buffer) it was unable to share the buffer. */
            return TRUE;
        }

        buffer_size_in_addresses = ep_buffer_details.b.in_place_buff_params.size << LOG2_ADDR_PER_WORD;
        /* check if the max buffer size changed. */
        if (buffer_size_in_addresses > inplace_buffer_list->max_buffer_size)
        {
            inplace_buffer_list->max_buffer_size = buffer_size_in_addresses;
            IN_PLACE_DBG_MSG1("traverse_chain: New max buffer size:%d",inplace_buffer_list->max_buffer_size);
        }

        /* Depending on the endpoint type add the buffer to the beginning or end of the
         * in place buffer list. In this manner the list will always be in order of
         * source buffer to the sink buffer. */
        if (!in_place_list_add(inplace_buffer_list,
                ep_buffer_details.b.in_place_buff_params.buffer))
        {
            return FALSE;
        }

        PRINT_IN_PLACE_CHAIN(inplace_buffer_list);

        temp_ep = get_op_connected_to_ep(ep_buffer_details.b.in_place_buff_params.in_place_terminal,ep->id);

        if (!temp_ep)
        {
            IN_PLACE_DBG_MSG("traverse_chain: in_place_terminal incorrect. maybe op endpoint not created?");
            /* Operator endpoints are only created during the connect. Don't fail
             * if the endpoint doesn't exist yet. */
            return TRUE;
        }

        /* Get where the routed to terminal is connected*/
        ep = temp_ep->connected_to;
        if (!ep)
        {
            IN_PLACE_DBG_MSG1("traverse_chain:not connected yet %x",temp_ep->id);
            /* not connected. Exit. */
            return TRUE;
        }
    }while (ep_buffer_details.runs_in_place);

    /* Everything went well. */
    return TRUE;
}


/****************************************************************************
 *
 *  \brief  Updates all the buffers in the in place buffer chain.
 *
 *  \param  inplace_buffer_list -
 *  \param  new_base -  Pointer to the new buffer base.
 *  \param  new_buffer_size -  The new buffer size in addresses.
 */
static void update_in_place_chain(IN_PLACE_BUFFER_LIST *inplace_buffer_list,
        int *new_base, unsigned new_buffer_size)
{
    int i;
    IN_PLACE_DBG_MSG("update_in_place_chain:");
    PRINT_IN_PLACE_CHAIN(inplace_buffer_list);

    /* Convert the buffer size to words. */
    new_buffer_size = new_buffer_size >> LOG2_ADDR_PER_WORD;

    for (i=0 ; i < inplace_buffer_list->count; i++)
    {
        cbuffer_buffer_configure(inplace_buffer_list->buffers[i], new_base, new_buffer_size,
                inplace_buffer_list->buffers[i]->descriptor | BUF_DESC_IN_PLACE_MASK);
    }
}

/****************************************************************************
Public Function Definitions
*/


/*
 * in_place_cancel_tail_kick
 */
void in_place_cancel_tail_kick(ENDPOINT *ep)
{
    IN_PLACE_BUFFER_LIST inplace_buffer_list_to_sink;

    if (ep->direction != SINK)
    {
        panic_diatribe(PANIC_AUDIO_IN_PLACE_KICK_CANCEL_FAILED, (DIATRIBE_TYPE)1);
    }

    /* Don't stop at running operators. */
    if (!traverse_chain(&inplace_buffer_list_to_sink, ep, FALSE))
    {
        panic_diatribe(PANIC_AUDIO_IN_PLACE_KICK_CANCEL_FAILED, (DIATRIBE_TYPE)2);
    }

    /* cancel kicking */
    in_place_list_cancel_kicks(in_place_get_tail_sink_endpoint(&inplace_buffer_list_to_sink));

}

/*
 * in_place_disconnect_on_error
 */
void in_place_disconnect_on_error(TRANSFORM* transform)
{
    /* Disconnect the in place buffers.*/
    IN_PLACE_DISCONNECT_PARAMETERS in_place_disconnect_parameters;
    if (in_place_disconnect_valid(transform, &in_place_disconnect_parameters))
    {
        disconnect_in_place_update(transform, &in_place_disconnect_parameters);
    }
}

/**
 *  Disconnects two endpoints involved in in-place cases.
 *
 *  Source   |   Sink    | Action
 * ----------|-------------------
 *    IPC    |    IPC    | The in-place buffer must be split. in_place_disconnect_valid
 *           |           | already allocated the new base. Update all the in_place chain.
 *           |           | Case 1.
 *           |           |
 *    IPC    |    N      | Check if the in place buffer towards to the source can
 *           |           | be shrunken. If the sink asked buffer size is bigger than
 *           |           | the max asked size for the in place chain towards to the
 *           |           | source the in place buffer chain base can be shrunken.
 *           |           | Case 2.
 *           |           |
 *    N      |    IPC    | Check if the in_place buffer towards to the sink can
 *           |           | be shrunken. If the sink buffer size is smaller than the
 *           |           | source the in place buffer chain can be shrinken. If the
 *           |           | source buffer size is smaller there will be no changes in
 *           |           | the in_place chain.
 *           |           | Case 3.
 *           |           |
 *    N      |    N      | No changes to the disconnect logic.
 *           |           | Case 4.
 *           |           |
 *
 *  where:
 *     IP - In place supported.
 *     N  - In-place NOT supported.
 *
 *
 * \brief Disconnects two endpoints which can be involved in a in place chain.
 *
 * \parma buff_to_destroy - Buffer which connects the two endpoints
 *
 * \param source_ep - Pointer to the source endpoint structure.
 * \param sink_ep - Pointer to the sink endpoint structure.
 *
 * \param source_buff - Pointer to the in place buffer shared with the source ep.
 * \param sink_buff - Pointer to the in place buffer shared with the sink ep.
 *
 */

void disconnect_in_place_update(TRANSFORM *transform,
        IN_PLACE_DISCONNECT_PARAMETERS* in_place_disconnect_parameters)
{
    tCbuffer *buff_to_destroy;
    IN_PLACE_BUFFER_LIST* inplace_buffer_list_to_source = in_place_disconnect_parameters->inplace_buffer_list_to_source;
    IN_PLACE_BUFFER_LIST* inplace_buffer_list_to_sink = in_place_disconnect_parameters->inplace_buffer_list_to_sink;

    patch_fn_shared(stream_in_place);

    IN_PLACE_DBG_MSG2("disconnect_in_place_update:  0x%4x  -  0x%4x ;", transform->source->id, transform->sink->id);

    if (!transform)
    {
        panic_diatribe(PANIC_AUDIO_IN_PLACE_DISCONNECT, 0);
    }

    if (!transform->shared_buffer)
    {
        /* Transform is not involved in any in place case. */
        return;
    }

    buff_to_destroy = transform->buffer;
    if (!buff_to_destroy)
    {
        panic_diatribe(PANIC_AUDIO_IN_PLACE_DISCONNECT, 1);
    }

    if ( (inplace_buffer_list_to_source) &&
         (inplace_buffer_list_to_sink)) /* Case 1 */
    {

        if (inplace_buffer_list_to_source->max_buffer_size >
            inplace_buffer_list_to_sink->max_buffer_size)
        {
            update_in_place_chain(inplace_buffer_list_to_sink,
                    in_place_disconnect_parameters->new_base,
                    inplace_buffer_list_to_sink->max_buffer_size);

        }
        else
        {
            update_in_place_chain(inplace_buffer_list_to_source,
                    in_place_disconnect_parameters->new_base,
                    inplace_buffer_list_to_source->max_buffer_size);
        }

        /* Update the head of the list. */
        in_place_set_head_aux_ptr(inplace_buffer_list_to_source,(in_place_list_get_tail(inplace_buffer_list_to_source)) );
        in_place_set_head_aux_ptr(inplace_buffer_list_to_sink, (in_place_list_get_tail(inplace_buffer_list_to_sink)) );

        /* inplace_buffer_list_to_source was pointing to parameters_inplace_buffer_list.
         * No need to destroy the in place buffer list. */
        destroy_in_place_buffer_list(inplace_buffer_list_to_sink);
    }
    else if (inplace_buffer_list_to_source)/* Case 2 */
    {
        /* Compare in addresses. */
        if (buff_to_destroy->size > inplace_buffer_list_to_source->max_buffer_size)
        {
            /* Reduce the buffer size. */
            int *new_base;

            /* Just freed the bigger buffer. This allocation won't fail because
             * it cannot be interrupted . */
            interrupt_block();
            pfree(buff_to_destroy->base_addr);
            /* max_buffer_size is already in bytes.*/
            new_base = (int *)xppmalloc(inplace_buffer_list_to_source->max_buffer_size, MALLOC_PREFERENCE_FAST);
            interrupt_unblock();

            update_in_place_chain(inplace_buffer_list_to_source,
                new_base,
                inplace_buffer_list_to_source->max_buffer_size);
        }


        /* Update the head of the list. */
        in_place_set_head_aux_ptr(inplace_buffer_list_to_source, (in_place_list_get_tail(inplace_buffer_list_to_source)) );

        /* inplace_buffer_list_to_source was pointing to parameters_inplace_buffer_list.
         * No need to destroy the in place buffer list. */
    }
    else if (inplace_buffer_list_to_sink)/* Case 3 */
    {
        /* Compare in addresses. */
        if (buff_to_destroy->size > inplace_buffer_list_to_sink->max_buffer_size)
        {
            /* Reduce the buffer size. */
            int *new_base;

            /* Just freed the bigger buffer. This allocation won't fail because
             * it cannot be interrupted . */
            interrupt_block();
            pfree(buff_to_destroy->base_addr);
            /* max_buffer_size is already in bytes.*/
            new_base = (int *)xppmalloc(inplace_buffer_list_to_sink->max_buffer_size, MALLOC_PREFERENCE_FAST);
            interrupt_unblock();

            update_in_place_chain(inplace_buffer_list_to_sink,
                new_base,
                inplace_buffer_list_to_sink->max_buffer_size);

        }

        /* Update the head of the list. */
        in_place_set_head_aux_ptr(inplace_buffer_list_to_sink, (in_place_list_get_tail(inplace_buffer_list_to_sink)) );

        /* inplace_buffer_list_to_sink was pointing to parameters_inplace_buffer_list.
         * No need to destroy the in place buffer list. */
    }
    else /* Case 4 */
    {
        pfree(buff_to_destroy->base_addr);
    }
}

/**
 * This functions prepares the in place transform for disconnect.
 */
static bool disconnect_in_place_chain(TRANSFORM *transform,
        IN_PLACE_DISCONNECT_PARAMETERS* in_place_disconnect_parameters,
        tCbuffer *source_buff, tCbuffer *sink_buff)
{
    /* Similarly to the connect_in_place during the disconnect a hybrid solution
     * is implemented to save the IN_PLACE_BUFFER_LIST. This time the
     * IN_PLACE_BUFFER_LIST is part of IN_PLACE_DISCONNECT_PARAMETERS which is stored
     * in the stack by the caller function stream_transform_disconnect. If a second
     * IN_PLACE_BUFFER_LIST is needed (case 1) it will be stored in the heap. */

    patch_fn_shared(stream_in_place);

    if (!transform->buffer)
    {
        panic_diatribe(PANIC_AUDIO_IN_PLACE_DISCONNECT, 2);
    }
    IN_PLACE_BUFFER_LIST* inplace_buffer_list_to_source;
    IN_PLACE_BUFFER_LIST* inplace_buffer_list_to_sink;
    /*
     * Endpoint used for traversing the chain towards the source/sink of the data.
     */
    if (source_buff && sink_buff) /* Case 1 */
    {
        IN_PLACE_DBG_MSG("disconnect_in_place_chain: case 1 (see function documentation )");


        inplace_buffer_list_to_source = &(in_place_disconnect_parameters->parameters_inplace_buffer_list);
        init_in_place_buffer_list(inplace_buffer_list_to_source);

        if ( !traverse_chain(inplace_buffer_list_to_source, transform->source, TRUE))
        {
            IN_PLACE_DBG_MSG("disconnect_in_place_chain: traverse_chain failed for source ep");
            return FALSE;
        }

        inplace_buffer_list_to_sink = xpnew(IN_PLACE_BUFFER_LIST);
        if (!(inplace_buffer_list_to_sink))
        {
            return FALSE;
        }
        init_in_place_buffer_list(inplace_buffer_list_to_sink);

        if (!traverse_chain(inplace_buffer_list_to_sink, transform->sink, TRUE))
        {
            IN_PLACE_DBG_MSG("disconnect_in_place_chain: traverse_chain failed for sink ep");
            destroy_in_place_buffer_list(inplace_buffer_list_to_sink);
            return FALSE;
        }

        if(inplace_buffer_list_to_source->count == 0)
        {
            /* No in place chain to the source. */
            if(inplace_buffer_list_to_sink->count != 0)
            {
                /* There is an in place chain at the sink side. */
                in_place_disconnect_parameters->inplace_buffer_list_to_sink = inplace_buffer_list_to_sink;
                /* cancel kicking */
                in_place_list_cancel_kicks(in_place_get_tail_sink_endpoint(inplace_buffer_list_to_sink));
            }
            else
            {
                destroy_in_place_buffer_list(inplace_buffer_list_to_sink);
            }
            return TRUE;
        }
        else if(inplace_buffer_list_to_sink->count == 0)
        {
            /* No in place chain to the sink. */
            in_place_disconnect_parameters->inplace_buffer_list_to_source = inplace_buffer_list_to_source;
            destroy_in_place_buffer_list(inplace_buffer_list_to_sink);

            /* cancel kicking */
            in_place_list_cancel_kicks(transform->sink);
            return TRUE;
        }
        else
        {
            patch_fn_shared(stream_in_place);

            in_place_disconnect_parameters->inplace_buffer_list_to_source = inplace_buffer_list_to_source;
            in_place_disconnect_parameters->inplace_buffer_list_to_sink = inplace_buffer_list_to_sink;

            unsigned new_base_size;
            if (inplace_buffer_list_to_source->max_buffer_size > inplace_buffer_list_to_sink->max_buffer_size)
            {
                new_base_size = inplace_buffer_list_to_source->max_buffer_size;
            }
            else
            {
                new_base_size = inplace_buffer_list_to_sink->max_buffer_size;
            }

            /* A new buffer allocation needed. new_base_size is already in bytes */
            in_place_disconnect_parameters->new_base = (int *)xppmalloc(new_base_size, MALLOC_PREFERENCE_FAST);
            if (in_place_disconnect_parameters->new_base == NULL)
            {
                L0_DBG_MSG("disconnect_in_place_chain: Not enough memory to split the in place chain");
                destroy_in_place_buffer_list(in_place_disconnect_parameters->inplace_buffer_list_to_sink);
                return FALSE;
            }
            else
            {
                /* cancel kicking */
                in_place_list_cancel_kicks(in_place_get_tail_sink_endpoint(inplace_buffer_list_to_sink));
                return TRUE;
            }
        }

    }
    else if (source_buff)/* Case 2 */
    {
        IN_PLACE_DBG_MSG("disconnect_in_place_chain: case 2 (see function documentation )");

        inplace_buffer_list_to_source =  &(in_place_disconnect_parameters->parameters_inplace_buffer_list);
        init_in_place_buffer_list(inplace_buffer_list_to_source);

        if ( !traverse_chain(inplace_buffer_list_to_source, transform->source, TRUE))
        {
            IN_PLACE_DBG_MSG("disconnect_in_place_chain: traverse_chain failed for source ep");
            return FALSE;
        }

        /* Check if the in place chain exist to the source. */
        if(inplace_buffer_list_to_source->count != 0)
        {
            in_place_disconnect_parameters->inplace_buffer_list_to_source = inplace_buffer_list_to_source;

            /* cancel kicking */
            in_place_list_cancel_kicks(transform->sink);
        }
        return TRUE;
    }
    else if (sink_buff)/* Case 3 */
    {
        IN_PLACE_DBG_MSG("disconnect_in_place_chain: case 3 (see function documentation )");


        inplace_buffer_list_to_sink = &(in_place_disconnect_parameters->parameters_inplace_buffer_list);
        init_in_place_buffer_list(inplace_buffer_list_to_sink);

        if ( !traverse_chain(inplace_buffer_list_to_sink, transform->sink, TRUE))
        {
            IN_PLACE_DBG_MSG("disconnect_in_place_chain: traverse_chain failed for source ep");
            return FALSE;
        }

        /* Check if the in place chain exist to the sink. */
        if(inplace_buffer_list_to_sink->count != 0)
        {
            in_place_disconnect_parameters->inplace_buffer_list_to_sink = inplace_buffer_list_to_sink;

            /* cancel kicking */
            in_place_list_cancel_kicks(in_place_get_tail_sink_endpoint(inplace_buffer_list_to_sink));
        }
        return TRUE;
    }
    else /* Case 4 */
    {
        IN_PLACE_DBG_MSG("disconnect_in_place_chain: case 4 (see function documentation )");
        /* Free the buffer base here. The cbuffer struct will be freed in
         * stream_transform_close. */
        return TRUE;

    }

}


/*
 * in_place_disconnect_valid
 */
bool in_place_disconnect_valid(TRANSFORM *transform,
        IN_PLACE_DISCONNECT_PARAMETERS* in_place_disconnect_parameters)
{
    ENDPOINT *source_ep = transform->source;
    ENDPOINT *sink_ep = transform->sink;
    BUFFER_DETAILS source_buffer_details;
    BUFFER_DETAILS sink_buffer_details;

    patch_fn_shared(stream_in_place);

    IN_PLACE_DBG_MSG2("in_place_disconnect_valid:  0x%4x  -  0x%4x ;", source_ep->id, sink_ep->id);

    if (!transform)
    {
        panic_diatribe(PANIC_AUDIO_IN_PLACE_DISCONNECT, 3);
    }

    if (!transform->shared_buffer)
    {
        /* Transform is not involved in any in place case. */
        return TRUE;
    }

    if (!source_ep->functions->buffer_details(source_ep,&source_buffer_details))
    {
        /* Failed to read the buffer_details. */
        return FALSE;
    }
    if (!sink_ep->functions->buffer_details(sink_ep,&sink_buffer_details))
    {
        /* Failed to read the buffer_details. */
        return FALSE;
    }

    in_place_disconnect_parameters->inplace_buffer_list_to_sink = NULL;
    in_place_disconnect_parameters->inplace_buffer_list_to_source = NULL;
    /* Set the new base as null. */
    in_place_disconnect_parameters->new_base = NULL;


    if ((!source_buffer_details.runs_in_place) && (!sink_buffer_details.runs_in_place))
    {
        return TRUE;

    }

    return disconnect_in_place_chain(transform, in_place_disconnect_parameters,
                    get_inplace_buff(&source_buffer_details), get_inplace_buff(&sink_buffer_details));

}

/*
 * Given an operator endpoint which failed to run in-place this function will traverse the
 * in-place chain which ends with this endpoint. If the in-place chain is complete
 * the head and tail of the chain will be configured to set up the kicks.
 */
void complete_kick_for_in_place_chain(ENDPOINT *ep, BUFFER_DETAILS* buffer_details )
{
    /** The operator endpoint which the given endpoint is trying to run in-place on. */
    ENDPOINT *in_place_ep;
    /** Endpoint connected to the in-place endpoint. */
    ENDPOINT *connected_to_in_place_ep;

    patch_fn_shared(stream_in_place);

    if(get_inplace_buff(buffer_details) == NULL)
    {
        /* The other side of the operator where the given endpoint is trying to run
         * in-pace on is not yet connected or not running in-place.
         * No kick setup is needed. */
        IN_PLACE_DBG_MSG("in_place_set_up_kicks: Source not connected or not running in-place");
        return;
    }

    in_place_ep = get_op_connected_to_ep(buffer_details->b.in_place_buff_params.in_place_terminal,ep->id);

    if (!in_place_ep)
    {
        /* If the endpoint is not connected than why do we have the buffer associated
         * with the endpoint. See first check. */
        panic_diatribe(PANIC_AUDIO_IN_PLACE_KICK_SETUP_FAILED, 4);
    }

    connected_to_in_place_ep = in_place_ep->connected_to;

    if (!connected_to_in_place_ep)
    {
        /* If the endpoint is not connected than why do we have the buffer associated
         * with the endpoint. See first check. */
        panic_diatribe(PANIC_AUDIO_IN_PLACE_KICK_SETUP_FAILED, 5);
    }
    else
    {
        IN_PLACE_BUFFER_LIST inplace_buffer_list_on_stack;
        init_in_place_buffer_list(&inplace_buffer_list_on_stack);

        if (!traverse_chain(&inplace_buffer_list_on_stack, connected_to_in_place_ep, FALSE))
        {
            /* Something went terribly wrong */
            panic_diatribe(PANIC_AUDIO_IN_PLACE_KICK_SETUP_FAILED, 6);
        }

        /* Set up kicking. */
        if (ep->direction == SOURCE)
        {
            in_place_list_set_up_kicks(in_place_get_head_source_endpoint(&inplace_buffer_list_on_stack),
                    in_place_ep);
        }
        else
        {
            in_place_list_set_up_kicks(in_place_ep,
                    in_place_get_tail_sink_endpoint(&inplace_buffer_list_on_stack));
        }
    }
}


/*
 *  Connects two endpoint with in place cases.
 *
 *  Source   |   Sink    | Action
 * ----------|-------------------
 *    IPC    |    IPC    | In place chain in both side. Chose the bigger base, remove
 *           |           | the smaller and update the in place chain.
 *           |           | Case 1.
 *           |           |
 *    IPC    |    N      | If the in-place chain base is big enough there is no need to
 *           |           | allocate a new for this buffer. If it is smaller the old base
 *           |           | from the chain should be freed and replaced with a newly
 *           |           | allocated one. The in place chain needs updating. The
 *           |           | ep_to_kick will be populated to set up the kick over the
 *           |           | in-place chain.
 *           |           | Case 2.
 *           |           |
 *    N      |    IPC    | Same as case 2 just the sides are swapped.
 *           |           | Case 3.
 *           |           |
 *    N      |    N      | There is no in-place chain in either of the side. It is like
 *           |           | a normal connect. (but it isn't bc we are here)
 *           |           | Case 4.
 *           |           |
 *
 *  where:
 *     IPC- There is an in-place chain in the associated endpoint.
 *     N  - There is NO in-place chain in the associated endpoint.
 *
 */
tCbuffer *connect_in_place(ENDPOINT *source_ep, ENDPOINT *sink_ep,
        BUFFER_DETAILS* source_buffer_details, BUFFER_DETAILS* sink_buffer_details,
        ENDPOINT **ep_to_kick)
{

    tCbuffer *source_buff = get_inplace_buff(source_buffer_details);
    tCbuffer *sink_buff = get_inplace_buff(sink_buffer_details);
    unsigned source_asked_size = get_buf_size(source_buffer_details);
    unsigned sink_asked_size = get_buf_size(sink_buffer_details);
    unsigned max_asked_size = MAX(source_asked_size, sink_asked_size);

    tCbuffer *created_cbuffer;

    patch_fn_shared(stream_in_place);
    
    /* Asume that there will be no backwards kick for the sink endpoint. */
    *ep_to_kick = NULL;

    /* To save some time on the memory allocation the IN_PLACE_BUFFER_LIST is saved on
     * the stack. Because in case 1 we need two lists which could be just too much for
     * the stack (46 words) a hybrid solution is implemented. In this case, one of the
     * list are in the heap and the other is on the stack (23 words are still safe to
     * store in the stack). */

    if (source_buff && sink_buff)/* Case 1 */
    {
        IN_PLACE_DBG_MSG("connect_in_place: case 1 (see function documentation )");

        IN_PLACE_BUFFER_LIST inplace_buffer_list_on_stack;
        IN_PLACE_BUFFER_LIST* inplace_buffer_list_to_source = &inplace_buffer_list_on_stack;
        init_in_place_buffer_list(inplace_buffer_list_to_source);

        if (!traverse_chain(inplace_buffer_list_to_source, source_ep, TRUE))
        {
            return NULL;
        }

        IN_PLACE_BUFFER_LIST* inplace_buffer_list_to_sink = xpnew(IN_PLACE_BUFFER_LIST);
        if(!inplace_buffer_list_to_sink)
        {
            return NULL;
        }
        init_in_place_buffer_list(inplace_buffer_list_to_sink);

        if (!traverse_chain(inplace_buffer_list_to_sink, sink_ep, TRUE))
        {
            /* The source traverse passed before. */
            destroy_in_place_buffer_list(inplace_buffer_list_to_sink);
            return NULL;
        }

        /* Remove the aux pointer from the sink list head. */
        in_place_set_head_aux_ptr(inplace_buffer_list_to_sink, NULL);

        /* There are in-place chains in both direction. Compare in addresses. */
        if (source_buff->size > sink_buff->size)
        {
            /* The buffer base provided by the source endpoint is bigger and
             * it will be used by the new in place chain. The sink endpoint base will be freed. */

            if (max_asked_size > cbuffer_get_size_in_words(source_buff))
            { 
                /* This probably means an in-place operator asked for 
                 * different buffer sizes at input and output terminals 
                 */
                L2_DBG_MSG1("In-place WARNING: new size %u larger than existing in-place chains", max_asked_size);
            }

            /* free the unused buffer.*/
            pfree(sink_buff->base_addr);

            created_cbuffer = cbuffer_create(source_buff->base_addr,
                    cbuffer_get_size_in_words(source_buff), BUF_DESC_IN_PLACE_MASK);
             
            if (!created_cbuffer)
            {
                destroy_in_place_buffer_list(inplace_buffer_list_to_sink);
                return NULL;
            }

            update_in_place_chain(inplace_buffer_list_to_sink,
                            created_cbuffer->base_addr, created_cbuffer->size);
        }
        else /* (source_buff->size =< sink_buff->size) */
        {
            /* The buffer base provided by the sink endpoint is bigger (or equal with the source) and
             * it will be used by the new in place chain. The source endpoint base will be freed. */

            if (max_asked_size > cbuffer_get_size_in_words(sink_buff))
            { 
                /* This probably means an in-place operator asked for 
                 * different buffer sizes at input and output terminals 
                 */
                L2_DBG_MSG1("In-place WARNING: new size %u larger than existing in-place chains", max_asked_size);
            }

            /* free the old buffer.*/
            pfree(source_buff->base_addr);

            created_cbuffer = cbuffer_create(sink_buff->base_addr,
                    cbuffer_get_size_in_words(sink_buff), BUF_DESC_IN_PLACE_MASK);
             
            if (!created_cbuffer)
            {
                destroy_in_place_buffer_list(inplace_buffer_list_to_sink);
                return NULL;
            }
            
            update_in_place_chain(inplace_buffer_list_to_source,
                            created_cbuffer->base_addr, created_cbuffer->size);
        }
        /* Set the aux pointer. */
        in_place_set_head_aux_ptr(inplace_buffer_list_to_source,(in_place_list_get_tail(inplace_buffer_list_to_sink)) );

        /* Set up kicking. */
        in_place_list_set_up_kicks(in_place_get_head_source_endpoint(inplace_buffer_list_to_source),
                in_place_get_tail_sink_endpoint(inplace_buffer_list_to_sink));


        destroy_in_place_buffer_list(inplace_buffer_list_to_sink);


        return created_cbuffer;

    }
    else if (source_buff)/* Case 2 */
    {
        IN_PLACE_DBG_MSG("connect_in_place: case 2 (see function documentation )");
        /* There is an in-place chain to the source.
         * Check if the base size is big enough.*/

        IN_PLACE_BUFFER_LIST inplace_buffer_list_on_stack;
        IN_PLACE_BUFFER_LIST* inplace_buffer_list_to_source = &inplace_buffer_list_on_stack;
        init_in_place_buffer_list(inplace_buffer_list_to_source);

        if(!traverse_chain(inplace_buffer_list_to_source, source_ep, TRUE))
        {
            return NULL;
        }

        /* Compare in words. */
        if (cbuffer_get_size_in_words(source_buff) < max_asked_size)
        {
            /* The buffer base provided by the endpoint running in place is too
             * small. Get the in place chain bc it need updating.  */

            /* free the old buffer.*/
            pfree(source_buff->base_addr);

            created_cbuffer = cbuffer_create_with_malloc_fast(max_asked_size, BUF_DESC_IN_PLACE_MASK);

        }
        else
        {
            /* The buffer base provided by the endpoint running in place is big enough.
             * Create a new cbuffer with the provided base. */
            created_cbuffer = cbuffer_create(source_buff->base_addr,
                    cbuffer_get_size_in_words(source_buff), BUF_DESC_IN_PLACE_MASK);

        }
             
        if (!created_cbuffer)
        {
            return NULL;
        }
        
        update_in_place_chain(inplace_buffer_list_to_source,
                created_cbuffer->base_addr, created_cbuffer->size);

        /* Set the aux pointer. */
        in_place_set_head_aux_ptr(inplace_buffer_list_to_source, created_cbuffer);

        /* Set up kicking. */
        *ep_to_kick = in_place_list_set_up_kicks(in_place_get_head_source_endpoint(inplace_buffer_list_to_source),
                sink_ep);


        return created_cbuffer;


    }
    else if (sink_buff)/* Case 3 */
    {
        IN_PLACE_DBG_MSG("connect_in_place: case 3 (see function documentation )");
        /* There is an in-place chain to the sink.
         * Check if the base size is big enough.*/

        IN_PLACE_BUFFER_LIST inplace_buffer_list_on_stack;
        IN_PLACE_BUFFER_LIST *inplace_buffer_list_to_sink = &inplace_buffer_list_on_stack;
        init_in_place_buffer_list(inplace_buffer_list_to_sink);

        if(!traverse_chain(inplace_buffer_list_to_sink, sink_ep, TRUE))
        {
            return NULL;
        }

        /* Remove the aux pointer from the sink list head. */
        in_place_set_head_aux_ptr(inplace_buffer_list_to_sink, NULL);


        /* Compare in words. */
        if (cbuffer_get_size_in_words(sink_buff) < max_asked_size)
        {
            /* The buffer base provided by the endpoint running in place is too
             * small. Get the in place chain bc it need updating.  */
            /* free the old buffer.*/

            pfree(sink_buff->base_addr);

            created_cbuffer = cbuffer_create_with_malloc_fast(max_asked_size, BUF_DESC_IN_PLACE_MASK);
        }
        else
        {
            /* The buffer base provided by the endpoint running in place is big enough.
             * Create a new cbuffer with the provided base. */

            created_cbuffer = cbuffer_create(sink_buff->base_addr,
                    cbuffer_get_size_in_words(sink_buff), BUF_DESC_IN_PLACE_MASK);
        }
             
        if (!created_cbuffer)
        {
            return NULL;
        }
            
        update_in_place_chain(inplace_buffer_list_to_sink,
                created_cbuffer->base_addr, created_cbuffer->size);

        /* Set the aux pointer. */
        created_cbuffer->aux_ptr = (int*)(in_place_list_get_tail(inplace_buffer_list_to_sink));

        /* Set up kicking. */
        in_place_list_set_up_kicks(source_ep,
                in_place_get_tail_sink_endpoint(inplace_buffer_list_to_sink));

        return created_cbuffer;
    }
    else /* Case 4 */
    {
        IN_PLACE_DBG_MSG("connect_in_place: case 4 (see function documentation )");

        /* There is no in-place chain in either of the side*/
        unsigned buffer_size = sink_asked_size > source_asked_size ?
                sink_asked_size : source_asked_size;
        /* there is noting at the other end. Creat a new SW buffer. First in-place
         * buffer in the chain no need for updates. */
        created_cbuffer = cbuffer_create_with_malloc_fast(buffer_size, BUF_DESC_IN_PLACE_MASK);

        return created_cbuffer;
    }


}
