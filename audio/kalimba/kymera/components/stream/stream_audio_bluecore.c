/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \addtogroup Audio Audio endpoint
 * \ingroup endpoints
 * \file  stream_audio_bluecore.c
 *
 * stream audio type file. <br>
 * This file contains stream functions for audio endpoints. <br>
 *
 * \section sec1 Contains:
 * stream_audio_get_endpoint <br>
 */

/****************************************************************************
Include Files
*/

#include "stream_private.h"
#include "stream_endpoint_audio.h" /* For protected access audio ep base class */
#include "io_defs.h"               /* for cbuffer_reconfigure_rd|wr_port */
#include "kalimba_messages.h"
#include "bc_msg.h"
#include "cbops_mgr/cbops_mgr.h"
#include "bluecore_port.h"
#include "opmgr/opmgr_endpoint_override.h"

/****************************************************************************
Private Constant Declarations
*/

enum bc_audio_endpoint_params
{
    BC_AUDIO_PARAMS_PORT = 0,
    BC_AUDIO_PARAMS_UNUSED,
    BC_AUDIO_PARAMS_NUM
};

/****************************************************************************
Private Macro Declarations
*/

/****************************************************************************
Private Variable Definitions
*/

/****************************************************************************
Private Function Declarations
*/

static bool audio_close (ENDPOINT *endpoint);
static bool audio_connect (ENDPOINT *endpoint, tCbuffer *Cbuffer_ptr, ENDPOINT *ep_to_kick, bool* start_on_connect);
static bool audio_disconnect (ENDPOINT *endpoint);
static bool audio_buffer_details (ENDPOINT *endpoint, BUFFER_DETAILS *details);
static void audio_sched_kick (ENDPOINT *endpoint, KICK_OBJECT *ko);
static bool audio_start (ENDPOINT *endpoint, KICK_OBJECT *ko);
static bool audio_stop (ENDPOINT *endpoint);
static bool audio_configure (ENDPOINT *endpoint, unsigned int key, uint32 value);
static bool audio_get_config (ENDPOINT *endpoint, unsigned int key, ENDPOINT_GET_CONFIG_RESULT* result);
static void audio_get_timing (ENDPOINT *endpoint, ENDPOINT_TIMING_INFORMATION *time_info);
static bool audio_set_data_format (ENDPOINT *endpoint, AUDIO_DATA_FORMAT format);

DEFINE_ENDPOINT_FUNCTIONS (audio_functions, audio_close, audio_connect,
                           audio_disconnect, audio_buffer_details,
                           audio_kick, audio_sched_kick,
                           audio_start, audio_stop,
                           audio_configure, audio_get_config,
                           audio_get_timing, sync_endpoints);


/****************************************************************************
Public Function Definitions
*/
/****************************************************************************
 *
 * bc_audio_ep_message_handler
 *
 */
void bc_audio_ep_message_handler(unsigned msg_id, unsigned *pdu)
{
    patch_fn_shared(stream_audio);
    switch(msg_id)
    {
        case XAP_KAL_MSG_AUDIO_INFO_RESP:
        {
            ENDPOINT *ep;
            unsigned i;
            ENDPOINT_DIRECTION dir = SOURCE;
            unsigned port = pdu[0];
            unsigned no_keys = pdu[1];

            if (port >= BC_NUM_PORTS)
            {
                port -= BC_NUM_PORTS;
                dir = SINK;
            }
            ep = stream_endpoint_from_extern_id(
                    stream_get_endpointid_from_portid(port, dir));

#ifdef INSTALL_SPDIF
            /* see if this is for a spdif endpoint */
            if (ep -> stream_endpoint_type == endpoint_spdif)
            {
                bc_spdif_ep_message_handler(msg_id, pdu);                
                return;                
            }
#endif /* #ifdef INSTALL_SPDIF */

            for (i = 0; i < no_keys; i++)
            {
                switch (pdu[2 + (i * 3)])
                {
                    case AUDIO_CONFIG_SAMPLE_RATE:
                    {
                        uint32 sample_rate = ((uint32)(pdu[2 + 1 + (i*3)]) << 16) |
                                                    (uint32)pdu[2 + 2 + (i * 3)];
                        /* Although sample rate doesn't fit in 16 bits it fits
                         * in 24 so downsize it. */
                        ep->state.audio.sample_rate = (unsigned)sample_rate;
                        break;
                    }
                    case AUDIO_CONFIG_LOCALLY_CLOCKED:
                    {
                        /* The bool value will only be in the lowest 16 bits so
                         * we only consider them. */
                        ep->state.audio.locally_clocked = (bool)pdu[2 + 2 + (i * 3)];
                        break;
                    }
                    default:
                        break;
                }
            }

#ifndef UNIT_TEST_BUILD
            /* We should now have enough information
                * to allow the endpoint to be used
                */
            stream_if_ep_creation_complete(ep, TRUE);
#endif

            break;
        }
        default:
        {
            panic_diatribe(PANIC_AUDIO_UNRECOGNISED_AUDIO_MESSAGE, msg_id);
            break;
        }
    }
}

/****************************************************************************
 *
 * stream_audio_get_endpoint
 *
 */
