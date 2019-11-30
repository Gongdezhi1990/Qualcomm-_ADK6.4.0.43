/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file hydra_cbuff.c
 * \ingroup buffer
 *
 * cbuffer 'C' code.
 */

/****************************************************************************
Include Files
*/

#include "buffer_private.h"
#include "hydra_cbuff.h"
#include "platform/pl_trace.h"



/****************************************************************************
Private Functions
*/
void cbuffer_advance_read_ptr_mmu_buff(tCbuffer *, int);
void cbuffer_advance_write_ptr_mmu_buff(tCbuffer *, int);

/*
 * \brief Maps circular buffer descriptor flags to mmu flags.
 *  packing not supported.
 * \param cb_desc circular buffer descriptor
 * \param get_wr_flags If set, get write flags else get read flags
 * \return mmu flags
 */
static inline unsigned cb_descriptor_to_mmu_flags(unsigned cb_desc, bool get_wr_flags)
{
    unsigned mmu_flags = get_wr_flags ? CBUF_TO_MMU_WR_PROT_FLAGS(cb_desc) : CBUF_TO_MMU_RD_PROT_FLAGS(cb_desc);

    return mmu_flags | CBUF_TO_MMU_SAMP_SIZE_FLAGS(cb_desc);
}

/*
 * \brief Helper function to populate cbuffer fields.
 *  Used by mmu_buffer_create and mmu_buffer_wrap.
 *  Clones write or aux handles as necessary.
 * \param cbuffer_struc_ptr pointer to cbuffer structure that needs filling up
 * \param handle valid mmu handle
 * \param flags cbuffer descriptor
 * \param buffer_size buffer size in chars
 */
static void create_mmu_buffer_common(tCbuffer *cbuffer_struc_ptr, mmu_handle handle,
                                unsigned int flags, unsigned int buffer_size, int *base_addr)
{
    audio_buf_handle_struc *temp_ptr; /* Pointer to the (first) buffer handle we create */

    patch_fn_shared(hydra_cbuff_create);

    /* buffer_size to cbuffer internal units */
    cbuffer_struc_ptr->size = buffer_size;

    /* Get hold of the buffer handle pointer and set up the base address */
    temp_ptr = mmu_buffer_get_handle_ptr_from_idx(handle.index);

    cbuffer_struc_ptr->base_addr = base_addr;

    if(BUF_DESC_RD_PTR_TYPE_MMU(flags) && !BUF_DESC_WR_PTR_TYPE_MMU(flags))
    {
        /* Read handle is MMU, Write handle is SW */
        cbuffer_struc_ptr->read_ptr = (int*)temp_ptr;
        cbuffer_struc_ptr->write_ptr = cbuffer_struc_ptr->base_addr;
    }
    else if (!BUF_DESC_RD_PTR_TYPE_MMU(flags) && BUF_DESC_WR_PTR_TYPE_MMU(flags))
    {
        /* Read handle is SW, Write handle is MMU */
        cbuffer_struc_ptr->read_ptr = cbuffer_struc_ptr->base_addr;
        cbuffer_struc_ptr->write_ptr = (int*)temp_ptr;
    }
    else
    {
        /* The complicated case: both handles are MMU.
         * Use the mmu_handle we have as the read handle, and clone it
         * to use as the write handle.
         */
        unsigned mmu_flags;
        mmu_handle wr_handle = mmu_buffer_clone_handle(handle);
        if (mmu_index_is_null(wr_handle.index))
        {
            /* We must have run out of buffer handles. */
            panic_diatribe(PANIC_AUDIO_BUFFER_HANDLES_EXHAUSTED, (DIATRIBE_TYPE)((uintptr_t)cbuffer_struc_ptr));
        }

        mmu_flags = cb_descriptor_to_mmu_flags(flags, TRUE);

        /* Set the appropriate flags for the write handle */
        mmu_buffer_set_flags(wr_handle, mmu_flags);

        /* make the write pointer point to the new buffer handle */
        cbuffer_struc_ptr->read_ptr = (int *)temp_ptr;
        cbuffer_struc_ptr->write_ptr = (int *)mmu_buffer_get_handle_ptr_from_idx(wr_handle.index);

        /* Now sort out the aux buffer, if necessary. */
        if (BUF_DESC_AUX_PTR_PRESENT(flags))
        {
            /* Clone the mmu_handle again */
            mmu_handle aux_handle = mmu_buffer_clone_handle(handle);
            if (mmu_index_is_null(aux_handle.index))
            {
                /* We must have run out of buffer handles. */
                panic_diatribe(PANIC_AUDIO_BUFFER_HANDLES_EXHAUSTED, (DIATRIBE_TYPE)((uintptr_t)cbuffer_struc_ptr));
            }

            /* Set the appropriate flags */
            if(BUF_DESC_AUX_PTR_TYPE(flags))
            {
                mmu_buffer_set_flags(aux_handle, cb_descriptor_to_mmu_flags(flags, TRUE));
            }
            else
            {
                mmu_buffer_set_flags(aux_handle, cb_descriptor_to_mmu_flags(flags, FALSE));
            }

            /* make the aux pointer point to the new buffer handle */
            cbuffer_struc_ptr->aux_ptr = (int *)mmu_buffer_get_handle_ptr_from_idx(aux_handle.index);
        }
    }

    /* Store the flags. */
    cbuffer_struc_ptr->descriptor = flags & BUF_DESC_STICKY_FLAGS_MASK;
}

/****************************************************************************
 *
 * cbuffer_create_mmu_buffer - used to create local MMU buffers
 *
 * The buffer data space will be allocated inside the function
 *
 * Input arguments:
 *      flags               - aggregated flags referring to read_ptr, write_ptr and aux_ptr
 *      buffer_size         - no of words for local MMU (it might be adjusted by submsg_mmu_create_buffer)
 *      preference          - MALLOC_PREFERENCE to allocate the data memory from
 *
 * Return value
 *      If successful - pointer to the created cbuffer structure else NULL
 *
 */
static tCbuffer *cbuffer_create_mmu_buffer_preference(unsigned int flags, unsigned int *buffer_size, unsigned preference)
{
    tCbuffer *cbuffer_struc_ptr;
    mmu_handle handle; /* (First) mmu handle we create */
    unsigned buffer_size_in_octets;
    bool get_wr_flags;
    int *buffer;

    if(!BUF_DESC_BUFFER_TYPE_MMU(flags))
    {
        /* This combination is effectively the same as cbuffer_create_with_malloc,
         * but for the convenience of other modules we permit it. */
        return cbuffer_create_with_malloc_preference(*buffer_size, BUF_DESC_SW_BUFFER, MALLOC_PREFERENCE_NONE, preference);
    }

    patch_fn_shared(hydra_cbuff_create);

    cbuffer_struc_ptr = zpnew(tCbuffer); /* Could panic */

    buffer_size_in_octets = SAMPLES_TO_BAC_OFFSET(*buffer_size, CBUF_TO_MMU_SAMP_SIZE_FLAGS(flags));

    /* If both RD and WR are MMU, get RD flags and generate RD handle first */
    get_wr_flags = (!BUF_DESC_RD_PTR_TYPE_MMU(flags) && BUF_DESC_WR_PTR_TYPE_MMU(flags));
    if ((buffer = mmu_buffer_create(&buffer_size_in_octets, &handle, cb_descriptor_to_mmu_flags(flags, get_wr_flags), preference)) == NULL)
    {
        /* Pass on the failure */
        pdelete(cbuffer_struc_ptr);
        return NULL;
    }

    create_mmu_buffer_common(cbuffer_struc_ptr, handle, flags, BAC_OFFSET_TO_CHARS(buffer_size_in_octets, CBUF_TO_MMU_SAMP_SIZE_FLAGS(flags)), buffer);

    PL_PRINT_P0(TR_CBUFFER, "cbuffer_create_mmu_buffer: ");
    PL_PRINT_BUFFER(TR_CBUFFER, cbuffer_struc_ptr);
    return cbuffer_struc_ptr;
}

/****************************************************************************
 *
 * cbuffer_create_mmu_buffer - used to create local MMU buffers
 *
 * The buffer data space will be allocated from SLOW RAM inside the function
 *
 * Input arguments:
 *      flags               - aggregated flags referring to read_ptr, write_ptr and aux_ptr
 *      buffer_size         - no of words for local MMU (it might be adjusted by submsg_mmu_create_buffer)
 *
 * Return value
 *      If successful - pointer to the created cbuffer structure else NULL
 *
 */
