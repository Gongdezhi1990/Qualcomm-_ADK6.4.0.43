/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  stream_file_hydra.c
 * \ingroup stream
 */

#ifdef INSTALL_FILE

#include "adaptor/adaptor.h"
#include "audio_data_service.h"
#include "audio_data_service_meta.h"
#include "buffer.h"
#include "hydra_log/hydra_log.h"
#include "limits.h"
#include "pl_assert.h"
#include "pmalloc/pl_malloc.h"
#include "stream.h"
#include "stream_audio_data_format.h"
#include "stream_endpoint.h"
#include "stream_for_audio_data_service.h"
#include "stream_kick_obj.h"
#include "stream_private.h"
#include "types.h"
#include "util.h"

/* Data service data buffer size (usable octets) */
#define FILE_DATA_BUFFER_SIZE_BYTES (1024)
/* Data service metadata buffer size (usable octets) */
#define FILE_META_BUFFER_SIZE_BYTES (256)
#define FILE_INPUT_OCTETS_PER_WORD (2)

static bool file_close (ENDPOINT *ep);
static bool file_connect (ENDPOINT *ep, tCbuffer *cb,  ENDPOINT *ep_to_kick, bool* start_on_connect);
static bool file_disconnect (ENDPOINT *ep);
static bool file_buffer_details (ENDPOINT *ep, BUFFER_DETAILS *details);
static inline void file_kick(ENDPOINT *ep, ENDPOINT_KICK_DIRECTION kick_dir);
static bool file_start (ENDPOINT *ep, KICK_OBJECT *ko);
static bool file_stop (ENDPOINT *ep);
static bool file_configure (ENDPOINT *ep, unsigned int key, uint32 value);
static bool file_get_config (ENDPOINT *ep, unsigned int key, ENDPOINT_GET_CONFIG_RESULT* result);
static void file_get_timing (ENDPOINT *ep, ENDPOINT_TIMING_INFORMATION *time_info);

DEFINE_ENDPOINT_FUNCTIONS(file_functions, file_close, file_connect,
                          file_disconnect, file_buffer_details,
                          file_kick, stream_sched_kick_dummy,
                          file_start, file_stop,
                          file_configure, file_get_config,
                          file_get_timing, stream_sync_sids_dummy);

static void audio_data_service_file_kick(void* priv, ENDPOINT_KICK_DIRECTION kick_dir)
{
    file_kick((ENDPOINT*)(priv), kick_dir);
}

bool stream_query_file(AUDIO_DATA_SERVICE_EP_PARAMS *params)
{
    patch_fn_shared(stream_file);
    params->data_buf_size = MAX(params->data_buf_size, FILE_DATA_BUFFER_SIZE_BYTES);
#ifdef BAC32
    params->data_buf_flags = BUF_DESC_UNPACKED_16BIT_MASK;
#else
    params->data_buf_flags = 0;
#endif
    params->meta_buf_size = params->meta_buf_size ? MAX(params->meta_buf_size, FILE_META_BUFFER_SIZE_BYTES) : 0;
    params->kick_fn = audio_data_service_file_kick;
    params->start_kicks_immediately = FALSE;
    params->want_octet_buffer = TRUE;
    return TRUE;
}

void stream_create_file(const AUDIO_DATA_SERVICE_EP_CFG *cfg, AUDIO_DATA_SERVICE_EP_CREATE_RESULT* create_res)
{
    ENDPOINT *ep;
    endpoint_file_state *file;
    patch_fn_shared(stream_file);
    ep = STREAM_NEW_ENDPOINT(file, cfg->key, cfg->dir, INVALID_CON_ID);
    if (!ep)
    {
        create_res->success = FALSE;
        return;
    }
    file = &ep->state.file;
    file->service_priv = cfg->service_priv;
    ep->can_be_closed = FALSE;
    ep->can_be_destroyed = FALSE;
    ep->is_real = TRUE;
    PL_ASSERT(cfg->dir == SOURCE);
    file->source_buf = cfg->data_buf;
    /** configure for SBC file streaming, by default */
    file->shift = 0;
    file->byte_swap = TRUE;
    file->usable_octets = 2;
    create_res->priv = (void*) ep;
    create_res->ext_ep_id = stream_external_id_from_endpoint(ep);
    create_res->success = TRUE;
}

bool stream_destroy_file(void *ep)
{
    patch_fn_shared(stream_file);
    ((ENDPOINT*)ep)->can_be_closed = TRUE;
    ((ENDPOINT*)ep)->can_be_destroyed = TRUE;
    return stream_close_endpoint((ENDPOINT*)ep);
}

