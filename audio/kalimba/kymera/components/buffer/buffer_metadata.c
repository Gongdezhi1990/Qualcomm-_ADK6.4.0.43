/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup buffer Buffer Subsystem
 * \file  buffer_metadata.c
 *
 * \ingroup buffer
 *
 * Buffer metadata implementation
 *
 */

/****************************************************************************
Include Files
*/
#include "buffer_private.h"
#include "sections.h"

#ifdef INSTALL_METADATA

/****************************************************************************
Private Constant Declarations
*/

/****************************************************************************
Private Macro Declarations
*/

/* Rounded length of a private data item (in allocation units) */
#define PRIV_ITEM_LENGTH(length) ROUND_UP_TO_WHOLE_WORDS(sizeof(metadata_priv_item) + (length))

/* Get a pointer to the next private data item in the array */
#define PRIV_ITEM_NEXT(item) (metadata_priv_item *)((unsigned *)(item) + PRIV_ITEM_LENGTH((item)->length)/sizeof(unsigned));

/* Attempt to limit the total number of allocated tags
 * This number is checked against the allocation count in buff_metadata_tag_threshold_exceeded()
 * Note: Not static or the compiler will optimise it out, and we want it in memory for easy patchability
 */
#ifndef DEFAULT_TAG_ALLOC_THRESHOLD
#error Builds with metadata must define DEFAULT_TAG_ALLOC_THRESHOLD
#else
unsigned tag_alloc_threshold = DEFAULT_TAG_ALLOC_THRESHOLD;
#endif
/****************************************************************************
Private Variable Definitions
*/

/* Count of currently-allocated tags */
static unsigned tag_alloc_count = 0;

/****************************************************************************
Private Function Declarations
*/

/****************************************************************************
Private Function Definitions
*/

static void buff_metadata_delay_core(tCbuffer *buff, unsigned delay_octets, bool add)
{
    metadata_list *start, *mlist;

    patch_fn_shared(buff_metadata);

    if (buff == NULL)
    {
        return;
    }

    mlist = buff->metadata;
    if (mlist == NULL)
    {
        /* When there is no metadata for the buffer then there is no action to perform */
        return;
    }
    /* The operation shouldn't be trying to wrap the buffer. */
    PL_ASSERT(delay_octets <= mlist->buffer_size);

    start = mlist;

    do
    {
        unsigned buffsize = mlist->buffer_size;
        unsigned curr_write = mlist->prev_wr_index;

        if (add)
        {
            /* Adding the delay so increment the write index */
            curr_write += delay_octets;
            if (curr_write > buffsize)
            {
                curr_write -= buffsize;
            }
        }
        else
        {
            /* Removing the delay so decrement the write index */
            if (curr_write < delay_octets)
            {
                curr_write += buffsize;
            }
            curr_write -= delay_octets;
        }

        mlist->prev_wr_index = curr_write;

        mlist = mlist->next;
    } while (mlist != start);
}

/**
 * \brief Get total length (in allocation units) of existing private data
 */
unsigned priv_data_length(metadata_tag *tag)
{
    unsigned count = 0, num_items;
    metadata_priv_item *item;

    PL_ASSERT(tag->xdata != NULL);

    num_items = tag->xdata->item_count;
    item = (metadata_priv_item *)&tag->xdata->items[0];
    while (count < num_items)
    {
        item = PRIV_ITEM_NEXT(item);
        count++;
    }

    /* Pointer subtraction will give size in addresses */
    return ((char *)item - (char *)(tag->xdata));
}

static void buff_metadata_align_to_buff_ptrs_core(tCbuffer *buff, bool read, bool write)
{
    if(buff != NULL)
    {
        metadata_list *mlist;
        if (BUF_DESC_BUFFER_TYPE_MMU(buff->descriptor))
        {
            /* This function only knows how to operate on SW buffers. */
            panic_diatribe(PANIC_AUDIO_METADATA_HW_BUFFERS_NOT_SUPPORTED, (DIATRIBE_TYPE)(uintptr_t)buff);
        }

        mlist = buff->metadata;
        if (mlist != NULL)
        {
            unsigned wr_offset, rd_offset;
            unsigned octets_per_word = buff_metadata_get_usable_octets(buff);
            /* Calculate the offset in addressable units and then convert this
             * to octets. */
            if (write)
            {
                wr_offset = (unsigned)(uintptr_t)(buff->write_ptr) - (unsigned)(uintptr_t)(buff->base_addr);
                mlist->prev_wr_index = (wr_offset * octets_per_word) / sizeof(int);
#ifdef METADATA_DEBUG_TRANSPORT
                mlist->next_tag_index = mlist->prev_wr_index;
                mlist->last_tag_still_covers = 0;
#endif /* METADATA_DEBUG_TRANSPORT */
            }
            if (read)
            {
                rd_offset = (unsigned)(uintptr_t)(buff->read_ptr) - (unsigned)(uintptr_t)(buff->base_addr);
                mlist->prev_rd_index = (rd_offset * octets_per_word) / sizeof(int);
            }
        }
    }
}

/****************************************************************************
Public Function Definitions
*/

void buff_metadata_init(unsigned count)
{
    /* Initialise the metadata system,
     * making space for the number of tags specified by count
     */
#ifdef METADATA_USE_PMALLOC
     /* Eventually this will probably use some private storage
     * but for initial prototyping we'll just use pmalloc
     * to allocate the tags on demand
     * so this doesn't need to do anything
     */
#else
    /* TODO some stuff to initialise local storage */
#endif
}