tCbuffer *cbuffer_create_mmu_buffer(unsigned int flags, unsigned int *buffer_size)
{
    return cbuffer_create_mmu_buffer_preference(flags, buffer_size, MALLOC_PREFERENCE_NONE);
}

/****************************************************************************
 *
 * cbuffer_create_mmu_buffer_fast - used to create local MMU buffers
 *
 * The buffer data space will be allocated from FAST RAM inside the function
 *
 * Input arguments:
 *      flags               - aggregated flags referring to read_ptr, write_ptr and aux_ptr
 *      buffer_size         - no of words for local MMU (it might be adjusted by submsg_mmu_create_buffer)
 *
 * Return value
 *      If successful - pointer to the created cbuffer structure else NULL
 *
 */
tCbuffer *cbuffer_create_mmu_buffer_fast(unsigned int flags, unsigned int *buffer_size)
{
    return cbuffer_create_mmu_buffer_preference(flags, buffer_size, MALLOC_PREFERENCE_FAST);
}

/****************************************************************************
 *
 * cbuffer_wrap_remote - used for wrapping remote MMU buffers in cbuffer structures
 *
 * Input arguments:
 *      flags                   - aggregated flags referring to permissions to modify read and write handles
 *      remote_handle1          - read MMU handle (as understood by hydra code)
 *      remote_handle2          - write MMU handle (as understood by hydra code)
 *      buffer_size             - remote MMU buffer size
 *
 * Return value
 *      If successful - pointer to the created cbuffer structure else NULL
 *
 */
tCbuffer *cbuffer_wrap_remote(int flags, mmu_handle read_handle, mmu_handle write_handle, unsigned int buffer_size)
{
    tCbuffer *cbuffer_struc_ptr;

    patch_fn_shared(hydra_cbuff_create);

    cbuffer_struc_ptr = zpnew(tCbuffer); /* Could panic */
    cbuffer_struc_ptr->read_ptr = (int *)(uintptr_t)mmu_handle_pack(read_handle);
    cbuffer_struc_ptr->write_ptr = (int *)(uintptr_t)mmu_handle_pack(write_handle);
    cbuffer_struc_ptr->size = buffer_size;
    cbuffer_struc_ptr->descriptor = flags;

    PL_PRINT_P0(TR_CBUFFER, "cbuffer_wrap_remote: ");
    PL_PRINT_BUFFER(TR_CBUFFER, cbuffer_struc_ptr);
    return cbuffer_struc_ptr;
}



/*
 *    Buffer
 *  |------- |                                     |
 *  |        |                                     |   amount space
 *  |--------|   <- write_offset   ^               v
 *  |########|                     | amount data
 *  |########|                     |
 *  |--------|   <- read_offset    v               ^
 *  |        |                                     |
 *  |--------|                                     |
*/


/****************************************************************************
 *
 * cbuffer_calc_amount_data_remote_buff -
 * Input arguments:
 *      cbuffer   - pointer to a cbuffer structure to read from
 *
 * Return value
 *      Amount of data in the buffer, in words.
 *
 * NB. This is only used for remote buffers.
 */
unsigned int cbuffer_calc_amount_data_remote_buff(tCbuffer *cbuffer)
{
    int ret;
    mmu_handle rd_handle, wr_handle;
    int rd_offset, wr_offset;    /* they are naturally unsigned, this is one way
                                     to make sure their difference is signed */
    patch_fn_shared(hydra_cbuff_remote);

    if (BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor))
    {
        mmu_handle_unpack(cbuffer->read_ptr, &rd_handle);
        mmu_handle_unpack(cbuffer->write_ptr, &wr_handle);

        rd_offset = mmu_buffer_get_handle_offset(rd_handle);
        wr_offset = mmu_buffer_get_handle_offset(wr_handle);

        /* Amount of data is write offset - read offset (see sketch above). If
         * the result is negative the write offset wrapped and the buffer size
         * needs to be added to the result.
         * Careful: the offsets are in octets, so we need to convert to words. */
        ret = buffer_octets_to_words(wr_offset - rd_offset);  /* must be signed subtraction */
        if (ret < 0)
        {
            ret += cbuffer->size;
        }
    }
    else
    {
        /* Shouldn't use the _remote_buff methods for local buffers. */
        fault_diatribe(FAULT_AUDIO_UNSUPPORTED, (DIATRIBE_TYPE)((uintptr_t)cbuffer));
        return 0;
    }

    return ret;
}



/****************************************************************************
 *
 * cbuffer_calc_amount_space_remote_buff
 *
 * Input arguments:
 *      cbuffer   - pointer to a cbuffer structure to read from
 *
 * Return value
 *      Amount of space in the buffer, in words.
 *
 * NB. This is only used for remote buffers.
 */
unsigned int cbuffer_calc_amount_space_remote_buff(tCbuffer *cbuffer)
{
    /* Amount of space in buffer is buffer size - amount_data */
    unsigned int space = cbuffer->size - cbuffer_calc_amount_data_remote_buff(cbuffer);

    /* Always say it's 1 word less so that buffer never gets totally filled up */
    return (space - 1);
}



/****************************************************************************
 *
 * cbuffer_read_remote_buff - read data from remote buffer
 *
 * Input arguments:
 *      cbuffer   - pointer to a cbuffer structure that wraps the remote buffer
 *      buffer    - destination buffer
 *      amount_to_read  - amount to read, in words
 *
 * Return value
 *      how much data we actually read, in words.
 */
unsigned int cbuffer_read_remote_buff(tCbuffer *cbuffer, int *buffer, unsigned int amount_to_read)
{
    patch_fn_shared(hydra_cbuff_remote);

    /* If the buffer is remote, need to call into mmu_buffer. */
    if( BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor) )
    {
        if(BUF_DESC_REMOTE_RDH_MOD(cbuffer->descriptor))
        {
            mmu_handle handle;
            bool it_worked;
            unsigned int amount_in_buffer;

            /* Check amount of data actually in the buffer. The amount we want to
             * read is min (amount_to_read, amount_in_buffer).
             * use the private, more direct remote_buff version, we know it's remote */
            amount_in_buffer = cbuffer_calc_amount_data_remote_buff(cbuffer);
            amount_to_read = (amount_in_buffer < amount_to_read) ? amount_in_buffer : amount_to_read;

            /* Get hold of the read handle */
            mmu_handle_unpack(cbuffer->read_ptr, &handle);

            it_worked = mmu_buffer_read(handle, mmu_buffer_get_handle_offset(handle),
                    buffer_words_to_octets(amount_to_read), buffer,
                    BUF_DESC_REMOTE_RDH_BSWAP(cbuffer->descriptor));
            if(!it_worked)
            {
                amount_to_read = 0;
            }
            else
            {
                cbuffer_advance_read_ptr_mmu_buff(cbuffer, amount_to_read);
            }
        }
        else
        {
            /* We are not allowed to change the MMU handle */
            panic_diatribe(PANIC_AUDIO_MODIFY_ATTEMPT_ON_PROTECTED_MMU_HANDLE, (DIATRIBE_TYPE)((uintptr_t)cbuffer));
        }
    }
    else
    {
        /* Shouldn't use the _remote_buff methods for local buffers. */
        fault_diatribe(FAULT_AUDIO_UNSUPPORTED, (DIATRIBE_TYPE)((uintptr_t)cbuffer));
        return 0;
    }
    return amount_to_read;
}



/****************************************************************************
 *
 * cbuffer_write_remote_buff - write data to remote buffer
 *
 * Input arguments:
 *      cbuffer   - pointer to a cbuffer structure that wraps the remote buffer
 *      buffer    - source buffer
 *      amount_to_write  - amount to write, in words
 *
 * Return value
 *      how much data we actually wrote, in words.
 */
