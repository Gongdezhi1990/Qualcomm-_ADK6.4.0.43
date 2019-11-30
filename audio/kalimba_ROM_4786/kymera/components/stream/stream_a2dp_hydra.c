/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \ingroup endpoints
 * \file  stream_a2dp_hydra.c
 *
 * stream a2dp type file. <br>
 * This file contains stream functions for hydra a2dp endpoints. <br>
 *
 */

#include "audio_data_service.h"
#include "audio_data_service_kicks.h"
#include "audio_log/audio_log.h"
#include "stream_private.h"
#include "stream_endpoint_a2dp.h"
#include "stream_for_audio_data_service.h"

/** The size of the mmu buffer that holds incoming a2dp data (usable octets) */
#define HYDRA_A2DP_MMU_DATA_BUFFER_SIZE_BYTES (2048)
/** The size of the mmu buffer that holds incoming a2dp metadata (usable octets) */
#define A2DP_META_BUFFER_SIZE_BYTES (64)

/** The number of usec without data arriving over the air before we consider
 *  the input stream to be stalled.
 */
static volatile
#if !defined(__GNUC__)
_Pragma("datasection DM_SHARED")
#endif
unsigned a2dp_stall_threshold = 80000;

/** The number of usec to wait before attempting to retry pushing data forwards */
#define A2DP_SELF_KICK_THRESHOLD 5000

/** Fixed point precision to use in the buffer level average calculations. This
 *  value defines the amount of fixed point resolution.
 */

static void a2dp_get_timing(ENDPOINT *, ENDPOINT_TIMING_INFORMATION *);
static inline void a2dp_kick(ENDPOINT *endpoint, ENDPOINT_KICK_DIRECTION kick_dir);
static bool a2dp_start(ENDPOINT *, KICK_OBJECT *);
static bool a2dp_stop(ENDPOINT *);

static void a2dp_stall_handler(void *ep)
{
    patch_fn_shared(stream_a2dp_kick);
    endpoint_a2dp_state *a2dp = &(((ENDPOINT *)ep)->state.a2dp);
    a2dp->stalled = TRUE;
}

DEFINE_ENDPOINT_FUNCTIONS(a2dp_functions, stream_close_dummy, a2dp_connect,
                          a2dp_disconnect, a2dp_buffer_details,
                          a2dp_kick, stream_sched_kick_dummy, a2dp_start,
                          a2dp_stop, a2dp_configure, a2dp_get_config,
                          a2dp_get_timing, stream_sync_sids_dummy);

static void audio_data_service_a2dp_kick(void* priv, ENDPOINT_KICK_DIRECTION kick_dir)
{
    a2dp_kick((ENDPOINT*)(priv), kick_dir);
}

static void a2dp_self_kick(void *priv)
{
    ((ENDPOINT*)(priv))->state.a2dp.self_kick_timer_id = TIMER_ID_INVALID;
    a2dp_kick((ENDPOINT*)(priv), STREAM_KICK_INTERNAL);
}


bool stream_query_a2dp(AUDIO_DATA_SERVICE_EP_PARAMS *params)
{
    patch_fn_shared(stream_a2dp);
    params->data_buf_size = MAX(params->data_buf_size, HYDRA_A2DP_MMU_DATA_BUFFER_SIZE_BYTES);
#ifdef BAC32
    params->data_buf_flags = BUF_DESC_UNPACKED_16BIT_MASK;
#else
    params->data_buf_flags = 0;
#endif
    params->meta_buf_size = params->meta_buf_size ? MAX(params->meta_buf_size, A2DP_META_BUFFER_SIZE_BYTES) : 0;
    params->kick_fn = audio_data_service_a2dp_kick;
    params->start_kicks_immediately = FALSE;
    params->want_octet_buffer = TRUE;
    return TRUE;
}

