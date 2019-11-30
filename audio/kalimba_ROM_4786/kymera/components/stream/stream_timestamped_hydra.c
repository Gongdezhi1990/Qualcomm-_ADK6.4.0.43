/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \timestamped  stream_timestamped_hydra.c
 * \ingroup stream
 */

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

#ifndef INSTALL_METADATA
#error "Timestamped endpoint requires Kymera metadata support"
#endif

/* Local definition of frame metadata structure
 * See CS-345679-DD
 *
 * Use of this relies on it being correctly aligned within the metadata
 */
typedef struct
{
    uint32 ttp;
    uint32 spa;
    uint8 data;
} frame_metadata_struct;


/* Data service data buffer size (usable octets) */
#define DATA_BUFFER_SIZE_BYTES (1024)

/* Pick a sensible lower limit for packet size (octets).
 * this is used to determine how big the metadata buffer needs to be
 */
#define MIN_PACKET_SIZE (64)

/* Timestamped metadata record is 11 octets, but gets rounded up */
#define TIMESTAMPED_META_SIZE (16)

#define STREAM_START_MASK 0x80

static bool timestamped_close (ENDPOINT *ep);
static bool timestamped_connect (ENDPOINT *ep, tCbuffer *cb,  ENDPOINT *ep_to_kick, bool* start_on_connect);
static bool timestamped_disconnect (ENDPOINT *ep);
static bool timestamped_buffer_details (ENDPOINT *ep, BUFFER_DETAILS *details);
static void timestamped_kick(ENDPOINT *ep, ENDPOINT_KICK_DIRECTION kick_dir);
static bool timestamped_start (ENDPOINT *ep, KICK_OBJECT *ko);
static bool timestamped_stop (ENDPOINT *ep);
static bool timestamped_configure (ENDPOINT *ep, unsigned int key, uint32 value);
static bool timestamped_get_config (ENDPOINT *ep, unsigned int key, ENDPOINT_GET_CONFIG_RESULT* result);
static void timestamped_get_timing (ENDPOINT *ep, ENDPOINT_TIMING_INFORMATION *time_info);

DEFINE_ENDPOINT_FUNCTIONS(timestamped_functions, timestamped_close, timestamped_connect,
                          timestamped_disconnect, timestamped_buffer_details,
                          timestamped_kick, stream_sched_kick_dummy,
                          timestamped_start, timestamped_stop,
                          timestamped_configure, timestamped_get_config,
                          timestamped_get_timing, stream_sync_sids_dummy);

static void audio_data_service_timestamped_kick(void* priv, ENDPOINT_KICK_DIRECTION kick_dir)
{
    timestamped_kick((ENDPOINT*)(priv), kick_dir);
}

bool stream_query_timestamped(AUDIO_DATA_SERVICE_EP_PARAMS *params)
{
    unsigned meta_req_size;
    patch_fn_shared(stream_timestamped);
    params->data_buf_size = MAX(params->data_buf_size, DATA_BUFFER_SIZE_BYTES);

#ifdef BAC32
    params->data_buf_flags = BUF_DESC_UNPACKED_16BIT_MASK;
#else
    params->data_buf_flags = 0;
#endif

    /* Calculate the required metadata buffer size.
     * Fragmentation means each packet might need two metadata records
     * All sizes here are in octets
     */
    meta_req_size = (params->data_buf_size / MIN_PACKET_SIZE) * TIMESTAMPED_META_SIZE * 2;

    params->meta_buf_size = params->meta_buf_size ? MAX(params->meta_buf_size, meta_req_size) : 0;
    params->kick_fn = audio_data_service_timestamped_kick;
    params->start_kicks_immediately = FALSE;
    params->want_octet_buffer = FALSE;
    return TRUE;
}

