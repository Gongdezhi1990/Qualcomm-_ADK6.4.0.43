/****************************************************************************
 * Copyright (c) 2015 - 2018 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  splitter_metadata.c
 * \ingroup  capabilities
 *
 *  Splitter's metadata handling related functions.
 *
 */
/****************************************************************************
Include Files
*/
#include "splitter_private.h"
#ifdef INSTALL_METADATA

/**
 * Helper function which creates the metadata buffer.
 */
bool create_internal_metadata(SPLITTER_OP_DATA *splitter, data_buffer_t* internal_buff, unsigned buffer_size)
{
    tCbuffer *metadata_buffer;

    if (splitter->working_mode == CLONE_BUFFER)
    {
        SPLITTER_MSG("Splitter: No need to create internal metadata when cloning!");
        return TRUE;
    }

    /* Make sure that metadata is only created once. */
    SPLITTER_DEBUG_INSTR(PL_ASSERT(splitter->internal_metadata.buffer == NULL));

    /* Reduce the buffer size to samples.*/
    if (splitter->tag_size_to_samples)
    {
        buffer_size = buffer_size>>2;
    }

    /* This buffer is only used for metadata.*/
    metadata_buffer = cbuffer_create(NULL, buffer_size, BUF_DESC_SW_BUFFER);

    if (metadata_buffer == NULL)
    {
        SPLITTER_ERRORMSG("splitter: Unable to allocate metadata buffer for internal data!" );
        return FALSE;
    }

    metadata_buffer->metadata = xzpnew(metadata_list);
    if (metadata_buffer->metadata == NULL)
    {
        SPLITTER_ERRORMSG("splitter: Unable to allocate metadata for internal buffer.!" );
        cbuffer_destroy_struct(metadata_buffer);
        return FALSE;
    }

    /* Only one metadata for the internal buffers. */
    metadata_buffer->metadata->next = metadata_buffer->metadata;
    metadata_buffer->metadata->ref_cnt = 1;

    /* Set the metadata flag!*/
    metadata_buffer->descriptor |= BUF_DESC_METADATA_MASK;

    /* Set the usable octets based on the packing.
     * cbuffer_set_usable_octets will also sets the metadata buffer size
     * based on the cbuffer buffer size! */
    cbuffer_set_usable_octets(metadata_buffer,
            OCTETS_PER_SAMPLE);

    if (splitter->packing == PACKED&&
        ((splitter->data_format == AUDIO_DATA_FORMAT_FIXP_WITH_METADATA) ||
         (splitter->data_format == AUDIO_DATA_FORMAT_FIXP)))
    {
        /* To avoid manipulating the tag length when packing audio
         * just double the metadata buffer size.
         * internal_metadata_buff->metadata->buffer_size is change by
         * cbuffer_set_usable_octets as all internal buffer has
         * the same metadata*/
        metadata_buffer->metadata->buffer_size =
                metadata_buffer->metadata->buffer_size * 2;
    }

    /* Save the created metadata buffer*/
    splitter->internal_metadata.buffer = metadata_buffer;
    return TRUE;
}

/**
 * Helper function which deleted the metadata buffer.
 */
void delete_internal_metadata(SPLITTER_OP_DATA *splitter)
{
    if (splitter->working_mode == CLONE_BUFFER)
    {
        SPLITTER_MSG("Splitter: No internal metadata when cloning!");
        return;
    }
    if (splitter->internal_metadata.buffer == NULL)
    {
        SPLITTER_MSG("Splitter: No internal metadata!");
        return;
    }

    SPLITTER_MSG1("Deleting internal metadata buffer 0x%08x", splitter->internal_metadata.buffer);
    cbuffer_destroy_struct(splitter->internal_metadata.buffer);
    
    splitter->internal_metadata.buffer = NULL;
}


/**
 * Helper function to get the metadata buffers for the input and outputs.
 */
