
/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  stream_spdif_bluecore.c
 * \ingroup stream
 *
 * stream spdif type file. <br>
 *
 * This file contains stream functions for spdif endpoints in bluecore chips.
 * Currently this file is only for S/PDIF Rx endpoints and S/PDIF Tx will
 * use standard PCM audio endpoints.
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
#include "stream_endpoint_spdif.h"
#include "pl_assert.h"

/****************************************************************************
Private Macro Declarations
*/

/* S/PDIF endpoint is kicked in constant period */
#define BC_SPDIF_KICK_PERIOD_US 1000
#define BC_SPDIF_BUFFER_SIZE 288
#define SPDIF_TYPICAL_SAMPLE_RATE 48000
enum bc_spdif_endpoint_params
{
    BC_SPDIF_PARAMS_PORT = 0,
    BC_SPDIF_PARAMS_NUM
};

/* default supported rates for blue core chips */
#define SPDIF_DEFAULT_SUPPORTED_RATES (SPSRM_48KHZ|SPSRM_44K1HZ|SPSRM_32KHZ|SPSRM_96KHZ|SPSRM_88K2HZ)

/* in bluecore different procedure is
 * used for detection of higher rates
 */
#define IS_SPDIF_HIGH_RATE(r) (r>48000)

/****************************************************************************
Private Constant Declarations
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
};

/****************************************************************************
Private Variable Definitions
*/

/****************************************************************************
Private Function Declarations
*/
static bool spdif_close (ENDPOINT *endpoint);
static bool spdif_connect (ENDPOINT *endpoint, tCbuffer *Cbuffer_ptr, ENDPOINT *ep_to_kick, bool* start_on_connect);
static bool spdif_disconnect (ENDPOINT *endpoint);
static bool spdif_buffer_details (ENDPOINT *endpoint, BUFFER_DETAILS *details);
static void spdif_kick(ENDPOINT *endpoint, ENDPOINT_KICK_DIRECTION kick_dir);
static void spdif_sched_kick (ENDPOINT *endpoint, KICK_OBJECT *ko);
static bool spdif_start (ENDPOINT *endpoint, KICK_OBJECT *ko);
static bool spdif_stop (ENDPOINT *endpoint);
static bool spdif_configure (ENDPOINT *endpoint, unsigned int key, uint32 value);
static bool spdif_get_config (ENDPOINT *endpoint, unsigned int key, ENDPOINT_GET_CONFIG_RESULT *result);
static void spdif_get_timing (ENDPOINT *endpoint, ENDPOINT_TIMING_INFORMATION *time_info);
static void stream_spdif_complete_endpoint(ENDPOINT *endpoint);
static bool spdif_sync_endpoints (ENDPOINT *ep1, ENDPOINT *ep2);
static void spdif_handle_bluecore_highrates(ENDPOINT *endpoint);
static bool stream_spdif_chsts_forward_call_cback(unsigned con_id, unsigned status,
                                                  unsigned op_id, unsigned num_resp_params, unsigned *resp_params);

DEFINE_ENDPOINT_FUNCTIONS (spdif_functions, spdif_close, spdif_connect,
                           spdif_disconnect, spdif_buffer_details,
                           spdif_kick, spdif_sched_kick,
                           spdif_start, spdif_stop,
                           spdif_configure, spdif_get_config,
                           spdif_get_timing, spdif_sync_endpoints);

/****************************************************************************
Public Function Definitions
*/
/****************************************************************************
 *
 * bc_spdif_ep_message_handler
 *
 * This function is used to handle the following messages received
 * from firmware in bluecore chips:
 *
 * XAP_KAL_MSG_AUDIO_INFO_RESP:
 *  This is a response message and endpoint wont get completed until
 *  XAP_KAL_MSG_AUDIO_INFO_REQ is received. The endpoint needs to know
 *  the channel order and sample widgth before it can start operating.
 *
 * XAP_KAL_MSG_AUDIO_STREAM_RATE_EVENT:
 *  The message is sent by Blue Firmware when a new rate is detected,
 *  DSP will double-check this rate. Only when both agrees on a rate
 *  will the stream be valid.
 *
 * XAP_KAL_MSG_SPDIF_CHNL_STS_EVENT:
 *  Received from FW whenever the channel status bits changes. The
 *  bits are received for currently selected channel only.
 *
 * XAP_KAL_MSG_SPDIF_BLOCK_START_EVENT:
 *  This message will tell us that a proper s/pdif block has started
 *
 * XAP_KAL_MSG_ACTIVATE_AUDIO_RESPONSE:
 *  This is a response from FW when DSP activates or de-activates the
 *  interface
 *
 * ANY OTHER MESSAGES:
 *  Not expected and we panic
 ***************************************************************************/
