/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  stream_sco_hydra.c
 * \ingroup stream
 *
 * stream sco type file. <br>
 * This file contains stream functions for sco endpoints. <br>
 *
 * \section sec1 Contains:
 * stream_sco_get_endpoint <br>
 * stream_create_sco_endpoints_and_cbuffers <br>
 * stream_delete_sco_endpoints_and_cbuffers <br>
 */

/****************************************************************************
Include Files
*/

#include "stream_private.h"
#include "stream/stream_for_sco_processing_service.h"
#include "stream/stream_for_sco_operators.h"
#include "pl_assert.h"

/****************************************************************************
Private Type Declarations
*/

/****************************************************************************
Private Constant Declarations
*/
/** The location of the hci handle in the sco_get_endpoint params */
#define HCI_HANDLE  0

#ifdef SCO_RX_GENERATE_METADATA
/* size of in-band metadata in words*/
#define SCO_IN_BAND_METADATA_HEADER_SIZE 5
/* max packet offset */
#define SCO_RX_TOA_MAX_PACKET_OFFSET 2
#endif
/****************************************************************************
Private Macro Declarations
*/

/****************************************************************************
Private Variable Definitions
*/

/****************************************************************************
Private Function Declarations
*/
static bool sco_close(ENDPOINT *endpoint);
static bool sco_connect(ENDPOINT *endpoint, tCbuffer *Cbuffer_ptr, ENDPOINT *ep_to_kick, bool* start_on_connect);
static bool sco_disconnect(ENDPOINT *endpoint);
static void sco_kick(ENDPOINT *endpoint, ENDPOINT_KICK_DIRECTION kick_dir);
static void sco_get_timing (ENDPOINT *endpoint, ENDPOINT_TIMING_INFORMATION *time_info);
static bool sco_buffer_details(ENDPOINT *endpoint, BUFFER_DETAILS *details);
#ifdef SCO_RX_GENERATE_METADATA
static void sco_rx_generate_metadata(ENDPOINT *endpoint);
static void  sco_rx_reset_toa_packet_offset(endpoint_sco_state *sco);
#endif
DEFINE_ENDPOINT_FUNCTIONS(sco_functions, sco_close, sco_connect,
                          sco_disconnect, sco_buffer_details,
                          sco_kick, sco_sched_kick, sco_start,
                          sco_stop, sco_configure, sco_get_config,
                          sco_get_timing, stream_sync_sids_dummy);

/****************************************************************************
Public Function Definitions
*/

/****************************************************************************
 *
 * stream_sco_get_endpoint
 *
 */
ENDPOINT *stream_sco_get_endpoint(unsigned con_id, ENDPOINT_DIRECTION dir,
                                    unsigned num_params, unsigned *params)
{
    ENDPOINT *ep;
    unsigned key;

    /* Expect an hci handle and potentially some padding */
    if (num_params < 1)
    {
        L3_DBG_MSG("hydra stream_sco_get_endpoint (num_params < 1) return NULL");
        return NULL;
    }
    /* The hci handle forms the key (unique for the type and direction) */
    key = params[HCI_HANDLE];

    L3_DBG_MSG1("hydra stream_sco_get_endpoint hci handle: %d", key);

    /* Return the requested endpoint (NULL if not found) */
    ep = stream_get_endpoint_from_key_and_functions(key, dir,
                                                  &endpoint_sco_functions);
    if (ep)
    {
        /* The endpoint has been created, however we now need to check
           the ID */
        if (ep->con_id == INVALID_CON_ID)
        {
            ep->con_id = con_id;
        }
        /* If the client does not own the endpoint they can't access it. */
        else if (ep->con_id != con_id)
        {
            L3_DBG_MSG("hydra stream_sco_get_endpoint (ep->con_id != con_id) return NULL");
            return NULL;
        }
    }
    L3_DBG_MSG1("hydra stream_sco_get_endpoint normal execution ep: %d", ep);
    return ep;
}

/****************************************************************************
 *
 * stream_create_sco_endpoints_and_cbuffers
 *
 */
