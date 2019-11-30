/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  source_sync_metadata_rcv.c
 * \ingroup  capabilities
 *
 *  src_sync operator
 *
 */
/*
 * This file implements functions which keep track of metadata
 * received on a sink group
 */

#include "source_sync_defs.h"

#if defined(INSTALL_METADATA)

/****************************************************************************
Private Function Declarations
*/

static void src_sync_trace_rcv_tags(
        SRC_SYNC_OP_DATA* op_extra_data,
        SRC_SYNC_SINK_GROUP* sink_grp);
static void src_sync_update_rcv_time(
        SRC_SYNC_OP_DATA* op_extra_data,
        metadata_tag* tag, SRC_SYNC_SINK_METADATA* ts_state,
        unsigned rcv_afteridx_octets, unsigned transfer_octets,
        unsigned sample_rate, unsigned src_grp_idx, bool is_primary );


/****************************************************************************
Private Function Definitions
*/

/**
 * Interpret a last metadata tag of a buffer to see if it has
 * any timing i.e. TTP or ToA. If it does, extrapolate its
 * timestamp to the next sample past the buffer.
 */
static void src_sync_update_rcv_time(
        SRC_SYNC_OP_DATA* op_extra_data,
        metadata_tag* tag,
        SRC_SYNC_SINK_METADATA* ts_state,
        unsigned rcv_afteridx_octets,
        unsigned transfer_octets,
        unsigned inv_sample_rate,
        unsigned sink_grp_idx,
        bool is_primary )
{
    if (tag != NULL)
    {
        RATE_TIMESTAMP_TYPE ts_type = rate_metadata_get_timestamp_type(tag);

#ifdef SOSY_VERBOSE
        if ( (ts_type != ts_state->timestamp_type)
             && (ts_type != RATE_TIMESTAMP_TYPE_NONE)
             && (ts_state->timestamp_type != RATE_TIMESTAMP_TYPE_NONE) )
        {
            SOSY_MSG3( SRC_SYNC_TRACE_RCV_METADATA,
                       "src_sync pt sink_g%d rcv ts type changed %d->%d",
                       sink_grp_idx, ts_state->timestamp_type, ts_type );
        }
#endif /* SOSY_VERBOSE */

        /* Ignore non-timestamps; continue extrapolating timing */
        if (ts_type != RATE_TIMESTAMP_TYPE_NONE)
        {
            unsigned afteridx_samples =
                    rcv_afteridx_octets / OCTETS_PER_SAMPLE;
            ts_state->timestamp_type = ts_type;

            /* Extrapolate the timestamp of the
             * tag to the end of this buffer
             */
            ts_state->sp_adjust = tag->sp_adjust;
            ts_state->ts_start_of_next_buffer =
                src_sync_get_next_timestamp(
                        tag->timestamp, afteridx_samples,
                        inv_sample_rate, tag->sp_adjust);

            SOSY_MSG3( SRC_SYNC_TRACE_RCV_GAP,
                       "tag t %d + after_w %d = tsonb %d",
                       tag->timestamp,
                       afteridx_samples,
                       ts_state->ts_start_of_next_buffer );

            if (rcv_afteridx_octets <= tag->length)
            {
                ts_state->remaining_octets =
                        (tag->length - rcv_afteridx_octets);
            }
            else
            {
                ts_state->remaining_octets = 0;
            }

            /* For now, just save any received value */
            if (is_primary)
            {
                op_extra_data->primary_sp_adjust = tag->sp_adjust;
            }

            return;
        }
    }

    /* There was no metadata, or there was but it provided no timestamps */
    if ( ts_state->remaining_octets >= transfer_octets)
    {
        ts_state->remaining_octets -= transfer_octets;
    }
    else
    {
        ts_state->remaining_octets = 0;
    }

    if (ts_state->timestamp_type != RATE_TIMESTAMP_TYPE_NONE)
    {
        unsigned transfer_w = transfer_octets/OCTETS_PER_SAMPLE;
#ifdef SOSY_VERBOSE
        unsigned old_tsonb = ts_state->ts_start_of_next_buffer;
#endif
        ts_state->ts_start_of_next_buffer =
            src_sync_get_next_timestamp(
                    ts_state->ts_start_of_next_buffer, transfer_w,
                    inv_sample_rate, ts_state->sp_adjust );

        SOSY_MSG3( SRC_SYNC_TRACE_RCV_GAP,
                   "prev tsonb %d + len_w %d = tsonb %d",
                   old_tsonb, transfer_w,
                   ts_state->ts_start_of_next_buffer );
    }
}

