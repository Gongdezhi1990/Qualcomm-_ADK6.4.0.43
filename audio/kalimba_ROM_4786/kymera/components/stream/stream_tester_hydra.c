/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  stream_tester_hydra.c
 * \ingroup stream
 *
 * stream tester type file. <br>
 * This file contains stream functions for tester endpoints. <br>
 *
 */

#ifdef INSTALL_AUDIO_DATA_SERVICE_TESTER
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

/* Data service data buffer size (usable octets) */
#define TESTER_DATA_BUFFER_SIZE_BYTES (4096)
/* Data service metadata buffer size (usable octets) */
#define TESTER_META_BUFFER_SIZE_BYTES (256)

static bool tester_close (ENDPOINT *ep);
static bool tester_connect (ENDPOINT *ep, tCbuffer *cb,  ENDPOINT *ep_to_kick, bool* start_on_connect);
static bool tester_disconnect (ENDPOINT *ep);
static bool tester_buffer_details (ENDPOINT *ep, BUFFER_DETAILS *details);
static inline void tester_kick(ENDPOINT *ep, ENDPOINT_KICK_DIRECTION kick_dir);
static bool tester_start (ENDPOINT *ep, KICK_OBJECT *ko);
static bool tester_stop (ENDPOINT *ep);
static bool tester_configure (ENDPOINT *ep, unsigned int key, uint32 value);
static bool tester_get_config (ENDPOINT *ep, unsigned int key, ENDPOINT_GET_CONFIG_RESULT* result);
static void tester_get_timing (ENDPOINT *ep, ENDPOINT_TIMING_INFORMATION *time_info);

DEFINE_ENDPOINT_FUNCTIONS(tester_functions, tester_close, tester_connect,
                          tester_disconnect, tester_buffer_details,
                          tester_kick, stream_sched_kick_dummy,
                          tester_start, tester_stop,
                          tester_configure, tester_get_config,
                          tester_get_timing, stream_sync_sids_dummy);

static void audio_data_service_tester_kick(void* priv, ENDPOINT_KICK_DIRECTION kick_dir)
{
    tester_kick((ENDPOINT*)(priv), kick_dir);
}

bool stream_query_tester(AUDIO_DATA_SERVICE_EP_PARAMS *params)
{
    params->data_buf_size = MAX(params->data_buf_size, TESTER_DATA_BUFFER_SIZE_BYTES);
#ifdef BAC32
    params->data_buf_flags = BUF_DESC_UNPACKED_32BIT_MASK;
#else
    params->data_buf_flags = 0;
#endif
    params->meta_buf_size = params->meta_buf_size ? MAX(params->meta_buf_size, TESTER_META_BUFFER_SIZE_BYTES) : 0;
    params->kick_fn = audio_data_service_tester_kick;
    params->start_kicks_immediately = FALSE;
    params->want_octet_buffer = FALSE;
    return TRUE;
}

void stream_create_tester(const AUDIO_DATA_SERVICE_EP_CFG *cfg, AUDIO_DATA_SERVICE_EP_CREATE_RESULT* create_res)
{
    ENDPOINT *ep;
    endpoint_tester_state *tester;

    ep = STREAM_NEW_ENDPOINT(tester, cfg->key, cfg->dir, INVALID_CON_ID);
    if (!ep)
    {
        create_res->success = FALSE;
        return;
    }
    tester = &ep->state.tester;
    tester->service_priv = cfg->service_priv;
    tester->priv_hdr_len = cfg->traffic->header_length;
    ep->can_be_closed = FALSE;
    ep->can_be_destroyed = FALSE;
    ep->is_real = TRUE;
    if (cfg->dir == SOURCE)
    {
        tester->source_data_buf = cfg->data_buf;
    }
    else
    {
        tester->sink_data_buf = cfg->data_buf;
    }
    create_res->priv = (void*) ep;
    create_res->ext_ep_id = stream_external_id_from_endpoint(ep);
    create_res->success = TRUE;
}

bool stream_destroy_tester(void *ep)
{
    ((ENDPOINT*)ep)->can_be_closed = TRUE;
    ((ENDPOINT*)ep)->can_be_destroyed = TRUE;
    return stream_close_endpoint((ENDPOINT*)ep);
}

/**
 * \brief closes the tester ep
 * \param ep pointer to the endpoint that is being closed
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool tester_close(ENDPOINT *ep)
{
    /* TODO */
    return TRUE;
}