ENDPOINT *stream_audio_get_endpoint(unsigned int con_id,
                                    ENDPOINT_DIRECTION dir,
                                    unsigned int hardware,
                                    unsigned num_params,
                                    unsigned *params,
                                    bool *pending)
{

	if (num_params!=BC_AUDIO_PARAMS_NUM)
	{
		return NULL;
	}

    unsigned int port = params[BC_AUDIO_PARAMS_PORT];

    /* First go and find a stream on the same port, if not found
     * then create a new stream. */
    unsigned key = create_stream_key(hardware, 0, port);
    ENDPOINT *endpoint = stream_get_endpoint_from_key_and_functions(key, dir,
                                                  &endpoint_audio_functions);
    patch_fn_shared(stream_audio);
    if (!endpoint)
    {
        unsigned message[4];
        if ((endpoint = STREAM_NEW_ENDPOINT(audio, key, dir, con_id)) == NULL)
        {
            return NULL;
        }

        /* Wrap the port up as a cbuffer, if we can't do that then fail here */
        if(SOURCE == dir)
        {
            if (NULL == (endpoint->state.audio.source_buf =
                    cbuffer_wrap_bc_port(BUF_DESC_MMU_BUFFER_HW_WR, port)))
            {
                stream_destroy_endpoint(endpoint);
                return NULL;
            }
            cbuffer_reconfigure_read_port(port, BITMODE_16BIT);      // (sign extension), 16-bit
        }
        else
        {
            if (NULL == (endpoint->state.audio.sink_buf =
                    cbuffer_wrap_bc_port(BUF_DESC_MMU_BUFFER_HW_RD, port)))
            {
                stream_destroy_endpoint(endpoint);
                return NULL;
            }
            cbuffer_reconfigure_write_port(port, BITMODE_16BIT);      // (no saturation), 16-bit
        }

        /* All is well */
        endpoint->state.audio.portid = port;
        endpoint->state.audio.kick_id = TIMER_ID_INVALID;
        endpoint->can_be_closed = TRUE;
        endpoint->can_be_destroyed = FALSE;
        /* Audio endpoints are always at the end of a chain */
        endpoint->is_real = TRUE;
        endpoint->state.audio.is_overridden = FALSE;

        /* By default we expect to produce FIXP audio, so activate the necessary cbops */
        if((endpoint->cbops = cbops_mgr_create(endpoint->direction,
            CBOPS_DISCARD | CBOPS_DC_REMOVE | CBOPS_RATEADJUST | CBOPS_SHIFT | CBOPS_UNDERRUN)) == NULL)
        {
            stream_destroy_endpoint(endpoint);
            return NULL;
        }

        /* Initialise endpoint synchronisation values */
        endpoint->state.audio.head_of_sync = endpoint;
        endpoint->state.audio.nep_in_sync = NULL;

        /* We set a default rate for starters, while we wait for the firmware
         * to inform us. In some Kalsim unit tests this will be used as the
         * reply is never sent. */
        endpoint->state.audio.sample_rate = 8000;
        /* For scheduling purposes find out the audio sampling rate */
        message[0] = port + (dir == SOURCE?0:BC_NUM_PORTS);
        message[1] = 2;
        message[2] = AUDIO_CONFIG_SAMPLE_RATE;
        message[3] = AUDIO_CONFIG_LOCALLY_CLOCKED;
        send_non_baton_message(XAP_KAL_MSG_AUDIO_INFO_REQ, 4, message);

/* Unit test builds won't send the audio info response
 * So don't try waiting for it...
 */
#ifndef UNIT_TEST_BUILD
        if (pending)
        {
            /* we need to wait for the audio info response
             * before the client can connect / start this endpoint
             */
            *pending = TRUE;
        }
#endif

    }

    return endpoint;
}


/****************************************************************************
Private Function Definitions
*/
/* ******************************* Helper functions ************************************ */

/* remove_from_synchronisation */
bool remove_from_synchronisation(ENDPOINT *ep)
{
    patch_fn_shared(stream_audio);
    if ( IS_AUDIO_ENDPOINT_SYNCED(ep) )
    {
        ENDPOINT *ep_restore_kick = ep;

        if (IS_ENDPOINT_HEAD_OF_SYNC(ep))
        {
            /* If the head is removed from the list
             * the next endpoint in the list will become the head. */
            ep_restore_kick = ep->state.audio.nep_in_sync;
        }

        remove_from_sync_list(ep);

        /* Restore kicks with the new sync head if it's connected to anything */
        if (ep_restore_kick->connected_to != NULL)
        {
            if (ep_restore_kick->direction == SOURCE)
            {
                set_timing_information_for_real_source(ep_restore_kick);
            }
            else
            {
                set_timing_information_for_real_sink(ep_restore_kick);
            }
        }
    }

    return TRUE;
}

/* add_to_synchronisation */
bool add_to_synchronisation(ENDPOINT *ep1, ENDPOINT *ep2)
{
    KICK_OBJECT *ko;
    ENDPOINT *p_ep;
    patch_fn_shared(stream_audio);

    /* If the two endpoint is the same or they are already synced return TRUE. */
    if ((ep1 == ep2) || (ALREADY_SYNCHRONISED(ep1,ep2)))
    {
        return TRUE;
    }

    /* Cannot synchronise with a running group of endpoints.*/
    if (SYNC_GROUP_IS_RUNNING(ep1) || SYNC_GROUP_IS_RUNNING(ep2))
    {
        return FALSE;
    }

    /* Remove kick objects.
     * For audio endpoints we don't have hard deadlines
     * which means kick_object->ep_kick == kick_object->ep_sched.
     */
    if ( (ko = kick_obj_from_sched_endpoint(ep1)) != NULL)
    {
        kick_obj_destroy(ko);
    }
    if ( (ko = kick_obj_from_sched_endpoint(ep2)) != NULL)
    {
        kick_obj_destroy(ko);
    }

    /* add new endpoint to the synchronisation list */
    add_to_sync_list(ep1,ep2);
    /* On Bluecore these endpoints are already connected and it is highly likely
     * that they are connected to the same operator, so check this. If another
     * endpoint is already connected to the same thing this endpoint doesn't need
     * to kick it too.
     */
    for(p_ep = ep1->state.audio.head_of_sync; p_ep != NULL; p_ep = p_ep->state.audio.nep_in_sync)
    {
        if (p_ep == ep2)
        {
            /* ep2 is added to the end of the list of ep1 hence anything below it in the list
             * has already had any adjustments made to the propagates_kicks flag. */
            break;
        }
        if (stream_is_connected_to_same_entity(p_ep, ep2))
        {
            ep2->ep_to_kick = NULL;
            break;
        }
    }

    if (ep1->direction == SOURCE)
    {
        set_timing_information_for_real_source(ep1);
    }
    else
    {
        set_timing_information_for_real_sink(ep1);
    }

    if (ep2->direction == SOURCE)
    {
        set_timing_information_for_real_source(ep2);
    }
    else
    {
        set_timing_information_for_real_sink(ep2);
    }

    return TRUE;
}

/* ********************************** API functions ************************************* */

/**
 * \brief closes the audio endpoint by requesting the release method on the
 *        audio hardware
 *
 * \param *endpoint pointer to the endpoint that is being closed.
 *
 * \return success or failure
 */