/*
 * buff_metadata_tag_threshold_exceeded
 */
bool buff_metadata_tag_threshold_exceeded(void)
{
    return (tag_alloc_count > tag_alloc_threshold);
}

metadata_tag *buff_metadata_new_tag(void)
{
    patch_fn_shared(buff_metadata);
#ifdef METADATA_USE_PMALLOC
    /* See above, just use the normal dynamic memory system for now */
    metadata_tag *tag = xzpnew(metadata_tag);
    if (tag != NULL)
    {
        LOCK_INTERRUPTS;
        tag_alloc_count++;
        UNLOCK_INTERRUPTS;

        return tag;
    }
    fault_diatribe(FAULT_AUDIO_METADATA_TAG_ALLOCATION_FAILED, 0);
#endif /* METADATA_USE_PMALLOC */
    return NULL;
}

void buff_metadata_delete_tag(metadata_tag *tag, bool process_eof)
{
    patch_fn_shared(buff_metadata);

    if (tag != NULL)
    {
        /* Check if this tag has an end-of-file callback */
        if (process_eof && METADATA_STREAM_END(tag))
        {
            metadata_handle_eof_tag_deletion(tag);
        }
        pdelete(tag->xdata);
#ifdef METADATA_USE_PMALLOC
        /* See above, just use the normal dynamic memory system for now */
        LOCK_INTERRUPTS;
        if (tag_alloc_count > 0)
        {
            tag_alloc_count--;
        }
        else
        {
            /* Not much we can do here except maybe fault ? */
#ifndef UNIT_TEST_BUILD
            L2_DBG_MSG("Metadata tag deleted but count is already zero ?");
#endif
        }
        UNLOCK_INTERRUPTS;
        pdelete(tag);
#endif /* METADATA_USE_PMALLOC */
    }
}

void buff_metadata_tag_list_delete(metadata_tag *list)
{
    metadata_tag *t;

    while (list != NULL)
    {
        t = list;
        list = list->next;
        buff_metadata_delete_tag(t, TRUE);
    }
}


metadata_tag *buff_metadata_copy_tag(metadata_tag *tag)
{
    metadata_tag *new_cpy;

    patch_fn_shared(buff_metadata);

    if (tag == NULL)
    {
        return NULL;
    }

    new_cpy = buff_metadata_new_tag();

    if (new_cpy != NULL)
    {
        *new_cpy = *tag;
        if (tag->xdata != NULL)
        {
            unsigned length = priv_data_length(tag);
            metadata_priv_data *new_data = (metadata_priv_data *)xpmalloc(length);

            /* If there isn't enough RAM for this, tough the data gets lost */
            if (new_data != NULL)
            {
                memcpy(new_data, tag->xdata, length);
                if (METADATA_STREAM_END(tag))
                {
                    metadata_handle_eof_tag_copy(tag, FALSE);
                }
            }
            new_cpy->xdata = new_data;
        }
    }
    return new_cpy;
}

void buff_metadata_set_usable_octets(tCbuffer *buff, unsigned usable_octets)
{
    PL_ASSERT(buff->metadata != NULL);

    if (usable_octets >= 4)
    {
        if (usable_octets > OCTETS_PER_SAMPLE)
        {
            fault_diatribe(FAULT_AUDIO_METADATA_USABLE_OCTETS_INVALID, usable_octets);
        }
        /* metadata->usable_octets is a 2-bit value, allowing up to 4 octets (32-bit words) */
        buff->metadata->usable_octets = 0;  /* all octets used */
        usable_octets = 4;                  /* for 32-bit platforms */
    }
    else
    {
        buff->metadata->usable_octets = usable_octets;
    }
    buff->metadata->buffer_size = cbuffer_get_size_in_words(buff) * usable_octets;

#ifdef INSTALL_METADATA_DUALCORE
    buff_metadata_kip_set_usable_octets(buff, usable_octets);
#endif /* INSTALL_METADATA_DUALCORE */

}

unsigned int buff_metadata_get_usable_octets(tCbuffer *buff)
{
    unsigned usable_octets = 0;
    if ((buff != NULL) && (buff->metadata != NULL))
    {

#ifdef INSTALL_METADATA_DUALCORE
        buff_metadata_kip_get_usable_octets(buff);
#endif /* INSTALL_METADATA_DUALCORE */

        usable_octets = buff->metadata->usable_octets;
    }
    if (0 == usable_octets)
    {
        /* metadata->usable_octets is a 2-bit value, allowing up to 4 octets (32-bit words) */
        usable_octets = 4;
    }
    return usable_octets;
}

unsigned int buff_metadata_get_buffer_size(tCbuffer *buff)
{
    return buff->metadata->buffer_size;
}


void buff_metadata_add_delay(tCbuffer *buff, unsigned delay_octets)
{
    buff_metadata_delay_core(buff, delay_octets, TRUE);
}

void buff_metadata_remove_delay(tCbuffer *buff, unsigned delay_octets)
{
    buff_metadata_delay_core(buff, delay_octets, FALSE);
}

void buff_metadata_align_to_buff_ptrs(tCbuffer *buff)
{
    buff_metadata_align_to_buff_ptrs_core(buff, TRUE, TRUE);
}

