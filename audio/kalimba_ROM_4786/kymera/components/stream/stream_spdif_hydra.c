/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
 ****************************************************************************/
/**
 * \addtogroup spdif endpoint
 * \ingroup endpoints
 * \file  stream_spdif_hydra.c
 *
 * stream spdif type file. <br>
 * This file contains stream functions for spdif endpoints. <br>
 *
 * \section sec1 Contains:
 * stream_spdif_get_endpoint <br>
 */

/****************************************************************************
Include Files
*/
#define HYDRA_SPDIF_BUFFER_SIZE  288
#define SPDIF_RX_BLOCK_SIZE_SIZE 192
#define HYDRA_SPDIF_KICK_PERIOD_US  1000
#define SPDIF_TYPICAL_SAMPLE_RATE 48000
#include "stream_private.h"
#include "stream_endpoint_audio.h" /* For protected access to audio ep base class */
#include "opmgr/opmgr_endpoint_override.h"
#include "buffer.h"
#include "stream_endpoint_spdif.h"
#include "stream_for_hal_audio_spdif.h"
#include "pl_assert.h"

/* parameters used when getting the endpoint */
enum hydra_spdif_endpoint_params
{
    HYDRA_SPDIF_PARAMS_INSTANCE = 0,
    HYDRA_SPDIF_PARAMS_CHANNEL,
    HYDRA_SPDIF_PARAMS_NUM
};

/****************************************************************************
Private Function Declarations
*/
static bool spdif_close (ENDPOINT *endpoint);
static bool spdif_connect (ENDPOINT *endpoint, tCbuffer *Cbuffer_ptr,  ENDPOINT *ep_to_kick, bool* start_on_connect);
static bool spdif_disconnect (ENDPOINT *endpoint);
static bool spdif_buffer_details (ENDPOINT *endpoint, BUFFER_DETAILS *details);
static void spdif_kick(ENDPOINT *endpoint, ENDPOINT_KICK_DIRECTION kick_dir);
static void spdif_sched_kick (ENDPOINT *endpoint, KICK_OBJECT *ko);
static bool spdif_start (ENDPOINT *endpoint, KICK_OBJECT *ko);
static bool spdif_stop (ENDPOINT *endpoint);
static bool spdif_configure (ENDPOINT *endpoint, unsigned int key, uint32 value);
static bool spdif_get_config (ENDPOINT *endpoint, unsigned int key, ENDPOINT_GET_CONFIG_RESULT* result);
static void spdif_get_timing (ENDPOINT *endpoint, ENDPOINT_TIMING_INFORMATION *time_info);
static bool spdif_sync_endpoints (ENDPOINT *ep1, ENDPOINT *ep2);
static void stream_spdif_complete_endpoint(ENDPOINT *endpoint);
static void spdif_switch_to_rate_monitor_kick(ENDPOINT *ep, KICK_OBJECT *ko);
static void spdif_switch_to_timer_event_kick(ENDPOINT *ep,  KICK_OBJECT *ko);
static void spdif_enable_rate_monitor(ENDPOINT *ep, KICK_OBJECT *ko, bool enable);
static void spdif_enable_time_event(ENDPOINT *ep, KICK_OBJECT *ko, bool enable);
static bool stream_spdif_chsts_forward_call_cback(unsigned con_id, unsigned status,
                                                  unsigned op_id, unsigned num_resp_params, unsigned *resp_params);
static int spdif_get_rate(ENDPOINT* endpoint);

DEFINE_ENDPOINT_FUNCTIONS (spdif_functions, spdif_close, spdif_connect,
                           spdif_disconnect, spdif_buffer_details,
                           spdif_kick, spdif_sched_kick,
                           spdif_start, spdif_stop,
                           spdif_configure, spdif_get_config,
                           spdif_get_timing, spdif_sync_endpoints);

/****************************************************************************
Private Macro Declarations
*/

/* default supported rates for blue core chips */
#define SPDIF_DEFAULT_SUPPORTED_RATES (SPSRM_48KHZ|SPSRM_44K1HZ|SPSRM_32KHZ|SPSRM_96KHZ|SPSRM_88K2HZ|SPSRM_192KHZ|SPSRM_176K4HZ)

#define SPDIF_RATE_WAITING_BLOCK_START_MASK (1<<23)

/****************************************************************************
Private Variable Definitions
*/
/* define all supported rates,
 * TODO: move this to flash
 */
const unsigned spdif_sample_rates[] = {
    48000,
    44100,
    32000,
    96000,
    88200,
    192000,
    176400
};

/****************************************************************************
Public Function Definitions
*/

/**
 * hydra_spdif_ep_event_handler
 *
 * \brief handle events raised by spdif hal
 *
 * This function is used to handle the following messages received
 *  from spdif hal firmware:
 *
 *  SPDIF_EVENT_RATE_CHANGE:
 *     Whenever hal firmware detects any rate change it will send this event
 *     to the endpoint. When the rate is a valid rate the endpoint will use the
 *     rate monitor events to read the input data (the threshold is set to half of
 *     a full SPDIF block, ie. 96 samples/channel. When the rate is invalid, we use
 *     timer event if we want to generate silence during pause time.
 *
 *     When two channel config is used, there will be a single even for both endpoints.
 *
 *  SPDIF_EVENT_CHSTS_CHANGE:
 *     Informs the endpoint about change in channel status bits.
 *
 *  SPDIF_EVENT_BLOCK_START:
 *    It tells the endpoint that the first S/PDIF block has started. Currently like in bluecore
 *    we ignore this event as endpoint double checks the validity of input.
 *
 * \param event determines type of event
 * \param instance the spdif instance that event happened on
 * \param channel channel number within the instance
 * \param message_length length of the message
 * \param message payload of the message
 *
 */
