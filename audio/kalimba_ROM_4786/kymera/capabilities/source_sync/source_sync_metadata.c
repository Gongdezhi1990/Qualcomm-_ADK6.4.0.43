/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  source_sync_metadata.c
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

#if defined(INSTALL_METADATA)

/* Local function declarations */
#ifdef SOSY_CHECK_METADATA_TAG_COUNTS
static unsigned src_sync_count_cached_tags(SRC_SYNC_OP_DATA* op_extra_data);
#endif /* SOSY_CHECK_METADATA_TAG_COUNTS */

/**
 * Returns the number of tags in a linked list of metadata tags.
 */
unsigned src_sync_metadata_count_tags(const metadata_tag* tags)
{
    unsigned n;
    for (n = 0 ; tags != NULL; tags = tags->next)
    {
        n += 1;
    }
    return n;
}

/**
 * If src_tag contains EOF callback private data,
 * move this data to dest_tag.
 */

bool src_sync_metadata_move_eof(
        metadata_tag *dest_tag,
        metadata_tag *src_tag)
{
    void* data;
    unsigned data_length;

    if ( (dest_tag == NULL)
         || (src_tag == NULL)
         || ! METADATA_STREAM_END(src_tag)
         || METADATA_STREAM_END(dest_tag))
    {
        return FALSE;
    }

    /* If dest_tag contains EOF callback private data
     * despite having its stream end flag not set,
     * buff_metadata_add_private_data would append the new
     * callback data and buff_metadata_find_private_data
     * will subsequently find the old one, not the new one.
     * Catch this case. The caller should then delete src_tag
     * with EOF handling.
     */
    if (buff_metadata_find_private_data(dest_tag, META_PRIV_KEY_EOF_CALLBACK,
                                        &data_length, &data))
    {
        return FALSE;
    }

    if (buff_metadata_find_private_data(src_tag, META_PRIV_KEY_EOF_CALLBACK,
                                        &data_length, &data))
    {
        buff_metadata_add_private_data(dest_tag, META_PRIV_KEY_EOF_CALLBACK,
                                       data_length, data);
        METADATA_STREAM_END_SET(dest_tag);

        /* According to the current implementation of buff_metadata_delete_tag,
         * clearing the stream end bit will prevent EOF callback handling
         * of the src_tag even if it still contains the EOF private data.
         */
        METADATA_STREAM_END_UNSET(src_tag);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/**
 * Returns the last tag, if any, of a linked list of metadata tags
 */
metadata_tag* src_sync_metadata_find_last_tag(metadata_tag* tag)
{
    if (tag != NULL)
    {
        while (tag->next != NULL)
        {
            tag = tag->next;
        }
    }
    return tag;
}

/* Essentially a copy of ttp_get_next_timestamp,
 * but I'm keeping control of rounding
 */
unsigned src_sync_get_next_timestamp( TIME timestamp, unsigned nr_of_samples,
                                      unsigned inv_sample_rate, int sp_adjust)
{
    unsigned advance = src_sync_samples_to_usec(nr_of_samples, inv_sample_rate);

    /* If sp_adjust is negative, this addition will wrap around. */
    advance = advance + frac_mult(advance, sp_adjust);

    return time_add(timestamp, advance);
}

/**
 * \param tag Metadata tag from whose timestamp value the result is derived
 * \param nr_of_samples Number of samples by which to extrapolate the
 *                      timestamp backwards from the tag's start.
 * \param sample_rate Sample rate in Hz
 * \note Derived from ttp_get_next_timestamp.
 *       nr_of_samples is assumed to be small enough that the time in
 *       microseconds resulting in the local variable reverse
 *       never wraps. The value passed is at most the number of words
 *       in a buffer.
 */
unsigned src_sync_get_prev_timestamp( const metadata_tag* tag,
                                      unsigned nr_of_samples,
                                      unsigned inv_sample_rate)
{
    unsigned reverse = src_sync_samples_to_usec(nr_of_samples, inv_sample_rate);

    /* If sp_adjust is negative, this addition will wrap around. */
    reverse = reverse + frac_mult(reverse, tag->sp_adjust);

    /* This subtraction may wrap when tag->timestamp has recently wrapped. */
    return tag->timestamp - reverse;
}

#ifdef SOSY_CHECK_METADATA_TRANSPORT_POINTERS
void src_sync_check_md_transport_pre(SRC_SYNC_OP_DATA* op_extra_data)
{
    SRC_SYNC_SOURCE_GROUP* src_grp;

    for ( src_grp = op_extra_data->source_groups;
          src_grp != NULL;
          src_grp = next_source_group(src_grp))
    {
        if ( src_grp->common.metadata_enabled
             &&(src_grp->common.metadata_buffer != NULL))
        {
            SRC_SYNC_SOURCE_ENTRY* src_ptr;
            unsigned md_wp =
                    buff_metadata_get_write_offset(
                            src_grp->common.metadata_buffer);

            /* Paranoid check */
            if (md_wp != src_grp->last_md_write_idx)
            {
                L2_DBG_MSG4("src_sync 0x%04x src_g%d md_wp changed since "
                            "last pt: %d -> %d",
                            op_extra_data->id, src_grp->common.idx,
                            src_grp->last_md_write_idx, md_wp);
            }
            src_grp->last_md_write_idx = md_wp;

            for ( src_ptr = source_entries_from_group(src_grp);
                  src_ptr != NULL;
                  src_ptr = next_source_entry(src_ptr) )
            {
                if (src_ptr->common.buffer != NULL)
                {
                    unsigned cb_wp = src_ptr->common.buffer->write_ptr
                                     - src_ptr->common.buffer->base_addr;

                    /* Paranoid check */
                    if (cb_wp != src_ptr->last_cb_write_idx)
                    {
                        L2_DBG_MSG4("src_sync 0x%04x src_%d cb_wp changed "
                                    "since last pt: %d -> %d",
                                    op_extra_data->id, src_ptr->common.idx,
                                    src_ptr->last_cb_write_idx, cb_wp);
                    }
                    src_ptr->last_cb_write_idx = cb_wp;
                }
            }
        }
    }
}

void src_sync_check_md_transport_post(SRC_SYNC_OP_DATA* op_extra_data)
{
    SRC_SYNC_SOURCE_GROUP* src_grp;

    for ( src_grp = op_extra_data->source_groups;
          src_grp != NULL;
          src_grp = next_source_group(src_grp))
    {
        if ( src_grp->common.metadata_enabled
             &&(src_grp->common.metadata_buffer != NULL))
        {
            unsigned md_wp = buff_metadata_get_write_offset(
                src_grp->common.metadata_buffer);
            unsigned md_sz = buff_metadata_get_buffer_size(
                src_grp->common.metadata_buffer);
            SRC_SYNC_SOURCE_ENTRY* src_ptr;

            /* Calculate number of octets written to the metadata buffer */
            unsigned md_wr_o = md_wp + md_sz - src_grp->last_md_write_idx;
            if (md_wr_o >= md_sz)
            {
                md_wr_o -= md_sz;
            }
            unsigned md_wr_w = md_wr_o / OCTETS_PER_SAMPLE;
            if (md_wr_o != (md_wr_w * OCTETS_PER_SAMPLE))
            {
                L2_DBG_MSG3("src_sync 0x%04x src_g%d md "
                            "wrote odd num octets %d",
                            op_extra_data->id, src_grp->common.idx,
                            md_wr_o);
            }
            src_grp->last_md_write_idx = md_wp;

            for ( src_ptr = source_entries_from_group(src_grp);
                  src_ptr != NULL;
                  src_ptr = next_source_entry(src_ptr) )
            {
                if (src_ptr->common.buffer != NULL)
                {
                    unsigned cb_wp = src_ptr->common.buffer->write_ptr
                                     - src_ptr->common.buffer->base_addr;
                    unsigned cb_sz = cbuffer_get_size_in_words(
                            src_ptr->common.buffer);

                    if (cb_wp * OCTETS_PER_SAMPLE != md_wp)
                    {
                        L2_DBG_MSG5("src_sync 0x%04x src_g%d md wp_o %d "
                                    "!= src_%d cb wp_w %d",
                                    op_extra_data->id, src_grp->common.idx,
                                    md_wp, src_ptr->common.idx, cb_wp);
                    }

                    /* Check number of words written to the cbuffer */
                    unsigned cb_wr_w = cb_wp + cb_sz - src_ptr->last_cb_write_idx;
                    if (cb_wr_w >= cb_sz)
                    {
                        cb_wr_w -= cb_sz;
                    }
                    if (cb_wr_w != md_wr_w)
                    {
                        L2_DBG_MSG5("src_sync 0x%04x src_g%d md_wr_w %d "
                                    "!= src_%d cb_wr_w %d",
                                    op_extra_data->id, src_grp->common.idx,
                                    md_wr_w, src_ptr->common.idx, cb_wr_w);
                    }
                    src_ptr->last_cb_write_idx = cb_wp;
                }
            }
        }
    }
}
#endif /* SOSY_CHECK_METADATA_TRANSPORT_POINTERS */

#ifdef SOSY_CHECK_METADATA_TAG_COUNTS
static unsigned src_sync_count_cached_tags(SRC_SYNC_OP_DATA* op_extra_data)
{
    unsigned count = 0;
    SRC_SYNC_SOURCE_GROUP* src_grp;

    for ( src_grp = op_extra_data->source_groups;
          src_grp != NULL;
          src_grp = next_source_group(src_grp))
    {
        if (src_grp->metadata_dest.eof_tag != NULL)
        {
            count += 1;
        }
    }
    return count;
}

void src_sync_clear_tag_counts(SRC_SYNC_OP_DATA* op_extra_data)
{
    op_extra_data->num_tags_received = 0;
    op_extra_data->num_tags_allocated = 0;
    op_extra_data->num_tags_deleted = 0;
    op_extra_data->num_tags_sent = 0;
    op_extra_data->num_tags_cached =
            src_sync_count_cached_tags(op_extra_data);
}

void src_sync_check_tag_counts(SRC_SYNC_OP_DATA* op_extra_data)
{
    unsigned num_cached_tags = src_sync_count_cached_tags(op_extra_data);
    unsigned num_in = op_extra_data->num_tags_cached
                        + op_extra_data->num_tags_received
                        + op_extra_data->num_tags_allocated;
    unsigned num_out = op_extra_data->num_tags_deleted
                       + op_extra_data->num_tags_sent
                       + num_cached_tags;
    if (num_in != num_out)
    {
        L2_DBG_MSG5("src_sync 0x%04x tag count mismatch: pre cached %d rcvd %d alloc %d tot %d",
                    op_extra_data->id, op_extra_data->num_tags_cached,
                    op_extra_data->num_tags_received,
                    op_extra_data->num_tags_allocated, num_in);
        L2_DBG_MSG5("src_sync 0x%04x ... post cached %d deleted %d sent %d tot %d",
                    op_extra_data->id, num_cached_tags,
                    op_extra_data->num_tags_deleted,
                    op_extra_data->num_tags_sent, num_out);
    }
    PL_ASSERT(num_in == num_out);
}
#endif /* SOSY_CHECK_METADATA_TAG_COUNTS */


#endif /* INSTALL_METADATA */