void stream_create_timestamped(const AUDIO_DATA_SERVICE_EP_CFG *cfg, AUDIO_DATA_SERVICE_EP_CREATE_RESULT* create_res)
{
    ENDPOINT *ep;
    endpoint_timestamped_state *timestamped;

    patch_fn_shared(stream_timestamped);

    ep = STREAM_NEW_ENDPOINT(timestamped, cfg->key, cfg->dir, INVALID_CON_ID);
    if (!ep)
    {
        create_res->success = FALSE;
        return;
    }
    timestamped = &ep->state.timestamped;
    timestamped->service_priv = cfg->service_priv;
    ep->can_be_closed = FALSE;
    ep->can_be_destroyed = FALSE;
    ep->is_real = TRUE;
    audio_data_service_set_data_buffer_byte_swap(cfg->service_priv, TRUE);
    if (cfg->dir == SOURCE)
    {
        timestamped->source_buf = cfg->data_buf;
    }
    else
    {
        timestamped->sink_buf = cfg->data_buf;
    }

    if (buff_has_metadata(cfg->data_buf))
    {
        buff_metadata_set_usable_octets(cfg->data_buf, NR_OF_OCTETS_IN_WORD(USE_16BIT_PER_WORD));
    }

    timestamped->shift = 0;
    create_res->priv = (void*) ep;
    create_res->ext_ep_id = stream_external_id_from_endpoint(ep);
    create_res->success = TRUE;
}

bool stream_destroy_timestamped(void *ep)
{
    ((ENDPOINT*)ep)->can_be_closed = TRUE;
    ((ENDPOINT*)ep)->can_be_destroyed = TRUE;
    return stream_close_endpoint((ENDPOINT*)ep);
}

/**
 * \brief closes the timestamped ep
 * \param ep pointer to the endpoint that is being closed
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool timestamped_close(ENDPOINT *ep)
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
static bool timestamped_connect(ENDPOINT *ep, tCbuffer *cb,  ENDPOINT *ep_to_kick, bool* start_on_connect)
{
    endpoint_timestamped_state *timestamped = &ep->state.timestamped;
    ep->ep_to_kick = ep_to_kick;
    if (buff_has_metadata(cb))
    {
        buff_metadata_set_usable_octets(cb, NR_OF_OCTETS_IN_WORD(USE_16BIT_PER_WORD));
    }

    if (ep->direction == SOURCE)
    {
        timestamped->sink_buf = cb;
    }
    else
    {
        timestamped->source_buf = cb;
    }
    audio_data_service_set_data_buffer_shift(timestamped->service_priv, timestamped->shift);
    *start_on_connect = FALSE;
    return TRUE;
}

/**
 * \brief disconnect from the timestamped ep
 * \param ep pointer to the endpoint that is being connected
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool timestamped_disconnect(ENDPOINT *ep)
{
    endpoint_timestamped_state *timestamped = &ep->state.timestamped;
    if (ep->direction == SOURCE)
    {
        timestamped->sink_buf = NULL;
    }
    else
    {
        timestamped->source_buf = NULL;
    }
    return TRUE;
}

/**
 * metadata_translate
 *
 * Convert between timestamped audio data service metadata and Kymera internal metadata
 * This looks a bit like metadata_strict_transport, but the tag contents change
 */