void hydra_spdif_ep_event_handler(SPDIF_RX_EVENT event,
                                  unsigned instance,
                                  unsigned channel,
                                  unsigned message_length,
                                  const void *message)
{
    unsigned key;
    ENDPOINT *ep;

    patch_fn_shared(stream_spdif_hydra);

    /*
     * search for A or AB endpoints, the function channel argument
     * might not always represent the actual channel order of the
     * endpoint, for example when channel status is received it will
     * be for the actual physical channel even if interleaved config
     * is used.
     */
    key = create_stream_key(STREAM_DEVICE_SPDIF, instance, SPCO_CHANNEL_A);
    ep = stream_get_endpoint_from_key_and_functions(key, SOURCE, &endpoint_spdif_functions);
    if(NULL == ep)
    {
        /* if A not found search for AB endpoint */
        key = create_stream_key(STREAM_DEVICE_SPDIF, instance, SPCO_CHANNEL_AB);
        ep = stream_get_endpoint_from_key_and_functions(key, SOURCE, &endpoint_spdif_functions);
        if(NULL == ep)
        {
            /* TODO: panic or fault at least */
            return;
        }
    }

    /* Corresponding endpoint found */
    switch (event)
    {
        /* Rate event */
        case SPDIF_EVENT_RATE_CHANGE:
        {
            unsigned new_sample_rate;
            unsigned prev_sample_rate;

            /* check the length just to be in the safe side */
            if(message_length != 1)
            {
                panic_spdif_invalid_param(SIRP_SPDIF_EP_BAD_MESSAGE, message_length);
            }

            new_sample_rate = ((const unsigned *) message)[0];
            prev_sample_rate = (ep->state.spdif.extra->fw_sample_rate)&(~SPDIF_RATE_WAITING_BLOCK_START_MASK);

            if(prev_sample_rate != new_sample_rate)
            {
                /* A change in sample rate has occurred,
                 * store the sample rate detected by firmware
                 */
                ep->state.spdif.extra->fw_sample_rate = new_sample_rate;

                if(new_sample_rate != 0)
                {
                    /* if the rate is valid, we also need to wait for block start to
                     * make sure the rate is really valid */
                    ep->state.spdif.extra->fw_sample_rate |= SPDIF_RATE_WAITING_BLOCK_START_MASK;
                }

                else if(ep->state.spdif.running && !ep->state.spdif.is_overridden)
                {
                    /* The endpoint is running, but the input is
                     * invalid, we use timer event to handle pause
                     * and to generate silence if required
                     */
                    ep->state.spdif.extra->fw_sample_rate = 0;
                    if(TIMER_ID_INVALID == ep->state.spdif.kick_id)
                    {
                        ep->state.spdif.extra->fw_sample_rate_changed = TRUE;
                        spdif_switch_to_timer_event_kick(ep, NULL);
                    }
                }
            }
        }
        break;

        /* Channel status event */
        case SPDIF_EVENT_CHSTS_CHANGE:
        {

            const unsigned *msg = (const unsigned *) message;
            unsigned *chsts;
            unsigned chsts_changed = 0;
            int i;
            unsigned tmp;

            /* check the length just to be in the safe side */
            if(message_length != SPDIF_NOROF_CHSTS_WORDS)
            {
                panic_spdif_invalid_param(SIRP_SPDIF_EP_BAD_MESSAGE, message_length);
            }

            if(channel == SPCO_CHANNEL_A)
            {
                /* message is for A channel */
                chsts = &ep->state.spdif.extra->channel_status[0];
            }
            else if (channel == SPCO_CHANNEL_B)
            {
                /* message is for B channel */
                chsts = &ep->state.spdif.extra->channel_status[SPDIF_NOROF_CHSTS_WORDS];
            }
            else
            {
                /* channel status shall be only for A or B */
                panic_spdif_invalid_param(SIPR_NOT_MATCHING_CHANNEL_ORDER, channel);
            }

            /* copy channel status and see if
             * it has changed since last time
             */
            for(i=0; i<SPDIF_NOROF_CHSTS_WORDS; ++i)
            {
                tmp = msg[i] & 0xFFFF; /* channel status words are 16-bit in all archs */
                if(tmp != chsts[i])
                    chsts_changed |= (1 << i);
                chsts[i] = tmp;
            }

            /* What we receive here isn't synchronised to audio blocks,
             * For professional format, we only report if changes is seen
             * in first 3 words.
             */
            if(chsts[0] & 0x1)
            {
                /* it's professional format */
                chsts_changed &= 0x7;
            }


            if(chsts_changed &&
               (NULL != ep->connected_to) &&
               !ep->state.spdif.is_overridden)
            {

                /* the client is informed via the operator only
                 * so the channel status is sent to the operator
                 */
                unsigned chsts_msg[2+SPDIF_NOROF_CHSTS_WORDS];
                chsts_msg[0] =  OPMSG_SPDIF_DECODE_ID_NEW_CHSTS_FROM_EP;
                chsts_msg[1] =  channel;
                memcpy(&chsts_msg[2], chsts, SPDIF_NOROF_CHSTS_WORDS*sizeof(unsigned));
                opmgr_operator_message(RESPOND_TO_OBPM,
                                       ep->connected_to->id, sizeof(chsts_msg)/sizeof(unsigned),
                                       chsts_msg, stream_spdif_chsts_forward_call_cback);
            }
        }
        break;

        /* Block Start Event */
        case SPDIF_EVENT_BLOCK_START:
        {
            /* Block start has received, the rate is flagged to be fully valid */
            ep->state.spdif.extra->fw_sample_rate &= ~SPDIF_RATE_WAITING_BLOCK_START_MASK;
            if(ep->state.spdif.extra->fw_sample_rate != 0)
            {
                /* the endpoint is running, and the stream
                 * is expected to be active, we use rate monitor
                 * event for reading the input
                 */
                if(ep->state.spdif.running && !ep->state.spdif.is_overridden)
                {
                    if(!ep->state.spdif.monitor_enabled)
                    {
                        spdif_switch_to_rate_monitor_kick(ep, NULL);
                    }
                }

                ep->state.spdif.extra->fw_sample_rate_changed = TRUE;
            }

        }
        break;

        default:
            break;
    }
}

/**
 * stream_spdif_complete_endpoint
 * helper function to complete the creation of spdif endpoint
 */
static void stream_spdif_complete_endpoint(ENDPOINT *endpoint)
{
    struct endpoint_spdif_state *state = &endpoint->state.spdif;

    patch_fn_shared(stream_spdif_hydra);

    /* see if there is L/R endpoint */
    if(state->channel_order != SPCO_CHANNEL_AB)
    {
        unsigned key;
        ENDPOINT *ep;

        PL_ASSERT(state->channel_order == SPCO_CHANNEL_A || state->channel_order == SPCO_CHANNEL_B);

        /* create a key for the twin end point */
        if(state->channel_order == SPCO_CHANNEL_A)
        {
            /* for A channel, create a key for corresponding B channel in the same instance */
            key = create_stream_key(STREAM_DEVICE_SPDIF, state->instance, SPCO_CHANNEL_B);
        }

        else
        {
            /* for B channel, create a  for corresponds A channel in the same instance */
            key = create_stream_key(STREAM_DEVICE_SPDIF, state->instance, SPCO_CHANNEL_A);
        }

        /* see if a twin endpoint exists */
        ep = stream_get_endpoint_from_key_and_functions(key, SOURCE, &endpoint_spdif_functions);
        if(NULL != ep)
        {
            /* group the two endpoints */
            endpoint->state.spdif.twin_endpoint = ep;
            ep->state.spdif.twin_endpoint = endpoint;
        }

    }
}