tCbuffer* get_metadata_buffer(SPLITTER_OP_DATA *splitter, bool is_input, unsigned index)
{
    SPLITTER_CHANNEL_STRUC *channel = splitter->channel_list;

    if (channel != NULL)
    {
        if (is_input)
        {
            /* Return the medata input buffer. */
            return channel->input_buffer;
        }
        else
        {
            if (splitter->working_mode != CLONE_BUFFER)
            {
                return channel->output_buffer[index];
            }
            else
            {
                unsigned i;
                /* When splitter is in cloning mode each output has the same
                 * metadata so use the one which is connected. */
                for (i=0; i<SPLITTER_MAX_OUTPUTS_PER_CHANNEL; i++)
                {
                    if (channel->output_buffer[i] != NULL)
                    {
                        return channel->output_buffer[i];
                    }
                }
            }
        }
    }

    /* Couldn't find anything. */
    return NULL;
}

/**
 * As the name suggests this function converts the tag sizes
 * to samples.
 */
static void convert_tags_length_to_samples(metadata_tag *mtag)
{
    while (mtag != NULL)
    {
        SPLITTER_DEBUG_INSTR(PL_ASSERT((mtag->length&(OCTETS_PER_SAMPLE-1)) == 0));
        mtag->length = mtag->length/OCTETS_PER_SAMPLE;
        mtag = mtag->next;
    }
}

/**
 * Just like convert_tags_length_to_samples, but this function transforms
 * the tag sizes to octets.
 */
static void convert_tags_length_to_octets(metadata_tag *mtag)
{
    while (mtag != NULL)
    {
        mtag->length = mtag->length*OCTETS_PER_SAMPLE;
        mtag = mtag->next;
    }
}

/**
 * Transports the metadata form the input to the internal buffer.
 * Very similar to metadata_strict_transport, with the exception that this function sets
 * the new metadata tag head for the channel for tracking reasons.
 */
