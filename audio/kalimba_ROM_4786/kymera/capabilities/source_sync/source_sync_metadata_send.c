/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  source_sync_metadata_send.c
 * \ingroup  capabilities
 *
 *  src_sync operator
 *
 */
/*
 * This file implements low level operations on metadata,
 * some of which access private definitions and might therefore
 * eventually be better placed in buffer_metadata.c
 */

#include "source_sync_defs.h"
#include "patch/patch.h"

/* Enables extra logging of content of written metadata buffers */
/* #define SOSY_CHECK_WRITTEN_BUFFERS */

#if defined(INSTALL_METADATA)

/****************************************************************************
Private Function Declarations
*/

static metadata_tag* src_sync_metadata_get_void_tag(
        SRC_SYNC_OP_DATA* op_extra_data,
        SRC_SYNC_METADATA_DEST* dest_state,
        unsigned length );
static void src_sync_trace_send_tags( SRC_SYNC_OP_DATA* op_extra_data,
                                      SRC_SYNC_TERMINAL_GROUP* dest_grp,
                                      const metadata_tag* tag);

/****************************************************************************
Private Function Definitions
*/

void src_sync_metadata_drop_tags( SRC_SYNC_OP_DATA* op_extra_data,
                                  metadata_tag* tags,
                                  SRC_SYNC_METADATA_DEST* dest_state )
{
    while (tags != NULL)
    {
        metadata_tag* removed_tag = tags;
        tags = tags->next;
        removed_tag->next = NULL;

#ifdef SOSY_VERBOSE
        /* record what was contained in the removed tags */
        if (METADATA_PACKET_START(removed_tag))
        {
            SOSY_MSG( SRC_SYNC_TRACE_RCV_METADATA|SRC_SYNC_TRACE_SEND_METADATA|SRC_SYNC_TRACE_METADATA,
                      "md forward losing packet start");
        }
        if (METADATA_PACKET_END(removed_tag))
        {
            SOSY_MSG( SRC_SYNC_TRACE_RCV_METADATA|SRC_SYNC_TRACE_SEND_METADATA|SRC_SYNC_TRACE_METADATA,
                      "md forward losing packet end");
        }
        if (METADATA_STREAM_START(removed_tag))
        {
            SOSY_MSG( SRC_SYNC_TRACE_RCV_METADATA|SRC_SYNC_TRACE_SEND_METADATA|SRC_SYNC_TRACE_METADATA,
                      "md forward losing stream start");
        }
#endif /* SOSY_VERBOSE */
        if (METADATA_STREAM_END(removed_tag))
        {
            SOSY_MSG( SRC_SYNC_TRACE_EOF, "md_fwd save EOF tag");
            if (dest_state->eof_tag != NULL)
            {
                /* Overrun with EOF tags? */
                L2_DBG_MSG("src_sync md_fwd WARNING overran EOF tag");
#ifdef SOSY_CHECK_METADATA_TAG_COUNTS
                op_extra_data->num_tags_deleted += 1;
#endif /* SOSY_CHECK_METADATA_TAG_COUNTS */
                buff_metadata_delete_tag(dest_state->eof_tag, TRUE);
            }
            removed_tag->flags &= ~ (METADATA_TIMESTAMP_MASK
                                    | METADATA_TIME_OF_ARRIVAL_MASK
                                    | METADATA_PACKET_START_MASK
                                    | METADATA_PACKET_END_MASK
                                    | METADATA_STREAM_START_MASK);
            dest_state->eof_tag = removed_tag;
        }
        else
        {
#ifdef SOSY_CHECK_METADATA_TAG_COUNTS
            op_extra_data->num_tags_deleted += 1;
#endif /* SOSY_CHECK_METADATA_TAG_COUNTS */
            buff_metadata_delete_tag(removed_tag, FALSE);
        }
    }
}