static void src_sync_trace_rcv_tags( SRC_SYNC_OP_DATA* op_extra_data,
                                     SRC_SYNC_SINK_GROUP* sink_grp)
{
#ifdef SOSY_VERBOSE
    TIME now = hal_get_time();
    SRC_SYNC_SINK_METADATA* md = &sink_grp->timestamp_state;
    metadata_tag *tag = md->received;

    if (tag != NULL)
    {
        SOSY_MSG4( SRC_SYNC_TRACE_RCV_METADATA,
                   "pt sink_g%d rcv %d tags before_oct %d after_oct %d",
                   sink_grp->common.idx,
                   src_sync_metadata_count_tags(tag),
                   md->rcv_beforeidx, md->rcv_afteridx);

        while (tag != NULL)
        {
            if (IS_TIMESTAMPED_TAG(tag))
            {
                SOSY_MSG4(  SRC_SYNC_TRACE_RCV_METADATA,
                            "pt sink_g%d rcv ttp "
                            "bf_oct %d t 0.%04d fut oct %d",
                            sink_grp->common.idx, md->rcv_beforeidx,
                            time_sub(tag->timestamp, now)/100, tag->length);
            }
            else if (IS_TIME_OF_ARRIVAL_TAG(tag))
            {
                SOSY_MSG4(  SRC_SYNC_TRACE_RCV_METADATA,
                            "pt sink_g%d rcv toa "
                            "bf_oct %d t 0.%04d past oct %d",
                            sink_grp->common.idx, md->rcv_beforeidx,
                            time_sub(now, tag->timestamp)/100, tag->length);
            }
            else if (IS_VOID_TTP_TAG(tag))
            {
                SOSY_MSG3(  SRC_SYNC_TRACE_RCV_METADATA,
                            "pt sink_g%d rcv void bf_oct %d oct %d",
                            sink_grp->common.idx, md->rcv_beforeidx,
                            tag->length);
            }
            else
            {
                SOSY_MSG4(  SRC_SYNC_TRACE_RCV_METADATA,
                            "pt sink_g%d rcv other tag flags 0x%02x "
                            "bf_oct %d oct %d",
                            sink_grp->common.idx, tag->flags,
                            md->rcv_beforeidx, tag->length);
            }
            if (METADATA_STREAM_START(tag))
            {
                SOSY_MSG( SRC_SYNC_TRACE_RCV_METADATA,
                          "+ stream_start");
            }
            tag = tag->next;
        }
    }
    else
    {
        SOSY_MSG1( SRC_SYNC_TRACE_RCV_METADATA,
                   "pt sink_g%d rcv no tags",
                   sink_grp->common.idx);
    }
#endif /* SOSY_VERBOSE */
}

/****************************************************************************
Module Function Definitions
*/

/**
 * Function to read metadata from an explicitly specified cbuffer
 * and save the result in a SRC_SYNC_SINK_METADATA structure.
 * This can be used either for processing via src_sync_get_sink_metadata
 * or with the connection input buffers for rate adjustment.
 */
