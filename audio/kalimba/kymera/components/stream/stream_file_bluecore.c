/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \addtogroup File File endpoint
 * \ingroup endpoints
 * \file  stream_file_bluecore.c
 *
 * stream file type file. <br>
 * This file contains stream functions for bluecore file endpoints. <br>
 *
 */

/****************************************************************************
Include Files
*/
#include "stream_private.h"
#include "cbops_mgr/cbops_mgr.h"
#include "opmgr/opmgr_endpoint_override.h"

/****************************************************************************
Private Type Declarations
*/

/****************************************************************************
Private Constant Declarations
*/
#define PARAM_PORT_OFFSET 0

#define FILE_BUFFER_MIN_SIZE 256

#define FILE_KICK_PERIOD    (2 * MILLISECOND)

/****************************************************************************
Private Macro Declarations
*/

/****************************************************************************
Private Variable Definitions
*/

/****************************************************************************
Private Function Declarations
*/
static bool file_close (ENDPOINT *ep);
static bool file_connect (ENDPOINT *ep, tCbuffer *Cbuffer_ptr, ENDPOINT *ep_to_kick, bool* start_on_connect);
static bool file_disconnect (ENDPOINT *ep);
static bool file_buffer_details (ENDPOINT *ep, BUFFER_DETAILS *details);
static void file_sched_kick (ENDPOINT *ep, KICK_OBJECT *ko);
static void file_kick(ENDPOINT *ep, ENDPOINT_KICK_DIRECTION kick_dir);
static bool file_start (ENDPOINT *ep, KICK_OBJECT *ko);
static bool file_stop (ENDPOINT *ep);
static bool file_configure (ENDPOINT *ep, unsigned int key, uint32 value);
static bool file_get_config (ENDPOINT *ep, unsigned int key, ENDPOINT_GET_CONFIG_RESULT* result);
static void file_get_timing (ENDPOINT *ep, ENDPOINT_TIMING_INFORMATION *time_info);
static bool file_set_data_format (ENDPOINT *ep, AUDIO_DATA_FORMAT format);

DEFINE_ENDPOINT_FUNCTIONS (file_functions, file_close, file_connect,
                           file_disconnect, file_buffer_details,
                           file_kick, file_sched_kick,
                           file_start, file_stop,
                           file_configure, file_get_config,
                           file_get_timing, stream_sync_sids_dummy);

/****************************************************************************
Public Function Definitions
*/
/****************************************************************************
 *
 * stream_file_get_endpoint
 *
 */
ENDPOINT *stream_file_get_endpoint(unsigned int con_id,
                                    ENDPOINT_DIRECTION dir,
                                    unsigned num_params,
                                    unsigned *params)
{
    /* First go and find a stream on the same port, if not found
     * then create a new stream. */
    unsigned port, key;
    ENDPOINT *ep;

    if (NULL == params)
    {
        return NULL;
    }

    key = port = params[PARAM_PORT_OFFSET];

    ep = stream_get_endpoint_from_key_and_functions(key, dir,
                                                  &endpoint_file_functions);
    if (!ep)
    {
        if ((ep = STREAM_NEW_ENDPOINT(file, key, dir, con_id)) == NULL)
        {
            return NULL;
        }

        /* Wrap the port up as a cbuffer, if we can't do that then fail here */
        if(SOURCE == dir)
        {
            if (NULL == (ep->state.file.source_buf =
                    cbuffer_wrap_bc_port(BUF_DESC_MMU_BUFFER_HW_WR, port)))
            {
                stream_destroy_endpoint(ep);
                return NULL;
            }
            cbuffer_reconfigure_read_port(port, BITMODE_16BIT);      // (sign extension), 16-bit
        }
        else
        {
            if (NULL == (ep->state.file.sink_buf =
                    cbuffer_wrap_bc_port(BUF_DESC_MMU_BUFFER_HW_RD, port)))
            {
                stream_destroy_endpoint(ep);
                return NULL;
            }
            cbuffer_reconfigure_write_port(port, BITMODE_16BIT);      // (no saturation), 16-bit
        }

        /* All is well */
        ep->state.file.portid = port;
        ep->state.file.kick_id = TIMER_ID_INVALID;
        /* Start with the default kick period */
        ep->state.file.kick_period = FILE_KICK_PERIOD;
        ep->can_be_closed = TRUE;
        ep->can_be_destroyed = FALSE;
        /* File endpoints are always at the end of a chain */
        ep->is_real = TRUE;

        /* By default we expect to produce FIXP audio, so activate the necessary cbops */
        if((ep->cbops = cbops_mgr_create(ep->direction, CBOPS_SHIFT)) == NULL)
        {
            stream_destroy_endpoint(ep);
            return NULL;
        }
    }

    return ep;
}