/**
 * \brief Connect to the endpoint.
 *
 * \param *ep pointer to the endpoint that is being connected
 * \param *cb pointer to the Cbuffer struct for the buffer that is being connected.
 * \param *ep_to_kick pointer to the endpoint which will be kicked after a successful
 *              run. Note: this can be different from the connected to endpoint when
 *              in-place running is enabled.
 * \param *start_on_connect return flag which indicates if the endpoint wants be started
 *              on connect. Note: The endpoint will only be started if the connected
 *              to endpoint wants to be started too.
 *
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool tester_connect(ENDPOINT *ep, tCbuffer *cb,  ENDPOINT *ep_to_kick, bool* start_on_connect)
{
    endpoint_tester_state *tester = &ep->state.tester;
    ep->ep_to_kick = ep_to_kick;
    if (ep->direction == SOURCE)
    {
        tester->sink_data_buf = cb;
    }
    else
    {
        tester->source_data_buf = cb;
    }
    *start_on_connect = FALSE;
    return TRUE;
}

/**
 * \brief disconnect from the tester ep
 * \param ep pointer to the endpoint that is being connected
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool tester_disconnect(ENDPOINT *ep)
{
    endpoint_tester_state *tester = &ep->state.tester;
    if (ep->direction == SOURCE)
    {
        tester->sink_data_buf = NULL;
    }
    else
    {
        tester->source_data_buf = NULL;
    }
    return TRUE;
}

static void tester_internal_kick(void *priv)
{    
    ((ENDPOINT*)(priv))->state.tester.internal_kick_timer_id = TIMER_ID_INVALID;
    tester_kick((ENDPOINT*)(priv), STREAM_KICK_INTERNAL);
}

/**
 * \brief kick the tester ep
 * \param ep pointer to the endpoint that is being kicked
 * \param kick_dir direction of the kick
 * \return TRUE if the call succeeded, FALSE otherwise
 */

static inline void tester_kick(ENDPOINT *ep, ENDPOINT_KICK_DIRECTION kick_dir)
{
    endpoint_tester_state *tester = &ep->state.tester;
    unsigned copied, copy_samples, data_available, space;
    /**
     *  To prevent race we let the core kick function run at one and only one priority level
     *  A kick from an operator is actually postponed slightly so it appears as a kick from
     *  a timer source. Also, we setup a timer only if there's some data to copy.
     */
    if ((kick_dir == STREAM_KICK_BACKWARDS) || (kick_dir == STREAM_KICK_FORWARDS))
    {
        /* Schedule a new event only if there isn't one pending */
        if (cbuffer_calc_amount_data_in_words(tester->source_data_buf) && (tester->internal_kick_timer_id == TIMER_ID_INVALID))
        {
            timer_schedule_event_in_atomic(0, tester_internal_kick, (void *)(ep), &tester->internal_kick_timer_id);
        }
        return;
    }

    if (SOURCE == ep->direction)
    {
        if ( BUFF_METADATA(tester->source_data_buf) )
        {
            audio_data_service_queue_meta(tester->service_priv);
        }
    }
    /* try copying as much as possible (min(data_available, space)) */
    data_available = cbuffer_calc_amount_data_in_words(tester->source_data_buf);
    space = cbuffer_calc_amount_space_in_words(tester->sink_data_buf);
    copy_samples = (data_available < space) ? data_available : space;
    /* Be aware this is called during connect phase (tester_start),
       when not all the setup has been completed.
     */
    if (copy_samples != 0)
    {
        unsigned size_in_words;

        if ((SOURCE == ep->direction && BUFF_METADATA(tester->source_data_buf)) ||
            (SINK == ep->direction && BUFF_METADATA(tester->sink_data_buf)) )
        {
            metadata_strict_transport( tester->source_data_buf,
                      tester->sink_data_buf, copy_samples * OCTETS_PER_SAMPLE);
            /* NB. return value not checked (mtag) */
        }
        size_in_words = cbuffer_get_size_in_words(tester->source_data_buf);
        PL_ASSERT( copy_samples < size_in_words);
        size_in_words = cbuffer_get_size_in_words(tester->sink_data_buf);
        PL_ASSERT( copy_samples < size_in_words);
        copied = cbuffer_copy(tester->sink_data_buf, tester->source_data_buf, copy_samples);
        /* should always have enough data */
        PL_ASSERT( copied == copy_samples);

        if (SINK == ep->direction)
        {
            if (BUFF_METADATA(tester->sink_data_buf))
            {
                audio_data_service_dequeue_meta(tester->service_priv, copy_samples * OCTETS_PER_SAMPLE);
            }
        }
        audio_data_service_update_buffers(tester->service_priv);


        /*  Kick whether or not the deque wrote an entry into
         *  the meta buffer. This can happen because there
         *  is no metadata or there was no space in the meta
         *  buffer. The latter is not expected to happen. If
         *  the latter does happen, the consumer will not consume
         *  extra data that was written because it is expected
         *  (per CS-326894-SP) to use the offset as implied
         *  by the last metadata record. Next time the endpoint
         *  gets kicked, meta entries will be dequeued and written
         *  into the meta buffer.
         */
        audio_data_service_kick_consumer(tester->service_priv);

        ENDPOINT_KICK_DIRECTION dir = (SOURCE == ep->direction ? STREAM_KICK_FORWARDS : STREAM_KICK_BACKWARDS);
        propagate_kick(ep, dir);
    }
}

