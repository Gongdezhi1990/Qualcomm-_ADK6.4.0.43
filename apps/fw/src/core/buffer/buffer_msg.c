/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Functions associated with buffer messages.
*/

#include "buffer/buffer_private.h"


/** Advance the front of a message stream and update index */
void buf_add_to_front(BUFFER_MSG *msg_buf, uint16 len)
{
    buf_raw_write_update(&msg_buf->buf, len);
    msg_buf->msg_lengths[msg_buf->front] = len;
    msg_buf->front = BUF_NEXT_MSG(msg_buf->front);
}

/** Advance the back of a message stream but don't update outdex */
void buf_update_back_only(BUFFER_MSG *msg_buf)
{
    msg_buf->back = BUF_NEXT_MSG(msg_buf->back);
    msg_buf->back_header_bytes_consumed = 0;
}

/** Advance the back of a message stream and update outdex */
void buf_update_back(BUFFER_MSG *msg_buf)
{
    buf_raw_read_update(&msg_buf->buf, msg_buf->msg_lengths[msg_buf->back]);
    buf_update_back_only(msg_buf);
}


bool buf_unupdate_back(BUFFER_MSG *msg_buf)
{
    /* If the data are still there, push back and outdex back (npi) to their
     * previous setting */
    if (msg_buf->behind != msg_buf->back)
    {
        msg_buf->back = BUF_PREV_MSG(msg_buf->back);
        buf_raw_read_update(&msg_buf->buf,
            BUF_GET_SIZE_OCTETS(&msg_buf->buf) - msg_buf->msg_lengths[msg_buf->back]);
        msg_buf->back_header_bytes_consumed = 0;
        return TRUE;
    }
    return FALSE;

}

/** Advance the behind of a message stream, freeing memory */
void buf_update_behind_free(BUFFER_MSG *msg_buf)
{
    /*
     * Update the tail:  so long as buffer sizes are powers of
     * two (which they are), then the function will take care
     * of reducing the new_tail to within the buffer size.
     */
    ipc_send_buf_update_behind_free(msg_buf);
}

/** Advance the behind of a message stream, without freeing memory */
void buf_update_behind(BUFFER_MSG *msg_buf)
{
    /*
     * Update the tail:  so long as buffer sizes are powers of
     * two (which they are), then the function will take care
     * of reducing the new_tail to within the buffer size.
     */
    msg_buf->buf.tail = (uint16)((msg_buf->buf.tail +
                            msg_buf->msg_lengths[msg_buf->behind])
                        & (BUF_GET_SIZE_OCTETS(&msg_buf->buf) - 1));
    msg_buf->behind = BUF_NEXT_MSG(msg_buf->behind);
}

uint16 buf_combine_entries_at_offset(BUFFER_MSG * msg_buf, uint16 offset,
                                                        uint16 meta_size_bytes)
{
    uint16 entry, entry_offset;

    /* Make sure the requested offset is within the range of the buffer */
    offset = (uint16)(offset & msg_buf->buf.size_mask);

    entry = msg_buf->behind;
    entry_offset = msg_buf->buf.tail;
    /*
     * Step through the message ring looking for an entry that starts at the
     * given offset. The offset of behind is given by the tail pointer and
     * we add on the length of each message ring entry as we go along.
     */
    while(entry != msg_buf->back && entry_offset != offset)
    {
        entry_offset = (uint16)((entry_offset + msg_buf->msg_lengths[entry])
                                                    & msg_buf->buf.size_mask);
        entry = BUF_NEXT_MSG(entry);
    }
    if(entry_offset != offset)
    {
        return 0;
    }
    return buf_combine_entries(msg_buf, entry, meta_size_bytes);
}

uint16 buf_combine_entries(BUFFER_MSG * msg_buf, uint16 entry,
                                                        uint16 meta_size_bytes)
{
    uint16 next_entry, prev_entry, combined_length_bytes;

    next_entry = BUF_NEXT_MSG(entry);
    if(next_entry == msg_buf->back)
    {
        /* There aren't two packets to combine */
        return 0;
    }

    /* Combine the two entry lengths and copy the meta data */
    combined_length_bytes = (uint16)(msg_buf->msg_lengths[next_entry] +
                                            msg_buf->msg_lengths[entry]);
    msg_buf->msg_lengths[next_entry] = combined_length_bytes;
    if(meta_size_bytes)
    {
        memcpy(BUF_GET_ENTRY_GENERIC_META(msg_buf, next_entry, meta_size_bytes),
            BUF_GET_ENTRY_GENERIC_META(msg_buf, entry, meta_size_bytes),
            meta_size_bytes);
    }

    /* Now step back through the list to shuffle each entry along one place */
    while(entry != msg_buf->behind)
    {
        prev_entry = BUF_PREV_MSG(entry);
        msg_buf->msg_lengths[entry] = msg_buf->msg_lengths[prev_entry];
        if(meta_size_bytes)
        {
            memcpy(BUF_GET_ENTRY_GENERIC_META(msg_buf, entry, meta_size_bytes),
                BUF_GET_ENTRY_GENERIC_META(msg_buf, prev_entry, meta_size_bytes),
                meta_size_bytes);
        }
        entry = prev_entry;
    }
    /*
     * Zero the length of the message entry at behind (for cosmetic
     * and diagnostic reasons only)
     */
    msg_buf->msg_lengths[msg_buf->behind] = 0;
    /*
     * Advance the behind pointer by one to skip over the zero length entry
     * and make more space in the message ring
     */
    msg_buf->behind = BUF_NEXT_MSG(msg_buf->behind);

    return combined_length_bytes;
}