/**
 * stream_spdif_chsts_forward_call_cback
 *
 * \brief call back for sending channel status messages to OP
 */
static bool stream_spdif_chsts_forward_call_cback(unsigned con_id, unsigned status,
                                                  unsigned op_id, unsigned num_resp_params, unsigned *resp_params)
{
    /* ignore everything and return TRUE */
    UNUSED(con_id);
    UNUSED(status);
    UNUSED(op_id);
    UNUSED(num_resp_params);
    UNUSED(resp_params);
    return TRUE;
}

/**
 * stream_spdif_get_endpoint
 *
 * \brief get spdif-type end point
 *
 * \param con_id connection id
 * \param dir direction, expected to be SOURCE.
 * \param port port number associated with this end point.
 * \param unused unused
 * \param *pending if the function returns success this will mean whether the endpoint creation is yet to get complete.
 *
 * \return the created end point, NULL will be returned if the endpoint cannot be created.
 */
ENDPOINT *stream_spdif_get_endpoint (unsigned int con_id,
                                     ENDPOINT_DIRECTION dir,

                                     unsigned num_params,
                                     unsigned *params,
                                     bool *pending)
{
    int instance;
    int channel;
    unsigned key;
    ENDPOINT *endpoint;

    patch_fn_shared(stream_spdif_hydra);

    /* make sure we have received all required params */
	if (num_params!=HYDRA_SPDIF_PARAMS_NUM)
	{
		return NULL;
	}

    /* get instance number and channel order */
	instance = params[HYDRA_SPDIF_PARAMS_INSTANCE];
	channel  = params[HYDRA_SPDIF_PARAMS_CHANNEL];

    /* check that the channel order isn't a random number */
    if(channel != SPCO_CHANNEL_AB &&
       channel != SPCO_CHANNEL_A &&
       channel != SPCO_CHANNEL_B)
    {
        return NULL;

    }

    /* this is only for S/PDIF rx */
    if(SINK == dir)
    {
        return NULL;
    }

    /* First go and find a stream on the same spdif audio hardware, if not found
     * then create a new stream. */

    key = create_stream_key(STREAM_DEVICE_SPDIF, instance, channel);
    endpoint = stream_get_endpoint_from_key_and_functions(key, dir, &endpoint_spdif_functions);
    if(NULL == endpoint)
    {
        if ((endpoint = STREAM_NEW_ENDPOINT(spdif, key, dir, con_id)) == NULL)
        {
            return NULL;
        }

        endpoint->state.spdif.channel_order = channel;

        /* extra buffer for s/pdif endpoint,
         * B type doesn't need this as it will be
         * the same with its twin A type
         */
        if(channel != SPCO_CHANNEL_B)
        {
            endpoint->state.spdif.extra = xzpnew (struct spdif_extra_states );
            if(endpoint->state.spdif.extra == NULL)
            {
                stream_destroy_endpoint(endpoint);
                return NULL;
            }
            endpoint->state.spdif.extra->nrof_supported_rates = sizeof(spdif_sample_rates)/sizeof(unsigned);
            endpoint->state.spdif.extra->supported_rates = spdif_sample_rates;
            endpoint->state.spdif.extra->supported_rates_mask = SPDIF_DEFAULT_SUPPORTED_RATES;
            endpoint->state.spdif.extra->silence_duration = SPDIF_UNLIMITED_SILENCE_DURING_PAUSE;
            /* ensure rate is reported as perfect until measured */
            endpoint->state.spdif.extra->norm_rate_ratio = 1 << STREAM_RATEMATCHING_FIX_POINT_SHIFT;
        }
        else
        {
            endpoint->state.spdif.extra = NULL;
        }

        /* All is well */
        endpoint->can_be_closed = TRUE;
        endpoint->can_be_destroyed = FALSE;
        endpoint->is_real = TRUE;
        endpoint->state.spdif.locally_clocked = FALSE;
        endpoint->state.spdif.is_overridden = FALSE;
        endpoint->state.spdif.output_format = SPDIF_INPUT_DATA_FORMAT;
        endpoint->state.spdif.instance = instance;

        /* sample rate hasn't been received/detected yet */
        endpoint->state.spdif.sample_rate = 0;

        /* we don't need a cbops for S/PDIF end_point */
        endpoint->cbops = NULL;

        /* By default we expect to produce FIXP audio, so activate the necessary HW shift and cbops */
        endpoint->state.spdif.shift = DAWTH - 16;

        /* monitor threshold is set to half of a S/PDFI block (96 samples) */
        endpoint->state.spdif.monitor_threshold = SPDIF_RX_BLOCK_SIZE_SIZE/2;
        endpoint->state.spdif.kick_period = HYDRA_SPDIF_KICK_PERIOD_US;
        endpoint->state.spdif.kick_id = TIMER_ID_INVALID;

        endpoint->deferred.config_deferred_kick = TRUE;

        stream_spdif_complete_endpoint(endpoint);
        /* Obtain S/PDIF hardware. NOTE: This should be the last thing in endpoint create since the
         * post create check may complete asynchronously */
        if(!stream_audio_post_create_check(STREAM_DEVICE_SPDIF, instance, channel, dir,
                                           endpoint, pending))
        {
            /* Failed to get everything, give back what we might have asked for */
            endpoint->state.spdif.hw_allocated = FALSE;
            spdif_close(endpoint);
            stream_destroy_endpoint(endpoint);
            return NULL;
        }

        endpoint->state.spdif.hw_allocated = !(*pending);

    }
    return endpoint;
}

/**
 * spdif_enable_rate_monitor
 * \brief enables or disables rate monitor event for this endpoint
 *
 * \param ep pointer to the endpoint
 *
 * \param ko kick object
 *
 * \param enable if TRUE it will enable it else it will disable the rate
 *        monitor interrupt for this endpoint
 *
 */