static metadata_tag *metadata_translate(tCbuffer *src, tCbuffer *dst, unsigned trans_octets, ENDPOINT_DIRECTION dir)
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
                /* Source endpoint, converting to Kymera metadata */
                frame_metadata_struct *frame_metadata;
                unsigned length = 0;
                if (buff_metadata_find_private_data(list_tag, META_PRIV_KEY_USER_DATA, &length, (void **)&frame_metadata)
                    && (length >= sizeof(frame_metadata_struct)))
                {
                    /* Private data items are always word-aligned, so it should be safe to access the structure directly */
                    METADATA_TIMESTAMP_SET(list_tag, frame_metadata->ttp, METADATA_TIMESTAMP_LOCAL);
                    list_tag->sp_adjust = frame_metadata->spa;
                    if ((frame_metadata->data & STREAM_START_MASK) != 0)
                    {
                        METADATA_STREAM_START_SET(list_tag);
                    }
                }

                /* Now tidy up any private data, copying EOF callback info if present */
                if (METADATA_STREAM_END(list_tag))
                {
                    metadata_eof_callback_ref *cb_ref;
                    if (buff_metadata_find_private_data(list_tag, META_PRIV_KEY_EOF_CALLBACK, &length, (void **)&cb_ref))
                    {
                        pfree(list_tag->xdata);
                        list_tag->xdata = NULL;
                        buff_metadata_add_private_data(list_tag, META_PRIV_KEY_EOF_CALLBACK, sizeof(metadata_eof_callback_ref *), &cb_ref);
                    }
                    else
                    {
                        pfree(list_tag->xdata);
                        list_tag->xdata = NULL;
                    }
                }
                else
                {
                    pfree(list_tag->xdata);
                    list_tag->xdata = NULL;
                }
            }
            else
            {
                /* Sink endpoint, converting from Kymera metadata */
                frame_metadata_struct frame_metadata;
                frame_metadata.ttp = list_tag->timestamp;
                frame_metadata.spa = list_tag->sp_adjust;
                if (METADATA_STREAM_START(list_tag) != 0)
                {
                    frame_metadata.data = STREAM_START_MASK;
                }
                else
                {
                    frame_metadata.data = 0;
                }

                if (buff_metadata_add_private_data(list_tag, META_PRIV_KEY_USER_DATA, sizeof(frame_metadata), &frame_metadata) == NULL)
                {
                    /* Failed - probably out of memory ? */
                    L1_DBG_MSG("metadata_translate SINK failed");
                }
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
 * \brief internal kick timer handler
 */

static void timestamped_internal_kick(void *priv)
{
    patch_fn_shared(stream_timestamped);
    ((ENDPOINT*)(priv))->state.timestamped.internal_kick_timer_id = TIMER_ID_INVALID;
    timestamped_kick((ENDPOINT*)(priv), STREAM_KICK_INTERNAL);
}

/**
 * brief Get a count of data covered by complete metadata tags
 */

static unsigned meta_complete_tag_octets(unsigned prev, tCbuffer *cb, unsigned data_available)
{
    metadata_tag *tag;
    unsigned sum = prev;
    PL_ASSERT((cb != NULL) && (cb->metadata != NULL));
    tag = cb->metadata->tags.head;
    while ((tag != NULL) && ((sum + tag->length) <= data_available))
    {
        sum += tag->length;
        tag = tag->next;
    }
    return sum;
}

/**
 * brief Align the source buffer to the next word-aligned metadata tag
 *
 * This is necessary for two reasons - firstly depending on the connected entity
 * it may not be possible to determine the actual byte alignment of the data,
 * and secondly even if it were, the octet copy function requires the alignments
 * to match. So to keep things simple we force the input alignment to be even,
 * knowing the sink buffer will always start up like that.
 */

static bool timestamped_source_buffer_align(ENDPOINT *ep)
{
    tCbuffer *cb;
    endpoint_timestamped_state *timestamped = &ep->state.timestamped;
    unsigned data_avail;

    /* This only makes sense for sink endpoints */
    PL_ASSERT(SINK == ep->direction);

    cb = timestamped->source_buf;

    data_avail = cbuffer_calc_amount_data_in_words(cb);

    if (data_avail == 0)
    {
        /* No input data, so we can't do anything yet */
        return FALSE;
    }

    if (BUFF_METADATA(cb))
    {
        metadata_tag *tag = cb->metadata->tags.head;

        /* Search for the first tag with even index
         * (data is unpacked 16-bit, so that means word-aligned
         */
        while ((tag != NULL) && ((tag->index & 1) != 0))
        {
            /* Odd tag index, keep looking for an even one */
            tag = tag->next;
        }
        if (tag != NULL)
        {
            metadata_tag *taglist;
            int octet_advance = tag->index - cb->metadata->prev_rd_index;
            unsigned before, after;
            unsigned data_advance;

            /* Attempt to align the source buffer to this tag */
            if (octet_advance == 0)
            {
                /* Already aligned, nothing to do */
                return TRUE;
            }

            if (octet_advance < 0)
            {
                octet_advance += cb->metadata->buffer_size;
            }

            /* This will round down if the octet advance is odd.
             * If so the buffer pointer will have been rounded up, so
             * this results in the right aligned position.
             */
            data_advance = octet_advance / buff_metadata_get_usable_octets(cb);

            /* Make sure there's enough data to advance over */
            if (data_advance <= data_avail)
            {
                L2_DBG_MSG4("timestamped_source_buffer_align Buff read = %u Meta read = %u tag index %u octet advance %u",
                    cbuffer_get_read_offset(cb), cb->metadata->prev_rd_index, tag->index, octet_advance);
                cbuffer_advance_read_ptr(cb, octet_advance / buff_metadata_get_usable_octets(cb));
                taglist = buff_metadata_remove(cb, octet_advance, &before, &after);
                buff_metadata_tag_list_delete(taglist);

                return TRUE;
            }
        }
        return FALSE;
    }
    return TRUE;
}

/**
 * \brief kick the timestamped ep
 * \param ep pointer to the endpoint that is being kicked
 * \param kick_dir direction of the kick
 * \return TRUE if the call succeeded, FALSE otherwise
 */

static void timestamped_kick(ENDPOINT *ep, ENDPOINT_KICK_DIRECTION kick_dir)
{
    endpoint_timestamped_state *timestamped = &ep->state.timestamped;
    unsigned data_available = cbuffer_calc_amount_data_ex(timestamped->source_buf);
    unsigned copy_octets, complete_tag_octets = 0;

    /**
     *  To prevent race we let the core kick function run at one and only one priority level
     *  so a kick from an operator is actually postponed slightly so it appears as a kick from
     *  a timer source. Also, we setup a timer only if there's some data to copy.
     */
    if (kick_dir != STREAM_KICK_INTERNAL)
    {
        /* Schedule a new event only if there isn't one pending */
        if ((data_available > 0) && (timestamped->internal_kick_timer_id == TIMER_ID_INVALID))
        {
            timer_schedule_event_in_atomic(0, timestamped_internal_kick, (void *)(ep), &timestamped->internal_kick_timer_id);
        }
        return;
    }

    if (SOURCE == ep->direction)
    {
        if (BUFF_METADATA(timestamped->source_buf))
        {
            audio_data_service_queue_meta(timestamped->service_priv);
        }
    }

    /* Work out how much to copy
     * For source endpoints, as much as possible min(data_available, metadata_available_octets)
     * For sink endpoints, limit to complete metadata tags
     * The metadata is checked to eliminate any race condition between cbuffer and metadata
     * update and to remove the difference between octet and _ex buffers.
     */
    if (BUFF_METADATA(timestamped->source_buf))
    {
        unsigned metadata_available_octets = buff_metadata_available_octets(timestamped->source_buf);
        if (SINK == ep->direction)
        {
            if (!timestamped->aligned)
            {
                timestamped->aligned = timestamped_source_buffer_align(ep);
                if (!timestamped->aligned)
                {
                    return;
                }
            }

            complete_tag_octets = meta_complete_tag_octets(timestamped->tag_octets_remaining,
                timestamped->source_buf, MIN(data_available, metadata_available_octets));
            copy_octets =  complete_tag_octets;
        }
        else
        {
            copy_octets =  MIN(data_available, metadata_available_octets);
        }
    }
    else
    {
        copy_octets = data_available;
    }

    if ((ep->connected_to != NULL) && (copy_octets > 0))
    {
        /* copy as much as possible. */
        copy_octets = cbuffer_copy_16bit_be_zero_shift_ex(timestamped->sink_buf, timestamped->source_buf, copy_octets);
    }

    if (copy_octets != 0)
    {
        timestamped->tag_octets_remaining = complete_tag_octets - copy_octets;

        metadata_translate(timestamped->source_buf,
                  timestamped->sink_buf, copy_octets, ep->direction);

        if (SINK == ep->direction)
        {
            if (BUFF_METADATA(timestamped->sink_buf))
            {
                audio_data_service_dequeue_meta(timestamped->service_priv, copy_octets);
            }
        }

        audio_data_service_update_buffers(timestamped->service_priv);
        audio_data_service_kick_consumer(timestamped->service_priv);

        propagate_kick(ep, (SOURCE == ep->direction ? STREAM_KICK_FORWARDS : STREAM_KICK_BACKWARDS));
    }
}

/**
 * \brief obtain buffer details of the timestamped ep
 * \param ep pointer to the endpoint for which buffer details are requested
 * \param details pointer to the buffer details structure
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool timestamped_buffer_details(ENDPOINT *ep, BUFFER_DETAILS *details)
{
    details->supports_metadata = TRUE;
    details->metadata_buffer = NULL;
    details->supplies_buffer = FALSE;
    details->runs_in_place = FALSE;
    details->can_override = details->wants_override = FALSE;
    /* Format is always 16-bit unpacked, so 2 octets / word */
    details->b.buff_params.size = DATA_BUFFER_SIZE_BYTES / 2;
    details->b.buff_params.flags = BUF_DESC_SW_BUFFER;

    return TRUE;
}