unsigned int cbuffer_write_remote_buff(tCbuffer *cbuffer, int *buffer, unsigned int amount_to_write)
{
    patch_fn_shared(hydra_cbuff_remote);

    /* If the buffer is remote, need to call into mmu_buffer. */
    if( BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor) )
    {
        if(BUF_DESC_REMOTE_WRH_MOD(cbuffer->descriptor))
        {
            mmu_handle handle;
            bool it_worked;
            unsigned int amount_of_space;

            /* Check amount of space in the buffer. The amount we want to
             * write is min (amount_to_write, amount_of_space).
             * use the private, more direct remote_buff version, we know it's remote */
            amount_of_space = cbuffer_calc_amount_space_remote_buff(cbuffer);
            amount_to_write = (amount_of_space < amount_to_write) ? amount_of_space : amount_to_write;

            /* Get hold of the write handle */
            mmu_handle_unpack(cbuffer->write_ptr, &handle);

            /* We need to use a separate function if we want to perform a byte swap */
            if (BUF_DESC_REMOTE_WRH_BSWAP(cbuffer->descriptor))
            {
                it_worked = mmu_buffer_write_circular(handle, mmu_buffer_get_handle_offset(handle),
                        buffer_words_to_octets(amount_to_write),
                        buffer, 0,
                        buffer_words_to_octets(amount_to_write), TRUE);
            }
            else
            {
                it_worked = mmu_buffer_write(handle, mmu_buffer_get_handle_offset(handle),
                        buffer_words_to_octets(amount_to_write),
                        (const uint8 *)buffer);
            }
            if(!it_worked)
            {
                amount_to_write = 0;
            }
            else
            {
                cbuffer_advance_write_ptr_mmu_buff(cbuffer, amount_to_write);
            }
        }
        else
        {
            /* We are not allowed to change the MMU handle */
            panic_diatribe(PANIC_AUDIO_MODIFY_ATTEMPT_ON_PROTECTED_MMU_HANDLE, (DIATRIBE_TYPE)((uintptr_t)cbuffer));
        }
    }
    else
    {
        /* Shouldn't use the _remote_buff methods for local buffers. */
        fault_diatribe(FAULT_AUDIO_UNSUPPORTED, (DIATRIBE_TYPE)((uintptr_t)cbuffer));
        return 0;
    }

    return amount_to_write;
}


/****************************************************************************
 *
 * cbuffer_copy - copy data from one cbuffer to another
 *
 * Input arguments:
 *      cbuffer_src     - pointer to a cbuffer structure to read from
 *      cbuffer_dest    - pointer to a cbuffer structure to write to
 *      amount_to_copy  - amount to copy, in words
 *
 * Return value
 *      how much data we actually copied, in words.
 *
 * Note
 *      one buffer is supposed to be remote.
 */
unsigned int cbuffer_copy_remote_buff(tCbuffer *cbuffer_dest, tCbuffer *cbuffer_src, unsigned int amount_to_copy)
{
    mmu_handle handle;
    bool it_worked = FALSE;
    unsigned int amount_in_src_buffer, amount_of_dest_space;

    patch_fn_shared(hydra_cbuff_remote);

    if( BUF_DESC_IS_REMOTE_MMU(cbuffer_src->descriptor) &&
        BUF_DESC_IS_REMOTE_MMU(cbuffer_dest->descriptor) )
    {
        /* We don't (currently) support copying from one remote buffer to another.
         * It's a pain because we'd need to use two different remote windows in
         * the mmu_buffer code. */
        fault_diatribe(FAULT_AUDIO_UNSUPPORTED, (DIATRIBE_TYPE)((uintptr_t)cbuffer_src));
        return 0;
    }

    /* Check amount of data actually in the buffers. The amount we want to
     * copy is min (amount_to_copy, amount_in_src_buffer, amount_of_dest_space). */
    amount_in_src_buffer = cbuffer_calc_amount_data_in_words(cbuffer_src);
    amount_of_dest_space = cbuffer_calc_amount_space_in_words(cbuffer_dest);
    amount_to_copy = (amount_in_src_buffer < amount_to_copy) ? amount_in_src_buffer : amount_to_copy;
    amount_to_copy = (amount_of_dest_space < amount_to_copy) ? amount_of_dest_space : amount_to_copy;

    if( BUF_DESC_IS_REMOTE_MMU(cbuffer_src->descriptor) )
    {
        unsigned int dest_buf_offset; /* in words; Not trivial to get this */
        bool byte_swap = BUF_DESC_REMOTE_RDH_BSWAP(cbuffer_src->descriptor);

        if(!BUF_DESC_REMOTE_RDH_MOD(cbuffer_src->descriptor))
        {
            /* We are not allowed to change the MMU handle */
            panic_diatribe(PANIC_AUDIO_MODIFY_ATTEMPT_ON_PROTECTED_MMU_HANDLE, (DIATRIBE_TYPE)((uintptr_t)cbuffer_src));
        }

        /* Get hold of the remote read handle (remote buffers don't have aux
         * handles, so we can ignore that) */
        mmu_handle_unpack(cbuffer_src->read_ptr, &handle);

        /* Extract the (local) write handle information from the dest buffer.
         * Could be using an aux handle. */
        dest_buf_offset = cbuffer_get_write_offset(cbuffer_dest);

        /* Do the copy, converting amounts into octets.
         * NB - this is sub-optimal; mmu_buffer_read_circular will convert back into
         * words again to call the circular_buffer_copy routine anyway. */
        it_worked = mmu_buffer_read_circular(
                handle,
                mmu_buffer_get_handle_offset(handle),
                buffer_words_to_octets(amount_to_copy),
                (uint8 *)cbuffer_dest->base_addr,
                buffer_words_to_octets(dest_buf_offset),
                buffer_words_to_octets(cbuffer_dest->size),
                byte_swap);

        /* advance cbuffer pointers if it worked */
        if(it_worked)
        {
            cbuffer_advance_read_ptr_mmu_buff(cbuffer_src, amount_to_copy);
        }
    }

    else if( BUF_DESC_IS_REMOTE_MMU(cbuffer_dest->descriptor) )
    {
        unsigned int src_buf_offset; /* in words; Not trivial to get this */
        bool byte_swap = BUF_DESC_REMOTE_WRH_BSWAP(cbuffer_dest->descriptor);

        if(!BUF_DESC_REMOTE_WRH_MOD(cbuffer_dest->descriptor))
        {
            /* We are not allowed to change the MMU handle */
            panic_diatribe(PANIC_AUDIO_MODIFY_ATTEMPT_ON_PROTECTED_MMU_HANDLE, (DIATRIBE_TYPE)((uintptr_t)cbuffer_dest));
        }

        /* Get hold of the remote write handle (remote buffers don't have aux
         * handles, so we can ignore that) */
        mmu_handle_unpack(cbuffer_dest->write_ptr, &handle);

        /* Extract the (local) read handle information from the src buffer.
         * Could be using an aux handle. */
        src_buf_offset = cbuffer_get_read_offset(cbuffer_src);

        /* Do the copy, converting amounts into octets.
         * NB - this is sub-optimal; mmu_buffer_write_circular will convert back into
         * words again to call the circular_buffer_copy routine anyway. */
        it_worked = mmu_buffer_write_circular(handle,
                mmu_buffer_get_handle_offset(handle),
                buffer_words_to_octets(amount_to_copy),
                cbuffer_src->base_addr,
                buffer_words_to_octets(src_buf_offset),
                buffer_words_to_octets(cbuffer_src->size),
                byte_swap);

        /* advance cbuffer pointers if it worked */
        if(it_worked)
        {
            cbuffer_advance_write_ptr_mmu_buff(cbuffer_dest, amount_to_copy);
        }
    }
    else
    {
        /* Shouldn't use the _remote_buff methods for local buffers (they are both local) */
        fault_diatribe(FAULT_AUDIO_UNSUPPORTED, (DIATRIBE_TYPE)((uintptr_t)cbuffer_dest)); /* src local as well */
        return 0;
    }

    return (it_worked) ? amount_to_copy : 0;
}


#ifndef TODO_BUFFER_ONE_OCTET
// cbuffer_copy_one_octet needs cbuffer_get_write_address_and_size
//    and that is no longer public neither C accessible
//    audio-cpu only called from asm decoder_fw opmgr_shunt_copy

/****************************************************************************
 *
 * cbuffer_copy_one_octet - copy a single octet from one cbuffer to another
 *
 * Input arguments:
 *      cbuffer_src     - pointer to a cbuffer structure to read from
 *      cbuffer_dest    - pointer to a cbuffer structure to write to
 *      append          - if TRUE, copy the octet to word before cbuffer_dest->wrp
 *
 * Return value
 *      TRUE if we copied an octet, FALSE otherwise
 *
 * NOTE: Unlike the other cbuffer read/write functions, this one also adjusts
 * (or not) the write pointer of cbuffer_dest so that it points to the next
 * full word to be written.
 */