void splitter_metadata_transport_to_internal(SPLITTER_OP_DATA *splitter,  unsigned trans_octets)
{
#ifdef METADATA_DEBUG_TRANSPORT
    unsigned return_addr = pl_get_return_addr();
#endif /* METADATA_DEBUG_TRANSPORT */
    tCbuffer *src;
    tCbuffer *dst;
    metadata_tag *ret_mtag;
    unsigned b4idx, afteridx;
    unsigned i;

    src = get_metadata_buffer(splitter, TRUE, 0);
    dst = splitter->internal_metadata.buffer;

    if (trans_octets == 0)
    {
        SPLITTER_MSG("splitter_metadata_transport_to_internal: ignoring zero transfer");
        return;
    }

    /* convert the consumed octets.*/
    if (splitter->cbuffer.data_size == 4)
    {
        trans_octets = samples_to_octets(trans_octets);
    }

    if (src != NULL)
    {
        /* transport metadata, first (attempt to) consume tag associated with src */
#ifdef METADATA_DEBUG_TRANSPORT
        ret_mtag = buff_metadata_remove_dbg(src, trans_octets, &b4idx,
                                            &afteridx, return_addr);
#else /* METADATA_DEBUG_TRANSPORT */
        ret_mtag = buff_metadata_remove(src, trans_octets, &b4idx, &afteridx);
#endif /* METADATA_DEBUG_TRANSPORT */
    }
    else
    {
        b4idx = 0;
        afteridx = trans_octets;
        ret_mtag = NULL;
    }
#if defined(SPLITTER_DEBUG)
    if ((ret_mtag != NULL) && (ret_mtag->length !=  0))
    {
        unsigned frame_size = splitter->frame_size;
        if (splitter->cbuffer.data_size == 1)
        {
            frame_size = words_to_octets(frame_size);
        }
        if (ret_mtag->length > frame_size)
        {
            /* for most operator the tag size is the good indication about
             * the frame size. If the splitter frame size is too small
             * during buffering the operator can block. */
            SPLITTER_ERRORMSG1(
                "splitter_metadata_transport_to_internal: "
                    "Splitter frame size of %d could be too small.",
                    splitter->frame_size);
        }
    }
#endif

    if (splitter->reframe_enabled)
    {
        metadata_tag *mtag_ip, *mtag;
        unsigned new_ttp, base_ttp = 0, sample_offset = 0;
        ttp_status status;

        /* Find the first timestamped tag */
        mtag_ip = ret_mtag;
        while (mtag_ip != NULL)
        {
            if (IS_TIMESTAMPED_TAG(mtag_ip))
            {
                break;
            }
            else
            {
                SPLITTER_ERRORMSG(
                    "splitter_metadata_transport_to_internal: Tag not timestamped!"
                        "\n Splitter cannot reframe timestamped tags.");
                b4idx += mtag_ip->length;
                mtag_ip = mtag_ip->next;
            }
        }

        if ((b4idx == 0) && (mtag_ip != NULL))
        {
            /* If the old tag is already at the start of the encoded frame,
             * Just use its timestamp directly
             */
            base_ttp = mtag_ip->timestamp;
            sample_offset = 0;
        }
        else
        {
            /* Otherwise, use the previously-stashed timestamp.
             * There had better be one ! */
            if (splitter->reframe_data.last_tag_samples != LAST_TAG_SAMPLES_INVALID)
            {
                base_ttp = splitter->reframe_data.last_tag_timestamp;
                sample_offset = splitter->reframe_data.last_tag_samples;
            }
        }

        /* Save the tag for future reference when not tag was found. */
        if (mtag_ip != NULL)
        {
            unsigned *err_offset_id;
            unsigned length;
            /* Save the timestamp info from the incoming metadata */
            splitter->reframe_data.last_tag_timestamp = mtag_ip->timestamp;
            splitter->reframe_data.last_tag_spa = mtag_ip->sp_adjust;
            splitter->reframe_data.last_tag_samples = (trans_octets - b4idx) / OCTETS_PER_SAMPLE;
            if (buff_metadata_find_private_data(mtag_ip, META_PRIV_KEY_TTP_OFFSET, &length, (void **)&err_offset_id))
            {
                splitter->reframe_data.last_tag_err_offset_id = (*err_offset_id);
            }
            else
            {
                splitter->reframe_data.last_tag_err_offset_id = INFO_ID_INVALID;
            }
        }
        else
        {
            if (splitter->reframe_data.last_tag_samples != LAST_TAG_SAMPLES_INVALID)
            {
                splitter->reframe_data.last_tag_samples += trans_octets / OCTETS_PER_SAMPLE;
            }
        }

        status.sp_adjustment = splitter->reframe_data.last_tag_spa;
        status.err_offset_id = splitter->reframe_data.last_tag_err_offset_id;
        status.stream_restart = (METADATA_STREAM_START(mtag_ip) != 0);

        /* Create a new tag for the output */
        mtag = buff_metadata_new_tag();

        if (mtag != NULL)
        {
            /* create a tag which covers the newly generated output. */
            mtag->length = trans_octets;

            if (splitter->reframe_data.last_tag_samples != LAST_TAG_SAMPLES_INVALID)
            {
                /* Calculate new TTP from incoming data and sample offset */
                new_ttp = ttp_get_next_timestamp(base_ttp, sample_offset,
                        splitter->reframe_data.sample_rate, splitter->reframe_data.last_tag_spa);
                status.ttp = new_ttp;
                ttp_utils_populate_tag(mtag, &status);
                status.stream_restart = FALSE;
            }
            else
            {
                SPLITTER_ERRORMSG(
                    "splitter_metadata_transport_to_internal: Last tag samples invalid");
            }
        }
        else
        {
            SPLITTER_ERRORMSG(
                "splitter_metadata_transport_to_internal: Failed to allocate tag");
        }

        if (splitter->tag_size_to_samples)
        {
            /* convert the the sizes to samples. */
            convert_tags_length_to_samples(mtag);
            trans_octets = trans_octets>>2;
        }

        buff_metadata_append(dst, mtag, 0, trans_octets);

        /* Free all the incoming tags */
        buff_metadata_tag_list_delete(ret_mtag);

        /* Set the created tag as potential new head. */
        ret_mtag = mtag;
    }
    else
    {
        if (dst != NULL)
        {
            if (splitter->tag_size_to_samples)
            {
                /* convert the the sizes to samples. */
                convert_tags_length_to_samples(ret_mtag);
                b4idx = b4idx>>2;
                afteridx = afteridx>>2;
            }
            /* Even if the src is a NULL buffer we append to dst. It makes no sense
             * for the current configuration. However if another connection is made
             * later to the src which does support metadata the dst metadata write
             * pointer needs to be at the right offset. */
#ifdef METADATA_DEBUG_TRANSPORT
            buff_metadata_append_dbg(dst, ret_mtag, b4idx, afteridx, return_addr);
#else /* METADATA_DEBUG_TRANSPORT */
            buff_metadata_append(dst, ret_mtag, b4idx, afteridx);
#endif /* METADATA_DEBUG_TRANSPORT */
        }
        else
        {
            buff_metadata_tag_list_delete(ret_mtag);
        }
    }

    /* Check if ret_mtag is a new head for any of the output streams. */
    if (ret_mtag != NULL)
    {
        for (i=0; i<SPLITTER_MAX_OUTPUTS_PER_CHANNEL; i++)
        {
            SPLITTER_OUTPUT_STATE output_state;
            output_state = get_current_output_state(splitter, i);
            if ((output_state == ACTIVE) || (output_state == HOLD))
            {
                if (splitter->internal_metadata.head_tag[i] == NULL)
                {

                    SPLITTER_MSG2("Splitter: Setting head for out %d head 0x%08x",
                            i, ret_mtag);
                    splitter->internal_metadata.head_tag[i] = ret_mtag;
                }
            }
        }
    }
}



