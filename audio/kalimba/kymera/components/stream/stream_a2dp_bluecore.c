/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup A2dp A2dp endpoint
 * \ingroup endpoints
 * \file  stream_a2dp_bluecore.c
 *
 * stream a2dp type file. <br>
 * This file contains stream functions for bluecore a2dp endpoints. <br>
 *
 * \section sec1 Contains:
 * stream_a2dp_get_endpoint <br>
 */

/****************************************************************************
Include Files
*/

#include "stream_private.h"
#include "io_defs.h"            /* for cbuffer_reconfigure_rd|wr_port */
#include "bluecore_port.h"
#include "stream_endpoint_a2dp.h"

/****************************************************************************
Private Type Declarations
*/

/****************************************************************************
Private Constant Declarations
*/

/** The timer kick period of the endpoint */
#define A2DP_KICK_PERIOD 8000

/** The location of the port number in the a2dp_get_endpoint params */
#define PORT_NUM 0

/** The number of kicks without data arriving over the air before we consider
 * the input stream to be stalled. */
#define NO_KICK_LIMIT 10

/****************************************************************************
Private Macro Declarations
*/

/****************************************************************************
Private Variable Definitions
*/

/****************************************************************************
Private Function Declarations
*/
static bool a2dp_close(ENDPOINT *endpoint);
static void a2dp_get_timing(ENDPOINT *, ENDPOINT_TIMING_INFORMATION *);
static void a2dp_kick(ENDPOINT *endpoint, ENDPOINT_KICK_DIRECTION kick_dir);
static void a2dp_sched_kick(ENDPOINT *, KICK_OBJECT *);
static bool a2dp_start(ENDPOINT *, KICK_OBJECT *);
static bool a2dp_stop(ENDPOINT *);

DEFINE_ENDPOINT_FUNCTIONS(a2dp_functions, a2dp_close, a2dp_connect,
                          a2dp_disconnect, a2dp_buffer_details,
                          a2dp_kick, a2dp_sched_kick, a2dp_start,
                          a2dp_stop, a2dp_configure, a2dp_get_config,
                          a2dp_get_timing, stream_sync_sids_dummy);

/****************************************************************************
Public Function Definitions
*/

/****************************************************************************
 *
 * stream_a2dp_get_endpoint
 *
 */
ENDPOINT *stream_a2dp_get_endpoint(unsigned con_id, ENDPOINT_DIRECTION dir,
                                    unsigned num_params, unsigned *params)
{
    ENDPOINT *ep;
    unsigned key;

    /* There should only be a port in the params list, but there is always a
     * space in the baton message union for a list of params packed with the
     * port so there will be 2 params the portid and a 0. */
    if (num_params != 2)
    {
        return NULL;
    }

    /* On Bluecore its always a new endpoint so no need to check whether it exists. */
    key = stream_get_new_endpoint_key(dir, endpoint_a2dp);

    /* Create the endpoint */
    if ((ep = STREAM_NEW_ENDPOINT(a2dp, key, dir, con_id)) == NULL)
    {
        return NULL;
    }

    /* Wrap the port up as a cbuffer, if we can't do that then fail here */
    if(SOURCE == dir)
    {
        if (NULL == (ep->state.a2dp.source_buf =
                cbuffer_wrap_bc_port(BUF_DESC_MMU_BUFFER_HW_WR, params[PORT_NUM])))
        {
            stream_destroy_endpoint(ep);
            return NULL;
        }
        /* configure port for 16-bit, byte swap and no sign extension */
        cbuffer_reconfigure_read_port(params[PORT_NUM],
                    BITMODE_16BIT | BYTESWAP_MASK | NOSIGNEXT_MASK ); 
    }
    else
    {
        if (NULL == (ep->state.a2dp.sink_buf =
                cbuffer_wrap_bc_port(BUF_DESC_MMU_BUFFER_HW_RD, params[PORT_NUM])))
        {
            stream_destroy_endpoint(ep);
            return NULL;
        }
        /* configure port for 16-bit, byte swap and no sign extension */
        cbuffer_reconfigure_write_port(params[PORT_NUM], BITMODE_16BIT |  BYTESWAP_MASK ); 
    }

    ep->state.a2dp.portid = params[PORT_NUM];
    ep->can_be_closed = TRUE;
    ep->can_be_destroyed = TRUE;
    ep->is_real = TRUE;

#ifdef INSTALL_SHUNT
    ep->state.shunt.cid = params[1];
#endif /* INSTALL_SHUNT */


    return ep;
}


/****************************************************************************
Private Function Definitions
*/
static bool a2dp_close(ENDPOINT *endpoint)
{
    /* By the time we reach this function we will have stopped everything from
     * running, so all we need to do is tidy up the buffer for the port.
     */
    if (SOURCE == endpoint->direction)
    {
        cbuffer_destroy(endpoint->state.a2dp.source_buf);
    }
    else
    {
        cbuffer_destroy(endpoint->state.a2dp.sink_buf);
    }
    return TRUE;
}

static void a2dp_get_timing(ENDPOINT *ep, ENDPOINT_TIMING_INFORMATION *time_info)
{
    time_info->period = A2DP_KICK_PERIOD;
    a2dp_get_timing_common(ep, time_info);
    }

/**
 * \brief Performs a port buffer copy. It simply tries to copy as much data as
 * possible to/from the port. This might turn out to be too simple.
 *
 * \param endpoint pointer to the endpoint that received a kick
 * \param kick_dir kick direction
 */
