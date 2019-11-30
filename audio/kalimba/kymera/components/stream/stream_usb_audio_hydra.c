/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
 ****************************************************************************/
/**
 * \file  stream_usb_audio_hydra.c
 * \ingroup stream
 *
 * stream usb_audio type file. <br>
 * This file contains stream functions for usb audio endpoints. <br>
 *
 */
#include "stream_private.h"
#include "adaptor/adaptor.h"
#include "audio_data_service.h"
#include "audio_data_service_meta.h"
#include "buffer.h"
#include "hydra_log/hydra_log.h"
#include "limits.h"
#include "pl_assert.h"
#include "pmalloc/pl_malloc.h"
#include "stream.h"
#include "stream_endpoint.h"
#include "stream_for_audio_data_service.h"
#include "stream_kick_obj.h"
#include "types.h"
#include "util.h"
#include "hydra_mmu_buff.h"
#include "hydra_cbuff.h"
#include "stream_endpoint_usb_audio.h"
#include "stream_for_usb_audio_operator.h"
#include "mib/mib.h"
#include "platform/pl_fractional.h"
#include "ttp/timed_playback.h"

#ifndef INSTALL_METADATA
#error "USB Audio endpoint requires Kymera metadata support"
#endif
/****************************************************************************
Private Type Declarations
*/
#define USB_AUDIO_RATE_MEASURE_DURATION_US      49500             /* measurement update every 50 packets, (mid-way between 49 and 50ms) */
#define USB_AUDIO_RATE_MEASURE_HIST_LEN         20                /* average rate in last second (50*20=1000 packets) */
#define USB_AUDIO_RATE_MEASURE_SMOOTHING_FACTOR  FRACTIONAL(0.01) /* calculate rate is smoothed, this is an arbitrarily value
                                                                   * but provides adequate tracking performance for 50ms updating period  */
#define USB_AUDIO_RATE_MEASURE_MAX              FRACTIONAL(0.01)  /* maximum expected mismatch rate */
#define USB_AUDIO_RATE_MEASURE_SHIFT            6                 /* used for better smoothing performance
                                                                   * Note: make sure
                                                                   * (USB_AUDIO_RATE_MEASURE_MAX<<USB_AUDIO_RATE_MEASURE_SHIFT)
                                                                   * doesn't exceed 1.0
                                                                   */
typedef struct
{
    /* audio packets received so far */
    unsigned tot_audio_packets;

    /* end time for this record */
    TIME end_time;
} usb_audio_packets_element;

typedef struct usb_audio_rate_measure
{
    /* current index */
    unsigned hist_index;

    /* history is a ring buffer */
    bool hist_is_full;

    /* total number of packets read/written */
    unsigned cur_tot_packets;

    /* start time for current measurement */
    TIME cur_start_time;

    /* calculate mismatch rate (x 64)
     * Note: sp_adjust is 6 bit shifted for higher precision
     */
    int sp_adjust;

    /* short history of measurements for averaging */
    usb_audio_packets_element hist[USB_AUDIO_RATE_MEASURE_HIST_LEN+1];
};

/****************************************************************************
Private Function Declarations
*/
static bool usb_audio_close (ENDPOINT *ep);
static bool usb_audio_connect (ENDPOINT *ep, tCbuffer *cb,  ENDPOINT *ep_to_kick, bool* start_on_connect);
static bool usb_audio_disconnect (ENDPOINT *ep);
static bool usb_audio_buffer_details (ENDPOINT *ep, BUFFER_DETAILS *details);
static void usb_audio_kick(ENDPOINT *ep, ENDPOINT_KICK_DIRECTION kick_dir);
static bool usb_audio_start (ENDPOINT *ep, KICK_OBJECT *ko);
static bool usb_audio_stop (ENDPOINT *ep);
static bool usb_audio_configure (ENDPOINT *ep, unsigned int key, uint32 value);
static bool usb_audio_get_config (ENDPOINT *ep, unsigned int key, ENDPOINT_GET_CONFIG_RESULT *result);
static void usb_audio_get_timing (ENDPOINT *ep, ENDPOINT_TIMING_INFORMATION *time_info);
static void audio_data_service_usb_audio_kick(void* priv, ENDPOINT_KICK_DIRECTION kick_dir);
static void usb_audio_tx_update_packet_length(endpoint_usb_audio_state *usb_audio);
static void usb_audio_tx_transfer_metadata(endpoint_usb_audio_state *usb_audio, unsigned frames_consumed, unsigned frames_written);
static void usb_audio_sched_kick (ENDPOINT *endpoint, KICK_OBJECT *ko);
static void usb_audio_tx_enable_time_event(ENDPOINT *ep, KICK_OBJECT *ko, bool enable);
static bool usb_audio_tx_get_packet_ttp_error(endpoint_usb_audio_state *usb_audio, TIME cur_time, TIME_INTERVAL *error);
static void usb_audio_tx_update_last_read_time_stamp(endpoint_usb_audio_state *usb_audio, unsigned frames_read);
static void usb_audio_tx_update_timer_period_adjust(endpoint_usb_audio_state *usb_audio, unsigned packets_sent);
static void usb_audio_tx_reset_timer_period_adjust(endpoint_usb_audio_state *usb_audio);
static void usb_audio_tx_control_ttp_error(endpoint_usb_audio_state *usb_audio, TIME_INTERVAL error, unsigned input_frames_left, unsigned packets_sent);
static void usb_audio_rate_measure_update(endpoint_usb_audio_state *usb_audio, TIME current_time, unsigned n_packets);
static void usb_audio_rate_measure_reset(endpoint_usb_audio_state *usb_audio, TIME current_time);

DEFINE_ENDPOINT_FUNCTIONS(usb_audio_functions, usb_audio_close, usb_audio_connect,
                          usb_audio_disconnect, usb_audio_buffer_details,
                          usb_audio_kick, usb_audio_sched_kick,
                          usb_audio_start, usb_audio_stop,
                          usb_audio_configure, usb_audio_get_config,
                          usb_audio_get_timing, stream_sync_sids_dummy);

/****************************************************************************
Public Function Definitions
*/

/**
 * \brief configures the usb audio endpoint
 *
 * \param *endpoint pointer to the endpoint that we want to configure
 * \param sample_rate sample rate of the endpoint
 * \param nrof_channels number of channels in the usb audio connection
 * \param subframe_size size of subframe in number of bits
 * \return TRUE if configured successfully else FALSE
 */