static metadata_tag* src_sync_metadata_get_void_tag( SRC_SYNC_OP_DATA* op_extra_data,
                                SRC_SYNC_METADATA_DEST* dest_state,
                                unsigned length )
{
    metadata_tag* void_tag;

    if (dest_state->eof_tag != NULL)
    {
        SOSY_MSG( SRC_SYNC_TRACE_EOF, "reuse EOF tag");
        void_tag = dest_state->eof_tag;
        dest_state->eof_tag = NULL;
    }
    else
    {
        void_tag = buff_metadata_new_tag();
#ifdef SOSY_CHECK_METADATA_TAG_COUNTS
        if (void_tag != NULL)
        {
            op_extra_data->num_tags_allocated += 1;
        }
#endif /* SOSY_CHECK_METADATA_TAG_COUNTS */
    }
    if (void_tag != NULL)
    {
        void_tag->length = length;
        if (dest_state->provide_ttp)
        {
            METADATA_VOID_TTP_SET(void_tag);
        }
    }
    return void_tag;
}

/****************************************************************************
Module Function Definitions
*/

void src_sync_metadata_forward(
        SRC_SYNC_OP_DATA* op_extra_data, SRC_SYNC_SINK_METADATA* sink,
        unsigned octets_length, SRC_SYNC_TERMINAL_GROUP* dest_common,
        SRC_SYNC_METADATA_DEST* dest_state )
{
    bool append_success = TRUE;
    metadata_tag* tags = sink->received;
    unsigned octets_before = sink->rcv_beforeidx;
    unsigned octets_after = sink->rcv_afteridx;

    patch_fn_shared(src_sync);

    if ( ! dest_common->metadata_enabled )
    {
        /* Does this case get here? */
        /* Would call back on any EOF tags that were received */
#ifdef SOSY_CHECK_METADATA_TAG_COUNTS
        op_extra_data->num_tags_deleted +=
                src_sync_metadata_count_tags(tags);
#endif /* SOSY_CHECK_METADATA_TAG_COUNTS */
        buff_metadata_tag_list_delete(tags);
        return;
    }

    if (dest_state->provide_ttp && (tags != NULL))
    {
        /* In this mode, only forward upstream tags if they are TTP.
         * Produce void TTP for all other types of input metadata.
         */
        if (! IS_TIMESTAMPED_TAG(tags))
        {
            SOSY_MSG( SRC_SYNC_TRACE_SEND_METADATA,
                      "md_fwd drop non-ttp tags");
            src_sync_metadata_drop_tags(op_extra_data, tags, dest_state);
            tags = NULL;
        }
    }

    if (octets_length <= dest_state->remaining_octets)
    {
        /* Includes the case where tags == NULL, either because there is no
         * input metadata or there is no new tag in the input range
         */
        /*
         * before:
         *     |------------------remaining_octets--------->------...-->
         *     |----------octets_length-------------------->|
         *     |                                     |--oa->|
         *     |--octets_before->|--tl0->...--tln-1->|-tln->...  -->|
         *
         * after:
         *                                                 >|-ro'-...-->
         *                                                  |
         *                                                  |
         *                                                 >|-ob'..>|
         *
         *     tlk = length of k'th tag
         *     tln = length of last tag
         *     oa  = octets_after from rcv
         *     ob' = octets_before from rcv (next time)
         *     ro' = remaining_octets (next time)
         *
         *     tln >= oa
         */
        /* Free all received tags, keep any with EOF callbacks */
        src_sync_metadata_drop_tags(op_extra_data, tags, dest_state);

        /* Only increment write pointer */
        append_success = buff_metadata_append(
                dest_common->metadata_buffer, NULL,
                0, octets_length);

        dest_state->remaining_octets -= octets_length;

        SOSY_MSG3( SRC_SYNC_TRACE_SEND_METADATA,
                   "md_fwd src_g%d adv %d octets still tagged, "
                   "remaining %d (only advance wptr)",
                   dest_common->idx, octets_length,
                   dest_state->remaining_octets);
    }
    else if (tags == NULL)
    {
        /* There is an untagged portion at the end of the buffer, and no
         * metadata input. Not silence but the same metadata actions.
         */
        src_sync_metadata_silence( op_extra_data, dest_common,
                                   dest_state, octets_length );
    }
    else
    {
        if (dest_state->remaining_octets < octets_before)
        {
            /* The previously tagged range ends before the first tag to forward.
             * Make the first tag fit. If it has a TTP or ToA timestamp,
             * extrapolate this time value backwards.
             */

            unsigned predate_octets = octets_before
                                      - dest_state->remaining_octets;
            unsigned predate_samples = predate_octets / OCTETS_PER_SAMPLE;

            SOSY_MSG4( SRC_SYNC_TRACE_FWD_SPLICE,
                       "md_fwd src_g%d rem_o %d < oct_bf %d "
                       "predate_o %d",
                       dest_common->idx, dest_state->remaining_octets,
                       octets_before, predate_octets);

            tags->length += predate_octets;
            /* If there is only one tag, octets_after has to be extended */
            if (tags->next == NULL)
            {
                SOSY_MSG4( SRC_SYNC_TRACE_FWD_SPLICE,
                           "md_fwd src_g%d predate_o %d "
                           "extedn oct_aft %d -> %d",
                           dest_common->idx, predate_octets,
                           octets_after, octets_after+predate_octets);
                octets_after += predate_octets;
            }
            if (IS_TIMESTAMPED_TAG(tags) || IS_TIME_OF_ARRIVAL_TAG(tags))
            {
                tags->timestamp = src_sync_get_prev_timestamp(
                        tags, predate_samples,
                        dest_common->inv_sample_rate);
            }
        }
        else
        {
            /* the previously tagged range ends within one of the tags.
             * Shorten that tag and discard the ones before it. */
            while ( (tags != NULL)
                    && ( dest_state->remaining_octets
                         >= (octets_before + tags->length) ) )
            {
                metadata_tag* removed_tag = tags;
                tags = removed_tag->next;
                removed_tag->next = NULL;

                SOSY_MSG4( SRC_SYNC_TRACE_FWD_SPLICE,
                           "md_fwd src_g%d skip rx tag "
                           "rm_o %d oct_bf(pre) %d len %d",
                           dest_common->idx, dest_state->remaining_octets,
                           octets_before, removed_tag->length);

                octets_before += removed_tag->length;

                src_sync_metadata_drop_tags( op_extra_data, removed_tag,
                                             dest_state);
            }

            /* Fixup opportunity since the following assert has been observed */
            patch_fn_shared(src_sync);

            PL_ASSERT( (tags != NULL)
                       && ( dest_state->remaining_octets
                            >= octets_before )
                       && ( dest_state->remaining_octets
                            < (octets_before + tags->length) ) );

            unsigned postdate_octets = dest_state->remaining_octets
                                       - octets_before;
            if (postdate_octets > 0)
            {
                unsigned postdate_samples = postdate_octets
                                            / OCTETS_PER_SAMPLE;

                SOSY_MSG4( SRC_SYNC_TRACE_FWD_SPLICE,
                           "md_fwd src_g%d rem_o %d > oct_bf %d "
                           "postdate_o %d",
                           dest_common->idx, dest_state->remaining_octets,
                           octets_before, postdate_octets);

                /* Shorten first tag */
                tags->length -= postdate_octets;
                /* If there is only one tag, octets_after has to be shortened */
                if (tags->next == NULL)
                {
                    SOSY_MSG4( SRC_SYNC_TRACE_FWD_SPLICE,
                               "md_fwd src_g%d postdate_o %d "
                               "shorten oct_aft %d -> %d",
                               dest_common->idx, postdate_octets,
                               octets_after, octets_after-postdate_octets);
                    PL_ASSERT(octets_after >= postdate_octets);
                    octets_after -= postdate_octets;
                }

                if (IS_TIMESTAMPED_TAG(tags) || IS_TIME_OF_ARRIVAL_TAG(tags))
                {
                    tags->timestamp = src_sync_get_next_timestamp(
                            tags->timestamp, postdate_samples,
                            dest_common->inv_sample_rate,
                            tags->sp_adjust);
                }
            }
        }

        metadata_tag* last_tag = src_sync_metadata_find_last_tag(tags);
        if (last_tag->length < octets_after)
        {
            SOSY_MSG3( SRC_SYNC_TRACE_FWD_SPLICE,
                       "md_fwd src_g%d last_tag->length %d < oct_af %d "
                       "force extend tag",
                       dest_common->idx, last_tag->length, octets_after);
            last_tag->length = octets_after;
        }
        /* PL_ASSERT(last_tag->length >= octets_after); */
        unsigned next_remaining_octets = last_tag->length - octets_after;

        if (dest_state->eof_tag != NULL)
        {
            /* The stream end flag is set in dest_state->eof_tag. */
#ifdef SOSY_VERBOSE
            bool move_success =
#endif /* SOSY_VERBOSE */
                    src_sync_metadata_move_eof(last_tag, dest_state->eof_tag);
            SOSY_MSG1( SRC_SYNC_TRACE_EOF,
                       "moved EOF info success %d", move_success);

#ifdef SOSY_CHECK_METADATA_TAG_COUNTS
            op_extra_data->num_tags_deleted += 1;
#endif /* SOSY_CHECK_METADATA_TAG_COUNTS */
            buff_metadata_delete_tag(dest_state->eof_tag, TRUE);
            dest_state->eof_tag = NULL;
        }

        src_sync_trace_send_tags(op_extra_data, dest_common, tags);

#ifdef SOSY_CHECK_METADATA_TAG_COUNTS
        op_extra_data->num_tags_sent += src_sync_metadata_count_tags(tags);
#endif /* SOSY_CHECK_METADATA_TAG_COUNTS */
        append_success = buff_metadata_append(
                dest_common->metadata_buffer, tags,
                dest_state->remaining_octets, octets_after);

        SOSY_MSG4( SRC_SYNC_TRACE_SEND_METADATA,
                   "send md src_g%d oct %d bf_o %d af_o %d",
                   dest_common->idx, octets_length,
                   dest_state->remaining_octets, octets_after);

        dest_state->remaining_octets = next_remaining_octets;

    }
    if (! append_success)
    {
        L2_DBG_MSG("src_sync md_fwd: WARNING failed to append");
    }
}