void stream_create_a2dp(const AUDIO_DATA_SERVICE_EP_CFG *cfg, AUDIO_DATA_SERVICE_EP_CREATE_RESULT* create_res)
{
    patch_fn_shared(stream_a2dp);
    ENDPOINT *ep;
    endpoint_a2dp_state *a2dp;
    ep = STREAM_NEW_ENDPOINT(a2dp, cfg->key, cfg->dir, INVALID_CON_ID);
    if (NULL == ep)
    {
        create_res->success = FALSE;
        return;
    }
    a2dp = &ep->state.a2dp;
    a2dp->service_priv = cfg->service_priv;

    ep->can_be_closed = FALSE;
    ep->can_be_destroyed = FALSE;
    ep->is_real = TRUE;
    /* L2CAP/A2DP endpoint is assumed to always carry encoded data, which
     * it is conventional to byte-swap. */
    audio_data_service_set_data_buffer_byte_swap(cfg->service_priv, TRUE);
    if (cfg->dir == SOURCE)
    {
        a2dp->source_buf =  cfg->data_buf;
    }
    else
    {
        a2dp->sink_buf = cfg->data_buf;
    }
    create_res->priv = (void*)ep;
    create_res->ext_ep_id = stream_external_id_from_endpoint(ep);
    create_res->success = TRUE;
}

bool stream_destroy_a2dp(void *ep)
{
    patch_fn_shared(stream_a2dp);
    ((ENDPOINT*)ep)->can_be_closed = TRUE;
    ((ENDPOINT*)ep)->can_be_destroyed = TRUE;
    return stream_close_endpoint((ENDPOINT*)ep);
}

static void a2dp_get_timing(ENDPOINT *ep, ENDPOINT_TIMING_INFORMATION *time_info)
{
    time_info->period = 0;
    /** TODO: check if kicks are required for an a2dp sink  */
    time_info->wants_kicks = FALSE;
    a2dp_get_timing_common(ep, time_info);
}

/**
 * Copies the tags to the destination buffer. Sets the time of arrival for them if the
 * endpoint is source.
 */
static metadata_tag *metadata_transport_and_set_toa(tCbuffer *src, tCbuffer *dst, unsigned trans_octets, ENDPOINT_DIRECTION dir)
{
    metadata_tag *ret_mtag, *list_tag;
    unsigned b4idx, afteridx;

    patch_fn_shared(stream_timestamped);

    if (src != NULL)
    {
        /* transport metadata, first (attempt to) consume tag associated with src */
        ret_mtag = buff_metadata_remove(src, trans_octets, &b4idx, &afteridx);
    }
    else
    {
        b4idx = trans_octets;
        afteridx = 0;
        ret_mtag = NULL;
    }

    list_tag = ret_mtag;

    if (dst != NULL)
    {
        while (list_tag != NULL)
        {
            if (dir == SOURCE)
            {
                /* Set the time of arrival. */
                METADATA_TIME_OF_ARRIVAL_SET(list_tag, hal_get_time());
            }
            list_tag = list_tag->next;
        }
        /* Even if the src is a NULL buffer we append to dst. It makes no sense
         * for the current configuration. However if another connection is made
         * later to the src which does support metadata the dst metadata write
         * pointer needs to be at the right offset. */
        buff_metadata_append(dst, ret_mtag, b4idx, afteridx);
    }
    else
    {
        buff_metadata_tag_list_delete(ret_mtag);
    }
    return ret_mtag;
}


/**
 * \brief kick the a2dp ep
 * \param ep pointer to the endpoint that is being kicked
 * \param kick_dir direction of the kick
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static inline void a2dp_kick(ENDPOINT *ep, ENDPOINT_KICK_DIRECTION kick_dir)
{
    int copied = 0;
    endpoint_a2dp_state *a2dp = &ep->state.a2dp;
    patch_fn_shared(stream_a2dp_kick);

#ifdef INSTALL_METADATA
    if (SOURCE == ep->direction)
    {
        if (BUFF_METADATA(a2dp->source_buf))
        {
            if (buff_metadata_tag_threshold_exceeded())
            {
                /* Running out of space for tags, so come back and try again later */
                if (a2dp->self_kick_timer_id == TIMER_ID_INVALID)
                {
                    a2dp->self_kick_timer_id = timer_schedule_event_in(
                                                 A2DP_SELF_KICK_THRESHOLD,
                                                 a2dp_self_kick,
                                                 (void *)(ep));
                    L2_DBG_MSG("a2dp: setting self kick timer (tag threshold exceeded)");
                }
                return;
            }

            audio_data_service_queue_meta(a2dp->service_priv);
        }
    }
    else
    {
        /* TODO encoders not supported yet */
    }