bool stream_usb_audio_configure_ep(ENDPOINT *ep, unsigned sample_rate, unsigned nrof_channels, unsigned subframe_size)
{
    patch_fn_shared(stream_usb_audio_hydra);

    endpoint_usb_audio_state *usb_audio;

    /* make sure this is really a usb audio endpoint */
    if(ep->stream_endpoint_type != endpoint_usb_audio)
    {
        return FALSE;
    }

    usb_audio = &ep->state.usb_audio;

    /* make sure endpoint isn't running */
    if(usb_audio->running)
    {
        return FALSE;

    }

    /* 16, 24, and 32 bit frame sizes are supported
     * by this endpoint, although we currently don't have a use
     * case to support 32 bit.
     */
    if(subframe_size != 16 &&
       subframe_size != 24 &&
       subframe_size != 32)
    {
        return FALSE;
    }

    /* set sample rate, sample rate */
    usb_audio->sample_rate = sample_rate;

    /* set number of channels */
    usb_audio->n_channels = nrof_channels;

    /* subframe size in octets, for optimisation only */
    usb_audio->subframe_in_octets = subframe_size / 8;

    /* frame size in octets, for optimisation only */
    usb_audio->frame_in_octets = usb_audio->subframe_in_octets * nrof_channels;

    /* integer part of packet length, used in Rx as well */
    usb_audio->frames_in_packet_int = sample_rate / USB_AUDIO_PACKET_RATE_HZ;

    if(ep->direction == SINK)
    {
        unsigned min_buff_size;
        unsigned max_packet_size;

        /* calculate some variables used only for Tx packetising,
         * the division is calculated here to avoid per packet division
         */
        usb_audio->frames_in_packet_rem = sample_rate % USB_AUDIO_PACKET_RATE_HZ;
        usb_audio->packet_rem_acc = 0;
        usb_audio->packet_len_in_frames = usb_audio->frames_in_packet_int;

        /* max packet size in octets */
        max_packet_size = usb_audio->frames_in_packet_int * usb_audio->frame_in_octets;
        if(usb_audio->frames_in_packet_rem != 0)
        {
            max_packet_size += usb_audio->frame_in_octets;
        }

        /* min buffer size should be able to contain the required number of packets */
        min_buff_size = max_packet_size * usb_audio->max_packets_in_tx_output_buffer  + ADDR_PER_WORD;

        /* return failure if buffer is not large enough */
        if(min_buff_size > cbuffer_get_size_in_addrs(usb_audio->sink_buf))
        {
            /* not large enough buffer */
            return FALSE;
        }
    }

    /* set subframe size, this is the last thing to do as it is used to
     * see whether configuration has been don successfully
     */
    usb_audio->subframe_size = subframe_size;

    L2_DBG_MSG5("EP_USB_AUDIO, CONFIG RECEIVED, time=%08x, dir=%d, sample rate=%dHz, channels=%d, frame_size=%dbits",
                hal_get_time(),
                ep->direction,
                ep->state.usb_audio.sample_rate,
                ep->state.usb_audio.n_channels,
                ep->state.usb_audio.subframe_size);

    return TRUE;
}

/**
 * stream_usb_audio_can_enact_rate_adjust
 * \brief querying the endpoint whether it can enact rate adjustment
 *
 * \param endpoint pointer to the endpoint that we want to query
 * \param rate_adjust_val if endpoint enacting the address of rate
 *        adjust value will be returned on this parameter, else NULL will
 *        be returned.
 * \return TRUE if query was successful else FALSE;
 */
bool stream_usb_audio_can_enact_rate_adjust(ENDPOINT *ep, unsigned **rate_adjust_val)
{
    patch_fn_shared(stream_usb_audio_hydra);

    /* make sure this is really a usb audio endpoint */
    if(ep->stream_endpoint_type != endpoint_usb_audio ||
       ep->direction != SINK)
    {
        return FALSE;
    }

    /* make sure endpoint isn't running */
    if(ep->state.usb_audio.running)
    {
        return FALSE;
    }

    /* This EP is to perform rate adjustment */
    *rate_adjust_val = &ep->state.usb_audio.target_rate_adjust_val;

    return TRUE;
}

/**
 * stream_usb_audio_set_cbops_sra_op
 * \brief sets the cbops op that is used for rate adjustment
 *
 * \param endpoint pointer to the usb audio endpoint
 * \param rate_adjust_op pointer to the rate adjustment op
 */
void stream_usb_audio_set_cbops_sra_op(ENDPOINT *ep, cbops_op* rate_adjust_op)
{
    patch_fn_shared(stream_usb_audio_hydra);

    /* No check is needed to do here, as all the checks have been done
     * when querying the enactment
     */

    /* the rate adjust operator, in TTP mode the endpoint will
     * regularly updates the rate, in non TTP mode this will
     * be provided by the rate match manager
     */
    ep->state.usb_audio.rate_adjust_op = rate_adjust_op;

    /* non pass-through mode if enacting is in this side */
    cbops_rateadjust_passthrough_mode(ep->state.usb_audio.rate_adjust_op,
                                      !ep->state.usb_audio.ep_ratematch_enacting);
}

/*
 * stream_query_usb_audio
 */
bool stream_query_usb_audio(AUDIO_DATA_SERVICE_EP_PARAMS *params)
{
    unsigned buff_size = 0;

    patch_fn_shared(stream_usb_audio_hydra);

    L2_DBG_MSG1("EP_USB_AUDIO, QUERY RECEIVED, time=%08x", hal_get_time());

#ifdef BAC32
    params->data_buf_flags = BUF_DESC_UNPACKED_32BIT_MASK;
#else
    params->data_buf_flags = 0;
#endif

    params->start_kicks_immediately = FALSE;
    if(params->dir == SINK)
    {
        /* USB Tx has metadata enabled */
        if(params->meta_buf_size != 0)
        {
            params->meta_buf_size = MAX(params->meta_buf_size, USB_AUDIO_TX_METADATA_BUFFER_SIZE);
        }

        /* get the buffer size required */
        buff_size = mibgetrequ16(USBSINKOUTPUTBUFFERSIZE);
        if(0 == buff_size)
        {
            buff_size = USB_TYPICAL_AUDIO_BUFFER_SIZE* OCTETS_PER_SAMPLE;
        }

        /* USB Tx doesn't need any kick from audio data service side */
        params->kick_fn = NULL;
    }
    else
    {
        /* USB Rx doesn't receive metadata */
        params->meta_buf_size = 0;
        params->kick_fn = audio_data_service_usb_audio_kick;

        /* get the buffer size required */
        buff_size = mibgetrequ16(USBSOURCEINPUTBUFFERSIZE);
        if(0 == buff_size)
        {
            buff_size = USB_TYPICAL_AUDIO_BUFFER_SIZE* OCTETS_PER_SAMPLE;
        }
    }

    /* Data service buffer is in octets */
    params->data_buf_size = MAX(params->data_buf_size, buff_size);

    return TRUE;
}

/*
 * stream_create_usb_audio
 */
void stream_create_usb_audio(const AUDIO_DATA_SERVICE_EP_CFG *cfg, AUDIO_DATA_SERVICE_EP_CREATE_RESULT* create_res)
{
    ENDPOINT *ep;
    endpoint_usb_audio_state *usb_audio;
    patch_fn_shared(stream_usb_audio_hydra);

    L2_DBG_MSG2("EP_USB_AUDIO, CREATE RECEIVED, time=0x%08x dir=%d",  hal_get_time(), cfg->dir);
    ep = STREAM_NEW_ENDPOINT(usb_audio, cfg->key, cfg->dir, INVALID_CON_ID);
    if (!ep)
    {
        create_res->success = FALSE;
        return;
    }
    usb_audio = &ep->state.usb_audio;
    usb_audio->service_priv = cfg->service_priv;

    /* allocation for rate measure struct */
    usb_audio->rate_measure = xzpnew (usb_audio_rate_measure);
    if(NULL ==  usb_audio->rate_measure)
    {
        create_res->success = FALSE;
        stream_destroy_endpoint(ep);
        return;
    }

    ep->can_be_closed = FALSE;
    ep->can_be_destroyed = FALSE;
    ep->is_real = TRUE;
    if (cfg->dir == SOURCE)
    {
        usb_audio->source_buf = cfg->data_buf;
    }
    else
    {
        usb_audio->sink_buf = cfg->data_buf;

        /* TX endpoint is kicked by timer ISR, set the kick period */
        usb_audio->kick_period = USB_AUDIO_TX_KICK_PERIOD_US;

        /* max packets in the output buffer, minimum and default is 3 but can be
         * configured using mib key
         */
        usb_audio->max_packets_in_tx_output_buffer = mibgetrequ16(USBSINKMAXPACKETSINOUTPUTBUFFER);
        usb_audio->max_packets_in_tx_output_buffer = MAX(usb_audio->max_packets_in_tx_output_buffer,
                                                         USB_AUDIO_TX_MAX_PACKET_IN_OUTPUT_BUFFER);

        /* initialise pid controller parameters */
        timed_set_pid_controller_default_settings(&usb_audio->pid_params);

        /* set pid controller p factor,
         * use USB-Tx specific config
         */
        timed_set_pid_controller_pfactor(&usb_audio->pid_params,
                                         (int)(mibgetreqi32(USBSINKTIMEDPLAYBACKPFACTOR) >> (32 - DAWTH)));

        /* set pid controller i factor,
         * use USB-Tx specific config
         */
        timed_set_pid_controller_ifactor(&usb_audio->pid_params,
                                         (int)(mibgetreqi32(USBSINKTIMEDPLAYBACKIFACTOR) >> (32 - DAWTH)));

    }
    ep->deferred.config_deferred_kick = TRUE;

    /* initialise the rate mismatch ratio */
    usb_audio->norm_rate_ratio = RM_PERFECT_RATE;

    create_res->priv = (void*) ep;
    create_res->ext_ep_id = stream_external_id_from_endpoint(ep);

    create_res->success = TRUE;
}

