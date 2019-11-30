/**
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
 * 
 * \file  timestamp_reframe.c
 *
 * \ingroup ttp
 *
 * Timestamp reframing implementation
 *
 */

/****************************************************************************
Include Files
*/

#include "ttp_private.h"

/****************************************************************************
Private Type Declarations
*/

/****************************************************************************
Private Constant Declarations
*/

/**
 * The maximum copy amount.
 */
#define MAX_COPY (0xffff)

/****************************************************************************
Private Macro Declarations
*/

/**
 * Increments the buffer index with a given length. Take in account any buffer wraps.
 */
#define BUF_IDX_INC(IDX, INC, SIZE)   ( ((IDX) + (INC)) % (SIZE) )

/****************************************************************************
Public Variable Definitions
*/

/****************************************************************************
Private Function Declarations
*/

/****************************************************************************
Private Function Definitions
*/

/**
 * \brief Function checks if the tag with the given index is between the read and write
 *        index. (This is useful for checking if the data in tha buffer contains the tag)
 *
 * \param rd_index The read index.
 * \param wr_index The write index.
 * \param tag_index The tag index.
 *
 * \return True, if the tag is on the available data, false otherwise.
 */
static bool is_tag_present(unsigned rd_index, unsigned wr_index,
        unsigned tag_index)
{
    if (rd_index == wr_index)
    {
        /* The buffer is empty. */
        return FALSE;
    }
    else if (rd_index < wr_index)
    {
        /* The write pointer is not wrapped. (It is after the read index.) */
        if ((tag_index >= rd_index) && (tag_index < wr_index))
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else/* (prev_rd_idx > prev_wr_index) */
    {
        /* The write pointer is wrapped. (It is before the read index.) */
        if ((tag_index >= rd_index) || (tag_index < wr_index))
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
}

/**
 * Search for the error offset id of the tag.
 *
 * \param tag Metadata tag.
 *
 * \return error offset id of the tag. INFO_ID_INVALID if the tag has no error offset.
 */
static int get_err_offset_id(metadata_tag *tag)
{
    unsigned *err_offset_id;
    unsigned out_length;
    /* use the last tag to calculate the the next tag timestamp. */
    if (buff_metadata_find_private_data(tag, META_PRIV_KEY_TTP_OFFSET, &out_length, (void **)&err_offset_id))
    {
        return  *err_offset_id;
    }
    else
    {
        return INFO_ID_INVALID;
    }
}

/**
 * Function which returns the available octets between index1 and index2.
 *
 * \param index1 Index 1.
 * \param index2 Index 2 must be after Index 1 or the calculation will be wrong.
 * \param buffer_size The size of the buffer
 *
 * \return available octets between index1 and index2.
 */
static unsigned octets_between_indexes(unsigned index1, unsigned index2, unsigned  buffer_size)
{
    unsigned octets;

    /* Limit the tag length. */
    octets =  index2 - index1;
    if (octets >= buffer_size)
    {
        octets = octets + buffer_size;
    }
    return octets;
}

/****************************************************************************
Public Function Definitions for the timed playback module reframing.
*/

/*
 * reframe_init
 */
void reframe_init(REFRAME *reframe, tCbuffer *metadata_buff,
        unsigned reframe_period, unsigned sample_rate)
{
    patch_fn_shared(timestamp_reframe);
    /* Sanity check.*/
    PL_ASSERT(reframe != NULL);
    PL_ASSERT(metadata_buff != NULL);
    PL_ASSERT(metadata_buff->metadata != NULL);

    reframe->metadata_buff = metadata_buff;
    reframe->sample_rate = sample_rate;
    reframe->desired_tag_length = reframe_period * OCTETS_PER_SAMPLE;

    reframe->tag.length = reframe_period * OCTETS_PER_SAMPLE;
    reframe->tag.index = 0;
    /* The rest of the tag values are invalid until a tag is read from the input. */
    reframe->tag.timestamp = 0xdead;
    reframe->tag.sp_adjust = 0;
    reframe->tag.err_offset_id = INFO_ID_INVALID;
    reframe->tag.is_void = FALSE;

    reframe->prev_tag.octets_consumed = 0;
    reframe->prev_tag.length = 0;

    reframe->state = REFRAME_STATE_STARTUP;
    reframe->tag_read_index = 0;
}

/*
 * reframe_check_tags
 */
void reframe_check_tags(REFRAME *reframe)
{
    metadata_tag *tag = buff_metadata_get_head(reframe->metadata_buff);
    metadata_tag *last_timestamped_tag = NULL;
    unsigned prev_rd_index;
    unsigned prev_wr_index;
    unsigned buff_size;


    if (tag == NULL)
    {
        /* Nothing to do here. */
        return;
    }

    prev_rd_index = buff_metadata_get_read_offset(reframe->metadata_buff);
    prev_wr_index = buff_metadata_get_write_offset(reframe->metadata_buff);
    buff_size = buff_metadata_get_buffer_size(reframe->metadata_buff);

    /* First, check if the tag validity. If this failure is hit the metadata is out of
     * sync with the buffer and is nothing much we can do. */
    PL_ASSERT(is_tag_present(prev_rd_index, BUF_IDX_INC(prev_wr_index, 1, buff_size), tag->index));

    while(tag)
    {
        bool is_void_tag = IS_VOID_TTP_TAG(tag);
        bool before_tag;

        if (!is_void_tag && !IS_TIMESTAMPED_TAG(tag))
        {
            /* Only check timestamped and void tags. */
            tag = tag->next;
            continue;
        }

        if (!is_tag_present(reframe->tag_read_index, prev_wr_index, tag->index))
        {
            /* This stag was already checked. */
            tag = tag->next;
            continue;
        }

        /* A valid tag was found, exit form startup mode. */
        if (reframe->state == REFRAME_STATE_STARTUP)
        {
            reframe->state = REFRAME_STATE_TIMESTAMPED_TAGS;
        }

        /* Check if the current tag is until the next generated tag.
         * (including the tag index therefore the addition of 1) */
        before_tag = ((reframe->prev_tag.length - reframe->prev_tag.octets_consumed < buff_size) &&
                is_tag_present(prev_rd_index, BUF_IDX_INC(reframe->tag.index, 1, buff_size), tag->index))\
                                || (reframe->prev_tag.length - reframe->prev_tag.octets_consumed >= buff_size);

        /* Search for changes in the state. Note: "!=" is used as a boolean XOR to
         * detect changes. */
        if (reframe->tag.is_void != is_void_tag)
        {
            if (before_tag)
            {
                if (is_void_tag)
                {
                    TTP_DBG_MSG2("reframe: Transition from timestamped to void tags. "\
                            "tag->index = 0x%08x, tag->length = 0x%08x", tag->index, tag->length);
                    reframe->tag.is_void = TRUE;
                    reframe->tag.index = tag->index;
                    reframe->tag.length = tag->length;

                    /* The previously generated tag would not be finished. Set the length
                     * to the octets until to void tag.  */
                    reframe->prev_tag.length = octets_between_indexes(
                            prev_rd_index, tag->index, buff_size);
                    reframe->prev_tag.octets_consumed = 0;

                    reframe->state = REFRAME_STATE_VOID_TAGS;
                }
                else
                {
                    TTP_DBG_MSG1("reframe: Transition from void to timestamped tags. "\
                            "tag->index = 0x%08x", tag->index);
                    reframe->tag.is_void = FALSE;
                    reframe->tag.timestamp = tag->timestamp;
                    reframe->tag.length = reframe->desired_tag_length;
                    reframe->tag.err_offset_id = get_err_offset_id(tag);
                    reframe->tag.sp_adjust = tag->sp_adjust;

                    /* Check if the index is correct */
                    if (reframe->tag.index != tag->index)
                    {
                        TTP_WARN_MSG2("reframe:Transition from void to timestamped tags gives index mismatch "\
                                " reframe->tag.index = 0x%08x,  tag->index = 0x%08x!", reframe->tag.index, tag->index);
                        reframe->tag.index = tag->index;
                    }

                    reframe->state = REFRAME_STATE_TIMESTAMPED_TAGS;
                }

                reframe->tag_read_index = BUF_IDX_INC(tag->index, 1, buff_size);
            }
            else
            {
                if (is_void_tag)
                {
                    /* The tag is after the next index. Limit the tag size. */
                    unsigned octets_to_void_tag;
                    octets_to_void_tag = octets_between_indexes(
                            reframe->tag.index, tag->index, buff_size);

                    if (octets_to_void_tag < reframe->tag.length)
                    {
                        TTP_DBG_MSG1("reframe: limit  timestamped tag size to. "\
                                "octets_to_void_tag = 0x%08x", octets_to_void_tag);
                        reframe->tag.length = octets_to_void_tag;
                    }
                }
            }

            /* exit after the first changes. */
            break;
        }
        else
        {
            if (!is_void_tag && before_tag)
            {
                /* No changes in the state and the next tag is not void.
                 * Save the last timestamped tag which may will be used to generate
                 * the new timestamp. Depending on wether we find another timestamped tag
                 * or a void tag. */
                last_timestamped_tag = tag;

                /* To avoid recalculating the same timestamp again increment the index
                 * with one. NOTE: Only incrementing with one because the tag length can
                 * be bigger than the buffer. */
                reframe->tag_read_index = BUF_IDX_INC(tag->index, 1, buff_size);

            }
            if (is_void_tag)
            {
                /* we are at the beginning of a void tag. Set the generated tag length.*/
                if (prev_rd_index == tag->index)
                {
                    reframe->tag.length = tag->length;
                }
                /* Only check one tag when in void state.*/
                break;
            }

        }

        /* Move to the next tag. */
        tag = tag->next;
    }

    if ((last_timestamped_tag != NULL)&&(!reframe->tag.is_void))
    {
        /* Update the timestamp of the tag.*/
        unsigned *err_offset_id;
        unsigned out_length;
        unsigned samples_to_tag;
        unsigned octets_to_tag;

        /* Calculate the distance between the current tag and the next generated tag. */
        octets_to_tag = octets_between_indexes(
                prev_rd_index, last_timestamped_tag->index, buff_size);
        octets_to_tag += reframe->prev_tag.octets_consumed;
        /* Panic if we consumed more than the tag length. reframe->prev_tag.length should
         * not be zero because only tags before are checked during the start.  */
        PL_ASSERT(reframe->prev_tag.length >= octets_to_tag);
        octets_to_tag = reframe->prev_tag.length - octets_to_tag;


        TTP_DBG_MSG4("reframe: last tag_index = 0x%08x, tag_length = 0x%08x, tag_timestamp = 0x%08x, reframe->tag.index = 0x%08x ",
                last_timestamped_tag->index, last_timestamped_tag->length, last_timestamped_tag->timestamp, reframe->tag.index);

        /* use the last tag to calculate the the next tag timestamp. */
        if (buff_metadata_find_private_data(last_timestamped_tag, META_PRIV_KEY_TTP_OFFSET, &out_length, (void **)&err_offset_id))
        {
            reframe->tag.err_offset_id = *err_offset_id;
        }
        else
        {
            reframe->tag.err_offset_id = INFO_ID_INVALID;
        }

        /* Sample rate adjustment. */
        reframe->tag.sp_adjust = last_timestamped_tag->sp_adjust;

        /* Calculate the new timestamp. */
        samples_to_tag = octets_to_tag / OCTETS_PER_SAMPLE;

        TTP_DBG_MSG3("reframe: samples_to_tag = 0x%08x, diff = 0x%08x, correct = 0x%08x ",
                samples_to_tag, reframe->tag.index - last_timestamped_tag->index, samples_to_tag*OCTETS_PER_SAMPLE);

        reframe->tag.timestamp = ttp_get_next_timestamp(last_timestamped_tag->timestamp, samples_to_tag, reframe->sample_rate, last_timestamped_tag->sp_adjust);
    }

    TTP_DBG_MSG3("reframe: After tag check read index = 0x%08x, next tag index = 0x%08x, tag void = %d",
            prev_rd_index, reframe->tag.index, reframe->tag.is_void);
}

/*
 * reframe_tag_available
 */
bool reframe_tag_available(REFRAME *reframe)
{
    if (reframe->state == REFRAME_STATE_STARTUP)
    {
        /* No tag just yet. */
        return FALSE;
    }
    else
    {
        unsigned buff_size = buff_metadata_get_buffer_size(reframe->metadata_buff);
        if (reframe->prev_tag.length - reframe->prev_tag.octets_consumed < buff_size)
        {
            unsigned prev_rd_index = buff_metadata_get_read_offset(reframe->metadata_buff);
            unsigned prev_wr_index = buff_metadata_get_write_offset(reframe->metadata_buff);
            unsigned tag_index = reframe->tag.index;

            return is_tag_present(prev_rd_index, prev_wr_index, tag_index);
        }
        else
        {
            return FALSE;
        }
    }

}

/*
 * reframe_tag_index
 */
unsigned reframe_tag_index(REFRAME *reframe)
{
    PL_ASSERT(reframe->state != REFRAME_STATE_STARTUP);
    PL_ASSERT(reframe_tag_available(reframe));
    return reframe->tag.index;
}

/*
 * reframe_tag_is_void
 */
bool reframe_tag_is_void(REFRAME *reframe)
{
    PL_ASSERT(reframe->state != REFRAME_STATE_STARTUP);
    return reframe->tag.is_void;
}

/*
 * reframe_tag_length
 */
unsigned reframe_tag_length(REFRAME *reframe)
{
    PL_ASSERT(reframe->state != REFRAME_STATE_STARTUP);
    return reframe->tag.length;
}

/*
 * reframe_tag_playback_time
 */
unsigned reframe_tag_playback_time(REFRAME *reframe)
{
    int *err_offset_ptr;
    PL_ASSERT(reframe->state != REFRAME_STATE_STARTUP);

    if ((err_offset_ptr = ttp_info_get(reframe->tag.err_offset_id)) != NULL)
    {

        TTP_DBG_MSG1("reframe: err_offset_ptr = 0x%08x", *err_offset_ptr);
        return time_sub(reframe->tag.timestamp, *err_offset_ptr);
    }
    else
    {
        return reframe->tag.timestamp;
    }
}

/*
 * reframe_tag_timestamp
 */
unsigned reframe_tag_timestamp(REFRAME *reframe)
{
    PL_ASSERT(reframe->state != REFRAME_STATE_STARTUP);
    return reframe->tag.timestamp;
}

/*
 * reframe_tag_error_offset
 */
int * reframe_tag_error_offset(REFRAME *reframe)
{
    PL_ASSERT(reframe->state != REFRAME_STATE_STARTUP);
    return ttp_info_get(reframe->tag.err_offset_id);
}

/*
 * reframe_sp_adjust
 */
unsigned reframe_sp_adjust(REFRAME *reframe)
{
    PL_ASSERT(reframe->state != REFRAME_STATE_STARTUP);
    return reframe->tag.sp_adjust;
}

/*
 * reframe_consume
 */
void reframe_consume(REFRAME *reframe, unsigned consumed_octets)
{
    unsigned int octets_b4idx, octets_afteridx;
    metadata_tag *tag;
    unsigned buff_size = buff_metadata_get_buffer_size(reframe->metadata_buff);
    unsigned buff_index = buff_metadata_get_read_offset(reframe->metadata_buff);
    unsigned after_buff_index;
    patch_fn_shared(timestamp_reframe);

    /* In the current implementation, the reframing module is sample based. */
    PL_ASSERT(consumed_octets % OCTETS_PER_SAMPLE == 0);
    PL_ASSERT(reframe->state != REFRAME_STATE_STARTUP);

    if (consumed_octets == 0)
    {
        /* Nothing to do. */
        return;
    }

    if (reframe->prev_tag.length - reframe->prev_tag.octets_consumed < buff_size)
    {
        unsigned buff_new_index = BUF_IDX_INC(buff_index, consumed_octets, buff_size);
        unsigned temp_read_index = BUF_IDX_INC(buff_index, 1, buff_size);

        /* Do not consume two partial tags. */
        if(is_tag_present(temp_read_index, buff_new_index, reframe->tag.index))
        {
            TTP_WARN_MSG("reframe: Two partial tag is consumed!");
            return;
        }
    }

    if ((reframe->prev_tag.octets_consumed == reframe->prev_tag.length) ||
        ((reframe->prev_tag.octets_consumed == 0) && (reframe->tag.index == 0) && (buff_index == 0)))
    {
        reframe->prev_tag.length = reframe->tag.length;
        reframe->prev_tag.octets_consumed = 0;

        /* Calculate the next tag index. */
        reframe->tag.index = BUF_IDX_INC(reframe->tag.index, reframe->tag.length, buff_size);

        /* Increment the current timestamp with the tag length. */
        reframe->tag.timestamp = ttp_get_next_timestamp(reframe->tag.timestamp,
                reframe->tag.length / OCTETS_PER_SAMPLE, reframe->sample_rate, reframe->tag.sp_adjust);
   }

    reframe->prev_tag.octets_consumed += consumed_octets;

    tag = buff_metadata_remove(reframe->metadata_buff, consumed_octets, &octets_b4idx , &octets_afteridx);

    after_buff_index = buff_metadata_get_read_offset(reframe->metadata_buff);
    /* Add the read offset. */
    TTP_DBG_MSG4("reframe: consumed_octets = 0x%08x, read index before = 0x%08x  and after = 0x%08x, next tag index = 0x%08x",
        consumed_octets, buff_index,  after_buff_index, reframe->tag.index);

    if ( is_tag_present(buff_index, after_buff_index,reframe->tag_read_index) )
    {
        /* Keep the tag_read_index between the read an write index of the metadata. */
        reframe->tag_read_index = after_buff_index;
    }
    else
    {
        /* Sanity check if the read index is beween the read and write index. */
        PL_ASSERT(is_tag_present(after_buff_index,
                buff_metadata_get_write_offset(reframe->metadata_buff), reframe->tag_read_index) );
    }

    if (reframe->state == REFRAME_STATE_VOID_TAGS)
    {
        /* Only check void tags. Timestaped may appear until the next void tag. */
        if ((tag != NULL) && IS_VOID_TTP_TAG(tag))
        {
            /* The index of the tag must be good if there are no overlaps/gaps between
             * tags, but the length is still the length of the previous tag. */
            unsigned buffer_size = buff_metadata_get_buffer_size(reframe->metadata_buff);
            PL_ASSERT(reframe->tag.index == BUF_IDX_INC(tag->index, tag->length, buffer_size));
            PL_ASSERT(reframe->tag.length == tag->length);
            PL_ASSERT(tag->next == NULL);
        }
    }
    else if(reframe->state != REFRAME_STATE_TIMESTAMPED_TAGS)
    {
        PL_ASSERT(FALSE);
    }

    /* Delete the consumed tags.*/
    buff_metadata_tag_list_delete(tag);
}