/**
 * \brief closes the file ep
 * \param ep pointer to the endpoint that is being closed
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool file_close(ENDPOINT *ep)
{
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
static bool file_connect(ENDPOINT *ep, tCbuffer *cb,  ENDPOINT *ep_to_kick, bool* start_on_connect)
{
    endpoint_file_state *file = &ep->state.file;
    ep->ep_to_kick = ep_to_kick;
#ifdef INSTALL_METADATA
    if (buff_has_metadata(cb))
    {
        buff_metadata_set_usable_octets(cb, file->usable_octets);
    }
    if (buff_has_metadata(file->source_buf))
    {
        buff_metadata_set_usable_octets(file->source_buf, FILE_INPUT_OCTETS_PER_WORD);
    }
#endif

    if (ep->direction == SOURCE)
    {
        file->sink_buf = cb;
    }
    else
    {
        file->source_buf = cb;
    }
    audio_data_service_set_data_buffer_byte_swap(file->service_priv, file->byte_swap);
    audio_data_service_set_data_buffer_shift(file->service_priv, file->shift);
    *start_on_connect = FALSE;
    return TRUE;
}

/**
 * \brief disconnect from the file ep
 * \param ep pointer to the endpoint that is being connected
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool file_disconnect(ENDPOINT *ep)
{
    endpoint_file_state *file = &ep->state.file;
    if (ep->direction == SOURCE)
    {
        file->sink_buf = NULL;
    }
    else
    {
        file->source_buf = NULL;
    }
    return TRUE;
}

static void file_internal_kick(void *priv)
{
    ((ENDPOINT*)(priv))->state.file.internal_kick_timer_id = TIMER_ID_INVALID;
    file_kick((ENDPOINT*)(priv), STREAM_KICK_INTERNAL);
}

/**
 * \brief kick the file ep
 * \param ep pointer to the endpoint that is being kicked
 * \param kick_dir direction of the kick
 * \return TRUE if the call succeeded, FALSE otherwise
 */

static inline void file_kick(ENDPOINT *ep, ENDPOINT_KICK_DIRECTION kick_dir)
{
    endpoint_file_state *file = &ep->state.file;
    unsigned octets_copied_from_source = 0;

    /** To prevent race we let the core kick function run at one and only one priority level
     *  so a kick from an operator is actually postponed slightly so it appears as a kick from
     *  a timer source.
     *  We could have setup the timer only if there was some data available in the input. That
     *  however means we need to call audio_data_service_queue_meta, disabling interupts during
     *  the call. We don't want to do that as it defeats the purpose of having a timer.
     */
    if (kick_dir == STREAM_KICK_BACKWARDS)
    {
        /* Schedule a new event only if there isn't one pending */
        if (file->internal_kick_timer_id == TIMER_ID_INVALID)
        {
            timer_schedule_event_in_atomic(0, file_internal_kick, (void *)(ep), &file->internal_kick_timer_id);
        }
        return;
    }

#ifdef INSTALL_METADATA
    PL_ASSERT(SOURCE == ep->direction);
    if (BUFF_METADATA(file->source_buf))
    {
        audio_data_service_queue_meta(file->service_priv);
    }
#endif
    unsigned available_octets =  cbuffer_calc_amount_data_ex(file->source_buf);
#ifdef INSTALL_METADATA
    if (BUFF_METADATA(file->sink_buf))
    {
        /* Limit to space available according to sink buffer metadata */
        available_octets = buff_metadata_available_space(file->sink_buf);
    }
#endif
    if (ep->connected_to)
    {
        if (0 == ep->state.file.shift)
        {
            /* copy as much as possible. */
            octets_copied_from_source = cbuffer_copy_16bit_be_zero_shift_ex(
                                                file->sink_buf,
                                                file->source_buf,
                                                available_octets);
        }
        else
        {
            /** There can be an extra invalid octet in the buffer (If there's an odd octet and
             *  there is metadata involved, the write pointer is rounded up by the audio data
             *  service). Therefore we always want to copy one word less  when there's an odd
             *  octet in the PCM case.
             */
            octets_copied_from_source =  FILE_INPUT_OCTETS_PER_WORD *
                                             cbuffer_copy(
                                                file->sink_buf,
                                                file->source_buf,
                                                /** Convert octets to words and also ensure that the
                                                 *  following operator won't see an invalid word
                                                 */
                                                available_octets >> 1);
        }
    }

    if (octets_copied_from_source != 0)
    {
#ifdef INSTALL_METADATA
        unsigned b4idx, afteridx;
        metadata_tag * mtag_list = NULL;
        metadata_tag * mtag;
        /** Encoded data should not have the tag lengths modified. PCM data ought to have
         *  to have tag lengths mutliplied by 2. Each sample is viewed as 4 octets at the
         *  output.
         */
        mtag_list = buff_metadata_remove(file->source_buf, octets_copied_from_source, &b4idx, &afteridx);
        if (16 == ep->state.file.shift)
        {
            b4idx *= 2;
            afteridx *= 2;
            mtag = mtag_list;
            while (mtag != NULL)
            {
                mtag->length *= 2;
                mtag = mtag->next;
            }
        }
        buff_metadata_append(file->sink_buf, mtag_list, b4idx, afteridx);
#endif /* INSTALL_METADATA */
        audio_data_service_update_buffers(file->service_priv);
        audio_data_service_kick_consumer(file->service_priv);
        propagate_kick(ep, STREAM_KICK_FORWARDS);
    }
}