/*
 * stream_destroy_usb_audio
 */
bool stream_destroy_usb_audio(void *ep)
{
    patch_fn_shared(stream_usb_audio_hydra);
    ENDPOINT *endpoint = (ENDPOINT *)ep;

    L2_DBG_MSG2("EP_USB_AUDIO, DESTROY RECEIVED, time=0x%08x dir=%d", hal_get_time(), ((ENDPOINT*)ep)->direction);

    endpoint->can_be_closed = TRUE;
    endpoint->can_be_destroyed = TRUE;

    /* free allocated rate_measure structure */
    pfree(endpoint->state.usb_audio.rate_measure);
    endpoint->state.usb_audio.rate_measure = NULL;

    return (stream_close_endpoint(endpoint));
}

/****************************************************************************
Private Function Definitions
*/

/**
 * \brief resets the mismatch rate measurement process
 * \param usb_audio pointer to the usb_audio state structure
 * \param current_time current time
 */
static void usb_audio_rate_measure_reset(endpoint_usb_audio_state *usb_audio, TIME current_time)
{
    usb_audio_rate_measure *rate_measure = usb_audio->rate_measure;

    patch_fn_shared(stream_usb_audio_hydra);

    if(NULL == rate_measure)
    {
        return;
    }

    /* reset the start time and packets in hist */
    rate_measure->cur_start_time = current_time;
    rate_measure->cur_tot_packets = 0;

    /* This will reset the history */
    rate_measure->hist_is_full = FALSE;
    rate_measure->hist_index = 0;
}

/**
 * \brief run the mismatch rate measurement process
 * \param usb_audio pointer to the usb_audio state structure
 * \param current_time current time
 * \param n_packets number of new audio packets read/written since last update
 */

static void usb_audio_rate_measure_update(endpoint_usb_audio_state *usb_audio, TIME current_time, unsigned n_packets)
{
    usb_audio_rate_measure *rate_measure = usb_audio->rate_measure;
    unsigned time_passed;

    patch_fn_shared(stream_usb_audio_hydra);

    if((n_packets == 0) || (NULL == rate_measure))
    {
        /* no update if link isn't active, if this
         * continues the rate measurement  will be reset
         */
        return;
    }

    /* time passed since previous measurement */
    time_passed = (unsigned) time_sub(current_time, rate_measure->cur_start_time);
    if(time_passed > (USB_AUDIO_RATE_MEASURE_DURATION_US*2))
    {
        /* hasn't been run for long time, reset the process */
        usb_audio_rate_measure_reset(usb_audio, current_time);
        return;
    }

    /* accumulate number of packets received/sent */
    rate_measure->cur_tot_packets += n_packets;

    /* see if time to do a new measurement */
    if(time_passed > USB_AUDIO_RATE_MEASURE_DURATION_US)
    {
        /* update history */
        unsigned newest_index = rate_measure->hist_index;
        unsigned oldest_index;
        rate_measure->hist[newest_index].tot_audio_packets = rate_measure->cur_tot_packets;
        rate_measure->hist[newest_index].end_time = current_time;
        rate_measure->cur_start_time = current_time;
        rate_measure->hist_index++;
        /*Note: hsit index [0 to USB_AUDIO_RATE_MEASURE_HIST_LEN] inclusive */
        if(rate_measure->hist_index > USB_AUDIO_RATE_MEASURE_HIST_LEN)
        {
            rate_measure->hist_index = 0;
            rate_measure->hist_is_full = TRUE;
        }

        /* averaging over the history of records, at the beginning
         * history isn't full, so averaging will be done over shorter
         * time
         */
        oldest_index = rate_measure->hist_is_full? rate_measure->hist_index:0;

        if(oldest_index != newest_index)
        {
            /* number of packets in the hist buff */
            unsigned packets = rate_measure->cur_tot_packets - rate_measure->hist[oldest_index].tot_audio_packets;

            /* total duration represented by hist buff */
            unsigned dur = (unsigned) time_sub(current_time, rate_measure->hist[oldest_index].end_time);

            /* subtract expected duration */
            int drift = dur - (int)packets*(SECOND/USB_AUDIO_PACKET_RATE_HZ);

            /* mismatch ratio */
            int rm = frac_div(drift, (int)dur);

            /* limit mismatch ratio */
            rm = MIN(rm, USB_AUDIO_RATE_MEASURE_MAX);
            rm = MAX(rm, -USB_AUDIO_RATE_MEASURE_MAX);

            /* smooth averaging, Note: sp_adjust is 6 bit shifted for higher precision */
            rate_measure->sp_adjust += frac_mult(rm*(1<<USB_AUDIO_RATE_MEASURE_SHIFT)-rate_measure->sp_adjust,
                                                 USB_AUDIO_RATE_MEASURE_SMOOTHING_FACTOR);

            /* convert sp_adjust to format needed by rate match manager */
            usb_audio->norm_rate_ratio = RM_PERFECT_RATE - frac_mult(rate_measure->sp_adjust,
                                                                     RM_PERFECT_RATE>>USB_AUDIO_RATE_MEASURE_SHIFT);

            STREAM_USB_AUDIO_DBG_MSG5("Rate measure: measure=0x%x, packets=%d, dur=%d, rm=%d, sp_adjust=%d",
                        (unsigned) rate_measure,
                        packets,
                        dur,
                        rm,
                        rate_measure->sp_adjust>>USB_AUDIO_RATE_MEASURE_SHIFT);
        }
    }
}

/**
 * \brief closes the usb_audio Rep
 * \param DP pointer to the endpoint that is being closed
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool usb_audio_close(ENDPOINT *ep)
{
    L2_DBG_MSG2("EP_USB_AUDIO, CLOSE RECEIVED,time=0x%08x, dir=%d", hal_get_time(), ep->direction);

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
static bool usb_audio_connect(ENDPOINT *ep, tCbuffer *cb,  ENDPOINT *ep_to_kick, bool* start_on_connect)
{
    endpoint_usb_audio_state *usb_audio = &ep->state.usb_audio;

    ep->ep_to_kick = ep_to_kick;
    if (ep->direction == SOURCE)
    {
        usb_audio->sink_buf = cb;
        L2_DBG_MSG3("EP_USB_AUDIO, CONNECT RECEIVED, time=0x%08x, dir=SOURCE, source_buf=%d (octets), sink_buf=%d (words)",
                    hal_get_time(), cbuffer_get_size_in_addrs(cb) , cbuffer_get_size_in_words(usb_audio->source_buf));
    }
    else
    {
        usb_audio->source_buf = cb;
        L2_DBG_MSG3("EP_USB_AUDIO, CONNECT RECEIVED, time=0x%08x, dir=SINK, source_buf=%d (words), sink_buf=%d (octets)",
                    hal_get_time(), cbuffer_get_size_in_words(cb) , cbuffer_get_size_in_addrs(usb_audio->sink_buf));
    }
    *start_on_connect = FALSE;
    return TRUE;
}

/**
 * \brief disconnect from the usb_audio ep
 * \param ep pointer to the endpoint that is being connected
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool usb_audio_disconnect(ENDPOINT *ep)
{
    endpoint_usb_audio_state *usb_audio = &ep->state.usb_audio;

    L2_DBG_MSG2("EP_USB_AUDIO, DISCONNECT RECEIVED, time=0x%08x dir=%d", hal_get_time(), ep->direction);

    if (ep->direction == SOURCE)
    {
        usb_audio->sink_buf = NULL;
    }
    else
    {
        usb_audio->source_buf = NULL;
    }

    ep->ep_to_kick = NULL;

    /* config parameters are only valid for one
     * connection, here we invalidate them by setting the
     * subframe size to 0 */
    usb_audio->subframe_size = 0;

    return TRUE;
}