/**
 * \brief obtain buffer details of the tester ep
 * \param ep pointer to the endpoint for which buffer details are requested
 * \param details pointer to the buffer details structure
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool tester_buffer_details(ENDPOINT *ep, BUFFER_DETAILS *details)
{
#ifdef INSTALL_METADATA
    endpoint_tester_state *tester = &ep->state.tester;

    details->supports_metadata = FALSE;
    /* This endpoint type may produce/consume metadata. */
    if (SOURCE == ep->direction)
    {
        if ( BUFF_METADATA(tester->source_data_buf) )
        {
            details->supports_metadata = TRUE;
        }
    }
    else
    {
        if ( BUFF_METADATA(tester->sink_data_buf) )
        {
            details->supports_metadata = TRUE;
        }
    }
    details->metadata_buffer = NULL;
#endif /* INSTALL_METADATA */

    details->supplies_buffer = FALSE;
    details->runs_in_place = FALSE;
    details->can_override = details->wants_override = FALSE;
    details->b.buff_params.size = TESTER_DATA_BUFFER_SIZE_BYTES / sizeof(int);
    details->b.buff_params.flags = BUF_DESC_SW_BUFFER;

    return TRUE;
}

/*
 * \brief configure a tester ep with a key and value pair
 *
 * \param *ep pointer to the tester ep being configured
 * \param key key to configure
 * \param value value to be configured with
 *
 * \return TRUE if the call succeeded, FALSE otherwise
 *
 */
static bool tester_configure(ENDPOINT *ep, unsigned int key, uint32 value)
{
    /* TODO */
    return TRUE;
}

/*
 * \brief retrieve audio configuration
 *
 * \param ep pointer to the tester ep being configured
 * \param key key for which configuration needs to be retrieved
 * \param value pointer to the value that will be populated
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool tester_get_config(ENDPOINT *ep, unsigned int key, ENDPOINT_GET_CONFIG_RESULT* result)
{
    /* TODO */
    switch(key)
    {
        case EP_DATA_FORMAT:
            result->u.value = (uint32)(AUDIO_DATA_FORMAT_FIXP);
            return TRUE;
        case EP_RATEMATCH_ABILITY:
            result->u.value = (uint32)RATEMATCHING_SUPPORT_NONE;
            return TRUE;
        case EP_RATEMATCH_RATE:
            result->u.value = (uint32)(1 << STREAM_RATEMATCHING_FIX_POINT_SHIFT);
            return TRUE;
        case EP_RATEMATCH_MEASUREMENT:
            result->u.rm_meas.sp_deviation = 0;
            result->u.rm_meas.measurement.valid = FALSE;
            return TRUE;
        default:
            return FALSE;
    }
}

/**
 * \brief obtain timing information of the tester ep
 * \param ep pointer to the endpoint for which timing information is requested
 * \param time_info pointer to the timing information structure
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static void tester_get_timing(ENDPOINT *ep, ENDPOINT_TIMING_INFORMATION *time_info)
{
    time_info->period = 0;
    time_info->block_size = 1;
    time_info->has_deadline = FALSE;
    time_info->locally_clocked = TRUE;
    time_info->wants_kicks = TRUE;
    return;
}

/**
 * \brief start the tester ep
 * \param ep pointer to the endpoint being started
 * \param ko pointer to the kick object that called start
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool tester_start(ENDPOINT *ep, KICK_OBJECT *ko)
{
    endpoint_tester_state *tester = &ep->state.tester;
    tester->internal_kick_timer_id = TIMER_ID_INVALID;
    /* Get data moving so upstream doesn't give up */
    tester_kick(ep, STREAM_KICK_INTERNAL);
    audio_data_service_start_kick(tester->service_priv);
    return TRUE;
}

/**
 * \brief stop the tester ep
 * \param ep pointer to the endpoint being stopped
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool tester_stop(ENDPOINT *ep)
{
    endpoint_tester_state *tester = &ep->state.tester;
    audio_data_service_stop_kick(tester->service_priv);
    timer_cancel_event(tester->internal_kick_timer_id);
    tester->internal_kick_timer_id = TIMER_ID_INVALID;
    return TRUE;
}
#endif /* INSTALL_AUDIO_DATA_SERVICE_TESTER */