/**
 * \brief closes the file ep
 *
 * \param *ep pointer to the endpoint that is being closed.
 *
 * \return success or failure
 */
static bool file_close(ENDPOINT *ep)
{
    /* Once the endpoint is disconnected we can also destroy
     * the cbops_mgr. */
    if (!cbops_mgr_destroy(ep->cbops))
    {
        return FALSE;
    }

    /* cbops was successfully destroyed so delete the reference. */
    ep->cbops =NULL;

    /* By the time we reach this function we will have stopped everything from
     * running, so all we need to do is tidy up the buffer for the port.
     */
    if (SOURCE == ep->direction)
    {
        cbuffer_destroy(ep->state.file.source_buf);
    }
    else
    {
        cbuffer_destroy(ep->state.file.sink_buf);
    }

    return TRUE;
}

/**
 * \brief Connect to the endpoint.
 *
 * \param *ep pointer to the endpoint that is being connected
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
static bool file_connect(ENDPOINT *ep, tCbuffer *Cbuffer_ptr, ENDPOINT *ep_to_kick, bool* start_on_connect)
{
    ep->ep_to_kick = ep_to_kick;
    /* There isn't much to do here apart from remember this buffer so that the
     * endpoint can write into/read from it when it gets kicked. */
    if (SOURCE == ep->direction)
    {
        ep->state.file.sink_buf = Cbuffer_ptr;
    }
    else
    {
        ep->state.file.source_buf = Cbuffer_ptr;
    }
    *start_on_connect = FALSE;
    /* create cbops chain */
    return cbops_mgr_connect(ep->cbops, 1, &ep->state.file.source_buf, &ep->state.file.sink_buf, NULL);
}

/**
 * \brief Disconnects from an ep and stops the data from flowing
 *
 * \param *ep pointer to the endpoint that is being disconnected
 *
 * \return success or failure
 */
static bool file_disconnect(ENDPOINT *ep)
{
    /* Forget all about the connected buffer, it's gone away or is about to. If
     * the endpoint hasn't been stopped then this function shouldn't have been
     * called. */
    cbops_mgr_disconnect(ep->cbops);

    if (SOURCE == ep->direction)
    {
        ep->state.file.sink_buf = NULL;
    }
    else
    {
        ep->state.file.source_buf = NULL;
    }

    /* Return to default kick period in case reconnected and not given a new
     * value. */
    ep->state.file.kick_period = FILE_KICK_PERIOD;

    return TRUE;
}

/**
 * \brief Obtains details of the buffer required for this connection
 *
 * \param ep pointer to the endpoint from which the buffer
 *        information is required
 * \param details pointer to the BUFFER_DETAILS structure to be populated.
 *
 * \return TRUE/FALSE success or failure
 *
 */
static bool file_buffer_details(ENDPOINT *ep, BUFFER_DETAILS *details)
{
    if (ep == NULL || details == NULL)
    {
        return FALSE;
    }

    details->supplies_buffer = FALSE;
    details->runs_in_place = FALSE;

    details->b.buff_params.size = FILE_BUFFER_MIN_SIZE;
    /* All the fancy stuff happens in the port so we just want a SW buffer to
     * work with. */
    details->b.buff_params.flags = BUF_DESC_SW_BUFFER;

    details->can_override = FALSE;
    details->wants_override = FALSE;

    return TRUE;
}

/**
 * \brief Starts a writing or reading to/from a file endpoint.
 *
 * \param ep pointer to the endpoint which is responsible for scheduling
 * the kick.
 * \param ko pointer to the KICK_OBJECT that will call the kick function.
 *
 * \return TRUE/FALSE success or failure
 */