/**
 * usb_audio_tx_update_packet_length
 * \brief updates the length of next USB TX packet
 * \param usb_audio pointer to the usb_audio state structure
 *
 */
static void usb_audio_tx_update_packet_length(endpoint_usb_audio_state *usb_audio)
{
    if(usb_audio->frames_in_packet_rem == 0)
    {
        /* For optimisation only,
         * the packet length is fixed and calculated at config time */
        return;
    }

    /* set the packet len to default (integer part of packet length) */
    usb_audio->packet_len_in_frames = usb_audio->frames_in_packet_int;

    /* accumulate the fraction part(remainder) until it
     * becomes a full-frame worth of audio. This will ensure the
     * correct average rate of packets.
     */
    usb_audio->packet_rem_acc += usb_audio->frames_in_packet_rem;
    if(usb_audio->packet_rem_acc >= USB_AUDIO_PACKET_RATE_HZ)
    {   /* next packet will be one frame longer */
        usb_audio->packet_rem_acc -= USB_AUDIO_PACKET_RATE_HZ;
        usb_audio->packet_len_in_frames++;
    }
    return;
}

/**
 * usb_audio_tx_get_packet_ttp_error
 * \brief get the TTP error for next packet
 * \param usb_audio pointer to the usb_audio state structure
 * \param cur_time current time
 * \param error TTP error for next packet
 * return whether the error is valid
 */
static bool usb_audio_tx_get_packet_ttp_error(endpoint_usb_audio_state *usb_audio, TIME cur_time, TIME_INTERVAL *error)
{
    patch_fn_shared(stream_usb_audio_hydra);

    if (buff_has_metadata(usb_audio->source_buf))
    {
        /* see if input has a timestamped tag */
        unsigned b4idx = 0;
        metadata_tag *mtag = buff_metadata_peek_ex(usb_audio->source_buf, &b4idx);
        if((mtag != NULL) && (IS_TIMESTAMPED_TAG(mtag)))
        {

            /* if this tag is time stamped we can calculate the time stamp for output fairly accurately */
            unsigned frames_b4 = b4idx / (OCTETS_PER_SAMPLE * usb_audio->n_channels);

            /* go back to first input sample */
            TIME_INTERVAL time_back = convert_samples_to_time(frames_b4, usb_audio->sample_rate);

            /* set the time stamp */
            usb_audio->last_read_timestamp = time_sub(mtag->timestamp, time_back);

            /* this will stay valid */
            usb_audio->last_read_timestamp_valid = TRUE;

            if(!usb_audio->timed_playback)
            {
                /* endpoint is receiving timestamped input we
                   can now go to timed playback mode */
                usb_audio->timed_playback = TRUE;

                /* The rate adjustment is TTP based and only in the USB side */
                cbops_rateadjust_passthrough_mode(usb_audio->rate_adjust_op, FALSE);

                /* pid controller will be started */
                timed_reset_pid_controller(&usb_audio->pid_state);

                L2_DBG_MSG("EP_USB_AUDIO TX: Switched to timed playback mode");
            }
        }
    }

    if(usb_audio->last_read_timestamp_valid)
    {
        /* we have valid time stamp, so we can compute error,
         * we are calculating the error for next packet, in average
         * we are sending USB_AUDIO_TX_PACKETS_PER_KICK packets each kick,
         * but we could send less or more than that in individual kicks,
         * we consider the packet offset to avoid having sudden jumps in the
         * calculated error
         * Note: This is only needed for timed playback mode, the offset is 0 when
         * not in playback mode, so no need to check it.
         */
        TIME_INTERVAL offset_time_back =
            (TIME_INTERVAL) ((int)usb_audio->timer_period_adjust_packet_offset*(int)usb_audio->kick_period) /
            (int) USB_AUDIO_TX_PACKETS_PER_KICK;

        *error = time_sub(usb_audio->last_read_timestamp, cur_time);
        *error = *error - offset_time_back;
    }
    return usb_audio->last_read_timestamp_valid;
}

/**
 * usb_audio_tx_update_last_read_time_stamp
 * \brief works out the time stamp for next packet
 * \param usb_audio pointer to the usb_audio state structure
 * \param frames_read amount of frames read from input
 */
static void usb_audio_tx_update_last_read_time_stamp(endpoint_usb_audio_state *usb_audio, unsigned frames_read)
{
    if(usb_audio->last_read_timestamp_valid)
    {
        TIME_INTERVAL time_passed = convert_samples_to_time(frames_read, usb_audio->sample_rate);
        usb_audio->last_read_timestamp = time_add(usb_audio->last_read_timestamp, time_passed);
    }
}

/**
 * usb_audio_tx_transfer_packet_metadata
 * \brief transfer one packet worth of metadata to output buffer
 * \param usb_audio pointer to the usb_audio state structure
 * \param packet_len_in_frames packet length is frames, this is normally
 *        the same as usb_audio->packet_len_in_frames but isn't strictly
 *        required by this function.
 */
static void usb_audio_tx_transfer_metadata(endpoint_usb_audio_state *usb_audio, unsigned frames_consumed, unsigned frames_written)
{
    unsigned b4idx = 0, afteridx;

    patch_fn_shared(stream_usb_audio_hydra);

    /* see if input has metadata */
    if ((frames_consumed != 0 ) &&
        buff_has_metadata(usb_audio->source_buf))
    {
        /* read a frame of metadata from the source buffer
         * Note: No use of the tag info currently
         */
        metadata_tag *in_mtag = buff_metadata_remove(usb_audio->source_buf,
                                                     OCTETS_PER_SAMPLE * frames_consumed * usb_audio->n_channels,
                                                     &b4idx, &afteridx);

        usb_audio_tx_update_last_read_time_stamp(usb_audio, frames_consumed);

        /* delete the input tag, we don't need it any more */
        buff_metadata_tag_list_delete(in_mtag);
    }

    /* For effective packetising, USB TX sink buffer must have metadata available,
     * however we don't panic here as we can gracefully handle it.
     */
    if ((frames_written!=0) &&
        buff_has_metadata(usb_audio->sink_buf))
    {

        /* create new tag for output */
        metadata_tag *out_mtag = buff_metadata_new_tag();

        b4idx = 0;
        afteridx = frames_written * usb_audio->n_channels * ( usb_audio->subframe_size/8);
        if (out_mtag != NULL)
        {
            /* Each tag represent a full packet */
            out_mtag->length = afteridx;
            METADATA_PACKET_START_SET(out_mtag);
            METADATA_PACKET_END_SET(out_mtag);
        }

        /* append the packet metadata tag the output buffer */
        buff_metadata_append(usb_audio->sink_buf, out_mtag, b4idx, afteridx);

        /* This translates metadata tag to audio data service's representation */
        audio_data_service_dequeue_meta(usb_audio->service_priv, afteridx);
    }

    return;
}

/**
 * usb_audio_tx_update_timer_period_adjust
 * \brief adjusts the timer period for usb tx ep based on host consumption
 * \param usb_audio pointer to the usb_audio state structure
 * \param packets_sent number of packets that just sent to host
 */