#endif /* INSTALL_METADATA */

    if (ep->connected_to)
    {
        /* Default is to copy as much as possible. */
        unsigned max_copy = cbuffer_get_size_ex(a2dp->sink_buf);
#ifdef INSTALL_METADATA
        if (BUFF_METADATA(a2dp->sink_buf))
        {
            /* Limit to space available according to sink buffer metadata */
            max_copy = buff_metadata_available_space(a2dp->sink_buf);
        }
#endif
        copied = cbuffer_copy_16bit_be_zero_shift_ex(a2dp->sink_buf, a2dp->source_buf, max_copy);

        if (SOURCE == ep->direction)
        {
            unsigned octets = cbuffer_calc_amount_data_ex(a2dp->source_buf);
            /* If the source buffer can't be emptied, setup a timer based kick
             * to try emptying it so RTP can see data at the earliest */
            if (octets)
            {
                /* Schedule a new event only if there isn't one pending */
                if (a2dp->self_kick_timer_id == TIMER_ID_INVALID)
                {
                    a2dp->self_kick_timer_id = timer_schedule_event_in(
                                                 A2DP_SELF_KICK_THRESHOLD,
                                                 a2dp_self_kick,
                                                 (void *)(ep));
                    L2_DBG_MSG1("a2dp: setting self kick timer (pending octets=%d)", octets);
                }
            }
            else
            {
                /**
                 * self_kick_timer_id could be TIMER_INVALID but that's
                 * taken care by timer_cancel_event.
                 */
                timer_cancel_event(a2dp->self_kick_timer_id);
                a2dp->self_kick_timer_id = TIMER_ID_INVALID;
            }
        }
    }

#ifdef INSTALL_METADATA
    if (copied != 0)
    {
        metadata_transport_and_set_toa(a2dp->source_buf,
                a2dp->sink_buf, copied, ep->direction);
    }
#endif /* INSTALL_METADATA */

    audio_data_service_update_buffers(a2dp->service_priv);

    /*
     * New data may be available in the a2dp source buffer though we
     * may not have copied data into the sink buffer because the sink
     * buffer is full. This may delay cancelling and rescheduling the
     * stall timer. That is okay as the decoder is anyway expected
     * to consume data relatively soon afterwards that will cause the
     * stall timer to be cancelled and rescheduled.
     */
    if (copied != 0)
    {
        timer_cancel_event(a2dp->stall_timer_id);
        a2dp->stall_timer_id = timer_schedule_bg_event_in(
                                                a2dp_stall_threshold, a2dp_stall_handler,
                                               (void*)ep);
        a2dp->stalled = FALSE;
        audio_data_service_kick_consumer(a2dp->service_priv);
    }

    if (SOURCE == ep->direction)
    {
        a2dp_source_kick_common(ep);
    }
}

/**
 * \brief start the a2dp ep
 * \param ep pointer to the endpoint being started
 * \param ko pointer to the kick object that called start
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool a2dp_start(ENDPOINT *ep, KICK_OBJECT *ko)
{
    endpoint_a2dp_state *a2dp = &ep->state.a2dp;
    if (a2dp_start_common(ep))
    {
        /* Schedule the stall detection timer
         * Use casual events (through bg int)
         */
        a2dp->stall_timer_id = timer_schedule_bg_event_in(a2dp_stall_threshold, a2dp_stall_handler, (void*)ep);
        a2dp->self_kick_timer_id = TIMER_ID_INVALID;
        /* Get data moving so upstream doesn't give up */
        a2dp_kick(ep, STREAM_KICK_INTERNAL);
        a2dp->running = TRUE;
        /* The endpoint state must be configured before this call to start
         * consumer kicks in case the interrupt calls the kick handler before
         * this call is complete. */
        audio_data_service_start_kick(a2dp->service_priv);
    }
    return TRUE;
}

/**
 * \brief stop the a2dp ep
 * \param ep pointer to the endpoint being stopped
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool a2dp_stop(ENDPOINT *ep)
{
    endpoint_a2dp_state *a2dp = &ep->state.a2dp;
    if (a2dp->running)
    {
        audio_data_service_stop_kick(a2dp->service_priv);
        /* The kick source is stopped before cancelling
         * the timer so we need not lock interrupts when the
         * timer is being cancelled. It is also assumed that
         * host command processing is run at lower priority
         * than a bus interrupt */
        timer_cancel_event(a2dp->stall_timer_id);
        timer_cancel_event(a2dp->self_kick_timer_id);
        a2dp->stall_timer_id = TIMER_ID_INVALID;
        a2dp->self_kick_timer_id = TIMER_ID_INVALID;
        a2dp->running = FALSE;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