bool cbuffer_copy_one_octet(tCbuffer *cbuffer_dest, tCbuffer *cbuffer_src, bool append)
{
    uint16 the_octet;

    patch_fn_shared(hydra_cbuff_modify);

    /* First, read the octet out of the src buffer */
    if (BUF_DESC_IS_REMOTE_MMU(cbuffer_src->descriptor))
    {
        int rd_offset, wr_offset;
        int buffer_size_octets;
        mmu_handle rd_handle, wr_handle;
        bool it_worked;

        /* Check that we're allowed to modify the read handle. */
        if(!BUF_DESC_REMOTE_RDH_MOD(cbuffer_src->descriptor))
        {
            panic_diatribe(PANIC_AUDIO_MODIFY_ATTEMPT_ON_PROTECTED_MMU_HANDLE, (DIATRIBE_TYPE)((uintptr_t)cbuffer_src));
        }

        /* Check amount of data in the src buffer: it needs to hold at least one octet. */
        /* This is a slightly ugly re-implementation of cbuffer_calc_amount_data which
         * returns the result in octets rather than words. */
        mmu_handle_unpack(cbuffer_src->read_ptr, &rd_handle);
        mmu_handle_unpack(cbuffer_src->write_ptr, &wr_handle);

        rd_offset = mmu_buffer_get_handle_offset(rd_handle);
        wr_offset = mmu_buffer_get_handle_offset(wr_handle);

        /* Don't care exactly how much data there is in the buffer -
         * just that there is some (i.e. offsets aren't the same).
         */
        if( wr_offset == rd_offset )
        {
            return FALSE;
        }

        /* Do the read.
         * We can use the normal 'read' function, and pass in a single word
         * instead of an array as the 'destination buffer' */
        it_worked = mmu_buffer_read(rd_handle, rd_offset, 1, &the_octet, BUF_DESC_REMOTE_RDH_BSWAP(cbuffer_src->descriptor));

        if(!it_worked)
        {
            return FALSE;
        }

        /* Get the octet into the LSB of the word. If b'swap was set,
         * the octet will have been put into the MSB. */
        if (BUF_DESC_REMOTE_RDH_BSWAP(cbuffer_src->descriptor))
        {
            the_octet = the_octet >> 8;
        }

        /* This is an equally ugly re-implementation of cbuffer_advance_read_ptr
         * which can deal in octets. */
        buffer_size_octets = buffer_words_to_octets(cbuffer_src->size);

        /* Increment, accounting for wrapping.
         * (The bitwise-and only works because our buffer size is
         * currently always a power of two) */
        rd_offset += 1;
        rd_offset &= (buffer_size_octets - 1);

        mmu_buffer_set_handle_offset(rd_handle, rd_offset);
    }
    else
    {
        /* We could support a non-remote buffer as src if we wanted, but no need to implement that right now. */
        fault_diatribe(FAULT_AUDIO_UNSUPPORTED, (DIATRIBE_TYPE)((uintptr_t)cbuffer_src));
        return FALSE;
    }

    /* Now copy the octet into the destination buffer.
     * Also, adjust (or not) the write pointer so that it points to the next full
     * word to be written. */
    if( !BUF_DESC_IS_REMOTE_MMU(cbuffer_dest->descriptor) )
    {
        int *ptr_to_dest_word;
        int  buffer_size; /* We already know this, meh. */
        cbuffer_get_write_address_and_size(cbuffer_dest, &ptr_to_dest_word, &buffer_size);

        /* Put the octet into the LSB of the destination word, unless 'append' is set.
         * In that case we need to put it into the MSB of the word _before_ the current
         * write pointer. */
        if (append)
        {
            int tmp;
            /* Decrement by one.
             * Naasty cheat here to account for a buffer wrap,
             * which works because we're only adjusting by 1. */
            if (ptr_to_dest_word == cbuffer_dest->base_addr)
            {
                int offset_of_last_word = cbuffer_dest->size -1;
                ptr_to_dest_word = (int*)((uintptr_t)cbuffer_dest->base_addr + offset_of_last_word*sizeof(int));
            }
            else
            {
                ptr_to_dest_word -= 1;
            }

            tmp = *ptr_to_dest_word;
            tmp |= the_octet;
            *ptr_to_dest_word = tmp;

            /* Don't increment the write pointer */
        }
        else
        {
            *ptr_to_dest_word = the_octet;

            /* Now increment the write pointer; needs to point
             * to next full word. */
            cbuffer_advance_write_ptr_local(cbuffer_dest, 1);
        }
    }
    else
    {
        /* We could support a remote MMU buffer as destination if we wanted, but no need to implement that right now. */
        fault_diatribe(FAULT_AUDIO_UNSUPPORTED, (DIATRIBE_TYPE)((uintptr_t)cbuffer_dest));
        return FALSE;
    }

    return TRUE;
}
#endif      // TODO_BUFFER_ONE_OCTET


/**/
void cbuffer_advance_read_ptr_mmu_buff(tCbuffer *cbuffer, int amount)
{
    patch_fn_shared(hydra_cbuff_modify);

    /* If the buffer is remote, first check that we're allowed to modify the read handle. */
    if( BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor) && !BUF_DESC_REMOTE_RDH_MOD(cbuffer->descriptor))
    {
        panic_diatribe(PANIC_AUDIO_MODIFY_ATTEMPT_ON_PROTECTED_MMU_HANDLE, (DIATRIBE_TYPE)((uintptr_t)cbuffer));
    }

    /* Call into mmu_buffer if the handle is controlled by the MMU,
          could be remote or local HW buffer */
    if( BUF_DESC_REMOTE_RDH_MOD(cbuffer->descriptor) ||
        BUF_DESC_RD_PTR_TYPE_MMU(cbuffer->descriptor) )
    {
        mmu_handle handle;
        int offset;
        unsigned int buffer_size_octets;
        /* Get hold of the current read offset */
        if (BUF_DESC_RD_PTR_TYPE_MMU(cbuffer->descriptor))
        {
            handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->read_ptr);
        }
        else
        {
            mmu_handle_unpack(cbuffer->read_ptr, &handle);
        }

        offset = mmu_buffer_get_handle_offset(handle); /* offset in octets */

        /* Increment, accounting for wrapping.
         * (The bitwise-and only works because our buffer size is
         * currently always a power of two) */
        buffer_size_octets = CHARS_TO_BAC_OFFSET(cbuffer->size, mmu_buffer_get_flags(handle));
        offset += SAMPLES_TO_BAC_OFFSET(amount, mmu_buffer_get_flags(handle));
        offset &= (buffer_size_octets - 1);
        mmu_buffer_set_handle_offset(handle, offset);
    }
    else
    {
        /* Shouldn't use the _mmu_buff methods for local SW buffers */
        fault_diatribe(FAULT_AUDIO_UNSUPPORTED, (DIATRIBE_TYPE)((uintptr_t)cbuffer->read_ptr));
    }
}


/**/
void cbuffer_advance_write_ptr_mmu_buff(tCbuffer *cbuffer, int amount)
{
    patch_fn_shared(hydra_cbuff_modify);

    /* If the buffer is remote, first check that we're allowed to modify the write handle. */
    if( BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor) && !BUF_DESC_REMOTE_WRH_MOD(cbuffer->descriptor))
    {
        panic_diatribe(PANIC_AUDIO_MODIFY_ATTEMPT_ON_PROTECTED_MMU_HANDLE, (DIATRIBE_TYPE)((uintptr_t)cbuffer));
    }

    /* Call into mmu_buffer if the handle is controlled by the MMU,
          could be remote or local HW buffer */
    if( BUF_DESC_REMOTE_WRH_MOD(cbuffer->descriptor) ||
        BUF_DESC_WR_PTR_TYPE_MMU(cbuffer->descriptor) )
    {
        mmu_handle handle;
        int offset;
        unsigned int buffer_size_octets;
        /* Get hold of the current write offset */
        if (BUF_DESC_WR_PTR_TYPE_MMU(cbuffer->descriptor))
        {
            handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->write_ptr);
        }
        else
        {
            mmu_handle_unpack(cbuffer->write_ptr, &handle);
        }

        offset = mmu_buffer_get_handle_offset(handle); /* offset in octets */

        /* Increment, accounting for wrapping.
         * (The bitwise-and only works because our buffer size is
         * currently always a power of two) TODO could compare and wrap round if over (also _read_)
         */
        buffer_size_octets = CHARS_TO_BAC_OFFSET(cbuffer->size, mmu_buffer_get_flags(handle));
        offset += SAMPLES_TO_BAC_OFFSET(amount, mmu_buffer_get_flags(handle)); /* amount is in words */
        offset &= (buffer_size_octets - 1);

        mmu_buffer_set_handle_offset(handle, offset);
    }
    else
    {
        /* Shouldn't use the _mmu_buff methods for SW buffers */
        fault_diatribe(FAULT_AUDIO_UNSUPPORTED, (DIATRIBE_TYPE)((uintptr_t)cbuffer->write_ptr));
    }
}