/**
 * \brief obtain buffer details of the file ep
 * \param ep pointer to the endpoint for which buffer details are requested
 * \param details pointer to the buffer details structure
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool file_buffer_details(ENDPOINT *ep, BUFFER_DETAILS *details)
{
#ifdef INSTALL_METADATA
    endpoint_file_state *file = &ep->state.file;
    details->supports_metadata = FALSE;
    if (BUFF_METADATA(file->source_buf))
    {
        details->supports_metadata = TRUE;
    }
    details->metadata_buffer = NULL;
#endif /* INSTALL_METADATA */

    details->supplies_buffer = FALSE;
    details->runs_in_place = FALSE;
    details->can_override = details->wants_override = FALSE;
    details->b.buff_params.size = FILE_DATA_BUFFER_SIZE_BYTES / ep->state.file.usable_octets;
    details->b.buff_params.flags = BUF_DESC_SW_BUFFER;

    return TRUE;
}

/*
 * \brief configure a file ep with a key and value pair
 *
 * \param *ep pointer to the file ep being configured
 * \param key key to configure
 * \param value value to be configured with
 *
 * \return TRUE if the call succeeded, FALSE otherwise
 *
 */
static bool file_configure(ENDPOINT *ep, unsigned int key, uint32 value)
{
    switch(key)
    {
        AUDIO_DATA_FORMAT format;
        case EP_DATA_FORMAT:
            if (ep->connected_to != NULL)
            {
                return FALSE;
            }
            if (ep->state.file.running)
            {
                return FALSE;
            }
            format = (AUDIO_DATA_FORMAT)(value);
            if ((format == AUDIO_DATA_FORMAT_16_BIT) ||
                (format == AUDIO_DATA_FORMAT_16_BIT_WITH_METADATA))
            {
                ep->state.file.shift = 0;
                /** For encoded (sbc) data byte swap ought to be set */
                ep->state.file.byte_swap = TRUE;
                ep->state.file.usable_octets = 2;
                return TRUE;
            }
            else if ((format == AUDIO_DATA_FORMAT_FIXP) ||
                (format == AUDIO_DATA_FORMAT_FIXP_WITH_METADATA))
            {
                ep->state.file.shift = 16;
                ep->state.file.byte_swap = FALSE;
                ep->state.file.usable_octets = 4;
                return TRUE;
            }
            else
            {
                return FALSE;
            }
            break;
        default:
            return FALSE;
    }
}

/*
 * \brief retrieve file configuration
 *
 * \param ep pointer to the file ep being configured
 * \param key key for which configuration needs to be retrieved
 * \param value pointer to the value that will be populated
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool file_get_config(ENDPOINT *ep, unsigned int key, ENDPOINT_GET_CONFIG_RESULT* result)
{
    endpoint_file_state * file = &ep->state.file;
    switch(key)
    {
        case EP_DATA_FORMAT:
            if (0 == file->shift)
            {
                result->u.value = (uint32)(AUDIO_DATA_FORMAT_16_BIT);
            }
            else if (16 == file->shift)
            {
                result->u.value = (uint32)(AUDIO_DATA_FORMAT_FIXP);
            }
            else
            {
                return FALSE;
            }
            return TRUE;
        case EP_RATEMATCH_ABILITY:
            result->u.value = (uint32)RATEMATCHING_SUPPORT_AUTO;
            return TRUE;
        case EP_RATEMATCH_RATE:
            result->u.value = (uint32)RM_PERFECT_RATE;
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
 * \brief obtain timing information of the file ep
 * \param ep pointer to the endpoint for which timing information is requested
 * \param time_info pointer to the timing information structure
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static void file_get_timing(ENDPOINT *ep, ENDPOINT_TIMING_INFORMATION *time_info)
{
    time_info->period = 0;
    time_info->block_size = 1;
    time_info->has_deadline = FALSE;
    time_info->locally_clocked = TRUE;
    time_info->wants_kicks = TRUE;
    return;
}

/**
 * \brief start the file ep
 * \param ep pointer to the endpoint being started
 * \param ko pointer to the kick object that called start
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool file_start(ENDPOINT *ep, KICK_OBJECT *ko)
{
    endpoint_file_state * file = &ep->state.file;
    file->internal_kick_timer_id = TIMER_ID_INVALID;
    /* Get data moving so upstream doesn't give up, to prevent any chance of
     * preemption by connected operator the kick function will run in an immediate
     * timer event.
     */
    timer_schedule_event_in_atomic(0, file_internal_kick, (void *)(ep), &file->internal_kick_timer_id);
    audio_data_service_start_kick(file->service_priv);
    
    file->running = TRUE;
    return TRUE;
}

/**
 * \brief stop the file ep
 * \param ep pointer to the endpoint being stopped
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool file_stop(ENDPOINT *ep)
{
    endpoint_file_state * file = &ep->state.file;
    if (!file->running)
    {
        return FALSE;
    }
    else
    {
        audio_data_service_stop_kick(file->service_priv);
        timer_cancel_event(file->internal_kick_timer_id);
        file->internal_kick_timer_id = TIMER_ID_INVALID;
        file->running = FALSE;
        return TRUE;
    }
}

#endif      /* INSTALL_FILE */