bool stream_create_sco_endpoints_and_cbuffers(unsigned int hci_handle,
                                              sco_buf_desc *source_buf_desc,
                                              sco_buf_desc *sink_buf_desc,
                                              tCbuffer **source_cbuffer,
                                              tCbuffer **sink_cbuffer)
{
    ENDPOINT *source_ep, *sink_ep = NULL;
    unsigned flags, shift;
    patch_fn_shared(stream_sco);

    /* Clear contents of source and sink cbuffer pointer parameters as a
     * precaution to minimise the risk of a caller treating them as valid
     * pointers in the event of the function failing and returning FALSE.
     */
    *source_cbuffer = NULL;
    *sink_cbuffer = NULL;

    /* The hci handle is the key (unique for the type and direction) */
    unsigned key = hci_handle;


    L3_DBG_MSG1("!!!!!!!!hydra stream_create_sco_endpoints_and_cbuffers hci handle: %d", key);

    /* Check that we don't already have a source endpoint for the
     * specified hci handle.
     */
    if (stream_get_endpoint_from_key_and_functions(key, SOURCE,
                                            &endpoint_sco_functions) != NULL)
    {
        /* Caller should not have called us for a second time without
         * deleting the existing buffers first.
         */
        panic(PANIC_AUDIO_SCO_BUFFERS_ALREADY_EXIST);
    }


    /* Create and initialise a source endpoint
     * ---------------------------------------
     */
    if ((source_ep = STREAM_NEW_ENDPOINT(sco, key, SOURCE, INVALID_CON_ID)) == NULL)
    {
        goto handle_error;
    }
    source_ep->can_be_closed = FALSE;
    source_ep->can_be_destroyed = FALSE;
    /* SCO endpoints are always at the end of a chain */
    source_ep->is_real = TRUE;

    if (source_buf_desc->is_remote)
    {
        /* We're just going to wrap a remote buffer and handles with the
         * requirement that we can modify the read handle.
         */
        source_ep->state.sco.cbuffer = cbuffer_wrap_remote(
                                            BUF_DESC_WRAP_REMOTE_MMU_MOD_RD,
                                            source_buf_desc->remote_rd_handle,
                                            source_buf_desc->remote_wr_handle,
                                            source_buf_desc->size );
    }
    else
    {
#if defined(BAC32)
        flags = BUF_DESC_UNPACKED_16BIT_MASK;
        shift = 16;
#elif defined(BAC24)
        flags = 0;
        shift = 8;
#else
#error "SCO buffer flags and shift undefined for this BAC"
#endif
        /* We're hosting the buffer so create the source buffer and handles.
         * This includes a third auxiliary handle which is a write handle.
         */
        source_ep->state.sco.cbuffer = cbuffer_create_mmu_buffer(
                                          flags | BUF_DESC_MMU_BUFFER_AUX_WR,
                                          &source_buf_desc->size);
        cbuffer_set_write_shift(source_ep->state.sco.cbuffer, shift);
    }

    if (source_ep->state.sco.cbuffer == NULL)
    {
        goto handle_error;
    }

#ifdef SCO_RX_GENERATE_METADATA
    L2_DBG_MSG1("SCO_RX_GENERATE_METADATA, gen metadata enabled, buf=0x%x", (unsigned)(uintptr_t)source_ep->state.sco.cbuffer);
    /* by default the endpoint will generate metadata */
    source_ep->state.sco.generate_metadata = TRUE;
#endif /* SCO_RX_GENERATE_METADATA */

    /* Create and initialise a sink endpoint
     * -------------------------------------
     */
    if ((sink_ep = STREAM_NEW_ENDPOINT(sco, key, SINK, INVALID_CON_ID)) == NULL)
    {
        goto handle_error;
    }
    sink_ep->can_be_closed = FALSE;
    sink_ep->can_be_destroyed = FALSE;
    /* SCO endpoints are always at the end of a chain */
    sink_ep->is_real = TRUE;

    if (sink_buf_desc->is_remote)
    {
        /* We're just going to wrap a remote buffer and handles with the
         * requirement that we can modify the write handle.
         */
        sink_ep->state.sco.cbuffer = cbuffer_wrap_remote(
                                              BUF_DESC_WRAP_REMOTE_MMU_MOD_WR,
                                              sink_buf_desc->remote_rd_handle,
                                              sink_buf_desc->remote_wr_handle,
                                              sink_buf_desc->size );
    }
    else
    {
        /* We're hosting the buffer so create the sink buffer and handles.
         * This includes a third auxiliary handle which is a read handle.
         */
#if defined(BAC32)
        flags = BUF_DESC_UNPACKED_16BIT_MASK;
        shift = 16;
#elif defined(BAC24)
        flags = 0;
        shift = 8;
#else
#error "SCO buffer flags and shift undefined for this BAC"
#endif
        sink_ep->state.sco.cbuffer = cbuffer_create_mmu_buffer(
                                              flags | BUF_DESC_MMU_BUFFER_AUX_RD,
                                              &sink_buf_desc->size );
        cbuffer_set_read_shift(source_ep->state.sco.cbuffer, shift);
    }

    if (sink_ep->state.sco.cbuffer == NULL)
    {
        goto handle_error;
    }

#ifdef SCO_RX_GENERATE_METADATA
    if(source_ep->state.sco.generate_metadata)
    {
        /* source buffer will have metadata, it will be enabled during connection process */
        source_ep->state.sco.prev_write_offset = cbuffer_get_write_offset(source_ep->state.sco.cbuffer);
        L2_DBG_MSG2("SCO_RX_GENERATE_METADATA, buf configured, buf=0x%x, prev_offset=%d",
                    (unsigned)(uintptr_t)source_ep->state.sco.cbuffer,
                    source_ep->state.sco.prev_write_offset);
    }
#endif /* SCO_RX_GENERATE_METADATA */

    /* initialise measured rate for both sides */
    source_ep->state.sco.rate_measurement = 1<<STREAM_RATEMATCHING_FIX_POINT_SHIFT;
    sink_ep->state.sco.rate_measurement = 1<<STREAM_RATEMATCHING_FIX_POINT_SHIFT;
#ifdef INSTALL_SCO_EP_CLRM
    source_ep->state.sco.rm_enable_clrm_measurement = TRUE;
    source_ep->state.sco.rm_enable_clrm_trace = FALSE;
    sink_ep->state.sco.rm_enable_clrm_measurement = TRUE;
    sink_ep->state.sco.rm_enable_clrm_trace = FALSE;
#endif

    /* Update incoming pointer parameters to give caller access to the
     * created source and sink cbuffer structures.
     */
    *source_cbuffer = source_ep->state.sco.cbuffer;
    *sink_cbuffer = sink_ep->state.sco.cbuffer;

    /* Succeeded */
    return TRUE;

handle_error:
    /* Cleanup source endpoint and cbuffer if they exist */
    if (source_ep != NULL)
    {
        if (source_ep->state.sco.cbuffer != NULL)
        {
            /* Free up the buffer and associated data space */
            cbuffer_destroy(source_ep->state.sco.cbuffer);
        }
        source_ep->can_be_destroyed = TRUE;
        stream_destroy_endpoint(source_ep);
    }

    /* Cleanup sink endpoint and cbuffer if they exist */
    if (sink_ep != NULL)
    {
        if (sink_ep->state.sco.cbuffer != NULL)
        {
            /* Free up the buffer and associated data space */
            cbuffer_destroy(sink_ep->state.sco.cbuffer);
        }
        sink_ep->can_be_destroyed = TRUE;
        stream_destroy_endpoint(sink_ep);
    }

    /* Failed */
    return FALSE;
}

