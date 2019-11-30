/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
#include "hydra/hydra_types.h"
#include "hydra_log/hydra_log.h"
#include "mmu/mmu.h"
#include "utils/utils_set.h"
#include "buffer/buffer.h"
#include "pmalloc/pmalloc.h"
#include "int/int.h"
#include "assert.h"
#include "sched/sched.h"

/**
 * Structure for a linked list of message buffers that we wish to access
 * through pointers into their data space.
 */
typedef struct buffer_msg_list_s
{
    BUFFER_MSG * msg_buf;
    struct buffer_msg_list_s * next;
} buffer_msg_list_t;

/**
 * Head of the linked list of message buffers.
 */
static buffer_msg_list_t * buffer_msg_list;

/**
 * Find whether the given message buffer is in the linked list.
 * \param buf Message buffer to search for
 * \return TRUE if it is in the list
 */
static bool find_buf_in_list(BUFFER_MSG * buf)
{
    buffer_msg_list_t * entry = buffer_msg_list;
    while(entry)
    {
        if(entry->msg_buf == buf)
            return TRUE;
        entry = entry->next;
    }
    return FALSE;
}

/**
 * Find the message buffer that uses the given MMU handle
 * \param handle The MMU handle we are interested in
 * \return The message buffer using the given handle or NULL if none
 * has been registered for that handle.
 */
static BUFFER_MSG * find_msg_buf_from_mmu(mmu_handle handle)
{
    buffer_msg_list_t * entry = buffer_msg_list;
    while(entry)
    {
        if(entry->msg_buf->buf.handle == handle)
            return entry->msg_buf;
        entry = entry->next;
    }
    return NULL;
}

void buf_register_msg_buf_for_ptr_access(BUFFER_MSG * buf)
{
    buffer_msg_list_t * new_entry;
    if(find_buf_in_list(buf))
    {
        return;
    }
    new_entry = pnew(buffer_msg_list_t);
    new_entry->msg_buf = buf;
    block_interrupts();
    new_entry->next = buffer_msg_list;
    buffer_msg_list = new_entry;
    unblock_interrupts();
}

void buf_unregister_msg_buf_for_ptr_access(BUFFER_MSG * buf)
{
    buffer_msg_list_t * p_entry;
    buffer_msg_list_t ** pp_entry = &buffer_msg_list;
    block_interrupts();
    while(*pp_entry)
    {
        p_entry = *pp_entry;
        if((*pp_entry)->msg_buf == buf)
        {
            *pp_entry = p_entry->next;
            pfree(p_entry);
            unblock_interrupts();
            return;
        }
        pp_entry = &(p_entry->next);
    }
    unblock_interrupts();
}

void buf_free_entries_from_ptr(void * ptr)
{
    uint16 behind_len;
    mmu_handle handle;
    uint16 offset;
    BUFFER_MSG * msg_buf;

    handle = mmu_local_handle(MMU_INDEX_FROM_ADDRESS_SPACE(ptr));
    msg_buf = find_msg_buf_from_mmu(handle);
    assert(msg_buf);
    assert(BUF_ANY_MSGS_SENT(msg_buf));
    offset = (uint16)((uint32)ptr & msg_buf->buf.size_mask);
    behind_len = buf_get_behind_msg_len(msg_buf);
    if(BUF_SUB_INDICES(&msg_buf->buf, offset, msg_buf->buf.tail) < behind_len)
    {
        L3_DBG_MSG3("BUFFER: MMU h%x Behind @0x%x consumed by ptr 0x%x",
                        (uint32)handle, msg_buf->buf.tail, (uint32)ptr);
        buf_update_behind_free(msg_buf);
        return;
    }
    else
    {
        L0_DBG_MSG2("BUFFER: Attempted out of order free 0x%x from mmu h%x",
                                                (uint32)ptr, (uint32)handle);
        assert(0);  /* Out of order freeing is not yet supported */
    }
}