/**
 * Copies metadata to the destination without removing it from the source.
 */
static metadata_tag *metadata_copy_without_changing_src(tCbuffer *src, tCbuffer *dest, unsigned data_copied_octets, bool sample_to_octets)
{
    unsigned b4idx, afteridx;
    unsigned tag_length;
    metadata_tag *tag_list;
    metadata_tag *new_tag;
    metadata_tag *tail_tag;
    metadata_tag *head_tag;
    unsigned src_data = data_copied_octets;

    if (sample_to_octets)
    {
        /* convert the the sizes to octets. */
        data_copied_octets = data_copied_octets << 2;
    }

    /* Check if we have enough data/space. */
    PL_ASSERT(buff_metadata_available_octets(src) >= src_data);
    PL_ASSERT(buff_metadata_available_space(dest) >= data_copied_octets);

    /* Transport the metadata to the output buffer. */
    tag_list = buff_metadata_peek_ex(src, &b4idx);

    SPLITTER_MSG5("Splitter: buff_metadata_available_space(out) %d data_copied %d octets"
            "\n tag_list 0x%08x  tag_list->next 0x%08x b4idx %d octets " ,
            buff_metadata_available_space(dest), data_copied_octets, tag_list, tag_list->next, b4idx);

    /* Check the before index validity */
    if (tag_list != NULL)
    {
        PL_ASSERT(b4idx < src->metadata->buffer_size);
    }

    /* check if we will consume one tag at all?
     * Note: When tag_list is NULL b4idx is not set by buff_metadata_peek_ex */
    if ((tag_list == NULL) || (src_data < b4idx))
    {
        /* continuing to consume the previous tag. */
        buff_metadata_append(dest, NULL, 0, data_copied_octets);
        return tag_list;
    }

    /* Decrement the data before the first tag. */
    src_data -= b4idx;

    tail_tag = NULL;
    head_tag = NULL;
    afteridx = 0;

    while((tag_list != NULL) && (src_data != 0))
    {

        SPLITTER_MSG2("Splitter tag copy \n tag_list->index 0x%08x \n tag_list->length 0x%08x " ,
                tag_list->index, tag_list->length );

        tag_length = tag_list->length;
        if (src_data <= tag_length)
        {
            afteridx = src_data;
            src_data = 0;
        }
        else
        {
            src_data -= tag_length;
        }
        /* Create a new tag. */
        new_tag = buff_metadata_copy_tag(tag_list);
        /* Make sure the next pointer is NULL. */
        new_tag->next = NULL;
        /* The new tag will be the head if the list is not created yet */
        if (head_tag == NULL)
        {
            head_tag = new_tag;
        }
        /* Add the tag to the end of the tag list. */
        if (tail_tag != NULL)
        {
            tail_tag->next = new_tag;
        }
        tail_tag = new_tag;

        tag_list = tag_list->next;
    }

    if (sample_to_octets)
    {
        /* convert the the sizes to octets. */
        convert_tags_length_to_octets(head_tag);
        b4idx = b4idx<<2;
        afteridx = afteridx<<2;
    }

    buff_metadata_append(dest, head_tag, b4idx, afteridx);
    return tag_list;
}