/****************************************************************************
 *
 * cbuffer_get_read_mmu_handle - returns read mmu handle
 *
 * Input argument:
 *      cbuffer   - pointer to the cbuffer structure to query
 *
 * Return value
 *      constructed mmu_handle or empty mmu_handle if rd_ptr does not point to MMU buffer handle
 *
 */
mmu_handle cbuffer_get_read_mmu_handle(tCbuffer *cbuffer)
{
    mmu_handle ret_handle = MMU_HANDLE_NULL;

    if( BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor) )
    {
        mmu_handle_unpack(cbuffer->read_ptr, &ret_handle);
    }
    else
    {
        if(BUF_DESC_RD_PTR_TYPE_MMU(cbuffer->descriptor))
        {
            ret_handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->read_ptr);
        }
    }
    return ret_handle;
}


/****************************************************************************
 *
 * cbuffer_get_write_mmu_handle - returns write mmu handle
 *
 * Input argument:
 *      cbuffer   - pointer to the cbuffer structure to query
 *
 * Return value
 *      constructed mmu_handle or empty mmu_handle if wr_ptr does not point to MMU buffer handle
 *
 */
mmu_handle cbuffer_get_write_mmu_handle(tCbuffer *cbuffer)
{
    mmu_handle ret_handle = MMU_HANDLE_NULL;

    if( BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor) )
    {
        mmu_handle_unpack(cbuffer->write_ptr, &ret_handle);
    }
    else
    {
        if(BUF_DESC_WR_PTR_TYPE_MMU(cbuffer->descriptor))
        {
            ret_handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->write_ptr);
        }
    }
    return ret_handle;
}

/****************************************************************************
 *
 * cbuffer_get_aux_mmu_handle - returns the aux mmu handle, if present
 *
 * Input argument:
 *      cbuffer   - pointer to the cbuffer structure to query
 *
 * Return value
 *      constructed mmu_handle or empty mmu_handle if wr_ptr does not point to MMU buffer handle
 *
 */
mmu_handle cbuffer_get_aux_mmu_handle(tCbuffer *cbuffer)
{
    mmu_handle ret_handle = MMU_HANDLE_NULL;

    if( BUF_DESC_AUX_PTR_PRESENT(cbuffer->descriptor) )
    {
        ret_handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->aux_ptr);
    }
    return ret_handle;
}

/**/
bool cbuffer_is_read_handle_mmu(tCbuffer *cbuffer)
{
    return BUF_DESC_RD_PTR_TYPE_MMU(cbuffer->descriptor);
}

bool cbuffer_is_write_handle_mmu(tCbuffer *cbuffer)
{
    return BUF_DESC_WR_PTR_TYPE_MMU(cbuffer->descriptor);
}

bool cbuffer_is_remote(tCbuffer *cbuffer)
{
    return BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor);
}

/* Return BUFF_AUX_PTR_TYPE_NONE if there is no aux handle,
 * BUFF_AUX_PTR_TYPE_READ if there is an aux handle used for read,
 * BUFF_AUX_PTR_TYPE_WRITE if there is an aux handle used for write.
 */
int cbuffer_aux_in_use(tCbuffer *cbuffer)
{
    if( BUF_DESC_AUX_PTR_PRESENT(cbuffer->descriptor) )
    {
        return (BUF_DESC_AUX_PTR_TYPE(cbuffer->descriptor)) ?
                                                BUFF_AUX_PTR_TYPE_WRITE :
                                                BUFF_AUX_PTR_TYPE_READ;
    }
    else
    {
        return BUFF_AUX_PTR_TYPE_NONE;
    }
}

/****************************************************************************
 *
 * cbuffer_set_read_shift - set the shift amount for the read mmu handle
 *
 * Input arguments:
 *      cbuffer   - pointer to a cbuffer structure
 *      amount    - size of shift
 *
 * Return value
 *      TRUE if successful, FALSE otherwise.
 *
 */
bool cbuffer_set_read_shift(tCbuffer *cbuffer, int amount)
{
    unsigned flags;
    mmu_handle handle;

    patch_fn_shared(hydra_cbuff_modify);

    /* Read handle should be local MMU */
    if (BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor) ||
        !BUF_DESC_RD_PTR_TYPE_MMU(cbuffer->descriptor))
    {
        /* Can only set shift on a local MMU handle */
        return FALSE;
    }

    /* Get the read handle */
    handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->read_ptr);

    /* Set up the shift for the read handle */
    flags = mmu_buffer_get_flags(handle);
    MMU_BUF_SHIFT_SET(flags, amount);
    mmu_buffer_set_flags(handle, flags);

    /* If the aux handle is in use as a read handle */
    if (cbuffer_aux_in_use(cbuffer) == BUFF_AUX_PTR_TYPE_READ)
    {
        /* Get the aux handle */
        handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->aux_ptr);

        /* Set up the same shift for the aux handle */
        flags = mmu_buffer_get_flags(handle);
        MMU_BUF_SHIFT_SET(flags, amount);
        mmu_buffer_set_flags(handle, flags);
    }

    /* Succeeded */
    return TRUE;
}

/****************************************************************************
 *
 * cbuffer_set_write_shift - set the shift amount for the write mmu handle
 *
 * Input arguments:
 *      cbuffer   - pointer to a cbuffer structure
 *      amount    - size of shift
 *
 * Return value
 *      TRUE if successful, FALSE otherwise.
 *
 */
bool cbuffer_set_write_shift(tCbuffer *cbuffer, int amount)
{
    unsigned flags;
    mmu_handle handle;

    patch_fn_shared(hydra_cbuff_modify);

    /* Write handle should be local MMU */
    if (BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor) ||
        !BUF_DESC_WR_PTR_TYPE_MMU(cbuffer->descriptor))
    {
        /* Can only set shift on a local MMU handle */
        return FALSE;
    }

    /* Get the write handle */
    handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->write_ptr);

    /* Set up the shift for the write handle */
    flags = mmu_buffer_get_flags(handle);
    MMU_BUF_SHIFT_SET(flags, amount);
    mmu_buffer_set_flags(handle, flags);

    /* If the aux handle is in use as a write handle */
    if (cbuffer_aux_in_use(cbuffer) == BUFF_AUX_PTR_TYPE_WRITE)
    {
        /* Get the aux handle */
        handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->aux_ptr);

        /* Set up the same shift for the aux handle */
        flags = mmu_buffer_get_flags(handle);
        MMU_BUF_SHIFT_SET(flags, amount);
        mmu_buffer_set_flags(handle, flags);
    }

    /* Succeeded */
    return TRUE;
}

/****************************************************************************
 *
 * cbuffer_set_read_byte_swap - set byte swap for the read mmu handle
 *
 * Input arguments:
 *      cbuffer   - pointer to a cbuffer structure
 *      set       - bool indicating set or clear TRUE = set, False = clear
 *
 * Return value
 *      TRUE if successful, FALSE otherwise.
 *
 */
bool cbuffer_set_read_byte_swap(tCbuffer *cbuffer, bool set)
{
    patch_fn_shared(hydra_cbuff_modify);

    /* We can set a byte swap for remote MMU, but the process is completely
     * different from configuring a local buffer. We need to remember that
     * the buffer is to be byte-swapped, and set the appropriate flag when
      * performing a read. */
    if (BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor) && BUF_DESC_REMOTE_RDH_MOD(cbuffer->descriptor))
    {
        if (set)
        {
            BUF_DESC_REMOTE_RDH_BSWAP_SET(cbuffer->descriptor);
        }
        else
        {
            BUF_DESC_REMOTE_RDH_BSWAP_UNSET(cbuffer->descriptor);
        }

        return TRUE;
    }

    /* Is the read handle a local MMU buffer ?*/
    if (BUF_DESC_RD_PTR_TYPE_MMU(cbuffer->descriptor))
    {
        unsigned flags;
        mmu_handle handle;

        /* Get the read handle */
        handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->read_ptr);

        /* Set up the shift for the read handle */
        flags = mmu_buffer_get_flags(handle);
        if (set)
        {
            MMU_BUF_BSWAP_SET(flags);
        }
        else
        {
            MMU_BUF_BSWAP_UNSET(flags);
        }
        mmu_buffer_set_flags(handle, flags);

        /* If the aux handle is in use as a read handle */
        if (cbuffer_aux_in_use(cbuffer) == BUFF_AUX_PTR_TYPE_READ)
        {
            /* Get the aux handle */
            handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->aux_ptr);

            /* Set up the same shift for the aux handle */
            flags = mmu_buffer_get_flags(handle);
            if (set)
            {
                MMU_BUF_BSWAP_SET(flags);
            }
            else
            {
                MMU_BUF_BSWAP_UNSET(flags);
            }
            mmu_buffer_set_flags(handle, flags);
        }

        /* Succeeded */
        return TRUE;
    }

    /* If we got here, the buffer was either pure SW, or a remote buffer
     * not configured for read */
    return FALSE;
}