void buff_metadata_align_to_buff_read_ptr(tCbuffer *buff)
{
    buff_metadata_align_to_buff_ptrs_core(buff, TRUE, FALSE);
}

void buff_metadata_align_to_buff_write_ptr(tCbuffer *buff)
{
    buff_metadata_align_to_buff_ptrs_core(buff, FALSE, TRUE);
}


RUN_FROM_PM_RAM
#ifdef METADATA_DEBUG_TRANSPORT
bool buff_metadata_append_dbg(tCbuffer *buff, metadata_tag *tag,
             unsigned octets_pre_written, unsigned octets_post_written,
             unsigned caller_addr)
#else /* METADATA_DEBUG_TRANSPORT */
bool buff_metadata_append(tCbuffer *buff, metadata_tag *tag,
             unsigned octets_pre_written, unsigned octets_post_written)
#endif /* METADATA_DEBUG_TRANSPORT */
{
#ifdef METADATA_DEBUG_TRANSPORT
    unsigned return_addr = pl_get_return_addr();
#endif /* METADATA_DEBUG_TRANSPORT */

    metadata_list *mlist;
    metadata_list *mlist_start;

    patch_fn(metadata_append);

    if (buff == NULL)
    {
        buff_metadata_tag_list_delete(tag);
        return TRUE;
    }

    mlist_start = mlist = buff->metadata;

    if (mlist == NULL)
    {
        buff_metadata_tag_list_delete(tag);
        return TRUE;
    }

    if (tag != NULL)
    {
        /* The caller shouldn't be advancing by more than the buffer_size. It
         * doesn't make sense that that much data was written, this function
         * doesn't handle it on account of it not being valid. */
        PL_ASSERT(octets_pre_written <= mlist->buffer_size);
        PL_ASSERT(octets_post_written <= mlist->buffer_size);

#ifdef METADATA_DEBUG_TRANSPORT
#if ((METADATA_DEBUG_TRANSPORT+0) > 1)
        /* An extra check to make sure the tags to be written fit in the
         * destination buffer. It is normally disabled to avoid the CPU cost.
         * Define METADATA_DEBUG_TRANSPORT to 2 or a larger number to enable.
         */
        {
            metadata_tag *temp_tag = tag;
            unsigned available_space = buff_metadata_available_space(buff);
            unsigned total_length = octets_pre_written + octets_post_written;
            while (temp_tag != NULL)
            {
                if (temp_tag->next != NULL)
                {
                    total_length += temp_tag->length;
                }
                temp_tag = temp_tag->next;
            }
            if (available_space < total_length)
            {
                L2_DBG_MSG5("buff_metadata_append: wp %d rp %d sz %d ob %d oa %d",
                            mlist->prev_wr_index, mlist->prev_rd_index,
                            mlist->buffer_size, octets_pre_written,
                            octets_post_written);
                L2_DBG_MSG2( "buff_metadata_append: "
                             "available_space %d < total_length %d",
                             available_space, total_length);
#ifdef METADATA_DEBUG_TRANSPORT_FAULT
                fault_diatribe(FAULT_AUDIO_METADATA_APPEND_EXCEEDS_SPACE,
                               ((caller_addr != 0)?caller_addr:return_addr));
#else /* METADATA_DEBUG_TRANSPORT_FAULT */
                L2_DBG_MSG1("AUDIO_METADATA_APPEND_EXCEEDS_SPACE caller 0x%08x",
                            ((caller_addr != 0)?caller_addr:return_addr));
#endif /* METADATA_DEBUG_TRANSPORT_FAULT */
            }
        }
#endif /* METADATA_DEBUG_TRANSPORT > 1 */
#endif /* METADATA_DEBUG_TRANSPORT */

        do
        {
            metadata_tag *lp_tag;
            unsigned buffsize, index;

#ifdef METADATA_DEBUG_TRANSPORT
            switch (mlist->metadbg_transport_state)
            {
            case METADBG_TRANSPORT_STARTING:
                mlist->metadbg_transport_state = METADBG_TRANSPORT_NORMAL;
                break;
            case METADBG_TRANSPORT_EMPTY:
#ifdef METADATA_DEBUG_TRANSPORT_FAULT
                fault_diatribe(FAULT_AUDIO_METADATA_APPEND_MISALIGNED,
                            ((caller_addr != 0)?caller_addr:return_addr));
#else /* METADATA_DEBUG_TRANSPORT_FAULT */
                L2_DBG_MSG1("AUDIO_METADATA_APPEND_MISALIGNED caller 0x%08x",
                            ((caller_addr != 0)?caller_addr:return_addr));
#endif /* METADATA_DEBUG_TRANSPORT_FAULT */
                break;
            case METADBG_TRANSPORT_NORMAL:
            default:
                break;
            }
#endif

            /* If this is the last one in the CLL then re-use the existing tags,
             * otherwise we make a copy. */
            if (mlist->next == mlist_start)
            {
                lp_tag = tag;
            }
            else
            {
                /* Make a copy of the list. The existing list is NULL terminated so
                 * don't need to do that, it'll naturally get copied as NULL.*/
                metadata_tag *new, *tail, *tag_2_cpy = tag;
                tail = lp_tag = NULL;

                while (tag_2_cpy != NULL)
                {
                    new = buff_metadata_copy_tag(tag_2_cpy);

                    if (lp_tag == NULL)
                    {
                        tail = lp_tag = new;
                    }
                    else
                    {
                        tail->next = new;
                        tail = new;
                    }
                    tag_2_cpy = tag_2_cpy->next;
                }
            }

            buffsize = mlist->buffer_size;
            index = mlist->prev_wr_index + octets_pre_written;

            if (index >= buffsize)
            {
                index -= buffsize;
            }
#ifdef METADATA_DEBUG_TRANSPORT
            if (mlist->last_tag_still_covers > octets_pre_written)
            {
                L2_DBG_MSG3("buff_metadata_append: new tag in previous tag's "
                            "range: still_covered %d oct_bf %d tag_len %d",
                            mlist->last_tag_still_covers, octets_pre_written,
                            tag->length);
#ifdef METADATA_DEBUG_TRANSPORT_FAULT
                fault_diatribe(FAULT_AUDIO_METADATA_APPEND_MISALIGNED,
                               ((caller_addr != 0)?caller_addr:return_addr));
#else /* METADATA_DEBUG_TRANSPORT_FAULT */
                L2_DBG_MSG1("AUDIO_METADATA_APPEND_MISALIGNED caller 0x%08x",
                            ((caller_addr != 0)?caller_addr:return_addr));
#endif /* METADATA_DEBUG_TRANSPORT_FAULT */
            }
            else if (index != mlist->next_tag_index)
            {
                L2_DBG_MSG5("buff_metadata_append: "
                            "next_tag_index %d != index %d "
                            "(prev_wr_index %d oct_pre %d bsz %d)",
                            mlist->next_tag_index, index, mlist->prev_wr_index,
                            octets_pre_written, buffsize);
#ifdef METADATA_DEBUG_TRANSPORT_FAULT
                fault_diatribe(FAULT_AUDIO_METADATA_APPEND_MISALIGNED,
                               ((caller_addr != 0)?caller_addr:return_addr));
#else /* METADATA_DEBUG_TRANSPORT_FAULT */
                L2_DBG_MSG1("AUDIO_METADATA_APPEND_MISALIGNED caller 0x%08x",
                            ((caller_addr != 0)?caller_addr:return_addr));
#endif /* METADATA_DEBUG_TRANSPORT_FAULT */
            }
#endif /* METADATA_DEBUG_TRANSPORT */

            LOCK_INTERRUPTS;
            while (lp_tag != NULL)
            {
                metadata_tag *nxt_tag;
                lp_tag->index = index;

                /* effectively appending the tag */
                if (mlist->tags.head == NULL)
                {
                    /* Empty list, tail should also be NULL */
                    PL_ASSERT(mlist->tags.tail == NULL);
                    mlist->tags.head = mlist->tags.tail = lp_tag;
                }
                else
                {
                    /* Non-empty list, tail won't be NULL */
                    PL_ASSERT(mlist->tags.tail != NULL);
                    mlist->tags.tail->next = lp_tag;
                    mlist->tags.tail = lp_tag;
                }

                nxt_tag = lp_tag->next;

#ifdef METADATA_DEBUG_TRANSPORT
                {
                    unsigned next_index = index + lp_tag->length;
                    if (next_index >= buffsize)
                    {
                        next_index -= buffsize;
                    }
                    if (nxt_tag != NULL)
                    {
                        index = next_index;
                    }
                    else
                    {
                        /* If tag spans beyond end of this transfer,
                         * it may wrap.
                         */
                        if (lp_tag->length < octets_post_written)
                        {
                            mlist->last_tag_still_covers = 0;
                            L2_DBG_MSG2("buff_metadata_append: last tag "
                                        "leaves gap last_tag->len %d < "
                                        "oct_post %d",
                                        lp_tag->length, octets_post_written);
#ifdef METADATA_DEBUG_TRANSPORT_FAULT
                            fault_diatribe(
                                FAULT_AUDIO_METADATA_APPEND_MISALIGNED,
                                ((caller_addr!=0)?caller_addr:return_addr));
#else /* METADATA_DEBUG_TRANSPORT_FAULT */
                            L2_DBG_MSG1("AUDIO_METADATA_APPEND_MISALIGNED caller 0x%08x",
                                        ((caller_addr != 0)?caller_addr:return_addr));
#endif /* METADATA_DEBUG_TRANSPORT_FAULT */
                        }
                        else
                        {
                            mlist->last_tag_still_covers =
                                    lp_tag->length - octets_post_written;
                        }
                        mlist->next_tag_index = next_index % buffsize;
                    }
                }
#else /* METADATA_DEBUG_TRANSPORT */
                if (nxt_tag != NULL)
                {
                    index += lp_tag->length;
                    if (index >= buffsize)
                    {
                        index -= buffsize;
                    }
                }
#endif /* METADATA_DEBUG_TRANSPORT */
                lp_tag = nxt_tag;
            }

            index = index + octets_post_written;
            if (index >= buffsize)
            {
                index -= buffsize;
            }
            mlist->prev_wr_index = index;
            UNLOCK_INTERRUPTS;

        mlist = mlist->next;
        } while (mlist != mlist_start);
    }
    else /* The tag is NULL so just update the write index */
    {
        do
        {
            unsigned index = mlist->prev_wr_index + octets_pre_written + octets_post_written;
            unsigned buffsize = mlist->buffer_size;
            if (index >= buffsize)
            {
                index -= buffsize;
            }

#ifdef METADATA_DEBUG_TRANSPORT
            /* If this is the first time mlist is accessed, update from  */
            /* starting to empty (we expect only NULL tags from now on). */
            if (mlist->metadbg_transport_state == METADBG_TRANSPORT_STARTING)
            {
                mlist->metadbg_transport_state = METADBG_TRANSPORT_EMPTY;
            }
            else
            if (mlist->metadbg_transport_state == METADBG_TRANSPORT_EMPTY)
            {
                /* NULL tag, empty transport, nothing to update */
            }
            else
            if (mlist->last_tag_still_covers >=
                    (octets_pre_written + octets_post_written))
            {
                mlist->last_tag_still_covers -=
                        (octets_pre_written + octets_post_written);
            }
            else
            {
                mlist->last_tag_still_covers = 0;
                L2_DBG_MSG5("buff_metadata_append: expected next tag at "
                            "next_idx %d prev_wr %d bsz %d "
                            "oct_bf %d oct_af %d",
                            mlist->next_tag_index, mlist->prev_wr_index,
                            buffsize, octets_pre_written, octets_post_written);
#ifdef METADATA_DEBUG_TRANSPORT_FAULT
                fault_diatribe(
                    FAULT_AUDIO_METADATA_APPEND_MISALIGNED,
                    ((caller_addr!=0)?caller_addr:return_addr));
#else /* METADATA_DEBUG_TRANSPORT_FAULT */
                L2_DBG_MSG1("AUDIO_METADATA_APPEND_MISALIGNED caller 0x%08x",
                            ((caller_addr != 0)?caller_addr:return_addr));
#endif /* METADATA_DEBUG_TRANSPORT_FAULT */
            }
#endif /* METADATA_DEBUG_TRANSPORT */

            mlist->prev_wr_index = index;

            mlist = mlist->next;
        } while (mlist != mlist_start);
    }
    return TRUE;
}