/*
 * \brief configure a timestamped ep with a key and value pair
 *
 * \param *ep pointer to the timestamped ep being configured
 * \param key key to configure
 * \param value value to be configured with
 *
 * \return TRUE if the call succeeded, FALSE otherwise
 *
 */
static bool timestamped_configure(ENDPOINT *ep, unsigned int key, uint32 value)
{
    switch(key)
    {
        case EP_DATA_FORMAT:
        {
            AUDIO_DATA_FORMAT format = (AUDIO_DATA_FORMAT)(value);
            if ((format == AUDIO_DATA_FORMAT_16_BIT) ||
                (format == AUDIO_DATA_FORMAT_16_BIT_WITH_METADATA))
            {
                return TRUE;
            }
            return FALSE;
        }
        default:
            return FALSE;
    }
}

/*
 * \brief retrieve timestamped configuration
 *
 * \param ep pointer to the timestamped ep being configured
 * \param key key for which configuration needs to be retrieved
 * \param value pointer to the value that will be populated
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool timestamped_get_config(ENDPOINT *ep, unsigned int key, ENDPOINT_GET_CONFIG_RESULT* result)
{
    switch(key)
    {
        case EP_DATA_FORMAT:
            result->u.value = (uint32)(AUDIO_DATA_FORMAT_16_BIT);
            return TRUE;
        case EP_RATEMATCH_ABILITY:
            if(SOURCE == ep->direction)
            {
                /* Source rate is driven by incoming data, no adjustment possible */
                result->u.value = (uint32)RATEMATCHING_SUPPORT_NONE;
            }
            else
            {
                /* Sink rate is driven by the connected graph */
                result->u.value = (uint32)RATEMATCHING_SUPPORT_AUTO;
            }
            return TRUE;
        case EP_RATEMATCH_RATE:
            result->u.value = RM_PERFECT_RATE;
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
 * \brief obtain timing information of the timestamped ep
 * \param ep pointer to the endpoint for which timing information is requested
 * \param time_info pointer to the timing information structure
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static void timestamped_get_timing(ENDPOINT *ep, ENDPOINT_TIMING_INFORMATION *time_info)
{
    time_info->period = 0;
    time_info->block_size = 1;
    time_info->has_deadline = FALSE;
    time_info->locally_clocked = TRUE;
    time_info->wants_kicks = TRUE;
    return;
}

/**
 * \brief start the timestamped ep
 * \param ep pointer to the endpoint being started
 * \param ko pointer to the kick object that called start
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool timestamped_start(ENDPOINT *ep, KICK_OBJECT *ko)
{
    ep->state.timestamped.internal_kick_timer_id = TIMER_ID_INVALID;
    /* Get data moving so upstream doesn't give up */
    ep->functions->kick(ep, STREAM_KICK_INTERNAL);
    audio_data_service_start_kick(ep->state.timestamped.service_priv);
    ep->state.timestamped.running = TRUE;
    return TRUE;
}

/**
 * \brief stop the timestamped ep
 * \param ep pointer to the endpoint being stopped
 * \return TRUE if the call succeeded, FALSE otherwise
 */
static bool timestamped_stop(ENDPOINT *ep)
{
    if (ep->state.timestamped.running)
    {
        audio_data_service_stop_kick(ep->state.timestamped.service_priv);
        timer_cancel_event(ep->state.timestamped.internal_kick_timer_id);
        ep->state.timestamped.internal_kick_timer_id = TIMER_ID_INVALID;
        ep->state.timestamped.running = FALSE;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