/****************************************************************************
 *
 * cbuffer_set_write_byte_swap - set byte swap for the write mmu handle
 *
 * Input arguments:
 *      cbuffer   - pointer to a cbuffer structure
 *      set       - bool indicating set or clear TRUE = set, False = clear
 * Return value
 *      TRUE if successful, FALSE otherwise.
 *
 */
bool cbuffer_set_write_byte_swap(tCbuffer *cbuffer, bool set)
{
    patch_fn_shared(hydra_cbuff_modify);

    /* We can set a byte swap for remote MMU, but the process is completely
     * different from configuring a local buffer. We need to remember that
     * the buffer is to be byte-swapped, and set the appropriate flag when
      * performing a write. */
    if (BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor) && BUF_DESC_REMOTE_WRH_MOD(cbuffer->descriptor))
    {
        if (set)
        {
            BUF_DESC_REMOTE_WRH_BSWAP_SET(cbuffer->descriptor);
        }
        else
        {
            BUF_DESC_REMOTE_WRH_BSWAP_UNSET(cbuffer->descriptor);
        }
        return TRUE;
    }

    /* Is the write handle a local MMU buffer ?*/
    if (BUF_DESC_WR_PTR_TYPE_MMU(cbuffer->descriptor))
    {
        unsigned flags;
        mmu_handle handle;

        /* Get the write handle */
        handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->write_ptr);

        /* Set up the shift for the write handle */
        flags = mmu_buffer_get_flags(handle);
        if (set)
        {
            MMU_BUF_BSWAP_SET(flags);
        }
        else
        {
            MMU_BUF_BSWAP_UNSET(flags);
        }
        mmu_buffer_set_flags(handle, flags);

        /* If the aux handle is in use as a write handle */
        if (cbuffer_aux_in_use(cbuffer) == BUFF_AUX_PTR_TYPE_WRITE)
        {
            /* Get the aux handle */
            handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->aux_ptr);

            /* Set up the same shift for the aux handle */
            flags = mmu_buffer_get_flags(handle);
            if (set)
            {
                MMU_BUF_BSWAP_SET(flags);
            }
            else
            {
                MMU_BUF_BSWAP_UNSET(flags);
            }
            mmu_buffer_set_flags(handle, flags);
        }

        /* Succeeded */
        return TRUE;
    }

    /* If we got here, the buffer was either pure SW, or a remote buffer
     * not configured for write */
    return FALSE;
}

/****************************************************************************
 *
 * cbuffer_get_read_shift - get the shift amount for the read mmu handle
 *
 * Input arguments:
 *      cbuffer  pointer to a cbuffer structure
 *
 * Return value
 *      size of shift currently configured.
 *
 */
unsigned int cbuffer_get_read_shift(tCbuffer *cbuffer)
{
    unsigned flags;
    mmu_handle handle;

    patch_fn_shared(hydra_cbuff_modify);

    /* Read handle should be local MMU */
    if (BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor) ||
        !BUF_DESC_RD_PTR_TYPE_MMU(cbuffer->descriptor))
    {
        /* Can only set shift on a local MMU handle */
        return 0;
    }

    /* Get the read handle */
    handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->read_ptr);

    /* Get up the shift amount for the read handle */
    flags = mmu_buffer_get_flags(handle);
    return (unsigned int)MMU_BUF_SHIFT_GET(flags);
}

/****************************************************************************
 *
 * cbuffer_get_write_shift - get the shift amount for the write mmu handle
 *
 * Input arguments:
 *      cbuffer  pointer to a cbuffer structure
 *
 * Return value
 *      size of shift currently configured.
 *
 */
unsigned int cbuffer_get_write_shift(tCbuffer *cbuffer)
{
    unsigned flags;
    mmu_handle handle;

    patch_fn_shared(hydra_cbuff_modify);

    /* Read handle should be local MMU */
    if (BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor) ||
        !BUF_DESC_RD_PTR_TYPE_MMU(cbuffer->descriptor))
    {
        /* Can only set shift on a local MMU handle */
        return 0;
    }

    /* Get the read handle */
    handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->write_ptr);

    /* Get up the shift amount for the read handle */
    flags = mmu_buffer_get_flags(handle);
    return (unsigned int)MMU_BUF_SHIFT_GET(flags);
}

/****************************************************************************
 *
 * cbuffer_get_read_offset - gets the offset into the buffer of the read pointer.
 *
 * Input arguments:
 *      cbuffer   - pointer to a cbuffer structure
 *
 * Return value
 *      read offset for the supplied cbuffer, in words
 */
unsigned int cbuffer_get_read_offset(tCbuffer *cbuffer)
{
    int *read_ptr;

    /* OBSOLETE cbuffer_get_read_address_and_size(cbuffer, &read_ptr, &buffer_size); */

    patch_fn_shared(hydra_cbuff_offset);

    read_ptr = cbuffer->read_ptr;

    if (BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor) )
    {
        mmu_handle handle;

        mmu_handle_unpack(read_ptr, &handle);
        return buffer_octets_to_words( mmu_buffer_get_handle_offset(handle) );
    }
    else if (BUF_DESC_RD_PTR_TYPE_MMU(cbuffer->descriptor) )
    {
        mmu_handle handle;

        /* Extract the (local) read handle information from the cbuffer.
         * Could be using an aux handle. */
        if (cbuffer_aux_in_use(cbuffer) == BUFF_AUX_PTR_TYPE_READ )
        {
            read_ptr = cbuffer->aux_ptr;
        }
        handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)read_ptr);
        return BAC_OFFSET_TO_SAMPLES(mmu_buffer_get_handle_offset(handle), mmu_buffer_get_flags(handle));
    }
    else
    {
        /* otherwise must be SW buffer (although shouldn't use offsets with pure SW buffers)
           more likely a rd handle of a HW WR buffer */
        /* Difference in pointers needs to be divided by number of memory locations */
        return ((uintptr_t)read_ptr - (uintptr_t)cbuffer->base_addr)/sizeof(unsigned int);
    }
}


/****************************************************************************
 *
 * cbuffer_get_write_offset - gets the offset into the buffer of the write pointer.
 *
 * Input arguments:
 *      cbuffer   - pointer to a cbuffer structure
 *
 * Return value
 *      write offset for the supplied cbuffer, in words
 */
unsigned int cbuffer_get_write_offset(tCbuffer *cbuffer)
{
    int *write_ptr;

    /* OBSOLETE cbuffer_get_write_address_and_size(cbuffer, &write_ptr, &buffer_size); */

    patch_fn_shared(hydra_cbuff_offset);

    write_ptr = cbuffer->write_ptr;

    if (BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor) )
    {
        mmu_handle handle;

        mmu_handle_unpack(write_ptr, &handle);
        return buffer_octets_to_words( mmu_buffer_get_handle_offset(handle) );
    }
    else if (BUF_DESC_WR_PTR_TYPE_MMU(cbuffer->descriptor) )
    {
        mmu_handle handle;

        /* Extract the (local) write handle information from the cbuffer.
         * Could be using an aux handle. */
        if (cbuffer_aux_in_use(cbuffer) == BUFF_AUX_PTR_TYPE_WRITE )
        {
            write_ptr = cbuffer->aux_ptr;
        }
        handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)write_ptr);
        return BAC_OFFSET_TO_SAMPLES(mmu_buffer_get_handle_offset(handle), mmu_buffer_get_flags(handle));
    }
    else
    {
        /* otherwise must be SW buffer (although shouldn't use offsets with pure SW buffers)
           more likely a wr handle of a HW RD buffer */
        /* Difference in pointers needs to be divided by number of memory locations */
        return ((uintptr_t)write_ptr - (uintptr_t)cbuffer->base_addr)/sizeof(unsigned int);
    }
}

/****************************************************************************
 *
 * cbuffer_set_read_offset - sets the offset into the buffer of the read pointer.
 *
 * Input arguments:
 *      cbuffer   - pointer to a cbuffer structure
 *      offset    - read offset for the supplied cbuffer, in words
 *
 * Notes. Should not be called on pure SW buffers.
 *        Use with caution, offset should not exceed buffer_size.
 */