metadata_tag *buff_metadata_peek(tCbuffer *buff)
{
    if (buff != NULL && buff->metadata != NULL)
    {
        return buff->metadata->tags.head;
    }

    return NULL;
}

metadata_tag *buff_metadata_peek_ex(tCbuffer *buff, unsigned *octets_b4idx)
{
    if (buff != NULL && buff->metadata != NULL)
    {
        metadata_list *mlist = buff->metadata;

        metadata_tag* tag = mlist->tags.head;
        if (tag != NULL)
        {
            int before_idx = tag->index - mlist->prev_rd_index;
            if (before_idx < 0)
            {
                before_idx += mlist->buffer_size;
            }
            *octets_b4idx = before_idx;
        }
        return tag;
    }
    return NULL;
}

unsigned buff_metadata_available_octets(tCbuffer *buff)
{
    unsigned available_octets;
    metadata_list *mlist;
    unsigned buffsize;

    /* Sanity check. */
    PL_ASSERT(buff != NULL);

    mlist = buff->metadata;
    PL_ASSERT(mlist != NULL);
    buffsize = mlist->buffer_size;

    /* This test below relies on integer underflow and the fact that addition
     * of buffsize will result in integer overflow and yield the desired result. */
    available_octets = mlist->prev_wr_index - mlist->prev_rd_index;
    if (available_octets >= buffsize)
    {
        available_octets += buffsize;
    }

    return available_octets;
}