static bool audio_close(ENDPOINT *endpoint)
{
    /* If endpoint is synced, remove it from sync list. This is the only thing that
     * can fail. So do that first. */
    if(!sync_endpoints(endpoint,NULL))
    {
        return FALSE;
    }

    /* By the time we reach this function we will have stopped everything from
     * running, so all we need to do is tidy up the buffer for the port.
     */
    if (SOURCE == endpoint->direction)
    {
        cbuffer_destroy(endpoint->state.audio.source_buf);
    }
    else
    {
        cbuffer_destroy(endpoint->state.audio.sink_buf);
    }

    /* Once the endpoint is disconnected we can also destroy the cbops_mgr. */
    if(endpoint->cbops != NULL)
    {
        /* Endpoint's cbops manager object will be independent of other endpoints since
         * it has been taken of a synchronisation list. So it is either a single slot
         * cbops manager object or a multichannel one with all the other channels marked
         * as not in use, because this is the last endpoint to call "desync".
         * In all cases, it is safe to call destroy.
         */
        cbops_mgr_destroy(endpoint->cbops);
        endpoint->cbops = NULL;
    }
    return TRUE;
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
static bool audio_connect(ENDPOINT *endpoint, tCbuffer *Cbuffer_ptr, ENDPOINT *ep_to_kick, bool* start_on_connect)
{
    /* If we are already running or overridden then don't do anything */
    if(endpoint->state.audio.is_overridden)
    {
        /* No need the to save the buffer and to start the cbops. The endpoint is
         * overriden.*/
        return TRUE;
    }
    /* If the connected endpoint doesn't want kicks then we can save effort so
     * record this fact. */
    endpoint->ep_to_kick = ep_to_kick;

    /* Every ep created single-channel cbops. Now we arrived at the point when the cbops in the chain
       get created and set up, with buffer and alg params hooked in.

     1. If connect() is done before sync for this endpoint having been done, then
        connect is done on just the "original" single-channel cbops,
        adding buffer info etc. as in the old days...

        When later on the sync is done, we consolidate the various single-channel cbops into
        a multi-channel cbops, based on existing single-channel cbops information on buffers - but 
        we only use sync head's information for various alg parameters!

     2. If sync was done before connect (so currently connected EP is found in a sync group, i.e.
        it has non-NULL nep_in_sync or the ptr to sync head is not pointing to itself)
        then populate the already existing multi-channel cbops with the connect information as per above;
        the single-channel cbops in that case was already destroyed for this endpoint and
        its cbop ptr field already points to the multi-channel cbops chain!

     */

    /* There isn't much to do here apart from remember this buffer so that the
     * endpoint can write into/read from it when it gets kicked. */
    if (SOURCE == endpoint->direction)
    {
        endpoint->state.audio.sink_buf = Cbuffer_ptr;

        /* If source already in a sync group, then connect info in multi-channel cbops */
        if((endpoint->state.audio.head_of_sync != endpoint) || (endpoint->state.audio.nep_in_sync != NULL))
        {
            /* This ep has already been added to a sync group, so "plug" its connection info into the multi-channel
             * cbops chain created by the "add to sync" operation, which removed the single-channel cbops originally created for
             * this endpoint.
             */

            /* This just connects channel information up, cbops chain was already created! */
            if (!cbops_mgr_connect_channel(endpoint->cbops, endpoint->state.audio.channel,
                                           endpoint->state.audio.source_buf, endpoint->state.audio.sink_buf))
            {
                cbuffer_destroy(endpoint->state.audio.source_buf);
                endpoint->state.audio.source_buf = NULL;
                return FALSE;
            }
        }
        else
        {
            /* Standalone endpoint, with its single-channel cbops created as in the olde dayes.
             * Using a barely populated common vals struct, the only values currently used by in-chain is
             * rate adjust amount ptr and shift amount (also set inside cbops mgr). In future if vals carries any relevant value for whatever
             * input chain operator, then fill fields here.
             */
            CBOP_VALS vals;
            vals.rate_adjustment_amount = &(endpoint->state.audio.rm_adjust_amount);
            vals.shift_amount = 0;

            if (!cbops_mgr_connect(endpoint->cbops, 1, &endpoint->state.audio.source_buf, &endpoint->state.audio.sink_buf, &vals))
            {
                cbuffer_destroy(endpoint->state.audio.source_buf);
                endpoint->state.audio.source_buf = NULL;
                return FALSE;
            }
        }
    }
    else
    {
        endpoint->state.audio.source_buf = Cbuffer_ptr;

        if((endpoint->state.audio.head_of_sync != endpoint) || (endpoint->state.audio.nep_in_sync != NULL))
        {
            /* This EP has already been added to a sync group, so "plug" its connection info into the multi-channel
             * cbops chain created by the "add to sync" operation, which removed the single-channel cbops originally created for
             * this endpoint.
             */

            /* This just connects channel information up, cbops chain was already created! */
            if (!cbops_mgr_connect_channel(endpoint->cbops, endpoint->state.audio.channel,
                                           endpoint->state.audio.source_buf, endpoint->state.audio.sink_buf))
            {
                cbuffer_destroy(endpoint->state.audio.sink_buf);
                endpoint->state.audio.sink_buf = NULL;
                return FALSE;
            }
        }
        else
        {
            /* It's still a standalone endpoint, so connect as a single-channel cbops chain from scratch as in the olde dayes.
             * Fill in initial parameters cbops will use to communicate between themselves and with the endpoint(s).
             */
            CBOP_VALS vals;
            set_endpoint_cbops_param_vals(endpoint, &vals);

            if (!cbops_mgr_connect(endpoint->cbops, 1, &endpoint->state.audio.source_buf, &endpoint->state.audio.sink_buf, &vals))
            {
                cbuffer_destroy(endpoint->state.audio.sink_buf);
                endpoint->state.audio.sink_buf = NULL;
                return FALSE;
            }
        }
    }
    cbops_mgr_rateadjust_passthrough_mode(endpoint->cbops, TRUE);
    *start_on_connect = FALSE;
    return TRUE;
}

/**
 * \brief Disconnects from an endpoint and stops the data from flowing
 *
 * \param *endpoint pointer to the endpoint that is being disconnected
 *
 * \return success or failure
 */
static bool audio_disconnect(ENDPOINT *endpoint)
{
    /* Forget all about the connected buffer, it's gone away or is about to. If
     * the endpoint hasn't been stopped then this function shouldn't have been
     * called. */
    if (endpoint->state.audio.is_overridden)
    {
        if(endpoint->direction == SINK)
        {
            /* If the sink is the head of the sync group, it silences all the endpoints
             * in the group. Otherwise the endpoint silences itself only.
             */
            if(IS_ENDPOINT_HEAD_OF_SYNC(endpoint))
            {
                ENDPOINT *synced;
                /* At stop the output buffer is filled with silence. This doesn't get
                 * called when we're overriden so ensure it happens here in case the
                 * responsible party didn't do it.
                 */
                for (synced = endpoint; synced != NULL;
                                        synced = synced->state.audio.nep_in_sync)
                {
                    cbuffer_flush_and_fill(synced->state.audio.sink_buf,
                                           get_ep_buffer_zero_value(synced));
                }
            }
            else
            {
                cbuffer_flush_and_fill(endpoint->state.audio.sink_buf,
                                       get_ep_buffer_zero_value(endpoint));
            }
        }
        /*Until disconnect the ep is overridden*/
        endpoint->state.audio.is_overridden = FALSE;
        return TRUE;
    }
    if(endpoint->state.audio.running)
    {
        panic_diatribe(PANIC_AUDIO_UNEXPECTED_ENDPOINT_DISCONNECT,
                stream_external_id_from_endpoint(endpoint));
    }

    if((endpoint->state.audio.head_of_sync != endpoint) || (endpoint->state.audio.nep_in_sync != NULL))
    {
        /* If endpoint in a sync group, then mark the channel as unused in cbops.
         * The endpoint will keep hold of the same channel within the multichannel
         * cbop as the channel number was allocated in the synchronise routine */
        cbops_mgr_set_unused_channel(endpoint->cbops, endpoint->state.audio.channel);
    }
    else
    {
        /* For stand alone endpoints, disconnect the cbops */
        cbops_mgr_disconnect(endpoint->cbops);
    }

    if (SOURCE == endpoint->direction)
    {
        endpoint->state.audio.sink_buf = NULL;
    }
    else
    {
        endpoint->state.audio.source_buf = NULL;
    }

    /* Reset ep_to_kick to it's default state of no endpoint to kick */
    endpoint->ep_to_kick = NULL;

    return TRUE;
}

/**
 * \brief Obtains details of the buffer required for this connection
 *
 * \param endpoint pointer to the endpoint from which the buffer
 *        information is required
 * \param details pointer to the BUFFER_DETAILS structure to be populated.
 *
 * \return TRUE/FALSE success or failure
 *
 */
static bool audio_buffer_details(ENDPOINT *endpoint, BUFFER_DETAILS *details)
{
    if (endpoint == NULL || details == NULL)
    {
        return FALSE;
    }

    details->supplies_buffer = endpoint->state.audio.is_overridden;

    if (details->supplies_buffer)
    {
        if (endpoint->direction == SOURCE)
        {
            details->b.buffer = endpoint->state.audio.source_buf;
        }
        else
        {
            details->b.buffer = endpoint->state.audio.sink_buf;
        }
    }
    else
    {
        /*
         *  The buffer on the XAP side of the port is 256 words although the audio
         *  module only requests 128 words so there isn't really any point this
         *  endpoint asking for anything any bigger. The hydra endpoint requests the
         *  same size so this keeps behaviours similar.
         */
        details->b.buff_params.size = get_audio_buffer_length(endpoint->state.audio.sample_rate,
                                                              endpoint->direction, FALSE);
        /* All the fancy stuff happens in the port so we just want a SW buffer to
         * work with. */
        details->b.buff_params.flags = BUF_DESC_SW_BUFFER;
    }

    details->runs_in_place = FALSE;
    details->can_override = TRUE;
    details->wants_override = FALSE;

    return TRUE;
}



/**
 * \brief Starts a kick interrupt source based off this audio endpoint.
 *
 * \param ep pointer to the endpoint which is responsible for scheduling
 * the kick.
 * \param ko pointer to the KICK_OBJECT that received an interrupt and called
 * this function.
 *
 * \return TRUE/FALSE success or failure
 */
static bool audio_start (ENDPOINT *ep, KICK_OBJECT *ko)
{
    /* If we are already running or overridden then don't do anything */
    if(ep->state.audio.running || ep->state.audio.is_overridden)
    {
        return TRUE;
    }

    /* Initialise rate adjustment-related parameters */
    ep->state.audio.rm_diff = 0;
    ep->state.audio.rm_sample_cnt = 0;
    ep->state.audio.rm_kick_cnt = 0;
    ep->state.audio.rm_starting = TRUE;

    if(IS_ENDPOINT_HEAD_OF_SYNC(ep))
    {
        ENDPOINT *synced;
        unsigned buf_offset;
        /* Initialise the rate matching counters for the sync head only */
        ep->state.audio.rm_curr_rate = 1 << RM_FIX_POINT_SHIFT;
        ep->state.audio.rm_kick_frac = 0;
        ep->state.audio.acc_samples = 0;
        ep->state.audio.acc_kick_time = 0;

        /* Mark the endpoint running before we create a kick timer, just in case it
         * fires before returning. */
        ep->state.audio.running = TRUE;

        /* Use an explicit timer based on the kick period. */
        PL_PRINT_P1(TR_STREAM, "stream_IS_AUDIO_ENDPOINT_start: starting kick timer in %d us\n",
                        STREAM_KICK_PERIOD_TO_USECS(ep->state.audio.kick_period));

        /* Schedule the kick timer for this chain. Starting the interrupt
         * source first so that an interrupt here doesn't erode the priming
         * level.*/
         timer_schedule_event_in_atomic(
                    STREAM_KICK_PERIOD_TO_USECS(ep->state.audio.kick_period),
                    kick_obj_kick, (void*)ko, &ep->state.audio.kick_id);

        ep->state.audio.sync_started = FALSE;

        /* Sync read and write pointers, at the port side - source buf for source, sink buf for sink endpoint,
         * and wipe the other buffer - just to start clean. Deduce zero value from data format.
         */
        if(ep->direction == SOURCE)
        {
            /* The output buffer feeding downstream component to have its rd/wr pointers sync'ed so
             * it looks empty to downstream component at this point in case anybody kicks it before this endpoint
             * really produces first lump of data. Move the pointers that the endpoint owns.
             */
            cbuffer_move_write_to_read_point(ep->state.audio.sink_buf, 0);
            cbuffer_move_read_to_write_point(ep->state.audio.source_buf, CBOP_MIN_HEADROOM_SAMPLES);

            /* Re-init the cbops with the Cbuffers as they are at this moment! Otherwise they would carry on
             * from where they left off (if there was a previous stop, then start of the endpoint).
             */
            cbops_mgr_buffer_reinit(ep->cbops);

            /* Bring all the synchronised buffers/ports into alignment */
            buf_offset = cbuffer_get_port_read_offset(ep->state.audio.source_buf, NULL);

            for (synced = ep->state.audio.nep_in_sync; synced != NULL;
                                    synced = synced->state.audio.nep_in_sync)
            {
                cbuffer_move_write_to_read_point(synced->state.audio.sink_buf, 0);
                cbuffer_set_port_read_offset(synced->state.audio.source_buf, buf_offset);

                /* Re-init the cbops with the Cbuffers as they are at this moment! Otherwise they would carry on
                 * from where they left off (if there was a previous stop, then start of the endpoint).
                 */
                cbops_mgr_buffer_reinit(synced->cbops);

                synced->state.audio.sync_started = FALSE;
            }
            ep->state.audio.rm_buff_offset = cbuffer_get_port_write_offset(ep->state.audio.source_buf, NULL);
        }
        else
        {
            /* Calculate a cbop headroom amount based on sampling rate and a
             * default processing time allowance. An extra headroom is added in
             * case of a rate missmatch to prevent underruns. */
            unsigned initial_amount = ep->state.audio.block_size +
                                      AUDIO_RM_HEADROOM_AMOUNT +
                                      (STREAM_KICK_PERIOD_FROM_USECS(CBOP_PROCESSING_TIME_ALLOWANCE_IN_USECS) /
                                       ((unsigned int)((unsigned long)STREAM_KICK_PERIOD_FROM_USECS(SECOND) /
                                       ep->state.audio.sample_rate)));

            /* The real sink's input buffer to look empty, first time underrun handling will create silence,
             * output buffer to have its rd/wr pointers set up, too. Move the pointers that the endpoint owns.
             */
            cbuffer_move_read_to_write_point(ep->state.audio.source_buf, 0);
            cbuffer_flush_and_fill(ep->state.audio.sink_buf, get_ep_buffer_zero_value(ep));
            cbuffer_move_write_to_read_point(ep->state.audio.sink_buf, initial_amount);
            cbops_mgr_buffer_reinit(ep->cbops);

            /* Bring all the synchronised buffers/ports into alignment */
            buf_offset = cbuffer_get_port_write_offset(ep->state.audio.sink_buf, NULL);

            for (synced = ep->state.audio.nep_in_sync; synced != NULL;
                                    synced = synced->state.audio.nep_in_sync)
            {
                cbuffer_move_read_to_write_point(synced->state.audio.source_buf, 0);
                cbuffer_flush_and_fill(synced->state.audio.sink_buf, get_ep_buffer_zero_value(synced));
                cbuffer_set_port_write_offset(synced->state.audio.sink_buf, buf_offset);

                /* Re-init the cbops with the Cbuffers as they are at this moment! Otherwise they would carry on
                 * from where they left off (if there was a previous stop, then start of the endpoint).
                 */
                cbops_mgr_buffer_reinit(synced->cbops);

                synced->state.audio.sync_started = FALSE;
            }

            ep->state.audio.rm_buff_offset = cbuffer_get_port_read_offset(ep->state.audio.sink_buf, NULL);
        }

    }

    return TRUE;
}

/**
 * \brief Stops the kick interrupt source that was started by this endpoint.
 *
 * \param ep Pointer to the endpoint this function is being called on.
 *
 * \return TRUE/FALSE success or failure
 */
static bool audio_stop (ENDPOINT *ep)
{
    if (ep->state.audio.is_overridden)
    {
        /*Until disconnect the ep is overridden*/
        return TRUE;
    }
    if(!ep->state.audio.running)
    {
        /* The kick source is already stopped */
        return FALSE;
    }

    /* kick_id will be different than TIMER_ID_INVALID only for the head of sync.
     * Because of this reason is not needed to check if the endpoint is the head.*/

    /* This needs to be atomic, to make sure the right timer gets cancelled */
    LOCK_INTERRUPTS;
    if(TIMER_ID_INVALID != ep->state.audio.kick_id)
    {
        /* This was the last transform in the sync list. Cancel any timer */
        timer_cancel_event(ep->state.audio.kick_id);
        ep->state.audio.kick_id = TIMER_ID_INVALID;
    }
    UNLOCK_INTERRUPTS;

    if(ep->direction == SINK)
    {
        ENDPOINT *synced;
        /* Leave behind a pristine buffer if this is a sink - so only silence is pumped out.
         * The pointers are not bothered with, HW can free-run and start will snap the pointers
         * to proper places as per priming.
         */
        for (synced = ep; synced != NULL; synced = synced->state.audio.nep_in_sync)
        {
            cbuffer_flush_and_fill(synced->state.audio.sink_buf, get_ep_buffer_zero_value(synced));
        }
    }

    ep->state.audio.running = FALSE;

    return TRUE;
}

/**
 * \brief Performs a fast HW divide rather than letting C promote the calculation
 * to a slow SW assisted double precision divide.
 *
 * This function does a double precision/single precision divide, inline
 * asm only accepts single precision parameters, fortunately as the numerator is
 * shifted up by a constant amount this can be coded in the asm.
 *
 * This function also assumes that the double precision value will not use the
 * highest bit in the numerator so no carrying is required in the calculation.
 *
 * \param num value to use for the numerator which will be shifted up by RM_FIX_POINT_SHIFT
 * \param denom value to use for the denominator
 *
 * \return The result of the division.
 */
#ifndef __GNUC__
asm int fast_div_with_shift(int num, int denom)
{
    @[    .scratch numerator
          .scratch shift
          .restrict numerator:large_rmac<rMAC>
          .restrict shift:bank1
          /* Because we know we're not going to use a multiply instruction with any
           * of the input parameters, it's safe to let the compiler choose any
           * bank 1 register, including rMAC. This lets the compiler make better
           * decisions about which registers to allocate for use in the inline
           * assembly function. This prevents the compiler doing superfluous
           * register juggling.
           */
          .restrict num:bank1_with_rmac, denom:bank1_with_rmac
     ]
    /* Can't do a 56bit shift using rMAC so using a multiply instruction instead.
     * Because the multiply operation will shift the result left by 1 bit (see
     * Kalimba manual for more details) then reduce the effective shift amount
     * by 1 to compensate. */
    @{shift} = 1 << (RM_FIX_POINT_SHIFT - 1);
    @{numerator} = @{num} * @{shift};
    Div = @{numerator} / @{denom};
    @{} = DivResult;
}
#else /* __GNUC__ */
#define fast_div_with_shift(num, denom) (int)((int48)(num << RM_FIX_POINT_SHIFT)/(denom))
#endif /* __GNUC__ */

/**
 * \brief Calculates the current audio hardware rate seen by the endpoint
 *
 * \param ep Pointer to the endpoint to calculate the rate of
 */

#define RM_CONVERGENCE_FACTOR 1

static void calc_audio_rate(ENDPOINT *ep)
{
    int val, num, diff, exp;
    patch_fn_shared(stream_audio);

    if (ep->state.audio.locally_clocked)
    {
        /* If the endpoint is locally clocked there is nothing to compensate
         * for so do nothing. */
        return;
    }

    if ((0 == ep->state.audio.rm_sample_cnt) || (ep->state.audio.rm_kick_cnt < RM_MIN_KICKS_BEFORE_ADJUST))
    {
        /* There isn't enough information to improve
         * on the last measurement so wait until next time. */
        return;
    }

    /* Compare the measured sample rate with the expected sample rate and report
     * the ratio.
     * The inverse of the rate required to be reported to the ratematchmgr is
     * calculated here as then we can multiply by the rate in the sched_kick
     * function and do an arithmetic shift rather than a slow divide. This does
     * mean that this has to be inverted subsequently in get_audio_rate().
     */
    exp = ep->state.audio.rm_kick_cnt * ep->state.audio.block_size;
    num = RM_CONVERGENCE_FACTOR * ep->state.audio.sample_rate;
    diff = ep->state.audio.rm_sample_cnt - exp;
    val = fast_div_with_shift(num, num + diff);

    if (val != 1 << RM_FIX_POINT_SHIFT)
    {
        ENDPOINT* sync;

        ep->state.audio.rm_curr_rate = (int)(((int48)val * ep->state.audio.rm_curr_rate) >> RM_FIX_POINT_SHIFT);

        /* Limit the min / max calculated rate in case something has gone wrong
         * Don't let it get to more than 1.5 or less than half the nominal rate.
         */
        if (ep->state.audio.rm_curr_rate > (3 << (RM_FIX_POINT_SHIFT - 1)))
        {
            ep->state.audio.rm_curr_rate = 3 << (RM_FIX_POINT_SHIFT - 1);
        }
        if (ep->state.audio.rm_curr_rate < (1 << (RM_FIX_POINT_SHIFT - 1)))
        {
            ep->state.audio.rm_curr_rate = 1 << (RM_FIX_POINT_SHIFT - 1);
        }

        /* Maintain the output buffer level by adjusting the number of samples
         * we copy next kick to compensate for those that we lost.
         * Update rate matching difference for every sink in the sync group.
         * We can nonchalantly iterate from the ep onwards in the list, as this function is
         * only called by audio_sched_kick, which in turn is only done for the
         * sync group head.
         */
        sync = ep;
        while(sync != NULL)
        {
            sync->state.audio.rm_diff += -diff;
            sync = sync->state.audio.nep_in_sync;
        }
    }

    ep->state.audio.rm_sample_cnt = 0;
    ep->state.audio.rm_kick_cnt = 0;
}

unsigned get_rm_data(ENDPOINT *endpoint)
{
    endpoint_audio_state *audio = &endpoint->state.audio;
    unsigned buf_size, new_offset, new_adj_offset;
    unsigned delta_samples = 0;
    patch_fn_shared(stream_audio);

    if(endpoint->connected_to != NULL)
    {
        if (endpoint->direction == SINK)
        {
            new_offset = cbuffer_get_port_read_offset(endpoint->state.audio.sink_buf, &buf_size);
        }
        else
        {
            new_offset = cbuffer_get_port_write_offset(endpoint->state.audio.source_buf, &buf_size);
        }
        if (new_offset < endpoint->state.audio.rm_buff_offset)
        {
            new_adj_offset = new_offset + buf_size;
        }
        else
        {
            new_adj_offset = new_offset;
        }
        delta_samples = new_adj_offset - endpoint->state.audio.rm_buff_offset;
        audio->rm_buff_offset = new_offset;
        audio->rm_sample_cnt += delta_samples;
        audio->rm_kick_cnt++;
        /* If our rate looks to be off and we are in a startup condition 
         * then as long as we have sufficient data to make a valid assessment 
         * then force an update to the measured rate.
         */
        if (delta_samples != endpoint->state.audio.block_size)
        {
            if (endpoint->state.audio.rm_starting)
            {
                if (endpoint->state.audio.rm_kick_cnt > RM_MIN_KICKS_BEFORE_ADJUST)
                {
                    calc_audio_rate(endpoint);
                }
            }
        }
    }
    return delta_samples;
}

/**
 * Process data collected by get_rm_data
 */
void process_rm_data(ENDPOINT *endpoint,
                     unsigned num_cbops_read, unsigned num_cbops_written)
{
}

/**
 * \brief Performs a fast HW divide rather than letting C promote the calculation
 * to a slow SW assisted double precision divide to work out the next kick time.
 *
 * This function does a double precision/single precision divide, inline
 * asm only accepts single precision parameters, fortunately as the numerator is
 * multiplied by a constant amount this can be coded in the asm.
 *
 * This function also assumes that the double precision value will not use the
 * highest bit in the numerator so no carrying is required in the calculation.
 *
 * \param samples number of samples that have accumulated
 * \param sample_rate the hardware sample rate
 *
 * \return The new kick time
 */
#ifndef __GNUC__
asm unsigned calc_new_kick(unsigned samples, unsigned sample_rate)
{
    @[    .scratch numerator
          .restrict numerator:large_rmac<rMAC>
          /* Because we know we're not going to use a multiply instruction with any
           * of the input parameters, it's safe to let the compiler choose any
           * bank 1 register, including rMAC. This lets the compiler make better
           * decisions about which registers to allocate for use in the inline
           * assembly function. This prevents the compiler doing superfluous
           * register juggling.
           */
          .restrict samples:bank1_with_rmac, sample_rate:bank1_with_rmac
    ]
    /* Can't use the C definition of Second here as it's got type casts in it.
     * The UU multiply needs left shifting by -1 afterwards. We skip this step
     * by multiplying by SECOND/2 so the multiply yields the desired number in
     * the first place. */
    @{numerator} = @{samples} * (1000000 / 2) (UU);
    Div = @{numerator} / @{sample_rate};
    @{} = DivResult;
}
#else /* __GNUC__ */
#define calc_new_kick(samples, sample_rate) (unsigned int)((((unsigned long)samples) * SECOND) / sample_rate)
#endif /* __GNUC__ */

/**
 * \brief Schedules the next kick for the chain based upon the audio data
 *
 * \param ep pointer to the endpoint this function is called on.
 * \param ko pointer to the kick object that called this function. This is used
 * for rescheduling rather than caching the value.
 */
static void audio_sched_kick (ENDPOINT *ep, KICK_OBJECT *ko)
{
    TIME next_fire_time;
    unsigned int acc_kick_new, kp_delta;
    uint48 kp_delta_num;

    /* Calculate the new kick time based on the number of samples we should have accumulated */
    ep->state.audio.acc_samples += ep->state.audio.block_size;

    acc_kick_new = calc_new_kick(ep->state.audio.acc_samples, ep->state.audio.sample_rate);

    /* compensate the delta given the curently measured rate */
    kp_delta_num = ((uint48)(acc_kick_new - ep->state.audio.acc_kick_time) *
                        ep->state.audio.rm_curr_rate) + ep->state.audio.rm_kick_frac;
    ep->state.audio.rm_kick_frac = kp_delta_num & ((1 << RM_FIX_POINT_SHIFT) - 1);

    kp_delta = (unsigned int)(kp_delta_num >> RM_FIX_POINT_SHIFT);
    ep->state.audio.acc_kick_time = acc_kick_new;

    /* Limit accumulated sample count and kick times to 1s */
    if (ep->state.audio.acc_samples >= ep->state.audio.sample_rate)
    {
        ep->state.audio.acc_samples -= ep->state.audio.sample_rate;
        ep->state.audio.acc_kick_time -= SECOND;
        /* If this endpoint is a slave then calculate the rate. */
        calc_audio_rate(ep);
        ep->state.audio.rm_starting = FALSE;
    }

    /* This is a periodic timer so ask timers when it was scheduled to fire and
     * use that to schedule the next one. */
    next_fire_time = time_add(get_last_fire_time(), kp_delta);

    ep->state.audio.kick_id = timer_schedule_event_at(next_fire_time,
            kick_obj_kick, (void*)ko);
}

/**
 *
 */
static void adjust_audio_rate(ENDPOINT *ep, int32 adjust_val)
{
    /* For the sake of reducing the MIPS burden no safety checks just do it. */
    L3_DBG_MSG2("Audio adjustment : %d EP: %06X", (int)(adjust_val), (uintptr_t)ep);

    ep->state.audio.rm_adjust_amount = ((int)adjust_val);
    return;
}

/*
 * \brief configure an audio endpoint with a key and value pair
 *
 * \param *endpoint pointer to the endpoint to be configured
 * \param key denoting what is being configured
 * \param value value to which the key is to be configured
 *
 * \return Whether the request succeeded.
 *
 */
static bool audio_configure(ENDPOINT *endpoint, unsigned int key, uint32 value)
{
    switch(key)
    {
    case EP_DATA_FORMAT:
        return audio_set_data_format(endpoint, (AUDIO_DATA_FORMAT)value);

    case EP_BLOCK_SIZE:
        /* Use block size and sample rate to calculate kick period */
        {
            endpoint->state.audio.block_size = (unsigned int)value;
            endpoint->state.audio.kick_period =  (unsigned int)(value * (unsigned long)STREAM_KICK_PERIOD_FROM_USECS(1000000UL) /
                        endpoint->state.audio.sample_rate);
        }
        return TRUE;

    case EP_OVERRIDE_ENDPOINT:
        /* Set the logical value of override flag. */
        endpoint->state.audio.is_overridden = (bool)value;
        /* Depending on the direction of the endpoint, the source/sink bufffer is
         * already set from the endpoint creation. */
        return TRUE;

    case EP_CBOPS_PARAMETERS:
    {
        bool retval;
        CBOPS_PARAMETERS *parameters = (CBOPS_PARAMETERS *)(uintptr_t) value;

        if (parameters)
        {
            /* cbops_mgr should not be updated when endpoint is running. */
            if (!endpoint->is_enabled &&  endpoint->cbops && \
                opmgr_override_pass_cbops_parameters(parameters, endpoint->cbops,
                          endpoint->state.audio.source_buf, endpoint->state.audio.sink_buf))
            {
                retval = TRUE;
            }
            else
            {
                retval = FALSE;
            }
        }
        else
        {
            /* Panic can return in unit test*/
            retval = FALSE;
            panic_diatribe(PANIC_AUDIO_STREAM_INVALID_CONFIGURE_KEY,
                                                            endpoint->id);
        }

        free_cbops_parameters(parameters);
        return retval;
    }

    case EP_SET_INPUT_GAIN:
    case EP_SET_OUTPUT_GAIN:
    {
        /*
         * TODO: The expected behaviour is to send a message to the XAP (let's say
         * XAP_KAL_MSG_AUDIO_CONFIGURE) to configure the gain of the endpoint.
         * See the audio API CS-209064-MM Codec input/output gain.
        unsigned message[4];
        unsigned int gain = kymera_to_bluecore_gain(value, key == EP_SET_OUTPUT_GAIN);
        message[0] = endpoint->state.audio.portid + (endpoint->direction == SOURCE?0:BC_NUM_PORTS);
        The following relies on the fact that our enum and the BlueCore
        firmware's have the same meaning.
        FIXME: BlueCore firmware stream_audio_configure() doesn't actually allow
               the DSP to control this yet:
        message[1] = (key == EP_SET_INPUT_GAIN) ? STREAM_CONFIG_KEY_CODEC_INPUT_GAIN
                                                : STREAM_CONFIG_KEY_CODEC_OUTPUT_GAIN;
        message[2] = (uint16)gain;
        message[3] = 0;
        return send_non_baton_message(XAP_KAL_MSG_AUDIO_CONFIGURE, 4, message);*/

        return FALSE;
    }


    case EP_RATEMATCH_ADJUSTMENT:
        adjust_audio_rate(endpoint, (int32)value);
        return TRUE;
    case EP_RATEMATCH_ENACTING:
        return audio_configure_rm_enacting(endpoint, value);
    default:
        return FALSE;
    }
}

/**
 * \brief Calculates the current rate of the audio hardware that is being measured
 *
 * \param ep The endpoint to calculate the hardware rate of
 *
 * \return The normalised value of the measured rate in FIXP format.
 */
static int get_audio_rate(ENDPOINT *ep)
{
    patch_fn_shared(stream_audio);
    int val;
    if (ep->state.audio.kick_id == TIMER_ID_INVALID)
    {
        /* If the endpoint isn't running there is nothing to compensate for so
         * indicate that the rate is perfect. */
        return 1 << STREAM_RATEMATCHING_FIX_POINT_SHIFT;
    }

    /* The value stored in rm_curr_rate is inverted to speed up the sched_kick calculation
     * so invert it here to report to the ratematching_mgr */
    val = (int)((int48)((uint48)1ul << (2 * RM_FIX_POINT_SHIFT)) / ep->state.audio.rm_curr_rate);

    return (val << (STREAM_RATEMATCHING_FIX_POINT_SHIFT - RM_FIX_POINT_SHIFT));
}


/*
 * \brief get audio endpoint configuration
 *
 * \param *endpoint pointer to the endpoint to be configured
 * \param key denoting what is being configured
 * \param value pointer to a value which is populated with the current value
 *
 * \return Whether the request succeeded.
 *
 */
static bool audio_get_config(ENDPOINT *endpoint, unsigned int key, ENDPOINT_GET_CONFIG_RESULT* result)
{
    switch(key)
    {
    case EP_DATA_FORMAT:
        result->u.value = (uint32)audio_get_data_format(endpoint);
        return TRUE;

    case STREAM_INFO_KEY_AUDIO_SAMPLE_RATE:
        /* Get the sampling rate of the endpoint.*/
        result->u.value =  endpoint->state.audio.sample_rate;
        return TRUE;

    case EP_RATEMATCH_ABILITY:
        return audio_get_config_rm_ability(endpoint, &result->u.value);
    case EP_RATEMATCH_RATE:
        result->u.value = get_audio_rate(endpoint);
        L3_DBG_MSG2("Audio rate : %d EP: %06X", (int)(result->u.value), (uintptr_t)endpoint);
        return TRUE;
    case EP_RATEMATCH_MEASUREMENT:
        result->u.rm_meas.sp_deviation =
                STREAM_RATEMATCHING_RATE_TO_FRAC(get_audio_rate(endpoint));
        result->u.rm_meas.measurement.valid = FALSE;
        return TRUE;

    case EP_CBOPS_PARAMETERS:
    {
        CBOPS_PARAMETERS *parameters;
        unsigned cbops_flags = cbops_get_flags(endpoint->cbops);
        parameters = create_cbops_parameters(cbops_flags, EMPTY_FLAG);

        if (!parameters)
        {
            return FALSE;
        }

        if (cbops_flags & CBOPS_SHIFT)
        {
            if(endpoint->direction == SOURCE)
            {
                if (!cbops_parameters_set_shift_amount(parameters, 8))
                {
                    return FALSE;
                }
            }
            else
            {
                if (!cbops_parameters_set_shift_amount(parameters, -8))
                {
                    return FALSE;
                }
            }
        }
        result->u.value = (uint32)(uintptr_t) parameters;
        return TRUE;
    }

    default:
        return FALSE;
    }
}

/**
 * \brief Get the timing requirements of this audio endpoint
 *
 * \param endpoint pointer to the endpoint to get the timing info for
 * \param time_info a pointer to an ENDPOINT_TIMING_INFORMATION structure to
 * populate with the endpoint's timing information
 */
static void audio_get_timing (ENDPOINT *endpoint, ENDPOINT_TIMING_INFORMATION *time_info)
{
#ifdef DESKTOP_TEST_BUILD
    /* For unit tests, just return the value set by the test */
    extern unsigned audio_sink_rate, audio_source_rate;
    if (SOURCE == endpoint->direction)
    {
        time_info->period = (unsigned int)((unsigned long)STREAM_KICK_PERIOD_FROM_USECS(1000000UL) / audio_source_rate);
    }
    else
    {
        time_info->period = (unsigned int)((unsigned long)STREAM_KICK_PERIOD_FROM_USECS(1000000UL) / audio_sink_rate);
    }
#else
    /* an audio endpoint only is concerned with the frequency */
    time_info->period = (unsigned int)((unsigned long)STREAM_KICK_PERIOD_FROM_USECS(1000000UL) /
                        endpoint->state.audio.sample_rate);
#endif
    time_info->block_size = 1;
    time_info->has_deadline = FALSE;
    time_info->locally_clocked = endpoint->state.audio.locally_clocked;
    /* Effort is only done on this endpoint's timer interrupt so no kicks are
     * desired from the rest of the chain. */
    time_info->wants_kicks = FALSE;

    return;
}

/*
 * audio_get_data_format
 */
AUDIO_DATA_FORMAT audio_get_data_format (ENDPOINT *endpoint)
{
    /* If we've got a shift and a dc remove cbop then we're producing/consuming
     * FIXP data */
    if ((cbops_get_flags(endpoint->cbops) & (CBOPS_DC_REMOVE | CBOPS_SHIFT)) ==
                                                (CBOPS_DC_REMOVE | CBOPS_SHIFT))
    {
        return AUDIO_DATA_FORMAT_FIXP;
    }

    return AUDIO_DATA_FORMAT_16_BIT;
}

/**
 * \brief set the audio data format that the endpoint will place in/consume from
 * the buffer
 *
 * \param endpoint pointer to the endpoint to set the data format of.
 * \param format AUDIO_DATA_FORMAT requested to be produced/consumed by the endpoint
 *
 * \return whether the set operation was successful
 */
static bool audio_set_data_format (ENDPOINT *endpoint, AUDIO_DATA_FORMAT format)
{
    bool status = FALSE;
    patch_fn_shared(stream_audio);

    /* The data format can only be set before connect */
    if (NULL != endpoint->connected_to)
    {
        return status;
    }
    /* If the format is already set to the value requested do nothing. */
    if (format == audio_get_data_format(endpoint))
    {
        return TRUE;
    }

    tCbuffer **in_buffs, **out_buffs;
    unsigned nr_chans;
    bool head_setup = TRUE;

    if(!cbops_mgr_alloc_buffer_info(endpoint->cbops, &nr_chans, &in_buffs, &out_buffs))
    {
        return status;
    }

    if(nr_chans > 0)
    {
        /* Snapshot cbuffer into at this point in the cbops. It may return fail if number of channels
         * is out of kilter, but we just created things properly so it can't whine.
         */
        cbops_mgr_get_buffer_info(endpoint->cbops, nr_chans, in_buffs, out_buffs);

        /* The head's cbuffer info may have changed since it was last seen in cbops down under...
         * Hook in the buffer info before re-creation of chain happens, for bullet proofing against
         * lifecycle phase-related mishaps.
         */
        cbops_mgr_connect_channel(endpoint->cbops, endpoint->state.audio.channel,
                                  endpoint->state.audio.source_buf, endpoint->state.audio.sink_buf);
    }
    else
    {
        /* Even section head hasn't yet been set up - this is therefore a set format before chain gets properly
         * created. Hence it can't belong to an existing sync group, therefore chain is single channel chain
         * for this endpoint in question.
         */
        nr_chans = 1;
        head_setup = FALSE;
        in_buffs = &endpoint->state.audio.source_buf;
        out_buffs = &endpoint->state.audio.sink_buf;
    }

    switch (format)
    {
        case AUDIO_DATA_FORMAT_FIXP:
        {
            if (!endpoint->is_enabled)
            {
                /* cbops_mgr should not be updated when endpoint is running. */
                CBOP_VALS vals;

                /* Ensure that what gets "recreated" is having the sync group head's parameters!
                 * Of course, on some platforms may not have yet sync'ed, in which case it takes its own params.
                 * Add to sync list operation will later anyway set things up with sync head driving the show.
                 */
                set_endpoint_cbops_param_vals(endpoint->state.audio.head_of_sync , &vals);

                vals.rate_adjustment_amount = &(endpoint->state.audio.rm_adjust_amount);

                if(endpoint->direction == SOURCE)
                {
                    vals.shift_amount = 8;
                }
                else
                {
                    vals.shift_amount = -8;
                }

                status = cbops_mgr_append(endpoint->cbops, CBOPS_DC_REMOVE | CBOPS_SHIFT | CBOPS_RATEADJUST, nr_chans,
                                        in_buffs, out_buffs, &vals, FALSE);
                /* Start rateadjust in passthrough mode */
                cbops_mgr_rateadjust_passthrough_mode(endpoint->cbops, TRUE);
            }

            break;
        }
        case AUDIO_DATA_FORMAT_16_BIT:
        {
            if (!endpoint->is_enabled)
            {
                /* cbops_mgr should not be updated when endpoint is running. */
                /* DC_REMOVE only works on FIXP data so it's not applied to 16 bit */
                status = cbops_mgr_remove(endpoint->cbops, CBOPS_DC_REMOVE | CBOPS_SHIFT | CBOPS_RATEADJUST, nr_chans,
                                        in_buffs, out_buffs, FALSE);
            }
            break;
        }

        default:
            break;
    }

    if(head_setup)
    {
        cbops_mgr_free_buffer_info(in_buffs, out_buffs);
    }

    return status;
}


/* Set cbops values */
void set_endpoint_cbops_param_vals(ENDPOINT* ep, CBOP_VALS *vals)
{
    patch_fn_shared(stream_audio);
    vals->data_block_size_ptr = &(ep->state.audio.latency_ctrl_info.data_block);

    /* The silence counter is to become the single sync group counter (when synced) */
    vals->total_inserts_ptr = &(ep->state.audio.latency_ctrl_info.silence_samples);

    /* The rm_diff is to become the single sync group rm_diff (when synced) */
    vals->rm_diff_ptr = &(ep->state.audio.rm_diff);

    vals->rate_adjustment_amount = &(ep->state.audio.rm_adjust_amount);

    /* TODO: using silence insertion only for now */
    vals->insertion_vals_ptr = NULL;

    /* Delta samples - it is used by certain cbops and at endpoint level it is 
     * on BC this is calculated at endpoint level */
    vals->delta_samples_ptr = &ep->state.audio.delta_samples;

    /* Endpoint block size (that equates to the endpoint kick period's data amount).
     * This "arrives" later on, and is owned, possibly updated, by endpoint only.
     */
    vals->block_size_ptr = (unsigned*)&(ep->state.audio.block_size);

    vals->rm_headroom = AUDIO_RM_HEADROOM_AMOUNT;

    vals->sync_started_ptr = &ep->state.audio.sync_started;
}