static void usb_audio_tx_update_timer_period_adjust(endpoint_usb_audio_state *usb_audio, unsigned packets_sent)
{
    patch_fn_shared(stream_usb_audio_hydra);

    /* if we cannot copy, it might be that host isn't consuming
     * any more. In this case we reset the period adjusting and
     * will resume once host becomes active.
     */
    if(packets_sent == 0)
    {
        /* unable to send packet could be that the reader
         * has been busy, so need to see this for a few times
         * in row before deciding that host stalled.
         */
        if(usb_audio->timer_period_adjust_stall_counter < USB_AUDIO_TX_KICK_PERIOD_STALL_DETECTION_THRESHOLD)
        {
            usb_audio->timer_period_adjust_stall_counter++;
            if(usb_audio->timer_period_adjust_stall_counter == USB_AUDIO_TX_KICK_PERIOD_STALL_DETECTION_THRESHOLD)
            {
                /* reset period adjusting process */
                usb_audio_tx_reset_timer_period_adjust(usb_audio);
                return;
            }
        }
        else
        {
            /* We should already be in stall mode */
            return;
        }
    }
    else
    {
        /* as soon as packets are moving we end stall mode */
        usb_audio->timer_period_adjust_stall_counter = 0;
    }

    /* Data is flowing but to continue adjusting process we wait until we are
     * certain that host is consuming
     */
    if(!usb_audio->timer_period_adjust_normal_mode)
    {
        usb_audio->timer_period_adjust_normal_counter++;
        if(usb_audio->timer_period_adjust_normal_counter < USB_AUDIO_TX_KICK_PERIOD_NORMAL_DETECTION_THRESHOLD)
        {
            return;
        }
        /* In normal mode */
        usb_audio->timer_period_adjust_normal_mode = TRUE;
    }

    /* adjust the period, if host consuming faster than expected we
     * decrease the period, if slower we increases it. There are lower
     * and higher limits.
     */
    usb_audio->kick_period = (usb_audio->kick_period + USB_AUDIO_TX_PACKETS_PER_KICK) - packets_sent;
    usb_audio->kick_period = MAX(usb_audio->kick_period, USB_AUDIO_TX_KICK_PERIOD_US_MIN);
    usb_audio->kick_period = MIN(usb_audio->kick_period, USB_AUDIO_TX_KICK_PERIOD_US_MAX);

    /* accumulated difference, this should be very small */
    usb_audio->timer_period_adjust_packet_offset += packets_sent;
    usb_audio->timer_period_adjust_packet_offset -= USB_AUDIO_TX_PACKETS_PER_KICK;
    if(pl_abs_i32(usb_audio->timer_period_adjust_packet_offset) > USB_AUDIO_TX_KICK_PERIOD_MAX_PACKETS_DRIFT)
    {
        /* This can only happen if the host is too slow or too fast than the above limits.
         * We reset the timer period adjustment.
         */
        L2_DBG_MSG2("EP_USB_AUDIO TX: reset timer period adjust - drift is too high, time=%d, drift=%d",
                    hal_get_time(),
                    usb_audio->timer_period_adjust_packet_offset);
        usb_audio_tx_reset_timer_period_adjust(usb_audio);
    }
}

/**
 * usb_audio_tx_reset_timer_period_adjust
 * \brief resets the process of adjusting timer period
 * \param usb_audio pointer to the usb_audio state structure
 */
static void usb_audio_tx_reset_timer_period_adjust(endpoint_usb_audio_state *usb_audio)
{
    patch_fn_shared(stream_usb_audio_hydra);

    usb_audio->kick_period = USB_AUDIO_TX_KICK_PERIOD_US;
    usb_audio->timer_period_adjust_normal_counter = 0;
    usb_audio->timer_period_adjust_normal_mode = FALSE;
    usb_audio->timer_period_adjust_packet_offset = 0;

    /* reset pid controller, error control won't be
     * valid if period adjustment isn't in normal mode
     */
    timed_reset_pid_controller(&usb_audio->pid_state);

    /* reset rate measurement */
    usb_audio_rate_measure_reset(usb_audio, hal_get_time());
}

/**
 * usb_audio_tx_control_ttp_error
 * \brief simple PI controller to minimise the ttp error
 * \param usb_audio pointer to the usb_audio state structure
 * \param error difference between expected ttp and now
 * \param input_frames_left amount of frames left in the input buffer
 * \param packets_sent number of packets sent this kick
 */
static void usb_audio_tx_control_ttp_error(endpoint_usb_audio_state *usb_audio, TIME_INTERVAL error, unsigned input_frames_left, unsigned packets_sent)
{
    int error_diff;

    patch_fn_shared(stream_usb_audio_hydra);

    /* these two parameters aren't needed,
     * just in case patching needed them
     */
    UNUSED(input_frames_left);
    UNUSED(packets_sent);

    /* ignore the error if we have a big jump in error*/
    error_diff = error - usb_audio->ttp_control_prev_error;
    usb_audio->ttp_control_prev_error = error;
    if (pl_abs_i32(error_diff) > USB_AUDIO_TX_TTP_CONTROL_MAX_EXPECTED_ERROR_JUMP_US)
    {
        return;
    }

    /* accumulate error for averaging */
    usb_audio->ttp_control_error_acc_counter++;
    usb_audio->ttp_control_error_acc += error;
    if(usb_audio->ttp_control_error_acc_counter == USB_AUDIO_TX_TTP_CONTROL_ERROR_AVARAGING_PERIOD)
    {
        /* calculate average error over N packets (N=8) */
        int error_avg = frac_mult(usb_audio->ttp_control_error_acc, FRACTIONAL(1.0/USB_AUDIO_TX_TTP_CONTROL_ERROR_AVARAGING_PERIOD));

        /* run controller to minimise the error */
        timed_run_pid_controller(&usb_audio->pid_state, &usb_audio->pid_params, error_avg);
        STREAM_USB_AUDIO_DBG_MSG4("EP_USB_AUDIO: TTP error control, time=%d, error=%d, period=%d,  warp=%-8d",
                                  hal_get_time(),
                                  error_avg,
                                  usb_audio->kick_period,
                                  usb_audio->pid_state.warp);

        /* Set the SRA */
        cbops_sra_set_rate_adjust(usb_audio->rate_adjust_op,
                                  usb_audio->n_channels,
                                  usb_audio->pid_state.warp);

        /* reset the averaging accumulator */
        usb_audio->ttp_control_error_acc_counter = 0;
        usb_audio->ttp_control_error_acc = 0;
    }
}