static bool file_start (ENDPOINT *ep, KICK_OBJECT *ko)
{
    /* If not already running copy all the available data and setup a timer*/
    if (ep->state.file.kick_id == TIMER_ID_INVALID)
    {
        unsigned buffer_size;
        ep->is_enabled = TRUE;
        if (ep->direction == SOURCE)
        {
            buffer_size = cbuffer_get_size_in_words(ep->state.file.sink_buf);
        }
        else
        {
            buffer_size = cbuffer_get_size_in_words(ep->state.file.source_buf);
        }
        cbops_mgr_process_data(ep->cbops, buffer_size);

        timer_schedule_event_in_atomic(ep->state.file.kick_period,
                                       kick_obj_kick, (void*)ko, &ep->state.file.kick_id);
    }
    return TRUE;
}

/**
 * \brief Stops the file endpoint from reading/writing from/to a file.
 *
 * \param ep Pointer to the endpoint this function is being called on.
 *
 * \return TRUE/FALSE success or failure
 */
static bool file_stop (ENDPOINT *ep)
{
    if (FALSE == ep->is_enabled)
    {
        return FALSE;
    }
    /* This needs to be atomic, to make sure the right timer gets cancelled */
    LOCK_INTERRUPTS;
    if(TIMER_ID_INVALID != ep->state.file.kick_id)
    {
        /* This was the last transform in the sync list. Cancel any timer */
        timer_cancel_event(ep->state.file.kick_id);
        ep->state.file.kick_id = TIMER_ID_INVALID;
    }
    UNLOCK_INTERRUPTS;
    ep->is_enabled = FALSE;
    return TRUE;
}

/**
 * \brief Schedules the next kick for the file endpoint
 *
 * \param ep pointer to the endpoint this function is called on.
 * \param ko pointer to the kick object that called this function. This is used
 * for rescheduling rather than caching the value.
 */
static void file_sched_kick (ENDPOINT *ep, KICK_OBJECT *ko)
{
    /* The source needs to pull data in so has interrupt driven kicks. The
     * sink just pushes data as and when it turns up so a timer is only
     * scheduled for a SOURCE. */
    if (ep->direction == SOURCE)
    {
        TIME next_fire_time = time_add(get_last_fire_time(), ep->state.file.kick_period);

        ep->state.file.kick_id = timer_schedule_event_at(next_fire_time,
                kick_obj_kick, (void*)ko);
    }
}

static void file_kick(ENDPOINT *ep, ENDPOINT_KICK_DIRECTION kick_dir)
{
    unsigned buffer_size;
    /* Execute the cbops chain, it will copy as much as we can
     * into/out of the port and execute the necessary cbops_operation. */
    if (ep->direction == SOURCE)
    {
        buffer_size = cbuffer_get_size_in_words(ep->state.file.sink_buf);
    }
    else
    {
        buffer_size = cbuffer_get_size_in_words(ep->state.file.source_buf);
    }
    cbops_mgr_process_data(ep->cbops, buffer_size);

    /* As long as something was written propagate a kick up or down stream */
    if (cbops_mgr_amount_written(ep->cbops) != 0)
    {
        propagate_kick(ep, kick_dir);
    }
    else
    {
        /* If this is a SOURCE and we can't copy because there is already a full
         * buffer down stream then we should propagate a kick. It's probably
         * only a benefit when the kick source is KICK_INTERNAL, but if this
         * kick isn't internal then the thing downstream should have just
         * removed something from the buffer so amount_written would be non-zero.
         * So no effort is wasted here checking the source of the interrupt.
         */
        if (SOURCE == ep->direction && 0 != cbuffer_calc_amount_data(ep->state.file.sink_buf))
        {
            propagate_kick(ep, kick_dir);
        }
    }
}

/**
 * \brief Get the timing requirements of this file endpoint
 *
 * \param ep pointer to the endpoint to get the timing info for
 * \param time_info a pointer to an ENDPOINT_TIMING_INFORMATION structure to
 * populate with the endpoint's timing information
 */
static void file_get_timing (ENDPOINT *ep, ENDPOINT_TIMING_INFORMATION *time_info)
{
    /* an audio endpoint only is concerned with the frequency */
    time_info->period = 0;

    /* File endpoints wants kicks because it is driven by the chain.
     * TODO The current implementation uses timer which is not the standard kicking
     * behaviour therefore it will be changed in B-212308.*/
    time_info->wants_kicks = TRUE;

    time_info->block_size = 1;
    time_info->has_deadline = FALSE;
    /* We should ask the XAP, for now assume worst case that we are slave */
    time_info->locally_clocked = TRUE;

    return;
}