void cbuffer_set_read_offset(tCbuffer *cbuffer, unsigned int offset)
{

    patch_fn_shared(hydra_cbuff_offset);

    if( offset >= (cbuffer->size))
    {
        panic_diatribe(PANIC_AUDIO_DEBUG_ASSERT_FAILED, cbuffer->size);
    }

    if( (cbuffer->descriptor) == BUF_DESC_SW_BUFFER)
    {
        /* pure SW buffers shouldn't use offsets. */
        panic_diatribe(PANIC_AUDIO_DEBUG_ASSERT_FAILED, cbuffer->descriptor);
    }
    else
    {
        if(!BUF_DESC_RD_PTR_TYPE_MMU(cbuffer->descriptor))
        {
            /* not mmu rd handle, must be SW rd pointer */
            int  *new_rd_ptr;

            /* Offset in words converted to number of memory locations */
            new_rd_ptr = (int*)((uintptr_t)cbuffer->base_addr + (uintptr_t)offset*sizeof(int));
            /* aux should not be modified, so we can ignore that) */
            /* OBSOLETE cbuffer_set_read_address, but we know it's SW handle */
            cbuffer->read_ptr = new_rd_ptr;
        }
        else
        {
            /* must be mmu or remote */
            mmu_handle handle;
            if (BUF_DESC_BUFFER_TYPE_MMU(cbuffer->descriptor))
            {
                handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->read_ptr);
            }
            else
            {
                /* otherwise must be remote */
                mmu_handle_unpack(cbuffer->read_ptr, &handle);
            }
            mmu_buffer_set_handle_offset(handle, SAMPLES_TO_BAC_OFFSET(offset, mmu_buffer_get_flags(handle)));
        }
    }
}


/****************************************************************************
 *
 * cbuffer_set_write_offset - sets the offset into the buffer of the write pointer.
 *
 * Input arguments:
 *      cbuffer   - pointer to a cbuffer structure
 *      offset    - write offset for the supplied cbuffer, in words
 *
 * Notes. Should not be called on pure SW buffers.
 *        Use with caution, offset should not exceed buffer_size.
 */
void cbuffer_set_write_offset(tCbuffer *cbuffer, unsigned int offset)
{
    patch_fn_shared(hydra_cbuff_offset);

    if( offset >= (cbuffer->size))
    {
        panic_diatribe(PANIC_AUDIO_DEBUG_ASSERT_FAILED, cbuffer->size);
    }

    if( (cbuffer->descriptor) == BUF_DESC_SW_BUFFER )
    {
        /* pure SW buffers shouldn't use offsets. */
        panic_diatribe(PANIC_AUDIO_DEBUG_ASSERT_FAILED, cbuffer->descriptor);
    }
    else
    {
        if(!BUF_DESC_WR_PTR_TYPE_MMU(cbuffer->descriptor))
        {
            /* not mmu wr handle, must be SW wr pointer */
            int  *new_wr_ptr;

            /* Offset in words converted to number of memory locations */
            new_wr_ptr = (int*)((uintptr_t)cbuffer->base_addr + (uintptr_t)offset*sizeof(int));
            /* aux should not be modified, so we can ignore that) */
            /* OBSOLETE cbuffer_set_write_address, but we know it's SW handle */
            cbuffer->write_ptr = new_wr_ptr;
        }
        else
        {
            /* must be mmu or remote */
            mmu_handle handle;
            if (BUF_DESC_BUFFER_TYPE_MMU(cbuffer->descriptor))
            {
                handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->write_ptr);
            }
            else
            {
                /* otherwise must be remote */
                mmu_handle_unpack(cbuffer->write_ptr, &handle);
            }
            mmu_buffer_set_handle_offset(handle, SAMPLES_TO_BAC_OFFSET(offset, mmu_buffer_get_flags(handle)));
        }
    }
}

/****************************************************************************
 *
 * cbuffer_set_read_sample_size - Sets the sample size for the read mmu handle
 *
 * Input arguments:
 *      cbuffer     - pointer to a cbuffer structure
 *      sample_size - enum indicating the size of data stored in buffer
 * Return value
 *      TRUE if successful, FALSE otherwise.
 *
 */
bool cbuffer_set_read_sample_size(tCbuffer *cbuffer, buffer_sample_size sample_size)
{
    unsigned flags;
    mmu_handle handle;

    patch_fn_shared(hydra_cbuff_sample);

    /* Read handle should be local MMU */
    if (BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor) ||
        !BUF_DESC_RD_PTR_TYPE_MMU(cbuffer->descriptor))
    {
        /* Can only set sample size on a local MMU handle */
        return FALSE;
    }

    /* Get the read handle */
    handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->read_ptr);

    /* Set up the sample size for the read handle */
    flags = mmu_buffer_get_flags(handle);
    MMU_BUF_SAMP_SZ_SET(flags, sample_size);
    mmu_buffer_set_flags(handle, flags);

    /* If the aux handle is in use as a read handle */
    if (cbuffer_aux_in_use(cbuffer) == BUFF_AUX_PTR_TYPE_READ)
    {
        /* Get the aux handle */
        handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->aux_ptr);

        /* Set up the same sample size for the aux handle */
        flags = mmu_buffer_get_flags(handle);
        MMU_BUF_SAMP_SZ_SET(flags, sample_size);
        mmu_buffer_set_flags(handle, flags);
    }

    /* Succeeded */
    return TRUE;
}

/****************************************************************************
 *
 * cbuffer_set_write_sample_size - Sets the sample size for the write mmu handle
 *
 * Input arguments:
 *      cbuffer     - pointer to a cbuffer structure
 *      sample_size - enum indicating the size of data stored in buffer
 * Return value
 *      TRUE if successful, FALSE otherwise.
 *
 */
bool cbuffer_set_write_sample_size(tCbuffer *cbuffer, buffer_sample_size sample_size)
{
    unsigned flags;
    mmu_handle handle;

    patch_fn_shared(hydra_cbuff_sample);

    /* Write handle should be local MMU */
    if (BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor) ||
        !BUF_DESC_WR_PTR_TYPE_MMU(cbuffer->descriptor))
    {
        /* Can only set sample size on a local MMU handle */
        return FALSE;
    }

    /* Get the write handle */
    handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->write_ptr);

    /* Set up the sample size for the write handle */
    flags = mmu_buffer_get_flags(handle);
    MMU_BUF_SAMP_SZ_SET(flags, sample_size);
    mmu_buffer_set_flags(handle, flags);

    /* If the aux handle is in use as a write handle */
    if (cbuffer_aux_in_use(cbuffer) == BUFF_AUX_PTR_TYPE_WRITE)
    {
        /* Get the aux handle */
        handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->aux_ptr);

        /* Set up the same sample size for the aux handle */
        flags = mmu_buffer_get_flags(handle);
        MMU_BUF_SAMP_SZ_SET(flags, sample_size);
        mmu_buffer_set_flags(handle, flags);
    }

    /* Succeeded */
    return TRUE;
}

/****************************************************************************
 *
 * cbuffer_set_read_sign_extend - Sets the sign extend for the read mmu handle
 *
 * Input arguments:
 *      cbuffer   - pointer to a cbuffer structure
 *      set       - bool indicating set or clear TRUE = set, False = clear
 * Return value
 *      TRUE if successful, FALSE otherwise.
 *
 */
bool cbuffer_set_read_sign_extend(tCbuffer *cbuffer, bool set)
{
    unsigned flags;
    mmu_handle handle;

    patch_fn_shared(hydra_cbuff_modify);

    /* Read handle should be local MMU */
    if (BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor) ||
        !BUF_DESC_RD_PTR_TYPE_MMU(cbuffer->descriptor))
    {
        /* Can only set sign extend on a local MMU handle */
        return FALSE;
    }

    /* Get the read handle */
    handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->read_ptr);

    /* Set up the sign extend for the read handle */
    flags = mmu_buffer_get_flags(handle);
    if (set)
    {
        MMU_BUF_SE_SET(flags);
    }
    else
    {
        MMU_BUF_SE_UNSET(flags);
    }
    mmu_buffer_set_flags(handle, flags);

    /* If the aux handle is in use as a read handle */
    if (cbuffer_aux_in_use(cbuffer) == BUFF_AUX_PTR_TYPE_READ)
    {
        /* Get the aux handle */
        handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->aux_ptr);

        /* Set up the same sign extend for the aux handle */
        flags = mmu_buffer_get_flags(handle);
        if (set)
        {
            MMU_BUF_SE_SET(flags);
        }
        else
        {
            MMU_BUF_SE_UNSET(flags);
        }
        mmu_buffer_set_flags(handle, flags);
    }

    /* Succeeded */
    return TRUE;
}