/**
 * \brief kick the usb_audio ep
 * \param ep pointer to the endpoint that is being kicked
 * \param kick_dir direction of the kick
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static void usb_audio_kick(ENDPOINT *ep, ENDPOINT_KICK_DIRECTION kick_dir)
{
    endpoint_usb_audio_state *usb_audio = &ep->state.usb_audio;
    unsigned subframe_in_octets = usb_audio->subframe_in_octets;
    unsigned frame_in_octets = usb_audio->frame_in_octets;

    if(ep->direction == SOURCE)
    {
        /* USB Rx is kicked only from audio data service */
        PL_ASSERT(kick_dir != STREAM_KICK_BACKWARDS);
    }
    else
    {
        /* USB TX kicks are from periodic time events */
        PL_ASSERT(kick_dir == STREAM_KICK_INTERNAL);
    }

    /* We expect to be in running state */
    PL_ASSERT(usb_audio->running);
    PL_ASSERT(!ep->can_be_closed);

    /* process only when endpoint is connected */
    if(ep->connected_to &&
       usb_audio->subframe_size /* this shows that the endpoint
                                 * has been configured successfully */
       )
    {
        if(ep->direction == SOURCE)
        {
            unsigned subframes_read;
            unsigned data_available_in_octets = cbuffer_calc_amount_data_in_addrs(usb_audio->source_buf);
            unsigned space_available_in_words = cbuffer_calc_amount_space_in_words(usb_audio->sink_buf);
            TIME cur_time = hal_get_time();

            /* see if we have enough space to read the whole data we have */
            if(data_available_in_octets > (space_available_in_words*subframe_in_octets))
            {
                /* we don't have enough space to read the whole packet, this shouldn't happen in practice
                 * if we have a good rate adjustment in place. If it happens we discard the whole packet,
                 * so we always keep space for new data to arrive from host.
                 */
                L2_DBG_MSG2("EP_USB_AUDIO, Rx, NOT ENOUGH SPACE, space=%d, data=%d",
                            space_available_in_words, data_available_in_octets);

                /* discard anything in the input buffer */
                stream_usb_audio_discard_octets_from_source_buffer(usb_audio->source_buf, data_available_in_octets);

                /* buffer has discarded, so update buffer */
                audio_data_service_update_buffers(usb_audio->service_priv);
                audio_data_service_kick_consumer(usb_audio->service_priv);

                /* we also kick the connected operator, it could be that the lack of enough
                 * space was due to the op not being kicked */
                propagate_kick(ep, STREAM_KICK_FORWARDS);

                /* reset rate measurement process */
                usb_audio_rate_measure_reset(usb_audio, cur_time);

                return;
            }

            if(data_available_in_octets%frame_in_octets)
            {
                STREAM_USB_AUDIO_DBG_MSG1("EP_USB_AUDIO, Rx, Looks not healthy stream,  data=%d", data_available_in_octets);
            }

            /* make sure we read integer number of audio frames.*/
            data_available_in_octets = (data_available_in_octets/frame_in_octets)*frame_in_octets;

            if(data_available_in_octets)
            {
                unsigned packets_received;

                /* read subframes into the sink buffer */
                subframes_read = stream_usb_audio_read_audio_subframes(usb_audio->sink_buf,
                                                                       usb_audio->source_buf,
                                                                       usb_audio->subframe_size,
                                                                       data_available_in_octets);

                STREAM_USB_AUDIO_DBG_MSG2("EP_USB_AUDIO, Rx, raw data read,  time=0x%08x, subframes=%d",
                                          hal_get_time(),subframes_read);

                PL_ASSERT(subframes_read == (data_available_in_octets/subframe_in_octets));

                /* work out the number of packets received by dividing to frame size.
                 *
                 * We divide to integer part only, this will always be correct for sample rates
                 * with no frames_in_packet_rem like 8khz or 48khz.
                 *
                 * For 44.1khz and other similar rates, this will be also correct in practice as we expect limited
                 * number of packets in the input buffer at this time(one or two packets typically), e.g:
                 *  1  packet in the buffer, 44 or 45 frames in the buffer, division will result 1 in both cases
                 *  2  packets in the buffer, 44x2 or 44+45 frames in the buffer, division will result 2 in both cases
                 *  15 packets in the buffer, (44x14+45) or (44x13+45x2) frames in the buffer, division will result 15 in both cases
                 * number of packets should reach 430 for the division to result incorrect value (N/10 + 1 >= 44)!
                 */
                packets_received = subframes_read/(usb_audio->n_channels*usb_audio->frames_in_packet_int);

                /* measure mismatch rate, quite possible that the result will not
                 * be used by rate match manager if USB Rx is played by TTP
                 */
                usb_audio_rate_measure_update(usb_audio, cur_time, packets_received);

                /* some data read from usb input buffer into the output,
                 * now let's all the parties involved know that we have
                 * updated buffers.
                 */
                audio_data_service_update_buffers(usb_audio->service_priv);
                audio_data_service_kick_consumer(usb_audio->service_priv);
                if(subframes_read)
                {
                    propagate_kick(ep, STREAM_KICK_FORWARDS);
                }
            }
        }
        else /* ep->direction == SOURCE */
        {
            /* start time of this kick*/
            TIME cur_time = ep->deferred.kick_is_deferred? ep->deferred.interrupt_handled_time:hal_get_time();
            /* total number of packets copied this time, including silence packets */
            unsigned packets_copied = 0;

            /* input available in sub frames */
            unsigned input_data_available = cbuffer_calc_amount_data_in_words(usb_audio->source_buf);
            unsigned space_available_in_frames = cbuffer_calc_amount_space_in_addrs(usb_audio->sink_buf) / frame_in_octets;

            /* Number of packets currently in the output buf, will be  as we write more packets*/
            unsigned packets_in_buf =  (((cbuffer_get_size_in_addrs(usb_audio->sink_buf) - ADDR_PER_WORD) / frame_in_octets) -
                                        space_available_in_frames) / usb_audio-> frames_in_packet_int;

            /* TTP error for first packet */
            TIME_INTERVAL first_packet_error = 0;
            int error_valid = usb_audio_tx_get_packet_ttp_error(usb_audio, cur_time, &first_packet_error);
            TIME_INTERVAL error = first_packet_error;

            /* consumption from input also limited by amount of
             * metadata available if input has metadata */
            if (buff_has_metadata(usb_audio->source_buf))
            {
                unsigned input_meta_available =  (buff_metadata_available_octets(usb_audio->source_buf)/OCTETS_PER_SAMPLE);
                input_data_available = MIN(input_data_available, input_meta_available);
            }

            /* convert samples available to frames */
            input_data_available = input_data_available / usb_audio->n_channels;

            /* see if we need to discard input */
            if(error_valid)
            {
                /* if the input is too late */
                if(error < -usb_audio->error_threshold &&
                   input_data_available > 0)
                {
                    /* work out the amount of frames to trash
                     * (we need to trash in full frames)
                     */
                    int err_diff = -error;
                    unsigned frames_to_trash = convert_time_to_samples((unsigned)err_diff, usb_audio->sample_rate);
                    frames_to_trash = MIN(input_data_available, frames_to_trash);
                    if(frames_to_trash > 0)
                    {
                        /* We have something to trash */
                        cbuffer_advance_read_ptr(usb_audio->source_buf, frames_to_trash*usb_audio->n_channels);
                        usb_audio_tx_transfer_metadata(usb_audio, frames_to_trash, 0);
                        L3_DBG_MSG3("EP_USB_AUDIO - discard frames=%d, time =%d , error=%d", frames_to_trash, cur_time, error);

                        /* update the error, and the amount of data left to packetise */
                        error = time_sub(usb_audio->last_read_timestamp, cur_time);
                        input_data_available -= frames_to_trash;
                        usb_audio->error_threshold = USB_AUDIO_TX_MAX_LATENCY_LOW_US;

                        /* host consumption isn't reliable, reset time period adjust */
                        usb_audio_tx_reset_timer_period_adjust(usb_audio);
                    }
                }
            }

            /* Send packets until usb_audio->max_packets_in_tx_output_buffer packets
             * are in the buffer. If we don't have enough input to packetise we will
             * send silence packet. So at the end of this service there will be exactly
             * usb_audio->max_packets_in_tx_output_buffer in the output buffer. The size
             * of buffer is checked at the start time to make sure it can contain at least
             * this number of packets.
             */
            while(packets_in_buf < usb_audio->max_packets_in_tx_output_buffer)
            {
                /* packetise and send if we have on-time input */
                if (input_data_available >= usb_audio->packet_len_in_frames
                    && (!error_valid || pl_abs_i32(error) < usb_audio->error_threshold))

                {
                    /* write usb tx audio data into output buffer */
                    unsigned subframes_written =
                        stream_usb_audio_write_audio_subframes(usb_audio->sink_buf,
                                                               usb_audio->source_buf,
                                                               usb_audio->subframe_size,
                                                               usb_audio->packet_len_in_frames*frame_in_octets);

                    /* There is no reason that not all the data written */
                    PL_ASSERT(subframes_written == usb_audio->packet_len_in_frames*usb_audio->n_channels);

                    /* transfer metadata, each packet will have one metadata tag */
                    usb_audio_tx_transfer_metadata(usb_audio, usb_audio->packet_len_in_frames, usb_audio->packet_len_in_frames);

                    STREAM_USB_AUDIO_DBG_MSG2("EP_USB_AUDIO, Tx, raw data written,  time = %08x, subframes=%d",
                                              hal_get_time(), subframes_written);

                    /* one packet consumed */
                    input_data_available -= usb_audio->packet_len_in_frames;

                    /* real audio is being sent, go to higher threshold */
                    usb_audio->error_threshold = USB_AUDIO_TX_MAX_LATENCY_HIGH_US;
                }
                else
                {
                    /* either no data or early data, write a silence packet */
                    unsigned packet_in_octets = usb_audio->packet_len_in_frames*frame_in_octets;
                    unsigned octets_written = stream_usb_audio_write_silence_packet(usb_audio->sink_buf,
                                                                                    packet_in_octets);
                    /* No reason why we couldn't copy a packet */
                    PL_ASSERT(packet_in_octets == octets_written);

                    /* transfer metadata, each packet will have one metadata tag */
                    usb_audio_tx_transfer_metadata(usb_audio, 0, usb_audio->packet_len_in_frames);

                    /* only go back to real audio if error is sufficiently low */
                    usb_audio->error_threshold = USB_AUDIO_TX_MAX_LATENCY_LOW_US;

                    L2_DBG_MSG3("EP_USB_AUDIO: one silence packet sent: time=%d, len=%d octets, input_data=%d",
                                cur_time, octets_written, input_data_available);
                }

                /* update packet_len_in_frames */
                usb_audio_tx_update_packet_length(usb_audio);
                packets_copied++;
                packets_in_buf++;

                if(error_valid)
                {   /* update error based on last read time stamp, Normally it
                     * shouldn't change the error noticeably, but in case the
                     * input timestamp has had a big jump we update the error
                     * after writing each packet.
                     */
                    error = time_sub(usb_audio->last_read_timestamp, cur_time)
                        - (int)((packets_copied*usb_audio->kick_period)/USB_AUDIO_TX_PACKETS_PER_KICK);
                }
            }

            if(packets_copied != 0)
            {
                /* one or more packet was written, let the other side knows about this */
                audio_data_service_update_buffers(usb_audio->service_priv);
                audio_data_service_kick_consumer(usb_audio->service_priv);
            }
            else if (input_data_available  >= usb_audio->packet_len_in_frames)
            {
                /* There is data available to write, but sink buffer is full,
                 * one extra kick only to remind the consumer.
                 */
                audio_data_service_kick_consumer(usb_audio->service_priv);
            }

            /* we could copy up to usb_audio->max_packets_in_tx_output_buffer in each kick,
             * kick the input if less than that left in the input buffer
             */
            if(input_data_available <= (usb_audio->max_packets_in_tx_output_buffer*usb_audio->frames_in_packet_int))
            {
                /* Also kick the connected operator */
                propagate_kick(ep, STREAM_KICK_BACKWARDS);
            }

            /* adjust the timer period, so in average, in timed playback mode this
             * is essential, in non-playback mode we also synchronise so the mismatch
             * rate can be estimated more accurately with less variation.
             * The adjustment process makes sure that in average
             * we send exactly USB_AUDIO_TX_PACKETS_PER_KICK packets to host */
            usb_audio_tx_update_timer_period_adjust(usb_audio, packets_copied);
            if(usb_audio->timer_period_adjust_normal_mode)
            {

                if(usb_audio->timed_playback)
                {

                    /* run error control only if period adjustment is running in normal mode*/
                    usb_audio_tx_control_ttp_error(usb_audio, first_packet_error, input_data_available, packets_copied);
                }
                else
                {
                    /* Not in playback mode, so estimate
                     * the mismatch rate for rate match manager
                     */
                    usb_audio_rate_measure_update(usb_audio, cur_time, USB_AUDIO_TX_PACKETS_PER_KICK);
                }
            }
        }
    }
    else /* ep->connected_to */
    {
        STREAM_USB_AUDIO_DBG_MSG3("EP_USB_AUDIO, KICK NOT PROCESSED, time=0x%08x dir=%d, ep=0x%x",
                                  hal_get_time(), ep->direction, (unsigned)ep);
        if(ep->direction == SOURCE)
        {
            /* discard anything in the input buffer */
            stream_usb_audio_discard_octets_from_source_buffer(usb_audio->source_buf,
                                                               cbuffer_calc_amount_data_in_addrs(usb_audio->source_buf));

            /* buffer has discarded, so update buffer */
            audio_data_service_update_buffers(usb_audio->service_priv);
            audio_data_service_kick_consumer(usb_audio->service_priv);
        }
        else
        {
            /* USB Tx isn't connected yet, No action
             * TODO: handle this case */
        }
    }
}