void src_sync_metadata_silence( SRC_SYNC_OP_DATA* op_extra_data,
                                SRC_SYNC_TERMINAL_GROUP* dest_common,
                                SRC_SYNC_METADATA_DEST* dest_state,
                                unsigned silence_octets )
{
    if ( ! dest_common->metadata_enabled )
    {
        /* Does this case get here? */
    }
    else if (silence_octets <= dest_state->remaining_octets)
    {
        buff_metadata_append( dest_common->metadata_buffer, NULL,
                              0, silence_octets );

        SOSY_MSG2( SRC_SYNC_TRACE_SEND_METADATA,
                   "send md src_g%d oct %d (adv wptr instead of void)",
                   dest_common->idx, silence_octets);

        dest_state->remaining_octets -= silence_octets;
    }
    else
    {
        unsigned void_tag_octets =
                silence_octets - dest_state->remaining_octets;
        metadata_tag* void_tag =
                src_sync_metadata_get_void_tag(
                        op_extra_data, dest_state, void_tag_octets );

        src_sync_trace_send_tags(op_extra_data, dest_common, void_tag);

#ifdef SOSY_CHECK_WRITTEN_BUFFERS
        unsigned md_wp_before = buff_metadata_get_write_offset(
                                    dest_common->metadata_buffer);
#endif /* SOSY_CHECK_WRITTEN_BUFFERS */

#ifdef SOSY_CHECK_METADATA_TAG_COUNTS
        if (void_tag != NULL)
        {
            op_extra_data->num_tags_sent += 1;
        }
#endif /* SOSY_CHECK_METADATA_TAG_COUNTS */
        buff_metadata_append( dest_common->metadata_buffer, void_tag,
                              dest_state->remaining_octets, void_tag_octets );

        SOSY_MSG4( SRC_SYNC_TRACE_SEND_METADATA,
                   "send md src_g%d oct %d bf_o %d af_o %d (1 void tag)",
                   dest_common->idx, silence_octets,
                   dest_state->remaining_octets, void_tag_octets);

#ifdef SOSY_CHECK_WRITTEN_BUFFERS
        {
            unsigned md_wp_after = buff_metadata_get_write_offset(
                                      dest_common->metadata_buffer);
            metadata_tag* peeked =
                    buff_metadata_peek( dest_common->metadata_buffer);
            metadata_tag* t;
            SOSY_MSG2( SRC_SYNC_TRACE_SEND_METADATA,
                       "... md_wp before %d after %d",
                       md_wp_before, md_wp_after);
            for (t = peeked; t != NULL; t = t->next)
            {
                SOSY_MSG2( SRC_SYNC_TRACE_SEND_METADATA,
                         "... tag idx %d len %d", t->index, t->length);
            }
        }
#endif /* SOSY_CHECK_WRITTEN_BUFFERS */

        dest_state->remaining_octets = 0;
    }
}