bool src_sync_get_input_metadata(
        SRC_SYNC_OP_DATA* op_extra_data, SRC_SYNC_SINK_GROUP* sink_grp,
        SRC_SYNC_SINK_METADATA* md, tCbuffer* input_metadata_buffer,
        unsigned* remove_octets )
{
    unsigned available;

    available = buff_metadata_available_octets(input_metadata_buffer);

    if (available < *remove_octets)
    {
        L2_DBG_MSG4( "src_sync 0x%04x WARNING sink_g%d md "
                     "avail %d less than expected %d",
                     op_extra_data->id, sink_grp->common.idx, available,
                     *remove_octets);

        *remove_octets = available;
    }

    if (*remove_octets > 0)
    {
        md->received =
                buff_metadata_remove(
                        input_metadata_buffer, *remove_octets,
                        &md->rcv_beforeidx, &md->rcv_afteridx );

#ifdef SOSY_CHECK_METADATA_TAG_COUNTS
        op_extra_data->num_tags_received +=
                src_sync_metadata_count_tags(md->received);
#endif /* SOSY_CHECK_METADATA_TAG_COUNTS */

        src_sync_trace_rcv_tags( op_extra_data, sink_grp );

        metadata_tag* last_tag =
                src_sync_metadata_find_last_tag(md->received);

        src_sync_update_rcv_time(
                op_extra_data, last_tag, md, md->rcv_afteridx,
                *remove_octets, sink_grp->common.inv_sample_rate,
                sink_grp->common.idx, !sink_grp->rate_adjust_enable );

        /* Provide a copy of the received tags for timestamped
         * rate measurement
         */
        if (sink_grp->ts_rate_master)
        {
            rate_measure_metadata_record_tags(
                    &(op_extra_data->rm_measure_primary),
                    *remove_octets / OCTETS_PER_SAMPLE,
                    md->rcv_beforeidx, md->rcv_afteridx, md->received);
        }

        return TRUE;
    }
    else
    {
        /* metadata not available (remove_octets == 0) */
        md->rcv_afteridx = 0;
        /* fall through to common return for no received metadata */
        return FALSE;
    }
}

unsigned src_sync_get_sink_metadata( SRC_SYNC_OP_DATA* op_extra_data,
                                     SRC_SYNC_SINK_GROUP* sink_grp,
                                     unsigned remove_octets )
{
    SRC_SYNC_SINK_METADATA* md = &(sink_grp->timestamp_state);

    /* TODO cache this condition ("metadata_connected") */
    if ( sink_grp->common.metadata_enabled
         && (sink_grp->common.metadata_buffer != NULL) )
    {
        if (src_sync_get_input_metadata(
                op_extra_data, sink_grp, md,
                sink_grp->metadata_input_buffer,
                &remove_octets))
        {
            return remove_octets;
        }
        /* fall through to common return for no received metadata */
    }
    else
    {
        /* metadata disabled, or not connected
         * (sink_grp->common.metadata_buffer == NULL):
         * Allow forwarder to substitute void ttp tags
         */
        md->rcv_afteridx = remove_octets;
    }

    md->received = NULL;
    md->rcv_beforeidx = 0;
    return 0;
}


/**
 * Look at metadata of a buffer after a stall, if any, and decide
 * whether it is
 * 1. continuation without gap
 * 2. continuation with a small gap
 * 3. restart
 * 4. don't know (no metadata)
 *
 * At this point, data has not been read, so the tag list is the result
 * of a metadata_peek.
 */