static void spdif_enable_rate_monitor(ENDPOINT *ep, KICK_OBJECT *ko, bool enable)
{
    patch_fn_shared(stream_spdif_hydra);

    if(enable)
    {
        /* it should be disabled at this point, it's the caller responsibility to check this */
        PL_ASSERT(!ep->state.spdif.monitor_enabled);

        /* Enableg rate monitor event */
        if(stream_spdif_monitor_int_wr_enable(ep, cbuffer_get_write_mmu_handle(ep->state.spdif.source_buf), ko))
        {
            /* synchronise the read and write pointers */
            unsigned new_buf_offset;
            unsigned current_buf_offset;
            int *addr;

            /* get current read offset */
            current_buf_offset = cbuffer_get_read_offset(ep->state.spdif.source_buf);

            /* make the buffer near empty by moving the read offset to
             * near write point */
            cbuffer_move_read_to_write_point(ep->state.spdif.source_buf,
                                             ep->state.spdif.channel_order == SPCO_CHANNEL_AB?
                                             2*CBOP_MIN_HEADROOM_SAMPLES:CBOP_MIN_HEADROOM_SAMPLES);

            /* get the new offset */
            new_buf_offset = cbuffer_get_read_offset(ep->state.spdif.source_buf);

            if(ep->state.spdif.channel_order == SPCO_CHANNEL_AB)
            {
                /* for interleaved mode, we need to make sure
                 * the amount of movement is an even value */
                if((new_buf_offset-current_buf_offset)&1)
                {
                    if(new_buf_offset < current_buf_offset)
                    {
                        new_buf_offset++;
                    }
                    else
                    {
                        new_buf_offset--;
                    }
                }
            }

            /* update the buff read offset */
            addr = ep->state.spdif.source_buf->base_addr + new_buf_offset;
            ep->state.spdif.source_buf->read_ptr = addr;

            if(NULL != ep->state.spdif.twin_endpoint)
            {
                /* use the same offset for its twin endpoint */
                ENDPOINT *ep2 = ep->state.spdif.twin_endpoint;
                addr = ep2->state.spdif.source_buf->base_addr + new_buf_offset;
                ep2->state.spdif.source_buf->read_ptr = addr;
            }

            if(!ep->state.spdif.running)
            {
                /* clear the sink buff (Note: do this only once before ep starts */
                cbuffer_move_write_to_read_point(ep->state.spdif.sink_buf, 0);
                if(NULL != ep->state.spdif.twin_endpoint)
                {
                    /* use the same offset for its twin endpoint */
                    ENDPOINT *ep2 = ep->state.spdif.twin_endpoint;
                    cbuffer_move_write_to_read_point(ep2->state.spdif.sink_buf, 0);
                }
            }
            ep->state.spdif.monitor_enabled = TRUE;
            return;
        }

        else
        {
            /* For some reason it couldn't succeed, this is fatal */
            panic_diatribe(PANIC_AUDIO_STREAM_MONITOR_TOO_FEW_RESOURCES, stream_external_id_from_endpoint(ep));
        }
    }
    else
    {
        /* disable the rate monitor event, it should be enabled at this point */
        if(ep->state.spdif.monitor_enabled)
        {
            if(!stream_monitor_int_wr_disable(ko))
            {
                panic_diatribe(PANIC_AUDIO_STREAM_MONITOR_STOP_FAILED,
                               stream_external_id_from_endpoint(ep));
            }
            ep->state.spdif.monitor_enabled = FALSE;
        }
        else
        {
            /* shouldn't get to here */
            panic_spdif_invalid_param(SIRP_SPDIF_EP_BAD_STATE, ep->state.spdif.monitor_enabled);
        }
    }
}

/**
 * spdif_enable_time_event
 * \brief enables or disables timer event for this endpoint
 *
 * \param ep pointer to the endpoint
 *
 * \param ko kick object
 *
 * \param enable if TRUE it will enable it else it will disable the rate
 *        time event for this endpoint
 *
 */
static void spdif_enable_time_event(ENDPOINT *ep, KICK_OBJECT *ko, bool enable)
{
    patch_fn_shared(stream_spdif_hydra);

    if(enable)
    {
        /* timer should be invalid at this point */
        PL_ASSERT(TIMER_ID_INVALID == ep->state.spdif.kick_id);

        /* Schedule the kick timer for this chain. */
        ep->state.spdif.kick_id = timer_schedule_event_in(
            STREAM_KICK_PERIOD_TO_USECS(ep->state.spdif.kick_period),
            kick_obj_kick, (void*)ko);
    }
    else
    {
        /* timer should be valid at this point */
        PL_ASSERT(TIMER_ID_INVALID != ep->state.spdif.kick_id);

        /* This was the last transform in the sync list. Cancel any timer */
        timer_cancel_event(ep->state.spdif.kick_id);
        ep->state.spdif.kick_id = TIMER_ID_INVALID;
    }
}

/**
 * spdif_switch_to_rate_monitor_kick
 * \brief will make the endpoint to use rate monitor events,
 * this will be suitable when the input stream is valid
 *
 * \param ep pointer to the endpoint
 *
 * \param ko kick object, if null kick object will be taken from the endpoint
 */
static void spdif_switch_to_rate_monitor_kick(ENDPOINT *ep, KICK_OBJECT *ko)
{
    patch_fn_shared(stream_spdif_hydra);

    if (ko == NULL)
        ko = kick_obj_from_sched_endpoint(ep);

    /* make sure that disabling time event and
     * enabling rate monitor events happen atomically */
    LOCK_INTERRUPTS;
    spdif_enable_time_event(ep, ko, FALSE);
    spdif_enable_rate_monitor(ep, ko, TRUE);
    UNLOCK_INTERRUPTS;
}

/**
 * spdif_switch_to_timer_event_kick
 * \brief will make the endpoint to use time events,
 * this will be suitable when the input stream is invalid
 *
 * \param ep pointer to the endpoint
 *
 * \param ko kick object, if null kick object will be taken from the endpoint
 */
static void spdif_switch_to_timer_event_kick(ENDPOINT *ep, KICK_OBJECT *ko)
{
    patch_fn_shared(stream_spdif_hydra);

    if (ko == NULL)
        ko = kick_obj_from_sched_endpoint(ep);

    /* make sure that enabling time event and
     * disabling rate monitor events happen atomically
     */
    LOCK_INTERRUPTS;
    spdif_enable_rate_monitor(ep, ko, FALSE);
    spdif_enable_time_event(ep, ko, TRUE);
    UNLOCK_INTERRUPTS;
}

/* ********************************** API functions ************************************* */
/**
 * \brief closes the spdif end point by releasing all the extra resources
 *  that the endpoint has claimed
 *
 *
 * \param *endpoint pointer to the endpoint that is being closed.
 *
 * \return success or failure
 */