/****************************************************************************
 *
 * cbuffer_set_write_sign_extend - Sets the sign extend for the write mmu handle
 *
 * Input arguments:
 *      cbuffer   - pointer to a cbuffer structure
 *      set       - bool indicating set or clear TRUE = set, False = clear
 * Return value
 *      TRUE if successful, FALSE otherwise.
 *
 */
bool cbuffer_set_write_sign_extend(tCbuffer *cbuffer, bool set)
{
    unsigned flags;
    mmu_handle handle;

    patch_fn_shared(hydra_cbuff_modify);

    /* Write handle should be local MMU */
    if (BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor) ||
        !BUF_DESC_WR_PTR_TYPE_MMU(cbuffer->descriptor))
    {
        /* Can only set sign extend on a local MMU handle */
        return FALSE;
    }

    /* Get the write handle */
    handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->write_ptr);

    /* Set up the sign extend for the write handle */
    flags = mmu_buffer_get_flags(handle);
    if (set)
    {
        MMU_BUF_SE_SET(flags);
    }
    else
    {
        MMU_BUF_SE_UNSET(flags);
    }
    mmu_buffer_set_flags(handle, flags);

    /* If the aux handle is in use as a write handle */
    if (cbuffer_aux_in_use(cbuffer) == BUFF_AUX_PTR_TYPE_WRITE)
    {
        /* Get the aux handle */
        handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->aux_ptr);

        /* Set up the same sign extend for the aux handle */
        flags = mmu_buffer_get_flags(handle);
        if (set)
        {
            MMU_BUF_SE_SET(flags);
        }
        else
        {
            MMU_BUF_SE_UNSET(flags);
        }
        mmu_buffer_set_flags(handle, flags);
    }

    /* Succeeded */
    return TRUE;
}

/****************************************************************************
 *
 * cbuffer_set_read_packing - Sets the packing for the read mmu handle
 *
 * Input arguments:
 *      cbuffer   - pointer to a cbuffer structure
 *      set       - bool indicating set or clear TRUE = set, False = clear
 * Return value
 *      TRUE if successful, FALSE otherwise.
 *
 */
bool cbuffer_set_read_packing(tCbuffer *cbuffer, bool set)
{
    unsigned flags;
    mmu_handle handle;

    patch_fn_shared(hydra_cbuff_sample);

    /* Read handle should be local MMU */
    if (BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor) ||
        !BUF_DESC_RD_PTR_TYPE_MMU(cbuffer->descriptor))
    {
        /* Can only set packing on a local MMU handle */
        return FALSE;
    }

    /* Get the read handle */
    handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->read_ptr);

    /* Set up the packing for the read handle */
    flags = mmu_buffer_get_flags(handle);
    if (set)
    {
        MMU_BUF_PKG_EN_SET(flags);
    }
    else
    {
        MMU_BUF_PKG_EN_UNSET(flags);
    }
    mmu_buffer_set_flags(handle, flags);

    /* If the aux handle is in use as a read handle */
    if (cbuffer_aux_in_use(cbuffer) == BUFF_AUX_PTR_TYPE_READ)
    {
        /* Get the aux handle */
        handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->aux_ptr);

        /* Set up the same packing for the aux handle */
        flags = mmu_buffer_get_flags(handle);
        if (set)
        {
            MMU_BUF_PKG_EN_SET(flags);
        }
        else
        {
            MMU_BUF_PKG_EN_UNSET(flags);
        }
        mmu_buffer_set_flags(handle, flags);
    }

    /* Succeeded */
    return TRUE;
}

/****************************************************************************
 *
 * cbuffer_set_write_packing - Sets the packing for the write mmu handle
 *
 * Input arguments:
 *      cbuffer   - pointer to a cbuffer structure
 *      set       - bool indicating set or clear TRUE = set, False = clear
 * Return value
 *      TRUE if successful, FALSE otherwise.
 *
 */
bool cbuffer_set_write_packing(tCbuffer *cbuffer, bool set)
{
    unsigned flags;
    mmu_handle handle;

    patch_fn_shared(hydra_cbuff_sample);

    /* Write handle should be local MMU */
    if (BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor) ||
        !BUF_DESC_WR_PTR_TYPE_MMU(cbuffer->descriptor))
    {
        /* Can only set packing on a local MMU handle */
        return FALSE;
    }

    /* Get the write handle */
    handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->write_ptr);

    /* Set up the packing for the write handle */
    flags = mmu_buffer_get_flags(handle);
    if (set)
    {
        MMU_BUF_PKG_EN_SET(flags);
    }
    else
    {
        MMU_BUF_PKG_EN_UNSET(flags);
    }
    mmu_buffer_set_flags(handle, flags);

    /* If the aux handle is in use as a write handle */
    if (cbuffer_aux_in_use(cbuffer) == BUFF_AUX_PTR_TYPE_WRITE)
    {
        /* Get the aux handle */
        handle = mmu_buffer_get_handle_from_ptr((audio_buf_handle_struc *)cbuffer->aux_ptr);

        /* Set up the same packing for the aux handle */
        flags = mmu_buffer_get_flags(handle);
        if (set)
        {
            MMU_BUF_PKG_EN_SET(flags);
        }
        else
        {
            MMU_BUF_PKG_EN_UNSET(flags);
        }
        mmu_buffer_set_flags(handle, flags);
    }

    /* Succeeded */
    return TRUE;
}

/****************************************************************************
 *
 * cbuffer_flush_and_fill_remote_buff - fills a remote cbuffer with the supplied value.
 *
 * This is different to (the asm-only) cbuffer.fill because that function
 * only fills up the available free space in the buffer. This fills the
 * _entire_ buffer.
 *
 * Input arguments:
 *      cbuffer   - pointer to a cbuffer structure
 *
 */
void cbuffer_flush_and_fill_remote_buff(tCbuffer *cbuffer, int fill_value)
{
    patch_fn_shared(hydra_cbuff_modify);

    /* This works (in theory) on remote buffers, but only if we're the one doing the writing */
    if (BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor))
    {
        if (BUF_DESC_REMOTE_WRH_MOD(cbuffer->descriptor))
        {
            /* Ok I know I just said this works for remote buffers, but for now
             * it's not supported.
             * Reason:
             *   If the remote buffer is in the BTSS, then attempting to actually
             *   fill it up may cause the BTSS to run out of buffer pages.
             * Mitigation:
             *   The main use case for this function is to 'silence' a cbuffer that
             *   that PCM hardware is chomping through. That's not going to be the
             *   case with a remote buffer anyway.
             */
            fault_diatribe(FAULT_AUDIO_UNSUPPORTED, (DIATRIBE_TYPE)((uintptr_t)cbuffer));
        }
        else
        {
            /* We are not allowed to modify the MMU write handle */
            panic_diatribe(PANIC_AUDIO_MODIFY_ATTEMPT_ON_PROTECTED_MMU_HANDLE, (DIATRIBE_TYPE)((uintptr_t)cbuffer));
        }
    }
    else
    {
        /* Shouldn't use the _remote_buff methods for local buffers */
        fault_diatribe(FAULT_AUDIO_UNSUPPORTED, (DIATRIBE_TYPE)((uintptr_t)cbuffer));
    }
}

unsigned cbuffer_get_mmu_flags(const tCbuffer *cb, bool read_flags)
{
    return mmu_buffer_get_flags(
                mmu_buffer_get_handle_from_ptr(
                    (audio_buf_handle_struc *)(read_flags ? cb->read_ptr : cb->write_ptr)));
}


/* TODO not needed, added for metadata / audio_data_service (queue),
    answer-me do we keep it?
int cbuffer_compare_offsets(tCbuffer * cb, unsigned off_1, unsigned off_2)
{
    unsigned wo = cbuffer_get_write_offset((tCbuffer *)cb);

    if (((off_1 > wo) && (off_2 > wo)) || ((off_1 <= wo) && (off_2 <= wo)))
    {
        return off_1 - off_2;
    }
    else
    {
        int size = cbuffer_get_size_in_words((tCbuffer *)cb);
        return  (off_1 > off_2) ? off_1 - off_2 - size : off_1 - off_2 + size;
    }
}
 */