void bc_spdif_ep_message_handler(unsigned msg_id, unsigned *pdu)
{
    switch(msg_id)
    {
        /* Info response message to complete */
        case XAP_KAL_MSG_AUDIO_INFO_RESP:
        {
            ENDPOINT *ep = NULL;
            unsigned i;
            ENDPOINT_DIRECTION dir = SOURCE;
            unsigned port = pdu[0];
            unsigned no_keys = pdu[1];
            unsigned sample_rate = 0;

            if (port >= BC_NUM_PORTS)
            {
                /* We shouldn't come to this point
                 * until this ep supports SPDIF TX too
                 */
                panic_spdif_invalid_param(SIPR_INVALID_PORT_ID,port);
            }

            /* get the corresponding spdif endpoint */
            ep = stream_endpoint_from_extern_id(
                stream_get_endpointid_from_portid(port, dir));

            for (i = 0; i < no_keys; i++)
            {
                switch (pdu[2 + (i * 3)])
                {
                    /* instance number will be used to match channel A & B
                     */
                    case AUDIO_CONFIG_INSTANCE:
                    {
                        ep->state.spdif.instance = pdu[2 + 2 + (i * 3)];
                        break;
                    }

                    /* get the channel order */
                    case AUDIO_CONFIG_CHANNEL:
                    {
                        ep->state.spdif.channel_order = pdu[2 + 2 + (i * 3)];
                        /* check that the channel order isn't a random number */
                        if(ep->state.spdif.channel_order != SPCO_CHANNEL_AB &&
                           ep->state.spdif.channel_order != SPCO_CHANNEL_A &&
                           ep->state.spdif.channel_order != SPCO_CHANNEL_B)
                        {
                            panic_spdif_invalid_param(SIPR_UNEXPECTED_CHANNEL_ORDER, ep->state.spdif.channel_order);
                        }
                        break;
                    }

                    /* get the sample width */
                    case AUDIO_CONFIG_SAMPLE_FORMAT:
                    {
                        /* in bluecore this shall be either 16 or 24 */
                        ep->state.spdif.input_width = (bool)pdu[2 + 2 + (i * 3)];
                        break;
                    }

                    /* get the sample rate*/
                    case AUDIO_CONFIG_SAMPLE_RATE:
                    {
                        /* we normally need this if the interface doesn't use auto rate
                         * detection
                         */
                        sample_rate = (unsigned)(((uint32)(pdu[2 + 1 + (i*3)]) << 16) |
                                                 (uint32)pdu[2 + 2 + (i * 3)]);
                        break;
                    }

                    default:
                        break;
                }
            }

            /* If the end point is for right channel only we don't
             * need the extra structure
             */
            if(ep->state.spdif.channel_order == SPCO_CHANNEL_B)
            {
                /* delete extra allocated buffer */
                if(ep->state.spdif.extra)
                {
                    pdelete(ep->state.spdif.extra);
                    ep->state.spdif.extra = NULL;
                }
            }
            else
            {
                /* set the sample rate, this could be 0 if the auto rate detection is enabled
                 * if not enabled it must provide the endpoint with the right sample rate else the
                 * output will be muted.
                 */

                /* flag that sample rate has changed */
                ep->state.spdif.extra->fw_sample_rate = sample_rate;
                ep->state.spdif.extra->fw_sample_rate_changed = TRUE;

                /* for high rates only */
                if(IS_SPDIF_HIGH_RATE(sample_rate))
                {
                    ep->state.spdif.extra->can_restart_interface = 1;
                    ep->state.spdif.extra->highrate_timer_time = hal_get_time();
                }
            }

            /* we now know everything about the s/pdif endpoint
             * so endpoint can be fully configured
             */
            stream_spdif_complete_endpoint(ep);

#ifndef UNIT_TEST_BUILD
            /* We should now have enough information
             * to allow the endpoint to be used
             */
            stream_if_ep_creation_complete(ep, TRUE);
#endif

            break;
        }

        case XAP_KAL_MSG_AUDIO_STREAM_RATE_EVENT:
        case XAP_KAL_MSG_SPDIF_CHNL_STS_EVENT:
        {
            ENDPOINT *ep = NULL;
            ENDPOINT *ep2 = NULL;
            uint16 port = pdu[1];
            unsigned int new_sample_rate;
            unsigned numports = pdu[0];

            /* number of ports */
            if(numports != 1 && numports != 2)
            {
                panic_spdif_invalid_param(SIPR_UEXPECTED_NUM_PORTS, numports);
            }

            /* expected for source only */
            if (port >= BC_NUM_PORTS)
            {
                panic_spdif_invalid_param(SIPR_INVALID_PORT_ID, port);
            }

            /* get end point */
            ep = stream_endpoint_from_extern_id(
                stream_get_endpointid_from_portid(port, SOURCE));

            /* the rate detection mechanism
             * could be started before the corresponding
             * endpoint created in kymera*/
            if(!ep)
                return;

            if(ep->stream_endpoint_type != endpoint_spdif)
            {
                panic_spdif_invalid_param(SIPR_SPDIF_EP_NOT_FOUND, port);
            }

            if(numports==2)
            {
                ep2 = stream_endpoint_from_extern_id(
                    stream_get_endpointid_from_portid(pdu[2], SOURCE));

                if(!ep2 ||
                   (ep2->stream_endpoint_type != endpoint_spdif) ||
                   (ep2->state.spdif.channel_order != SPCO_CHANNEL_B) ||
                   (ep->state.spdif.twin_endpoint != ep2))
                {
                    ep2 = NULL;
                }
            }

            if(msg_id == XAP_KAL_MSG_AUDIO_STREAM_RATE_EVENT)
            {

                /* NOTE: We don't support mono config
                 * Expect the first channel not to be of B type
                 */
                if( ep->stream_endpoint_type != endpoint_spdif ||
                    ((ep->state.spdif.channel_order != SPCO_CHANNEL_A) &&
                     (ep->state.spdif.channel_order != SPCO_CHANNEL_AB)))
                {
                    /* something is clearly wrong*/
                    panic_spdif_invalid_param(SIPR_NOT_MATCHING_CHANNEL_ORDER, ep->state.spdif.channel_order);
                }

                /* get the new sample rate */
                new_sample_rate = ((unsigned)pdu[1+numports])|(((unsigned)pdu[2+numports])<<16);

                if(ep->state.spdif.extra->fw_sample_rate != new_sample_rate)
                {
                    /* flag that sample rate has changed */
                    ep->state.spdif.extra->fw_sample_rate = new_sample_rate;
                    ep->state.spdif.extra->fw_sample_rate_changed = TRUE;
                }

                /* for high rates only */
                ep->state.spdif.extra->can_restart_interface = 1;
                ep->state.spdif.extra->highrate_timer_time = hal_get_time();

            }
            else if (msg_id == XAP_KAL_MSG_SPDIF_CHNL_STS_EVENT)
            {
                unsigned *chsts;
                unsigned i;
                unsigned *msg = &pdu[1+numports];
                unsigned tmp;
                unsigned chsts_changed = 0;
                unsigned channel;

                if((ep->state.spdif.channel_order == SPCO_CHANNEL_AB) ||
                   (ep->state.spdif.channel_order == SPCO_CHANNEL_A))
                {
                    /* status bits for A channel received */
                    chsts = &ep->state.spdif.extra->channel_status[0];
                    channel = SPCO_CHANNEL_A;
                }
                else
                {
                    /* status bits for B channel received */
                    chsts = &ep->state.spdif.twin_endpoint->state.spdif.extra->channel_status[SPDIF_NOROF_CHSTS_WORDS];
                    channel = SPCO_CHANNEL_B;
                }

                for(i=0; i<SPDIF_NOROF_CHSTS_WORDS; ++i)
                {
                    tmp = msg[i] & 0xFFFF; /* channel status words are 16-bit in all archs */
                    if(tmp != chsts[i])
                       chsts_changed |= (1 << i);
                    chsts[i] = tmp;
                }

                if(ep2)
                {
                    /* channel status message is expected to be only for a single channel, but in case
                     * that two channels are sent, it means that the status bits are the same for both
                     * channels
                     */
                    chsts = &ep->state.spdif.extra->channel_status[SPDIF_NOROF_CHSTS_WORDS];
                    for(i=0; i<SPDIF_NOROF_CHSTS_WORDS; ++i)
                    {
                        tmp = msg[i] & 0xFFFF; /* channel status words are 16-bit in all archs */
                        if(tmp != chsts[i])
                           chsts_changed |= (1 << i);
                        chsts[i] = tmp;
                    }
                }

                /* What we receive here isn't synchronised to audio blocks,
                 * For professional format, we only report if changes is seen
                 * in first 3 words.
                 */
                 if(chsts[0]&0x1)
                 {
                     /* it's professional format */
                     chsts_changed &= 0x7;
                 }

                 if(chsts_changed &&
                   ep->connected_to)
                 {
                     /* the client is informed via the operator only
                      * so the channel status is sent to the operator
                      */
                     unsigned chsts_msg[2 + SPDIF_NOROF_CHSTS_WORDS];
                     chsts_msg[0] =  OPMSG_SPDIF_DECODE_ID_NEW_CHSTS_FROM_EP;
                     chsts_msg[1] =  channel;
                     memcpy(&chsts_msg[2], chsts, SPDIF_NOROF_CHSTS_WORDS*sizeof(unsigned));
                     opmgr_operator_message(RESPOND_TO_OBPM,
                                           ep->connected_to->id, sizeof(chsts_msg)/sizeof(unsigned),
                                           chsts_msg, stream_spdif_chsts_forward_call_cback);
                 }
            }

            break;
        }

        /* No action needed for block start message */
        case XAP_KAL_MSG_SPDIF_BLOCK_START_EVENT:
            break;

        /* handling activation/deactivation response,
         * this will be needed for high rates only */
        case XAP_KAL_MSG_ACTIVATE_AUDIO_RESPONSE:
        {
            ENDPOINT *ep = NULL;
            uint16 port = pdu[0];
            uint16 resp = pdu[1];

            /* get end point */
            ep = stream_endpoint_from_extern_id(
                stream_get_endpointid_from_portid(port, SOURCE));

            /* just to make sure that the message is meant
             * to be processed here.
             */
            if(!ep || ep->stream_endpoint_type != endpoint_spdif)
            {
                panic_spdif_invalid_param(SIPR_SPDIF_EP_NOT_FOUND, port);
            }

            /* information about B channel is available via its
             * twin A channel
             */
            if(ep->state.spdif.channel_order == SPCO_CHANNEL_B)
                ep = ep->state.spdif.twin_endpoint;

            /* we also panic if fw fails to do the activation request */
            if(ep->state.spdif.extra->confirms_pending && !resp)
            {
                panic_spdif_invalid_param(SIPR_SPDIF_EP_ACT_REQUEST_FAILED, port);
            }

            /* one more request confirmed */
            ep->state.spdif.extra->confirms_pending--;

            if(ep->state.spdif.extra->confirms_pending == 0)
            {
                /* all the requests confirmed, if the interface is not active
                 * then we need to activate it here
                 */
                ep->state.spdif.extra->inactive = !ep->state.spdif.extra->inactive;
                if(ep->state.spdif.extra->inactive)
                {
                    unsigned msg[2];

                    /* re-activate the interface */
                    msg[0] = ep->state.spdif.portid;
                    msg[1] = 1;
                    send_non_baton_message(XAP_KAL_MSG_ACTIVATE_AUDIO, 2, msg);
                    ep->state.spdif.extra->confirms_pending++;

                    /* same for its twin endpoint if exists */
                    if(ep->state.spdif.twin_endpoint)
                    {
                        msg[0] = ep->state.spdif.twin_endpoint->state.spdif.portid;
                        msg[1] = 1;
                        send_non_baton_message(XAP_KAL_MSG_ACTIVATE_AUDIO, 2, msg);
                        ep->state.spdif.extra->confirms_pending++;
                    }
                }
            }
        }
        break;

        default:
        {
            panic_diatribe(PANIC_AUDIO_UNRECOGNISED_AUDIO_MESSAGE, msg_id);
            break;
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
 * stream_spdif_complete_endpoint
 * helper function to complete the creation of spdif endpoint
 */
static void stream_spdif_complete_endpoint(ENDPOINT *endpoint)
{
    struct endpoint_spdif_state *state = &endpoint->state.spdif;

    /* configure input port
     * TODO: 24-bit support */
    cbuffer_reconfigure_read_port(state->portid, ((state->input_width == 24)?BITMODE_24BIT:BITMODE_16BIT)| NOSIGNEXT_MASK);

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
        if(ep)
        {
                /* group the two endpoints */
                endpoint->state.spdif.twin_endpoint = ep;
                ep->state.spdif.twin_endpoint = endpoint;
        }

    }
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
    if (num_params < BC_SPDIF_PARAMS_NUM)
    {
        return NULL;
    }
    unsigned int port = params[BC_SPDIF_PARAMS_PORT];
    /* First go and find a stream on the same port, if not found
     * then create a new stream. */
    uint24 key = create_stream_key(STREAM_DEVICE_SPDIF, 0, port);
    ENDPOINT *endpoint = stream_get_endpoint_from_key_and_functions(key, dir,
                                                                    &endpoint_spdif_functions);

    if (!endpoint)
    {
        unsigned message[6];

        /* create a new endpoint */
        if ((endpoint = STREAM_NEW_ENDPOINT(spdif, key, dir, con_id)) == NULL)
        {
            return NULL;
        }

        /* Wrap the port up as a cbuffer, if we can't do that then fail here */
        if(SOURCE == dir)
        {
            if (NULL == (endpoint->state.spdif.source_buf =
                         cbuffer_wrap_bc_port(BUF_DESC_MMU_BUFFER_HW_WR, port)))
            {
                stream_destroy_endpoint(endpoint);
                return NULL;
            }
        }
        else
        {
            /* this is only for S/PDIF rx*/
            stream_destroy_endpoint(endpoint);
            return NULL;
        }

        /* set default input width */
        endpoint->state.spdif.input_width = 16;

        /* extra buffer for s/pdif endpoint,
         * B type doesn't need this, it will
         * be deleted for type B once type is known
         */
        endpoint->state.spdif.extra = (struct spdif_extra_states *)zpmalloc(sizeof(struct spdif_extra_states));
        if(endpoint->state.spdif.extra == NULL)
        {
            stream_destroy_endpoint(endpoint);
            return NULL;
        }
        endpoint->state.spdif.extra->last_read_time = hal_get_time();
        endpoint->state.spdif.extra->nrof_supported_rates = sizeof(spdif_sample_rates)/sizeof(unsigned);
        endpoint->state.spdif.extra->supported_rates = spdif_sample_rates;
        endpoint->state.spdif.extra->supported_rates_mask = SPDIF_DEFAULT_SUPPORTED_RATES;
        endpoint->state.spdif.extra->silence_duration = SPDIF_UNLIMITED_SILENCE_DURING_PAUSE;

        /* All is well */
        endpoint->state.spdif.kick_period = BC_SPDIF_KICK_PERIOD_US;
        endpoint->state.spdif.portid = port;
        endpoint->state.spdif.kick_id = TIMER_ID_INVALID;
        endpoint->can_be_closed = TRUE;
        endpoint->can_be_destroyed = FALSE;
        endpoint->is_real = TRUE;
        endpoint->state.spdif.locally_clocked = FALSE;
        endpoint->state.spdif.instance = 0;

        /* sample rate hasn't been received/detected yet */
        endpoint->state.spdif.sample_rate = 0;

        /* we don't need a cbops for S/PDIF end_point */
        endpoint->cbops = NULL;

        /* Request the information needed for running the endpoint*/
        message[0] = port + (dir == SOURCE?0:BC_NUM_PORTS);
        message[1] = 4;
        message[2] = AUDIO_CONFIG_INSTANCE;
        message[3] = AUDIO_CONFIG_CHANNEL;
        message[4] = AUDIO_CONFIG_SAMPLE_FORMAT;
        message[5] = AUDIO_CONFIG_SAMPLE_RATE;
        send_non_baton_message(XAP_KAL_MSG_AUDIO_INFO_REQ, 6, message);
        if (pending)
        {
            /* we need to wait for the audio info response
             * before the client can connect / start this endpoint
             */
            *pending = TRUE;
        }
    }
    return endpoint;
}

/* ********************************** API functions ************************************* */

/**
 * \brief closes the spdid end point by releasing all the extra resources
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
    if(endpoint->state.spdif.twin_endpoint)
    {
        /* de-link A & B */
        endpoint->state.spdif.twin_endpoint->state.spdif.twin_endpoint = NULL;
        endpoint->state.spdif.twin_endpoint = NULL;
    }

    /* free extra structure */
    if(endpoint->state.spdif.extra)
    {
        pfree(endpoint->state.spdif.extra);
        endpoint->state.spdif.extra = NULL;
    }

    /* By the time we reach this function we will have stopped everything from
     * running, so all we need to do is tidy up the buffer for the port.
     */
    cbuffer_destroy(endpoint->state.spdif.source_buf);

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
static bool spdif_connect(ENDPOINT *endpoint, tCbuffer *Cbuffer_ptr, ENDPOINT *ep_to_kick, bool* start_on_connect)
{

    /* If the connected endpoint doesn't want kicks then we can save effort so
     * record this fact. */
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

    /* remember this buffer so that the endpoint can write into/read
     * from it when it gets kicked. */
    endpoint->state.spdif.sink_buf = Cbuffer_ptr;
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
static bool spdif_disconnect(ENDPOINT *endpoint)
{
    /* Forget all about the connected buffer, it's gone away or is about to. If
     * the endpoint hasn't been stopped then this function shouldn't have been
     * called. */

    if(endpoint->state.spdif.running)
    {
        panic_diatribe(PANIC_AUDIO_UNEXPECTED_ENDPOINT_DISCONNECT,
                       stream_external_id_from_endpoint(endpoint));
    }

    endpoint->state.spdif.sink_buf = NULL;

    /* Reset ep_to_kick to it's default state of no endpoint to kick*/
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
static bool spdif_buffer_details(ENDPOINT *endpoint, BUFFER_DETAILS *details)
{
    if (endpoint == NULL || details == NULL)
    {
        return FALSE;
    }

    details->supplies_buffer = FALSE;

    /* set the minimum size required */
    details->b.buff_params.size = endpoint->state.spdif.channel_order == SPCO_CHANNEL_AB? 2*BC_SPDIF_BUFFER_SIZE:BC_SPDIF_BUFFER_SIZE;

    /* we just want a SW buffer to work with. */
    details->b.buff_params.flags = BUF_DESC_SW_BUFFER;

    /* no overriding is needed */
    details->can_override = FALSE;
    details->wants_override = FALSE;

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
    /* If we are already running don't do anything */
    if(ep->state.spdif.running)
    {
        return TRUE;
    }

    /* At this point a type A channel must be paired
     * with a B type channel
     */
    if(ep->state.spdif.channel_order == SPCO_CHANNEL_A)
    {
        if(ep->state.spdif.twin_endpoint == NULL ||
           ep->state.spdif.twin_endpoint->state.spdif.channel_order != SPCO_CHANNEL_B)
        {
            return FALSE;
        }
    }

    /* Mark the endpoint running before we create a kick timer, just in case it
     * fires before returning. */
    ep->state.spdif.running = TRUE;

    PL_PRINT_P1(TR_STREAM, "stream_IS_SPDIF_ENDPOINT_start: starting kick timer in %d us\n",
                STREAM_KICK_PERIOD_TO_USECS(ep->state.spdif.kick_period));

    if(ep->state.spdif.channel_order != SPCO_CHANNEL_B)
    {
        /* Schedule the kick timer for this chain. */
        timer_schedule_event_in_atomic(
            STREAM_KICK_PERIOD_TO_USECS(ep->state.spdif.kick_period),
            kick_obj_kick, (void*)ko, &ep->state.spdif.kick_id);
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
    if(!ep->state.spdif.running)
    {
        /* The kick source is already stopped */
        return FALSE;
    }

    /* This needs to be atomic, to make sure the right timer gets cancelled */
    LOCK_INTERRUPTS;
    if(TIMER_ID_INVALID != ep->state.spdif.kick_id)
    {
        /* This was the last transform in the sync list. Cancel any timer */
        timer_cancel_event(ep->state.spdif.kick_id);
        ep->state.spdif.kick_id = TIMER_ID_INVALID;
    }
    UNLOCK_INTERRUPTS;

    ep->state.spdif.running = FALSE;

    return TRUE;
}

/**
 * \brief handle detection of higher sample rates
 *
 * \param endpoint Pointer to the endpoint this function is being called on.
 *
 * \return TRUE/FALSE success or failure
 */
static void spdif_handle_bluecore_highrates(ENDPOINT *endpoint)
{
    struct spdif_extra_states *extra = endpoint->state.spdif.extra;

    /* FW has told us that rate is a higher rate, this will mean that
     * DSP is fully responsible to detect which higher rate it is, and
     * if it cannot detect any higher rate then it has to restart the
     * interface, other wise fw will not search for a new rate.
     */
    if(!extra -> can_restart_interface ||
       IS_SPDIF_HIGH_RATE(extra->dsp_sample_rate))
    {
        /* DSP has detected a higher rate, or
         * there is no need to restart the interface
         * for other reasons (e.g, already been done, or
         * it's in progress.
         */
        extra -> highrate_timer_time = hal_get_time();
        return;
    }

    /* see if it's time to restart the interface,
     * to avoid message flooding when fw cant detect
     * it right, we wait for one second before
     * starting the process.
     */
    if((extra->confirms_pending == 0) &&
       (extra->stream_valid ||
        (time_sub(hal_get_time(),extra -> highrate_timer_time) > SECOND)))
    {
        unsigned msg[2];
        /* We now start the process, it cannot be
         * restarted again
         */
        extra->can_restart_interface = 0;

        /* de-activate the interface */
        msg[0] = endpoint->state.spdif.portid;
        msg[1] = 0;
        send_non_baton_message(XAP_KAL_MSG_ACTIVATE_AUDIO, 2, msg);
        extra->confirms_pending++;

        /* same for the its twin endpoint if exists */
        if(endpoint->state.spdif.twin_endpoint)
        {
            msg[0] = endpoint->state.spdif.twin_endpoint->state.spdif.portid;
            msg[1] = 0;
            send_non_baton_message(XAP_KAL_MSG_ACTIVATE_AUDIO, 2, msg);
            extra->confirms_pending++;
        }
    }
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
    if (kick_dir == STREAM_KICK_INTERNAL && endpoint->state.spdif.channel_order != SPCO_CHANNEL_B)
    {
        struct spdif_extra_states *extra = endpoint->state.spdif.extra;
        TIME current_time = hal_get_time();
        unsigned amount_to_read =  cbuffer_calc_amount_data(endpoint->state.spdif.source_buf);
        unsigned amount_to_write = cbuffer_calc_amount_space(endpoint->state.spdif.sink_buf);
        unsigned int interleaved = 0;
        unsigned silence_to_insert = 0;
        ENDPOINT *endpoint_b = NULL;

        /* The output is always 24bit */
        unsigned int shift_amount = 24 - endpoint->state.spdif.input_width;

        /* update the actual read interval */
        extra->read_interval = time_sub(current_time, extra->last_read_time);
        extra->last_read_time = current_time;

        if(endpoint->state.spdif.channel_order == SPCO_CHANNEL_A)
        {
            /* process channel A and Channel B*/
            endpoint_b = endpoint->state.spdif.twin_endpoint;
            amount_to_read = MIN(amount_to_read, cbuffer_calc_amount_data(endpoint_b->state.spdif.source_buf));
            amount_to_write = MIN(amount_to_write, cbuffer_calc_amount_space(endpoint_b->state.spdif.sink_buf));
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

        /* extra process needed when dealing with higher rates */
        if(IS_SPDIF_HIGH_RATE(extra->fw_sample_rate))
        {
            spdif_handle_bluecore_highrates(endpoint);
        }

        /* stream is valid if both FW and DSP agrees on sample rate, or
         * at least they agree that the sample rate is a valid high rate
         */
        extra->stream_valid =
            (extra->fw_sample_rate != 0) &&
            ((extra->dsp_sample_rate == extra->fw_sample_rate) ||
             (IS_SPDIF_HIGH_RATE(extra->fw_sample_rate) && IS_SPDIF_HIGH_RATE(extra->dsp_sample_rate)));

        if(extra->stream_valid)
        {
            /* set the final sample rate
             * this rate doesn't change until next
             * valid sample rate
             */
            endpoint->state.spdif.sample_rate = extra->dsp_sample_rate;
            if(endpoint_b)
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
            if(endpoint_b)
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
        if(silence_to_insert)
        {
            spdif_copy_raw_data(endpoint->state.spdif.sink_buf,
                                endpoint->state.spdif.source_buf,
                                0,
                                silence_to_insert<<interleaved,
                                shift_amount);
            if(endpoint_b)
                spdif_copy_raw_data(endpoint_b->state.spdif.sink_buf,
                                    endpoint_b->state.spdif.source_buf,
                                    0,
                                    silence_to_insert,
                                    shift_amount);

        }

        /* kick forward if anything written to outputs
         */
        if(amount_to_write ||
           amount_to_read ||
           (extra->pause_state==SPEPS_ACTIVE_PAUSE))
        {
            propagate_kick(endpoint, STREAM_KICK_FORWARDS);
            /* Normally B type won't generate kick */
            if(endpoint_b)
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

    /* This is a periodic timer so ask timers when it was scheduled to fire and
     * use that to schedule the next one. */
    next_fire_time = time_add(get_last_fire_time(), endpoint->state.spdif.kick_period);

    endpoint->state.spdif.kick_id = timer_schedule_event_at(next_fire_time,
                                                            kick_obj_kick, (void*)ko);

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
    switch(key)
    {
        case EP_DATA_FORMAT:
            return(value == SPDIF_INPUT_DATA_FORMAT);
        /* TODO
         * - silence duration
         * - channel status select
         */
        default:
            return FALSE;
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
static bool spdif_get_config(ENDPOINT *endpoint, unsigned key, ENDPOINT_GET_CONFIG_RESULT *result)
{
    switch(key)
    {
        /* generic data format of the endpoint */
        case EP_DATA_FORMAT:
            result->u.value = SPDIF_INPUT_DATA_FORMAT;
            return TRUE;

            /* sample rate */
        case EP_SAMPLE_RATE:
            if(endpoint->state.spdif.channel_order == SPCO_CHANNEL_B)
                /* B-type info is stored in its twin endpoint */
                endpoint = endpoint->state.spdif.twin_endpoint;
            if(endpoint->state.spdif.extra->stream_valid)
                result->u.value = endpoint->state.spdif.sample_rate;
            else
                result->u.value = 0;
            return TRUE;

        case EP_RATEMATCH_ABILITY:
            /* s/pdif stream can only be consumed by
             * a spdif_decode op
             */
            result->u.value = (uint32)RATEMATCHING_SUPPORT_NONE;
            return TRUE;

        case EP_RATEMATCH_RATE:
            if(endpoint->state.spdif.channel_order == SPCO_CHANNEL_B)
                /* B-type info is stored in its twin endpoint */
                endpoint = endpoint->state.spdif.twin_endpoint;

            /* get the rate inaccuracy */
            result->u.value = endpoint->state.spdif.extra->norm_rate_ratio;
            return TRUE;

        case EP_RATEMATCH_MEASUREMENT:
            if(endpoint->state.spdif.channel_order == SPCO_CHANNEL_B)
                /* B-type info is stored in its twin endpoint */
                endpoint = endpoint->state.spdif.twin_endpoint;

            /* get the rate inaccuracy */
            result->u.rm_meas.sp_deviation =
                    STREAM_RATEMATCHING_RATE_TO_FRAC(
                            endpoint->state.spdif.extra->norm_rate_ratio);
            result->u.rm_meas.measurement.valid = FALSE;
            return TRUE;

        case EP_CHANNEL_ORDER:
            result->u.value = (uint32) endpoint->state.spdif.channel_order;
            return TRUE;

        default:
            return FALSE;
    }
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
    time_info->period = (STREAM_KICK_PERIOD_FROM_USECS(1000000UL) / SPDIF_TYPICAL_SAMPLE_RATE);
    time_info->block_size = 1;
    time_info->has_deadline = FALSE;

    /* spdif rx is always slave */
    time_info->locally_clocked = endpoint->state.spdif.locally_clocked;

    return;
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

    /* no synchronisation done here, there is no need for the client
     * to send sync message for S/PDIF endpoints, however(as a penalty)
     * if it is called we do check to make sure that the endpoint grouping
     * is right.
     */
    if(ep2 != NULL)
    {
        if(ep1->state.spdif.channel_order == SPCO_CHANNEL_A &&
           ep2->state.spdif.channel_order == SPCO_CHANNEL_B &&
           ep2 == ep1->state.spdif.twin_endpoint)
            return TRUE;

        if(ep2->state.spdif.channel_order == SPCO_CHANNEL_A &&
           ep1->state.spdif.channel_order == SPCO_CHANNEL_B &&
           ep1 == ep2->state.spdif.twin_endpoint)
            return TRUE;
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