/****************************************************************************
 *
 * stream_delete_sco_endpoints_and_cbuffers
 *
 */
void stream_delete_sco_endpoints_and_cbuffers(unsigned int hci_handle)
{
    ENDPOINT *ep;
    tCbuffer *temp_buffer_ptr;

    /* The hci handle is the key (unique for the type and direction) */
    unsigned key = hci_handle;
    patch_fn_shared(stream_sco);

    /* Get and close the source endpoint associated with the hci handle */
    if ((ep = stream_get_endpoint_from_key_and_functions(key, SOURCE,
                                           &endpoint_sco_functions)) != NULL)
    {
        /* Remember the buffer that we need to free */
        temp_buffer_ptr = ep->state.sco.cbuffer;

        ep->can_be_closed = TRUE;
        ep->can_be_destroyed = TRUE;
        stream_close_endpoint(ep);

        /* Free up the buffer and associated data space */
        cbuffer_destroy(temp_buffer_ptr);
    }

    /* Get and close the sink endpoint associated with the hci handle */
    if ((ep = stream_get_endpoint_from_key_and_functions(key, SINK,
                                           &endpoint_sco_functions)) != NULL)
    {
        /* Remember the buffer that we need to free */
        temp_buffer_ptr = ep->state.sco.cbuffer;

        ep->can_be_closed = TRUE;
        ep->can_be_destroyed = TRUE;
        stream_close_endpoint(ep);

        /* Free up the buffer and associated data space */
        cbuffer_destroy(temp_buffer_ptr);
    }
}