src_sync_stall_recovery_type src_sync_peek_resume(
        SRC_SYNC_OP_DATA* op_extra_data, metadata_tag* tag,
        SRC_SYNC_SINK_METADATA* ts_state, unsigned rcv_beforeidx_octets,
        unsigned available_samples, unsigned sample_rate,
        unsigned* p_gap_samples, unsigned sink_grp_idx )
{
    if (tag != NULL)
    {
        if (METADATA_STREAM_START(tag))
        {
            SOSY_MSG1( SRC_SYNC_TRACE_PEEK_RESUME,
                       "src_g%d stall recovery restart (flagged)",
                       sink_grp_idx );
            return SRC_SYNC_STALL_RECOVERY_RESTART;
        }
        else if (IS_TIMESTAMPED_TAG(tag) || IS_TIME_OF_ARRIVAL_TAG(tag))
        {
            if (ts_state->timestamp_type == RATE_TIMESTAMP_TYPE_NONE)
            {
                SOSY_MSG1( SRC_SYNC_TRACE_PEEK_RESUME,
                           "src_g%d stall recovery restart (tags appeared)",
                           sink_grp_idx );
                return SRC_SYNC_STALL_RECOVERY_RESTART;
            }
            else
            {
                unsigned rcv_beforeidx_samples = rcv_beforeidx_octets
                                                 / OCTETS_PER_SAMPLE;
                int gap_time;
                unsigned gap_samples;

                /* Gap or restart */

                /* Use signed subtraction:
                 * If the result is negative, treat it like timestamps
                 * may actually have jumped backwards, e.g. the new block
                 * comes from a different stream with less target latency,
                 * is a repeat, etc. rather than >36 minutes late.
                 */
                gap_time = time_sub( tag->timestamp,
                                     ts_state->ts_start_of_next_buffer);

                if ((gap_time < 0) || (gap_time >= SECOND))
                {
                    /* Timestamp appeared to go backwards,
                     * or a long time passed.
                     * The limit of 1 second is arbitrary,
                     * mainly serves to limit the size of variables
                     * for the calculation in the next case */
                    SOSY_MSG1( SRC_SYNC_TRACE_PEEK_RESUME,
                               "src_sync src_g%d stall recovery restart "
                               "(excess gap)",
                               sink_grp_idx );
                    return SRC_SYNC_STALL_RECOVERY_RESTART;
                }
                else
                {
                    /* Caller should discard audio for which no timestamp
                     * is known, i.e. up to the start of this buffer.
                     *
                     * Calculate gap up to the tag in samples from time
                     * difference.
                     * 1e6 * sample rate may need up to about 38 bits
                     */
                    unsigned gap_to_start_of_buffer_w;

                    gap_samples = (unsigned)
                            ( ((int48)gap_time * (int)sample_rate * 2 + SECOND)
                              / (2 * SECOND) );

                    /* Go back to the start of this buffer */
                    gap_to_start_of_buffer_w =
                            gap_samples
                            - pl_min(gap_samples, rcv_beforeidx_samples);

                    SOSY_MSG4( SRC_SYNC_TRACE_RCV_GAP,
                               "gap_us %d gap_w %d bef_w %d gtsob_w %d",
                               gap_time, gap_samples, rcv_beforeidx_samples,
                               gap_to_start_of_buffer_w);

                    SOSY_MSG2( SRC_SYNC_TRACE_PEEK_RESUME,
                               "src_g%d stall recovery gap_w %d",
                               sink_grp_idx, gap_to_start_of_buffer_w );

                    /* gap_samples could be 0, but the caller has to
                     * discard samples anyway to make up for inserted silence
                     */
                    *p_gap_samples = gap_to_start_of_buffer_w;

                    return SRC_SYNC_STALL_RECOVERY_GAP;
                }
            }
        }
    }

    /* Non-time tags and no tags are treated similarly */
    if (ts_state->timestamp_type == RATE_TIMESTAMP_TYPE_NONE)
    {
        SOSY_MSG1( SRC_SYNC_TRACE_PEEK_RESUME,
                   "src_g%d stall recovery unknown",
                   sink_grp_idx );
        /* Didn't see timestamps before the stall either */
        return SRC_SYNC_STALL_RECOVERY_UNKNOWN;
    }
    else
    {
        /* Tell the caller to call this function again
         * when the next buffer arrives.
         * The caller should discard the data up to the first tag.
         * This is acceptable since some silence has already
         * been inserted.
         * Hopefully the typical case is that the stall/drop
         * originates upstream of the decoder/TTP generator
         * (e.g. over the air) and the decoder will tag
         * the first data it sends when it recovers.
         */
        SOSY_MSG1( SRC_SYNC_TRACE_PEEK_RESUME,
                   "src_g%d stall recovery wait for tag",
                   sink_grp_idx );
        ts_state->remaining_octets = 0;
        return SRC_SYNC_STALL_RECOVERY_WAITING_FOR_TAG;
    }
}



#endif /* INSTALL_METADATA */