unsigned buff_metadata_available_space(tCbuffer *buff)
{
    unsigned available_space;
    metadata_list *mlist;
    unsigned buffsize;

    /* Sanity check. */
    PL_ASSERT(buff != NULL);

    mlist = buff->metadata;
    PL_ASSERT(mlist != NULL);
    buffsize = mlist->buffer_size;

    available_space = buffsize - buff_metadata_available_octets(buff) - 1;

    return available_space;
}


RUN_FROM_PM_RAM
#ifdef METADATA_DEBUG_TRANSPORT
metadata_tag *buff_metadata_remove_dbg(tCbuffer *buff, unsigned octets_consumed, unsigned *octets_b4idx, unsigned *octets_afteridx, unsigned caller_addr)
#else /* METADATA_DEBUG_TRANSPORT */
metadata_tag *buff_metadata_remove(tCbuffer *buff,
                         unsigned octets_consumed, unsigned *octets_b4idx, unsigned *octets_afteridx)
#endif /* METADATA_DEBUG_TRANSPORT */
{
#ifdef METADATA_DEBUG_TRANSPORT
    unsigned return_addr = pl_get_return_addr();
#endif /* METADATA_DEBUG_TRANSPORT */
    metadata_list *mlist;

    patch_fn(metadata_remove);

    if (buff == NULL)
    {
        return NULL;
    }

    mlist = buff->metadata;

    if (mlist != NULL)
    {
        unsigned last_tag_idx = 0;
        unsigned prev_idx = mlist->prev_rd_index;
        unsigned rd_idx = prev_idx + octets_consumed;
        unsigned buffsize = mlist->buffer_size;
        metadata_tag **new_head = &(mlist->tags.head);

        /* The operation shouldn't be trying to wrap the buffer. */
        PL_ASSERT(octets_consumed <= buffsize);

        /* Check if there is enough data.*/
        unsigned avail = buff_metadata_available_octets(buff);
        if (avail < octets_consumed)
        {
            /* This used to be an ASSERT, but it fired too often,
                * so for now just log the error condition and hope we recover
                */
            L2_DBG_MSG2("buff_metadata_remove consumed = %d avail = %d", octets_consumed, avail);
#ifdef METADATA_DEBUG_TRANSPORT
#ifdef METADATA_DEBUG_TRANSPORT_FAULT
            fault_diatribe(FAULT_AUDIO_METADATA_REMOVE_EXCEEDS_AVAILABLE,
                           ((caller_addr != 0)?caller_addr:return_addr));
#else /* METADATA_DEBUG_TRANSPORT_FAULT */
            L2_DBG_MSG1("AUDIO_METADATA_REMOVE_EXCEEDS_AVAILABLE caller 0x%08x",
                        ((caller_addr != 0)?caller_addr:return_addr));
#endif /* METADATA_DEBUG_TRANSPORT_FAULT */
#endif /* METADATA_DEBUG_TRANSPORT */
        }

        if (*new_head != NULL)
        {
            int octets_2idx = (*new_head)->index - prev_idx;

            if (octets_2idx < 0)
            {
                octets_2idx += buffsize;
            }
            *octets_b4idx = octets_2idx;

            /* If this remove will wrap the index then find all tags that
             * precede the end of the buffer before looking for those that
             * come post wrap */
            if (rd_idx >= buffsize)
            {
                rd_idx -= buffsize;
                /* prev_idx points to the next place to read hence >= */
                while (*new_head != NULL && (*new_head)->index >= prev_idx)
                {
                    last_tag_idx = (*new_head)->index;
                    new_head = &(*new_head)->next;
                }
                prev_idx = 0;
            }
            /* Look for any tags between the current position and where
             * we're removing up to */
            while (*new_head != NULL
                  && (*new_head)->index >= prev_idx
                  && (*new_head)->index < rd_idx )
            {
                last_tag_idx = (*new_head)->index;
                new_head = &(*new_head)->next;
            }
            /* also remove any zero-length tags (such as STREAM_END) right at the end */
            if( (*new_head) != NULL  && 0 == (*new_head)->length
                    && (*new_head)->index == rd_idx )
            {
                last_tag_idx = (*new_head)->index;
                new_head = &(*new_head)->next;
            }

            mlist->prev_rd_index = rd_idx;
            /* If the new head value is the same as the old one then nothing to remove */
            if (mlist->tags.head == *new_head)
            {
                *octets_b4idx = octets_consumed;
                *octets_afteridx = 0;
                return NULL;
            }
            else
            {
                metadata_tag *ret_mtag = mlist->tags.head;

                if ((*new_head) == NULL)
                {
                    metadata_tag * volatile * vol_new_head = new_head;
                    /* Removing the last element in the list need to do this atomically
                     * otherwise an append could pre-empt us and the pointer gets lost */
                    LOCK_INTERRUPTS;
                    /* If append pre-empted the last check before interrupts were locked
                     * then the tail will have been updated and we should leave it alone
                     * and this becomes a snip the list operation. */
                    if ((mlist->tags.head = (metadata_tag *)(*vol_new_head)) == NULL)
                    {
                        mlist->tags.tail = NULL;
                    }
                    else
                    {
                        *new_head = NULL;
                    }
                    UNLOCK_INTERRUPTS;
                }
                else
                {
                    /* Snip the list */
                    mlist->tags.head = *new_head;
                    *new_head = NULL;
                }
                if (rd_idx < last_tag_idx)
                {
                    rd_idx += buffsize;
                }
                *octets_afteridx = rd_idx - last_tag_idx;
                return ret_mtag;
            }
        }

        if (rd_idx >= buffsize)
        {
            rd_idx -= buffsize;
        }
        mlist->prev_rd_index = rd_idx;

    }
    *octets_b4idx = octets_consumed;
    *octets_afteridx = 0;
    return NULL;
}