static bool spdif_close(ENDPOINT *endpoint)
{
    /* see if it has a twin endpoint */
    if(NULL != endpoint->state.spdif.twin_endpoint)
    {
        /* de-link A & B */
        endpoint->state.spdif.twin_endpoint->state.spdif.twin_endpoint = NULL;
        endpoint->state.spdif.twin_endpoint = NULL;
    }

    /* free extra structure */
    if(NULL != endpoint->state.spdif.extra)
    {
        pfree(endpoint->state.spdif.extra);
        endpoint->state.spdif.extra = NULL;
    }

    /* By the time we reach this function we will have stopped everything from
     * running, so all we need to do is call the close hw method on audio, if it has
     * been allocated
     */
    return (endpoint->state.spdif.hw_allocated)
        ? audio_vsm_release_hardware(stream_external_id_from_endpoint(endpoint))
        : TRUE;
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
static bool spdif_connect(ENDPOINT *endpoint, tCbuffer *Cbuffer_ptr, ENDPOINT *ep_to_kick, bool* start_on_connect)
{

    unsigned int hw_buf_size = CHARS_TO_SAMPLES(Cbuffer_ptr->size);    /* in samples */
    unsigned flags = 0;

    patch_fn_shared(stream_spdif_hydra);
    
    endpoint->ep_to_kick = ep_to_kick;

    /* optimisation for two-channel config connected to the same op terminal */
    if(ep_to_kick != NULL && endpoint->state.spdif.twin_endpoint)
    {
        /* both endpoints are expected to connect to same spdif_decode operator,
         * however as this can be overriten explictly by user, we check it
         */
        if (stream_is_connected_to_same_entity(endpoint, endpoint->state.spdif.twin_endpoint))
        {
            /* both connected to the same operator, so we only kick through A channel */
            if(endpoint->state.spdif.channel_order == SPCO_CHANNEL_B)
            {
                endpoint->ep_to_kick = NULL;
            }
            else
            {
                endpoint->state.spdif.twin_endpoint->ep_to_kick = NULL;
            }
        }
    }

    /* Set MMU handle sample size to 32-bit unpacked until we teach audio endpoints
     * to decide to use specific sample types.
     */
#ifdef BAC32
#ifdef TODO_CRESCENDO_STREAMS_SAMPLE_SIZE_SUPPORT
    flags = BUF_DESC_UNPACKED_32BIT_MASK;
#else /* TODO_CRESCENDO_STREAMS_SAMPLE_SIZE_SUPPORT */
#error "sample size support in streams is not implemented"
#endif /* TODO_CRESCENDO_STREAMS_SAMPLE_SIZE_SUPPORT */
#endif /* BAC32 */

    if (!endpoint->state.spdif.is_overridden)
    {
        endpoint->state.spdif.sink_buf = Cbuffer_ptr;
        endpoint->state.spdif.source_buf = cbuffer_create_mmu_buffer(flags | BUF_DESC_MMU_BUFFER_HW_WR,
                                                                     &hw_buf_size);

        if (endpoint->state.spdif.source_buf == NULL)
        {
            return FALSE;
        }
    }
    else
    {
        /* The input buffer is already wrapped because the buffer_details is
         * changed to BUF_DESC_MMU_BUFFER_HW_WR when the endpoint is overridden*/
        endpoint->state.spdif.sink_buf = NULL;
        endpoint->state.spdif.source_buf = Cbuffer_ptr;
    }

    /* Configure hardware transformation flags, to produce desired data format */
    cbuffer_set_write_shift(endpoint->state.spdif.source_buf, endpoint->state.spdif.shift);
#ifdef KAL_ARCH4
    /* Set MMU handle sample size to 32-bit unpacked until we teach audio endpoints
     * to decide to use specific sample types. Also set sign extend.
     */
#endif

    *start_on_connect = TRUE;
    return TRUE;
}

/**
 * \brief Disconnects from an endpoint and stops the data from flowing
 *
 * \param *endpoint pointer to the endpoint that is being disconnected
 *
 * \return success or failure
 */
static bool spdif_disconnect(ENDPOINT *endpoint)
{
    patch_fn_shared(stream_spdif_hydra);
    
    endpoint->state.spdif.sink_buf = NULL;

    /* Reset ep_to_kick flag to it's default state of no endpoint to kick. */
    endpoint->ep_to_kick = NULL;
    if (!endpoint->state.spdif.is_overridden)
    {
        cbuffer_destroy(endpoint->state.spdif.source_buf);
        endpoint->state.spdif.source_buf = NULL;
        
    }

    /* Clear the override flag */
    endpoint->state.spdif.is_overridden = FALSE;

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
static bool spdif_buffer_details(ENDPOINT *endpoint, BUFFER_DETAILS *details)
{
    patch_fn_shared(stream_spdif_hydra);
    
    if (endpoint == NULL || details == NULL)
    {
        return FALSE;
    }

    details->supplies_buffer = FALSE;

    /* set the minimum size required */
    details->b.buff_params.size = endpoint->state.spdif.channel_order == SPCO_CHANNEL_AB? 2*HYDRA_SPDIF_BUFFER_SIZE:HYDRA_SPDIF_BUFFER_SIZE;

    if (endpoint->state.spdif.is_overridden)
    {
        details->b.buff_params.flags = BUF_DESC_MMU_BUFFER_HW_WR;
        /* Set MMU handle sample size to 32-bit unpacked until we teach audio endpoints
         * to decide to use specific sample types.
         */
#ifdef BAC32
#ifdef TODO_CRESCENDO_STREAMS_SAMPLE_SIZE_SUPPORT
        details->b.buff_params.flags |= BUF_DESC_UNPACKED_32BIT_MASK;
#else /* TODO_CRESCENDO_STREAMS_SAMPLE_SIZE_SUPPORT */
#error "sample size support in streams is not implemented"
#endif /* TODO_CRESCENDO_STREAMS_SAMPLE_SIZE_SUPPORT */
#endif /* BAC32 */
    }
    else
    {
        details->b.buff_params.flags = BUF_DESC_SW_BUFFER;
    }

    details->supplies_buffer = FALSE;
    details->runs_in_place = FALSE;
    details->can_override = TRUE;
    details->wants_override = TRUE;
    return TRUE;
}

/**
 * \brief Starts a kick interrupt source based off this spdif endpoint.
 *
 * \param ep pointer to the endpoint which is responsible for scheduling
 * the kick.
 * \param ko pointer to the KICK_OBJECT that received an interrupt and called
 * this function.
 *
 * \return TRUE/FALSE success or failure
 */
static bool spdif_start (ENDPOINT *ep, KICK_OBJECT *ko)
{
    mmu_handle handle;
    unsigned max_offset;
    unsigned offset = 0;

    patch_fn_shared(stream_spdif_hydra);
    
    /* If we are already running or overridden then don't do anything */
    if(ep->state.spdif.running)
    {
        return TRUE;
    }

    /* A & B must be connected to the same spdif_decode op, unless
     * user has changed the ep format*/
    if(NULL != ep->state.spdif.twin_endpoint)
    {
        ENDPOINT *ep2 = ep->state.spdif.twin_endpoint;
        ENDPOINT_GET_CONFIG_RESULT ep_result;
        ENDPOINT_GET_CONFIG_RESULT ep2_result;
        spdif_get_config(ep, EP_DATA_FORMAT, &ep_result);
        spdif_get_config(ep2, EP_DATA_FORMAT, &ep2_result);

        if(SPDIF_INPUT_DATA_FORMAT == ep_result.u.value &&
           SPDIF_INPUT_DATA_FORMAT == ep2_result.u.value)
        {
            if (!stream_is_connected_to_same_entity(ep, ep2))
            {
                /* we could also panic, but we just return FALSE, so
                 * the client can reconnect the endpoint to right op
                 */
                return FALSE;
            }
        }
    }

    /* Retrieve the buffer handle, max offset and initial offset for the endpoint */
    handle = cbuffer_get_write_mmu_handle(ep->state.spdif.source_buf);
    max_offset = mmu_buffer_get_size(handle);

    /* Set the offset to halfway through the buffer,
     * accounting for the fact that the read offset may have moved
     */
    offset = (mmu_buffer_get_handle_offset(handle) + (max_offset / 2)) % max_offset;
    if(NULL != ep->connected_to)
    {
        audio_vsm_update_initial_buffer_offset(stream_external_id_from_endpoint(ep), offset);
    }

    /* if the endpoint has a twin, make sure same offset used for the twin as well,
     * we need to keep the two always in sync.
     */
    if(NULL != ep->state.spdif.twin_endpoint)
    {
        if(NULL != ep->state.spdif.twin_endpoint->connected_to)
        {
            audio_vsm_update_initial_buffer_offset(stream_external_id_from_endpoint(ep->state.spdif.twin_endpoint), offset);
        }
    }

    /* Activate S/PDIF endpoint */
    if (!audio_vsm_activate_sid(stream_external_id_from_endpoint(ep), handle, offset, max_offset))
    {
        return FALSE;
    }

    /* now is fully running and will return TRUE */
    ep->state.spdif.running = TRUE;

     /* If endpoint is overridden, don't do anything else */
    if (ep->state.spdif.is_overridden)
    {
        return TRUE;
    }

    if(ep->state.spdif.channel_order != SPCO_CHANNEL_B)
    {
        stream_set_deferred_kick(ep, ep->deferred.config_deferred_kick);

        LOCK_INTERRUPTS;
        if(ep->state.spdif.extra->fw_sample_rate_changed)
        {
            spdif_enable_rate_monitor(ep, ko, TRUE);
        }
        else
        {
            spdif_enable_time_event(ep, ko, TRUE);
        }
        UNLOCK_INTERRUPTS;
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
static bool spdif_stop (ENDPOINT *ep)
{
    KICK_OBJECT *ko = kick_obj_from_sched_endpoint(ep);

    patch_fn_shared(stream_spdif_hydra);
    
    if(!ep->state.spdif.running)
    {
        /* The kick source is already stopped */
        return FALSE;
    }

    /* Deactivate the audio before unwrapping the mmu buffer as the audio code
     * does some tidying up of the buffer. */
    if(!audio_vsm_deactivate_sid(stream_external_id_from_endpoint(ep)))
    {
        /* If the deactivation failed then don't complete the disconnection as the
         * buffer may still be in use. */
        return FALSE;
    }

    /* The stop process now is complete */
    ep->state.spdif.running = FALSE;

    if (ep->state.spdif.is_overridden)
    {
        /*Until disconnect the ep is overridden*/
        return TRUE;
    }

    if (ep->state.spdif.channel_order != SPCO_CHANNEL_B)
    {

        LOCK_INTERRUPTS;

        if(ep->state.spdif.monitor_enabled)
        {
            spdif_enable_rate_monitor(ep, ko, FALSE);
        }
        else
        {
            spdif_enable_time_event(ep, ko, FALSE);
        }
        UNLOCK_INTERRUPTS;

        stream_set_deferred_kick(ep, FALSE);
    }
    return TRUE;

}

/**
 * \brief Copies S/PDIF data from the port and verifies whether the input is valid.
 *
 * \param endpoint pointer to the endpoint that received a kick
 * \param kick_dir kick direction
 */

static void spdif_kick(ENDPOINT *endpoint, ENDPOINT_KICK_DIRECTION kick_dir)
{
    /* We won't receive kick for B channels but ignore it if we did */
    if (endpoint->state.spdif.channel_order != SPCO_CHANNEL_B)
    {
        struct spdif_extra_states *extra = endpoint->state.spdif.extra;
        TIME current_time;
        unsigned amount_to_read =  cbuffer_calc_amount_data_in_words(endpoint->state.spdif.source_buf);
        unsigned amount_to_write = cbuffer_calc_amount_space_in_words(endpoint->state.spdif.sink_buf);
        unsigned int interleaved = 0;
        unsigned silence_to_insert = 0;
        ENDPOINT *endpoint_b = NULL;

        /* The output is always 24bit */
        unsigned int shift_amount = 0;

        if (endpoint->deferred.kick_is_deferred)
        {
            current_time = endpoint->deferred.interrupt_handled_time;
        }
        else
        {
            current_time = hal_get_time();
        }

        /* update the actual read interval */
        extra->read_interval = time_sub(current_time, extra->last_read_time);

        extra->last_read_time = current_time;

        if(endpoint->state.spdif.channel_order == SPCO_CHANNEL_A)
        {
            /* process channel A and Channel B*/
            endpoint_b = endpoint->state.spdif.twin_endpoint;
            amount_to_read = MIN(amount_to_read, cbuffer_calc_amount_data_in_words(endpoint_b->state.spdif.source_buf));
            amount_to_write = MIN(amount_to_write, cbuffer_calc_amount_space_in_words(endpoint_b->state.spdif.sink_buf));
        }
        else
        {
            /* process channel AB */
            amount_to_read = amount_to_read >> 1;
            amount_to_write = amount_to_write >> 1;
            interleaved = 1;
        }
        silence_to_insert = amount_to_write;

        /* double check the sample rate received from fw */
        if(!extra->fw_sample_rate_changed)
        {
            spdif_detect_sample_rate(amount_to_read, &(extra->stream_valid));
        }
        else
        {
            extra->fw_sample_rate_changed = FALSE;
        }

        /* stream is valid if both FW and DSP agrees on sample rate, or
         * at least they agree that the sample rate is a valid high rate
         */
        extra->stream_valid =
            (extra->fw_sample_rate != 0) &&
            ((extra->dsp_sample_rate == extra->fw_sample_rate));

        if(extra->stream_valid)
        {
            /* set the final sample rate
             * this rate doesn't change until next
             * valid sample rate
             */
            endpoint->state.spdif.sample_rate = extra->dsp_sample_rate;
            if(NULL != endpoint_b)
                endpoint_b->state.spdif.sample_rate = extra->dsp_sample_rate;
        }
        else
        {
            /* no data will be written*/
            amount_to_write = 0;
        }
        amount_to_write = MIN(amount_to_write, amount_to_read);
        silence_to_insert -= amount_to_write;

        /* now read from the input ports */
        if(amount_to_read)
        {
            /* copy raw data */
            spdif_copy_raw_data(endpoint->state.spdif.sink_buf,
                                endpoint->state.spdif.source_buf,
                                amount_to_read << interleaved,
                                amount_to_write << interleaved,
                                shift_amount);

            /* Also copy raw data for corresponding B channel */
            if(NULL != endpoint_b)
                spdif_copy_raw_data(endpoint_b->state.spdif.sink_buf,
                                    endpoint_b->state.spdif.source_buf,
                                    amount_to_read,
                                    amount_to_write,
                                    shift_amount);
        }

        /* See if need to insert silence at this kick */
        silence_to_insert =  MIN(silence_to_insert,
                                 spdif_handle_pause_state(extra->silence_duration,
                                                          (void *)&extra->stream_valid,
                                                          extra->stream_valid,
                                                          endpoint->state.spdif.sample_rate));

        /* Insert silence if in pause mode */
        if(silence_to_insert != 0)
        {
            spdif_copy_raw_data(endpoint->state.spdif.sink_buf,
                                endpoint->state.spdif.source_buf,
                                0,
                                silence_to_insert<<interleaved,
                                shift_amount);
            if(NULL != endpoint_b)
                spdif_copy_raw_data(endpoint_b->state.spdif.sink_buf,
                                    endpoint_b->state.spdif.source_buf,
                                    0,
                                    silence_to_insert,
                                    shift_amount);

        }

        /* kick forward if anything written to outputs
         * TODO: optimise this by configurable decimation
         */
        if(amount_to_write != 0 ||
           amount_to_read != 0||
           (extra->pause_state==SPEPS_ACTIVE_PAUSE))
        {
            propagate_kick(endpoint, STREAM_KICK_FORWARDS);

            /* Normally B type won't generate kick */
            if(endpoint_b != NULL)
            {
                propagate_kick(endpoint_b, STREAM_KICK_FORWARDS);
            }
        }
    }
}

/**
 * \brief Schedules the next kick for the chain based upon the audio data and
 *        performs a port<->buffer copy
 *
 * \param endpoint pointer to the endpoint this function is called on.
 * \param ko pointer to the kick object that called this function. This is used
 * for rescheduling rather than caching the value.
 */
static void spdif_sched_kick (ENDPOINT *endpoint, KICK_OBJECT *ko)
{
    TIME next_fire_time;

    if(TIMER_ID_INVALID != endpoint->state.spdif.kick_id)
    {

        /* This is a periodic timer so ask timers when it was scheduled to fire and
         * use that to schedule the next one. */
        next_fire_time = time_add(get_last_fire_time(), endpoint->state.spdif.kick_period);

        endpoint->state.spdif.kick_id = timer_schedule_event_at(next_fire_time,
                                                                kick_obj_kick, (void*)ko);
    }
    else
    {
        /* TODO: worth a panic here */
    }
}

/*
 * \brief configure a spdif endpoint with a key and value pair
 *
 * \param *endpoint pointer to the endpoint to be configured
 * \param key denoting what is being configured
 * \param value value to which the key is to be configured
 *
 * \return Whether the request succeeded.
 *
 */
static bool spdif_configure(ENDPOINT *endpoint, unsigned key, uint32 value)
{
    if(key & 0x010000)
    {
        switch (key)
        {

            case EP_DATA_FORMAT:
                /* we don't allow spdif data format to be changed internally by framework,
                 * so fail if it doesn't match
                 */
                return (endpoint->state.spdif.output_format == value);

            case EP_BLOCK_SIZE:
                /* ignore value, just use half a block size */
                endpoint->state.spdif.monitor_threshold = SPDIF_RX_BLOCK_SIZE_SIZE/2;
                return TRUE;

            case EP_OVERRIDE_ENDPOINT:
                /* Set the logical value of override flag. */
                endpoint->state.spdif.is_overridden = (bool)value;
                return TRUE;

            case EP_CBOPS_PARAMETERS:
            {
                CBOPS_PARAMETERS *parameters = (CBOPS_PARAMETERS *)(uintptr_t) value;
                /* spdif endpoint doesn't have any active cbops */
                if (parameters)
                {
                    free_cbops_parameters(parameters);
                    return endpoint->state.spdif.is_overridden;
                }
                return FALSE;
            }

            default:
                return FALSE;
        }
    }
    else
    {
        switch (key)
        {
        case ACCMD_CONFIG_KEY_STREAM_SPDIF_SET_EP_FORMAT:
        {
            /* The spdif endpoint shall not be connected to anything other than
             * a spdif_decode endpoint which expects to receive input in
             * SPDIF_INPUT_DATA_FORMAT (means pcm or coded audio), however to allow
             * testing hw without getting the op involved the user can config the output,
             * that shall not be used in a real app though.
             */
            if (value == SPDIF_INPUT_DATA_FORMAT || value == AUDIO_DATA_FORMAT_FIXP)
            {
                endpoint->state.spdif.output_format = value;
                return TRUE;
            }
            return FALSE;
        }

        case ACCMD_CONFIG_KEY_STREAM_SPDIF_OUTPUT_RATE:
        {
            if(audio_vsm_configure_sid(stream_external_id_from_endpoint(endpoint),
                                       (stream_config_key)key, value))
            {
                /* sample rate explicitly changed by client(not by auto rate detection),
				 * perhaps for testing purpose, we treat it as if we have received
                 * it via a message from hal fw.
                 */
                if(endpoint->state.spdif.channel_order != SPCO_CHANNEL_B)
                {
                    endpoint->state.spdif.extra->fw_sample_rate = (unsigned)value;
                    endpoint->state.spdif.extra->fw_sample_rate_changed = TRUE;
                }
                return TRUE;
            }
            return FALSE;
        }

        case ACCMD_CONFIG_KEY_STREAM_RM_ENABLE_DEFERRED_KICK:
        {
            endpoint->deferred.config_deferred_kick = (value != 0);
            return TRUE;
        }
        default:
            if (key == ACCMD_CONFIG_KEY_STREAM_AUDIO_SAMPLE_SIZE)
            {
                /* set the hardware shift value */
                if(value == HAL_AUDIO_SAMPLE_SIZE_16)
                {
                    endpoint->state.spdif.shift = DAWTH - 16;
                }
                else if(value == HAL_AUDIO_SAMPLE_SIZE_24)
                {
                    endpoint->state.spdif.shift = DAWTH- 24;
                }
                else
                {
                    /* only 16 and 24 bit configs are allowed for SPDIF */
                    return FALSE;
                }
            }

            return audio_vsm_configure_sid(stream_external_id_from_endpoint(endpoint),
                                           (stream_config_key)key, value);
        }
    }
}

/**
 * \brief Get SPDIF (RX) endpoint rate
 */
static int spdif_get_rate(ENDPOINT* endpoint)
{
    if(endpoint->state.spdif.channel_order == SPCO_CHANNEL_B)
    {
        /* B-type info is stored in its twin endpoint */
        endpoint = endpoint->state.spdif.twin_endpoint;
    }

    /* get the rate inaccuracy */
    if (endpoint->state.spdif.running)
    {
        return endpoint->state.spdif.extra->norm_rate_ratio;
    }
    else
    {
        return RM_PERFECT_RATE;
    }
}

/*
 * \brief get spdif endpoint configuration
 *
 * \param *endpoint pointer to the endpoint to be configured
 * \param key denoting what is being configured
 * \param value pointer to a value which is populated with the current value
 *
 * \return Whether the request succeeded.
 *
 */
static bool spdif_get_config(ENDPOINT *endpoint, unsigned key, ENDPOINT_GET_CONFIG_RESULT* result)
{
    switch(key)
    {
        /* generic data format of the endpoint */
        case EP_DATA_FORMAT:
            result->u.value = endpoint->state.spdif.output_format;
            if(result->u.value == 0)
            {
                result->u.value = SPDIF_INPUT_DATA_FORMAT;
            }
            return TRUE;

        case EP_BLOCK_SIZE:
            result->u.value = endpoint->state.spdif.monitor_threshold;
            return TRUE;

            /* sample rate */
        case EP_SAMPLE_RATE:
        case STREAM_INFO_KEY_AUDIO_SAMPLE_RATE:

            if(endpoint->state.spdif.channel_order == SPCO_CHANNEL_B)
            {
                /* B-type info is stored in its twin endpoint */
                endpoint = endpoint->state.spdif.twin_endpoint;
            }

            if(endpoint->state.spdif.is_overridden)
            {
                /* get the sample rate directly from hal fw*/
                result->u.value = audio_vsm_get_sample_rate_from_sid(stream_external_id_from_endpoint(endpoint));
            }
            else
            {
                /* get the sample rate from endpoint*/
                if(endpoint->state.spdif.extra->stream_valid)
                {
                    result->u.value = endpoint->state.spdif.sample_rate;
                }
                else
                {
                    result->u.value = 0;
                }
            }

            return TRUE;

        case EP_RATEMATCH_ABILITY:
            /* s/pdif stream can only be consumed by
             * a spdif_decode op
             */
            result->u.value = (uint32)RATEMATCHING_SUPPORT_NONE;
            return TRUE;

        case EP_RATEMATCH_RATE:
            result->u.value = spdif_get_rate(endpoint);
            return TRUE;

        case EP_RATEMATCH_MEASUREMENT:
            result->u.rm_meas.sp_deviation =
                    STREAM_RATEMATCHING_RATE_TO_FRAC(spdif_get_rate(endpoint));
            result->u.rm_meas.measurement.valid = FALSE;
            return TRUE;

        case EP_CHANNEL_ORDER:
            result->u.value = (uint32) endpoint->state.spdif.channel_order;
            return TRUE;

        case EP_CBOPS_PARAMETERS:

        {
            /* spdif endpoint doesn't have/need any cbops processing
             * however if requested it supplies a pass-through equivalent
             * of cbops. Note that no processing shall be applied to s/pdif
             * input before it is processed by the spdif_decode operator,
             * however it is fine to route it directly to an spdif output endpoint
             * without any cbops or other type of processing, this shall not be used
             * in production.
             */
            CBOPS_PARAMETERS *parameters = create_cbops_parameters(EMPTY_FLAG, EMPTY_FLAG);
            if (!parameters)
            {
                return FALSE;
            }

            /* Set additional parameters if needed.*/
            result->u.value = (uint32)(uintptr_t) parameters;
            return TRUE;
        }

        default:
            return FALSE;
    }
}

/**
 * \brief synchronise two spdif end_points
 *
 * \param ep1 pointer to the endpoint to synchronise
 * \param ep2 pointer to the endpoint to synchronise
 *
 */

static bool spdif_sync_endpoints (ENDPOINT *ep1, ENDPOINT *ep2)
{
    if(ep2 != NULL)
    {
        if(ep2 == ep1->state.spdif.twin_endpoint &&
           ep1 == ep2->state.spdif.twin_endpoint)
        {
            /* make sure A channel is the head */
            if(ep1->state.spdif.channel_order == SPCO_CHANNEL_A)
            {
                return audio_vsm_sync_sids(stream_external_id_from_endpoint(ep1), stream_external_id_from_endpoint(ep2));
            }
            else
            {
                return audio_vsm_sync_sids(stream_external_id_from_endpoint(ep2), stream_external_id_from_endpoint(ep1));
            }
        }
        else
        {
            return FALSE;
        }
    }
    /* we can't unsync two channels, so in this case
     * we only expect AB channel
     */
    else if (ep1->state.spdif.channel_order == SPCO_CHANNEL_AB)
    {
        return TRUE;
    }
    return FALSE;

}

/**
 * \brief Get the timing requirements of this spdif endpoint
 *
 * \param endpoint pointer to the endpoint to get the timing info for
 * \param time_info a pointer to an ENDPOINT_TIMING_INFORMATION structure to
 * populate with the endpoint's timing information
 */
static void spdif_get_timing (ENDPOINT *endpoint, ENDPOINT_TIMING_INFORMATION *time_info)
{
    /* we don't apply rate matching at spdif rx side, because it needs to be procecessed by
     * spdif_decode first. However we set the 'period' to a meaningful value, and
     * since the rate changes we use most common rate of 48000 for this purpose.
     */
    time_info->period = (unsigned int)((unsigned long)STREAM_KICK_PERIOD_FROM_USECS(1000000UL) / SPDIF_TYPICAL_SAMPLE_RATE);
    time_info->block_size = 1;
    time_info->has_deadline = FALSE;

    /* spdif rx is always slave,
     * but for overriden mode however we assume it's locally clocked
     * */
    time_info->locally_clocked = endpoint->state.spdif.is_overridden? TRUE:
        endpoint->state.spdif.locally_clocked;
    time_info->wants_kicks = FALSE;
    return;
}