/**
 * audio_data_service_usb_audio_kick
 */
static void audio_data_service_usb_audio_kick(void* priv, ENDPOINT_KICK_DIRECTION kick_dir)
{
    patch_fn_shared(stream_usb_audio_hydra);
    ENDPOINT* ep = (ENDPOINT*)priv;

    if (ep->deferred.kick_is_deferred)
    {
        ep->deferred.kick_dir = kick_dir;
        raise_bg_int(ep->deferred.bg_task);
    }
    else
    {
        usb_audio_kick(ep, kick_dir);
    }
}

/**
 * \brief obtain buffer details of the usb_audio ep
 * \param ep pointer to the endpoint for which buffer details are requested
 * \param details pointer to the buffer details structure
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool usb_audio_buffer_details(ENDPOINT *ep, BUFFER_DETAILS *details)
{
    unsigned buff_size;
    details->supplies_buffer = FALSE;
    details->runs_in_place = FALSE;
    details->can_override = details->wants_override = FALSE;

    if(SOURCE == ep->direction)
    {
        /* size required depends on sample rate and number of channels */
        buff_size = mibgetrequ16(USBSOURCEBUFFERSIZE);
    }
    else
    {
        /* size required depends on sample rate and number of channels */
        buff_size = mibgetrequ16(USBSINKINPUTBUFFERSIZE);
    }

    if (buff_size == 0)
    {
        buff_size = USB_TYPICAL_AUDIO_BUFFER_SIZE;
    }

    details->b.buff_params.size = buff_size;

    details->b.buff_params.flags = BUF_DESC_SW_BUFFER;

    if (SINK == ep->direction)
    {
        /* TX endpoint supports metadata */
        details->supports_metadata = TRUE;
        details->metadata_buffer = NULL;
    }

    return TRUE;
}

/**
 * \brief configure a usb_audio ep with a key and value pair
 *
 * \param *ep pointer to the usb_audio ep being configured
 * \param key key to configure
 * \param value value to be configured with
 *
 * \return TRUE if the call succeeded, FALSE otherwise
 *
 */