extern void buff_metadata_enable(tCbuffer *buff)
{
    PL_ASSERT(buff != NULL);
    PL_ASSERT(buff->metadata == NULL);
    buff->metadata = zpnew(metadata_list);
    /* link to self and cnt to 1, i.e. no multi-channel */
    buff->metadata->next = buff->metadata;
    buff->metadata->ref_cnt = 1;
    /* store size locally, used in sw cbuffer wrap round adjustments */
    buff->metadata->buffer_size = cbuffer_get_size_in_octets(buff);
    /* NB. possibly not all octets are usable, when this is the case
        metadata->usable_octets must be configured (buff_metadata_set_usable_octets)
        before consuming any data from this buffer */
}

/*
 * buff_metadata_get_read_offset
 */
unsigned buff_metadata_get_read_offset(tCbuffer *buff)
{
    PL_ASSERT(buff != NULL);
    return buff->metadata->prev_rd_index;
}

/*
 * buff_metadata_get_write_offset
 */
unsigned buff_metadata_get_write_offset(tCbuffer *buff)
{
    PL_ASSERT(buff != NULL);
    return buff->metadata->prev_wr_index;
}

/*
 * buff_metadata_get_head
 */
metadata_tag* buff_metadata_get_head(tCbuffer *buff)
{
    PL_ASSERT(buff != NULL);
    return buff->metadata->tags.head;
}