static void src_sync_trace_send_tags( SRC_SYNC_OP_DATA* op_extra_data,
                                      SRC_SYNC_TERMINAL_GROUP* dest_grp,
                                      const metadata_tag* tag)
{
#ifdef SOSY_VERBOSE
    TIME now = hal_get_time();

    if (tag != NULL)
    {
        SOSY_MSG2( SRC_SYNC_TRACE_SEND_METADATA,
                   "pt src_g%d send %d tags",
                   dest_grp->idx, src_sync_metadata_count_tags(tag));

        while (tag != NULL)
        {
            if (IS_TIMESTAMPED_TAG(tag))
            {
                SOSY_MSG4(  SRC_SYNC_TRACE_SEND_METADATA,
                            "pt src_g%d send ttp flags 0x%02x t 0.%04d fut oct %d",
                            dest_grp->idx,
                            tag->flags,
                            time_sub(tag->timestamp, now)/100,
                            tag->length);
            }
            else if (IS_TIME_OF_ARRIVAL_TAG(tag))
            {
                SOSY_MSG4(  SRC_SYNC_TRACE_SEND_METADATA,
                            "pt src_g%d send toa flags 0x%02x t 0.%04d past oct %d",
                            dest_grp->idx,
                            tag->flags,
                            time_sub(now, tag->timestamp)/100,
                            tag->length);
            }
            else if (IS_VOID_TTP_TAG(tag))
            {
                SOSY_MSG3(  SRC_SYNC_TRACE_SEND_METADATA,
                            "pt src_g%d send void flags 0x%02x oct %d",
                            dest_grp->idx,
                            tag->flags,
                            tag->length);
            }
            else
            {
                SOSY_MSG3(  SRC_SYNC_TRACE_SEND_METADATA,
                            "pt src_g%d send other tag flags 0x%02x oct %d",
                            dest_grp->idx, tag->flags, tag->length);
            }
            tag = tag->next;
        }
    }
    else
    {
        SOSY_MSG1( SRC_SYNC_TRACE_SEND_METADATA,
                   "pt src_g%d send no tags",
                   dest_grp->idx);
    }
#endif /* SOSY_VERBOSE */
}

#endif /* INSTALL_METADATA */