/****************************************************************************
Private Function Definitions
*/

static bool sco_close(ENDPOINT *endpoint)
{
    /* The endpoint still persists, but it has been released so potentially
     * another user can do something with it */
    endpoint->con_id = INVALID_CON_ID;

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
static bool sco_connect(ENDPOINT *endpoint, tCbuffer *Cbuffer_ptr, ENDPOINT *ep_to_kick, bool* start_on_connect)
{
    endpoint->ep_to_kick = ep_to_kick;
    if (SOURCE == endpoint->direction)
    {
#ifdef SCO_RX_GENERATE_METADATA
        if(endpoint->state.sco.generate_metadata)
        {
            if(!buff_has_metadata(Cbuffer_ptr))
            {
                /* The connected operator is expected to support metadata, so at
                 * this point buffer should have metadata enabled, generate fault
                 * if it isn't enabled.
                 */
                L2_DBG_MSG("SCO_RX_GENERATE_METADATA, buffer doesn't have metadata");
                fault_diatribe(FAULT_SCO_METADATA_NOT_PRESENT, (DIATRIBE_TYPE)0);
            }
            else
            {
                L2_DBG_MSG("SCO_RX_GENERATE_METADATA, connect with metadata");
            }
        }
#endif /* SCO_RX_GENERATE_METADATA */

        /* Make a record of the SCO source endpoint in the sps structure for
         * first kick scheduling calculations. */
        sco_from_air_endpoint_set(stream_sco_get_hci_handle(endpoint), endpoint);


    }
    else
    {
        /* Make a record of the SCO sink endpoint in the sps structure for
         * first kick scheduling calculations. */
        sco_to_air_endpoint_set(stream_sco_get_hci_handle(endpoint), endpoint);
    }
    *start_on_connect = FALSE;
    return TRUE;
}

static bool sco_disconnect(ENDPOINT *endpoint)
{
    /* Clear out any configured hardware bit shift; this is necessary if
     * the SCO endpoint is to be subsequently reconnected.
     */
    cbuffer_set_read_shift(endpoint->state.sco.cbuffer, 0);
    cbuffer_set_write_shift(endpoint->state.sco.cbuffer, 0);

#ifdef SCO_RX_GENERATE_METADATA
    buff_metadata_release(endpoint->state.sco.cbuffer);
#endif

    return TRUE;
}

#ifdef SCO_RX_GENERATE_METADATA
/**
 * sco_rx_reset_toa_packet_offset
 * \brief resets the toa packet offset procedure
 * \param sco pointer to sco endpoint state structure
 */
static void  sco_rx_reset_toa_packet_offset(endpoint_sco_state *sco)
{
    sco->packet_offset = 0;
    sco->packet_offset_counter = 0;
    sco->packet_offset_stable = FALSE;
}

/**
 * sco_rx_generate_metadata
 * \brief generates metadata for sco rx
 * \param endpoint pointer to sco rx endpoint structure
 */
static void sco_rx_generate_metadata(ENDPOINT *endpoint)
{
    endpoint_sco_state *sco = &endpoint->state.sco;
    unsigned new_write_offset = cbuffer_get_write_offset(sco->cbuffer);
    unsigned buf_size = cbuffer_get_size_in_words(sco->cbuffer);
    unsigned new_words_received;
    int new_packets = 0;

    patch_fn_shared(stream_sco);

    if(!buff_has_metadata(sco->cbuffer))
    {
        /* metadata isn't enabled */
        return;
    }

    /* work out the amount of new samples received */
    if(new_write_offset >= sco->prev_write_offset)
    {
        new_words_received = new_write_offset - sco->prev_write_offset;
    }
    else
    {
        new_words_received = (buf_size + new_write_offset) - sco->prev_write_offset;
    }

    if(new_words_received> buf_size)
    {
        /* This shouldn't happen */
        L0_DBG_MSG4("SCO_RX_GENERATE_METADATA, ASSERT, prev=%d, new=%d, buf_size=%d, new_w=%d",
                    sco->prev_write_offset,
                    new_write_offset,buf_size,
                    new_words_received);
    }

    if(new_words_received != 0)
    {
        metadata_tag *mtag;
        unsigned b4idx, afteridx;
        int32 wallclock_value = sco_wallclock_get(endpoint->key);
        TIME time_of_arrival = time_sub(sco->data_avail_time, wallclock_value);
        unsigned packet_size = sco_from_air_length_get(endpoint->key) + SCO_IN_BAND_METADATA_HEADER_SIZE;

        /* store new offset for later use */
        sco->prev_write_offset = new_write_offset;

        /* time of arrival for the first sample of this chunk */
        time_of_arrival = time_add(time_of_arrival, (int)STREAM_KICK_PERIOD_TO_USECS(sco->kick_period)*(sco->packet_offset-1));

        while(new_words_received > 0)
        {
            /* set the tag size to packet_size,
             * Note: new_words_received is always a multiple of packet_size
             * unless a wrapping around has happened
             */
            unsigned tag_size = MIN(new_words_received, packet_size);

            /* Create a new tag for the output */
            b4idx = 0;
            afteridx = tag_size*OCTETS_PER_SAMPLE;
            mtag = buff_metadata_new_tag();
            if (mtag != NULL)
            {
                /* length of this tag in octets */
                mtag->length = afteridx;

                /* convert rate adjust to fractional value */
#ifdef INSTALL_SCO_EP_CLRM
                mtag->sp_adjust = sco->rm_result.sp_deviation;
#else /* INSTALL_SCO_EP_CLRM */
                mtag->sp_adjust = STREAM_RATEMATCHING_RATE_TO_FRAC(sco->rate_measurement);
#endif /* INSTALL_SCO_EP_CLRM */

                /* set the time of arrival */
                METADATA_TIME_OF_ARRIVAL_SET(mtag, time_of_arrival);

                STREAM_METADATA_DBG_MSG5("SCO_RX_GENERATE_METADATA, NEW TAG ADDED, adjust=0x%08x,"
                                         "toa=0x%08x(%dus in the past), packet_offset=%d, tag_size=%d",
                                         (unsigned)mtag->sp_adjust,
                                         mtag->timestamp,
                                         time_sub(hal_get_time(), mtag->timestamp),
                                         sco->packet_offset,
                                         tag_size);
            }
            else
            {
                STREAM_METADATA_DBG_MSG1("SCO_RX_GENERATE_METADATA, NULL TAG ADDED, time=0x%08x", hal_get_time());
            }

            /* append generated metadata to the output buffer */
            buff_metadata_append(sco->cbuffer, mtag, b4idx, afteridx);
            
            /* update time of arrival for next tag */
            time_of_arrival = time_add(time_of_arrival, (int)STREAM_KICK_PERIOD_TO_USECS(sco->kick_period));

            /* one packet tagged */
            new_words_received -= tag_size;
            new_packets++;
        }
    }
    else /* new_words_received != 0 */
    {
        STREAM_METADATA_DBG_MSG2("SCO_RX_GENERATE_METADATA, No new packet observed: packet_offset=%d, time=0x%08x",
                                 sco->packet_offset, hal_get_time());
    }

    /* Normally in audio side we expect to see the arrival of one new
     * packet in each kick. However for different reasons it is possible
     * that from time to time we don't see the arrival of a packet at kick
     * time, instead we will see two new packets in next kick.
     * To make sure that each arrived chunk is tagged with proper toa,
     * we keep a counter for not-seen packets so the toa will be adjusted
     * accordingly.
     */
    if(sco->packet_offset_stable)
    {
        /* we expect one packet in each kick, any difference will be added to offset */
        sco->packet_offset += new_packets;
        sco->packet_offset -= 1;
        if(pl_abs_i32(sco->packet_offset) > SCO_RX_TOA_MAX_PACKET_OFFSET)
        {
            /* shall not happen, for piece of mind */
            sco_rx_reset_toa_packet_offset(sco);
        }
    }
    else /* sco->packet_offset_stable */
    {
        if(new_packets == 1)
        {
            sco->packet_offset_counter++;
            if(sco->packet_offset_counter >= 3)
            {
                /* if we see arrival of one packet for three consecutive kicks
                 * then we can assume that we are in offset 0 */
                sco->packet_offset_stable = TRUE;
                sco->packet_offset = 0;
            }
        }
        else /* new_packets == 1 */
        {
            if(new_packets == 0 &&
               sco->packet_offset_counter > 0)
            {
                /* also if we don't see any packet,
                 * but have seen one in previous kick,
                 * then we are one packet behind.
                 */
                sco->packet_offset_stable = TRUE;
                sco->packet_offset = -1;
            }
            else
            {
                /* will keep using offset 0. Normally we
                 * quickly go to stable mode.
                 */
                sco->packet_offset_counter = 0;
            }
        } /* new_packets == 1 */
    }
}
#endif /* #ifdef SCO_RX_GENERATE_METADATA */

/**
 * \brief Kicks down the chain only if this is a source.
 *
 * \param endpoint pointer to the endpoint that received a kick
 * \param kick_dir kick direction
 */
static void sco_kick(ENDPOINT *endpoint, ENDPOINT_KICK_DIRECTION kick_dir)
{
    /* This endpoint is a hard deadline so the operator is tied to the
     * endpoint scheduling and expects an INTERNAL kick to the connected to endpoint. */
#ifdef SCO_RX_GENERATE_METADATA
    if(SOURCE == endpoint->direction &&
        endpoint->state.sco.generate_metadata)
    {
        sco_rx_generate_metadata(endpoint);
    }
#endif /* SCO_RX_GENERATE_METADATA */

    /* At connect the endpoint asked not to be kicked. If it is a source endpoint
     * the kick object kicks this endpoint. In this manner this function is only called
     * for source endpoints. So the type of endpoint and direction aren't checked. */
	 propagate_kick(endpoint, STREAM_KICK_INTERNAL);
}

void sco_get_timing (ENDPOINT *endpoint, ENDPOINT_TIMING_INFORMATION *time_info)
{
    /* Most of the work here is the same on all platforms, the wants kicks field
     * differs and is populated by the platform specific code. */
    sco_common_get_timing(endpoint, time_info);

    time_info->wants_kicks = FALSE;
}

static bool sco_buffer_details(ENDPOINT *endpoint, BUFFER_DETAILS *details)
{
    if (endpoint == NULL || details == NULL)
    {
        return FALSE;
    }

#ifdef SCO_RX_GENERATE_METADATA
    if(endpoint->state.sco.generate_metadata &&
       SOURCE == endpoint->direction)
    {
        L2_DBG_MSG("SCO_RX_GENERATE_METADATA, buffer detail, return support");

        /* SCO Rx can optionally generate metadata */
        details->supports_metadata = TRUE;
        details->metadata_buffer = endpoint->state.sco.cbuffer;
    }
#endif /* SCO_RX_GENERATE_METADATA */

    details->supplies_buffer = TRUE;
    details->runs_in_place = FALSE;
    details->b.buffer = endpoint->state.sco.cbuffer;
    return TRUE;
}

/**
 * sco_get_data_format
 */
AUDIO_DATA_FORMAT sco_get_data_format (ENDPOINT *endpoint)
{
    patch_fn_shared(stream_sco);
    if(stream_direction_from_endpoint(endpoint) == SOURCE)
    {
        if (cbuffer_get_write_shift(endpoint->state.sco.cbuffer) == (DAWTH- 16))
        {
            return AUDIO_DATA_FORMAT_FIXP_WITH_METADATA;
        }
        return AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP_WITH_METADATA;
    }
    else
    {
        if (cbuffer_get_read_shift(endpoint->state.sco.cbuffer) == (DAWTH- 16))
        {
            return AUDIO_DATA_FORMAT_FIXP;
        }
        return AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP;
    }
}

/**
 * sco_set_data_format
 */
bool sco_set_data_format (ENDPOINT *endpoint, AUDIO_DATA_FORMAT format)
{
    patch_fn_shared(stream_sco);
    /* The data format can only be set before connect */
    if (NULL != endpoint->connected_to)
    {
        return FALSE;
    }

    /* Sources and sinks have different data formats due to metadata */
    if(stream_direction_from_endpoint(endpoint) == SOURCE)
    {
        switch(format)
        {
        case AUDIO_DATA_FORMAT_FIXP_WITH_METADATA:
            cbuffer_set_write_shift(endpoint->state.sco.cbuffer, DAWTH - 16);
            cbuffer_set_write_byte_swap(endpoint->state.sco.cbuffer, FALSE);
            return TRUE;
        case AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP_WITH_METADATA:
            cbuffer_set_write_shift(endpoint->state.sco.cbuffer, 0);
            cbuffer_set_write_byte_swap(endpoint->state.sco.cbuffer, TRUE);
            return TRUE;
        default:
            return FALSE;
        }
    }
    else
    {
        switch(format)
        {
        case AUDIO_DATA_FORMAT_FIXP:
            cbuffer_set_read_shift(endpoint->state.sco.cbuffer, DAWTH - 16);
            cbuffer_set_read_byte_swap(endpoint->state.sco.cbuffer, FALSE);
            return TRUE;
        case AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP:
            cbuffer_set_read_shift(endpoint->state.sco.cbuffer, 0);
            cbuffer_set_read_byte_swap(endpoint->state.sco.cbuffer, TRUE);
            return TRUE;
        default:
            return FALSE;
        }
    }
}

/**
 * flush_endpoint_buffers
 */
void flush_endpoint_buffers(ENDPOINT *ep)
{
    tCbuffer* cbuffer = stream_transform_from_endpoint(ep)->buffer;
    patch_fn_shared(stream_sco);

    /* Wipe the buffer completely - history starts now, nobody knows what ended up in the buffer so far.
     * As it is SCO endpoint, the zero value used is... zero.
     */

    if(ep->direction == SOURCE)
    {
#ifdef SCO_RX_GENERATE_METADATA
        cbuffer_empty_buffer_and_metadata(cbuffer);
#else
        cbuffer_empty_buffer(cbuffer);
#endif
    }
    else
    {
        cbuffer_flush_and_fill(cbuffer, 0);
    }
}

/**
 * stream_sco_reset_sco_metadata_buffer_offset
 *
 * NOTE: the buffer must be fully cleared before running this
 *       function.
 */
void stream_sco_reset_sco_metadata_buffer_offset(ENDPOINT *ep)
{
#ifdef SCO_RX_GENERATE_METADATA
    /* make sure this is really a sco RX endpoint */
    if(NULL == ep ||
       ep->stream_endpoint_type != endpoint_sco ||
       SOURCE != ep->direction )
    {
        return;
    }

    /* reset the prev write offset for the metadata generation */
    ep->state.sco.prev_write_offset = cbuffer_get_read_offset(ep->state.sco.cbuffer);

    /* reset sco packet offset */
    sco_rx_reset_toa_packet_offset(&ep->state.sco);

#else
    UNUSED(ep);
#endif
}