static bool usb_audio_configure(ENDPOINT *ep, unsigned int key, uint32 value)
{
    switch (key)
    {
        case EP_DATA_FORMAT:
            if (value == AUDIO_DATA_FORMAT_FIXP ||
                value == USB_AUDIO_DATA_FORMAT)
            {
                ep->state.usb_audio.data_format = value;
                return TRUE;
            }
            return FALSE;

        case ACCMD_CONFIG_KEY_STREAM_RM_ENABLE_DEFERRED_KICK:
        {
            ep->deferred.config_deferred_kick = (value != 0);
            return TRUE;
        }

        case EP_RATEMATCH_ADJUSTMENT:
            if(ep->direction == SINK)
            {
                /* if we are in timed_playback mode then the value is calculated
                 * here, so don't write over it.
                 */
                if(!ep->state.usb_audio.timed_playback)
                {
                    /* update rate adjust value */
                    ep->state.usb_audio.target_rate_adjust_val = (unsigned) value;
                }
                return TRUE;
            }
            else
            {
                /* Only for TX we can do rate adjustment */
                return FALSE;
            }

        case EP_RATEMATCH_ENACTING:
            if(ep->direction == SINK)
            {
                /* This can be set only for TX, and when it isn't running,
                 * connected operator that performs the actual rate adjustment
                 * needs to know this before starting
                 */
                ep->state.usb_audio.ep_ratematch_enacting = (bool) value;

                /* turn off timed playback, it will be activated if endpoint is enacting
                 * and sees TTP tags.
                 */
                ep->state.usb_audio.timed_playback = FALSE;

                /* The rate adjust op could be not yet set at this time */
                if(NULL !=  ep->state.usb_audio.rate_adjust_op)
                {
                    cbops_rateadjust_passthrough_mode(ep->state.usb_audio.rate_adjust_op,
                                                      !ep->state.usb_audio.ep_ratematch_enacting);
                }
                L2_DBG_MSG1("EP_USB_AUDIO SINK: enacting=%d", ep->state.usb_audio.ep_ratematch_enacting);
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        default:
            return FALSE;
    }
}

/**
 * \brief retrieve audio configuration
 *
 * \param ep pointer to the usb_audio ep being configured
 * \param key key for which configuration needs to be retrieved
 * \param result pointer to the value that will be populated
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool usb_audio_get_config(ENDPOINT *ep, unsigned int key, ENDPOINT_GET_CONFIG_RESULT *result)
{
    switch(key)
    {
        case EP_DATA_FORMAT:
            if(!ep->state.usb_audio.data_format)
                result->u.value = (uint32)(AUDIO_DATA_FORMAT_FIXP);
            else
                result->u.value = (uint32)ep->state.usb_audio.data_format;
            return TRUE;

        case EP_RATEMATCH_ABILITY:
            if(SOURCE == ep->direction)
            {
                result->u.value = (uint32)RATEMATCHING_SUPPORT_NONE;
            }
            else
            {   /* USB TX can do rate matching via its
                 * connected USB TX operator
                 */
                result->u.value = (uint32)RATEMATCHING_SUPPORT_SW;
            }
            return TRUE;

        case EP_RATEMATCH_RATE:
            if(ep->state.usb_audio.timed_playback)
            {
                /* Tx only:
                 * in playback mode, return perfect rate so
                 * only timed playback rate matching happens */
                result->u.value = RM_PERFECT_RATE;
            }
            else
            {
                /* return estimated mismatch rate */
                result->u.value = ep->state.usb_audio.norm_rate_ratio;
            }

            return TRUE;
        default:
            return FALSE;
    }
}

/**
 * \brief obtain timing information of the usb_audio ep
 * \param ep pointer to the endpoint for which timing information is requested
 * \param time_info pointer to the timing information structure
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static void usb_audio_get_timing(ENDPOINT *ep, ENDPOINT_TIMING_INFORMATION *time_info)
{
    time_info->block_size = 1;
    time_info->has_deadline = FALSE;
    time_info->wants_kicks = FALSE;
    time_info->locally_clocked = FALSE;

    /* USB TX has kick period */
    time_info->period = STREAM_KICK_PERIOD_FROM_USECS(ep->state.usb_audio.kick_period);

    return;
}

/**
 * usb_audio_tx_enable_time_event
 * \brief enables or disables timer event for this endpoint (tx-only)
 *
 * \param ep pointer to the endpoint
 *
 * \param ko kick object
 *
 * \param enable if TRUE it enables the time events for usb tx endpoint and does
 *        the first scheduling. If FALSE it cancels time events for this ep.
 *
 */
static void usb_audio_tx_enable_time_event(ENDPOINT *ep, KICK_OBJECT *ko, bool enable)
{
    patch_fn_shared(stream_usb_audio_hydra);

    if(enable)
    {
        /* timer should be invalid at this point */
        PL_ASSERT(TIMER_ID_INVALID == ep->state.usb_audio.kick_id);

        /* Schedule the kick timer for this chain. */
        ep->state.usb_audio.kick_id =
            timer_schedule_event_in(ep->state.usb_audio.kick_period, kick_obj_kick, (void*)ko);
    }
    else
    {
        /* timer should be valid at this point */
        PL_ASSERT(TIMER_ID_INVALID != ep->state.usb_audio.kick_id);

        /* This was the last transform in the sync list. Cancel any timer */
        LOCK_INTERRUPTS;
        timer_cancel_event(ep->state.usb_audio.kick_id);
        ep->state.usb_audio.kick_id = TIMER_ID_INVALID;
        UNLOCK_INTERRUPTS;
    }
}

/**
 * \brief start the usb_audio ep
 * \param ep pointer to the endpoint being started
 *
 * \param ko pointer to the kick object that called start
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool usb_audio_start(ENDPOINT *ep, KICK_OBJECT *ko)
{
    L2_DBG_MSG4("EP_USB_AUDIO, START RECEIVED, time=0x%08x dir=%d, ep=0x%x, running=%d",
                hal_get_time(), ep->direction, (unsigned)ep, ep->state.usb_audio.running);

    /* at this point the ep must have already been configured */
    if(!ep->state.usb_audio.subframe_size)
    {
        return FALSE;
    }

    if(!ep->state.usb_audio.running)
    {
        ep->state.usb_audio.running = TRUE;
        stream_set_deferred_kick(ep, ep->deferred.config_deferred_kick);
        if(SOURCE == ep->direction)
        {
            /* one-off internal kick, so we read anything already received */
            usb_audio_kick(ep, STREAM_KICK_INTERNAL);

            audio_data_service_start_kick(ep->state.usb_audio.service_priv);
        }
        else
        {
            /* start time event for USB TX */
            usb_audio_tx_enable_time_event(ep, ko, TRUE);
        }
    }
    return TRUE;
}

/**
 * \brief stop the usb_audio ep
 * \param ep pointer to the endpoint being stopped
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool usb_audio_stop(ENDPOINT *ep)
{
    L2_DBG_MSG4("EP_USB_AUDIO, STOP RECEIVED, time=0x%08x dir=%d, ep=0x%x, running=%d",
                hal_get_time(), ep->direction, (unsigned)ep, ep->state.usb_audio.running);

    if(ep->state.usb_audio.running)
    {
        if(SOURCE == ep->direction)
        {
            /* Stop receiving kick from audio data service */
            audio_data_service_stop_kick(ep->state.usb_audio.service_priv);
        }
        else
        {
            /* stop timer setup for kicking TX endpoint */
            usb_audio_tx_enable_time_event(ep, kick_obj_from_sched_endpoint(ep), FALSE);

            /* exit timed playback mode */
            ep->state.usb_audio.timed_playback = FALSE;
        }

        stream_set_deferred_kick(ep, FALSE);
        ep->state.usb_audio.running = FALSE;
        return TRUE;
    }
    else
    {
        return FALSE;
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
static void usb_audio_sched_kick (ENDPOINT *endpoint, KICK_OBJECT *ko)
{
    if(endpoint->direction == SINK)
    {
        /* expect to have a valid timer ID */
        PL_ASSERT(TIMER_ID_INVALID != endpoint->state.usb_audio.kick_id);

        /* This is a periodic timer so ask timers when it was scheduled to fire and
         * use that to schedule the next one. */
        TIME next_fire_time = time_add(get_last_fire_time(), endpoint->state.usb_audio.kick_period);
        endpoint->state.usb_audio.kick_id = timer_schedule_event_at(next_fire_time,
                                                                    kick_obj_kick, (void*)ko);
    }
}