extern bool buff_metadata_connect(tCbuffer *buff, tCbuffer *src_meta_buff, tCbuffer *sink_meta_buff)
{
    metadata_list *src_meta;
    metadata_list *sink_meta;

    patch_fn_shared(buff_metadata);

    /* The buffer pointers can be NULL as well as the metadata itself being NULL
     * luckily the logic is exactly the same whether the buffer or the metadata
     * pointer is NULL, what is really needed is the metadata pointer if it is
     * non-NULL. Extract this first and then work with the result as it makes
     * the rest of the logic a lot less convoluted. */
    src_meta = (src_meta_buff == NULL) ? NULL : src_meta_buff->metadata;
    sink_meta = (sink_meta_buff == NULL) ? NULL : sink_meta_buff->metadata;

    PL_ASSERT(buff != NULL);
    PL_ASSERT(buff->metadata == NULL);

    if (sink_meta == NULL)
    {
        buff->metadata = xzpnew(metadata_list);
        if (buff->metadata == NULL)
        {
            return FALSE;
        }
        buff->metadata->buffer_size = cbuffer_get_size_in_octets(buff);

        /** Update the circular linked list */
        if (src_meta == NULL)
        {
            /** It's only thing in the list, make it point to itself */
            buff->metadata->next = buff->metadata;
        }
        else
        {
            /** Insert into the list */
            buff->metadata->next = src_meta->next;
            src_meta->next = buff->metadata;
        }
    }
    else
    {
        /* Need to check here if src & sink are in the same list */
        metadata_list *start = sink_meta;
        bool match = FALSE;
        do
        {
            if (src_meta == sink_meta)
            {
                match = TRUE;
                /* The metadata is to be shared with other buffers so re-use the
                 * metadata provided by the sink in the connection as that's the
                 * one in the list that represents this direction of flow. */
                buff->metadata = start;
                break;
            }
            sink_meta = sink_meta->next;
        } while (sink_meta != start);

        if (!match)
        {
            /* There is already metdata being supplied to this entity and it can't
             * accept any more on this connection so don't configure the buffer for
             * metadata and append will release the tags as they are appended to this
             * buffer. */
            return TRUE;
        }
    }

    BUFF_METADATA_SET(buff);
    buff->metadata->ref_cnt++;

    return TRUE;
}

extern void buff_metadata_release(tCbuffer *buff)
{
    metadata_list *meta;

    patch_fn_shared(buff_metadata);

    if (buff == NULL || !buff_has_metadata(buff))
    {
        return;
    }

    meta = buff->metadata;
    buff->metadata = NULL;
    BUFF_METADATA_UNSET(buff);

    meta->ref_cnt--;
    if (meta->ref_cnt <= 0)
    {
        /* To remove the element from the circularly LL we need to find the
         * element that precedes meta and make it point to the next element. */
        metadata_list *end = meta->next;
        while (end->next != meta)
        {
            end = end->next;
        }
        end->next = meta->next;

        /* If there are any meta data tags attached free them before we release
         * the metadata_list structure. */
        buff_metadata_tag_list_delete(meta->tags.head);

        /* Now that meta is no longer in the Circular LL it can be freed */
        pdelete(meta);
    }
}


/* metadata_strict_transport
    for operators that have an opinion about how much they want to process
    returns last tag transported over (removed); could be NULL
 */