/**
 * \brief set the file data format that the endpoint will place in/consume from
 * the buffer
 *
 * \param endpoint pointer to the endpoint to set the data format of.
 * \param format AUDIO_DATA_FORMAT requested to be produced/consumed by the endpoint
 *
 * \return whether the set operation was successful
 */
static bool file_set_data_format (ENDPOINT *ep, AUDIO_DATA_FORMAT format)
{
    /* The data format can only be set before connect */
    if (NULL != ep->connected_to)
    {
        return FALSE;
    }

    if (ep->is_enabled)
    {
        /* cbops_mgr should not be updated when endpoint is running. */
        return FALSE;
    }

    switch (format)
    {
    case AUDIO_DATA_FORMAT_FIXP:
        {
            CBOP_VALS vals;
            vals.shift_amount = 0;
            return cbops_mgr_append(ep->cbops, CBOPS_SHIFT, 1,
                    &ep->state.file.source_buf, &ep->state.file.sink_buf, &vals, FALSE);
        }
    case AUDIO_DATA_FORMAT_16_BIT:
        /* DC_REMOVE only works on FIXP data so it's not applied to 16 bit */
        return cbops_mgr_remove(ep->cbops, CBOPS_SHIFT, 1,
                &ep->state.file.source_buf, &ep->state.file.sink_buf, FALSE);
    default:
        return FALSE;
    }
}

/*
 * \brief configure an file endpoint with a key and value pair
 *
 * \param *ep pointer to the endpoint to be configured
 * \param key denoting what is being configured
 * \param value value to which the key is to be configured
 *
 * \return Whether the request succeeded.
 */
static bool file_configure(ENDPOINT *ep, unsigned int key, uint32 value)
{
    switch(key)
    {
    case EP_DATA_FORMAT:
        return file_set_data_format(ep, (AUDIO_DATA_FORMAT)value);

    case EP_KICK_PERIOD:
    {
        /* Use block size and sample rate to calculate kick period */
        ep->state.file.kick_period =
                        STREAM_KICK_PERIOD_TO_USECS((unsigned int)value);
        return TRUE;
    }
    case EP_CBOPS_PARAMETERS:
    {
        /* Cbops parameter passing is not supported be SCO.
         * Free the cbops_parameters to avoid any potential memory leak and fail. */
        free_cbops_parameters((CBOPS_PARAMETERS *)(uintptr_t) value);
        return FALSE;
    }

    default:
        return FALSE;
    }
}

/*
 * file_get_data_format
 */
static AUDIO_DATA_FORMAT file_get_data_format (ENDPOINT *endpoint)
{
    /* If we've got a shift and a dc remove cbop then we're producing/consuming
     * FIXP data */
    if ((cbops_get_flags(endpoint->cbops) & CBOPS_SHIFT) == CBOPS_SHIFT)
    {
        return AUDIO_DATA_FORMAT_FIXP;
    }

    return AUDIO_DATA_FORMAT_16_BIT;
}

/*
 * \brief get file endpoint configuration
 *
 * \param *ep pointer to the endpoint to be configured
 * \param key denoting what is being configured
 * \param value pointer to a value which is populated with the current value
 *
 * \return Whether the request succeeded.
 */
static bool file_get_config(ENDPOINT *ep, unsigned int key, ENDPOINT_GET_CONFIG_RESULT* result)
{
    switch(key)
    {
    case EP_DATA_FORMAT:
        result->u.value = (uint32)file_get_data_format(ep);
        return TRUE;

    case EP_RATEMATCH_ABILITY:
        result->u.value = (uint32)RATEMATCHING_SUPPORT_AUTO;
        return TRUE;
    case EP_RATEMATCH_RATE:
        /* File runs as fast or as slow as needed so always report perfect */
        result->u.value = RM_PERFECT_RATE;
        return TRUE;
    case EP_RATEMATCH_MEASUREMENT:
        /* File runs as fast or as slow as needed so always report perfect */
        result->u.rm_meas.sp_deviation = 0;
        result->u.rm_meas.measurement.valid = FALSE;
        return TRUE;

    default:
        return FALSE;
    }
}