/**
 * Copies metadata from the internal buffer to the output buffer.
 */
void splitter_metadata_copy(SPLITTER_OP_DATA *splitter, unsigned* data_to_copy, unsigned data_to_remove)
{
    unsigned i, output_prev_rd_indexes, before_prev_rd_indexes;
    metadata_tag *out_head_tag, *before_head_tag;
    tCbuffer *internal;


    /* the internal buffer is the metadata source. */
    internal = get_internal_metadata_buffer(splitter);
    PL_ASSERT(internal != NULL);


    /* Save the index value. */
    before_prev_rd_indexes = internal->metadata->prev_rd_index;
    before_head_tag = internal->metadata->tags.head;

    for (i=0; i<SPLITTER_MAX_OUTPUTS_PER_CHANNEL; i++)
    {
        SPLITTER_OUTPUT_STATE output_state;
        output_state = get_current_output_state(splitter, i);
        if ((data_to_copy[i] != 0) && (output_state == ACTIVE))
        {
            unsigned dst_data;
            /* Set up the internal buffer read pointers!*/
            output_prev_rd_indexes = splitter->internal_metadata.prev_rd_indexes[i];
            internal->metadata->prev_rd_index = output_prev_rd_indexes;

            out_head_tag = splitter->internal_metadata.head_tag[i];
            internal->metadata->tags.head = out_head_tag;

            SPLITTER_MSG4("Splitter: output index %d"
                            "\n read_index before 0x%08x\n head tag 0x%08x\n head index 0x%08x",
                            i, output_prev_rd_indexes, out_head_tag, out_head_tag->index);

            dst_data = data_to_copy[i];

            out_head_tag = metadata_copy_without_changing_src(
                    internal, get_metadata_buffer(splitter, FALSE, i), dst_data, splitter->tag_size_to_samples);

            SPLITTER_MSG1("Splitter will continue from tag 0x%08x",
                    out_head_tag);
            splitter->internal_metadata.head_tag[i] = out_head_tag;

            /* Calculate the read index of the output metadata. */
            output_prev_rd_indexes += dst_data;
            if (output_prev_rd_indexes >= internal->metadata->buffer_size)
            {
                output_prev_rd_indexes -= internal->metadata->buffer_size;
            }
            /* Update the read index for the output.*/
            splitter->internal_metadata.prev_rd_indexes[i] = output_prev_rd_indexes;
        }
    }

    /* restore the index value. */
    internal->metadata->prev_rd_index = before_prev_rd_indexes;
    internal->metadata->tags.head = before_head_tag;

    if (data_to_remove !=0)
    {
        remove_metadata_from_internal(splitter, data_to_remove);
    }

}