static void a2dp_kick(ENDPOINT *endpoint, ENDPOINT_KICK_DIRECTION kick_dir)
{
    unsigned new_data = 0;

    /* If the kick didn't come from our own interrupt then we don't care. */
    if (kick_dir != STREAM_KICK_INTERNAL)
    {
        return;
    }

    /* If this is a source endpoint we may have read a single octet from the
     * port last time, in which case we need to read another one to get the
     * buffer back in sync before doing further block copies.
     */
    if (endpoint->state.a2dp.read_an_octet)
    {
        /* If there is some new date read the octet and let cbuffer_copy do the
         * rest. If there isn't any data wait until there is some. */
        if (1 <= port_calc_amount_data_octets(endpoint->state.a2dp.portid))
        {
            endpoint->state.a2dp.read_an_octet = FALSE;
            /* There is space to write the extra octet as we wrote one already
             * so no need to check, just write it and reconfigure the port to
             * 16bit mode */
            port_copy_an_octet(endpoint->state.a2dp.source_buf,
                                       endpoint->state.a2dp.sink_buf, FALSE);

            /* configure port for 16-bit, byte swap and no sign extension */
            cbuffer_reconfigure_read_port(endpoint->state.a2dp.portid,
                                          BITMODE_16BIT | BYTESWAP_MASK | NOSIGNEXT_MASK ); 

            /* We've copied data in indicate it in new_data in case cbuffer_copy
             * copied nothing. */
            new_data = 1;
        }
        else
        {
            /* There was no data so return */
            return;
        }
    }
    stream_debug_buffer_levels(endpoint);
    /* Copy as much as we can into/out of the port */
    new_data += cbuffer_copy(endpoint->state.a2dp.sink_buf,
                            endpoint->state.a2dp.source_buf,
                            cbuffer_get_size_in_words(endpoint->state.a2dp.sink_buf));
    stream_debug_buffer_levels(endpoint);
    if (SOURCE == endpoint->direction)
    {
        /* There may be a lone octet to copy if there is more than 1 then the buffer
         * was full when we did the cbuffer copy so we won't waste effort. */
        if (1 == port_calc_amount_data_octets(endpoint->state.a2dp.portid))
        {
            if (cbuffer_calc_amount_space(endpoint->state.a2dp.sink_buf) >= 1)
            {
                endpoint->state.a2dp.read_an_octet = TRUE;
                /* There is space to write the extra octet so we'll do it. We
                 * have to remember that we read an octet and read the other
                 * half into the same word when it arrives otherwise the operator
                 * will see an empty octet in the buffer. */
                cbuffer_reconfigure_read_port(endpoint->state.a2dp.portid,
                                    BITMODE_8BIT | NOSIGNEXT_MASK );      /* (no sign extension), 8-bit */

                port_copy_an_octet(endpoint->state.a2dp.source_buf,
                                           endpoint->state.a2dp.sink_buf, TRUE);
                /* We've copied data in indicate it in new_data in case cbuffer_copy
                 * copied nothing. */
                new_data += 1;
            }
        }
        /* The endpoint's work is finished. Data should really be driving kicks so
         * only propagate a kick if new data turned up. */

        /* Stall state machine */

        if(new_data == 0)
        {
            endpoint->state.a2dp.no_kick_counter++;
        }
        else
        {
            endpoint->state.a2dp.no_kick_counter = 0;
        }

        if (endpoint->state.a2dp.no_kick_counter > NO_KICK_LIMIT)
        {
            endpoint->state.a2dp.stalled = TRUE;
            /*STALLED*/
        }
        else
        {
            endpoint->state.a2dp.stalled = FALSE;
        }

        a2dp_source_kick_common(endpoint);
        }
    return;
}

static void a2dp_sched_kick(ENDPOINT *ep, KICK_OBJECT *ko)
{
    TIME next_fire_time;
    /* This is a periodic timer so ask timers when it was scheduled to fire and
     * use that to schedule the next one. THIS FUNCTION CAN ONLY BE CALLED IN A
     * TIMER INTERRUPT CONTEXT OTHERWISE THE NEXT LINE IS UNSAFE. */
    next_fire_time = time_add(get_last_fire_time(), A2DP_KICK_PERIOD);

    ep->state.a2dp.kick_id = timer_schedule_event_at(next_fire_time,
                                            kick_obj_kick, (void*)ko);
}

static bool a2dp_start(ENDPOINT *ep, KICK_OBJECT *ko)
{
    endpoint_a2dp_state *a2dp = &ep->state.a2dp;
    if (a2dp_start_common(ep))
    {
        a2dp->no_kick_counter = 0;
        a2dp->running = TRUE;
        /* This will fire straight away so all the endpoint state must be configured
         * before this call */
        timer_schedule_event_in_atomic(0, kick_obj_kick, (void*)ko, &a2dp->kick_id);
    }
    return TRUE;
}

static bool a2dp_stop(ENDPOINT *ep)
{
    endpoint_a2dp_state *a2dp = &ep->state.a2dp;
    if (a2dp->running)
    {
        /* A timer interrupt can occur during the process of
         * cancelling the timer. Lock interrupts. Unlock
         * interrupts after cancelling the timer. It is also
         * assumed that host command processing is run at lower
         * priority than a timer interrupt.
         */
    LOCK_INTERRUPTS;
        timer_cancel_event(a2dp->kick_id);
        a2dp->kick_id = TIMER_ID_INVALID;
    UNLOCK_INTERRUPTS;
        a2dp->running = FALSE;
    return TRUE;
}
    else
    {
        return FALSE;
    }
}