extern metadata_tag *metadata_strict_transport(
                        tCbuffer *src, tCbuffer *dst, unsigned trans_octets)
{
#ifdef METADATA_DEBUG_TRANSPORT
    unsigned return_addr = pl_get_return_addr();
#endif /* METADATA_DEBUG_TRANSPORT */
    metadata_tag *ret_mtag;
    unsigned b4idx, afteridx;

    patch_fn_shared(buff_metadata);

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

    if (dst != NULL)
    {
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
    return ret_mtag;
}

/*
 * \brief Add private data to a metadata tag
 */
void* buff_metadata_add_private_data(metadata_tag *tag, unsigned key, unsigned length, void *data)
{
    unsigned old_size,new_size;
    metadata_priv_data *new_data = NULL;
    metadata_priv_item *new_item;

    patch_fn_shared(buff_metadata);

    if (tag->xdata == NULL)
    {
        /* Allow space for the item count in the first word */
        old_size = sizeof(unsigned);
    }
    else
    {
        old_size = priv_data_length(tag);
        /* If the old size isn't at least a word, something has gone badly wrong */
        PL_ASSERT(old_size >= sizeof(unsigned));
    }

    /* First check if we can reuse the existing allocation */
    new_size = old_size + PRIV_ITEM_LENGTH(length);
    /* Note psizeof(NULL) returns zero, so this is always safe */
    if (new_size > psizeof(tag->xdata))
    {
        /* New allocation needed */
        if ((new_data = (metadata_priv_data *)xpmalloc(new_size)) == NULL)
        {
            /* Allocation failed, just return NULL without changing anything */
            return NULL;
        }
    }
    if (tag->xdata != NULL)
    {
        /* Existing data, so we need to copy it if there was a new allocation */
        if (new_data != NULL)
        {
            /* Copy all of the existing data (including the item count)... */
            memcpy(new_data, tag->xdata, old_size);
            /* ...and free the old data */
            pfree(tag->xdata);
            tag->xdata = new_data;
        }
        /* Increment the item count for the new item */
        tag->xdata->item_count++;
    }
    else
    {
        /* No existing data, so this must be a new allocation */
        PL_ASSERT(new_data != NULL);
        tag->xdata = new_data;
        tag->xdata->item_count = 1;
    }
    /* Populate the new item */
    new_item = (metadata_priv_item *)((char *)(tag->xdata) + old_size);
    new_item->key = key;
    new_item->length = length;

    /* Copy the new data if provided */
    if (data != NULL)
    {
        memcpy(&(new_item->data), data, length);
    }

    return &(new_item->data);
}

/*
 * \brief Find private data in a metadata tag
 */
bool buff_metadata_find_private_data(metadata_tag *tag, unsigned key, unsigned *out_length, void **out_data)
{
    unsigned count = 0, num_items;
    metadata_priv_item *item;

    patch_fn_shared(buff_metadata);

    if (tag->xdata != NULL)
    {
        num_items = tag->xdata->item_count;
        item = (metadata_priv_item *)&tag->xdata->items[0];
        while (count < num_items)
        {
            if (item->key == key)
            {
                *out_length = item->length;
                *out_data = &item->data;
                return TRUE;
            }
            item = PRIV_ITEM_NEXT(item);
            count++;
        }
    }
    return FALSE;
}

#ifdef INSTALL_METADATA_DUALCORE
metadata_tag* buff_metadata_pop_tags_from_KIP(tCbuffer* shared_buffer,
                                              metadata_tag** tail)
{
    metadata_tag *first_t = buff_metadata_kip_tag_from_buff(shared_buffer);
    metadata_tag *t = first_t;
    while (t != NULL)
    {
        if (METADATA_STREAM_END(t))
        {
            buff_metadata_kip_prepare_eof_after_remove(t);
        }

        t->next = buff_metadata_kip_tag_from_buff(shared_buffer);

        /* This bit is not necessary for this while-loop but we need the tail
         * pointer for the next step. */
        if (t->next == NULL)
        {
            break;
        }
        else
        {
            t = t->next;
        }
    }

    *tail = t;
    return first_t;
}

metadata_tag* buff_metadata_push_tags_to_KIP(tCbuffer* shared_buffer,
                                                     metadata_tag* first_tag)
{
    metadata_tag* tag = first_tag;
    metadata_tag* temp_tag;

    while (NULL != tag &&
           buff_metadata_kip_tag_to_buff(shared_buffer, tag))
    {
        temp_tag = tag->next;
        buff_metadata_delete_tag(tag, FALSE);
        tag = temp_tag;
    }

    /* If we manage to push everything the KIP buffer this will be NULL */
    return tag;
}

bool buff_metadata_flush_KIP_tags(
               tCbuffer *buffer, metadata_tag **head_tag, metadata_tag **tail_tag)
{
    metadata_list *mlist;

    patch_fn_shared(kip_metadata);

    mlist = buffer->metadata;

    if (mlist != NULL)
    {
        unsigned prev_idx = mlist->prev_rd_index;
        metadata_tag  *temp_tag;

        /* Remove skipped tags */
        while (((*head_tag)        != NULL    ) && 
               ((*head_tag)->index  < prev_idx)    )
        {
            temp_tag       = (*head_tag);
            (*head_tag)    = (*head_tag)->next;
            temp_tag->next = NULL;
            buff_metadata_delete_tag(temp_tag, FALSE);
        }
        /* We're here if *head_tag is NULL, or if tag index  */
        /* >= prev_index (metadata buffer index).            */
        if ((*head_tag) == NULL)
        {
            (*tail_tag) = NULL;
            /* If *head_tag is NULL then there may be more   */
            /* skipped tags, so return FALSE so flag that    */
            /* buffer is not (necessarily) synchronised yet. */
            return FALSE;
        }
        /* Tag index >= prev_index (metadata buffer index)   */
        /* return TRUE to flag buffer is synchronised.       */
        return TRUE;
    }
    /* No buffer to synchronise with. Return TRUE. */
    return TRUE;
}
#endif /* INSTALL_METADATA_DUALCORE */

#ifdef DESKTOP_TEST_BUILD
void print_metadata(metadata_list *meta)
{
    if (meta == NULL)
    {
        printf("Metadata is NULL\n");
        return;
    }
    printf("Metadata with address %p\n", meta);
    printf("\tTag list tags.head: %p, tail: %p\n", meta->tags.head, meta->tags.tail);
    printf("\tReference count: %d\n", meta->ref_cnt);
    printf("\tNext Metadata: %p\n", meta->next);

}

void print_metadata_tag(metadata_tag *tag)
{
    if (tag == NULL)
    {
        printf("Tag is NULL\n");
        return;
    }
    printf("Metadata tag with address %p\n", tag);
    printf("\tNext Tag: %p\n", tag->next);
    printf("\tBuffer index (octets): %d\n", tag->index);
    printf("\tData length: %d\n", tag->length);
    printf("\tFlags: %x\n", tag->flags);
    printf("\tTimestamp: %d\n", tag->timestamp);
}

void print_metadata_priv(metadata_tag *tag)
{
    unsigned count = 0, num_items;
    metadata_priv_item *item;
    if (tag == NULL)
    {
        printf("Tag is NULL\n");
        return;
    }
    if (tag->xdata == NULL)
    {
        printf("Private data is NULL\n");
        return;
    }
    num_items = tag->xdata->item_count;
    item = (metadata_priv_item *)&tag->xdata->items[0];
    while (count < num_items)
    {
        printf("Item at %p key: %u length: %u data[0]: 0x%X \n", item, item->key, item->length, item->data[0]);

        item = PRIV_ITEM_NEXT(item);
        count++;
    }
}

#endif /* DESKTOP_TEST_BUILD */

#endif /* INSTALL_METADATA */