/**
 * Helper function to remove unnecessary data from the internal buffer.
 */
void remove_metadata_from_internal(SPLITTER_OP_DATA *splitter, unsigned data_to_remove)
{
    unsigned b4idx, afteridx;
    metadata_tag *ret_mtag;
    tCbuffer* internal_metadata_buffer = get_internal_metadata_buffer(splitter);

    /* remove the associated metadata too. */
    ret_mtag = buff_metadata_remove(
            internal_metadata_buffer,
            data_to_remove, &b4idx, &afteridx);
    buff_metadata_tag_list_delete(ret_mtag);

    /* Update channels on hold */
    if (splitter->hold_streams != OUT_STREAM__0_OFF__1_OFF)
    {
        unsigned i;

        for (i=0; i<SPLITTER_MAX_OUTPUTS_PER_CHANNEL; i++)
        {
            SPLITTER_OUTPUT_STATE output_state;
            output_state = get_current_output_state(splitter, i);
            if (output_state == HOLD)
            {
                /* Set up the internal buffer metadata read indexes!*/
                splitter->internal_metadata.prev_rd_indexes[i] = internal_metadata_buffer->metadata->prev_rd_index;
                splitter->internal_metadata.head_tag[i] = internal_metadata_buffer->metadata->tags.head;
            }
        }
    }
}
#if defined(SPLITTER_DEBUG)
/**
 * Helper function which check for potential tag leaks.
 */
void check_metadata_tags_length(SPLITTER_OP_DATA *splitter, tCbuffer *metadata_buffer)
{
    metadata_tag *tag_list;
    unsigned b4idx, available;

    /* Transport the metadata to the output buffer. */
    tag_list = buff_metadata_peek_ex(metadata_buffer, &b4idx);
    available = buff_metadata_available_octets(metadata_buffer);


    /* Make sure that there is no metadata tag when both of the
     * channels are deactivated. If we have tags that can point
     * to the fact the the buffer was not emptied properly.*/
    if ((get_current_output_state(splitter, 0) == INACTIVE)&&
        (get_current_output_state(splitter, 1) == INACTIVE))
    {
        if ((tag_list != NULL) || (available != 0))
        {
            SPLITTER_MSG("Splitter: Leaking tags? ");
            PL_ASSERT(FALSE);
        }
        /* Nothing else we can check. Exit*/
        return;
    }

    SPLITTER_MSG3("Splitter: check_buffers_validity metadata check"
        "\n available 0x%08x"
        "\n tag_list 0x%08x"
        "\n b4idx 0x%08x",
        available,
        tag_list,
        b4idx);

    if (tag_list)
    {
        PL_ASSERT(b4idx < available);
        available -= b4idx;
        while (tag_list)
        {
            /* make sure this was the last tag.
            SPLITTER_MSG4("Splitter: check_buffers_validity tag"
                    "\n tag_list 0x%08x tag_list->length 0x%08x tag_list->next 0x%08x available 0x%08x",
                    tag_list,
                    tag_list->length,
                    tag_list->next,
                    available);*/
            if (available >= tag_list->length)
            {
                available -= tag_list->length;
            }
            else
            {
                /* make sure this is the last tag. */
                SPLITTER_MSG4("Splitter: check_buffers_validity check if last tag."
                        "\n tag_list 0x%08x"
                        "\n tag_list->length 0x%08x"
                        "\n tag_list->next 0x%08x"
                        "\n available 0x%08x",
                        tag_list,
                        tag_list->length,
                        tag_list->next,
                        available);
                PL_ASSERT(tag_list->next == NULL);

            }
            tag_list = tag_list->next;
        }
    }

}
#endif

#endif /* INSTALL_METADATA */
